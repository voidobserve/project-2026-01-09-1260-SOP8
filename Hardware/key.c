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
            motor_set_dir(&motor_handle_0, MOTOR_DIR_NONE);
            // printf("key0 level == 1\n");
            // printf("toggle\n");
        }
        else
        {
            // 下降沿
            // printf("key0 level == 0\n");
        }
    }
    else // last == U8_MAX_VAL
    {
        // 第一次上电，忽略
        // printf("key0 last == U8_MAX_VAL\n");
    }

    last = curr;
}

void key1_scan_handle(void)
{
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
            motor_set_dir(&motor_handle_1, MOTOR_DIR_NONE);

            // printf("key1 level == 1\n");
        }
        else
        {
            // 下降沿
            // printf("key1 level == 0\n");
        }
    }
    else // last == U8_MAX_VAL
    {
        // 第一次上电，忽略

        // printf("key1 last == U8_MAX_VAL\n");
    }

    last = curr;
}