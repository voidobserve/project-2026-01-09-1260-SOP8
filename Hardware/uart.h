#ifndef __UART_H__
#define __UART_H__

#include "include.h"

#define UART0_BAUD_RATE ((u32)115200) // 波特率
#define UART_RX_BUF_SIZE 128

// 增加环形缓冲区结构体
typedef struct
{
	u8 buffer[UART_RX_BUF_SIZE];
	u8 head;
	u8 tail;
	u8 count;
} uart_rx_buffer_t;

void uart0_init(void);

void uart0_sendbyte(u8 byte);
// void uart0_sendbuf(u8 *buf, u8 len);

u8 uart0_rxbuffer_get_count(void);
u8 uart0_rxbuffer_get_byte(void);
void uart0_rxbuffer_put_byte(u8 byte);

#endif
