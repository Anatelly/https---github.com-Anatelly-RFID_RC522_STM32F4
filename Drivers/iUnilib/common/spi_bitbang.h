/* 
 *  spi_bitbang.h rev.2.0.0 2012-01-17
 *
 *  BitBang SPI interface
 *
 *  Contributors:
 *                Ivanov Anton (c) www.automatix.ru
 * 
 */

#ifndef _SPI_BITBANG_H_
#define _SPI_BITBANG_H_

#include <stdlib.h>
#include <stdint.h>
#include "spi_bitbang_conf.h"

void         spi_init          (void);
uint_fast8_t spi_transfer      (uint_fast8_t data);
void*        spi_transfer_block(void* ptr, size_t qty);
void*        spi_read_block    (void* dst, size_t qty);
void         spi_write_block   (const void* src, size_t qty);

#define _spi_device_action_block(ACTION, DEV, PTR, QTY) \
do                                                      \
{                                                       \
	spi_select(DEV);                                    \
	spi_##ACTION##_block((PTR), (QTY));                 \
	spi_deselect(DEV);                                  \
}                                                       \
while (0)

#define spi_device_transfer_block(DEV, PTR, QTY)  _spi_device_action_block(transfer, DEV, (PTR), (QTY))
#define spi_device_read_block(    DEV, DST, QTY)  _spi_device_action_block(read,     DEV, (DST), (QTY))
#define spi_device_write_block(   DEV, SRC, QTY)  _spi_device_action_block(write,    DEV, (SRC), (QTY))


#endif
