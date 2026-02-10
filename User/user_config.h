
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "include.h"

#define U8_MAX_VAL ((const u8)0xFF)
#define U16_MAX_VAL ((const u16)0xFFFF)

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define USER_DEBUG_ENABLE 1

#if USER_DEBUG_ENABLE
#include <stdio.h>
#define USER_DEBUG_PIN_ENABLE 1 // 使用demo板上的引脚
#else
#define USER_DEBUG_PIN_ENABLE 0 // 不使用demo板上的引脚，使用实际的引脚
#endif


#endif