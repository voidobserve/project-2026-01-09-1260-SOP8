#include "motor_handle.h"
#include "adc.h"

#if 0
volatile u8 motor_0_status = MOTOR_STATUS_STOP; // 电机的状态机
volatile u8 motor_1_status = MOTOR_STATUS_STOP; // 电机的状态机

volatile u16 motor_0_dir_change_dly = 0; //
volatile u16 motor_1_dir_change_dly = 0; //

/*
    标志位，是否允许电机改变方向，
    处理函数会检测 motor_0_status 的值，更新电机工作对应的方向
    如果这个变量置一，后续的处理函数循环又会给 motor_0_is_working 置一
*/
volatile u8 motor_0_dir_change_enable = 0;
volatile u8 motor_1_dir_change_enable = 0; // 标志位，是否允许电机改变方向

volatile u8 motor_0_is_working = 0; // 电机是否正在工作
volatile u8 motor_1_is_working = 0; // 电机是否正在工作

void motor_0_change_status(motor_status_t status)
{
    if (status != MOTOR_STATUS_FORWARD &&
        status != MOTOR_STATUS_REVERSE)
    {
        return;
    }

    motor_0_status = status;
    motor_0_is_working = 0;
    motor_0_dir_change_enable = 1;
    motor_0_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;

    MOTOR_0_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
    MOTOR_0_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
}

void motor_1_change_status(motor_status_t status)
{
    if (status != MOTOR_STATUS_FORWARD &&
        status != MOTOR_STATUS_REVERSE)
    {
        return;
    }

    motor_1_status = status;
    motor_1_is_working = 0;
    motor_1_dir_change_enable = 1;
    motor_1_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;

    MOTOR_1_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
    MOTOR_1_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
}

void motor_0_handler(void)
{
    static volatile u16 motor_0_working_timing = 0;
    static volatile u16 motor_0_overcurrent_cnt = 0; // 电机过流检测计数值
    static volatile u16 adc_val_reverse = 0;
    static volatile u16 adc_val_forward = 0;
    //  ====================  延时切换电机方向  ========================
    //  ====================  需求电机停止几百ms再启动  ========================

    // printf("motor_0_dir_change_dly == %u\n", motor_0_dir_change_dly);
    if (motor_0_dir_change_dly > 0)
    {
        motor_0_dir_change_dly--;
    }
    else
    {
        if (motor_0_dir_change_enable)
        {
            motor_0_dir_change_enable = 0;
            if (motor_0_status == MOTOR_STATUS_FORWARD)
            {
                MOTOR_0_FORWARD_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
                MOTOR_0_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
                motor_0_working_timing = 0;
                motor_0_is_working = 1;
                motor_0_overcurrent_cnt = 0;
                // printf("setting motor 0 forward\n");
            }
            else if (motor_0_status == MOTOR_STATUS_REVERSE)
            {
                MOTOR_0_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
                MOTOR_0_REVERSE_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
                motor_0_working_timing = 0;
                motor_0_is_working = 1;
                motor_0_overcurrent_cnt = 0;
                // printf("setting motor 0 reverse\n");
            }
        }
    }

    // =================== ADC检测  ========================
    // 有新的数值更新，才会覆盖旧值：
    if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_0))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_0);
        adc_val_forward = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_0);
        // adc_val_forward = 4095;
        // printf("adc_val_forward = %u\n", adc_val_forward);
    }

    if (adc_get_update_flag(ADC_CHANNEL_INDEX_REVERSE_0))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_REVERSE_0);
        adc_val_reverse = adc_get_val(ADC_CHANNEL_INDEX_REVERSE_0);
    }

    // =================== 电机工作过程  ========================
    // 当电机工作时，检测是否有过流
    if (motor_0_is_working)
    {
        // 向前过流，需要反向
        if (adc_val_forward > MOTOR_OVERCURRENT_ADC_VAL)
        {
            motor_0_overcurrent_cnt++;
            // printf("motor 0 forward overcurrent cnt ++\n");
            if (motor_0_overcurrent_cnt > 200)
            {
                // printf("motor 0 forward overcurrent\n");
                // 反向
                motor_0_status = MOTOR_STATUS_REVERSE;
                motor_0_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;
                motor_0_is_working = 0;        // 停止电机流程
                motor_0_dir_change_enable = 1; // 这里使能，后续函数循环又会给 motor_0_is_working 置一

                // 反向结束
            }
        }

        // 回来原始位置停止工作
        if (adc_val_reverse > MOTOR_OVERCURRENT_ADC_VAL)
        {
            motor_0_overcurrent_cnt++;
            if (motor_0_overcurrent_cnt > 200)
            {
                // printf("motor 0 reverse overcurrent\n");
                motor_0_status = MOTOR_STATUS_STOP;
            }
        }

        // 工作过程中，防止检测不到过流。够时间就强制复位
        if (motor_0_working_timing < MOTOR_WORKING_OVER_TIME)
        {
            motor_0_working_timing++;
        }
        else
        {
            // 反向
            // printf("working over time\n");

            motor_0_status = MOTOR_STATUS_REVERSE;
            motor_0_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;
            motor_0_is_working = 0; // 停止电机流程
            motor_0_dir_change_enable = 1;
            // 反向结束
        }
    }

    // =================== 过流后，全部拉低，停止工作  ========================

    if (motor_0_status == MOTOR_STATUS_STOP)
    {
        motor_0_is_working = 0;

        MOTOR_0_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
        MOTOR_0_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);

        // printf("motor 0 stop\n");
    }
}

void motor_1_handler(void)
{
    static volatile u16 adc_val_reverse = 0;
    static volatile u16 adc_val_forward = 0;
    static volatile u16 working_timing = 0;
    static volatile u16 motor_overcurrent_cnt = 0; // 电机过流检测计数值

    //  ====================  延时切换电机方向  ========================
    //  ====================  需求电机停止几百ms再启动  ========================
    if (motor_1_dir_change_dly > 0)
    {
        motor_1_dir_change_dly--;
    }
    else
    {
        if (motor_1_dir_change_enable)
        {
            motor_1_dir_change_enable = 0;
            if (motor_1_status == MOTOR_STATUS_FORWARD)
            {
                MOTOR_1_FORWARD_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
                MOTOR_1_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
                working_timing = 0;
                motor_1_is_working = 1;
                motor_overcurrent_cnt = 0;
            }
            else if (motor_1_status == MOTOR_STATUS_REVERSE)
            {
                MOTOR_1_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
                MOTOR_1_REVERSE_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
                working_timing = 0;
                motor_1_is_working = 1;
                motor_overcurrent_cnt = 0;
            }
        }
    }

    // =================== ADC检测  ========================
    if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_1))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_1);
        adc_val_forward = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_1);
    }

    if (adc_get_update_flag(ADC_CHANNEL_INDEX_REVERSE_1))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_REVERSE_1);
        adc_val_reverse = adc_get_val(ADC_CHANNEL_INDEX_REVERSE_1);
    }

    // =================== 电机工作过程  ========================
    // 当电机工作时，检测是否有过流
    if (motor_1_is_working)
    {
        // 向前过流，需要反向
        if (adc_val_forward > MOTOR_OVERCURRENT_ADC_VAL)
        {
            motor_overcurrent_cnt++;
            if (motor_overcurrent_cnt > 200)
            {
                // printf("motor 1 forward overcurrent\n");
                // 反向
                motor_1_status = MOTOR_STATUS_REVERSE;
                motor_1_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;
                motor_1_is_working = 0; // 停止电机流程
                motor_1_dir_change_enable = 1;

                // 反向结束
            }
        }

        // 回来原始位置停止工作
        if (adc_val_reverse > MOTOR_OVERCURRENT_ADC_VAL)
        {

            motor_overcurrent_cnt++;
            if (motor_overcurrent_cnt > 200)
            {
                // printf("motor 1 reverse overcurrent\n");
                motor_1_status = MOTOR_STATUS_STOP;
            }
        }

        // 工作过程中，防止检测不到过流。够时间就强制复位
        if (working_timing < MOTOR_WORKING_OVER_TIME)
        {
            working_timing++;
        }
        else
        {
            // 反向
            // printf("working over time\n");
            motor_1_status = MOTOR_STATUS_REVERSE;
            motor_1_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;
            motor_1_is_working = 0; // 停止电机流程
            motor_1_dir_change_enable = 1;
            // 反向结束
        }
    }

    // =================== 过流后，全部拉低，停止工作  ========================

    if (motor_1_status == MOTOR_STATUS_STOP)
    {
        motor_1_is_working = 0;

        MOTOR_1_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
        MOTOR_1_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
    }
}
#endif

// 定义电机状态
enum
{
    // MOTOR_STATUS_IDLE = 0, // 刚上电，电机处于空闲状态
    MOTOR_STATUS_STOP = 0, // 电机停止工作
    MOTOR_STATUS_FORWARD,  // 电机正在正转
                           // MOTOR_STATUS_FORWARD_OVERCURRENT, // 电机正转，遇到过流而停止
                           // MOTOR_STATUS_FORWARD_OVERTIME, // 电机正转，超过指定时间而停止
                           // MOTOR_STATUS_FORWARD_STOP, // 电机正转，遇到过流 或者 超过指定时间 而停止

    MOTOR_STATUS_REVERSE,
    // MOTOR_STATUS_REVERSE_OVERCURRENT,
    // MOTOR_STATUS_REVERSE_OVERTIME,
    // MOTOR_STATUS_REVERSE_STOP, // 电机反转，遇到过流 或者 超过指定时间 而停止
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

volatile motor_handle_t motor_handle_0;
volatile motor_handle_t motor_handle_1;

void motor_0_stop(void)
{
    MOTOR_0_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
    MOTOR_0_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
}

void motor_0_forward(void)
{
    MOTOR_0_FORWARD_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
}

void motor_0_reverse(void)
{
    MOTOR_0_REVERSE_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
}

void motor_0_adc_val_update(void)
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

void motor_handle_init(void)
{
    motor_handle_0.dest_dir = MOTOR_DIR_NONE;
    motor_handle_0.status = MOTOR_STATUS_STOP;
    motor_handle_0.change_dir_enable = 0;
    motor_handle_0.forward = motor_0_forward;
    motor_handle_0.reverse = motor_0_reverse;
    motor_handle_0.stop = motor_0_stop;
    motor_handle_0.adc_val_update = motor_0_adc_val_update;

    // motor_handle_1.status = MOTOR_STATUS_STOP;
    // motor_handle_0.change_dir_enable = 0;
}

// 设置延时，准备让电机改变方向
void motor_change_dir_enable(motor_handle_t *motor_handle)
{
    motor_handle->change_dir_cnt = MOTOR_DIR_CHANGE_DLY_TIME;
    motor_handle->change_dir_enable = 1;
}

// 反转电机的方向，供按键处理函数调用
void motor_toggle_dir(motor_handle_t *motor_handle)
{
    u8 dest_dir = motor_handle->dest_dir;
    if (dest_dir == MOTOR_DIR_FORWARD)
    {
        dest_dir = MOTOR_DIR_REVERSE;
    }
    else // dest_dir == MOTOR_DIR_NONE || dest_dir == MOTOR_DIR_REVERSE
    {
        dest_dir = MOTOR_DIR_FORWARD;
    }

    motor_handle->dest_dir = dest_dir; // 设置目标方向

    motor_change_dir_enable(motor_handle);
    motor_handle->stop(); // 停止电机
}

void motor_change_dir(motor_handle_t *motor_handle, motor_dest_dir_t dest_dir)
{
    motor_handle->dest_dir = dest_dir; // 设置目标方向
    motor_change_dir_enable(motor_handle);
    motor_handle->stop(); // 停止电机
}

void motor_handle(motor_handle_t *motor_handle)
{
    if (motor_handle->change_dir_enable && motor_handle->change_dir_cnt > 0)
    {
        // 如果还在准备改变方向，不处理
        motor_handle->change_dir_cnt--;
    }

    if (motor_handle->change_dir_enable && motor_handle->change_dir_cnt == 0)
    {
        // 如果要改变电机方向，并且已经计数完毕
        motor_handle->change_dir_enable = 0;
        if (motor_handle->dest_dir == MOTOR_DIR_FORWARD)
        {
            motor_handle->status = MOTOR_STATUS_FORWARD;
            motor_handle->forward();
        }
        else
        {
            motor_handle->status = MOTOR_STATUS_REVERSE;
            motor_handle->reverse();
        }
    }

    // 在电机转动时，检测电机是否过流：
    motor_handle->adc_val_update(); // 更新ad值
    if (motor_handle->status == MOTOR_STATUS_FORWARD)
    {
        if (motor_handle->adc_val_forward > MOTOR_OVERCURRENT_ADC_VAL)
        {
            motor_handle->overcurrent_time_cnt++;
        }
        else
        {
            motor_handle->overcurrent_time_cnt = 0;
        }
    }
    else if (motor_handle->status == MOTOR_STATUS_REVERSE)
    {
        if (motor_handle->adc_val_reverse > MOTOR_OVERCURRENT_ADC_VAL)
        {
            motor_handle->overcurrent_time_cnt++;
        }
        else
        {
            motor_handle->overcurrent_time_cnt = 0;
        }
    }

    if (motor_handle->status != MOTOR_STATUS_STOP)
    {
        motor_handle->working_time_cnt++; // 工作时间计数
    }

    if (motor_handle_0.overcurrent_time_cnt > 200 ||
        motor_handle_0.working_time_cnt > MOTOR_WORKING_OVER_TIME)
    {
        // 过流且累计一定次数，停下电机
        // 电机转动且超过一定时间，也停下电机
        motor_handle_0.status = MOTOR_STATUS_STOP;
        motor_handle_0.stop();
    }
}
