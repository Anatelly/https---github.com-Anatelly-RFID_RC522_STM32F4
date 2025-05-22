/**
******************************************************************************
* @file tl_protocol.с
* @author Дружинин А.А.
* @version v1.0
* @date  14-08-2022
* @brief Транспортный уровень передачи данных.
* ****************************************************************************
*
* Для использования протокола необходимо:
* 1) Вызвать функцию tl_init, в которую передать структуру протокола, а также следующие настройки:
*		- декриптор используемого интерфейса;
*		- время повтора сообщения передатчиком при отсутствии сообщения от приемника;
*		- время, через которое приемник сбрасывает информации, если передатчик долго не выходил на связь;
* 2) Установить файл для приема (Если устройство будет только передатчиком, то этот пункт можно опустить) через функцию tl_set_rx_file;
* 3) Отправка сообщения производится функцией tl_send;
* 4) Необходимо переодически вести опрос протокола функцией tl_task.
* 
* Вспомогательные функции:
* 1) Для контроля статуса передачи есть функцию tl_process_status, которая возвращает следующие статусы:
* 	TL_ERROR                        =  0,                      // Ошибка
*	TL_LOST_CONNECT                 =  1,                      // Потеря связи
*	TL_PROCESS_SEND                 =  2,                      // Идет передача
*	TL_PROCESS_END                  =  3,                      // Передача завершена
*	TL_EMPTY                        =  4,                      // Нет сообщения
*	TL_REPEAT                       =  5,                      // Произошел повтор сообщения
*	TL_PROCESS_END_MEM              =  6,                      // Передача завершена из-за нехватки места в буфере для приема
*
* 2) Для проверки занятости протокола есть функция tl_busy_status
* 3) Для получении информации о принятой длине пакета необходимо вызвать tl_get_rx_msg_size
* 4) Для сброса длины полученного сообщения есть функция tl_reset_rx_msg_size, ее необходимо вызывать после прочтения, 
* чтобы не было возможности прочитать одно сообщение несколько раз
******************************************************************************
* @attention
* Протокол используется в связке с модулем интерфейсов
*
* <h2><center>&copy; COPYRIGHT STC </center></h2>
******************************************************************************
*/
#include "tl_protocol.h"
#include "low_level.h"

static void tl_gen_header(tl_message_t* msg, tl_opcode_t opcode, uint16_t num_pckt, uint16_t msg_len, uint8_t* prev_opcode);
static void tl_reset_object(tl_object_t* tl_object);
static uint8_t tl_process_data(tl_object_t* tl_object, tl_message_t* in_msg, tl_opcode_t success_msg);
static void tl_repeat_pckt(tl_object_t* tl_object);
static tl_process_t tl_protocol_parser(tl_message_t* in_msg, tl_object_t* tl_object);
static tl_opcode_t tl_write_data(tl_object_t* tl_object);
static uint8_t tl_process_rx_pckt(tl_message_t* in_msg);
static void tl_info_msg(tl_object_t* tl_object, tl_opcode_t opcode);
static void tl_tx_repeat_pckt(tl_object_t* tl_object);
static void tl_rx_repeat_pckt(tl_object_t* tl_object);
static void tl_protocol_tx_parser(tl_message_t* in_msg, tl_object_t* tl_object);
static void tl_protocol_rx_parser(tl_message_t* in_msg, tl_object_t* tl_object);

/**
* @brief Инициализация приемника/передатчика
* @param[in] tl_object - указатель на структуру интерфейса
* @param[in] socket - номер дескриптора открытого интерфейса
* @param[in] timeout - время, через которое отправляется повторное сообщение
*/
void tl_init(tl_object_t* tl_object, int socket, uint16_t repeat_timeout, uint16_t reset_timeout)
{
	crc32sftwr_init();
	tl_object->tl_socket = socket;
	tl_object->tx_file.time_info.timeout = repeat_timeout;
	software_timer_stop(&tl_object->tx_file.time_info.timer);
	tl_object->rx_file.time_info.timeout = reset_timeout;
	software_timer_stop(&tl_object->rx_file.time_info.timer);
	tl_object->rx_file.data_file = NULL;
	tl_object->tx_file.data_file = NULL;
	tl_reset_object(tl_object);
}

/**
* @brief Получение длины принятого сообщения
* @param[in] tl_object - указатель на структуру интерфейса
* @return Длину принятого сообщения
*/
uint16_t tl_get_rx_msg_size(tl_object_t* tl_object)
{
	return tl_object->rx_file.result_offset;
}

/**
* @brief Сброс длины полученного сообщения
* @param[in] tl_object - указатель на структуру интерфейса
* @return void
*/
void tl_reset_rx_msg_size(tl_object_t* tl_object)
{
	tl_object->rx_file.result_offset = 0U;
}

/**
* @brief Проверка ведет ли устройство прием/передачу
* @param[in] tl_object - указатель на структуру интерфейса
* @return Статус устройства:\n
* true - ведется прием/передача\n
* false - устройство свободно\n
*/
uint8_t tl_busy_status(tl_object_t* tl_object)
{
	return tl_object->busy;
}


/**
* @brief Проверка статуса передачи
* @param[in] tl_object - указатель на структуру интерфейса
* @return Статус устройства процесса:\n
*/
tl_process_t tl_process_status(tl_object_t* tl_object)
{
	return tl_object->tl_process_status;
}

/**
* @brief Обработка сообщения которое пришло принимающему устройству
* @param[in] in_msg - указатель на поступившее сообщение
* @param[in] tl_object - указатель на структуру интерфейса
* @return void
*/
static void tl_protocol_rx_parser(tl_message_t* in_msg, tl_object_t* tl_object)
{
	// Обрабатываем сообщение
	switch (in_msg->opcode) {

	// Пришел запрос на подключение для отправки данных
	case TL_CONNECTION_REQUEST:
	{
		// Сбрасываем все, что насчитали, потому что передача могла оборваться, а у нас выставлен офсет и тд
		tl_reset_object(tl_object);

		tl_object->busy = 1;
		tl_object->current_pckt = 1U;
		// Отправляет подтверждение
		tl_info_msg(tl_object, TL_SEND_DATA_CONFIRM);

		software_timer_start(&tl_object->rx_file.time_info.timer, tl_object->rx_file.time_info.timeout);

		tl_object->tl_process_status = TL_PROCESS_SEND;
	}

	break;

		// Пришла очередная посылка данных
	case TL_SEND_DATA:
	{
		if(tl_process_data(tl_object, in_msg, TL_SEND_DATA_CONFIRM) == 0)
		{
			tl_object->tl_process_status = TL_PROCESS_END_MEM;
		}
		else
		{
			tl_object->tl_process_status = TL_PROCESS_SEND;
		}
	}

	break;

		// Пришла последняя посылка данных
	case TL_SEND_STOP:
	{
		if(tl_process_data(tl_object, in_msg, TL_SEND_STOP_CONFIRM) == 0)
		{
			tl_object->tl_process_status = TL_PROCESS_END_MEM;
		}
		else
		{
			// Останавливаем таймер приемника
			software_timer_stop(&tl_object->rx_file.time_info.timer);
			// Сбрасываем флаг занятости
			tl_object->busy = 0;
			tl_object->tl_process_status = TL_PROCESS_END;
		}
	}

	break;

		// Ошибка передачи
	case TL_ERROR_ACK:
	{
		tl_repeat_pckt(tl_object);

		tl_object->tl_process_status = TL_ERROR;
	}

	break;


	default:
	{
		// Останавливаем таймер приемника
		software_timer_stop(&tl_object->rx_file.time_info.timer);

		// Сбрасываем информацию
		tl_reset_object(tl_object);

		tl_object->tl_process_status = TL_ERROR;
	}

	break;
	}
}

/**
* @brief Обработка сообщения которое пришло передающему устройству
* @param[in] in_msg - указатель на поступившее сообщение
* @param[in] tl_object - указатель на структуру интерфейса
* @return void
*/
static void tl_protocol_tx_parser(tl_message_t* in_msg, tl_object_t* tl_object)
{
	// Обрабатываем сообщение
	switch (in_msg->opcode) {

	// Пришло подтверждение полученных данных
	case TL_SEND_DATA_CONFIRM:
	{
		// Отправляем пакет данных, если нет ошибок
		if (tl_write_data(tl_object) == TL_ERROR_ACK)
		{
			tl_info_msg(tl_object, TL_ERROR_ACK);

			tl_object->tl_process_status = TL_ERROR;
		}
		else
		{
			tl_object->tl_process_status = TL_PROCESS_SEND;
		}
	}
	break;

		// Подтверждение, что последняя посылка получена
	case TL_SEND_STOP_CONFIRM:
	{
		// Останавливаем таймер передатчика
		software_timer_stop(&tl_object->tx_file.time_info.timer);

		// Сбрасываем информацию
		tl_reset_object(tl_object);

		tl_object->tl_process_status = TL_PROCESS_END;
	}

	break;

		// Ошибка передачи
	case TL_ERROR_ACK:
	{
		tl_repeat_pckt(tl_object);

		tl_object->tl_process_status = TL_ERROR;
	}

	break;

	case TL_ERROR_MEMORY:
	{
		// Останавливаем таймер передатчика
		software_timer_stop(&tl_object->tx_file.time_info.timer);

		// Сбрасываем информацию
		tl_reset_object(tl_object);

		tl_object->tl_process_status = TL_PROCESS_END_MEM;
	}

	break;

	default:
	{
		// Останавливаем таймер приемника
		software_timer_stop(&tl_object->tx_file.time_info.timer);

		// Сбрасываем информацию
		tl_reset_object(tl_object);

		tl_object->tl_process_status = TL_ERROR;
	}
	break;

	}
}

/**
* @brief Обработка сообщения
* @param[in] in_msg - указатель на поступившее сообщение
* @param[in] tl_object - указатель на структуру интерфейса
* @return Статус передачи:\n
* TL_ERROR - ошибка передачи\n
* TL_PROCESS_SEND - идет передача\n
* TL_PROCESS_END - передача завершена\n
*/
static tl_process_t tl_protocol_parser(tl_message_t* in_msg, tl_object_t* tl_object)
{
	// Проверяем, что сообщение транспортного уровня протокола
	if (in_msg->prefix == TL_PREFIX)
	{
		// Проверяем совпала ли контрольная сумма
		if (tl_process_rx_pckt(in_msg) == 0)
		{
			tl_info_msg(tl_object, TL_ERROR_ACK);

			tl_object->tl_process_status = TL_ERROR;

			return TL_ERROR;
		}

		// Если работаем в режиме передатчика и это не сообщение о нехватке памяти или об ошибке передачи
		if(tl_object->tl_device_status == TL_TX)
		{
			// Сбрасываем таймеры повторной отправки
			software_timer_start(&tl_object->tx_file.time_info.timer, tl_object->tx_file.time_info.timeout);

			// Если номера пакетов не сходятся и это не сообщения об ошибке, значит это не наше сообщение, не отвечаем
			if((tl_object->current_pckt != in_msg->current_pckt)  && (in_msg->opcode != TL_ERROR_MEMORY) && (in_msg->opcode != TL_ERROR_ACK))
			{
				tl_object->tl_process_status = TL_EMPTY;

				return TL_EMPTY;
			}

			tl_protocol_tx_parser(in_msg, tl_object);
		}

		// Если работаем в режиме приемника и это не запрос на отправку или сообщение об ошибке передачи
		else if(tl_object->tl_device_status == TL_RX)
		{
			if((in_msg->opcode != TL_CONNECTION_REQUEST) && (in_msg->opcode != TL_ERROR_ACK))
			{
				// Если пакет с таким номером уже был отправлен, просто повторяем сообщение
				if(in_msg->current_pckt == tl_object->current_pckt)
				{
					tl_repeat_pckt(tl_object);

					tl_object->tl_process_status = TL_REPEAT;

					return TL_REPEAT;
				}

				// Если номера пакетов отличаются не на 1, значит это не наше сообщение, игнорируем
				else if((in_msg->current_pckt - tl_object->current_pckt) != 1)
				{
					tl_object->tl_process_status = TL_EMPTY;

					return TL_EMPTY;
				}
			}

			tl_protocol_rx_parser(in_msg, tl_object);
		}

		return tl_object->tl_process_status;
	}

	tl_object->tl_process_status = TL_EMPTY;

	return tl_object->tl_process_status;
}

/**
* @brief Формирование заголовка
* @param[in] msg - указатель на сообщение для отправки
* @param[in] opcode - код операции:\n
* TL_CONNECTION_REQUEST - Запрос на обмен данными\n
* TL_SEND_DATA - Отправка данных\n
* TL_SEND_DATA_CONFIRM - Получение данных подтверждено\n
* TL_SEND_STOP - Окончена передача данных\n
* TL_SEND_STOP_CONFIRM - Подтверждение, что информация об окончании получена\n
* TL_ERROR_ACK - Неверные данные
* @param[in] num_pckt - номер пакета
* @param[in] msg_len - длина без учета заголовка
*/
static void tl_gen_header(tl_message_t* msg, tl_opcode_t opcode, uint16_t num_pckt, uint16_t msg_len, uint8_t* prev_opcode)
{
	msg->prefix = TL_PREFIX;
	msg->opcode = opcode;
	msg->current_pckt = num_pckt;
	msg->msg_len = msg_len;
	msg->tl_crc = crc32_sftwr(0U, &msg->prefix, TL_HEAD_SIZE-TL_CRC_SIZE);

	*prev_opcode = opcode;

	if(msg->msg_len > 0)
	{
		msg->tl_crc = crc32_sftwr(msg->tl_crc, msg->msg, msg->msg_len);
	}
}

/**
* @brief Сброс объекта к моменту перед стартом
* @param[in] tl_object - указатель на структуру интерфейса
*
*/
static void tl_reset_object(tl_object_t* tl_object)
{
	if(tl_object->tl_device_status == TL_RX)
	{
		tl_object->rx_file.result_offset = 0U;
	}
	else if(tl_object->tl_device_status == TL_TX)
	{
		tl_object->tx_file.result_offset = 0U;
	}
	tl_object->prev_opcode = 0U;
	tl_object->busy = 0;
	tl_object->current_pckt = 0U;
	tl_object->repeat_pckt_num = 0U;
	tl_object->tl_device_status = TL_RX;
	tl_object->tl_process_status = TL_EMPTY;
}

/**
* @brief Обработка пакета данных
* @param[in] tl_object - указатель на структуру интерфейса
* @param[in] in_msg - указатель на принятое сообщение
* @param[in] success_msg - сообщение, которое отправится в случае успешной обработки
*/
static uint8_t tl_process_data(tl_object_t* tl_object, tl_message_t* in_msg, tl_opcode_t success_msg)
{
	// Если не хватает буфера для приема, то завершаем процесс
	if((tl_object->rx_file.result_offset + in_msg->msg_len) > tl_object->rx_file.file_size)
	{
		tl_info_msg(tl_object, TL_ERROR_MEMORY);
		// Сбрасываем информацию
		tl_reset_object(tl_object);

		// Останавливаем таймер приемника
		software_timer_stop(&tl_object->rx_file.time_info.timer);

		return 0U;
	}

	pbf_gluing_data(tl_object->rx_file.data_file, &tl_object->rx_file.result_offset, in_msg->msg, in_msg->msg_len);
	tl_object->current_pckt++;


	// Если ошибки нет, отправляем подтверждение о получении
	tl_info_msg(tl_object, success_msg);

	return 1U;
}

/**
* @brief Формирование заголовка
* @param[in] opcode - код операции:\n
* TL_CONNECTION_REQUEST - Запрос на обмен данными\n
* TL_SEND_DATA - Отправка данных\n
* TL_SEND_DATA_CONFIRM - Получение данных подтверждено\n
* TL_SEND_STOP - Окончена передача данных\n
* TL_SEND_STOP_CONFIRM - Подтверждение, что информация об окончании получена\n
* TL_ERROR_ACK - Неверные данные
* @param[in] tl_object - указатель на структуру интерфейса
*
*/
static void tl_info_msg(tl_object_t* tl_object, tl_opcode_t opcode)
{
	tl_gen_header(&tl_object->tl_buf, opcode, tl_object->current_pckt, 0U, &tl_object->prev_opcode);

	write(tl_object->tl_socket, (char*)&tl_object->tl_buf, TL_HEAD_SIZE);
}


/**
* @brief Установка файла для передачи
* @param[in] tl_object - указатель на структуру интерфейса
* @param[in] file - указатель на данные, которые надо передать
* @param[in] file_size - размер данных для передачи
*/
void tl_send(tl_object_t *tl_object, uint8_t* file, size_t file_size)
{
	// Переводим устройство в режим передачи
	tl_object->tl_device_status = TL_TX;
	tl_object->current_pckt = 1U;
	tl_object->busy = 1;
	tl_object->tx_file.result_offset = 0U;
	tl_object->repeat_pckt_num = 0U;
	tl_object->tx_file.data_file = file;
	tl_object->tx_file.file_size = file_size;
	tl_gen_header(&tl_object->tl_buf, TL_CONNECTION_REQUEST, tl_object->current_pckt, 0U, &tl_object->prev_opcode);

    switchToTx();
	write(tl_object->tl_socket, (char*)&tl_object->tl_buf, TL_HEAD_SIZE);

	software_timer_start(&tl_object->tx_file.time_info.timer, tl_object->tx_file.time_info.timeout);
}


/**
* @brief Установка буфера, в который будет сохраняться передаваемый файл
* @param[in] tl_object - указатель на структуру интерфейса
* @param[in] buffer_data - указатель на буфер
*/
void tl_set_rx_file(tl_object_t* tl_object, uint8_t* file, uint16_t file_size)
{
	tl_object->rx_file.data_file = file;
	tl_object->rx_file.file_size = file_size;
}

/**
* @brief Отправка куска данных
* @param[in] tl_object - указатель на структуру интерфейса
* @param[in] in_msg - указатель на полученное сообщение
*/
static tl_opcode_t tl_write_data(tl_object_t* tl_object)
{
	tl_opcode_t answer_opcode;
	size_t packet_size = 0U;

	// выделяем кусок для передачи
	packet_size = pbf_allocate_data(tl_object->tx_file.data_file,
									&tl_object->tx_file.file_size, &tl_object->tx_file.result_offset,
									tl_object->tl_buf.msg, TL_MESSAGE_DATA_SIZE);

	if(packet_size > 0U)
	{
		if(tl_object->tx_file.file_size > 0)
		{
			answer_opcode = TL_SEND_DATA;
		}
		else
		{
			answer_opcode = TL_SEND_STOP;
		}
		tl_object->current_pckt++;
	}
	else
	{
		return TL_ERROR_ACK;
	}

	tl_gen_header(&tl_object->tl_buf, answer_opcode, tl_object->current_pckt, packet_size, &tl_object->prev_opcode);

	write(tl_object->tl_socket, (char*)&tl_object->tl_buf, packet_size + TL_HEAD_SIZE);

	return answer_opcode;
}

/**
* @brief Повторная отправка сообщения от приемника
* @param[in] tl_object - указатель на структуру интерфейса
*/
static void tl_rx_repeat_pckt(tl_object_t* tl_object)
{
	tl_info_msg(tl_object, tl_object->prev_opcode);
}

/**
* @brief Повторная отправка сообщения от передатчика
* @param[in] tl_object - указатель на структуру интерфейса
*/
static void tl_tx_repeat_pckt(tl_object_t* tl_object)
{
	switch (tl_object->prev_opcode) {

	case TL_CONNECTION_REQUEST:
	{
		tl_info_msg(tl_object, TL_CONNECTION_REQUEST);
	}
	break;
	case TL_SEND_DATA:
	{
		tl_object->current_pckt --;
		tl_object->tx_file.result_offset -= TL_MESSAGE_DATA_SIZE;
		tl_write_data(tl_object);
	}
	break;
	case TL_SEND_STOP:
	{
		tl_object->current_pckt --;
		tl_object->tx_file.file_size = (tl_object->tx_file.result_offset);
		tl_object->tx_file.result_offset = (tl_object->current_pckt - 1)*TL_MESSAGE_DATA_SIZE;
		tl_write_data(tl_object);
	}
	break;
	case TL_ERROR_ACK:
	{
		tl_info_msg(tl_object, TL_ERROR_ACK);
	}
	break;
	default:
		break;
	}
}

/**
* @brief Повторная отправка сообщения
* @param[in] tl_object - указатель на структуру интерфейса
*/
static void tl_repeat_pckt(tl_object_t* tl_object)
{
	if(tl_object->tl_device_status == TL_TX)
	{
		tl_object->repeat_pckt_num++;
		tl_tx_repeat_pckt(tl_object);
	}
	else if(tl_object->tl_device_status == TL_RX)
	{
		tl_rx_repeat_pckt(tl_object);
	}
}

/**
* @brief Обработка принятного сообщения
* @param[in] in_msg - указатель на полученное сообщение
*/
static uint8_t tl_process_rx_pckt(tl_message_t* in_msg)
{
	uint32_t tl_crc = crc32_sftwr(0U, &in_msg->prefix, TL_HEAD_SIZE-TL_CRC_SIZE);

	if(in_msg->msg_len > 0)
	{
		tl_crc = crc32_sftwr(tl_crc, in_msg->msg, in_msg->msg_len);
	}

	if(tl_crc == in_msg->tl_crc)
	{
		return 1U;
	}

	return 0U;
}

/**
* @brief Таска для модуля
* @param[in] tl_object - указатель на структуру интерфейса
* @return
* TL_ERROR - Ошибка\n
* TL_LOST_CONNECT - Потеря связи\n
* TL_PROCESS_SEND - Идет передача\n
* TL_PROCESS_END - Передача завершена\n
* TL_EMPTY - Нет сообщения\n
* TL_PROCESS_END_MEM - Передача завершена из-за нехватки места в буфере для приема
*/
tl_process_t tl_task(tl_object_t* tl_object)
{
	// Проверяем не было прошло ли время потери связи
	if((tl_object->repeat_pckt_num == TL_LOST_NUM) && (tl_object->tl_device_status == TL_TX))
	{
		// Если работаем в режиме передатчика
		// Останавливаем таймер повторной отправки
		software_timer_stop(&tl_object->tx_file.time_info.timer);

		// Сбрасываем информацию
		tl_reset_object(tl_object);

		tl_object->tl_process_status = TL_LOST_CONNECT;

		return TL_LOST_CONNECT;
	}

	static uint16_t msg_offset = 0U;

	uint8_t byte = 0U;

	size_t res = read(tl_object->tl_socket, (char*)&byte, 1U);

	if(res > 0U)
	{
		// Если прочитанный байт равен префиксу протокола, значит это новое сообщение
		if(byte == TL_PREFIX)
		{
			msg_offset = 1U;

			tl_object->tl_buf.prefix = byte;
		}

		else
		{
			*((uint8_t*)&tl_object->tl_buf + msg_offset) = byte;
			msg_offset++;
		}

		res = read(tl_object->tl_socket, ((char*)&tl_object->tl_buf + msg_offset), (TL_BUFFER_SIZE-msg_offset));

		// Если длина сообщения не совпала с принятым количеством, значит ждем оставшуюся часть
		if((tl_object->tl_buf.msg_len + TL_HEAD_SIZE) > (res + msg_offset))
		{
			msg_offset += res;

			tl_object->tl_process_status = TL_EMPTY;

			return TL_EMPTY;
		}
		// Длина совпала с тем, что прочитали до этого
		else if((tl_object->tl_buf.msg_len + TL_HEAD_SIZE) == (res + msg_offset))
		{
			msg_offset = 0U;

			return tl_protocol_parser((tl_message_t *)&tl_object->tl_buf, tl_object);
		}
		else
		{
			msg_offset = 0U;

			tl_object->tl_process_status = TL_EMPTY;

			return TL_EMPTY;
		}
	}

	// Если работаем в режиме передачи и истек таймер для повтора сообщения
	else if(tl_object->tl_device_status == TL_TX && software_timer(&tl_object->tx_file.time_info.timer))
	{
		tl_repeat_pckt(tl_object);
		software_timer_start(&tl_object->tx_file.time_info.timer, tl_object->tx_file.time_info.timeout);

		tl_object->tl_process_status = TL_REPEAT;

		return TL_REPEAT;
	}

	// Если работаем в режиме передачи и истек таймер для ожидания сообщения, сбрасываем информацию
	else if(tl_object->tl_device_status == TL_RX && software_timer(&tl_object->rx_file.time_info.timer))
	{
		tl_reset_object(tl_object);
		software_timer_stop(&tl_object->rx_file.time_info.timer);

		tl_object->tl_process_status = TL_LOST_CONNECT;

		return TL_LOST_CONNECT;
	}

	tl_object->tl_process_status = TL_EMPTY;

	return TL_EMPTY;
}
