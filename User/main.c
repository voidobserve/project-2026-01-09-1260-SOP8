#include "include.h"
#include "user_config.h"

#include "timer.h"
#include "task.h"
#include "pwm.h"
#include "adc.h"

void main(void)
{
    system_init();

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x01; // 清除这个寄存器的值，实现关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;

    // WDT_KEY = WDT_KEY_VAL(0xDD); //  关闭看门狗
#if USER_DEBUG_ENABLE
    debug_init();

    P1_MD1 &= ~GPIO_P14_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P14_MODE_SEL(0x01);
    FOUT_S14 = GPIO_FOUT_AF_FUNC;

    printf("sys reset\n");
#endif

    pwm_init();
    adc_init();
    key_init();

    timer0_init(); // 系统定时器初始化，需要放在初始化的最后

    pwm_channel_0_set_duty(PWM_DUTY_VAL_85_PERCENT);
    pwm_channel_1_set_duty(PWM_DUTY_VAL_85_PERCENT);
    pwm_channel_2_set_duty(PWM_DUTY_VAL_85_PERCENT);
    pwm_channel_3_set_duty(PWM_DUTY_VAL_85_PERCENT);

    while (1)
    {
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗

        task_handle();

#if 0
        if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_0))
        {
            u16 adc_val = 0;
            adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_0);
            adc_val = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_0);
            printf("adc forward 0 val: %u\n", adc_val);
        }

        if (adc_get_update_flag(ADC_CHANNEL_INDEX_REVERSE_0))
        {
            u16 adc_val = 0;
            adc_clear_update_flag(ADC_CHANNEL_INDEX_REVERSE_0);
            adc_val = adc_get_val(ADC_CHANNEL_INDEX_REVERSE_0);
            printf("adc reverse 0 val: %u\n", adc_val);
        }

        if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_1))
        {
            u16 adc_val = 0;
            adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_1);
            adc_val = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_1);
            printf("adc forward 1 val: %u\n", adc_val);
        }

        if (adc_get_update_flag(ADC_CHANNEL_INDEX_REVERSE_1))
        {
            u16 adc_val = 0;
            adc_clear_update_flag(ADC_CHANNEL_INDEX_REVERSE_1);
            adc_val = adc_get_val(ADC_CHANNEL_INDEX_REVERSE_1);
            printf("adc reverse 1 val: %u\n", adc_val);
        }
#endif

        // printf("main\n");
    }
}
