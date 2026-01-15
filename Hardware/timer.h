#ifndef __TIMER_H__
#define __TIMER_H__
#include "include.h"

// 常用的定时时间 [0]-->1ms,  [1]-->500us,  [2]-->125us, [3]-->100us, [4]-->10us
static const u8 timer_div[] =
	{
		1,	 // 1ms
		2,	 // 500us
		8,	 // 125us
		10,	 // 100us
		100, // 10us
};

#define TIME_SEL_1MS (0)
#define TIME_SEL_500US (1)
#define TIME_SEL_125US (2)
#define TIME_SEL_100US (3)
#define TIME_SEL_10US (4)

#define TIMER0_TIME_SEL TIME_SEL_1MS

#if TIMER0_TIME_SEL == TIME_SEL_1MS
#define UP_1MS (1)
#elif TIMER0_TIME_SEL == TIME_SEL_500US
#define UP_1MS (2)
#elif TIMER0_TIME_SEL == TIME_SEL_125US
#define UP_1MS (8)
#elif TIMER0_TIME_SEL == TIME_SEL_100US
#define UP_1MS (10)
#elif TIMER0_TIME_SEL == TIME_SEL_10US
#define UP_1MS (100)
#endif

#define TIMER0_PEROID_VAL (SYSCLK / 1 / 1000 / timer_div[TIMER0_TIME_SEL] - 1) // 周期值=系统时钟/分频/频率 - 1

void timer0_init(void);

#endif
