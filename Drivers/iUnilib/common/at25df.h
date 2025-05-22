/**
******************************************************************************
* @file at25df.с
* @author Дружинин А.А.
* @version v0.3
* @date  16-02-2021
* @brief Модуль для исползования микросхемы at25df, переведен под модуль интерфейсов
******************************************************************************
* @attention
* Использовать 1 или 3 режим SPI;
* Конфигурация последовательности бит MSB;
* Задержка при включении устройства перед программированием или стиранием составляет 10 мс;
* Flash имеет страничную структуру с автоинкрементным указателем write/read внутри страницы;
* Перед использованием необходимо определить ногу CS в .h файле как для pin_macro.
*
* <h2><center>&copy; COPYRIGHT STC </center></h2>
******************************************************************************
*/
#include <stdint.h>
#include "global_macro.h"
#include "atomic.h"
#include "pin_macro.h"
#include "delay.h"
#include "interface.h"
#include "low_level.h"
#include "termios.h"



#ifndef _AT25DF_H_
#define _AT25DF_H_

#define AT25DF321                     1


#define AT25DF_TYPE                   AT25DF321         // Выбор используемой микросхемы из семейства at25df


#define CS                            MEM_CS  // Настройка CS

//=============================================================================
// Характеристики
//=============================================================================

#if (AT25DF_TYPE==AT25DF321)
#	define AT25DF_SIZE                0x400000UL
#	define AT25DF_PAGE_SIZE           256U
#	define AT25DF_PAGES_QTY           (AT25DF_SIZE/AT25DF_PAGE_SIZE)//-4

#	define AT25DF_SECTOR_SIZE         65536UL
#	define AT25DF_SECTORS_QTY         ((AT25DF_SIZE)/AT25DF_SECTOR_SIZE)// - 4 * AT25DF_PAGE_SIZE

#	define AT25DF_BLOCK_4K_SIZE       4096U
#	define AT25DF_BLOCK_32K_SIZE      32768U
#	define AT25DF_BLOCK_64K_SIZE      65536UL


#	define AT25DF_ID                  0x1F470100UL
#else
#	error WRONG OR NOT YET SUPPORTED AT25DF_TYPE!!!
#endif

//=============================================================================
// Команды
//=============================================================================
enum
{
	AT25DF_CMD_READ_ARRAY           = 0x0B,   // max 70MHz +1 don't care dummy byte
	AT25DF_CMD_READ_ARRAY_LOW_FRQ   = 0x03,   // max 33MHz w/o don't care dummy byte

    AT25DF_CMD_WRITE_ARRAY          = 0x02,   // 1.5-5 ms (write one byte time is 6us)

	AT25DF_CMD_ERASE_BLOCK4K        = 0x20,   // 50-200  ms
	AT25DF_CMD_ERASE_BLOCK32K       = 0x52,   // 350-600 ms
	AT25DF_CMD_ERASE_BLOCK64K       = 0xD8,   // 600-950 ms

	AT25DF_CMD_ERASE_CHIP           = 0x60,   // 36-56 sec !!!
	AT25DF_CMD_ERASE_CHIP2          = 0xC7,   // the same as prev

	AT25DF_CMD_WRITE_ENABLE         = 0x06,
	AT25DF_CMD_WRITE_DISABLE        = 0x04,

	AT25DF_CMD_PROTECT_SECTOR       = 0x36,
	AT25DF_CMD_UNPROTECT_SECTOR     = 0x39,
	AT25DF_CMD_READ_PROTECTION_REGS = 0x3C,

	AT25DF_CMD_READ_STATUS_REG      = 0x05,
	AT25DF_CMD_WRITE_STATUS_REG     = 0x01,   // 200 ns max

	AT25DF_CMD_READ_ID              = 0x9F,

	AT25DF_CMD_DEEP_POWER_DOWN      = 0xB9,
	AT25DF_CMD_DEEP_POWER_DOWN_OFF  = 0xAB
};

//=============================================================================
// Таймауты для выполнения операции
//=============================================================================
enum
{
	AT25DF_TIMEOUT_MS_WRITE_PAGE       = 5,
	AT25DF_TIMEOUT_MS_ERASE_CHIP       = 56000,
	AT25DF_TIMEOUT_MS_ERASE_BLOCK4K    = 200,
	AT25DF_TIMEOUT_MS_ERASE_BLOCK32K   = 600,
	AT25DF_TIMEOUT_MS_ERASE_BLOCK64K   = 950,

	AT25DF_TIMEOUT_NS_WRITE_STATUS_REG = 200, // 200 ns max
	AT25DF_TIMEOUT_NS_CS_HIGH          = 50   // 50  ns min
};

//=============================================================================
// Регистр статуса
//=============================================================================
enum
{
	AT25DF_STATUS_SPR_LOCK      = (1<<7),
	AT25DF_STATUS_DUMMY         = (1<<6),
	AT25DF_STATUS_ERROR         = (1<<5),
	AT25DF_STATUS_NWP_PIN_STATE = (1<<4),
	AT25DF_STATUS_SW_PROTECTION = (1<<3)|(1<<2),
	AT25DF_STATUS_WR_EN         = (1<<1),
	AT25DF_STATUS_BUSY          = (1<<0)
};

//=============================================================================

int at25df_init(char* ifname, int flags, struct termios conf);

uint32_t      at25df_id             (void);
void          at25df_read           (uint32_t address, void* dst,       size_t qty);
int           at25df_write          (uint32_t address, const void* src, size_t qty);
int           at25df_erase          (void);
int           at25df_erase_block4k  (uint32_t address);
int           at25df_erase_block32k (uint32_t address);
int           at25df_erase_block64k (uint32_t address);

uint_fast8_t  at25df_get_swp_bits   (void);
void          at25df_set_swp_bits   (uint_fast8_t mask);

void          at25df_power_down     (void);
void          at25df_power_down_off (void);
void          at25df_write_disable  (void);



#endif //_AT25DF_H_
