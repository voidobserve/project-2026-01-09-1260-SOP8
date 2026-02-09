#include "include.h"
#include "user_config.h"

#include "timer.h"
#include "task.h"
#include "pwm.h"
#include "adc.h"
#include "key.h"
#include "uart.h"

void main(void)
{
    system_init();

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x01; // 清除这个寄存器的值，实现关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;

    // WDT_KEY = WDT_KEY_VAL(0xDD); //  关闭看门狗
    uart0_init();

    pwm_init();
    adc_init();
    key_init();

    timer0_init(); // 系统定时器初始化，需要放在初始化的最后
#if USER_DEBUG_ENABLE
    printf("sys reset\n");
#endif

    // USER_TO_DO 测试时使用
    // pwm_channel_0_set_duty(PWM_DUTY_VAL_85_PERCENT);
    // pwm_channel_1_set_duty(PWM_DUTY_VAL_85_PERCENT);
    // pwm_channel_2_set_duty(PWM_DUTY_VAL_85_PERCENT);
    // pwm_channel_3_set_duty(PWM_DUTY_VAL_85_PERCENT);

    while (1)
    {
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗

        task_handle();

        // printf("main\n");
    }
}
