#ifndef _SOFT_I2C_H_
#define _SOFT_I2C_H_



void soft_i2c_init (void);         // Инициализация шины
void soft_i2c_start (void);        // Генерация условия старт
void soft_i2c_stop (void) ;        // Генерация условия стоп

void soft_i2c_send_byte (uint8_t data);       //Передать байт (вх. аргумент передаваемый байт)
uint8_t soft_i2c_get_byte  (unsigned char ack);  //Принять байт (если последний байт то входной аргумент = 1, если будем считывать еще то 0)(возвращает принятый байт)

uint8_t soft_i2c_wait_ack(); // Ожидать АСК от slave, возращает 1 если нет АСК и 0 в случае успеха
void soft_i2c_ack(void);   // Сгенерировать АСК
void soft_i2c_nack(void);  // Сгенерировать NACK

#endif

/* Пример использования:

I2C для магнитного компаса HMC6352
#define SDA_OUT                     E,15,L,OUTPUT_PUSH_PULL,SPEED_2MHZ
#define SCL_OUT                     E,14,L,OUTPUT_PUSH_PULL,SPEED_2MHZ

#define SDA_IN                      E,15,H,INPUT_PULL_UP,SPEED_2MHZ
#define SCL_IN                      E,14,H,INPUT_PULL_UP,SPEED_2MHZ

uint16_t hmc6352()
{
    uint16_t data;
    
    soft_i2c_init();
    soft_i2c_start();
    soft_i2c_send_byte(0x42 | 0); // отправляем адрес(7 бит) и указание, что следующий байт будет для "записи" 
    soft_i2c_wait_ack();          // ждём подтверждения от датчика
    soft_i2c_send_byte(0x41);     // отправляем команду "А" = Get data
    soft_i2c_wait_ack();          // ждём подтверждения от датчика
    
    soft_i2c_start();              // 
    soft_i2c_send_byte((0x43) | 1); // отправляем адрес 
    soft_i2c_wait_ack();

    data = soft_i2c_get_byte(0)<<8;
    data |= soft_i2c_get_byte(1);
    soft_i2c_stop();

    return data/10;
}
*/