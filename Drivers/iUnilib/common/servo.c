#include "servo.h"

void servo_init(servo_t *servo, TIM_TypeDef *tim, uint8_t tim_ch, uint16_t min, uint16_t max, int16_t neitral, uint16_t range_deg, servo_position_t init_position)
{
    servo->tim = tim;
    servo->position_pwm_min = min + (neitral * 10);
    servo->position_pwm_max = max + (neitral * 10);
    servo->position_pwm_neitral = ((min + max) / 2) + (neitral * 10);
    servo->angle_range = range_deg;

    switch (tim_ch)
    {
    case 1:
        servo->tim_ch = &tim->CCR1;
        break;
    case 2:
        servo->tim_ch = &tim->CCR2;
        break;
    case 3:
        servo->tim_ch = &tim->CCR3;
        break;
    case 4:
        servo->tim_ch = &tim->CCR4;
        break;

    default:
        break;
    };

    servo->flagOperation = true; // разрешаем рулить сервами

    switch (init_position)
    {
    case SERVO_POS_OPEN:
        *servo->tim_ch = servo->position_pwm_max;
        break;

    case SERVO_POS_CLOSE:
        *servo->tim_ch = servo->position_pwm_min;
        break;

    case SERVO_POS_NEITRAL:
        *servo->tim_ch = servo->position_pwm_neitral;
        break;
    }
    // Вычисляем текущий угол поворота сервы
    servo->curr_angle = 0;
}

void set_servo_angle(servo_t *servo, int8_t angle)
{
    uint16_t pulse_length;

    pulse_length = servo->position_pwm_neitral;
    pulse_length += ((servo->position_pwm_max - servo->position_pwm_min) / servo->angle_range) * angle;

    *servo->tim_ch = pulse_length;
}

/**
 * @brief отдельная функция для отклонения закрылок, так как они работают в одну стороны от начального положения
 */
void set_flaps_angle(servo_t *servo, uint8_t angle)
{
    uint16_t pulse_length;

    if (angle >= 0 && angle <= servo->angle_range)
    {
        pulse_length = servo->position_pwm_min;
        pulse_length += ((servo->position_pwm_max - servo->position_pwm_min) / servo->angle_range) * angle;
    }
    *servo->tim_ch = pulse_length;
}