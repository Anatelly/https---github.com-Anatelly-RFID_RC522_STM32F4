#ifndef _ADCLIB_H
#define _ADCLIB_H

#include "global_macro.h"

#if F1_CHECK
#include "stm32f1xx_hal.h"
#elif F3_CHECK
#include "stm32f3xx_hal.h"
#elif  F4_CHECK
#include "stm32f4xx_hal.h"
#endif /* MCU CHECKER */

#define INJECTED_CHANNEL_MAX								3

void ADC_DMAChannelInit (ADC_TypeDef* ADCx, uint16_t *pointer, uint8_t chNum, ...);
uint32_t* ADC_InjectedAddToRotation (ADC_TypeDef* ADCx, uint32_t channel, uint32_t convClocks);

#endif
