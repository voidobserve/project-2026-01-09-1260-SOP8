#include "uart.h"
#include "user_config.h"

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
