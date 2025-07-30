#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include "afs_nav.h"
#include "afs_rand.h"

#define DEMO_L1

#ifdef DEMO_L1
#define LAMBDA (0.190293672798365)
#define CARR_FREQ (1575.42e6) // = 1.023MHz * 1540.0 = 5.115MHz * 308.0
#define I_CODE_FREQ (1.023e6) // BPSK(1)
#define CARR_TO_I_CODE (1.0/1540.0)
#define Q_CODE_FREQ (5.115e6) // BPSK(5)
#define CARR_TO_Q_CODE (1.0/308.0)
#else
#define LAMBDA (0.120300597746093)
#define CARR_FREQ (2492.028e6) // = 1.023MHz * 2436.0 = 5.115MHz * 487.2
#define I_CODE_FREQ (1.023e6) // BPSK(1)
#define CARR_TO_I_CODE (1.0/2436.0)
#define Q_CODE_FREQ (5.115e6) // BPSK(5)
#define CARR_TO_Q_CODE (1.0/487.2)
#endif

#define MAX_CHAR (512)
#define MAX_SAT (12)

#define PI 3.1415926535898
#define R2D 57.2957795131

#define GM_MOON 4.9028e12
#define R_MOON 1737.4e3

#define SECONDS_IN_WEEK 604800.0
#define SECONDS_IN_HALF_WEEK 302400.0

#define SPEED_OF_LIGHT 2.99792458e8

#define POW2_M19 1.907348632812500e-6
#define POW2_M31 4.656612873077393e-10
#define POW2_M32 2.328306436538696e-10
#define POW2_M43 1.136868377216160e-13

#define GAIN_SCALE (128)

int scode[4][4] = {{1,1,1,0},{0,1,1,1},{1,0,1,1},{1,1,0,1}};
int tcode[210][1500];

int sinT[] = {
    2,   5,   8,  11,  14,  17,  20,  23,  26,  29,  32,  35,  38,  41,  44,  47,
    50,  53,  56,  59,  62,  65,  68,  71,  74,  77,  80,  83,  86,  89,  91,  94,
    97, 100, 103, 105, 108, 111, 114, 116, 119, 122, 125, 127, 130, 132, 135, 138,
    140, 143, 145, 148, 150, 153, 155, 157, 160, 162, 164, 167, 169, 171, 173, 176,
    178, 180, 182, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204, 205, 207,
    209, 210, 212, 214, 215, 217, 218, 220, 221, 223, 224, 225, 227, 228, 229, 230,
    232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 241, 242, 243, 244, 244, 245,
    245, 246, 247, 247, 248, 248, 248, 249, 249, 249, 249, 250, 250, 250, 250, 250,
    250, 250, 250, 250, 250, 249, 249, 249, 249, 248, 248, 248, 247, 247, 246, 245,
    245, 244, 244, 243, 242, 241, 241, 240, 239, 238, 237, 236, 235, 234, 233, 232,
    230, 229, 228, 227, 225, 224, 223, 221, 220, 218, 217, 215, 214, 212, 210, 209,
    207, 205, 204, 202, 200, 198, 196, 194, 192, 190, 188, 186, 184, 182, 180, 178,
    176, 173, 171, 169, 167, 164, 162, 160, 157, 155, 153, 150, 148, 145, 143, 140,
    138, 135, 132, 130, 127, 125, 122, 119, 116, 114, 111, 108, 105, 103, 100,  97,
    94,  91,  89,  86,  83,  80,  77,  74,  71,  68,  65,  62,  59,  56,  53,  50,
    47,  44,  41,  38,  35,  32,  29,  26,  23,  20,  17,  14,  11,   8,   5,   2,
    -2,  -5,  -8, -11, -14, -17, -20, -23, -26, -29, -32, -35, -38, -41, -44, -47,
    -50, -53, -56, -59, -62, -65, -68, -71, -74, -77, -80, -83, -86, -89, -91, -94,
    -97,-100,-103,-105,-108,-111,-114,-116,-119,-122,-125,-127,-130,-132,-135,-138,
    -140,-143,-145,-148,-150,-153,-155,-157,-160,-162,-164,-167,-169,-171,-173,-176,
    -178,-180,-182,-184,-186,-188,-190,-192,-194,-196,-198,-200,-202,-204,-205,-207,
    -209,-210,-212,-214,-215,-217,-218,-220,-221,-223,-224,-225,-227,-228,-229,-230,
    -232,-233,-234,-235,-236,-237,-238,-239,-240,-241,-241,-242,-243,-244,-244,-245,
    -245,-246,-247,-247,-248,-248,-248,-249,-249,-249,-249,-250,-250,-250,-250,-250,
    -250,-250,-250,-250,-250,-249,-249,-249,-249,-248,-248,-248,-247,-247,-246,-245,
    -245,-244,-244,-243,-242,-241,-241,-240,-239,-238,-237,-236,-235,-234,-233,-232,
    -230,-229,-228,-227,-225,-224,-223,-221,-220,-218,-217,-215,-214,-212,-210,-209,
    -207,-205,-204,-202,-200,-198,-196,-194,-192,-190,-188,-186,-184,-182,-180,-178,
    -176,-173,-171,-169,-167,-164,-162,-160,-157,-155,-153,-150,-148,-145,-143,-140,
    -138,-135,-132,-130,-127,-125,-122,-119,-116,-114,-111,-108,-105,-103,-100, -97,
    -94, -91, -89, -86, -83, -80, -77, -74, -71, -68, -65, -62, -59, -56, -53, -50,
    -47, -44, -41, -38, -35, -32, -29, -26, -23, -20, -17, -14, -11,  -8,  -5,  -2
};

int cosT[] = {
    250, 250, 250, 250, 250, 249, 249, 249, 249, 248, 248, 248, 247, 247, 246, 245,
    245, 244, 244, 243, 242, 241, 241, 240, 239, 238, 237, 236, 235, 234, 233, 232,
    230, 229, 228, 227, 225, 224, 223, 221, 220, 218, 217, 215, 214, 212, 210, 209,
    207, 205, 204, 202, 200, 198, 196, 194, 192, 190, 188, 186, 184, 182, 180, 178,
    176, 173, 171, 169, 167, 164, 162, 160, 157, 155, 153, 150, 148, 145, 143, 140,
    138, 135, 132, 130, 127, 125, 122, 119, 116, 114, 111, 108, 105, 103, 100,  97,
    94,  91,  89,  86,  83,  80,  77,  74,  71,  68,  65,  62,  59,  56,  53,  50,
    47,  44,  41,  38,  35,  32,  29,  26,  23,  20,  17,  14,  11,   8,   5,   2,
    -2,  -5,  -8, -11, -14, -17, -20, -23, -26, -29, -32, -35, -38, -41, -44, -47,
    -50, -53, -56, -59, -62, -65, -68, -71, -74, -77, -80, -83, -86, -89, -91, -94,
    -97,-100,-103,-105,-108,-111,-114,-116,-119,-122,-125,-127,-130,-132,-135,-138,
    -140,-143,-145,-148,-150,-153,-155,-157,-160,-162,-164,-167,-169,-171,-173,-176,
    -178,-180,-182,-184,-186,-188,-190,-192,-194,-196,-198,-200,-202,-204,-205,-207,
    -209,-210,-212,-214,-215,-217,-218,-220,-221,-223,-224,-225,-227,-228,-229,-230,
    -232,-233,-234,-235,-236,-237,-238,-239,-240,-241,-241,-242,-243,-244,-244,-245,
    -245,-246,-247,-247,-248,-248,-248,-249,-249,-249,-249,-250,-250,-250,-250,-250,
    -250,-250,-250,-250,-250,-249,-249,-249,-249,-248,-248,-248,-247,-247,-246,-245,
    -245,-244,-244,-243,-242,-241,-241,-240,-239,-238,-237,-236,-235,-234,-233,-232,
    -230,-229,-228,-227,-225,-224,-223,-221,-220,-218,-217,-215,-214,-212,-210,-209,
    -207,-205,-204,-202,-200,-198,-196,-194,-192,-190,-188,-186,-184,-182,-180,-178,
    -176,-173,-171,-169,-167,-164,-162,-160,-157,-155,-153,-150,-148,-145,-143,-140,
    -138,-135,-132,-130,-127,-125,-122,-119,-116,-114,-111,-108,-105,-103,-100, -97,
    -94, -91, -89, -86, -83, -80, -77, -74, -71, -68, -65, -62, -59, -56, -53, -50,
    -47, -44, -41, -38, -35, -32, -29, -26, -23, -20, -17, -14, -11,  -8,  -5,  -2,
    2,   5,   8,  11,  14,  17,  20,  23,  26,  29,  32,  35,  38,  41,  44,  47,
    50,  53,  56,  59,  62,  65,  68,  71,  74,  77,  80,  83,  86,  89,  91,  94,
    97, 100, 103, 105, 108, 111, 114, 116, 119, 122, 125, 127, 130, 132, 135, 138,
    140, 143, 145, 148, 150, 153, 155, 157, 160, 162, 164, 167, 169, 171, 173, 176,
    178, 180, 182, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204, 205, 207,
    209, 210, 212, 214, 215, 217, 218, 220, 221, 223, 224, 225, 227, 228, 229, 230,
    232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 241, 242, 243, 244, 244, 245,
    245, 246, 247, 247, 248, 248, 248, 249, 249, 249, 249, 250, 250, 250, 250, 250
};

typedef struct
{
    int wn;
    int itow;
    int toi;
    double fsec;
} afstime_t;

typedef struct
{
    int week;
    double sec;
} gpstime_t;

typedef struct
{
    int y;
    int m;
    int d;
    int hh;
    int mm;
    double sec;
} datetime_t;

typedef struct
{
    int vflg;

    gpstime_t toe;
    gpstime_t toc;

    double ecc;   // Eccentricity
    double sqrta; // SQRT(A) 
    double m0;    // Mean Anom
    double omg0;  // Right Ascen at Week
    double inc0;  // Orbital Inclination
    double aop;   // Argument of Perigee

    double af0;
    double af1;

    // Working variables
    double n; // Mean motion
    double A; // Semi-major axis
    double sq1e2; // sqrt(1-e^2)

} ephem_t;

typedef struct
{
    int code[2046];
    double f_code;
    double code_phase;
    int C; // spreading code, signal level

    int ibit; // data bit counter
    int iframe; // data frame counter
    int D; // data signal level
    uint8_t data[2][6000];

} Ich_t;

typedef struct
{
    int code[10230];
    double f_code;
    double code_phase;
    int C; // spreading code, signal level
    int S; // seconday code
    int T; // tertiary code

    int ibit; // seconday code bit counter
    int ichip; // tertiary code chip counter

} Qch_t;

typedef struct
{
    int prn;
    Ich_t I;
    Qch_t Q;
    gpstime_t g0;
    double f_carr;
    double carr_phase;
    int* iq_buff;
    double azel[2];
} channel_t;

typedef struct
{
    gpstime_t g;
    double range;
    double rate;
} range_t;

int hex2bin(char hex, int* bin)
{
	int val;

	if (hex >= '0' && hex <= '9') {
		val = hex - '0';
	}
	else if (hex >= 'A' && hex <= 'F') {
		val = hex - 'A' + 10;
	}
	else if (hex >= 'a' && hex <= 'f') {
		val = hex - 'a' + 10;
	}
	else { // Invalid hex character
		return 0;
	}

	for (int i = 3; i >= 0; i--) {
		bin[i] = val % 2;
		val /= 2;
	}

	return 1;
}

int readTertiary(char* fname)
{
	FILE* fp;
	char str[MAX_CHAR];
	int nsv;

	if (NULL == (fp = fopen(fname, "rt"))) {
		return 0;
	}

	// Ignore the first line
	fgets(str, MAX_CHAR, fp);

	for (nsv = 0; nsv < 210; nsv++) {

		if (NULL == fgets(str, MAX_CHAR, fp))
			break;

		for (int i = 0; i < 375; i++) {
			hex2bin(str[i + 1], tcode[nsv] + i * 4); // Skip the first charactor
		}
	}

	fclose(fp);

	return nsv;
}

void icodegen(int* code, int prn)
{
    // Logic Level | Signal Level
    //      0      |      +1
    //      1      |      -1

    int delay[] = {
       1845, 1071,  170, 2035, 1214, 1292, 1284, 1894, 1537,  735, //   1 -  10
        561, 1789, 1453,  196, 1040,  326, 1787,  982, 1030, 1380, //  11 -  20
       1932, 1188,  390,  714,  303, 1001,  707, 1984,  139,  182, //  21 -  30
       1891, 1247, 1434, 2000, 1843,  865,  616,  514,  449, 1173, //  31 -  40
         24, 1383, 1940, 1594, 1765,  752,  145, 1615, 1666, 1372, //  41 -  50
       1634, 1068, 1181,  879, 1153, 1621,  927, 1848,  402,  413, //  51 -  60
       1090,  657,  609, 1547,  370,  271, 1353,  635,  299,  697, //  61 -  70
        152,  678, 1329,   15, 1974, 1884, 1868,  277,  302,    9, //  71 -  80
        603, 1583,  848, 1234, 1568,  510, 1303, 1921,  823, 1187, //  81 -  90
       1299,  824,  672, 2034, 1388,   13,  223, 1840, 1161, 1132, //  91 - 100
        365,    2,  924, 1373,  959,  220, 1542,  188,  264,  453, // 101 - 110
         68,  715,   75, 1095,  938, 1316,  394, 1156,  166,  969, // 111 - 120
        269,  179,  957,  400,  625, 1513, 1796,  100, 1660, 1454, // 121 - 130
       1613, 1064,  844,  518,  320,  661, 2031,  694, 1143, 1167, // 131 - 140
       1885,  833, 1601,  903,  399, 1896,  899,  133,  556,  331, // 141 - 150
        198,  212, 1024, 1070, 1972, 1573,  884, 1177, 1691,  533, // 151 - 160
        480,  751,  447,  734,  973,  857, 1767, 1548, 1876,  614, // 161 - 170
       1017, 1978,  275, 1141, 1252, 1952, 1714, 1067,  557,  522, // 171 - 180
       1159,  545, 1580,  610,  935, 1134,  780,  691, 1038, 1418, // 181 - 190
        295,  916, 1654,  624,  706, 1033, 1633,  790, 1451, 1300, // 191 - 200
        459,  106,  861, 1541,  114, 1381, 1945, 1069,  242,  356  // 201 - 210
    };

    static int g1[2047], g2[2047];
    int r1[11] = { 0 }, r2[11] = { 0 }, c1, c2;
    int i, j;

    for (i = 0; i < 11; i++)
        r1[i] = r2[i] = -1;

    for (i = 0; i < 2047; i++) {
        g1[i] = r1[10];
        g2[i] = r2[10];
        c1 = r1[1] * r1[10];
        c2 = r2[1] * r2[4] * r2[7] * r2[10];

        for (j = 10; j > 0; j--) {
            r1[j] = r1[j - 1];
            r2[j] = r2[j - 1];
        }
        r1[0] = c1;
        r2[0] = c2;
    }

    for (i = 0, j = 2047 - delay[prn - 1]; i < 2046; i++, j++) {
        //code[i] = (1 - g1[i] * g2[j % 2047]) / 2; // Logic level
        code[i] = g1[i] * g2[j % 2047]; // Signal level
    }

    return;
}

static char legendre[10223] = { 0 };

void gen_legendre_sequence()
{
    int i;
    for (i = 0; i < 10223; i++) {
        legendre[i] = 1;
    }

    for (i = 0; i < 10224; i++) {
        legendre[(i * i) % 10223] = -1;
    }
    legendre[0] = 1;

    return;
}

void qcodegen(int* code, int prn) // L1CP code (IS-GPS-800)
{
    // Kudos to Taro Suzuki: https://github.com/taroz/GNSS-SDRLIB/blob/master/src/sdrcode.c

    static const short weil[] = { /* Weil Index */
       5111, 5109, 5108, 5106, 5103, 5101, 5100, 5098, 5095, 5094, /*   1- 10 */
       5093, 5091, 5090, 5081, 5080, 5069, 5068, 5054, 5044, 5027, /*  11- 20 */
       5026, 5014, 5004, 4980, 4915, 4909, 4893, 4885, 4832, 4824, /*  21- 30 */
       4591, 3706, 5092, 4986, 4965, 4920, 4917, 4858, 4847, 4790, /*  31- 40 */
       4770, 4318, 4126, 3961, 3790, 4911, 4881, 4827, 4795, 4789, /*  41- 50 */
       4725, 4675, 4539, 4535, 4458, 4197, 4096, 3484, 3481, 3393, /*  51- 60 */
       3175, 2360, 1852, 5065, 5063, 5055, 5012, 4981, 4952, 4934, /*  61- 70 */
       4932, 4786, 4762, 4640, 4601, 4563, 4388, 3820, 3687, 5052, /*  71- 80 */
       5051, 5047, 5039, 5015, 5005, 4984, 4975, 4974, 4972, 4962, /*  81- 90 */
       4913, 4907, 4903, 4833, 4778, 4721, 4661, 4660, 4655, 4623, /*  91-100 */
       4590, 4548, 4461, 4442, 4347, 4259, 4256, 4166, 4155, 4109, /* 101-110 */
       4100, 4023, 3998, 3979, 3903, 3568, 5088, 5050, 5020, 4990, /* 111-120 */
       4982, 4966, 4949, 4947, 4937, 4935, 4906, 4901, 4872, 4865, /* 121-130 */
       4863, 4818, 4785, 4781, 4776, 4775, 4754, 4696, 4690, 4658, /* 131-140 */
       4607, 4599, 4596, 4530, 4524, 4451, 4441, 4396, 4340, 4335, /* 141-150 */
       4296, 4267, 4168, 4149, 4097, 4061, 3989, 3966, 3789, 3775, /* 151-160 */
       3622, 3523, 3515, 3492, 3345, 3235, 3169, 3157, 3082, 3072, /* 161-170 */
       3032, 3030, 4582, 4595, 4068, 4871, 4514, 4439, 4122, 4948, /* 171-180 */
       4774, 3923, 3411, 4745, 4195, 4897, 3047, 4185, 4354, 5077, /* 181-190 */
       4042, 2111, 4311, 5024, 4352, 4678, 5034, 5085, 3646, 4868, /* 191-200 */
       3668, 4211, 2883, 2850, 2815, 2542, 2492, 2376, 2036, 1920  /* 201-210 */
    };

    static const short insert[] = { /* Insertion Index */
        412,  161,    1,  303,  207, 4971, 4496,    5, 4557,  485, /*   1- 10 */
        253, 4676,    1,   66, 4485,  282,  193, 5211,  729, 4848, /*  11- 20 */
        982, 5955, 9805,  670,  464,   29,  429,  394,  616, 9457, /*  21- 30 */
       4429, 4771,  365, 9705, 9489, 4193, 9947,  824,  864,  347, /*  31- 40 */
        677, 6544, 6312, 9804,  278, 9461,  444, 4839, 4144, 9875, /*  41- 50 */
        197, 1156, 4674,10035, 4504,    5, 9937,  430,    5,  355, /*  51- 60 */
        909, 1622, 6284, 9429,   77,  932, 5973,  377,10000,  951, /*  61- 70 */
       6212,  686, 9352, 5999, 9912, 9620,  635, 4951, 5453, 4658, /*  71- 80 */
       4800,   59,  318,  571,  565, 9947, 4654,  148, 3929,  293, /*  81- 90 */
        178,10142, 9683,  137,  565,   35, 5949,    2, 5982,  825, /*  91-100 */
       9614, 9790, 5613,  764,  660, 4870, 4950, 4881, 1151, 9977, /* 101-110 */
       5122,10074, 4832,   77, 4698, 1002, 5549, 9606, 9228,  604, /* 111-120 */
       4678, 4854, 4122, 9471, 5026,  272, 1027,  317,  691,  509, /* 121-130 */
       9708, 5033, 9938, 4314,10140, 4790, 9823, 6093,  469, 1215, /* 131-140 */
        799,  756, 9994, 4843, 5271, 9661, 6255, 5203,  203,10070, /* 141-150 */
         30,  103, 5692,   32, 9826,   76,   59, 6831,  958, 1471, /* 151-160 */
      10070,  553, 5487,   55,  208,  645, 5268, 1873,  427,  367, /* 161-170 */
       1404, 5652,    5,  368,  451, 9595, 1030, 1324,  692, 9819, /* 171-180 */
       4520, 9911,  278,  642, 6330, 5508, 1872, 5445,10131,  422, /* 181-190 */
       4918,  787, 9864, 9753, 9859,  328,    1, 4733,  164,  135, /* 191-200 */
        174,  132,  538,  176,  198,  595,  574,  321,  596,  491  /* 201-210 */
    };

    char weilcode[10223] = { 0 };
    int i, j, ind;
    int w = weil[prn - 1], p = insert[prn - 1] - 1;
    static const char insertbit[7] = {-1, 1, 1, -1, 1, -1, -1};

    /* Generate Legendre Sequence */
    if (!legendre[0]) gen_legendre_sequence();

    for (i = 0; i < 10223; i++) {
        ind = (i + w) % 10223;
        weilcode[i] = -legendre[i] * legendre[ind];
    }

    /* Insert bits */
    for (i = 0; i < p; i++) 
        code[i] = weilcode[i];
    
    for (j = 0; j < 7; j++) 
        code[i++] = insertbit[j];
    
    for (i = p + 7; i < 10230; i++)
        code[i] = weilcode[i - 7];

    for (i = 0; i < 10230; i++) {
        //code[i] = (1 + code[i]) / 2; // Logic level
        code[i] = -1 * code[i]; // Signal level
    }

    return;
}

void timeadd(gpstime_t *g, double dt)
{
    g->sec += dt;

    if (g->sec < 0.0) {
        g->sec += SECONDS_IN_WEEK;
        g->week -= 1;
    }
    else if (g->sec >= SECONDS_IN_WEEK) {
        g->sec -= SECONDS_IN_WEEK;
        g->week += 1;
    }

    return;
}

void gpst2afst(const gpstime_t *gpst, afstime_t *afst)
{
    afst->wn = gpst->week;
    afst->fsec = gpst->sec;

    if (afst->fsec < 0.0) {
        afst->fsec += SECONDS_IN_WEEK;
        afst->wn -= 1;
    }
    else if (afst->fsec >= SECONDS_IN_WEEK) {
        afst->fsec -= SECONDS_IN_WEEK;
        afst->wn += 1;
    }
    
    afst->itow = (int)(afst->fsec / 1200.0);
    afst->fsec -= (double)(afst->itow) * 1200.0;
    
    afst->toi = (int)(afst->fsec / 12.0);
    afst->fsec -= (double)(afst->toi) * 12.0;

    return;
}

void gpst2date(const gpstime_t* g, datetime_t* t)
{
    // Convert Julian day number to calendar date
    int c = (int)(7 * g->week + floor(g->sec / 86400.0) + 2444245.0) + 1537;
    int d = (int)((c - 122.1) / 365.25);
    int e = 365 * d + d / 4;
    int f = (int)((c - e) / 30.6001);

    t->d = c - e - (int)(30.6001 * f);
    t->m = f - 1 - 12 * (f / 14);
    t->y = d - 4715 - ((7 + t->m) / 10);

    t->hh = ((int)(g->sec / 3600.0)) % 24;
    t->mm = ((int)(g->sec / 60.0)) % 60;
    t->sec = g->sec - 60.0 * floor(g->sec / 60.0);

    return;
}

void llh2xyz(double* llh, double* xyz)
{
    double a, h, tmp;

    a = R_MOON;
    h = a + llh[2];

    tmp = h * cos(llh[0]);
    xyz[0] = tmp * cos(llh[1]);
    xyz[1] = tmp * sin(llh[1]);
    xyz[2] = h * sin(llh[0]);

    return;
}

void ltcmat(double* llh, double t[3][3])
{
    double slat, clat;
    double slon, clon;

    slat = sin(llh[0]);
    clat = cos(llh[0]);
    slon = sin(llh[1]);
    clon = cos(llh[1]);

    t[0][0] = -slat * clon;
    t[0][1] = -slat * slon;
    t[0][2] = clat;
    t[1][0] = -slon;
    t[1][1] = clon;
    t[1][2] = 0.0;
    t[2][0] = clat * clon;
    t[2][1] = clat * slon;
    t[2][2] = slat;

    return;
}

void xyz2neu(double* xyz, double t[3][3], double* neu)
{
    neu[0] = t[0][0] * xyz[0] + t[0][1] * xyz[1] + t[0][2] * xyz[2];
    neu[1] = t[1][0] * xyz[0] + t[1][1] * xyz[1] + t[1][2] * xyz[2];
    neu[2] = t[2][0] * xyz[0] + t[2][1] * xyz[1] + t[2][2] * xyz[2];

    return;
}

void neu2azel(double* azel, double* neu)
{
    double ne;

    azel[0] = atan2(neu[1], neu[0]);
    if (azel[0] < 0.0)
        azel[0] += (2.0 * PI);

    ne = sqrt(neu[0] * neu[0] + neu[1] * neu[1]);
    azel[1] = atan2(neu[2], ne);

    return;
}

void satpos(ephem_t eph, gpstime_t g, double* pos, double* vel, double* clk)
{
    double tk;
    double mk;
    double ek;
    double ekold;
    double OneMinusecosE;
    double cek, sek;
    double ekdot;
    double uk;
    double cuk, suk;
    double ukdot;
    double rk;
    double rkdot;
    double ik;
    double cik, sik;
    double xpk, ypk;
    double xpkdot, ypkdot;
    double ok;
    double cok, sok;

    tk = g.sec - eph.toe.sec;

    if (tk > SECONDS_IN_HALF_WEEK)
        tk -= SECONDS_IN_WEEK;
    else if (tk < -SECONDS_IN_HALF_WEEK)
        tk += SECONDS_IN_WEEK;

    // Mean anomaly
    mk = eph.m0 + eph.n * tk;

    // Eccentric anomaly
    ek = mk;
    ekold = ek + 1.0;

    OneMinusecosE = 1.0;

    while (fabs(ek - ekold) > 1.0E-14)
    {
        ekold = ek;
        OneMinusecosE = 1.0 - eph.ecc * cos(ekold);
        ek = ek + (mk - ekold + eph.ecc * sin(ekold)) / OneMinusecosE;
    }

    sek = sin(ek);
    cek = cos(ek);

    ekdot = eph.n / OneMinusecosE;

    // True anomaly + Argument of perigee
    uk = atan2(eph.sq1e2 * sek, cek - eph.ecc) + eph.aop;
    suk = sin(uk);
    cuk = cos(uk);
    ukdot = eph.sq1e2 * ekdot / OneMinusecosE;

    // Range and range rate
    rk = eph.A * OneMinusecosE;
    rkdot = eph.A * eph.ecc * sek * ekdot;

    xpk = rk * cuk;
    ypk = rk * suk;
    xpkdot = rkdot * cuk - ypk * ukdot;
    ypkdot = rkdot * suk + xpk * ukdot;

    // Inclination
    ik = eph.inc0;

    sik = sin(ik);
    cik = cos(ik);

    // RAAN
    ok = eph.omg0;
    sok = sin(ok);
    cok = cos(ok);

    // Moon-centered inertial coordinates
    pos[0] = xpk * cok - ypk * cik * sok;
    pos[1] = xpk * sok + ypk * cik * cok;
    pos[2] = ypk * sik;

    vel[0] = xpkdot * cok - ypkdot * cik * sok;
    vel[1] = xpkdot * sok + ypkdot * cik * cok;
    vel[2] = ypkdot * sik;

    // Satellite clock correction
    tk = g.sec - eph.toc.sec;

    if (tk > SECONDS_IN_HALF_WEEK)
        tk -= SECONDS_IN_WEEK;
    else if (tk < -SECONDS_IN_HALF_WEEK)
        tk += SECONDS_IN_WEEK;

    clk[0] = eph.af0 + tk * eph.af1;
    clk[1] = eph.af1;
}

int readAlmanac(ephem_t eph[], const char* fname)
{
    int nsat = 0;

    FILE* fp;
    int sv;
    char str[MAX_CHAR];

    if (NULL == (fp = fopen(fname, "rt")))
        return(-1);

    for (sv = 0; sv < MAX_SAT; sv++)
        eph[sv].vflg = 0;

    while (1)
    {
        if (NULL == fgets(str, MAX_CHAR, fp))
            break;

        if (strlen(str) < 25)
            continue; // Skip empty line

        if (str[0] == '*')
        {
            // ID
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            sv = atoi(str + 26) - 1;

            if (sv<0 || sv>MAX_SAT)
                break;

            // Health
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            // Eccentricity
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].ecc = atof(str + 26);

            // Time of Applicability(s)
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].toe.sec = atof(str + 26);

            // Orbital Inclination(rad)
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].inc0 = atof(str + 26);

            // Rate of Right Ascen(r/s)
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            // SQRT(A)  (m 1 / 2)
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].sqrta = atof(str + 26);

            // Right Ascen at Week(rad)
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].omg0 = atof(str + 26);

            // Argument of Perigee(rad)
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].aop = atof(str + 26);

            // Mean Anom(rad)
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].m0 = atof(str + 26);

            // Af0(s)
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].af0 = atof(str + 26);

            // Af1(s/s)
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].af1 = atof(str + 26);

            // Week
            if (NULL == fgets(str, MAX_CHAR, fp))
                break;

            eph[sv].toe.week = atoi(str + 26);
            // GPS week number rollover on April 6, 2019.
            eph[sv].toe.week += 2048;

            eph[sv].toc = eph[sv].toe;

            // Valid almanac
            eph[sv].vflg = 1;
            nsat++;

            // Update the working variables
            eph[sv].A = eph[sv].sqrta * eph[sv].sqrta;
            eph[sv].n = sqrt(GM_MOON / (eph[sv].A * eph[sv].A * eph[sv].A));
            eph[sv].sq1e2 = sqrt(1.0 - eph[sv].ecc * eph[sv].ecc);
        }
    }

    return (nsat);
}

void eph2sbf(const ephem_t eph, uint8_t *syms)
{
    uint32_t toe;
    uint32_t toc;
    uint32_t ecc;
	uint32_t sqrta;
	int32_t m0;
	int32_t omg0;
	int32_t inc0;
	int32_t aop;
    int32_t af0;
	int32_t af1;

	toc = (uint32_t)(eph.toc.sec/16.0);
    ecc = (uint32_t)(eph.ecc/POW2_M32);
	sqrta = (uint32_t)(eph.sqrta/POW2_M19);
    inc0 = (int32_t)(eph.inc0/POW2_M31/PI);
	omg0 = (int32_t)(eph.omg0/POW2_M31/PI);
	aop = (int32_t)(eph.aop/POW2_M31/PI);
    m0 = (int32_t)(eph.m0/POW2_M31/PI);

    toe = (uint32_t)(eph.toe.sec/16.0);
    af0 = (int32_t)(eph.af0 / POW2_M31);
	af1 = (int32_t)(eph.af1 / POW2_M43);
    
    sdr_unpack_data(toe, 16, syms);
    sdr_unpack_data(ecc, 32, syms + 16);
    sdr_unpack_data(sqrta, 32, syms + 48);
    sdr_unpack_data(inc0, 32, syms + 80);
    sdr_unpack_data(omg0, 32, syms + 112);
    sdr_unpack_data(aop, 32, syms + 144);
    sdr_unpack_data(m0, 32, syms + 176);
    sdr_unpack_data(toc, 16, syms + 208);
    sdr_unpack_data(af0, 22, syms + 224);
    sdr_unpack_data(af1, 16, syms + 246);

    return;
}


void subVect(double* y, double* x1, double* x2)
{
    y[0] = x1[0] - x2[0];
    y[1] = x1[1] - x2[1];
    y[2] = x1[2] - x2[2];

    return;
}

double normVect(double* x)
{
    return(sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]));
}

double dotProd(double* x1, double* x2)
{
    return(x1[0] * x2[0] + x1[1] * x2[1] + x1[2] * x2[2]);
}

void computeRange(range_t* rho, ephem_t eph, gpstime_t g, double xyz[])
{
    double pos[3], vel[3], clk[2];
    double los[3];
    double tau;
    double range, rate;

    // SV position at time of the pseudorange observation.
    satpos(eph, g, pos, vel, clk);

    // Receiver to satellite vector and light-time.
    subVect(los, pos, xyz);
    tau = normVect(los) / SPEED_OF_LIGHT;

    // Extrapolate the satellite position backwards to the transmission time.
    pos[0] -= vel[0] * tau;
    pos[1] -= vel[1] * tau;
    pos[2] -= vel[2] * tau;

    // New observer to satellite vector and satellite range.
    subVect(los, pos, xyz);
    range = normVect(los);

    // Pseudorange
    rho->range = range - SPEED_OF_LIGHT * clk[0];

    // Relative velocity of SV and receiver
    rate = dotProd(vel, los) / range;

    // Pseudorange rate
    rho->rate = rate;

    // Time of application
    rho->g = g;

    return;
}

void computeCodePhase(channel_t* chan, range_t rho0, range_t rho1, double dt)
{
    double ms;
    double rhorate;
    int ibit, iframe;
    int sv;
    gpstime_t gt;
    afstime_t afst;

    // Pseudorange rate
    rhorate = (rho1.range - rho0.range) / dt;

    // Carrier and code frequency
    chan->f_carr = -rhorate / LAMBDA;
    chan->I.f_code = I_CODE_FREQ + chan->f_carr * CARR_TO_I_CODE;
    chan->Q.f_code = Q_CODE_FREQ + chan->f_carr * CARR_TO_Q_CODE;

    // Signal transmission time
    gt.sec = rho0.g.sec;
    gt.week = rho0.g.week;
    timeadd(&gt, -rho0.range / SPEED_OF_LIGHT);
    gpst2afst(&gt, &afst);
    
    iframe = afst.toi; // 1 frame = 12 sec
    ms = afst.fsec * 1000.0; // Fractional milliseconds within a frame
    ibit = (int)(ms / 2.0); // 1 bit = 1 code = 2 ms
    ms -= ibit * 2.0; // Fractional milliseconds within a code

    // Spreading code
    chan->I.code_phase = ms / 2.0 * 2046.0; // 1 chip = 2 ms
    chan->I.C = chan->I.code[(int)chan->I.code_phase];

    chan->Q.code_phase = ms / 2.0 * 10230.0; // 1 chip = 2 ms
    chan->Q.C = chan->Q.code[(int)chan->Q.code_phase];

    // Navigation message
    chan->I.ibit = ibit;
    chan->I.iframe = iframe;

    // Logic Level | Signal Level
    //      0      |      +1
    //      1      |      -1

    //chan->I.D = -2 * chan->I.data[iframe % 2][ibit] + 1;
    chan->I.D = -2 * chan->I.data[0][ibit] + 1;

    // Seconday code
    sv = chan->prn - 1;
    chan->Q.ibit = ibit % 4;
    chan->Q.S = -2 * scode[sv % 4][chan->Q.ibit] + 1;

    // Tertiary code
    chan->Q.ichip = (int)(afst.fsec * 1000.0 / 8.0);
    chan->Q.T = -2 * tcode[sv][chan->Q.ichip] + 1;

    return;
}

void bitncpy(uint8_t *syms1, const uint8_t *syms2, int n)
{
    //for (int i = 0; i < n; i++) {
    //    syms1[i] = syms2[i];
    //}

    memcpy(syms1, syms2, n);   

    return;
}

void printUsage(void)
{
    fprintf(stderr, "Usage: afs_sim [-t tsec] [-s freq] [-e feph] [-b bits] [-l lat:lon:hgt] fout | -\n");

    exit(0);
}

int main(int argc, char** argv)
{
    FILE *fp;
    const char *fout = "", *feph="";
    double tsec = 300.0;
    double freq = 12.0e6;
    int nbits = 16;
    double llh[3] = { 0.0, 0.0, -(R_MOON + 1.0e3) };
    double xyz[3];
    int ret;

    int sv;
    int neph;
    ephem_t eph[MAX_SAT];
    gpstime_t g0;
    datetime_t t0;

    int i;
    static channel_t chan[MAX_SAT]; // Avoid warning C6262
    double tmat[3][3];
    int nsat;
    double pos[3], vel[3], clk[2];
    double los[3];
    double neu[3];
    double azel[2];
    double elvmask = 0.0 / R2D;

    int iq_buff_size;
    double delt;
    void* iq_buff = NULL;

    gpstime_t grx;
    clock_t tstart, tend;
    range_t rho, rho0[MAX_SAT];

    int isim, nsim;
    int isamp;
    int ph;
    int ip, qp;
    int I, Q;

    int sample = 0;
    int thresh = 1; // 2-bit ADC threshold
    int twobitADC = 0;

    double path_loss;
    int gain;
    int noise_scale;

    int inv_q = 0; // Inverse Q sign flag

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-t") && i + 1 < argc) {
            tsec = atof(argv[++i]);
        }
        else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
            freq = atof(argv[++i]);
        }
        else if (!strcmp(argv[i], "-e") && i + 1 < argc) {
            feph = argv[++i];
        }
        else if (!strcmp(argv[i], "-b") && i + 1 < argc) {
            nbits = atoi(argv[++i]);
            if (nbits != 2) {
                fprintf(stderr, "ERROR: Invalid ADC bits for -b option.\n");
                exit(-1);
            }
        }
        else if (!strcmp(argv[i], "-l") && i + 1 < argc) {
            ret = sscanf(argv[++i], "%lf:%lf:%lf", &llh[0], &llh[1], &llh[2]);
            if (ret != 3) {
                fprintf(stderr, "ERROR: Wrong format for -l option.\n");
                exit(-1);
            }
            llh[0] /= R2D;
            llh[1] /= R2D;
        }
        else if (!strcmp(argv[i], "-InQ")) {
            inv_q = 1; // Set inverse Q sign flag for MAX2771
        }
        else if (argv[i][0] == '-' && argv[i][1] != '\0') {
        //else if (argv[i][0] == '-') {
            printUsage();
        }
        else {
            fout = argv[i];
        }
    }

    if (!*fout) {
        fprintf(stderr, "ERROR: Specify output file.\n");
        exit(-1);
    }
    
    if (!readTertiary("008_Weil1500hex210prns.txt")) {
		fprintf(stderr, "ERROR: Failed to read tertiary code file.\n");
		exit(-1);
	}
    
    // Set user location

    if (llh[2] < -(R_MOON)) {
        // Default user location; Shackleton crater
        llh[0] = -89.66 / R2D;
        llh[1] = 129.20 / R2D;
        llh[2] = 100.0;
    }

    llh2xyz(llh, xyz);

    fprintf(stderr, "xyz = %11.1f, %11.1f, %11.1f\n", xyz[0], xyz[1], xyz[2]);
    fprintf(stderr, "llh = %11.6f, %11.6f, %11.1f\n", llh[0] * R2D, llh[1] * R2D, llh[2]);

    // Read ephemeris

    if (!*feph) {
        feph = "default_almanac.txt";
    }

    neph = readAlmanac(eph, feph);

    if (neph == -1) {
        fprintf(stderr, "ERROR: Failed to open ephemeris file.\n");
        exit(1);
    }
    
    // Set simulation start time

    g0.week = -1;

    if (neph > 0) {
        for (sv = 0; sv < MAX_SAT; sv++) {
            if (g0.week < 0 && eph[sv].vflg == 1) {
                g0 = eph[sv].toe;
                break;
            }
        }
    }
    else
    {
        fprintf(stderr, "ERROR: No valid ephemeris has been found.\n");
        exit(1);
    }

    gpst2date(&g0, &t0);
    fprintf(stderr, "Start time = %4d/%02d/%02d,%02d:%02d:%02.0f (%d:%.0f)\n", t0.y, t0.m, t0.d, t0.hh, t0.mm, t0.sec, g0.week, g0.sec);

    afstime_t afst;
    gpst2afst(&g0, &afst);

    fprintf(stderr, "AFS time: WN = %d, ITOW = %d, TOI = %d, fsec = %.1f\n", afst.wn, afst.itow, afst.toi, afst.fsec);

    // Check visible satellites

    for (i = 0; i < MAX_SAT; i++)
        chan[i].prn = 0; // Idle channel

    ltcmat(llh, tmat);

    nsat = 0;

    for (sv = 0; sv < MAX_SAT; sv++) {
        if (eph[sv].vflg == 1) {
            satpos(eph[sv], g0, pos, vel, clk);
            subVect(los, pos, xyz);
            xyz2neu(los, tmat, neu);
            neu2azel(azel, neu);

            if (azel[1] > elvmask) {
                chan[nsat].prn = sv + 1;

                chan[nsat].azel[0] = azel[0];
                chan[nsat].azel[1] = azel[1];

                nsat++; // Number of visible satellites
            }
        }
    }

    fprintf(stderr, "Number of channels = %d\n", nsat);

    // Baseband signal buffer and output file

    freq = floor(freq / 10.0);
    iq_buff_size = (int)freq; // samples per 0.1sec
    freq *= 10.0;

    delt = 1.0 / freq;

    // Allocate I/Q buffer
    if (nbits == 2) {
        iq_buff = (signed char*)calloc(2 * iq_buff_size, 1);
    }
    else {
        iq_buff = (short*)calloc(2 * iq_buff_size, 2);
    }

    if (iq_buff == NULL) {
        fprintf(stderr, "ERROR: Faild to allocate global IQ buffer.\n");
        exit(1);
    }
    
    if (fout[0] == '-') {
        fp = stdout;
#ifdef _WIN32
        int retval = _setmode(_fileno(stdout), _O_BINARY); // Set stdout to binary mode
#endif
    }
    else if (NULL == (fp = fopen(fout, "wb"))) { // Open output file
        fprintf(stderr, "ERROR: Failed to open output file.\n");
        exit(1);
    }

    // Initialize signals

    grx = g0; // Initial reception time

    for (i = 0; i < nsat; i++) {

        // Code generation
        icodegen(chan[i].I.code, chan[i].prn);
        qcodegen(chan[i].Q.code, chan[i].prn);

        // Allocate I/Q buffer
        chan[i].iq_buff = (int*)calloc(2 * iq_buff_size, sizeof(int));

        if (chan[i].iq_buff == NULL)
        {
            fprintf(stderr, "ERROR: Faild to allocate channel IQ buffer.\n");
            exit(1);
        }
    }

    // Generate frames and data bits

    for (i = 0; i < nsat; i++) {

        chan[i].g0 = g0; // Data bit reference time
    }

    // Initialize carrier phase
    for (i = 0; i < nsat; i++) {
        chan[i].carr_phase = 0.0;
    }

    // Initial pseudorange
    fprintf(stderr, "SV    AZ    EL     RANGE     DOPP\n");
    for (i = 0; i < nsat; i++) {
        sv = chan[i].prn - 1;
        computeRange(&rho0[sv], eph[sv], grx, xyz);

        fprintf(stderr, "%02d %6.1f %5.1f %10.1f %+8.1f\n", chan[i].prn, 
            chan[i].azel[0] * R2D, chan[i].azel[1] * R2D, rho0[sv].range, -rho0[sv].rate/LAMBDA);
    }

    // Insert synchronization pattern 

    const uint8_t sync[9] ={0xCC, 0x63, 0xF7, 0x45, 0x36, 0xF4, 0x9E, 0x04, 0xA0}; // left justified

    for (i = 0; i < nsat; i++) {
        sdr_unpack_bits(sync, 68, chan[i].I.data[0]); // synchronization pattern
    }

    // Insert subframe 1

    uint8_t AFS_SB1[52];
    
    // The TOI corresponds to the node time epoch at the leading edge of the "next" 12-second frame.
    generate_BCH_AFS_SF1(AFS_SB1, 0, afst.toi + 1); // subframe 1 for frame ID 0

    for (i = 0; i < nsat; i++) {
        bitncpy(chan[i].I.data[0] + 68, AFS_SB1, 52);
    }

    // Insert subframe 2-4

    uint8_t AFS_SB234[5880];
    uint8_t syms[5880];

    for (i = 0; i < 846; i++) {
        syms[i] = i%2; // test pattern 0,1,0,1,...
    }
    append_CRC24(syms, 870);
    encode_LDPC_AFS_SF3(syms, AFS_SB234 + 2400); // Subframe 3

    encode_LDPC_AFS_SF3(syms, AFS_SB234 + 4140); // Subframe 4

    for (i = 0; i < nsat; i++) {

        for (int j = 0; j < 1176; j++) {
            syms[j] = j%2; // test pattern 0,1,0,1,...
        }
        uint32_t data = ((uint32_t)afst.wn<<9) | (afst.itow & 0x1ff);
        sdr_unpack_data(data, 22, syms); // Insert WN and ITOW
        
        // Insert ephemeris
        sv = chan[i].prn - 1;
        eph2sbf(eph[sv], syms + 22);

        append_CRC24(syms, 1200);
        encode_LDPC_AFS_SF2(syms, AFS_SB234); // Subframe 2
 
        interleave_AFS_SF234(AFS_SB234, syms); // Interleaving

        bitncpy(chan[i].I.data[0] + 120, syms, 5880);
    }

    // Generate baseband signals

    tstart = clock();

    fprintf(stderr, "Generating baseband signals...\n");

    fprintf(stderr, "\rTime = %4.1f", grx.sec - g0.sec);
    fflush(stderr);

    // Update receiver time
    grx.sec += 0.1;

    nsim = (int)((tsec - 0.1) * 10.0); // 10Hz update rate

    // 2-bit ADC threshold
    thresh = (int)(1250.0 * sqrt((double)nsat)); // 1-sigma value of thermal noise

    // Noise amplitude scale
    noise_scale = (int)(1250.0 / 1449.0 * sqrt((double)nsat) * GAIN_SCALE); 

    // Seed the gaussian noise generator
    srandn(time(NULL));

    for (isim = 0; isim < nsim; isim++) {
        #pragma omp parallel for private(sv, rho, path_loss, gain, I, Q, ip, qp, isamp)
        for (i = 0; i < nsat; i++) {

            // Refresh code phase and data bit counters
            sv = chan[i].prn - 1;

            // Current pseudorange
            computeRange(&rho, eph[sv], grx, xyz);

            // Update code phase and data bit counters
            computeCodePhase(&chan[i], rho0[sv], rho, 0.1);

            // Save current pseudorange
            rho0[sv] = rho;

            // Path loss
            path_loss = 5200000.0 / rho.range;

            // Gain
            gain = (int)(path_loss * (double)GAIN_SCALE);

            for (isamp = 0; isamp < iq_buff_size; isamp++) {

                ph = (int)floor(chan[i].carr_phase * 512.0);

                I = chan[i].I.C * chan[i].I.D;
                Q = chan[i].Q.C * chan[i].Q.S * chan[i].Q.T;

                ip = gain * (I * cosT[ph] - Q * sinT[ph]);
                qp = gain * (I * sinT[ph] + Q * cosT[ph]);

                if (inv_q == 1 && nbits == 2)
                    qp *= -1; // Inverse Q sign to emulate MAX2771 outputs

                // Store I/Q samples into buffer
                chan[i].iq_buff[isamp * 2] = ip;
                chan[i].iq_buff[isamp * 2 + 1] = qp;

                // Update code phase
                chan[i].I.code_phase += chan[i].I.f_code * delt;
                chan[i].Q.code_phase += chan[i].Q.f_code * delt;

                if (chan[i].I.code_phase >= 2046.0) {
                    chan[i].I.code_phase -= 2046.0;

                    chan[i].I.ibit++;
                    if (chan[i].I.ibit >= 6000) {
                        chan[i].I.ibit -= 6000;
                        chan[i].I.iframe++;
                    }

                    // Update navigation message data bit
                    chan[i].I.D = -2 * chan[i].I.data[0][chan[i].I.ibit] + 1;
                }

                if (chan[i].Q.code_phase >= 10230.0) {
                    chan[i].Q.code_phase -= 10230.0;

                    chan[i].Q.ibit++;
                    if (chan[i].Q.ibit >= 4) { // Secondary code period = 8 ms = 4 codes
                        chan[i].Q.ibit -= 4;
                        chan[i].Q.ichip++; // Tertiary code chip = Secondary code period

                        // Update tertiary code
                        chan[i].Q.T = -2 * tcode[sv][chan[i].Q.ichip % 1500] + 1;
                    }

                    // Update secondary code
                    chan[i].Q.S = -2 * scode[sv % 4][chan[i].Q.ibit] + 1;
                }

                // Set currnt code chip
                chan[i].I.C = chan[i].I.code[(int)chan[i].I.code_phase];
                chan[i].Q.C = chan[i].Q.code[(int)chan[i].Q.code_phase];

                // Update carrier phase
                chan[i].carr_phase += chan[i].f_carr * delt;

                if (chan[i].carr_phase >= 1.0)
                    chan[i].carr_phase -= 1.0;
                else if (chan[i].carr_phase < 0.0)
                    chan[i].carr_phase += 1.0;
            }
        }

        if (nbits == 2) {
            #pragma omp parallel for private(i, sample, twobitADC)
            for (isamp = 0; isamp < 2 * iq_buff_size; isamp++)
            {
                sample = 0;
                for (i = 0; i < nsat; i++)
                    sample += chan[i].iq_buff[isamp];

                sample += randn() * noise_scale; // Add thermal noise
                sample /= GAIN_SCALE;

                if (sample >= 0) {
                    twobitADC = (sample > thresh) ? +3 : +1;
                }
                else {
                    twobitADC = (sample < -thresh) ? -3 : -1;
                }
                ((signed char*)iq_buff)[isamp] = (signed char)twobitADC;
            }
            fwrite(iq_buff, 1, 2 * iq_buff_size, fp);
        }
        else {
            #pragma omp parallel for private(i, sample)
            for (isamp = 0; isamp < 2 * iq_buff_size; isamp++)
            {
                sample = 0;
                for (i = 0; i < nsat; i++)
                    sample += chan[i].iq_buff[isamp];

                sample /= GAIN_SCALE;

                ((short*)iq_buff)[isamp] = (short)sample;
            }
            fwrite(iq_buff, 2, 2 * iq_buff_size, fp);
        }

        // Update TOI in SB1 at every 12 seconds
        int igrx = (int)(grx.sec*10.0+0.5);
        
        if (igrx%120==110)
		{
            afst.toi++;

            generate_BCH_AFS_SF1(AFS_SB1, 0, afst.toi + 1); // subframe 1 for frame ID 0

            for (i = 0; i < nsat; i++) {
                bitncpy(chan[i].I.data[0] + 68, AFS_SB1, 52);
            }

            //printf("\nUpdate Data Frames: TOI = %d\n", afst.toi);
        }

        // Update receiver time
        grx.sec += 0.1;

        // Update time counter
        fprintf(stderr, "\rTime = %4.1f", grx.sec - g0.sec);
        fflush(stderr);
    }

    tend = clock();

    fprintf(stderr, "\nDone!\n");

    // Free I/Q buffers
    free(iq_buff);
    for (i = 0; i < nsat; i++) {
        free(chan[i].iq_buff);
    }
        
    // Close output file
    if (fp != stdout)
        fclose(fp);

    // Process time
    fprintf(stderr, "Process time = %.1f[sec]\n", (double)(tend - tstart) / CLOCKS_PER_SEC);

	return 0;
}
