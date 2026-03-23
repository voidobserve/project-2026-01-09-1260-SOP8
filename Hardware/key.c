#include "key.h"
#include "include.h"
#include "user_config.h"
#include "motor_handle.h"
#include "pwm.h"

// 检测按键的引脚，配置为输入上拉
void key_init(void)
{
    // SW1
    P1_PU |= GPIO_P16_PULL_UP(0x01);
    P1_MD1 &= ~GPIO_P16_MODE_SEL(0x03); // 输入

    // SW2
    P0_PU |= GPIO_P02_PULL_UP(0x01);
    P0_MD0 &= ~GPIO_P02_MODE_SEL(0x03); // 输入
}

void key0_scan_handle(void)
{
#if 0
    // 使用边沿检测
    volatile u8 curr = KEY_0_PIN;         // 存放当前检测到的电平
    static volatile u8 last = U8_MAX_VAL; // 存放上一次检测到的电平

    if (curr == last)
    {
        return;
    }

    if (last != U8_MAX_VAL)
    {
        if (curr)
        {
            // 上升沿
            motor_0_status = MOTOR_STATUS_FORWARD;
            motor_0_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;

            MOTOR_0_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
            MOTOR_0_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);

            motor_0_dir_change_enable = 1;
            motor_0_is_working = 0;

            // printf("key0 curr == 1\n");
        }
        else
        {
            // 下降沿

            // 电机不转

            // printf("key0 curr == 0\n");
        }
    }
    else // last == U8_MAX_VAL
    {
        // 第一次上电，忽略
        motor_0_status = MOTOR_STATUS_STOP;

        // printf("key0 last == U8_MAX_VAL\n");
    }

    last = curr;
#endif
}

void key1_scan_handle(void)
{
#if 0
    // 使用边沿检测
    volatile u8 curr = KEY_1_PIN;         // 存放当前检测到的电平
    static volatile u8 last = U8_MAX_VAL; // 存放上一次检测到的电平

    if (curr == last)
    {
        return;
    }

    if (last != U8_MAX_VAL)
    {
        if (curr)
        {
            // 上升沿
            motor_1_status = MOTOR_STATUS_FORWARD;
            motor_1_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;

            MOTOR_1_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
            MOTOR_1_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);

            motor_1_dir_change_enable = 1;
            motor_1_is_working = 0;

            // printf("key1 curr == 1\n");
        }
        else
        {
            // 下降沿

            // 电机不转

            // printf("key1 curr == 0\n");
        }
    }
    else // last == U8_MAX_VAL
    {
        // 第一次上电，忽略
        motor_1_status = MOTOR_STATUS_STOP;

        // printf("key1 last == U8_MAX_VAL\n");
    }

    last = curr;
#endif
}