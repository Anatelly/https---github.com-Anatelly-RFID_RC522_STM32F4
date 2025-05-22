#include "include.h"

static void rcc_adc (ADC_TypeDef *ADCx)
{
	switch ((uint32_t)ADCx)
	{
	case (uint32_t)ADC1: __HAL_RCC_ADC1_CLK_ENABLE(); break;
#ifdef ADC2_BASE
	case (uint32_t)ADC2: __HAL_RCC_ADC2_CLK_ENABLE(); break;
#elif ADC3_BASE
	case (uint32_t)ADC3: __HAL_RCC_ADC3_CLK_ENABLE(); break;
#endif
	default: break;
	}
}

#if F1_CHECK

static void ADC_Callibration(ADC_TypeDef* ADCx);

#endif

/*!
 * \brief Функция добавляет канал в ротацию инжектированных каналов
 * И возвращает указатель на регистр, откуда можно будет забирать данные
 * \param ADC_TypeDef *ADCx - номер ADC (ADC1, ADC2, ADC3)
 * \param uint32_t channel - номер канала ADC (0, 1, 2, ..., 18)
 * \param uint32_t convClocks - количество циклов АЦП на преобразование (ADC_SAMPLETIME_480CYCLES..)
 * \return *uint32_t указатель на регистр, в котором находится результат
 */
uint32_t* ADC_InjectedAddToRotation (ADC_TypeDef* ADCx, uint32_t channel, uint32_t convClocks)
{
	uint8_t chNumCur;
	uint8_t isFirst = 0;
	uint32_t *result;
	uint32_t temp;

	rcc_adc(ADCx);

	// Считаем, сколько уже каналов используется
	chNumCur = (ADCx->JSQR & ADC_JSQR_JL) >> 20;

	// Если каналов уже 4, то возвращаем 0 и ничего больше не делаем
	if (chNumCur == INJECTED_CHANNEL_MAX) return 0;

	// Если бит ADON не стоит, то значит АЦП еще не используется и значит этот канал будет первым
	if (!(ADCx->CR2 & ADC_CR2_ADON)) isFirst = 1;

	// Выключаем АЦП
	ADCx->CR2 &=~ ADC_CR2_ADON;

	// Если мы уже не первые, то увеличиваем номер канала
	if (!isFirst)	chNumCur++;

	// Добавляем канал в ротацию
	temp = ADCx->JSQR;
	temp &=~ ADC_JSQR_JL;
	temp >>= 5;
	temp |= (channel << 15);
	temp |= chNumCur << 20;

	ADCx->JSQR = temp;

	// Выставляем время преобразования на указанный канал
	if (channel < 10) ADCx->SMPR2 |= (convClocks << (3 * channel));
	else ADCx->SMPR1 |= (convClocks << (3 * (channel - 10)));

	// Если это первый вызов - то конфигурируем АЦП в режим непрерывного преобразования в ротации инжектированных каналов
	// + запуск инжектированного канала по установке JSWSTART
	if (isFirst)
	{
		ADCx->CR1 |= ADC_CR1_SCAN | ADC_CR1_JAUTO;

		ADCx->CR2 |= ADC_CR2_CONT;
	}

	// Включаем АЦП и стартуем
	ADCx->CR2 |= ADC_CR2_ADON;

#if F1_CHECK

	if(isFirst)
	{
		ADC_Callibration(ADCx);
	}

#endif

	ADCx->CR2 |= ADC_CR2_JSWSTART | ADC_CR2_SWSTART;

	// Включаем АЦП и стартуем
	ADCx->CR2 |= ADC_CR2_ADON;

	// Получим адрес регистра, в котором у нас будет результат
	result = (uint32_t*)&(ADCx->JDR1) + chNumCur;

	return result;
}

#if F1_CHECK

static void ADC_Callibration(ADC_TypeDef* ADCx)
{
	// Сбросим калибровку
	ADCx->CR2 |= ADC_CR2_RSTCAL;
	while (ADCx->CR2 & ADC_CR2_RSTCAL)
	{
		;
	}

	// Калибруем заново
	ADCx->CR2 |= ADC_CR2_CAL;
	while (ADCx->CR2 & ADC_CR2_CAL)
	{
		;
	}
}

/*!
 * \brief Функция добавляет указанные входы АЦП в регулярные каналы и запускает конвертацию
 * \details Функция добавляет указанные входы АЦП в регулярные каналы и запускает конвертацию
 * Используется DMA для сохранения данных в памяти
 * \param ADC_TypeDef *ADCx - Номер используемого ADC
 * \param uint16_t *pointer - указатель на начало области памяти, для сохранения результатов
 * \param uint8_t chNum - количество используемых каналов (максимум 16)
 * \param ...  1) номер канала в виде обычного числа (0 - канал 0, 1 - канал 1, 13 - канал 13)
 */
void ADC_DMAChannelInit (ADC_TypeDef* ADCx, uint16_t *pointer, uint8_t chNum, ...)
{
	uint8_t n = 0;
	uint8_t *st = &chNum;
	uint32_t *p = (uint32_t*)(st)+1;
	uint8_t ch;
	uint8_t isFirst = 0;
	DMA_HandleTypeDef hDma = {0};

	// В F1 у нас только один DMA контроллер
	rcc_adc(ADCx);
	__HAL_RCC_DMA1_CLK_ENABLE();

	DMA_InitTypeDef dma_setup = {0};
	// Включаем режим сканирования и повторения + старт по установке SWSTART
	ADCx->CR1 |= ADC_CR1_SCAN;
	ADCx->CR2 |= ADC_CR2_CONT | ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;
	ADCx->CR2 |= ADC_CR2_SWSTART;
	// Если бит ADON не стоит, то значит АЦП еще не используется и значит этот канал будет первым
	if (!(ADCx->CR2 & ADC_CR2_ADON)) isFirst = 1;

	while ((n < chNum) && (n < 16))
	{
		if (p != 0)
		{
			ch = *p;

			if (n < 6) ADCx->SQR3 |= (ch << (5 * n));
			else if (n < 12) ADCx->SQR2 |= (ch << (5 * (n - 6)));
			else ADCx->SQR1 |= (ch << (5 * (n - 12)));

			if (ch < 10) ADCx->SMPR2 |= (7 << (3 * ch ));
			else ADCx->SMPR1 |= (7 << (3 * (ch - 10)));

			n++;
			p++;
		}
		else break;
	}
	ADCx->SQR1 |= (n-1) << 20;

	dma_setup.Direction = DMA_PERIPH_TO_MEMORY;
	dma_setup.PeriphInc = DMA_PINC_DISABLE;
	dma_setup.MemInc = DMA_MINC_ENABLE;
	dma_setup.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	dma_setup.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	dma_setup.Mode = DMA_CIRCULAR;
	dma_setup.Priority = DMA_PRIORITY_MEDIUM;

	hDma.Instance = DMA1_Channel1;
	hDma.Init = dma_setup;
	HAL_DMA_Init(&hDma);

	HAL_DMA_Start(&hDma, (uint32_t)&ADCx->DR, (uint32_t)pointer, n);

	//ADCx->CR2 |= ADC_CR2_DDS;
	ADCx->CR2 |= ADC_CR2_DMA;
	ADCx->CR2 |= ADC_CR2_ADON;

	if(isFirst)
	{
		ADC_Callibration(ADCx);
	}

	ADCx->CR2 |= ADC_CR2_SWSTART;
}

#elif F4_CHECK

/*!
 * \brief Функция добавляет указанные входы АЦП в регулярные каналы и запускает конвертацию
 * \details Функция добавляет указанные входы АЦП в регулярные каналы и запускает конвертацию
 * Используется DMA для сохранения данных в памяти
 * \param ADC_TypeDef *ADCx - Номер используемого ADC
 * \param uint16_t *pointer - указатель на начало области памяти, для сохранения результатов
 * \param uint8_t chNum - количесвто используемых каналов (максимум 16)
 * \param ...  1) номер канала в виде обычного числа (0 - канал 0, 1 - канал 1, 13 - канал 13)
 */
void ADC_DMAChannelInit (ADC_TypeDef* ADCx, uint16_t *pointer, uint8_t chNum, ...)
{
	uint8_t n = 0;
	uint8_t *st = &chNum;
	uint32_t *p = (uint32_t*)(st)+1;
	uint8_t ch;
	DMA_Stream_TypeDef *DMAx;
	uint32_t DMA_Channel;
	DMA_InitTypeDef sDMA = {0};
	DMA_HandleTypeDef hDMA = {0};

	// Подаем такты на АЦП и DMA
	rcc_adc(ADCx);
	__HAL_RCC_DMA2_CLK_ENABLE();

	// Включаем режим сканирования и повторения
	ADCx->CR1 |= ADC_CR1_SCAN;
	ADCx->CR2 |= ADC_CR2_CONT;

	while ((n < chNum) && (n < 16))
	{
		if (p != 0)
		{
			ch = *p;

			if (n < 6) ADCx->SQR3 |= (ch << (5 * n));
			else if (n < 12) ADCx->SQR2 |= (ch << (5 * (n - 6)));
			else ADCx->SQR1 |= (ch << (5 * (n - 12)));

			if (ch < 10) ADCx->SMPR2 |= (7 << (3 * ch ));
			else ADCx->SMPR1 |= (7 << (3 * (ch - 10)));

			n++;
			p++;
		}
		else break;
	}
	ADCx->SQR1 |= (n-1) << 20;

	// Теперь конфигурируем DMA
	if (ADCx == ADC1)
	{
		DMAx = DMA2_Stream0;
		DMA_Channel = DMA_CHANNEL_0;
	}
#ifdef ADC2_BASE
	else if (ADCx == ADC2)
	{
		DMAx = DMA2_Stream2;
		DMA_Channel = DMA_CHANNEL_1;
	}
#endif
	else
	{
		DMAx = DMA2_Stream1;
		DMA_Channel = DMA_CHANNEL_2;
	}

	sDMA.Channel = DMA_Channel;
	sDMA.Direction = DMA_PERIPH_TO_MEMORY;
	sDMA.PeriphInc = DMA_PINC_DISABLE;
	sDMA.MemInc = DMA_MINC_ENABLE;
	sDMA.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	sDMA.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	sDMA.Mode = DMA_CIRCULAR;
	sDMA.Priority = DMA_PRIORITY_MEDIUM;
	sDMA.FIFOMode = DMA_FIFOMODE_DISABLE;
	sDMA.MemBurst = DMA_MBURST_SINGLE;
	sDMA.PeriphBurst = DMA_PBURST_SINGLE;
	hDMA.Instance = DMAx;
	hDMA.Init = sDMA;
	HAL_DMA_Init(&hDMA);
	HAL_DMA_Start(&hDMA, (uint32_t)&ADCx->DR, (uint32_t)pointer, n);

	ADCx->CR2 |= ADC_CR2_DDS;
	ADCx->CR2 |= ADC_CR2_DMA;
	ADCx->CR2 |= ADC_CR2_ADON;
	ADCx->CR2 |= ADC_CR2_SWSTART;
}

#else

#error "This mcu is not supported, or select mcu type"

#endif
