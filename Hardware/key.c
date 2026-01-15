#include "key.h"
#include "include.h"
#include "user_config.h"

// 检测按键的引脚，配置为输入上拉
void key_init(void)
{
    // P16 P17
    
    P1_MD1 &= ~GPIO_P16_MODE_SEL(0x03);
    P1_PU |= GPIO_P16_PULL_UP(0x01);

    P1_MD1 &= ~GPIO_P17_MODE_SEL(0x03);
    P1_PU |= GPIO_P17_PULL_UP(0x01);
}

void key_scan(void)
{
    
}