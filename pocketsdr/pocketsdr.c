#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//===== sdr_cmn.c ==============================================================

//------------------------------------------------------------------------------
//  Allocate memory. If no memory allocated, it exits the AP immediately with
//  an error message.
//  
//  args:
//      size     (I)  memory size (bytes)
//
//  return:
//      memory pointer allocated.
//
void *sdr_malloc(size_t size)
{
    void *p;
    
    if (!(p = calloc(size, 1))) {
        fprintf(stderr, "memory allocation error size=%d\n", (int)size);
        exit(-1);
    }
    return p;
}

//------------------------------------------------------------------------------
//  Free memory allocated by sdr_malloc()
//  
//  args:
//      p        (I)  memory pointer allocated.
//
//  return:
//      none
//
void sdr_free(void *p)
{
    free(p);
}

//===== sdr_func.c =============================================================

// pack bit array to uint8_t array ---------------------------------------------
void sdr_pack_bits(const uint8_t *data, int nbit, int nz, uint8_t *buff)
{
    memset(buff, 0, (nz + nbit + 7) / 8);
    for (int i = nz; i < nz + nbit; i++) {
        buff[i / 8] |= data[i-nz] << (7 - i % 8);
    }
}

// unpack uint8_t array to bit array -------------------------------------------
void sdr_unpack_bits(const uint8_t *data, int nbit, uint8_t *buff)
{
    //for (int i = 0; i < nbit * 8; i++) {
    for (int i = 0; i < nbit; i++) {
        buff[i] = (data[i / 8] >> (7 - i % 8)) & 1;
    }
}

// unpack data to bits ---------------------------------------------------------
void sdr_unpack_data(uint32_t data, int nbit, uint8_t *buff)
{
    for (int i = 0; i < nbit; i++) {
        buff[i] = (data >> (nbit - 1 - i)) & 1;
    }
}

// exclusive-or of all bits ----------------------------------------------------
uint8_t sdr_xor_bits(uint32_t X)
{
    static const uint8_t xor_8b[] = { // xor of 8 bits
        0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1,
        1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1, 0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0,
        1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1, 0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0,
        0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1,
        1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1, 0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0,
        0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1,
        0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1,
        1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1, 0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0
    };
    return xor_8b[(uint8_t)X] ^ xor_8b[(uint8_t)(X >> 8)] ^
        xor_8b[(uint8_t)(X >> 16)] ^ xor_8b[(uint8_t)(X >> 24)];
}

//===== sdr_code.c =============================================================

// constants ------------------------------------------------------------------
static const int8_t CHIP[] = {-1, 1};

// reverse bits in shift register ----------------------------------------------
int32_t rev_reg(int32_t R, int N)
{
    int32_t RR = 0;
    
    for (int i = 0; i < N; i++) {
        RR = (RR << 1) | ((R >> i) & 1);
    }
    return RR;
}

// generate code by LFSR -------------------------------------------------------
int8_t *LFSR(int N, int32_t R, int32_t tap, int n)
{
    int8_t *code = (int8_t *)sdr_malloc(N);
    
    for (int i = 0; i < N; i++) {
        code[i] = CHIP[R & 1];
        R = (sdr_xor_bits(R & tap) << (n - 1)) | (R >> 1);
    }
    return code;
}
