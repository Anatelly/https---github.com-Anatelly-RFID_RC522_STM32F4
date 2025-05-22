/**
  ******************************************************************************
  * @file eeprom_protected.с 
  * @author 
  * @version v1.0
  * @date  27-04-2022
  * @brief stm32f10xxx or stm32f10xxx eeprom on flash memory emulation.
  ******************************************************************************
  * @attention 
  *
  * <h2><center>&copy; COPYRIGHT STC </center></h2>
  ******************************************************************************
  */
 
#ifndef EEPROM_PROTECTED_H_
#define EEPROM_PROTECTED_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
*	For STM32F40x use this table and look at flash_iap.
*	from 0 to 3 - 16kB, 0 - 64kB, 0 to 6 - 128 - kbyte, if you change 
*	page number page flash/configure flash/target/on-chip IROM1 and IROM2
*	
*	For stm32f1xx diap from 0 to 127
*/

#include <stdint.h>
#include <string.h>
#include "global_macro.h"
#include "var_eeprom.h"
#include "flash_hal.h"
#include "crc_hw.h"

#if F1_CHECK
#include "stm32f1xx_hal.h"
#elif F3_CHECK
#include "stm32f4xx_hal.h"
#elif F4_CHECK
#include "stm32f4xx_hal.h"
#endif

#ifndef NDEBUG
/*
**********************TEST THIS MODULE*****************************************
*	1. For test this module you must uncomment DEBUG_EEPROM 
*	2. You must define struct eeprom_t in module var_eeprom.h. For test you must copy struct below.
*		typedef struct
*		{		
*			float	current_default;
*			float	pressure_default;
*			float	supply;
*			size_t length;
*			uint32_t crc32;
*		} eeprom_t;
*	3. var_eeprom.h must include upper, than eeprom.h
*	4. 
**********************TEST THIS MODULE*****************************************
*/
//#define DEBUG_EEPROM
	#ifdef DEBUG_EEPROM
		void TEST_eeprom_protected_init(void);
	#endif
#endif

#ifndef DEBUG_EEPROM
	#define EEPROM_MAX_SIZE    1024
	//определяем номер страницы для записи(надо учитывать бутлоадер)
	//#define EEPROM_FLASH_BASE  FLSH_STARTING_ADDRESS + 127*FLASH_PAGE_SIZE 
#else

	#define EEPROM_FLASH_BASE	(uint32_t)&eeprom_arr[0]
	#define EEPROM_MAX_SIZE    10*sizeof(eeprom_t)-sizeof(eeprom_t)/2
	#ifdef MAIN
		eeprom_t eeprom_arr[10];
	#else
		extern eeprom_t eeprom_arr[10];
	#endif
#endif

typedef struct
{
	uint32_t for_write;
	uint32_t last_correct_data;
}eeprom_addr_t;

typedef struct
{
	uint8_t 	error;
	eeprom_addr_t addr;
}eeprom_settings_t;

void eeprom_protected_init(void);
int  eeprom_protected_sync(void);

#ifdef __cplusplus
}
#endif

#endif
