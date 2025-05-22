
#include "include.h"

/** 
* @brief  Функция задержки в микросекундах us
*/
__STATIC_INLINE void Delay_us(uint32_t __IO us) 
{
    us *= (SystemCoreClock / 1000000) / 5;
    while (us--)
        ;
}

/** 
* @brief  функция инициализации. Всё притягиваем к 0.
*/
void soft_i2c_init(void)
{
    pin_init(SDA_OUT); // притянуть SDA (лог.0)
    pin_clr(SDA_OUT);
    Delay_us(100);

    pin_init(SCL_OUT); // притянуть SCL (лог.0)
    pin_clr(SCL_OUT);
    Delay_us(100);
}

/** 
* @brief  функция генерации условия "старт"
*/
void soft_i2c_start(void)
{
    pin_init(SDA_IN); // отпустить SDA (лог.1)
    Delay_us(100);
    pin_init(SCL_IN); // отпустить SCL (лог.1)
    Delay_us(100);

    pin_init(SDA_OUT); // притянуть SDA (лог.0)
    pin_clr(SDA_OUT);
    Delay_us(100);

    pin_init(SCL_OUT); // притянуть SCL (лог.0)
    pin_clr(SCL_OUT);
    Delay_us(100);
}

/** 
* @brief  функция генерации условия "стоп"
*/
void soft_i2c_stop(void) 
{
    pin_init(SCL_OUT); // притянуть SCL (лог.0)
    pin_clr(SCL_OUT);
    pin_init(SDA_OUT); // притянуть SDA (лог.0)
    pin_clr(SDA_OUT);
    Delay_us(100);

    pin_init(SCL_IN); // отпустить SCL (лог.1)
    Delay_us(10);
    pin_init(SDA_IN); // отпустить SDA (лог.1)
    Delay_us(100);
}

/** 
* @brief Функция ожидания ACK от slave
* @return 1 - АСК получено, 0 - NACK
*/
uint8_t soft_i2c_wait_ack()
{
    uint8_t count;
    pin_init(SDA_IN); // отпустить SDA (лог.1)
    Delay_us(100);
    pin_init(SCL_IN); // отпустить SCL (лог.1)
    Delay_us(100);

    while (pin_test(SDA_IN))
    {
        count++;
        if (count > 250)
        {
            soft_i2c_stop;
            return 0;
        }
    }

    pin_init(SCL_OUT); // притянуть SCL (лог.0)
    pin_clr(SCL_OUT);

    return 1;
}

/** 
* @brief Функция генерации ACK от мастера
*/
void soft_i2c_ack(void)
{
    pin_init(SCL_OUT); // притянуть SCL (лог.0)
    pin_clr(SCL_OUT);
    pin_init(SDA_OUT); // притянуть SDA (лог.0)
    pin_clr(SDA_OUT);
    Delay_us(100);
    pin_init(SCL_IN); // отпустить SCL (лог.1)
    Delay_us(100);
    pin_init(SCL_OUT); // притянуть SCL (лог.0)
    pin_clr(SCL_OUT);
}

/** 
* @brief Функция генерации NACK от мастера
*/
void soft_i2c_nack(void)
{
    pin_init(SCL_OUT); // притянуть SCL (лог.0)
    pin_clr(SCL_OUT);
    pin_init(SDA_IN); // притянуть SDA (лог.1)
    Delay_us(100);
    pin_init(SCL_IN); // отпустить SCL (лог.1)
    Delay_us(100);
    pin_init(SCL_OUT); // притянуть SCL (лог.0)
    pin_clr(SCL_OUT);
}
/** 
* @brief Функция отправки 1 байта
* @param data отправляемый байт
*/
void soft_i2c_send_byte(uint8_t data)
{
    uint8_t i;

    pin_init(SCL_OUT); // притянуть SCL (лог.0)
    pin_clr(SCL_OUT);

    for (i = 0; i < 8; i++)
    {
        if ((data & 0x80) >> 7 == 1)
        {
            pin_init(SDA_IN); // лог.1
        }
        else
        {
            pin_init(SDA_OUT); // Выставить бит на SDA (лог.0)
            pin_clr(SDA_OUT);
        }
        Delay_us(100);
        pin_init(SCL_IN); // отпустить SCL (лог.1)
        Delay_us(100);
        pin_init(SCL_OUT); // притянуть SCL (лог.0)
        pin_clr(SCL_OUT);
        data <<= 1; // сдвигаем на 1 бит влево
    }
}

/** 
* @brief Функция принятия 1 байта
* @param ack если принимаем последний байт то генерируем АСК = 1, если будем считывать ещё байт то АСК = 0
* @return uint8_t - принятый байт
*/
uint8_t soft_i2c_get_byte(unsigned char ack) // функция принятия байта
{
    unsigned char inData = 0, i;

    for (i = 0; i < 8; i++)
    {
        pin_init(SCL_OUT); // притянуть SCL (лог.0)
        pin_clr(SCL_OUT);
        Delay_us(100);
        pin_init(SCL_IN); // отпустить SCL (лог.1)
        Delay_us(100);

        inData <<= 1;
        if (pin_test(SDA_IN) == 1)
        {
            inData++;
        }
    }
    if(ack == 1) soft_i2c_ack;
    else soft_i2c_nack;

    return inData;
}