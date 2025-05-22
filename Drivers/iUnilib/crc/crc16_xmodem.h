/*
 *  crc16.h rev.2.2.0 2010-06-07
 *
 *  CRC-16 XMODEM
 *
 *  Polynomial: x^16 + x^12 + x^5 + 1 (0x1021)<br>
 *  Initial value: 0x0000
 *
 *  This CRC is normally used by XMODEM protocol.
 *
 *  Contributors:
 *                Ivanov Anton (c) www.automatix.ru
 */




#ifndef CRC16_XMODEM_H_
#define CRC16_XMODEM_H_

#include <stdlib.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t crc16_xmodem(const uint8_t* buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif 
