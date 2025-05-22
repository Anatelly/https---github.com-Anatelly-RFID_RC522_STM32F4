#ifndef _CRC_SUM_H_
#define _CRC_SUM_H_

#include <stdlib.h>
#include <inttypes.h>

uint8_t crc_256(const void *buf, size_t len);

#endif