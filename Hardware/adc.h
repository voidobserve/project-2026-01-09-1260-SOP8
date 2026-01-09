#ifndef __ADC_H__
#define __ADC_H__

#include "include.h"

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

void adc_scan(void);

#endif
