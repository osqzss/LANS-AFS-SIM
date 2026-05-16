#include <math.h>
#include <complex.h>
#include "multipath.h"

#define ERR_EPS 1e-12

static double dot3(const double a[3], const double b[3])
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static double norm3(const double a[3])
{
    return sqrt(dot3(a, a));
}

static void sub3(const double a[3], const double b[3], double c[3])
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}

static void add3(const double a[3], const double b[3], double c[3])
{
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}

static void scale3(const double a[3], double s, double b[3])
{
    b[0] = s * a[0];
    b[1] = s * a[1];
    b[2] = s * a[2];
}

static int unit3(const double a[3], double u[3])
{
    double n = norm3(a);

    if (n < ERR_EPS) {
        u[0] = u[1] = u[2] = 0.0;
        return 0;
    }

    u[0] = a[0] / n;
    u[1] = a[1] / n;
    u[2] = a[2] / n;
    return 1;
}

static void cross3(const double a[3], const double b[3], double c[3])
{
    c[0] = a[1] * b[2] - a[2] * b[1];
    c[1] = a[2] * b[0] - a[0] * b[2];
    c[2] = a[0] * b[1] - a[1] * b[0];
}

static double clamp(double x)
{
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

static int enu_basis(const double rx_pos[3], double east[3], double north[3], double up[3])
{
    double zref[3] = {0.0, 0.0, 1.0};
    double xref[3] = {1.0, 0.0, 0.0};
    double tmp[3];

    if (!unit3(rx_pos, up)) {
        return 0;
    }

    cross3(zref, up, tmp);

    if (norm3(tmp) < 1e-8) {
        cross3(xref, up, tmp);
    }

    if (!unit3(tmp, east)) {
        return 0;
    }

    cross3(up, east, north);

    if (!unit3(north, north)) {
        return 0;
    }

    return 1;
}

static int el_az(const double rx_pos[3], const double sat_pos[3],  double *elev, double *az, double los_unit[3])
{
    double east[3], north[3], up[3];
    double rho_vec[3];
    double e, n, u;
    double h;

    if (!enu_basis(rx_pos, east, north, up)) {
        return 0;
    }

    sub3(sat_pos, rx_pos, rho_vec);

    if (!unit3(rho_vec, los_unit)) {
        return 0;
    }

    e = dot3(los_unit, east);
    n = dot3(los_unit, north);
    u = dot3(los_unit, up);

    h = sqrt(e * e + n * n);

    *elev = atan2(u, h);
    *az   = atan2(e, n);

    return 1;
}

static double fresnel_reflection(double elev, double complex eps_r, int pol)
{
    double sin_e = sin(elev);
    double cos_e = cos(elev);

    double complex root;
    double complex gamma_h;
    double complex gamma_v;

    // cos(theta_i) = sin(elev)
    // sin(theta_i) = cos(elev)

    if (sin_e < 0.0) sin_e = 0.0;
    if (cos_e < 0.0) cos_e = 0.0;

    root = csqrt(eps_r - cos_e * cos_e);

    gamma_h = (sin_e - root) / (sin_e + root);
    gamma_v = (eps_r * sin_e - root) / (eps_r * sin_e + root);

    if (pol == 1) {
        return cabs(gamma_h);
    }
    else if (pol == 2) {
        return cabs(gamma_v);
    }
    else {
        double gh = cabs(gamma_h);
        double gv = cabs(gamma_v);

        // Average in power, then convert back to amplitude.
        return sqrt(0.5 * (gh * gh + gv * gv));
    }
}

static double roughness_attenuation(double elev, double lambda, double sigma)
{
    double x;

    if (sigma <= 0.0) {
        return 1.0;
    }

    if (lambda <= 0.0) {
        return 1.0;
    }

    x = 4.0 * M_PI * sigma * sin(elev) / lambda;

    return exp(-0.5 * x * x);
}

multipath_t reflection_error(const double rx_pos[3], const double sat_pos[3], const reflection_t *refl)
{
    multipath_t mp;
    double elev, az;
    double los[3];
    double gamma_mag;
    double rough_loss;
    double amp;

    double complex eps_r;

    mp.range_offset = 0.0;
    mp.amplitude_ratio = 0.0;
    mp.valid = 0;

    if (refl == 0) {
        return mp;
    }

    if (refl->lambda <= 0.0 || refl->antenna_height <= 0.0) {
        return mp;
    }

    if (!el_az(rx_pos, sat_pos, &elev, &az, los)) {
        return mp;
    }

    if (elev <= 0.0) {
        return mp;
    }

    mp.range_offset = 2.0 * refl->antenna_height * sin(elev);

    eps_r = refl->eps_r_real - I * refl->eps_r_imag;
    gamma_mag = fresnel_reflection(elev, eps_r, 0);
    rough_loss = roughness_attenuation(elev, refl->lambda, refl->surface_rms);

    amp = gamma_mag * rough_loss * refl->ant_gain_scale;

    mp.amplitude_ratio = clamp(amp);
    mp.valid = 1;

    return mp;
}

static double knife_edge_amp_scale(double v)
{
    double loss_db;

    if (v <= -0.7) {
        loss_db = 0.0;
    }
    else {
        loss_db = 6.9 + 20.0 * log10(sqrt((v - 0.1) * (v - 0.1) + 1.0) + v - 0.1);
    }

    return clamp(pow(10.0, -loss_db / 20.0));
}

static int edge_position(const double rx_pos[3], const double sat_pos[3], 
    double edge_distance, double mask_elev, double edge_pos[3])
{
    double east[3], north[3], up[3];
    double elev, az;
    double los[3];
    double horiz_dir[3];
    double tmp1[3], tmp2[3];
    double edge_height;

    if (edge_distance <= 0.0) {
        return 0;
    }

    if (!enu_basis(rx_pos, east, north, up)) {
        return 0;
    }

    if (!el_az(rx_pos, sat_pos, &elev, &az, los)) {
        return 0;
    }

    horiz_dir[0] = cos(az) * north[0] + sin(az) * east[0];
    horiz_dir[1] = cos(az) * north[1] + sin(az) * east[1];
    horiz_dir[2] = cos(az) * north[2] + sin(az) * east[2];

    if (!unit3(horiz_dir, horiz_dir)) {
        return 0;
    }

    edge_height = edge_distance * tan(mask_elev);

    scale3(horiz_dir, edge_distance, tmp1);
    scale3(up, edge_height, tmp2);

    add3(rx_pos, tmp1, edge_pos);
    add3(edge_pos, tmp2, edge_pos);

    return 1;
}

multipath_t diffraction_error(const double rx_pos[3], const double sat_pos[3], const diffraction_t *diff)
{
    multipath_t mp;
    double elev, az;
    double los[3];

    double edge_pos[3];
    double sat_to_rx[3];
    double sat_to_edge[3];
    double edge_to_rx[3];

    double d_direct;
    double d1;
    double d2;
    double excess;

    double edge_minus_rx[3];
    double rx_to_sat[3];
    double closest[3];
    double edge_to_closest[3];
    double t;
    double h_perp;
    double v;
    double amp;

    mp.range_offset = 0.0;
    mp.amplitude_ratio = 0.0;
    mp.valid = 0;

    if (diff == 0) {
        return mp;
    }

    if (diff->lambda <= 0.0 || diff->edge_distance <= 0.0) {
        return mp;
    }

    if (!el_az(rx_pos, sat_pos, &elev, &az, los)) {
        return mp;
    }

    if (elev >= diff->mask_elev) {
        return mp;
    }

    if (!edge_position(rx_pos, sat_pos, diff->edge_distance, diff->mask_elev, edge_pos)) {
        return mp;
    }

    sub3(sat_pos, rx_pos, sat_to_rx);
    sub3(sat_pos, edge_pos, sat_to_edge);
    sub3(edge_pos, rx_pos, edge_to_rx);

    d_direct = norm3(sat_to_rx);
    d2 = norm3(sat_to_edge);  // satellite-side distance
    d1 = norm3(edge_to_rx);   // receiver-side distance

    if (d_direct <= 0.0 || d1 <= 0.0 || d2 <= 0.0) {
        return mp;
    }

    excess = d1 + d2 - d_direct;

    if (excess < 0.0) {
        excess = 0.0;
    }

    sub3(edge_pos, rx_pos, edge_minus_rx);
    sub3(sat_pos, rx_pos, rx_to_sat);

    t = dot3(edge_minus_rx, rx_to_sat) / dot3(rx_to_sat, rx_to_sat);

    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    scale3(rx_to_sat, t, closest);
    add3(rx_pos, closest, closest);

    sub3(edge_pos, closest, edge_to_closest);

    h_perp = norm3(edge_to_closest);

    v = h_perp * sqrt(2.0 * (d1 + d2) / (diff->lambda * d1 * d2));

    amp = knife_edge_amp_scale(v);

    mp.range_offset = excess;
    mp.amplitude_ratio = clamp(amp);
    mp.valid = 1;

    return mp;
}
