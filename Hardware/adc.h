#ifndef __ADC_H__
#define __ADC_H__

#include "include.h"

// 定义adc通道，在硬件驱动层使用
enum
{
    ADC_CHANNEL_NONE = 0,
    ADC_CHANNEL_FORWARD, // 检测电机正转对应的通道
    ADC_CHANNEL_REVERSE, // 检测电机反转对应的通道
};

// 定义adc切换通道状态机使用到的数值：
enum
{
    ADC_STATUS_NONE = 0,

    ADC_STATUS_SEL_GET_FORWARD_WAITING, // 等待adc稳定
    ADC_STATUS_SEL_GET_FORWARD,         // 切换至对应的通道

    ADC_STATUS_SEL_GET_REVERSE_WAITING,
    ADC_STATUS_SEL_GET_REVERSE,
};

// 定义adc通道编号，在软件层使用
enum
{
    ADC_CHANNEL_INDEX_FORWARD_0,
    ADC_CHANNEL_INDEX_REVERSE_0,
    ADC_CHANNEL_INDEX_FORWARD_1,
    ADC_CHANNEL_INDEX_REVERSE_1,
}; 
typedef u8 adc_channel_index_t;

/*
    USER_TO_DO 这里还没有配置好
    通道0 数据格式 左对齐
    通道0 dly功能 关闭
    模拟通道 选择芯片 外部通道
    选择引脚对应的通道 0x16--P26
*/
// adc0 切换到 检测正转的通道
#define ADC0_SEL_FORWARD_CHANNEL()        \
    do                                    \
    {                                     \
        ADC_CHS0 = ADC_ANALOG_CHAN(0x16); \
    } while (0)

// adc0 切换到 检测反转的通道
#define ADC0_SEL_REVERSE_CHANNEL()        \
    do                                    \
    {                                     \
        ADC_CHS0 = ADC_ANALOG_CHAN(0x15); \
    } while (0)

// adc1 切换到 检测正转的通道
#define ADC1_SEL_FORWARD_CHANNEL()        \
    do                                    \
    {                                     \
        ADC_CHS1 = ADC_ANALOG_CHAN(0x14); \
    } while (0)

// adc1 切换到 检测反转的通道
#define ADC1_SEL_REVERSE_CHANNEL()        \
    do                                    \
    {                                     \
        ADC_CHS1 = ADC_ANALOG_CHAN(0x13); \
    } while (0)
 

void adc_init(void);
void adc_scan(void);

void adc_set_update_flag(adc_channel_index_t adc_channel_index);
void adc_clear_update_flag(adc_channel_index_t adc_channel_index);
u8 adc_get_update_flag(adc_channel_index_t adc_channel_index);
u16 adc_get_val(adc_channel_index_t adc_channel_index);


#endif
