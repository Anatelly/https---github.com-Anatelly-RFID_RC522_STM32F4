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
#include "at25df.h"


static uint8_t buf[5]; // буфер отправки команд и чтения статусов
int at25;


/**=================================================================================
/**
 * @brief Отправка команды
 * @param cmd - код команды
 * @return  none
 *
 */
static void at25df_cmd(uint_fast8_t cmd)
{
    pin_clr(CS);

    write(at25, (char*)&cmd, 1);

    pin_set(CS);

    delay_ns(AT25DF_TIMEOUT_NS_CS_HIGH);
}


/**=================================================================================
/**
 * @brief Отправка команды на разрешение записи
 * @param none
 * @return  none
 * @attention Необходимо вызывать перед любой операцией записи
 */
static void at25df_write_enable(void)
{
	at25df_cmd(AT25DF_CMD_WRITE_ENABLE);
}


/**=================================================================================
/**
 * @brief Отправка команды на запрещение записи
 * @param none
 * @return  none
 */
void at25df_write_disable(void)
{
	at25df_cmd(AT25DF_CMD_WRITE_DISABLE);
}


/**=================================================================================
/**
 * @brief Чтение статуса
 * @param none
 * @return none
 */
static uint_fast8_t at25df_status(void)
{
	buf[0] = AT25DF_CMD_READ_STATUS_REG;

    pin_clr(CS);

    write(at25, (char*)buf, 1);

    read(at25, (char*)buf, 2);

    pin_set(CS);

    delay_ns(AT25DF_TIMEOUT_NS_CS_HIGH);

	return (buf[1]);
}


/**=================================================================================
/**
 * @brief Запись в регистр статуса
 * @param status - необходимый статус
 * @return none
 * @attention для выполнения требуется не более 200 нс
 */
static void at25df_write_status(uint_fast8_t status)
{
	buf[0] = AT25DF_CMD_WRITE_STATUS_REG;
	buf[1] = (uint8_t)status;

	at25df_write_enable();

    pin_clr(CS);

    write(at25, (char*)buf, 2);

    pin_set(CS);

    delay_ns(AT25DF_TIMEOUT_NS_WRITE_STATUS_REG);
}


/**=================================================================================
/**
 * @brief Проверка готовности микросхемы к работе
 * @param none
 * @return none
 */
static uint_fast8_t at25df_ready(void)
{
	return (!(at25df_status() & (uint_fast8_t)AT25DF_STATUS_BUSY));
}


/**=================================================================================
/**
 * @brief Проверка наличия ошибки
 * @param none
 * @return none
 */
static uint_fast8_t at25df_error(void)
{
	return ((at25df_status() & (uint_fast8_t)AT25DF_STATUS_ERROR) != 0);
}


/**=================================================================================
/**
 * @brief Ожидание готовности микросхемы к работы
 * @param us - время ожидания ответа о готовности
 * @return состояние готовности
 */
static uint_fast8_t at25df_wait_us_ready(uint32_t us)
{
	uint_fast8_t ready = 0;

	while (!(ready|=at25df_ready()) && us--)
		delay_us(1);

	return (ready);
}


/**=================================================================================
/**
 * @brief Включение режима deep power-down
 * @param none
 * @return none
 */
void at25df_power_down(void)
{
	at25df_cmd(AT25DF_CMD_DEEP_POWER_DOWN);
}


/**=================================================================================
/**
 * @brief Выключение режима deep power-down
 * @param none
 * @return none
 */
void at25df_power_down_off(void)
{
	at25df_cmd(AT25DF_CMD_DEEP_POWER_DOWN_OFF);

	delay_us(3); // RDPD time
}


/**=================================================================================
/**
 * @brief Запрос id устройства
 * @param none
 * @return none
 */
uint32_t at25df_id(void)
{
	buf[0] = AT25DF_CMD_READ_ID;

    pin_clr(CS);

    write(at25, (char*)buf, 1);

    read(at25, (char*)buf, 4);

    pin_set(CS);

    delay_ns(AT25DF_TIMEOUT_NS_CS_HIGH);

	return (   ((uint32_t)buf[0]<<24)
	         | ((uint32_t)buf[1]<<16)
	         | ((uint32_t)buf[2]<< 8)
	         | ((uint32_t)buf[3]<< 0));
}


/**=================================================================================
/**
 * @brief Настройка spi и отправка команды для начала работы
 * @param ifname - строка с именем интерфейса.
 * @param flags - флаги открытия интерфейса. Оставлен для совместимости с POSIX стандартом.
 * @param conf - настройки spi.
 * @return Возвращает номер идентификатора открытого интерфейса, через который в дальнейшем и ведется работа
 * @attention Перед вызовом функции должна быть предусмотрена задержка 10мс.
 */
int at25df_init(char* ifname, int flags, struct termios conf)
{

    // Открываем и настраиваем spi для работы с at25df
    at25 = open (ifname, flags);
    if (at25 < 0) return at25;
    tcgetattr(at25, &conf);

    tcsetattr(at25, 0, &conf);

    // Разрешаем запись
    at25df_set_swp_bits(0);
    at25df_status()&AT25DF_STATUS_NWP_PIN_STATE;

    return at25;
}


/**=================================================================================
/**
 * @brief Чтение данных
 * @param address - адресс, откуда необходимо прочитать данные.
 * @param dst - буфер, куда будут записаны считанные данные.
 * @param qty - объем, который необходимо прочитать.
 * @return none
 */
void at25df_read(uint32_t address, void* dst, size_t qty)
{
	if (dst && qty)
	{
		buf[0] = AT25DF_CMD_READ_ARRAY;
		buf[1] = (uint8_t)(address>>16);
		buf[2] = (uint8_t)(address>>8);
		buf[3] = (uint8_t)(address>>0);
		buf[4] = (uint8_t)(0);           // don't care

        pin_clr(CS);

        write(at25, (char*)buf, 5);

        read(at25, (char*)dst, qty);

        pin_set(CS);

        delay_ns(AT25DF_TIMEOUT_NS_CS_HIGH);
	}
}


/**=================================================================================
/**
 * @brief Запись блока данных
 * @param address - адресс, куда необходимо записать данные.
 * @param src - буфер, откуда необходимо взять данные для записи.
 * @param qty - объем, который необходимо записать.
 * @return none
 */
static int at25df_write_block(uint32_t address, const void* src, size_t qty) // unsafe  (w/o page boundary checking)
{
	if (src && qty)
	{
		buf[0] = AT25DF_CMD_WRITE_ARRAY;
		buf[1] = (uint8_t)(address>>16);
		buf[2] = (uint8_t)(address>>8);
		buf[3] = (uint8_t)(address>>0);

		at25df_write_enable();  // must be exicuted before any writes operation

        pin_clr(CS);

        write(at25, (char*)buf, 4);

        write(at25, (char*)src, qty);

        pin_set(CS);

        delay_ns(AT25DF_TIMEOUT_NS_CS_HIGH);

	}

	return (at25df_wait_us_ready(1000UL*AT25DF_TIMEOUT_MS_WRITE_PAGE) && !at25df_error());
}


/**=================================================================================
/**
 * @brief Запись данных
 * @param address - адресс, куда необходимо записать данные.
 * @param src - буфер, откуда необходимо взять данные для записи.
 * @param qty - объем, который необходимо записать.
 * @return none
 */
int at25df_write(uint32_t address, const void* src, size_t qty)
{

	while (qty)
	{
		size_t once = AT25DF_PAGE_SIZE - (address % AT25DF_PAGE_SIZE);   // end page align

		if (once>qty)
		{
			once = qty;
		}

		if (!at25df_write_block(address, src, once))
		{
			return (0);
		}

		address += once;
		src      = (void*)((int)src+once);
		qty     -= once;
	}

	return (1);

}


/**=================================================================================
/**
 * @brief Очистка блока данных
 * @param address - адресс стирания.
 * @param erase_cmd - команда, которая указывает объем данных очистки.
 * @return none
 */
static void at25df_erase_block(uint32_t address, uint_fast8_t erase_cmd)
{
	buf[0] = (uint8_t)erase_cmd;
	buf[1] = (uint8_t)(address>>16);
	buf[2] = (uint8_t)(address>>8);
	buf[3] = (uint8_t)(address>>0);

	at25df_write_enable();

    pin_clr(CS);

    write(at25, (char*)buf, 4);

    pin_set(CS);

    delay_ns(AT25DF_TIMEOUT_NS_CS_HIGH);
}


/**=================================================================================
/**
 * @brief Очистка 4кБайт данных
 * @param address - адресс стирания.
 * @return наличие/отсутствие ошибки
 */
int at25df_erase_block4k(uint32_t address) // absolute memory address within block
{

	at25df_erase_block(address, AT25DF_CMD_ERASE_BLOCK4K);

	return (at25df_wait_us_ready(1000UL*AT25DF_TIMEOUT_MS_ERASE_BLOCK4K) && !at25df_error());
}


/**=================================================================================
/**
 * @brief Очистка 32кБайт данных
 * @param address - адресс стирания.
 * @return наличие/отсутствие ошибки
 */
int at25df_erase_block32k(uint32_t address) // absolute memory address within block
{
	at25df_erase_block(address, AT25DF_CMD_ERASE_BLOCK32K);

	return (at25df_wait_us_ready(1000UL*AT25DF_TIMEOUT_MS_ERASE_BLOCK32K) && !at25df_error());
}


/**=================================================================================
/**
 * @brief Очистка 32кБайт данных
 * @param address - адресс стирания.
 * @return наличие/отсутствие ошибки
 */
int at25df_erase_block64k(uint32_t address)
{
	at25df_erase_block(address, AT25DF_CMD_ERASE_BLOCK64K);

	return (at25df_wait_us_ready(1000UL*AT25DF_TIMEOUT_MS_ERASE_BLOCK64K) && !at25df_error());
}


/**=================================================================================
/**
 * @brief Полная очистка
 * @param none.
 * @return наличие/отсутствие ошибки
 */
int at25df_erase(void)
{
	at25df_write_enable();

	at25df_cmd(AT25DF_CMD_ERASE_CHIP);

	return (at25df_wait_us_ready(1000UL*AT25DF_TIMEOUT_MS_ERASE_CHIP) && !at25df_error());
}


/**=================================================================================
/**
 * @brief Запрос статуса Software Protection bits
 * @param none.
 * @return none
 */
uint_fast8_t at25df_get_swp_bits(void)
{
	return (at25df_status() & (uint_fast8_t)AT25DF_STATUS_SW_PROTECTION);
}


/**=================================================================================
/**
 * @brief Запись Software Protection bits
 * @param mask - битовая маска.
 * @return none
 */
void at25df_set_swp_bits(uint_fast8_t mask)
{
	uint_fast8_t status_wo_swp = at25df_status() & (uint_fast8_t)~AT25DF_STATUS_SW_PROTECTION;

	at25df_write_status(status_wo_swp & (mask & AT25DF_STATUS_SW_PROTECTION));
}
