#include "include.h"

static void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim);
static void MX_TIM3_Init(void);
static uint8_t Lock_check_password(void);
static void Lock_close(void);
static void Lock_open(void);
static void Lock_led(void);

void Lock_init(void)
{
    pin_init(PIN_R_EN);
    pin_init(PIN_L_EN);
    pin_init(PIN_POWER);

    MX_TIM3_Init();
}

static void MX_TIM3_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC;

  __HAL_RCC_TIM3_CLK_ENABLE();

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 9; //1 кГц на ШИМ
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP; //счет вверх
  htim3.Init.Period = 7199; //1 кГц на ШИМ
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; //доп деление 1
  HAL_TIM_PWM_Init(&htim3);

  sConfigOC.OCMode = TIM_OCMODE_PWM1; //режим ШИМ1
  sConfigOC.Pulse = 0; //Коэф заполнения 0 (нет сигнала)
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH; //При совпадении регистров CCRx и CNT выходной сигнал комплементарного канала устанавливается высоким
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE; //быстрый режим отключен
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);

  HAL_TIM_MspPostInit(&htim3);

}

static void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(htim->Instance==TIM3)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM3 GPIO Configuration
    PA6     ------> TIM3_CH1
    PA7     ------> TIM3_CH2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }

}

//========================================================================================================
/*!
 * \brief Функция запускает нужный канал ШИМ и открывает замок
 */
static void Lock_open(void)
{
    pin_set(PIN_POWER);
    pin_set(PIN_L_EN);
    pin_set(PIN_R_EN);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 65535); //максимальный ШИМ (100%) на канале 1
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0); //нет сигнала на канале 2

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //запуск 3ого таймера в режиме ШИМ на канал 1
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //запуск 3ого таймера в режиме ШИМ на канал 2
}

/*!
 * \brief Функция выключает ШИМ и закрывает замок
 */
static void Lock_close(void)
{
    pin_clr(PIN_POWER);
    pin_clr(PIN_L_EN);
    pin_clr(PIN_R_EN);

    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1); //остановка 3ого таймера в режиме ШИМ на канал 1
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2); //остановка 3ого таймера в режиме ШИМ на канал 2
}

static uint8_t Lock_check_password(void)
{
  uint8_t key[6] = {0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF};
  uint8_t password[16] = {
    0x11, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77, 0x88,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };
  if(RFID_getUID(rfid.uid) == MI_OK) {
    MFRC522_SelectTag(rfid.uid);
    if(RFID_ReadBlock(0x01, rfid.buff, key, rfid.uid) == MI_OK) {
      if(!memcmp(rfid.buff, password, 16)) //если пароль совпал - возвращаем ок
        return MI_OK;
    }
  } 
  return MI_ERR;
}

static void Lock_led(void)
{
    pin_set(PIN_BLINK_GREEN_LED);
    delay_ms(100);
    pin_clr(PIN_BLINK_GREEN_LED);
}

/*!
 * \brief Функция изменения ключа сектора 0 и его параметров.
 */
void Lock_change_key(void)
{
  uint8_t KeyData[16] = {
    0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,   //Key A
    0x7F, 0x07,  0x88,                    //Access Bits
    0xFF,                                 //User byte
    0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF    //Key B
  };
  if(RFID_getUID(rfid.uid) == MI_OK) {
    if(RFID_ChangeKey(0x03, rfid.defkey, KeyData, rfid.uid) == MI_OK) //меняем ключ первого сектора
      Lock_led();
  }
  RFID_close();
}

//========================================================================================================
/*!
 * \brief Функция записи пароля в блок 1, ключ к сектору: 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
 */
void Lock_write_password(void)
{
  uint8_t key[6] = {0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF};
  uint8_t password[16] = {
    0x11, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77, 0x88,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  if(RFID_getUID(rfid.uid) == MI_OK) {
    MFRC522_SelectTag(rfid.uid);
    if(RFID_WriteBlock(0x01, password, key, rfid.uid) == MI_OK)
      Lock_led();
  } 
  RFID_close();
}


void Lock_task(void)
{
    if(Lock_check_password() == MI_OK) {
        switch(lock_state) {   
            case state_close:
                //если было закрыто - открываем
                Lock_open();
                lock_state = state_open;
                break;
            case state_open:
                //если было открыто - закрываем
                Lock_close();
                lock_state = state_close;
                break;
        }
        Lock_led();
    }
    RFID_close();
}