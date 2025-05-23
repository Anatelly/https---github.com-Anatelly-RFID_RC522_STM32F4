#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global_macro.h"
#include "fifo.h"

#include "interface_conf.h"

#include "termios.h"


#ifdef INTERFACE_UART
    #include "uart_it.h"
    #include "uart_device.h"
#endif

#ifdef INTERFACE_SPI
    #include "spi_it.h"
    #include "spi_device.h"
#endif

#ifdef INTERFACE_I2C
    #include "i2c_ll.h"
    #include "i2c_device.h"
#endif

#ifdef INTERFACE_SX1276_LORA
    #include "sx1276_lora.h"
    #include "sx1276_lora_device.h"
#endif

#ifdef INTERFACE_SX1276_FSK
    #include "sx1276_fsk.h"
    #include "sx1276_fsk_device.h"
#endif

typedef enum
{
    IF_TYPES_NONE,
    IF_TYPES_UART,
    IF_TYPES_SPI,
    IF_TYPES_I2C,
    IF_TYPES_SX1276_FSK,
    IF_TYPES_SX1276_LORA
} interface_types_t;

typedef union
{
#ifdef INTERFACE_UART
    uart_t *uart;
#endif

#ifdef INTERFACE_SPI
    spi_t *spi;
#endif

#ifdef INTERFACE_I2C
    i2c_t *i2c;
#endif

#ifdef INTERFACE_SX1276_LORA
    sx1276lora_t *sx1276_lora;
#endif

#ifdef INTERFACE_SX1276_FSK
    sx1276fsk_t *sx1276_fsk;
#endif
} interface_handler_t;

typedef struct
{
    interface_handler_t handler;
    interface_types_t type;
    void* next;
    int desc;
    struct
    {
        ssize_t (*read)(char* buf, size_t length);
        ssize_t (*write)(char* buf, size_t length);
        void (*poll)(void);
    } syscalls;
    struct termios options;
} interface_t;

void interface_init (void);
int open (char* ifname, int flags);
void close (int desc);
ssize_t read (int desc, char* buffer, size_t size);
ssize_t write (int desc, char* buffer, size_t size);
int poll (int desc);

int tcgetattr (int fd, struct termios *opt);
int tcsetattr (int fd, int optional, struct termios *opt);
int tcsetiospeed (struct termios *tc, speed_t baud);

int interface_switch_set_to_tx_func (int fd, void (*switch_func)(void));
int interface_switch_set_to_rx_func (int fd, void (*switch_func)(void));
int interface_reswitch_set_to_tx_func (int fd);
int interface_reswitch_set_to_rx_func (int fd);


#endif /* _INTERFACE_H_ */
