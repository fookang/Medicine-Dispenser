#ifndef __UART__
#define __UART__

#include "constant.h"

/*
 * Set baud rate to 9600
 * UART2 Tx on PTE22, Rx on PTE23
 * NVIC priority for UART2 is 128
 */
#define BAUD_RATE 9600
#define UART_TX 22
#define UART_RX 23
#define UART2_INT_PRIO 128


void init_uart();
void uart_send(const TPacket *packet);

#endif
