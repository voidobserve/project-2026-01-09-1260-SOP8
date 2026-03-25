#include "include.h"
#include "user_config.h"
#include "motor_status_feedback.h"
#include "motor_handle.h" // 包含 motor_status_t 的定义
#include "uart.h"
#include "uart_data_handle.h"

static volatile u8 motor_0_status_buf[MOTOR_STATUS_FEEDBACK_PERIOD / MOTOR_STATUS_SCAN_PERIOD];
static volatile u8 motor_0_status_buf_index = 0;

static volatile u8 motor_1_status_buf[MOTOR_STATUS_FEEDBACK_PERIOD / MOTOR_STATUS_SCAN_PERIOD];
static volatile u8 motor_1_status_buf_index = 0;

// 往加权平均数组放入数据：
void motor_0_status_buf_push(feedback_motor_status_t status)
{
    motor_0_status_buf[motor_0_status_buf_index] = status;
    motor_0_status_buf_index++;
    if (motor_0_status_buf_index >= ARRAY_SIZE(motor_0_status_buf))
    {
        motor_0_status_buf_index = 0;
    }
}

void motor_1_status_buf_push(feedback_motor_status_t status)
{
    motor_1_status_buf[motor_1_status_buf_index] = status;
    motor_1_status_buf_index++;
    if (motor_1_status_buf_index >= ARRAY_SIZE(motor_1_status_buf))
    {
        motor_1_status_buf_index = 0;
    }
}

// 求出滑动加权平均的状态
feedback_motor_status_t motor_0_status_buf_get_weighted_average(void)
{
    u16 weighted_sum[FEEDBACK_MOTOR_STATUS_TOTAL_NUM] = {0}; // 加权和，索引对应:  feedback_motor_status_t
    feedback_motor_status_t ret_status = FEEDBACK_MOTOR_STATUS_NONE;
    u16 max_weighted_sum = 0;
    u8 i;
    u16 weight;
    u8 status;
    u8 actual_index;

    // 计算加权平均，越新的数据权重越高
    u8 buf_size = ARRAY_SIZE(motor_0_status_buf);
    for (i = 0; i < buf_size; i++)
    {
        // 计算当前数据在环形缓冲区中的实际位置
        /*
            当前最新的数据索引 == motor_0_status_buf_index - 1
            当前第二新的数据索引 == motor_0_status_buf_index - 2

            motor_0_status_buf_index：当前写入位置
            buf_size - 1：回退到最新的已写入数据位置
            - i：根据循环次数向前移动，获取更旧的数据
            % buf_size：确保索引在缓冲区范围内循环
        */
        actual_index = (motor_0_status_buf_index + buf_size - 1 - i) % buf_size;
        status = motor_0_status_buf[actual_index];

        if (status < FEEDBACK_MOTOR_STATUS_TOTAL_NUM) // 确保状态值有效
        {
            weight = buf_size - i; // 权重递增：最新的数据权重最高，数值最大
            weighted_sum[status] += weight;
        }
    }

    // 找出加权和最大的状态
    for (i = 0; i < FEEDBACK_MOTOR_STATUS_TOTAL_NUM; i++)
    {
        if (weighted_sum[i] > max_weighted_sum)
        {
            max_weighted_sum = weighted_sum[i];
            ret_status = (feedback_motor_status_t)i;
        }

        // printf("weighted_sum[%01d] = %02d\n", (u16)i, (u16)weighted_sum[i]);
    }

    return ret_status;
}

feedback_motor_status_t motor_1_status_buf_get_weighted_average(void)
{
    u16 weighted_sum[FEEDBACK_MOTOR_STATUS_TOTAL_NUM] = {0}; // 加权和，索引对应: feedback_motor_status_t
    feedback_motor_status_t ret_status = FEEDBACK_MOTOR_STATUS_NONE;
    u16 max_weighted_sum = 0; // 存放最大的加权和
    u8 i;
    u16 weight; // 存放单个状态的权重
    u8 status;  // 存放电机状态值（在 motor_status_t中有定义）
    u8 actual_index;

    // 计算加权平均，越新的数据权重越高
    u8 buf_size = ARRAY_SIZE(motor_1_status_buf);
    for (i = 0; i < buf_size; i++)
    {
        // 计算当前数据在环形缓冲区中的实际位置
        /*
            当前最新的数据索引 == motor_x_status_buf_index - 1
            当前第二新的数据索引 == motor_x_status_buf_index - 2

            motor_x_status_buf_index ：当前写入位置
            buf_size - 1：回退到最新的已写入数据位置
            - i：根据循环次数向前移动，获取更旧的数据
            % buf_size：确保索引在缓冲区范围内循环
        */
        actual_index = (motor_1_status_buf_index + buf_size - 1 - i) % buf_size;
        status = motor_1_status_buf[actual_index];

        if (status < FEEDBACK_MOTOR_STATUS_TOTAL_NUM) // 确保状态值有效
        {
            weight = buf_size - i; // 权重递增：最新的数据权重最高
            weighted_sum[status] += weight;
        }
    }

    // 找出加权和最大的状态
    for (i = 0; i < FEEDBACK_MOTOR_STATUS_TOTAL_NUM; i++)
    {
        if (weighted_sum[i] > max_weighted_sum)
        {
            max_weighted_sum = weighted_sum[i];
            ret_status = (feedback_motor_status_t)i;
        }

        // printf("weighted_sum[%01d] = %02d\n", (u16)i, (u16)weighted_sum[i]);
    }

    return ret_status;
}

/*
    向蓝牙ic反馈电机的状态

    由于电机的状态经常变化，
    需要采用类似滑动平均再加权的方式，来获取当前电机的状态
    例如：10ms扫描一次，每100ms通过滑动平均再加权的方式，得到状态
*/
// 系统初始化时调用
void motor_status_feedback_init(void)
{
    u8 i;
    for (i = 0; i < (MOTOR_STATUS_FEEDBACK_PERIOD / MOTOR_STATUS_SCAN_PERIOD); i++)
    {
        motor_0_status_buf[i] = FEEDBACK_MOTOR_STATUS_NONE;
        motor_1_status_buf[i] = FEEDBACK_MOTOR_STATUS_NONE;
    }
}

// 扫描电机状态，将电机状态放入缓冲区
void motor_status_scan(void)
{
    volatile feedback_motor_status_t feedback_status = FEEDBACK_MOTOR_STATUS_NONE;
    volatile u8 dest_dir = motor_handle_0.dest_dir;
    volatile u8 status = motor_handle_0.status;

    if (status == MOTOR_STATUS_STOP)
    {
        // 如果现在是停止状态，要判断是正向停止还是反向停止，还是刚上电没有在转动
        if (dest_dir == MOTOR_DIR_FORWARD)
        {
            feedback_status = FEEDBACK_MOTOR_STATUS_FORWARD_STOP;
        }
        else if (dest_dir == MOTOR_DIR_REVERSE)
        {
            feedback_status = FEEDBACK_MOTOR_STATUS_REVERSE_STOP;
        }
        else
        {
            // 这里可以省略
            // feedback_status = FEEDBACK_MOTOR_STATUS_NONE;
        }
    }
    else if (status == MOTOR_STATUS_FORWARD)
    {
        feedback_status = FEEDBACK_MOTOR_STATUS_FORWARD;
    }
    else if (status == MOTOR_STATUS_REVERSE)
    {
        feedback_status = FEEDBACK_MOTOR_STATUS_REVERSE;
    }

    // printf("motor 0 feedback status: %02d\n", (u16)feedback_status);
    if (motor_handle_0.is_status_need_to_feedback)
    {
        // 电机因为要换方向而短暂停止时，不把这个状态反馈给蓝牙ic
        motor_0_status_buf_push(feedback_status);
    }

    // 另外一个电机，这里要重新初始化相关变量：
    feedback_status = FEEDBACK_MOTOR_STATUS_NONE;
    dest_dir = motor_handle_1.dest_dir;
    status = motor_handle_1.status;
    if (status == MOTOR_STATUS_STOP)
    {
        if (dest_dir == MOTOR_DIR_FORWARD)
        {
            feedback_status = FEEDBACK_MOTOR_STATUS_FORWARD_STOP;
        }
        else if (dest_dir == MOTOR_DIR_REVERSE)
        {
            feedback_status = FEEDBACK_MOTOR_STATUS_REVERSE_STOP;
        }
        else
        {
            // 这里可以省略
            // feedback_status = FEEDBACK_MOTOR_STATUS_NONE;
        }
    }
    else if (status == MOTOR_STATUS_FORWARD)
    {
        feedback_status = FEEDBACK_MOTOR_STATUS_FORWARD;
    }
    else if (status == MOTOR_STATUS_REVERSE)
    {
        feedback_status = FEEDBACK_MOTOR_STATUS_REVERSE;
    }

    // printf("motor 1 feedback status: %02d\n", (u16)feedback_status);
    if (motor_handle_1.is_status_need_to_feedback)
    {
        motor_1_status_buf_push(feedback_status);
    }
}

// void motor_status_feedback_(void)
// {
//     uart0_sendbyte();
// }

void motor_status_feedback(void)
{
    // 加权平均，求得电机状态
    feedback_motor_status_t motor_0_status = motor_0_status_buf_get_weighted_average();
    feedback_motor_status_t motor_1_status = motor_1_status_buf_get_weighted_average();
    u8 check_sum = 0;

    // printf("motor_0_status: %02d\n", (u16)motor_0_status);
    // printf("motor_1_status: %02d\n", (u16)motor_1_status);

    // 将电机状态反馈给蓝牙ic：
    check_sum = UART_DATA_HANDLE_FORMAT_HEAD + 0x05 + 0x01 + motor_0_status;
    uart0_sendbyte(UART_DATA_HANDLE_FORMAT_HEAD); // 发送格式头
    uart0_sendbyte(0x05);                         // 整一帧数据的长度
    uart0_sendbyte(0x01);                         // 电机索引
    uart0_sendbyte(motor_0_status);               // 电机状态
    uart0_sendbyte(check_sum);

    check_sum = UART_DATA_HANDLE_FORMAT_HEAD + 0x05 + 0x02 + motor_1_status;
    uart0_sendbyte(UART_DATA_HANDLE_FORMAT_HEAD); // 发送格式头
    uart0_sendbyte(0x05);                         // 整一帧数据的长度
    uart0_sendbyte(0x02);                         // 电机索引
    uart0_sendbyte(motor_1_status);               // 电机状态
    uart0_sendbyte(check_sum);
}

/*
    测试方法：

        定时调用 motor_0_status_buf_push() 、motor_1_status_buf_push()，
    存放电机状态
        定时调用 motor_0_status_buf_get_weighted_average() 、
    motor_1_status_buf_get_weighted_average()，获取求得的电机状态

        可以在 motor_x_status_buf_push() 中打印每个状态对应的加权和
        在 motor_x_status_buf_get_weighted_average() 中打印求得的电机状态
*/
// motor_0_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_0_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_0_status_buf_push(MOTOR_STATUS_STOP);
// motor_0_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_0_status_buf_push(MOTOR_STATUS_REVERSE);

// motor_0_status_buf_push(MOTOR_STATUS_STOP);
// motor_0_status_buf_push(MOTOR_STATUS_REVERSE);
// motor_0_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_0_status_buf_push(MOTOR_STATUS_STOP);
// motor_0_status_buf_push(MOTOR_STATUS_STOP);
// ===

// motor_0_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_0_status_buf_push(MOTOR_STATUS_REVERSE);
// motor_0_status_buf_push(MOTOR_STATUS_STOP);
// motor_0_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_0_status_buf_push(MOTOR_STATUS_REVERSE);

// motor_0_status_buf_push(MOTOR_STATUS_STOP);
// motor_0_status_buf_push(MOTOR_STATUS_REVERSE);
// motor_0_status_buf_push(MOTOR_STATUS_STOP);
// motor_0_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_0_status_buf_push(MOTOR_STATUS_FORWARD);
// ========================

// motor_1_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_1_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_1_status_buf_push(MOTOR_STATUS_STOP);
// motor_1_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_1_status_buf_push(MOTOR_STATUS_REVERSE);

// motor_1_status_buf_push(MOTOR_STATUS_STOP);
// motor_1_status_buf_push(MOTOR_STATUS_REVERSE);
// motor_1_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_1_status_buf_push(MOTOR_STATUS_STOP);
// motor_1_status_buf_push(MOTOR_STATUS_STOP);

// motor_1_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_1_status_buf_push(MOTOR_STATUS_REVERSE);
// motor_1_status_buf_push(MOTOR_STATUS_STOP);
// motor_1_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_1_status_buf_push(MOTOR_STATUS_REVERSE);

// motor_1_status_buf_push(MOTOR_STATUS_STOP);
// motor_1_status_buf_push(MOTOR_STATUS_REVERSE);
// motor_1_status_buf_push(MOTOR_STATUS_STOP);
// motor_1_status_buf_push(MOTOR_STATUS_FORWARD);
// motor_1_status_buf_push(MOTOR_STATUS_FORWARD);