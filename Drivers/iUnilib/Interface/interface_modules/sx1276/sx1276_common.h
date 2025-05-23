//
// Created by dizar on 30.03.2022.
//

#ifndef SX1276_COMMON_H
#define SX1276_COMMON_H

#include <stdint.h>
#include <math.h>
#include "global_macro.h"
#include "delay.h"

#if F1_CHECK
#include "stm32f1xx_hal.h"
#elif F3_CHECK
#include "stm32f3xx_hal.h"
#elif F4_CHECK
#include "stm32f4xx_hal.h"
#endif /* CHECK MACRO */

/* ======================================================================	*/
/* Общие структуры для трансивера sx1276																	*/
/* ======================================================================	*/

#define SX1276_HIGH_BAND_FREQUENCY                              779000000

typedef enum
{
    SX1276_STATE_IDLE,
    SX1276_STATE_RX,
    SX1276_STATE_TX
} sx1276_state_t;

typedef struct
{
    SPI_TypeDef *SPIx;
    GPIO_TypeDef *CS_PORT;
    uint32_t CS_Pin;
    GPIO_TypeDef *RESET_PORT;
    uint32_t Reset_Pin;
    uint32_t oscFrequency;
} sx1276_SPIControl_t;

typedef struct
{
    uint8_t inAir;
    void (*switch_to_rx)(void);
    void (*switch_to_tx)(void);
} sx1276_ll_control_func_t;



#endif //SX1276_COMMON_H
