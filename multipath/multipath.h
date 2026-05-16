#ifndef MULTIPATH_H
#define MULTIPATH_H

//#include <complex.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Antenna height [m]
#define REFL_ANTENNA_HEIGHT    2.0

// Real part of the complex relative permittivity of lunar regolith
#define REFL_EPS_R_REAL        3.0

// Imaginary part of the complex relative permittivity of lunar regolith
#define REFL_EPS_R_IMAG        0.01

// RMS surface roughness [m]
#define REFL_SURFACE_RMS       0.013

// Amplitude scale factor for the antenna response to the reflected signal
#define REFL_ANT_GAIN_SCALE    1.0

// Elevation mask angle [deg / rad]
#define DIFF_MASK_ELEV_DEG     21.8
#define DIFF_MASK_ELEV_RAD     (DIFF_MASK_ELEV_DEG * M_PI / 180.0)

// Horizontal distance from the receiver to the Shackleton crater rim [m]
#define DIFF_EDGE_DISTANCE          10000.0

typedef struct {
    double range_offset;
    double amplitude_ratio;
    int    valid;
} multipath_t;

typedef struct {
    double lambda;          // Wavelength [m]
    double antenna_height;  // Antenna height [m]
    double eps_r_real;      // Real part of the relative permittivity
    double eps_r_imag;      // Imaginary part of the relative permittivity
    double surface_rms;     // RMS surface roughness [m] (0 = disable)
    double ant_gain_scale;  // Antenna gain ratio for reflected signal
} reflection_t;

typedef struct {
    double lambda;         // Wavelength [m]
    double mask_elev;      // Elevation threshold [rad] (below = diffraction)
    double edge_distance;  // Horizontal receiver-to-edge distance [m]
} diffraction_t;

// Function prototypes
multipath_t reflection_error(const double rx_pos[3], const double sat_pos[3], const reflection_t *refl);
multipath_t diffraction_error(const double rx_pos[3], const double sat_pos[3], const diffraction_t *diff);

#endif /* MULTIPATH_H */
