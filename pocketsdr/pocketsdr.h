#ifndef POCKET_SDR_H
#define POCKET_SDR_H

#include <stdint.h>

// function prototypes -------------------------------------------------------

// sdr_cmn.c
void *sdr_malloc(size_t size);
void sdr_free(void *p);

// sdr_func.c
void sdr_pack_bits(const uint8_t *data, int nbit, int nz, uint8_t *buff);
void sdr_unpack_bits(const uint8_t *data, int nbit, uint8_t *buff);
void sdr_unpack_data(uint32_t data, int nbit, uint8_t *buff);
uint8_t sdr_xor_bits(uint32_t X);

// sdr_code.c
int32_t rev_reg(int32_t R, int N);
int8_t *LFSR(int N, int32_t R, int32_t tap, int n);

#endif // POCKET_SDR_H 