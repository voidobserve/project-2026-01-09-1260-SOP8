#include "adc.h"
#include "user_config.h"

// 存放采集到的ad值
static volatile u16 adc_val_forward_0 = 0; // 电机0 正转 ad值
static volatile u16 adc_val_reverse_0 = 0; // 电机0 反转 ad值
static volatile u16 adc_val_forward_1 = 0; // 电机1 正转 ad值
static volatile u16 adc_val_reverse_1 = 0; // 电机1 反转 ad值

static volatile bit flag_is_adc_val_forward0_update = 0; // adc0 正转 ad值更新标志
static volatile bit flag_is_adc_val_reverse0_update = 0;
static volatile bit flag_is_adc_val_forward1_update = 0;
static volatile bit flag_is_adc_val_reverse1_update = 0;

// adc0 、 adc1 的状态机
static volatile u8 adc0_status = ADC_STATUS_NONE; // 如果不赋值，有概率，初始值会不是0，进而导致adc异常工作
static volatile u8 adc1_status = ADC_STATUS_NONE;

void adc_init(void)
{
    // 初始化对应的引脚：
    // M+ DET 第1路电机正转电流检测（ADC输入，检测门向前运动时是否遇到障碍物）
#if USER_DEBUG_PIN_ENABLE
    P0_MD1 |= GPIO_P05_MODE_SEL(0x03); // 模拟输入
#else
    P1_MD0 |= GPIO_P12_MODE_SEL(0x03); // 模拟输入
#endif

    // M- DET 第1路电机反转电流检测 （ADC输入，检测门是否回到原始位置）
    P0_MD0 |= GPIO_P00_MODE_SEL(0x03); // 模拟输入

    // M2+ DET 第2路电机正转电流检测（ADC输入，检测门向前运动时是否遇到障碍物）
    P2_MD0 |= GPIO_P21_MODE_SEL(0x03); // 模拟输入

    // M2- DET 第2路电机反转电流检测（ADC输入，检测门是否回到原始位置）
    P3_MD0 |= GPIO_P30_MODE_SEL(0x03); // 模拟输入

    // ADC配置
    ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) | ADC_EXREF_SEL(0x1)); // 关闭外部参考电压
    ADC_ACON1 |= ADC_VREF_SEL(0x5) |                        // 选择 4.2V 作为参考电压
                 ADC_TEN_SEL(0x3);                          // 关闭测试信号

    ADC_ACON0 = ADC_CMP_EN(0x1) |  // 打开ADC中的CMP使能信号
                ADC_BIAS_EN(0x1) | // 打开ADC偏置电流能使信号
                ADC_BIAS_SEL(0x1); // 打开 ADC偏置电流

    __EnableIRQ(ADC_IRQn); // 使能ADC中断
    IE_EA = 1;             // 使能总中断

    ADC_CFG1 |= (0x0F << 3) | // ADC时钟分频为16分频，为系统时钟/16（相当于把adc时钟设置为最慢，）
                (0x01 << 1) | // ADC1 通道中断使能
                (0x01 << 0);  // ADC0 通道中断使能

    ADC_CFG0 |= ADC_CHAN0_EN(0x1) | // 使能 通道0
                ADC_CHAN1_EN(0x1) | // 使能 通道1
                ADC_EN(0x1);        // 使能adc

    delay_ms(1); // 等待ADC模块配置稳定，需要等待20us以上
}

// 置位 对应通道编号的 ad值更新标志
void adc_set_update_flag(adc_channel_index_t adc_channel_index)
{

    switch (adc_channel_index)
    {
    case ADC_CHANNEL_INDEX_FORWARD_0:
    {
        flag_is_adc_val_forward0_update = 1;
    }
    break;
        // =======================================================
    case ADC_CHANNEL_INDEX_REVERSE_0:
    {
        flag_is_adc_val_reverse0_update = 1;
    }
    break;
        // =======================================================
    case ADC_CHANNEL_INDEX_FORWARD_1:
    {
        flag_is_adc_val_forward1_update = 1;
    }
    break;
    // =======================================================
    case ADC_CHANNEL_INDEX_REVERSE_1:
    {
        flag_is_adc_val_reverse1_update = 1;
    }
    break;
        // =======================================================
    }
}

// 清除 对应通道编号的 ad值更新标志
void adc_clear_update_flag(adc_channel_index_t adc_channel_index)
{
    switch (adc_channel_index)
    {
    case ADC_CHANNEL_INDEX_FORWARD_0:
    {
        flag_is_adc_val_forward0_update = 0;
    }
    break;
        // =======================================================
    case ADC_CHANNEL_INDEX_REVERSE_0:
    {
        flag_is_adc_val_reverse0_update = 0;
    }
    break;
        // =======================================================
    case ADC_CHANNEL_INDEX_FORWARD_1:
    {
        flag_is_adc_val_forward1_update = 0;
    }
    break;
    // =======================================================
    case ADC_CHANNEL_INDEX_REVERSE_1:
    {
        flag_is_adc_val_reverse1_update = 0;
    }
    break;
        // =======================================================
    }
}

// 获取 对应通道编号的 ad值更新标志
u8 adc_get_update_flag(adc_channel_index_t adc_channel_index)
{
    u8 ret = 0;

    switch (adc_channel_index)
    {
    case ADC_CHANNEL_INDEX_FORWARD_0:
    {
        ret = flag_is_adc_val_forward0_update;
    }
    break;
    // =======================================================
    case ADC_CHANNEL_INDEX_REVERSE_0:
    {
        ret = flag_is_adc_val_reverse0_update;
    }
    break;
    // =======================================================
    case ADC_CHANNEL_INDEX_FORWARD_1:
    {
        ret = flag_is_adc_val_forward1_update;
    }
    break;
    // =======================================================
    case ADC_CHANNEL_INDEX_REVERSE_1:
    {
        ret = flag_is_adc_val_reverse1_update;
    }
    break;
        // =======================================================
    }

    return ret;
}

/**
 * @brief 获取adc通道编号对应的 ad值
 *
 * @attention 调用该函数前，应该先判断 对应通道编号的 ad值更新标志
 *
 * @param adc_channel_index
 * @return u16
 */
u16 adc_get_val(adc_channel_index_t adc_channel_index)
{
    u16 ret = 0;

    // 获取ad值之前，先屏蔽中断，因为ad值是在中断中更新
    IE_EA = 0;
    switch (adc_channel_index)
    {
    case ADC_CHANNEL_INDEX_FORWARD_0:
    {
        ret = adc_val_forward_0;
    }
    break;
    // =======================================================
    case ADC_CHANNEL_INDEX_REVERSE_0:
    {
        ret = adc_val_reverse_0;
    }
    break;
        // =======================================================
    case ADC_CHANNEL_INDEX_FORWARD_1:
    {
        ret = adc_val_forward_1;
    }
    break;
    // =======================================================
    case ADC_CHANNEL_INDEX_REVERSE_1:
    {
        ret = adc_val_reverse_1;
    }
    break;
        // =======================================================
    }
    IE_EA = 1; // 使能总中断

    return ret;
}

/**
 * @brief 切换 adc0 的通道
 * @attention 函数内部没有延时等待adc稳定，调用时需要注意
 *
 * @param adc_channel
 */
static void adc0_sel_channel(u8 adc_channel)
{
    switch (adc_channel)
    {
    case ADC_CHANNEL_FORWARD:
    {
#if USER_DEBUG_PIN_ENABLE
        ADC_CHS0 = ADC_ANALOG_CHAN(0x05); // 模拟通道 0x05：P05
#else
        ADC_CHS0 = ADC_ANALOG_CHAN(0x0A); // 模拟通道 0x0A：P12
#endif
    }
    break;
    // =======================================================
    case ADC_CHANNEL_REVERSE:
    {
        ADC_CHS0 = ADC_ANALOG_CHAN(0x00); // 模拟通道 0x00：P00
    }
    break;
    // =======================================================
    default:
    {
    }
    break;
    }
}

/**
 * @brief 切换 adc1 的通道
 * @attention 函数内部没有延时等待adc稳定，调用时需要注意
 *
 * @param adc_channel
 */
static void adc1_sel_channel(u8 adc_channel)
{
    switch (adc_channel)
    {
    case ADC_CHANNEL_FORWARD:
    {
        ADC_CHS1 = ADC_ANALOG_CHAN(0x11); // 模拟通道 0x11：P21
    }
    break;
    // =======================================================
    case ADC_CHANNEL_REVERSE:
    {
        ADC_CHS1 = ADC_ANALOG_CHAN(0x18); // 模拟通道 0x18：P30
    }
    break;
    // =======================================================
    default:
    {
    }
    break;
    }
}

/**
 * @brief adc扫描函数，包含轮询、顺序切换通道和启动转换
 * @note 该函数启动ad转换之后，在adc中断服务函数中接收adc值
 * @attention 函数内部没有延时等待adc稳定，调用该函数的时间间隔至少要20us，一般是用1ms
 *
 */
void adc_scan(void)
{
    // printf("adc0_status == %bu\n", adc0_status);
    // printf("adc1_status == %bu\n", adc1_status);

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
    volatile u16 adc_val = 0;

    // 进入中断设置IP，不可删除
    __IRQnIPnPush(ADC_IRQn);

    // ---------------- 用户函数处理 -------------------
    // printf("ADC_IRQHandler\n");

    // adc0 转换完成：
    if (ADC_STA & ADC_CHAN0_DONE(0x01))
    {
        adc_val = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4); // 先接收ad值
        ADC_STA |= ADC_CHAN0_DONE(0x01);                 // 清除 ADC0 转换完成标志位

        if (adc0_status == ADC_STATUS_SEL_GET_FORWARD)
        {
            // 获取ad值
            adc_val_forward_0 = adc_val;
            adc_set_update_flag(ADC_CHANNEL_INDEX_FORWARD_0);
#if USER_DEBUG_ENABLE
            // printf("adc0 forward: %u\n", adc_val_forward_0);
#endif
        }
        else if (adc0_status == ADC_STATUS_SEL_GET_REVERSE)
        {
            // 获取ad值
            adc_val_reverse_0 = adc_val;
            adc_set_update_flag(ADC_CHANNEL_INDEX_REVERSE_0);
#if USER_DEBUG_ENABLE
            // printf("adc0 reverse: %u\n", adc_val_reverse_0);
#endif
        }
    }

    // adc1 转换完成：
    if (ADC_STA & ADC_CHAN1_DONE(0x01))
    {
        adc_val = (ADC_DATAH1 << 4) | (ADC_DATAL1 >> 4); // 先接收ad值
        ADC_STA |= ADC_CHAN1_DONE(0x01);                 // 清除 ADC1 转换完成标志位

        if (adc1_status == ADC_STATUS_SEL_GET_FORWARD)
        {
            // 获取ad值
            adc_val_forward_1 = adc_val;
            adc_set_update_flag(ADC_CHANNEL_INDEX_FORWARD_1);
#if USER_DEBUG_ENABLE
            // printf("adc1 forward: %u\n", adc_val_forward_1);
#endif
        }
        else if (adc1_status == ADC_STATUS_SEL_GET_REVERSE)
        {
            // 获取ad值
            adc_val_reverse_1 = adc_val;
            adc_set_update_flag(ADC_CHANNEL_INDEX_REVERSE_1);
#if USER_DEBUG_ENABLE
            // printf("adc1 reverse: %u\n", adc_val_reverse_1);
#endif
        }
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(ADC_IRQn);
}

// 测试adc功能：
#if 0
        // if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_0))
        // {
        //     u16 adc_val = 0;
        //     adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_0);
        //     adc_val = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_0);
        //     printf("adc forward 0 val: %u\n", adc_val);
        // }

        // if (adc_get_update_flag(ADC_CHANNEL_INDEX_REVERSE_0))
        // {
        //     u16 adc_val = 0;
        //     adc_clear_update_flag(ADC_CHANNEL_INDEX_REVERSE_0);
        //     adc_val = adc_get_val(ADC_CHANNEL_INDEX_REVERSE_0);
        //     printf("adc reverse 0 val: %u\n", adc_val);
        // }

        // if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_1))
        // {
        //     u16 adc_val = 0;
        //     adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_1);
        //     adc_val = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_1);
        //     printf("adc forward 1 val: %u\n", adc_val);
        // }

        // if (adc_get_update_flag(ADC_CHANNEL_INDEX_REVERSE_1))
        // {
        //     u16 adc_val = 0;
        //     adc_clear_update_flag(ADC_CHANNEL_INDEX_REVERSE_1);
        //     adc_val = adc_get_val(ADC_CHANNEL_INDEX_REVERSE_1);
        //     printf("adc reverse 1 val: %u\n", adc_val);
        // }
#endif