#ifndef _EXP_RUN_AVERAGE_H_
#define _EXP_RUN_AVERAGE_H_

typedef struct
{
    float finalValue;
}filter_run_avr_t;

float exp_running_average_adaptive(filter_run_avr_t *filter, float newVal);

#endif