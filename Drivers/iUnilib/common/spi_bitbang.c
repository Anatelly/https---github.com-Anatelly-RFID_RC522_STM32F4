#include "include.h"

//=============================================================================
void spi_init(void)
{
	spi_deselect_all();

	SPI_SCK(1);
}

//=============================================================================
// With SPI Modes 0 and 3, data is always latched in on the rising edge of SCK and always output on the falling edge of SCK
//=============================================================================
uint_fast8_t spi_transfer(uint_fast8_t data)
{
	for (int_fast8_t i=0; i<8; i++)
	{
		SPI_SCK(0);

		SPI_MOSI(data & (1<<7));        // write data at the begin of cycle 
		spi_delay();

		SPI_SCK(1);
		spi_delay();
        data = (data<<1) | SPI_MISO();  // read data at the end of cycle 
    }

    return (data & 0xff);               // clr possible garbage
}        

//=============================================================================
void* spi_transfer_block(void* ptr, size_t qty)
{
	void*    start = ptr;
	uint8_t* array = ptr;
	
	while (qty--)
	{
		uint8_t x = array[0];

		*array++ = spi_transfer(x);
	}

	return (start);
}

//=============================================================================
void* spi_read_block(void* dst, size_t qty)
{
	void*    start = dst;
	uint8_t* array = dst;
	
	while (qty--)
	{
		*array++ = spi_transfer(0);
	}

	return (start);
}

//=============================================================================
void spi_write_block(const void* src, size_t qty)
{
	const uint8_t* array = src;

	while (qty--)
	{
		spi_transfer(*array++);
	}
}


