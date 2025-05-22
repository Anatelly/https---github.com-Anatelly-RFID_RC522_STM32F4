//
// Created by dizar on 25.04.2023.
//

#include "crc7.h"

static uint8_t crc7_table[256];
static uint8_t table_ready = 0;
static const uint8_t crc7_poly = 0b10001001;                                // G(x) = x^7 + x^3 + x^0

static void crc7_generate_table (void)
{
    int i, j;

    for (i = 0; i < 256; i++) {
        crc7_table[i] = (i & 0x80) ? i ^ crc7_poly : i;
        for (j = 1; j < 8; j++) {
            crc7_table[i] <<= 1;
            if (crc7_table[i] & 0x80) {
                crc7_table[i] ^= crc7_poly;
            }
        }
    }

    table_ready = 1;
}

static uint8_t crc7_add (uint8_t crc, uint8_t byte)
{
    return crc7_table[(crc << 1) ^ byte];
}


/*!
 * @brief Функция расчета контрольной суммы по алгоритмку CRC7
 * @param data - указатель на начало данных
 * @param len - длина данных
 * @return возвращает посчитанную контрольную сумму CRC7
 */
uint8_t crc7_calc (uint8_t *data, size_t len)
{
    uint8_t crc = 0;

    if (!table_ready) {
        crc7_generate_table();
    }

    for (int i = 0; i < len; i++) {
        crc = crc7_add(crc, data[i]);
    }

    return crc;
}
