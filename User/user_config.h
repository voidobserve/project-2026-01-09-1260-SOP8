
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define USER_DEBUG_ENABLE 1

#if USER_DEBUG_ENABLE
#include <stdio.h>
#define USER_DEBUG_PIN_ENABLE 1 // 使用demo板上的引脚
#else
#define USER_DEBUG_PIN_ENABLE 0 // 不使用demo板上的引脚
#endif


#endif