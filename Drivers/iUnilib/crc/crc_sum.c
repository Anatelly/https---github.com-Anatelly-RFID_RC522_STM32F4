#include "crc_sum.h"

// суммирует байты структуры
uint8_t crc_256(const void *buf, size_t len)
{
    const uint8_t *ptr = buf;
    uint8_t crc = 0x00U;

    while (len--)
    {
        crc += *ptr++;
    }

    return (crc == 255) ? crc = 136 : crc;
}