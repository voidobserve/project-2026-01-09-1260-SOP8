#include "key.h"
#include "include.h"
#include "user_config.h"
#include "motor_handle.h"

// 检测按键的引脚，配置为输入上拉
void key_init(void)
{
    // SW1
    P1_MD1 &= ~GPIO_P16_MODE_SEL(0x03);
    P1_PU |= GPIO_P16_PULL_UP(0x01);

    // SW2
    P0_MD0 &= ~GPIO_P02_MODE_SEL(0x03);
    P0_PU |= GPIO_P02_PULL_UP(0x01);
}

void key0_scan_handle(void)
{
    // 使用边沿检测
    u8 curr = P31;
    static u8 last = U8_MAX_VAL;

    if (curr != last)
    {
        if (last != (u8)-1)
        {
            if (curr)
            {
                // 上升沿

                motor_0_status = MOTOR_FORWARD;
                g_delay_change = change_delay_time;

                forward_pwm(0);
                backward_pwm(0);

                allow_change = 1;
                allow_check = 0;
            }
            else
            {
                // 下降沿

                // 电机不转
            }
        }
        else
        {
            // 第一次上电，忽略
            motorState = MOTOR_STOP;
        }

        last = curr;
    }
}