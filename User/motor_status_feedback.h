#ifndef __MOTOR_STATUS_FEEDBACK_H__
#define __MOTOR_STATUS_FEEDBACK_H__

#include "motor_handle.h"

#define MOTOR_STATUS_SCAN_PERIOD 10      // 扫描电机状态的周期，单位：ms。这个时间要小于 MOTOR_STATUS_FEEDBACK_PERIOD
#define MOTOR_STATUS_FEEDBACK_PERIOD 200 // 向蓝牙ic反馈电机状态的周期，单位：ms
#if (MOTOR_STATUS_FEEDBACK_PERIOD / MOTOR_STATUS_SCAN_PERIOD) < 2
#error "MOTOR_STATUS_FEEDBACK_PERIOD must be greater than MOTOR_STATUS_SCAN_PERIOD * 2"
#endif

// 定义要反馈的电机状态
enum
{
    FEEDBACK_MOTOR_STATUS_NONE,
    FEEDBACK_MOTOR_STATUS_FORWARD,      // 正在正转
    FEEDBACK_MOTOR_STATUS_FORWARD_STOP, // 正转停止（正转之后，电机停了下来）
    FEEDBACK_MOTOR_STATUS_REVERSE,
    FEEDBACK_MOTOR_STATUS_REVERSE_STOP,

    FEEDBACK_MOTOR_STATUS_TOTAL_NUM, 
} ;
typedef u8 feedback_motor_status_t; 

void motor_0_status_buf_push(feedback_motor_status_t status);
void motor_1_status_buf_push(feedback_motor_status_t status);

void motor_status_feedback_init(void);
void motor_status_scan(void);

void motor_status_feedback(void);

#endif