#ifndef __MOTOR_HANDLE_H__
#define __MOTOR_HANDLE_H__

#include "include.h"
#include "pwm.h"

#define MOTOR_DIR_CHANGE_DLY_TIME ((u16)300) // 单位：ms
#define MOTOR_WORKING_PWM_DUTY_VAL ((u16)PWM_DUTY_VAL_85_PERCENT)
#define MOTOR_OVERCURRENT_ADC_VAL ((u16)1657) // 电机过流时，adc检测对应的阈值（约1.7V）
#define MOTOR_WORKING_OVER_TIME ((u16)2500)   // 电机工作正常时，超时时间

#define MOTOR_0_FORWARD_PWM_DUTY_SET(motor_0_pwm_duty) pwm_channel_0_set_duty(motor_0_pwm_duty)
#define MOTOR_0_REVERSE_PWM_DUTY_SET(motor_0_pwm_duty) pwm_channel_1_set_duty(motor_0_pwm_duty)
#define MOTOR_1_FORWARD_PWM_DUTY_SET(motor_1_pwm_duty) pwm_channel_2_set_duty(motor_1_pwm_duty)
#define MOTOR_1_REVERSE_PWM_DUTY_SET(motor_1_pwm_duty) pwm_channel_3_set_duty(motor_1_pwm_duty)

enum
{
    MOTOR_STATUS_STOP = 0,
    MOTOR_STATUS_FORWARD,
    MOTOR_STATUS_REVERSE,
};
typedef u8 motor_status_t;

extern volatile u8 motor_0_status;
extern volatile u8 motor_1_status;
extern volatile u16 motor_0_dir_change_dly;   //
extern volatile u16 motor_1_dir_change_dly;   //
extern volatile u8 motor_0_dir_change_enable; // 标志位，是否允许电机改变方向
extern volatile u8 motor_1_dir_change_enable; // 标志位，是否允许电机改变方向

extern volatile u8 motor_0_is_working; // 电机是否正在工作
extern volatile u8 motor_1_is_working; // 电机是否正在工作

void motor_0_change_status(motor_status_t status);
void motor_1_change_status(motor_status_t status);

void motor_0_handler(void);
void motor_1_handler(void);

#endif
