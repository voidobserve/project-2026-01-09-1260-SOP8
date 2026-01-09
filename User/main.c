#include "include.h"
#include "user_config.h"

#include "pwm.h"

void main(void)
{

    system_init();
    
    pwm_init();
    
    // ZD_GPIO_Init();

#if USER_DEBUG_ENABLE
    debug_init();

    P1_MD1 &= ~GPIO_P14_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P14_MODE_SEL(0x01);
    FOUT_S14 = GPIO_FOUT_AF_FUNC;

    printf("sys init\n");
#endif

    timer0_init(); // 系统定时器初始化

    pwm_channel_0_set_duty(PWM_DUTY_VAL_85_PERCENT);
    pwm_channel_1_set_duty(PWM_DUTY_VAL_85_PERCENT);
    pwm_channel_2_set_duty(PWM_DUTY_VAL_85_PERCENT);
    pwm_channel_3_set_duty(PWM_DUTY_VAL_85_PERCENT);

    while (1)
    {

        WDT_KEY = WDT_KEY_VAL(0xAA); // 关闭看门狗

        task_handle();
    }
}
