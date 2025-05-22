#ifndef _VAR_H_
#define _VAR_H_

#ifdef MAIN
int sck_2;
SPI_HandleTypeDef hspi2;
TIM_HandleTypeDef htim3;
RFID_522_struct_t rfid;
Lock_state_t lock_state;

#else
extern int sck_2;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim3;
extern RFID_522_struct_t rfid;
extern uint8_t Lock_state;
extern Lock_state_t lock_state;

#endif /* MAIN */

#endif /* _VAR_H_ */