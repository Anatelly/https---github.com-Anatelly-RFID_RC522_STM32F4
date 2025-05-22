#ifndef PROCESS_BIG_FILESIZE_H
#define PROCESS_BIG_FILESIZE_H

#include <stdint.h>
#include <stddef.h>
#include "string.h"

size_t pbf_allocate_data(uint8_t* file, uint16_t* file_size, uint16_t* result_offset, uint8_t* current_msg, uint16_t max_pckt_size);

void pbf_gluing_data(uint8_t* file, uint16_t* result_offset, uint8_t* current_msg, uint16_t pckt_size);


#endif //PROCESS_BIG_FILESIZE_H
