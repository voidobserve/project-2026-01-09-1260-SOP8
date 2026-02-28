#ifndef __MOTOR_STATUS_FEEDBACK_H__
#define __MOTOR_STATUS_FEEDBACK_H__ 

#include "motor_handle.h"

#define MOTOR_STATUS_SCAN_PERIOD 10 // 扫描电机状态的周期，单位：ms。这个时间要小于 MOTOR_STATUS_FEEDBACK_PERIOD
#define MOTOR_STATUS_FEEDBACK_PERIOD 100 // 向蓝牙ic反馈电机状态的周期，单位：ms
#if (MOTOR_STATUS_FEEDBACK_PERIOD / MOTOR_STATUS_SCAN_PERIOD) < 2
#error "MOTOR_STATUS_FEEDBACK_PERIOD must be greater than MOTOR_STATUS_SCAN_PERIOD * 2"
#endif

void motor_0_status_buf_push(motor_status_t status);
void motor_1_status_buf_push(motor_status_t status);

void motor_status_scan(void);

void motor_status_feedback(void); 
 

#endif