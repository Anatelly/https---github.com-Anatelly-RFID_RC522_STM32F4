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

//================================================================

#include "flash_hal.h"

//================================================================

/**
* @brief функция включения защиты от чтения
* @return none
*/
void flash_readout_protection(void)
{
	FLASH_OBProgramInitTypeDef OBInit;
	HAL_FLASHEx_OBGetConfig(&OBInit);

	if(OBInit.RDPLevel == OB_RDP_LEVEL_0)
	{
		OBInit.OptionType = OPTIONBYTE_RDP;
		OBInit.RDPLevel = OB_RDP_LEVEL_1;

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&OBInit);
		HAL_FLASH_OB_Launch();
		HAL_FLASH_OB_Lock();
		HAL_FLASH_Lock();
	}
}

#ifdef FLSH_PAGES

/**
* @brief функция для определения номера страницы по адресу
* @param address адрес флеш-памяти
* @return номер страницы
*/
uint32_t flash_page_get(uint32_t address)
{
	return (address - FLSH_STARTING_ADDRESS)/FLASH_PAGE_SIZE;
}

#elif defined FLSH_SECTORS

/**
* @brief функция для определения номера сектора по адресу
* @param address адрес флеш-памяти
* @return номер сектора памяти
*/
uint32_t flash_sector_get(uint32_t address)
{
	uint32_t sector_number = 0;


#ifdef FLASH_BANK_2
	uint16_t FLASH_SIZE = *(const uint16_t*)FLASHSIZE_BASE;

	if(FLASH_SIZE >= FLSH_SIZE_2M || (FLASH_SIZE == FLSH_SIZE_1M && READ_BIT(FLASH->OPTCR, FLASH_OPTCR_DB1M)) ){
		if(address >= FLSH_STARTING_ADDRESS + (FLASH_SIZE/2) * FLSH_1K_TO_BYTES){
			sector_number = FLSH_BANK_OFFSET;
			address -= (FLASH_SIZE/2) * FLSH_1K_TO_BYTES;
		}
	}
#endif

	if(address >= FLSH_STARTING_ADDRESS + FLSH_SECTOR_SIZE_128K){
		sector_number += ((address - FLSH_STARTING_ADDRESS)/FLSH_SECTOR_SIZE_128K + FLSH_128K_SECTOR_OFFSET);
	}
	else if (address >= FLSH_STARTING_ADDRESS + FLSH_SECTOR_SIZE_64K){
		sector_number += FLSH_128K_SECTOR_OFFSET;
	}
	else{
		sector_number += ((address - FLSH_STARTING_ADDRESS)/FLSH_SECTOR_SIZE_16K);
	}

	return sector_number;
}

#endif

/**
* @brief функция очистки flash-памяти
* @param address адрес флеш-памяти
* @param size размер байт, которые нужно стереть.
* Будьте внимательны: в результате выполнения операции будут стёрты все сектора, начиная от сектора, содержащего address и заканчивая сектором, содержащим address + size.
* Если требуется стереть всего 1 сектор, содержащий адрес, в качестве size можно подать 1.
* @return если всё в порядке и все сектора успешно стёрты, возвращает 0xFFFFFFFFU.
* В противном случае возвращает номер сектора/страницы с ошибкой.
*/
uint32_t flash_erase_size(uint32_t address, size_t size)
{
	FLASH_EraseInitTypeDef pEraseInit;
	uint32_t pError;
	uint32_t n = 0;

	// Защита от дураков
	if(size == 0)
	{
		size = 1;
	}

	size_t overflow_page = size - 1;

#ifdef FLSH_PAGES

	n = flash_page_get(address + overflow_page) - flash_page_get(address) + 1;

	pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	pEraseInit.PageAddress = address;
	pEraseInit.NbPages = n;

#ifndef STM32F3
#ifdef FLASH_BANK2_END
	pEraseInit.Banks = FLASH_BANK_BOTH;
#else
	pEraseInit.Banks = FLASH_BANK_1;
#endif
#endif

	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&pEraseInit, &pError);
	while(READ_BIT(FLASH->SR, FLASH_SR_BSY))
	{}
	HAL_FLASH_Lock();

#elif defined FLSH_SECTORS

	flsh_banks_erase banks = FLSH_ONE_BANK_ERASE;
	uint16_t FLASH_SIZE = *(const uint16_t*)FLASHSIZE_BASE;

#ifdef FLASH_BANK_2

	if(FLASH_SIZE >= FLSH_SIZE_2M || (FLASH_SIZE == FLSH_SIZE_1M && READ_BIT(FLASH->OPTCR, FLASH_OPTCR_DB1M)) )
	{
		if( (address < FLSH_STARTING_ADDRESS + (FLASH_SIZE/2) * FLSH_1K_TO_BYTES) && (address + overflow_page >= FLSH_STARTING_ADDRESS + (FLASH_SIZE/2) * FLSH_1K_TO_BYTES) )
		{
			banks = FLSH_TWO_BANKS_ERASE;
		}
	}

#endif

	switch(banks){

	case FLSH_ONE_BANK_ERASE:

		n = flash_sector_get(address + overflow_page) - flash_sector_get(address) + 1;
		pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
		pEraseInit.Sector = flash_sector_get(address);		// поиск сектора по адресу
		pEraseInit.NbSectors = n;
		pEraseInit.VoltageRange = VOLTAGE_RANGE;

		HAL_FLASH_Unlock();
		HAL_FLASHEx_Erase(&pEraseInit, &pError);
		while(READ_BIT(FLASH->SR, FLASH_SR_BSY))
		{}
		HAL_FLASH_Lock();

		break;

	case FLSH_TWO_BANKS_ERASE:
		// этап 1 - стираем всё, что нужно, в первой банке
		n = flash_sector_get(FLSH_STARTING_ADDRESS + (FLASH_SIZE/2) * FLSH_1K_TO_BYTES - 1) - flash_sector_get(address) + 1;
		pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
		pEraseInit.Sector = flash_sector_get(address);		// поиск сектора по адресу
		pEraseInit.NbSectors = n;
		pEraseInit.VoltageRange = VOLTAGE_RANGE;

		HAL_FLASH_Unlock();
		HAL_FLASHEx_Erase(&pEraseInit, &pError);
		while(READ_BIT(FLASH->SR, FLASH_SR_BSY))
		{}
		HAL_FLASH_Lock();

		// этап 2 - стираем всё, что нужно, во второй банке
		n = flash_sector_get(address + overflow_page) - flash_sector_get(FLSH_STARTING_ADDRESS + (FLASH_SIZE/2) * FLSH_1K_TO_BYTES) + 1;
		pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
		pEraseInit.Sector = flash_sector_get(FLSH_STARTING_ADDRESS + (FLASH_SIZE/2) * FLSH_1K_TO_BYTES);		// поиск сектора по адресу
		pEraseInit.NbSectors = n;
		pEraseInit.VoltageRange = VOLTAGE_RANGE;

		HAL_FLASH_Unlock();
		HAL_FLASHEx_Erase(&pEraseInit, &pError);
		while(READ_BIT(FLASH->SR, FLASH_SR_BSY))
		{}
		HAL_FLASH_Lock();

		break;

	default:
		pError = 0;
		break;

	}

#endif

	return pError;
}

/**
* @brief функция очистки flash-памяти
* @param address адрес флеш-памяти
* @param n число страниц/секторов, которые нужно стереть
* @return если всё в порядке и все сектора успешно стёрты, возвращает 0xFFFFFFFFU.
* В противном случае возвращает номер сектора/страницы с ошибкой.
*/
uint32_t flash_erase(uint32_t address, uint16_t n)
{
	FLASH_EraseInitTypeDef pEraseInit;
	uint32_t pError;

#ifdef FLSH_PAGES

	pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	pEraseInit.PageAddress = address;
	pEraseInit.NbPages = n;

#ifndef STM32F3
#ifdef FLASH_BANK2_END
	pEraseInit.Banks = FLASH_BANK_BOTH;
#else
	pEraseInit.Banks = FLASH_BANK_1;
#endif
#endif

	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&pEraseInit, &pError);
	while(READ_BIT(FLASH->SR, FLASH_SR_BSY))
	{}
	HAL_FLASH_Lock();

#elif defined FLSH_SECTORS

	pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	pEraseInit.Sector = flash_sector_get(address);		// поиск сектора по адресу
	pEraseInit.NbSectors = n;
	pEraseInit.VoltageRange = VOLTAGE_RANGE;

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
	HAL_FLASHEx_Erase(&pEraseInit, &pError);
	while(READ_BIT(FLASH->SR, FLASH_SR_BSY))
	{}
	HAL_FLASH_Lock();

#endif

	return pError;
}

/**
* @brief функция записи данных во флеш
* @param address указатель на переменную, содержащую адрес, куда будет производиться запись. Должен быть кратен 4-м.
* Инкрементируется внутри функции; при успешном результате будет равняться address + size (с выравниванием до 4-х байт).
* @param data массив данных
* @param size размер данных для записи в байтах
* @return статус операции
*/
flsh_error_status flash_write(uint32_t *address, void *data, size_t size)
{
	flsh_error_status status = FLSH_ERROR_NONE;
	HAL_StatusTypeDef hal_status = HAL_OK;
	uint32_t write = 0;
	size_t i = 0;
	// uint32_t *data = data_input;
	const size_t s = sizeof(uint32_t); // 4

	HAL_FLASH_Unlock();

	if(*address % s)
	{
		status = FLSH_ERROR;
		return status;
	}

	while(size)
	{
		if(size >= s)
		{
			memcpy(&write, data+i, sizeof(write));
			size -= s;
			i += s;
		}
		else
		{
			for(uint8_t e = 0; e < s; e++)
			{
				if(e < size)
				{
					write |= (uint32_t)(*(uint8_t *)(data + i + e)) << (8*e);
				}
				else
				{
					write |= (uint32_t)0xFF << (8*e);
				}
			}
			size = 0;
		}

		hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, *address, write);
		while(READ_BIT(FLASH->SR, FLASH_SR_BSY))
		{}

		if(hal_status == HAL_ERROR)	break;

		write = 0;
		*address += s;

	}
	HAL_FLASH_Lock();

	if(hal_status == HAL_ERROR) status = FLSH_ERROR;

	return status;
}
