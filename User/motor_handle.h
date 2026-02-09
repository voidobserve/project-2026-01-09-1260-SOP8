#ifndef __MOTOR_HANDLE_H__
#define __MOTOR_HANDLE_H__ 

#include "include.h"

enum
{
    MOTOR_STATUS_STOP = 0,
    MOTOR_STATUS_FORWARD,
    MOTOR_STATUS_REVERSE,
};
typedef u8 motor_status_t;

extern volatile u8 motor_0_status;
extern volatile u8 motor_1_status;

#endif
