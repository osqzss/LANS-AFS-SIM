#include "afs_rand.h"

#define N_SAMPLES 6         // Central Limit Theorem depth
#define DEFAULT_NUM_SAMPLES 100

// XORSHIFT32 PRNG state
static uint32_t rng_state = 1;

// Fast XORSHIFT32 PRNG
static inline uint32_t xorshift32()
{
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

// Generate approximate Gaussian noise in int16_t range
int16_t randn()
{
    int sum = 0;
    for (int i = 0; i < N_SAMPLES; ++i) {
        sum += (int)(xorshift32() & 0xFFFF);  // 0 to 65535
    }

    int midpoint = (65535 * N_SAMPLES) / 2;
    int scaled = sum - midpoint;

    //scaled >>= 2;  // Scale variance for target standard deviation value ~ 11,600
    scaled >>= 5; // for taget std ~ 1250

    if (scaled > 32767) return 32767;
    if (scaled < -32768) return -32768;

    return (int16_t)scaled;
}

void srandn(uint32_t seed) 
{
    if (seed == 0) seed = 1;
    rng_state = seed;
}
