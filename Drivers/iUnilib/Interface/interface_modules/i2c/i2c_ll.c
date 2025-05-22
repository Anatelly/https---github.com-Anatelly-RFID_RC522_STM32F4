/**
******************************************************************************
* @file i2c_ll.с
* @author Дружинин А.А.
* @version v1.0
* @date  05-03-2021
* @brief Модуль i2c для работы с модулем интерфейсов.
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT STC </center></h2>
******************************************************************************
*/


#include "interface.h"

#define WRITE 0
#define READ 1

static i2c_error_t i2c_read_byte(i2c_t* const i2c, char* byte);
static i2c_error_t i2c_write_byte(i2c_t* const i2c, char* byte);
static i2c_error_t i2c_check_flag(const volatile uint32_t* reg, uint32_t mask, uint16_t time_value);
static i2c_error_t i2c_send_address(i2c_t* const i2c, char* address, uint8_t rw_bit);


// ========================================================================================================
/**
 * @brief Инициализация тактирования
 * @param i2c - структура данных i2c
 * @return none
 *
 */
static void i2c_init_rcc(i2c_t* const i2c)
{
	switch ((uint32_t)i2c->sfr)
	{

#ifdef I2C1
	case I2C1_BASE: __HAL_RCC_I2C1_CLK_ENABLE(); break;
#endif /* I2C1 */

#ifdef I2C2
	case I2C2_BASE: __HAL_RCC_I2C2_CLK_ENABLE(); break;
#endif /* I2C2 */

	default: break;
	}
}

// ========================================================================================================
/**
 * @brief Инициализация i2c
 * @param i2c - структура данных i2c
 * @return none
 *
 */
void i2c_init(i2c_t *const i2c)
{
	i2c->handler->Instance = i2c->sfr;
	HAL_I2C_DeInit(i2c->handler);
	i2c_init_rcc(i2c);

	HAL_I2C_Init(i2c->handler);
}
// ========================================================================================================
/**
 * @brief Отправка блока данных на устройство
 * @param i2c - структура данных i2c
 * @param src - указатель на буфер для передачи данных (адрес устройства + данные для отправки):\n
 * src[0] - адрес устройства при работе в режиме 8-битной адресации, далее следуют данные для передачи\n
 * src[0] и src[1] - адрес устройства при работе в режиме 10-битной адресации, далее следуют данные для передачи
 * @param len - длина сообщения для отправки (данные для отправки без учета адреса)
 * @return сообщение об ошибке
 */
i2c_error_t i2c_write_block(i2c_t* const i2c, char *src, size_t len)
{
	uint16_t addr = 0;

	if (i2c->handler->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
	{
		addr = *src;
		src++;
	}
	else
	{
		addr = (*src << 8) | *(src+1);
		src+=2;
	}


	if(HAL_I2C_Master_Transmit(i2c->handler, addr, (uint8_t*)src, len, i2c->timeout) == HAL_OK)
		return I2C_ERROR_NONE;

	return I2C_ERROR;

}

/**
 * @brief Чтение блока данных с устройства
 * @param i2c - структура данных i2c
 * @param src - указатель на буфер для передачи данных (адрес устройства + данные для отправки):\n
 * src[0] - адрес устройства при работе в режиме 8-битной адресации, далее следуют данные для передачи\n
 * src[0] и src[1] - адрес устройства при работе в режиме 10-битной адресации, далее следуют данные для передачи
 * @param len - длина сообщения для отправки (данные для отправки без учета адреса)
 * @return сообщение об ошибке
 */
i2c_error_t i2c_read_block(i2c_t* const i2c, char *src, size_t len)
{
	uint16_t addr = 0;

	// если адресация 10-битная еще раз данные
	if (i2c->handler->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
		addr = *src;
	else
		addr = *src + *(src+1);

	if(HAL_I2C_Master_Receive(i2c->handler, addr, (uint8_t*)src, len, i2c->timeout) == HAL_OK)
		return I2C_ERROR_NONE;

	return I2C_ERROR;

}


#define I2C_ASSIGN(N)                                                   \
                                                                        \
                                                                        \
    static I2C_HandleTypeDef i2c##N##_handler;                          \
                                                                        \
    i2c_t i2c##N =                                                      \
    {                                                                   \
        (I2C_TypeDef*)I2C##N##_BASE,                                    \
        &i2c##N##_handler,                                              \
        0                                                               \
    }


// ========================================================================================================
// Генерация по надобности (указано в interface_conf)
// ========================================================================================================
#if defined (I2C1_TX_BUFFER_SIZE) || defined (I2C1_RX_BUFFER_SIZE)
I2C_ASSIGN(1);
#endif /* I2C1 */

// ========================================================================================================
#if defined (I2C2_TX_BUFFER_SIZE) || defined (I2C2_RX_BUFFER_SIZE)
I2C_ASSIGN(2);
#endif /* I2C2 */
