#include "motor_handle.h"
#include "adc.h"

volatile u8 motor_0_status = MOTOR_STATUS_STOP; // 电机的状态机
volatile u8 motor_1_status = MOTOR_STATUS_STOP; // 电机的状态机

volatile u16 motor_0_dir_change_dly = 0; //
volatile u16 motor_1_dir_change_dly = 0; //

/*
    标志位，是否允许电机改变方向，
    处理函数会检测 motor_0_status 的值，更新电机工作对应的方向
*/
volatile u8 motor_0_dir_change_enable = 0;
volatile u8 motor_1_dir_change_enable = 0; // 标志位，是否允许电机改变方向

volatile u8 motor_0_is_working = 0; // 电机是否正在工作
volatile u8 motor_1_is_working = 0; // 电机是否正在工作

volatile motor_handle_t motor_handle_0 = {0};

void motor_0_change_status(motor_status_t status)
{
#if 0
    if (status != MOTOR_STATUS_FORWARD &&
        status != MOTOR_STATUS_REVERSE)
    {
        return;
    }

    motor_0_status = status;
    motor_0_dir_change_enable = 1;
    motor_0_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;
#endif

    if (status == MOTOR_STATUS_FORWARD &&
            (motor_handle_0.status == MOTOR_STATUS_STOP_BY_FORWARD_TIMEOUT ||
             motor_handle_0.status == MOTOR_STATUS_STOP_BY_FORWARD_OVERCURRENT) ||
        status == MOTOR_STATUS_REVERSE &&
            (motor_handle_0.status == MOTOR_STATUS_STOP_BY_REVERSE_TIMEOUT ||
             motor_handle_0.status == MOTOR_STATUS_STOP_BY_REVERSE_OVERCURRENT))
    {
        // 要控制电机正转，但是电机之前因为正转超时或正转过流而停止，不处理
        // 要控制电机反转，但是电机之前因为反转超时或反转过流而停止，不处理
        return;
    }

    motor_handle_0.status = status;
    motor_handle_0.dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;
    motor_handle_0.dir_change_enable = 1;
}

void motor_1_change_status(motor_status_t status)
{
    if (status != MOTOR_STATUS_FORWARD &&
        status != MOTOR_STATUS_REVERSE)
    {
        return;
    }

    motor_1_status = status;
    motor_1_dir_change_enable = 1;
    motor_1_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;
}

void motor_0_handler(void)
{
#if 1
    static volatile u16 motor_0_working_timing = 0;
    static volatile u16 motor_0_overcurrent_cnt = 0; // 电机过流检测计数值
    static volatile u16 adc_val_reverse = 0;
    static volatile u16 adc_val_forward = 0;
    //  ====================  延时切换电机方向  ========================
    //  ====================  需求电机停止几百ms再启动  ========================
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
            }
            else if (motor_0_status == MOTOR_STATUS_REVERSE)
            {
                MOTOR_0_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
                MOTOR_0_REVERSE_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
                motor_0_working_timing = 0;
                motor_0_is_working = 1;
                motor_0_overcurrent_cnt = 0;
            }
        }
    }

    // =================== ADC检测  ========================
    // 有新的数值更新，才会覆盖旧值：
    if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_0))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_0);
        adc_val_forward = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_0);
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
                printf("motor 0 forward overcurrent\n");
                // 反向
                motor_0_status = MOTOR_STATUS_REVERSE;
                motor_0_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;
                motor_0_is_working = 0; // 停止电机流程
                motor_0_dir_change_enable = 1;

                // 反向结束
            }
        }

        // 回来原始位置停止工作
        if (adc_val_reverse > MOTOR_OVERCURRENT_ADC_VAL)
        {
            motor_0_overcurrent_cnt++;
            if (motor_0_overcurrent_cnt > 200)
            {
                printf("motor 0 reverse overcurrent\n");
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
            printf("working over time\n");

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
    }
#endif


#if 0
    static volatile u16 adc_val_reverse = 0;
    static volatile u16 adc_val_forward = 0;

    // =================== ADC检测  ========================
    // 有新的数值更新，才会覆盖旧值：
    if (adc_get_update_flag(ADC_CHANNEL_INDEX_FORWARD_0))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_FORWARD_0);
        adc_val_forward = adc_get_val(ADC_CHANNEL_INDEX_FORWARD_0);
        // printf("adc_val_forward = %u\n", adc_val_forward);
    }

    if (adc_get_update_flag(ADC_CHANNEL_INDEX_REVERSE_0))
    {
        adc_clear_update_flag(ADC_CHANNEL_INDEX_REVERSE_0);
        adc_val_reverse = adc_get_val(ADC_CHANNEL_INDEX_REVERSE_0);
    }

    if (adc_val_forward > MOTOR_OVERCURRENT_ADC_VAL)
    {

        motor_handle_0.overcurrent_time_cnt++;
        if (motor_handle_0.overcurrent_time_cnt > 200)
        {
            printf("motor 0 forward overcurrent\n");

            // 反向
            motor_0_status = MOTOR_STATUS_REVERSE;
            motor_0_dir_change_dly = MOTOR_DIR_CHANGE_DLY_TIME;
            motor_0_is_working = 0; // 停止电机流程
            motor_0_dir_change_enable = 1;

            // 反向结束
        }
    }

    //  ====================  延时切换电机方向  ========================
    //  ====================  需求电机停止几百ms再启动  ========================
    if (motor_handle_0.dir_change_enable && motor_handle_0.dir_change_dly > 0)
    {
        motor_handle_0.dir_change_dly--;
    }
    else
    {
        motor_handle_0.dir_change_enable = 0; // 清空标志位

        if (motor_0_status == MOTOR_STATUS_FORWARD)
        {
            MOTOR_0_FORWARD_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
            MOTOR_0_REVERSE_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
            motor_0_working_timing = 0;
            motor_0_is_working = 1;
            motor_0_overcurrent_cnt = 0;
        }
        else if (motor_0_status == MOTOR_STATUS_REVERSE)
        {
            MOTOR_0_FORWARD_PWM_DUTY_SET(PWM_DUTY_VAL_0_PERCENT);
            MOTOR_0_REVERSE_PWM_DUTY_SET(MOTOR_WORKING_PWM_DUTY_VAL);
            motor_0_working_timing = 0;
            motor_0_is_working = 1;
            motor_0_overcurrent_cnt = 0;
        }
    }

    // =================== 电机工作过程  ========================
    // 当电机工作时，检测是否有过流
    if (motor_0_is_working)
    {
        // 向前过流，需要反向

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
    }
#endif
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
