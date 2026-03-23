#include "pwm.h"
#include "include.h"
#include "user_config.h"

void pwm_init(void)
{
    // USER_TO_DO 上电之后会有脉冲，需要先设置PWM占空比为0，再使能PWM

    // M+ ON 电机正转开关
    P1_MD0 &= ~GPIO_P13_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P13_MODE_SEL(0x01);
    FOUT_S13 = GPIO_FOUT_STMR0_PWMOUT;

// M- ON 电机反转开关
#if USER_DEBUG_PIN_ENABLE
    P0_MD1 &= ~GPIO_P06_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P06_MODE_SEL(0x01);
    FOUT_S06 = GPIO_FOUT_STMR1_PWMOUT;
#else
    P1_MD0 &= ~GPIO_P11_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P11_MODE_SEL(0x01);
    FOUT_S11 = GPIO_FOUT_STMR1_PWMOUT;
#endif

    // M2+ ON 电机正转开关
    P2_MD1 &= ~GPIO_P27_MODE_SEL(0x03);
    P2_MD1 |= GPIO_P27_MODE_SEL(0x01);
    FOUT_S27 = GPIO_FOUT_STMR2_PWMOUT;

// M2- ON 电机反转开关
#if USER_DEBUG_PIN_ENABLE
    P0_MD0 &= ~GPIO_P01_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P01_MODE_SEL(0x01);
    FOUT_S01 = GPIO_FOUT_STMR3_PWMOUT;
#else
    P0_MD0 &= ~GPIO_P03_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P03_MODE_SEL(0x01);
    FOUT_S03 = GPIO_FOUT_STMR3_PWMOUT;
#endif

    // ===============================================================================
    // STIMER0 配置
    STMR0_PSC = STMR_PRESCALE_VAL(0x00);                        // 不分频
    STMR0_PRH = STMR_PRD_VAL_H((STMR0_PEROID_VAL >> 8) & 0xFF); // 周期高八位寄存器
    STMR0_PRL = STMR_PRD_VAL_L((STMR0_PEROID_VAL >> 0) & 0xFF); // 周期低八位寄存器
    STMR0_CMPAH = STMR_CMPA_VAL_H(((0) >> 8) & 0xFF);           // 比较值A点高八位寄存器
    STMR0_CMPAL = STMR_CMPA_VAL_L(((0) >> 0) & 0xFF);           // 比较值A点低八位寄存器
    STMR_PWMVALA &= ~STMR_0_PWMVALA(0x1);                       // 计数CNT小于比较值A,PWM输出1,大于等于输出0
    STMR_PWMEN |= STMR_0_PWM_EN(0x1);                           // PWM输出使能
    STMR_CNTMD |= STMR_0_CNT_MODE(0x1);                         // 选择连续计数模式
    STMR_LOADEN |= STMR_0_LOAD_EN(0x1);                         // 自动装载使能
    STMR_CNTCLR |= STMR_0_CNT_CLR(0x1);                         // 计数清零
    STMR_CNTEN |= STMR_0_CNT_EN(0x1);                           // 计数使能
    // ===============================================================================
    // STIMER1 配置
    STMR1_PSC = STMR_PRESCALE_VAL(0x00);                        // 不分频
    STMR1_PRH = STMR_PRD_VAL_H((STMR1_PEROID_VAL >> 8) & 0xFF); // 周期高八位寄存器
    STMR1_PRL = STMR_PRD_VAL_L((STMR1_PEROID_VAL >> 0) & 0xFF); // 周期低八位寄存器
    STMR1_CMPAH = STMR_CMPA_VAL_H(((0) >> 8) & 0xFF);           // 比较值A点高八位寄存器
    STMR1_CMPAL = STMR_CMPA_VAL_L(((0) >> 0) & 0xFF);           // 比较值A点低八位寄存器
    STMR_PWMVALA &= ~STMR_1_PWMVALA(0x1);                       // 计数CNT小于比较值A,PWM输出1,大于等于输出0
    STMR_PWMEN |= STMR_1_PWM_EN(0x1);                           // PWM输出使能
    STMR_CNTMD |= STMR_1_CNT_MODE(0x1);                         // 选择连续计数模式
    STMR_LOADEN |= STMR_1_LOAD_EN(0x1);                         // 自动装载使能
    STMR_CNTCLR |= STMR_1_CNT_CLR(0x1);                         // 计数清零
    STMR_CNTEN |= STMR_1_CNT_EN(0x1);                           // 计数使能

    // ===============================================================================
    // STIMER2 配置
    STMR2_PSC = STMR_PRESCALE_VAL(0x00);                        // 不分频
    STMR2_PRH = STMR_PRD_VAL_H((STMR2_PEROID_VAL >> 8) & 0xFF); // 周期高八位寄存器
    STMR2_PRL = STMR_PRD_VAL_L((STMR2_PEROID_VAL >> 0) & 0xFF); // 周期低八位寄存器
    STMR2_CMPAH = STMR_CMPA_VAL_H(((0) >> 8) & 0xFF);           // 比较值A点高八位寄存器
    STMR2_CMPAL = STMR_CMPA_VAL_L(((0) >> 0) & 0xFF);           // 比较值A点低八位寄存器
    STMR_PWMVALA &= ~STMR_2_PWMVALA(0x1);                       // 计数CNT小于比较值A,PWM输出1,大于等于输出0
    STMR_PWMEN |= STMR_2_PWM_EN(0x1);                           // PWM输出使能
    STMR_CNTMD |= STMR_2_CNT_MODE(0x1);                         // 选择连续计数模式
    STMR_LOADEN |= STMR_2_LOAD_EN(0x1);                         // 自动装载使能
    STMR_CNTCLR |= STMR_2_CNT_CLR(0x1);                         // 计数清零
    STMR_CNTEN |= STMR_2_CNT_EN(0x1);                           // 计数使能

    // ===============================================================================
    // STIMER3 配置
    STMR3_PSC = STMR_PRESCALE_VAL(0x00);                        // 不分频
    STMR3_PRH = STMR_PRD_VAL_H((STMR3_PEROID_VAL >> 8) & 0xFF); // 周期高八位寄存器
    STMR3_PRL = STMR_PRD_VAL_L((STMR3_PEROID_VAL >> 0) & 0xFF); // 周期低八位寄存器
    STMR3_CMPAH = STMR_CMPA_VAL_H(((0) >> 8) & 0xFF);           // 比较值A点高八位寄存器
    STMR3_CMPAL = STMR_CMPA_VAL_L(((0) >> 0) & 0xFF);           // 比较值A点低八位寄存器
    STMR_PWMVALA &= ~STMR_3_PWMVALA(0x1);                       // 计数CNT小于比较值A,PWM输出1,大于等于输出0
    STMR_PWMEN |= STMR_3_PWM_EN(0x1);                           // PWM输出使能
    STMR_CNTMD |= STMR_3_CNT_MODE(0x1);                         // 选择连续计数模式
    STMR_LOADEN |= STMR_3_LOAD_EN(0x1);                         // 自动装载使能
    STMR_CNTCLR |= STMR_3_CNT_CLR(0x1);                         // 计数清零
    STMR_CNTEN |= STMR_3_CNT_EN(0x1);                           // 计数使能
}

// 设置通道0的占空比
void pwm_channel_0_set_duty(u16 channel_duty)
{
    STMR0_CMPAH = STMR_CMPA_VAL_H(((channel_duty) >> 8) & 0xFF); // 比较值
    STMR0_CMPAL = STMR_CMPA_VAL_L(((channel_duty) >> 0) & 0xFF); // 比较值
    STMR_LOADEN |= STMR_0_LOAD_EN(0x1);                          // 自动装载使能
}

// 设置通道1的占空比
void pwm_channel_1_set_duty(u16 channel_duty)
{
    STMR1_CMPAH = STMR_CMPA_VAL_H(((channel_duty) >> 8) & 0xFF); // 比较值
    STMR1_CMPAL = STMR_CMPA_VAL_L(((channel_duty) >> 0) & 0xFF); // 比较值
    STMR_LOADEN |= STMR_1_LOAD_EN(0x1);                          // 自动装载使能
}

// 设置通道2的占空比
void pwm_channel_2_set_duty(u16 channel_duty)
{
    STMR2_CMPAH = STMR_CMPA_VAL_H(((channel_duty) >> 8) & 0xFF); // 比较值
    STMR2_CMPAL = STMR_CMPA_VAL_L(((channel_duty) >> 0) & 0xFF); // 比较值
    STMR_LOADEN |= STMR_2_LOAD_EN(0x1);                          // 自动装载使能
}

// 设置通道3的占空比
void pwm_channel_3_set_duty(u16 channel_duty)
{
    STMR3_CMPAH = STMR_CMPA_VAL_H(((channel_duty) >> 8) & 0xFF); // 比较值
    STMR3_CMPAL = STMR_CMPA_VAL_L(((channel_duty) >> 0) & 0xFF); // 比较值
    STMR_LOADEN |= STMR_3_LOAD_EN(0x1);                          // 自动装载使能
}
