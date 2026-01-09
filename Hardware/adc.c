#include "adc.h"

// 存放采集到的ad值
volatile u16 adc_val_forward_0; // 电机0 正转 ad值
volatile u16 adc_val_reverse_0; // 电机0 反转 ad值
volatile u16 adc_val_forward_1; // 电机1 正转 ad值
volatile u16 adc_val_reverse_1; // 电机1 反转 ad值

void adc_init(void)
{
    // ADC配置
    ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) | ADC_EXREF_SEL(0x1)); // 关闭外部参考电压
    ADC_ACON1 |= ADC_VREF_SEL(0x5) |                        // 选择 4.2V 作为参考电压
                 ADC_TEN_SEL(0x3);                          // 关闭测试信号

    ADC_ACON0 = ADC_CMP_EN(0x1) |  // 打开ADC中的CMP使能信号
                ADC_BIAS_EN(0x1) | // 打开ADC偏置电流能使信号
                ADC_BIAS_SEL(0x1); // 打开 ADC偏置电流

#if 0                
    ADC_CHS0 = ADC_ANALOG_CHAN(0x05) | // P02通路     选择对应的引脚配置
               ADC_EXT_SEL(0x0);       // 选择外部通路
    ADC_CFG0 |= ADC_CHAN0_EN(0x1) |    // 使能通道0转换
                ADC_EN(0x1);           // 使能A/D转换

    ADC_CHS1 = ADC_ANALOG_CHAN(0x12) | // P05通路     选择对应的引脚配置
               ADC_EXT_SEL(0x0);       // 选择外部通路
    ADC_CFG0 |= ADC_CHAN1_EN(0x1) |    // 使能通道1转换
                ADC_EN(0x1);           // 使能A/D转换
#endif

    __EnableIRQ(ADC_IRQn); // 使能ADC中断
    IE_EA = 1;             // 使能总中断
    // MARK: - USER_TO_DO
    // 需要检查一下对应的寄存器配置 ADC_CFG1
    ADC_CFG1 |= (0x0F << 3) | // ADC时钟分频为16分频，为系统时钟/16
                (0x01 << 0);  // ADC0 通道中断使能

    delay_ms(1); // 等待ADC模块配置稳定，需要等待20us以上
}

/**
 * @brief 切换 adc0 的通道
 * @attention 函数内部没有延时等待adc稳定，调用时需要注意
 *
 * @param adc_channel
 */
/*
    MARK: - USER_TO_DO
    需要补充对应的引脚配置
*/
static void adc0_sel_channel(u8 adc_channel)
{
    switch (adc_channel)
    {
    case ADC_CHANNEL_FORWARD:
    {
    }
    break;
    // =======================================================
    case ADC_CHANNEL_REVERSE:
    {
    }
    break;
    // =======================================================
    default:
    {
    }
    break;
    }

    ADC_CFG0 |= ADC_CHAN0_EN(0x1) | // 使能通道0
                ADC_EN(0x1);        // 使能adc
}

/**
 * @brief 切换 adc1 的通道
 * @attention 函数内部没有延时等待adc稳定，调用时需要注意
 *
 * @param adc_channel
 */
/*
    MARK: - USER_TO_DO
    需要补充对应的引脚配置
*/
static void adc1_sel_channel(u8 adc_channel)
{
    switch (adc_channel)
    {
    case ADC_CHANNEL_FORWARD:
    {
    }
    break;
    // =======================================================
    case ADC_CHANNEL_REVERSE:
    {
    }
    break;
    // =======================================================
    default:
    {
    }
    break;
    }

    ADC_CFG0 |= ADC_CHAN1_EN(0x1) | // 使能通道0
                ADC_EN(0x1);        // 使能adc
}

/**
 * @brief adc扫描函数，包含轮询、顺序切换通道和启动转换
 * @note 在adc中断服务函数中接收adc值
 * @attention 函数内部没有延时等待adc稳定，调用该函数的时间间隔至少要20us，一般是用1ms
 *
 */
void adc_scan(void)
{
    static volatile u8 adc0_status;
    static volatile u8 adc1_status;

    if (adc0_status == ADC_STATUS_NONE || adc0_status == ADC_STATUS_SEL_GET_REVERSE)
    {
        // 切换ad通道
        adc0_sel_channel(ADC_CHANNEL_FORWARD);
        adc0_status = ADC_STATUS_SEL_GET_FORWARD_WAITING;
    }
    else if (adc0_status == ADC_STATUS_SEL_GET_FORWARD_WAITING)
    {
        // 开启转换，之后在ad中断获取ad值
        ADC_CFG0 |= 0x01 << 0; // 开启 adc0 转换
        adc0_status = ADC_STATUS_SEL_GET_FORWARD;
    }
    else if (adc0_status == ADC_STATUS_SEL_GET_FORWARD)
    {
        // 切换ad通道
        adc0_sel_channel(ADC_CHANNEL_REVERSE);
        adc0_status = ADC_STATUS_SEL_GET_REVERSE_WAITING;
    }
    else if (adc0_status == ADC_STATUS_SEL_GET_REVERSE_WAITING)
    {
        // 开启转换，之后在ad中断获取ad值
        ADC_CFG0 |= 0x01 << 0; // 开启 adc0 转换
        adc0_status = ADC_STATUS_SEL_GET_REVERSE;
    }

    // ===========================================================

    if (adc1_status == ADC_STATUS_NONE || adc1_status == ADC_STATUS_SEL_GET_REVERSE)
    {
        // 切换ad通道
        adc1_sel_channel(ADC_CHANNEL_FORWARD);
        adc1_status = ADC_STATUS_SEL_GET_FORWARD_WAITING;
    }
    else if (adc1_status == ADC_STATUS_SEL_GET_FORWARD_WAITING)
    {
        // 开启转换，之后在ad中断获取ad值
        ADC_CFG0 |= 0x01 << 1; // 开启 adc1 转换
        adc1_status = ADC_STATUS_SEL_GET_FORWARD;
    }
    else if (adc1_status == ADC_STATUS_SEL_GET_FORWARD)
    {
        // 切换ad通道
        adc1_sel_channel(ADC_CHANNEL_REVERSE);
        adc1_status = ADC_STATUS_SEL_GET_REVERSE_WAITING;
    }
    else if (adc1_status == ADC_STATUS_SEL_GET_REVERSE_WAITING)
    {
        // 开启转换，之后在ad中断获取ad值
        ADC_CFG0 |= 0x01 << 1; // 开启 adc1 转换
        adc1_status = ADC_STATUS_SEL_GET_REVERSE;
    }
}

void ADC_IRQHandler(void) interrupt ADC_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(ADC_IRQn);

    // ---------------- 用户函数处理 -------------------

#if 0

    if (ADC_STA & ADC_CHAN0_DONE(0x01))
    {
        volatile u16 adc_val = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4); // 先接收ad值
        ADC_STA |= ADC_CHAN0_DONE(0x01);                              // 清除ADC0转换完成标志位

        if (ADC_STATUS_SEL_ENGINE == cur_adc_status)
        {
            // 更新发动机检测一端的ad值

            static u8 i = 0; // adc采集次数的计数
            static volatile u32 g_tmpbuff = 0;
            static volatile u16 g_adcmax = 0;
            static volatile u16 g_adcmin = 0xFFFF;

            if (i < 20)
            {
                i++;

                if (i >= 2) // 丢弃前两次采样值
                {
                    if (adc_val > g_adcmax)
                        g_adcmax = adc_val; // 最大
                    if (adc_val < g_adcmin)
                        g_adcmin = adc_val; // 最小
                    g_tmpbuff += adc_val;
                }

                if (i < 20)
                    ADC_CFG0 |= 0x01 << 0; // 开启adc0转换
            }

            if (i >= 20)
            {
                adc_val_from_engine = (g_tmpbuff >> 4); // 除以16，取平均值
                cur_adc_status = ADC_STATUS_SEL_ENGINE_DONE;

                // 重新初始化使用到的变量：
                i = 0;
                g_adcmax = 0;
                g_adcmin = 0xFFFF;
                g_tmpbuff = 0;
                // printf("1 engine scan done\n");
            }
        }
        else if (ADC_STATUS_SEL_KNOB == cur_adc_status)
        {
            // 更新旋钮检测一端的ad值
            adc_val_from_knob = adc_val;
            // printf("2 knob scan done\n");
        }
        else if (ADC_STATUS_SEL_GET_TEMP == cur_adc_status)
        {
            // 更新热敏电阻检测一端的ad值
            adc_val_from_temp = adc_val;
            // printf("3 temp scan done\n");
        }
        else if (ADC_STATUS_SEL_FAN_DETECT == cur_adc_status)
        {
            // 更新风扇检测一端的ad值
            adc_val_from_fan = adc_val;
            // printf("4 fan scan done\n");
        }
    }


    if (ADC_STA & ADC_CHAN1_DONE(0x01))
    {

    }
#endif

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(ADC_IRQn);
}
