#ifndef TL_PROTOCOL_H
#define TL_PROTOCOL_H

#include <stdint.h>

#include "software_timer.h"
#include "process_big_filesize.h"
#include "crc32_software.h"

#if defined(__linux) || defined(_WIN32)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#else
#include "interface.h"
#endif


#define TL_HEAD_SIZE                    (8U)                        // Размер заголовка
#define TL_CRC_SIZE                     (4U)                        // Размер crc
#define TL_PREFIX                       (0x4F)                      // Префикс транспортного уровня протокола
#define TL_LOST_NUM                     (5U)                        // Количество повторных пакетов посланных подряд для разрыва соединения

#define TL_BUFFER_SIZE                  63                         // Используемый буфер


#define TL_MESSAGE_DATA_SIZE     (TL_BUFFER_SIZE - (TL_HEAD_SIZE))  // Длина сообщения для передачи данных


typedef enum
{
	TL_CONNECTION_REQUEST           =  0,                      // Запрос на обмен данными
	TL_SEND_DATA                    =  1,                      // Отправка данных
	TL_SEND_DATA_CONFIRM            =  2,                      // Получение данных подтверждено
	TL_SEND_STOP                    =  3,                      // Окончена передача данных
	TL_SEND_STOP_CONFIRM            =  4,                      // Подтверждение, что информация об окончании получена
	TL_ERROR_ACK                    =  5,                      // Неверные данные
	TL_ERROR_MEMORY                 =  6,                      // В буфере для приема не хватает места
} tl_opcode_t;

typedef enum
{
	TL_ERROR                        =  0,                      // Ошибка
	TL_LOST_CONNECT                 =  1,                      // Потеря связи
	TL_PROCESS_SEND                 =  2,                      // Идет передача
	TL_PROCESS_END                  =  3,                      // Передача завершена
	TL_EMPTY                        =  4,                      // Нет сообщения
	TL_REPEAT                       =  5,                      // Произошел повтор сообщения
	TL_PROCESS_END_MEM              =  6,                      // Передача завершена из-за нехватки места в буфере для приема
} tl_process_t;

typedef enum
{
	TL_RX                        =  0,                          // Устройство находится в режиме приемника
	TL_TX                        =  1,                          // Устройство находится в режиме передатчика
} tl_device_status_t;

typedef struct __attribute__((packed))
{
	uint8_t prefix;                                            // Префикс транспортного уровня протокола
	uint8_t opcode;                                            // Код операции
	uint8_t current_pckt;                                      // Текущий пакет
	uint8_t msg_len;                                           // Длина данных
	uint32_t tl_crc;                                           // Контрольная сумма транспортного уровня
	uint8_t msg[TL_MESSAGE_DATA_SIZE];                         // Данные
} tl_message_t;

typedef struct
{
	timeout_t timer;                                     // Указатель на таймер для повторной отправки сообщения или сброса флага занятости
	uint16_t timeout;                                    // Таймаут таймера повторной отправки или сброса флага занятости
} tl_timer_t;

typedef struct
{
	uint8_t* data_file;                                         // Файл для приема/передачи
	uint16_t file_size;                                         // Размер файла для приема/передачи
	uint16_t result_offset;                                     // Переданная/полученная часть файла
	tl_timer_t time_info;                                       // Структура таймера
} tl_info_t;

typedef struct
{
	int tl_socket;                                              // Сокет модуля интерфейсов
	int busy;                                                   // Флаг занятости
	uint8_t prev_opcode;                                        // Предыдущая операция
	uint8_t repeat_pckt_num;                                    // Количество повторных отправок подряд
	uint16_t current_pckt;                                      // Текущий пакет
	tl_info_t rx_file;                                          // Данные для приема
	tl_info_t tx_file;                                          // Данные для передачи
	tl_device_status_t tl_device_status;                        // Статус режима работы
	tl_process_t tl_process_status;                             // Статус процесса
	tl_message_t tl_buf;                                        // Данные для приема/передачи
} tl_object_t;



tl_process_t tl_task(tl_object_t* tl_object);
uint8_t tl_busy_status(tl_object_t* tl_object);
void tl_send(tl_object_t *tl_object, uint8_t* file, size_t file_size);
void tl_init(tl_object_t* tl_object, int socket, uint16_t repeat_timeout, uint16_t reset_timeout);
void tl_set_rx_file(tl_object_t* tl_object, uint8_t* file, uint16_t file_size);
uint16_t tl_get_rx_msg_size(tl_object_t* tl_object);
tl_process_t tl_process_status(tl_object_t* tl_object);
void tl_reset_rx_msg_size(tl_object_t* tl_object);



#endif //TRANSPORT_LEVEL_PROTOCOL_H
