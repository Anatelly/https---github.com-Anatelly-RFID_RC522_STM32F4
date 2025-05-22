#ifndef __LOW_LEVEL_H
#define __LOW_LEVEL_H


#define PIN_UART2_RX                    A,3,L,ALT_OUTPUT_PUSH_PULL,SPEED_50MHZ           // uart RX
#define PIN_UART2_TX                    A,2,L,ALT_OUTPUT_PUSH_PULL,SPEED_50MHZ    // uart debug     TX 115200-8-n-1

// Светодиоды
#define PIN_BLINK_GREEN_LED     	    A,5,L,OUTPUT_PUSH_PULL,SPEED_2MHZ

//кнопка
#define PIN_BUTTON               	    C,13,L,INPUT_FLOATING,SPEED_2MHZ

// SPI
#define PIN_SPI_SCK                     B,10,L,ALT_OUTPUT_PUSH_PULL,SPEED_100MHZ
#define PIN_SPI_MISO                    C,2,H, ALT_OUTPUT_PUSH_PULL,SPEED_100MHZ
#define PIN_SPI_MOSI                    C,3,L,ALT_OUTPUT_PUSH_PULL,SPEED_100MHZ

#define PIN_CS     	                    B,0,L,OUTPUT_PUSH_PULL,SPEED_2MHZ //выбор slave
// управление датчиком
#define PIN_RESET     	                C,4,L,OUTPUT_PUSH_PULL,SPEED_2MHZ //сброс датчика нулем

void init_task(void);

#endif /* __LOW_LEVEL_H */