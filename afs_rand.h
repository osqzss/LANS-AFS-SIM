#ifndef AFS_RAND_H
#define AFS_RAND_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h> // for seed value

int16_t randn();
void srandn(uint32_t seed) ;

#endif
