#include "motor_handle.h"
#include "adc.h"

volatile motor_handle_t motor_handle_0;
volatile motor_handle_t motor_handle_1;
/*
    要让指针指向xdata区域的全局变量，需要同样将指针变为全局变量

    测试发现使用函数内部的局部变量作为指针也可以指向全局变量所在的地址了，但是为了避免意外情况，
    这里还是用全局变量指针来指向全局变量
*/
static volatile motor_handle_t *motor_handle_ptr = NULL;

static void motor_0_stop(void)
{
    MOTOR_0_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
    MOTOR_0_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
}

static void motor_0_forward(void)
{
    MOTOR_0_FORWARD_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
}

static void motor_0_reverse(void)
{
    MOTOR_0_REVERSE_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
}

static void motor_0_adc_val_update(void)
{
    if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_0))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_0);
        motor_handle_0.adc_val_forward = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_0);
    }

    if (adc_get_update_flag(ADC_CHANNEL_INDEX_REVERSE_0))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_REVERSE_0);
        motor_handle_0.adc_val_reverse = adc_get_val(ADC_CHANNEL_INDEX_REVERSE_0);
    }
}

static void motor_1_stop(void)
{
    MOTOR_1_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
    MOTOR_1_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
}

static void motor_1_forward(void)
{
    MOTOR_1_FORWARD_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
}

static void motor_1_reverse(void)
{
    MOTOR_1_REVERSE_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
}

static void motor_1_adc_val_update(void)
{
    if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_1))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_1);
        motor_handle_1.adc_val_forward = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_1);
    }

    if (adc_get_update_flag(ADC_CHANNEL_INDEX_REVERSE_1))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_REVERSE_1);
        motor_handle_1.adc_val_reverse = adc_get_val(ADC_CHANNEL_INDEX_REVERSE_1);
    }
}

void motor_handle_init(void)
{
    motor_handle_0.dest_dir = MOTOR_DIR_NONE;
    motor_handle_0.status = MOTOR_STATUS_STOP;
    motor_handle_0.change_dir_enable = 0;
    motor_handle_0.forward = motor_0_forward;
    motor_handle_0.reverse = motor_0_reverse;
    motor_handle_0.stop = motor_0_stop;
    motor_handle_0.adc_val_update = motor_0_adc_val_update;
    motor_handle_0.is_status_need_to_feedback = 0;

    motor_handle_1.dest_dir = MOTOR_DIR_NONE;
    motor_handle_1.status = MOTOR_STATUS_STOP;
    motor_handle_1.change_dir_enable = 0;
    motor_handle_1.forward = motor_1_forward;
    motor_handle_1.reverse = motor_1_reverse;
    motor_handle_1.stop = motor_1_stop;
    motor_handle_1.adc_val_update = motor_1_adc_val_update;
    motor_handle_0.is_status_need_to_feedback = 0;
}

/**
 * @brief 设置电机要切换到的方向（该函数相当于 motor_handle_x 的友元函数）
 *
 * @param motor_handle
 * @param dest_dir
 *       MOTOR_DIR_NONE ：自动切换，原来是正转则切换到反转，原来是反转则切换到正转
 *       MOTOR_DIR_FORWARD ：切换到正转
 *       MOTOR_DIR_REVERSE ：切换到反转
 */
void motor_set_dir(motor_handle_t *motor_handle, motor_dest_dir_t dest_dir)
{
    // 无论要执行什么操作，都先停止电机
    motor_handle_ptr = (motor_handle_t *)motor_handle;
    motor_handle_ptr->stop();
    motor_handle_ptr->status = MOTOR_STATUS_STOP;     // 表示电机已经停止（如果不加这句，电机工作的累计时间会一直持续，不会清零）
    motor_handle_ptr->is_status_need_to_feedback = 0; // 不把这个停止状态反馈给蓝牙ic

    switch (dest_dir)
    {
    case MOTOR_DIR_NONE: // 根据原来的方向，自动切换
        if (motor_handle_ptr->dest_dir == MOTOR_DIR_FORWARD)
        {
            // 原本的方向是正转，改成反转
            motor_handle_ptr->dest_dir = MOTOR_DIR_REVERSE;
        }
        else
        {
            /*
                motor_handle_ptr->dest_dir == MOTOR_DIR_REVERSE 或者
                motor_handle_ptr->dest_dir == MOTOR_DIR_NONE
                原本电机没有设置方向，或者原本电机转动方向是反转，都设置为正转
            */
            motor_handle_ptr->dest_dir = MOTOR_DIR_FORWARD;
        }
        break;
    case MOTOR_DIR_FORWARD:
        motor_handle_ptr->dest_dir = MOTOR_DIR_FORWARD;
        break;
    case MOTOR_DIR_REVERSE:
        motor_handle_ptr->dest_dir = MOTOR_DIR_REVERSE;
        break;
    }

    // 设置延时，准备让电机改变方向
    motor_handle_ptr->change_dir_cnt = MOTOR_DIR_CHANGE_DLY_TIME;
    motor_handle_ptr->change_dir_enable = 1;
}

static void motor_handle(motor_handle_t *motor_handle)
{
    motor_handle_ptr = (motor_handle_t *)motor_handle;

    if (motor_handle_ptr->change_dir_enable && motor_handle_ptr->change_dir_cnt > 0)
    {
        // 如果还在准备改变方向（还在计数中），不处理
        motor_handle_ptr->change_dir_cnt--;
    }

    if (motor_handle_ptr->change_dir_enable && motor_handle_ptr->change_dir_cnt == 0)
    {
        // 如果要改变电机方向，并且已经计数完毕
        motor_handle_ptr->change_dir_enable = 0;
        if (motor_handle_ptr->dest_dir == MOTOR_DIR_FORWARD)
        {
            motor_handle_ptr->status = MOTOR_STATUS_FORWARD;
            motor_handle_ptr->forward();
        }
        else
        {
            motor_handle_ptr->status = MOTOR_STATUS_REVERSE;
            motor_handle_ptr->reverse();
        }

        motor_handle_ptr->is_status_need_to_feedback = 1; // 允许反馈电机的状态
    }

    // 在电机转动时，检测电机是否过流：
    motor_handle_ptr->adc_val_update(); // 更新ad值
    if (motor_handle_ptr->status == MOTOR_STATUS_FORWARD)
    {
        if (motor_handle_ptr->adc_val_forward > MOTOR_OVERCURRENT_ADC_VAL)
        {
            motor_handle_ptr->overcurrent_time_cnt++;
        }
        else
        {
            motor_handle_ptr->overcurrent_time_cnt = 0;
        }
    }
    else if (motor_handle_ptr->status == MOTOR_STATUS_REVERSE)
    {
        if (motor_handle_ptr->adc_val_reverse > MOTOR_OVERCURRENT_ADC_VAL)
        {
            motor_handle_ptr->overcurrent_time_cnt++;
        }
        else
        {
            motor_handle_ptr->overcurrent_time_cnt = 0;
        }
    }
    else
    {
        // 电机没有在工作，清空过流检测的计数
        motor_handle_ptr->overcurrent_time_cnt = 0;
    }

    if (motor_handle_ptr->status != MOTOR_STATUS_STOP)
    {
        motor_handle_ptr->working_time_cnt++; // 工作时间计数
    }
    else // motor_handle_ptr->status == MOTOR_STATUS_STOP
    {
        motor_handle_ptr->working_time_cnt = 0; // 电机没有在转动，清空工作时间计数
    }

    if (motor_handle_ptr->overcurrent_time_cnt > MOTOR_OVERCURRENT_TIME_CNT_MAX ||
        motor_handle_ptr->working_time_cnt > MOTOR_WORKING_OVER_TIME)
    {
        // 过流且累计一定次数，停下电机
        // 电机转动且超过一定时间，也停下电机
        motor_handle_ptr->status = MOTOR_STATUS_STOP;
        motor_handle_ptr->stop();
        motor_handle_ptr->is_status_need_to_feedback = 1;
    }
}

void motor_handle_func_0(void)
{
    motor_handle(&motor_handle_0);
}

void motor_handle_func_1(void)
{
    motor_handle(&motor_handle_1);
}
