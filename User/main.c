#include "include.h"

void main(void)
{

    system_init();
    // debug_init();

    ZD_TIMER_Init();
    ZD_GPIO_Init();
    // ZD_ADC_Init();

    // printf("start code \n");

    while (1)
    {

        WDT_KEY = WDT_KEY_VAL(0xAA); // 关闭看门狗

        TaskHandler();
    }
}
