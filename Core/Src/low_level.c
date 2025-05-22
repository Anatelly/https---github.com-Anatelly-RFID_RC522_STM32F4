#include "include.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void initUart2 (void);

void init_task(void)
{
    HAL_Init();

    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI2_Init();
    interface_init();
    initUart2();
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
 void SystemClock_Config(void)
 {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 72;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
      Error_Handler();
    }
 }
 
 static void MX_SPI2_Init(void)
 {
    __HAL_RCC_SPI2_CLK_ENABLE();

    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 10;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Instance = SPI2;
    HAL_SPI_Init(&hspi2);
    __HAL_SPI_ENABLE(&hspi2);
 }
 
 
 static void MX_GPIO_Init(void)
 {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  pin_init(PIN_UART2_RX);
  pin_init(PIN_UART2_TX);
  pin_init_af(PIN_UART2_TX, GPIO_AF7_USART2);
  pin_init_af(PIN_UART2_RX, GPIO_AF7_USART2);

  pin_init(PIN_SPI_SCK);
  pin_init(PIN_SPI_MISO);
  pin_init(PIN_SPI_MOSI);
  pin_init_af(PIN_SPI_SCK, GPIO_AF5_SPI2);
  pin_init_af(PIN_SPI_MISO, GPIO_AF5_SPI2);
  pin_init_af(PIN_SPI_MOSI, GPIO_AF5_SPI2);
 
  pin_init(PIN_RESET);
  pin_init(PIN_CS);
  pin_clr(PIN_CS); //выбираем slave

  pin_init(PIN_BLINK_GREEN_LED);
  pin_init(PIN_BUTTON);
 }
 
 static void initUart2 (void)
 {
     // Инитим сам уарт
     sck_2 = open("uart2", 0);
     if (sck_2 < 0) {
         Error_Handler();
     }
 
     struct termios settings;
     tcgetattr(sck_2, &settings);
     tcsetiospeed(&settings, B9600); //скорость 115200 бит/c
     tcsetattr(sck_2, 0, &settings);
 }