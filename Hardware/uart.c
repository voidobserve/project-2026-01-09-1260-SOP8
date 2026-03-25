#include "uart.h"
#include "user_config.h"

volatile uart_rx_buffer_t uart0_rx_buffer= {0};

#if USER_DEBUG_ENABLE
// 重写puchar()函数
char putchar(char c)
{
    while (!(UART0_STA & UART_TX_DONE(0x01)))
        ;
    UART0_DATA = c;
    return c;
}
#endif

void uart0_init(void)
{
#if USER_DEBUG_PIN_ENABLE
    // P26 tx
    P2_MD1 &= ~GPIO_P26_MODE_SEL(0x03); // 清空对应的寄存器配置
    P2_MD1 |= GPIO_P26_MODE_SEL(0x01);  // 配置为 输出模式
    FOUT_S26 = GPIO_FOUT_UART0_TX;      // 输出功能选择
#else
    P1_MD1 &= ~GPIO_P15_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P15_MODE_SEL(0x01);
    FOUT_S15 = GPIO_FOUT_UART0_TX;
#endif

    // P14 rx
    P1_MD1 &= ~GPIO_P14_MODE_SEL(0x03); // 清空对应的寄存器配置，对应输入模式
    FIN_S7 = GPIO_FIN_SEL_P14;          // 选择 uart0 rx 对应的引脚

    UART0_BAUD1 = (((SYSCLK - UART0_BAUD_RATE) / UART0_BAUD_RATE) >> 8) & 0xFF;
    UART0_BAUD0 = ((SYSCLK - UART0_BAUD_RATE) / UART0_BAUD_RATE) & 0xFF;
    UART0_CON0 = UART_STOP_BIT(0x00) |  // 0x00：一位停止位
                 UART_RX_IRQ_EN(0x01) | // 0x01：rx中断使能
                 UART_EN(0x01);         // UART 使能

    __EnableIRQ(UART0_IRQn); // 打开模块中断
    IE_EA = 1;               // 打开总中断
}

// UART0发送一个字节数据的函数
void uart0_sendbyte(u8 byte)
{ 
    while (!(UART0_STA & UART_TX_DONE(0x01))) // 等待上一次发送完成
        ;
    // IE_EA = 0; // 关闭总中断
    UART0_DATA = byte;
    // IE_EA = 1;
    while (!(UART0_STA & UART_TX_DONE(0x01))) // 等待这次发送完成
        ;
}

// void uart0_sendbuf(u8 *buf, u8 len)
// {
//     u8 i = 0;
//     for (; i < len; i++) 
//     {
//         uart0_sendbyte(buf[i]);
//     }
// }

// 获取接收缓冲区中有效的数据个数，单位：字节Byte
u8 uart0_rxbuffer_get_count(void)
{
    return uart0_rx_buffer.count;
}

// 从接收缓冲区中取出一个字节数据
u8 uart0_rxbuffer_get_byte(void)
{
    u8 rxbyte;

    if (0 == uart0_rx_buffer.count)
    {
        // 缓冲区空
        return 0;
    }

    // 先偏移索引，再取出数据
    uart0_rx_buffer.tail = (uart0_rx_buffer.tail + 1) % UART_RX_BUF_SIZE;
    rxbyte = uart0_rx_buffer.buffer[uart0_rx_buffer.tail];

    uart0_rx_buffer.count--;

    return rxbyte;
}

void uart0_rxbuffer_put_byte(u8 byte)
{
    // 目前的逻辑：缓冲区满，覆盖旧的数据

    // 先偏移索引，再存放数据
    uart0_rx_buffer.head = (uart0_rx_buffer.head + 1) % UART_RX_BUF_SIZE;
    uart0_rx_buffer.buffer[uart0_rx_buffer.head] = byte;

    uart0_rx_buffer.count++;

    if (uart0_rx_buffer.count > UART_RX_BUF_SIZE)
    {
        uart0_rx_buffer.count = UART_RX_BUF_SIZE;
    }
}

// void uart0_handle(void)
// {
//     u8 recvbyte;
//     if (uart0_rxbuffer_get_count() == 0)
//     {
//         return;
//     }

//     // 接收缓冲区中有数据
//     recvbyte = uart0_rxbuffer_get_byte();
// }

// UART0中断服务函数（接收中断）
void UART0_IRQHandler(void) interrupt UART0_IRQn
{
    // u8 uart_data = 0;

    // 进入中断设置IP，不可删除
    __IRQnIPnPush(UART0_IRQn);
    // ---------------- 用户函数处理 -------------------
    // RX接收完成中断
    if (UART0_STA & UART_RX_DONE(0x1))
    {
        // uart_data = UART0_DATA; // 临时存放接收到的数据
        uart0_rxbuffer_put_byte(UART0_DATA); 
    }
    // 退出中断设置IP，不可删除
    __IRQnIPnPop(UART0_IRQn);
}
