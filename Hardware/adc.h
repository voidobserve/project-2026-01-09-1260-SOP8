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

void adc_init(void);
void adc_scan(void);

void adc_set_update_flag(adc_channel_index_t adc_channel_index);
void adc_clear_update_flag(adc_channel_index_t adc_channel_index);
u8 adc_get_update_flag(adc_channel_index_t adc_channel_index);
u16 adc_get_val(adc_channel_index_t adc_channel_index);

#endif
