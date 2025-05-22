/**
  ******************************************************************************
  * @file flash_hal.с 
  * @author Тепикин Д.А.
  * @version v1.0
  * @date  27-04-2022
  * @brief eeprom on flash memory emulation, основанный на библиотеке HAL
  ******************************************************************************
  * @attention 
  *
  * <h2><center>&copy; COPYRIGHT STC </center></h2>
  ******************************************************************************
  */

//================================================================
#include "eeprom_protected.h"

#include "include.h"
//================================================================

static eeprom_settings_t eeprom_settings;
static uint32_t			 EEPROM_FLASH_BASE;

/*
Выравнивание размера структуры eeprom до кратной 4.
Пример для структруы 87байт: ((87+4-3)/4)*4 = 88
Пример для структруы 84байт: ((84+4-3)/4)*4 = 84
*/
static uint8_t				 eesize_4_aligned =
	(((sizeof(eeprom_t) + sizeof(uint32_t) - 1) / sizeof(uint32_t)) << 2);

/**
 * @brief функция для определения конца памяти
 * @param address адрес флеш-памяти
 * @return Если произощел выход за границы памяти, возвращает 1.
 * В противном случае возвращает 0.
 */
static uint8_t eeprom_check_mem_end(uint32_t addr)
{
	//FLASHSIZE_BASE valid for stmf1xx
	uint32_t flash_size = (uint32_t)*(const uint16_t*)FLASHSIZE_BASE * 1024;

	if (addr + eesize_4_aligned > (FLSH_STARTING_ADDRESS + flash_size))
	{
		return (1);
	}
	else
	{
		return (0);
	}
}

/**
 * @brief функция проверки и очистки flash-памяти при необходимости 
 * @param eeprom_settings структура с настройками eeprom
 * @return none
 */
static void eeprom_clr_mem_if_need(eeprom_settings_t *eeprom_settings)
{
#if F1_CHECK
	if(!IS_FLASH_PROGRAM_ADDRESS(eeprom_settings->addr.for_write + eesize_4_aligned))
#elif F3_CHECK

#elif F4_CHECK
    if(!IS_FLASH_ADDRESS(eeprom_settings->addr.for_write + eesize_4_aligned))
#endif
	{

		eeprom_settings->addr.for_write = EEPROM_FLASH_BASE;
		
		#ifndef DEBUG_EEPROM
		/*	#ifdef STM32F40_41xxx

				 flash_erase_size(EEPROM_FLASH_BASE, sizeof(eeprom));
			#else
				flash_erase_size(EEPROM_FLASH_BASE, sizeof(eeprom));
			#endif*/
			flash_erase_size(EEPROM_FLASH_BASE, eesize_4_aligned);
		#else
			//for test clr some array may be.
		#endif
	}
}

/**
 * @brief Функция инициализации eeprom. Всегда вызывается первой.
 * @return none
 */
void eeprom_protected_init(void)
{
	//FLASHSIZE_BASE valid for stmf1xx
	uint32_t flash_size = (uint32_t)*(const uint16_t*)FLASHSIZE_BASE * 1024;

	//assert_param(sizeof(eeprom)<=flash_size);

	EEPROM_FLASH_BASE = FLSH_STARTING_ADDRESS + flash_size - EEPROM_MAX_SIZE;

	crc32hw_init();

	eeprom_settings.addr.for_write 			= EEPROM_FLASH_BASE;
	eeprom_settings.addr.last_correct_data 	= EEPROM_FLASH_BASE;
	
	uint32_t crc 					= 0xFFFFFFFF;	
	
	memcpy(&eeprom, (void*)EEPROM_FLASH_BASE, sizeof(eeprom));	

	if(sizeof(eeprom) != eeprom.length)
	{
		memset(&eeprom, 0, sizeof(eeprom));
		
		#ifndef DEBUG_EEPROM
			/*#ifdef STM32F40_41xxx
				flash_erase_size(EEPROM_FLASH_BASE, sizeof(eeprom));
			#else
				flash_erase_size(EEPROM_FLASH_BASE, sizeof(eeprom));
			#endif*/
			flash_erase_size(EEPROM_FLASH_BASE, eesize_4_aligned);
		#else
			//clr some array may be.
		#endif
		//dprintf("diff length. Flash erase\n");
	}
	else
	{
		for(uint32_t i = EEPROM_FLASH_BASE; i < FLSH_STARTING_ADDRESS + flash_size; i += eesize_4_aligned)
		{
			uint32_t addr = 0;
			if(eeprom_check_mem_end(i))
			{
				addr = i - eesize_4_aligned;
			}
			else
			{
				addr = i;
			}
			memcpy(&eeprom, (void*)addr, sizeof(eeprom));
			
			crc = crc32hw((uint8_t *)&eeprom, sizeof(eeprom) - 4, 1);
			if(eeprom.crc32 != crc)
			{				
				//dprintf("bad crc sector:%X,correct_sec:%X\n", addr, eeprom_settings.addr.last_correct_data);
				memset(&eeprom, 0xFF, sizeof(eeprom));
				int res = memcmp((void*)addr, (void*)&eeprom, sizeof(eeprom));
				if(res == 0) //if mem dummy
				{
					//dprintf("dummy sector:%X,correct_sec:%X\n", i, eeprom_settings.addr.last_correct_data);
					memcpy(&eeprom, (void*)eeprom_settings.addr.last_correct_data, sizeof(eeprom));
					eeprom_settings.addr.for_write = addr;

//					eeprom_clr_mem_if_need(&eeprom_settings);
					break;
				}
			}
			else
			{
				eeprom_settings.addr.last_correct_data = addr;
			}
		}
	}
	
	eeprom.length = sizeof(eeprom);
}


/**
* @brief функция записи изменений в eeprom
* @return Возвращает 1 если все байты удалось записать
* в противном случае возвращает 0.
*/
int eeprom_protected_sync(void)
{
	int i = 3; // retries qty

	eeprom_clr_mem_if_need(&eeprom_settings);

	eeprom.crc32 = crc32hw((uint8_t *)&eeprom, sizeof(eeprom) - 4, 1);

	#ifdef DEBUG_EEPROM
		//write some arr to mem.
	#else
		while (flash_write(&eeprom_settings.addr.for_write, (void*)&eeprom, sizeof(eeprom)) != FLSH_ERROR_NONE  &&  --i) {;}
	#endif
	
	eeprom_settings.addr.for_write += eesize_4_aligned;
		
#ifdef DEBUG_EEPROM
	static int z = 0;
	dprintf("addr:%X, i:%d\n", eeprom_settings.addr.for_write, z++);
#endif
	return (!i);
}

#ifdef DEBUG_EEPROM

//=============================================================================

/**
* @brief Проверка записи в eeprom на выход за границы flash-памяти.
*/
void TEST_eeprom_protected_init(void)
{
	eeprom_t ep = 
	{
		.current_default = 1.0F,
		.pressure_default = 1.1,
		.supply = 11.1,
		.length = sizeof(eeprom_t)
	};
	//init test sequence
	for(uint32_t i = 0; i < 10; i++)
	{
		ep.current_default 				+= 1.0F;
		eeprom_arr[i].current_default 	= ep.current_default;
		eeprom_arr[i].length 			= ep.length;
		eeprom_arr[i].crc32 			=  crc32_sftwr(0, &eeprom_arr[i], sizeof(eeprom) - 4);
	}
	
	
	eeprom_protected_init();
	
	uint32_t addr = (uint32_t)&eeprom_arr[8];
	
	if(addr == eeprom_settings.addr.last_correct_data)//in this case addr with current_default = 10.
	{
		 HAL_Delay(20);
	}
	else
	{
		while(1){ HAL_Delay(100);};
	}
	
}

#endif

