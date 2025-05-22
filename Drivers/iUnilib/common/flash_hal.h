/**
******************************************************************************
* @file flash_hal.с
* @author Филиппов А.А.
* @version v1.0
* @date  28-09-2021
* @brief Модуль для работы с flash-памятью, основанный на библиотеке HAL
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT STC </center></h2>
******************************************************************************
*/


#ifndef FLASH_HAL_H
#define FLASH_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "string.h"
#include "global_macro.h"

#if F1_CHECK
#include "stm32f1xx_hal.h"
#elif F3_CHECK
#include "stm32f4xx_hal.h"
#elif F4_CHECK
#include "stm32f4xx_hal.h"
#endif

typedef enum
{
	FLSH_ONE_BANK_ERASE    = 0x00,
	FLSH_TWO_BANKS_ERASE   = 0x01,

} flsh_banks_erase;

typedef enum
{
	FLSH_ERROR_NONE      = 0x00,
	FLSH_ERROR            = 0x01,
} flsh_error_status;


#if defined (STM32F1) || defined (STM32F3)
#define FLSH_PAGES
#elif defined (STM32F4)

#define FLSH_SECTORS

#define FLSH_SECTOR_SIZE_16K                        0x4000
#define FLSH_SECTOR_SIZE_64K                        0x10000
#define FLSH_SECTOR_SIZE_128K                       0x20000

#define FLSH_1K_TO_BYTES                            1024

#define FLSH_128K_SECTOR_OFFSET                     4
#define FLSH_BANK_OFFSET                            12

#define FLSH_SIZE_1M                                1024
#define FLSH_SIZE_2M                                2048

#define VOLTAGE_RANGE                               FLASH_VOLTAGE_RANGE_3

/**
*    VOLTAGE_RANGE необходимо выбрать из следующих вариантов:
*
*    FLASH_VOLTAGE_RANGE_1        !< Device operating range: 1.8V to 2.1V
*    FLASH_VOLTAGE_RANGE_2        !< Device operating range: 2.1V to 2.7V
*    FLASH_VOLTAGE_RANGE_3        !< Device operating range: 2.7V to 3.6V
*    FLASH_VOLTAGE_RANGE_4        !< Device operating range: 2.7V to 3.6V + External Vpp
*/

#endif

//================================================================

void flash_readout_protection(void);
flsh_error_status flash_write(uint32_t *address, void *data, size_t size);
uint32_t flash_erase_size(uint32_t address, size_t size);
uint32_t flash_erase(uint32_t address, uint16_t n);

#if defined FLSH_PAGES
uint32_t flash_page_get(uint32_t address);
#elif defined FLSH_SECTORS
uint32_t flash_sector_get(uint32_t address);
#endif

//================================================================

#define FLSH_STARTING_ADDRESS                    0x08000000


#ifdef __cplusplus
}
#endif

#endif
