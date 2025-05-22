/*!
 * \file
 * В данном файле создаются интерфейсы, которые будут использоваться в проекте.
 * Для того, чтобы создать интерфейс - необходимо просто определить макрос интерфейса и буфер
 * \warning Внимание! Размер буфера должен быть степенью числа 2
 * \warning i2c не работает в режиме прерываний, размер буфера для i2c введен для единообразия
 * \details дефайном INTERFACE_STATIC_MODE задается статичный режим работы модуля, т.е. создается конечный пул
 * элементов интерфейсов, количество которых задается с помощью дефайна INTERFACE_STATIC_MAX_INTERFACES
 * \details дефайном INTERFACE_DINAMIC_MODE задается динамический режим работы модуля, т.е. используется динамическое
 * выделение памяти под элементы интерфейса.
 */
#ifndef _INTERFACE_CONF_H_
#define _INTERFACE_CONF_H_


#if F1_CHECK
#include "stm32f1xx_hal.h"

#elif F3_CHECK
#include "stm32f3xx_hal.h"

#elif F4_CHECK
#include "stm32f4xx_hal.h"
#endif

#define INTERFACE_STATIC_MODE
//#define INTERFACE_DYNAMIC_MODE

#define INTERFACE_STATIC_MAX_INTERFACES                 6						// Количество используемых интерфейсов

#if !defined (INTERFACE_STATIC_MODE) && !defined (INTERFACE_DYNAMIC_MODE)
#error "Please select the interface work mode - dynamic or static"
#endif
#if defined (INTERFACE_STATIC_MODE) && defined (INTERFACE_DYNAMIC_MODE)
#error "Please select only one work mode of interface module"
#endif

// ********************* Определяем интерфейс, который будет использоваться ********************* //

#define INTERFACE_UART

//#define INTERFACE_SPI

//#define INTERFACE_I2C

//#define INTERFACE_SX1276_LORA

//#define INTERFACE_SX1276_FSK

// ********************* Определяем интерфейс, который будет использоваться ********************* //



// *************************** Определяем буфер у нужного интерфейса **************************** //

#define UART1_TX_BUFFER_SIZE                           256
#define UART1_RX_BUFFER_SIZE                           2048

#define UART2_TX_BUFFER_SIZE                           512
#define UART2_RX_BUFFER_SIZE                           2048

//#define UART3_TX_BUFFER_SIZE                             512
//#define UART3_RX_BUFFER_SIZE                             512

//#define UART4_TX_BUFFER_SIZE                           256
//#define UART4_RX_BUFFER_SIZE                           256

//#define UART5_TX_BUFFER_SIZE                           256
//#define UART5_RX_BUFFER_SIZE                           256

//#define UART6_TX_BUFFER_SIZE                           256
//#define UART6_RX_BUFFER_SIZE                           256

//#define I2C1_TX_BUFFER_SIZE                            256
//#define I2C1_RX_BUFFER_SIZE                            256

//#define I2C2_TX_BUFFER_SIZE                            256
//#define I2C2_RX_BUFFER_SIZE                            256

//#define SPI1_TX_BUFFER_SIZE                            256
//#define SPI1_RX_BUFFER_SIZE                            256

//#define SPI2_TX_BUFFER_SIZE                            256
//#define SPI2_RX_BUFFER_SIZE                            256

//#define SPI3_TX_BUFFER_SIZE                            256
//#define SPI3_RX_BUFFER_SIZE                            256
// *************************** Определяем буфер у нужного интерфейса **************************** //


// *************************** Настройка портов для радиоканала **************************** //

//#define SX1276_FSK1_TX_BUFFER_SIZE                      512
//#define SX1276_FSK1_RX_BUFFER_SIZE                      512
//#define SX1276_FSK1_SPI                                 SPI1
//#define SX1276_FSK1_CSPORT                              GPIOA
//#define SX1276_FSK1_CSPIN                               GPIO_PIN_4
//#define SX1276_FSK1_RESETPORT                           GPIOC
//#define SX1276_FSK1_RESETPIN                            GPIO_PIN_8
//
//#define SX1276_LORA1_TX_BUFFER_SIZE                      512
//#define SX1276_LORA1_RX_BUFFER_SIZE                      512
//#define SX1276_LORA1_SPI                                 SPI1
//#define SX1276_LORA1_CSPORT                              GPIOA
//#define SX1276_LORA1_CSPIN                               GPIO_PIN_4
//#define SX1276_LORA1_RESETPORT                           GPIOC
//#define SX1276_LORA1_RESETPIN                            GPIO_PIN_8


// *************************** Настройка портов для радиоканала **************************** //

#endif /* _INTERFACE_CONF_H_ */
