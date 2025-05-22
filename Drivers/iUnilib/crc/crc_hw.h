//
// Created by user on 09.03.2021.
//

#ifndef _CRC_HW_H
#define _CRC_HW_H

#include "global_macro.h"

#if F1_CHECK
#include "stm32f1xx_hal.h"
#elif F3_CHECK
#include "stm32f3xx_hal.h"
#elif F4_CHECK
#include "stm32f4xx_hal.h"
#endif

void crc32hw_init		(void);
uint32_t crc32hw		(const void *buf, int len, int clear);
uint32_t crc32hw_append	(uint32_t crc, const void *buf, uint32_t len);

#endif /* _CRC_HW_H */
