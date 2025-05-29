#ifndef AFS_LDPC_H
#define AFS_LDPC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "alloc.h"
#include "mod2sparse.h"
#include "pocketsdr.h"
#include "rtklib.h"

void encode_LDPC_AFS_SF2(const uint8_t* syms, uint8_t* syms_enc);
void encode_LDPC_AFS_SF3(const uint8_t* syms, uint8_t* syms_enc);
void append_CRC24(uint8_t* syms, int len);
void generate_BCH_AFS_SF1(uint8_t* syms, int fid, int toi);
void interleave_AFS_SF234(uint8_t* syms_in, uint8_t* syms_out);

#endif
