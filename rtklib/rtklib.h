#ifndef RTKLIB_H
#define RTKLIB_H

#include <stdint.h>

/* receiver raw data functions -----------------------------------------------*/
extern uint32_t rtk_crc24q (const uint8_t *buff, int len);

#endif /* RTKLIB_H */