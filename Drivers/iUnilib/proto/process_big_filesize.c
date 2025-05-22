/**
******************************************************************************
* @file process_big_filesize.с
* @author Дружинин А.А.
* @version v1.0
* @date  19-01-2021
* @brief Модуль для выделения куска данных из всего файла, а также склеивания этих данных для передачи больших файлов.
* ****************************************************************************
*
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT STC </center></h2>
******************************************************************************
*/
#include "process_big_filesize.h"

/**
* @brief Функция выделения куска данных под размер буфера
* @param[in] file - указатель на файл передачи
* @param[in] file_size - размер всего файла
* @param[in] result_offset - смещение, с которого пойдет выделения куска данных
* @param[in] current_msg - указатель на буфер для передачи
* @param[in] max_pckt_size - максимальный размер куска данных из файла, который можно передать за один раз
*/
size_t pbf_allocate_data(uint8_t* file, uint16_t* file_size, uint16_t* result_offset, uint8_t* current_msg, uint16_t max_pckt_size)
{
	size_t packet_size = 0;

	// Если разница между полной длиной файла и смещением больше, чем максимальный размер буфера, то размер пакета равен максимальному размеру
	if(((*file_size) - *result_offset) > max_pckt_size)
		packet_size = max_pckt_size;
	else
	{
		// Обнуляем буфер, чтобы не передались данные с прошлой передачи
		memset(current_msg, 0, max_pckt_size);

		// Вычисляем размер пакета
		packet_size = (*file_size) - *result_offset;

		// Если будет обращение за пределы массива
		if(packet_size < 0)
			return 0;

		*file_size = 0;
	}

	memcpy(current_msg, &file[*result_offset], packet_size);

	*result_offset += packet_size;

	return packet_size;
}

/**
* @brief Функция склеивания кусков данных в один
* @param[in] file - указатель на файл, куда помещаются склеенные данные
* @param[in] result_offset - смещение, с которого пойдет присоединение данных к файлу
* @param[in] current_msg - указатель на текущее принятое сообщение
* @param[in] pckt_size - размер текущего сообщения
*/
void pbf_gluing_data(uint8_t* file, uint16_t* result_offset, uint8_t* current_msg, uint16_t pckt_size)
{

	memcpy(&file[(*result_offset)], current_msg, pckt_size);

	(*result_offset) += pckt_size;
}