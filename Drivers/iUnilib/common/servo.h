#ifndef __SERVO_H_
#define __SERVO_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "ap3_config.h"

//================================================================
typedef struct
{
	TIM_TypeDef*	    tim;
	volatile uint32_t*	tim_ch;
	
	uint16_t		    position_pwm_min;
	uint16_t		    position_pwm_max;
	uint16_t		    position_pwm_neitral;
	uint16_t		    angle_range;
	uint16_t		    curr_angle;
	bool    		    flagOperation;
}
servo_t;

void servo_init         (servo_t *servo, TIM_TypeDef *tim, uint8_t tim_ch, uint16_t min, uint16_t max, int16_t neitral, uint16_t range_deg, servo_position_t init_position);
void set_servo_angle    (servo_t *servo, int8_t angle);
void set_flaps_angle	(servo_t *servo, uint8_t angle);

#endif