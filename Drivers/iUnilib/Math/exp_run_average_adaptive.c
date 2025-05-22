#include "exp_run_average_adaptive.h"
#include "stdlib.h"

// взято тут https://alexgyver.ru/lessons/filters/

float exp_running_average_adaptive(filter_run_avr_t *filter, float newVal)
{
    //   filter->finalValue = 0;
    float k;
    // резкость фильтра зависит от модуля разности значений
    if (abs(newVal - filter->finalValue) > 1.5)
        k = 0.7;
    else
        k = 0.02;

    filter->finalValue += (newVal - filter->finalValue) * k;
    return filter->finalValue;
}