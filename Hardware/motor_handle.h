#ifndef __MOTOR_HANDLE_H__
#define __MOTOR_HANDLE_H__

#include "include.h"
#include "pwm.h"

#define MOTOR_DIR_CHANGE_DLY_TIME ((u16)300) // 单位：ms
#define MOTOR_WORKING_PWM_DUTY_VAL ((u16)PWM_DUTY_VAL_85_PERCENT)
#define MOTOR_OVERCURRENT_ADC_VAL ((u16)1657)     // 电机过流时，adc检测对应的阈值（约1.7V）
#define MOTOR_OVERCURRENT_TIME_CNT_MAX ((u16)200) // 电机过流检测的计数值最大值
// #define MOTOR_WORKING_OVER_TIME ((u16)2500)   // 电机工作正常时，超时时间
#define MOTOR_WORKING_OVER_TIME ((u16)8000) // 电机工作正常时，超时时间

#define MOTOR_0_FORWARD_PWM_DUTY_SET(motor_0_pwm_duty) pwm_channel_0_set_duty(motor_0_pwm_duty)
#define MOTOR_0_REVERSE_PWM_DUTY_SET(motor_0_pwm_duty) pwm_channel_1_set_duty(motor_0_pwm_duty)
#define MOTOR_1_FORWARD_PWM_DUTY_SET(motor_1_pwm_duty) pwm_channel_2_set_duty(motor_1_pwm_duty)
#define MOTOR_1_REVERSE_PWM_DUTY_SET(motor_1_pwm_duty) pwm_channel_3_set_duty(motor_1_pwm_duty)

// enum
// {
//     MOTOR_STATUS_STOP = 0,
//     MOTOR_STATUS_FORWARD,
//     MOTOR_STATUS_REVERSE,
// };
// typedef u8 motor_status_t;

// 定义电机状态
enum
{
    // MOTOR_STATUS_IDLE = 0, // 刚上电，电机处于空闲状态
    MOTOR_STATUS_STOP = 0, // 电机停止工作
    MOTOR_STATUS_FORWARD,  // 电机正在正转  
    MOTOR_STATUS_REVERSE,  
};
typedef u8 motor_status_t;

// 电机方向，作为矢量来使用，并不代表电机当前的状态
enum
{
    MOTOR_DIR_NONE = 0, // 默认状态
    MOTOR_DIR_FORWARD,
    MOTOR_DIR_REVERSE,
};
typedef u8 motor_dest_dir_t;

typedef struct
{
    motor_dest_dir_t dest_dir; // 目标方向（只能是正转或者反转）
    motor_status_t status;     // 当前状态
    u8 is_status_need_to_feedback; // 是否要反馈电机的状态（电机要换方向而短暂停止时，不把这个停止状态反馈给蓝牙ic，因为蓝牙ic会认为电机已经停止工作，会作出对应的错误响应）

    u8 change_dir_enable; // 是否要改变方向
    u16 change_dir_cnt;   // 改变方向的计数器（电机不能马上改变方向，如果同时正转和反转，驱动电机的MOS管会短路）

    u16 overcurrent_time_cnt; // 过流检测计数器
    u16 adc_val_forward;      // 正转时，检测到的ad值
    u16 adc_val_reverse;      // 反转时，检测到的ad值

    u16 working_time_cnt; // 工作时间计数器

    void (*adc_val_update)(void); // 更新ad值
    void (*forward)(void);        // 正转
    void (*reverse)(void);        // 反转
    void (*stop)(void);           // 停止

} motor_handle_t;

extern volatile motor_handle_t motor_handle_0;
extern volatile motor_handle_t motor_handle_1;

void motor_handle_init(void);
void motor_handle_func_0(void);
void motor_handle_func_1(void);

// motor_handle_x 的友元函数
// 设置电机要切换到的转动方向
void motor_set_dir(motor_handle_t *motor_handle, motor_dest_dir_t dest_dir);

#if 0
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

#endif
