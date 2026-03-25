
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "include.h"

#define U8_MAX_VAL ((const u8)0xFF)
#define U16_MAX_VAL ((const u16)0xFFFF)

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define USER_DEBUG_ENABLE 0 // 打开用户调试，包含使用demo板上的引脚
#define USER_DEBUG_PIN_ENABLE 0 // 是否使用demo板来调试，使用demo板的引脚

#if ((0 == USER_DEBUG_ENABLE) && USER_DEBUG_PIN_ENABLE)
#error USER_DEBUG_PIN_ENABLE need (USER_DEBUG_ENABLE == 1)
#endif

#if USER_DEBUG_ENABLE
#include <stdio.h>
#endif

#include "motor_handle.h"

#endif