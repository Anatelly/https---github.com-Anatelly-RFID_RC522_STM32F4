#ifndef _SPI_BITBANG_CONF_H_
#define _SPI_BITBANG_CONF_H_

#include "low_level.h"

#define SPI_SCK(X)      do {if (X) pin_set(SCK);   else pin_clr(SCK);}   while (0)
#define SPI_MOSI(X)     do {if (X) pin_set(SDI);  else pin_clr(SDI);}  while (0)
#define SPI_MISO()      pin_test(SDO)


#define SPI_MEM_CS(X) do {if (X) pin_set(MEM_CS);  else pin_clr(MEM_CS);}  while (0)
#define SPI_MPU_CS(X) do {if (X) pin_set(MPU_CS);  else pin_clr(MPU_CS);}  while (0)
#define SPI_ADC_CS(X) do {if (X) pin_set(ADC_CS);  else pin_clr(ADC_CS);}  while (0)


//=============================================================================
// select one devise of:
//   SPI_VN100
//   SPI_MEM
//=============================================================================
#define spi_select(SPI_DEVICE) \
do                             \
{                              \
	SPI_DEVICE##_CS(0);        \
}                              \
while (0)                      \

//=============================================================================
// deselect one devise of:
//   SPI_VN100
//   SPI_MEM
//=============================================================================
#define spi_deselect(SPI_DEVICE) \
do                               \
{                                \
	SPI_DEVICE##_CS(1);          \
}                                \
while (0)                        \


//=============================================================================
// deselect all devices
//=============================================================================
static __inline void spi_deselect_all(void)
{
	spi_deselect(SPI_MEM);
	spi_deselect(SPI_MPU);
	spi_deselect(SPI_ADC);
}

//=============================================================================
static __inline void spi_delay(void)	  // 50 ns
{
	// 50 * F_CPU / 3000000;
	nop();
	nop();
	nop();
	nop();
}


#endif
