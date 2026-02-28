#include "include.h"
#include "user_config.h"
#include "motor_status_feedback.h"
#include "motor_handle.h" // 包含 motor_status_t 的定义

static volatile u8 motor_0_status_buf[MOTOR_STATUS_FEEDBACK_PERIOD / MOTOR_STATUS_SCAN_PERIOD];
static volatile u8 motor_0_status_buf_index = 0;

static volatile u8 motor_1_status_buf[MOTOR_STATUS_FEEDBACK_PERIOD / MOTOR_STATUS_SCAN_PERIOD];
static volatile u8 motor_1_status_buf_index = 0;

void motor_0_status_buf_push(motor_status_t status)
{
    motor_0_status_buf[motor_0_status_buf_index] = status;
    motor_0_status_buf_index++;
    if (motor_0_status_buf_index >= ARRAY_SIZE(motor_0_status_buf))
    {
        motor_0_status_buf_index = 0;
    }
}

void motor_1_status_buf_push(motor_status_t status)
{
    motor_1_status_buf[motor_1_status_buf_index] = status;
    motor_1_status_buf_index++;
    if (motor_1_status_buf_index >= ARRAY_SIZE(motor_1_status_buf))
    {
        motor_1_status_buf_index = 0;
    }
}

motor_status_t motor_0_status_buf_get_weighted_average(void)
{
    u16 weighted_sum[3] = {0}; // 加权和，索引对应: [STOP, FORWARD, REVERSE]
    // u16 total_weight = 0;
    motor_status_t ret_status = MOTOR_STATUS_STOP;
    u16 max_weighted_sum = 0;
    u8 i;

    // 计算加权平均，越新的数据权重越高
    u8 buf_size = ARRAY_SIZE(motor_0_status_buf);
    for (i = 0; i < buf_size; i++)
    {
        // 计算当前数据在环形缓冲区中的实际位置
        u8 actual_index = (motor_0_status_buf_index + buf_size - 1 - i) % buf_size;
        // u8 actual_index;
        u8 status;
        // if (motor_0_status_buf_index == 0) 
        // {
        //     actual_index = buf_size - 1;
        // }
        // else
        // {
        //     motor_0_status_buf_index - 1;
        // }

        status = motor_0_status_buf[actual_index];

        if (status < 3) // 确保状态值有效
        {
            u16 weight = buf_size - i; // 权重递增：最新的数据权重最高
            weighted_sum[status] += weight;
            // total_weight += weight;
        }
    }

    // 找出加权和最大的状态
    for (i = 0; i < 3; i++)
    {
        if (weighted_sum[i] > max_weighted_sum)
        {
            max_weighted_sum = weighted_sum[i];
            ret_status = (motor_status_t)i;
        }

        printf("weighted_sum[%01d] = %02d\n", (u16)i, (u16)weighted_sum[i]);
    }

    return ret_status;
}

motor_status_t motor_1_status_buf_get_weighted_average(void)
{
    u16 weighted_sum[3] = {0}; // 加权和，索引对应: [STOP, FORWARD, REVERSE]
    u16 total_weight = 0;
    motor_status_t ret_status = MOTOR_STATUS_STOP;
    u16 max_weighted_sum = 0;
    u8 i;

    // 计算加权平均，越新的数据权重越高
    u8 buf_size = ARRAY_SIZE(motor_1_status_buf);
    for (i = 0; i < buf_size; i++)
    {
        // 计算当前数据在环形缓冲区中的实际位置
        u8 actual_index = (motor_1_status_buf_index + buf_size - 1 - i) % buf_size;
        u8 status = motor_1_status_buf[actual_index];

        if (status < 3) // 确保状态值有效
        {
            u16 weight = i + 1; // 权重递增：最新的数据权重最高
            weighted_sum[status] += weight;
            total_weight += weight;
        }
    }

    // 找出加权和最大的状态
    for (i = 0; i < 3; i++)
    {
        if (weighted_sum[i] > max_weighted_sum)
        {
            max_weighted_sum = weighted_sum[i];
            ret_status = (motor_status_t)i;
        }
    }

    return ret_status;
}

#if 0

// 快速模式检测：使用位运算进行多数投票
motor_status_t motor_0_status_buf_get_fast_mode(void)
{
    // 对于小缓冲区，可以用位图方式快速统计
    u16 bit_map = 0;
    
    // 将最近几次的状态压缩到位图中
    u8 buf_size = ARRAY_SIZE(motor_0_status_buf);
    u8 sample_count = (buf_size < 8) ? buf_size : 8; // 最多采样8个
    
    for (u8 i = 0; i < sample_count; i++)
    {
        u8 actual_index = (motor_0_status_buf_index + buf_size - 1 - i) % buf_size;
        u8 status = motor_0_status_buf[actual_index];
        
        if (status < 3)
        {
            bit_map |= (status << (i * 2)); // 每2位存储一个状态
        }
    }
    
    // 简单的多数投票逻辑
    u8 counts[3] = {0};
    for (u8 i = 0; i < sample_count; i++)
    {
        u8 status = (bit_map >> (i * 2)) & 0x03;
        if (status < 3) counts[status]++;
    }
    
    // 返回出现次数最多的状态
    if (counts[1] >= counts[0] && counts[1] >= counts[2])
        return MOTOR_STATUS_FORWARD;
    else if (counts[2] >= counts[0] && counts[2] >= counts[1])
        return MOTOR_STATUS_REVERSE;
    else
        return MOTOR_STATUS_STOP;
}

void motor_1_status_buf_push(motor_status_t status)
{ 
    motor_1_status_buf[motor_1_status_buf_index] = status;
    motor_1_status_buf_index++;
    if (motor_1_status_buf_index >= ARRAY_SIZE(motor_1_status_buf))
    {
        motor_1_status_buf_index = 0;
    }
}

motor_status_t motor_1_status_buf_get_weighted_average(void)
{
    u16 weighted_sum[3] = {0}; // 加权和，索引对应: [STOP, FORWARD, REVERSE]
    u16 total_weight = 0;
    motor_status_t ret_status = MOTOR_STATUS_STOP;
    u16 max_weighted_sum = 0;

    // 计算加权平均，越新的数据权重越高
    u8 buf_size = ARRAY_SIZE(motor_1_status_buf);
    for (u8 i = 0; i < buf_size; i++)
    {
        // 计算当前数据在环形缓冲区中的实际位置
        u8 actual_index = (motor_1_status_buf_index + buf_size - 1 - i) % buf_size;
        u8 status = motor_1_status_buf[actual_index];
        
        if (status < 3) // 确保状态值有效
        {
            u16 weight = i + 1; // 权重递增：最新的数据权重最高
            weighted_sum[status] += weight;
            total_weight += weight;
        }
    }

    // 找出加权和最大的状态
    for (u8 i = 0; i < 3; i++)
    {
        if (weighted_sum[i] > max_weighted_sum)
        {
            max_weighted_sum = weighted_sum[i];
            ret_status = (motor_status_t)i;
        }
    }

    return ret_status;
}

// 快速模式检测：使用位运算进行多数投票
motor_status_t motor_1_status_buf_get_fast_mode(void)
{
    // 对于小缓冲区，可以用位图方式快速统计
    u16 bit_map = 0;
    
    // 将最近几次的状态压缩到位图中
    u8 buf_size = ARRAY_SIZE(motor_1_status_buf);
    u8 sample_count = (buf_size < 8) ? buf_size : 8; // 最多采样8个
    
    for (u8 i = 0; i < sample_count; i++)
    {
        u8 actual_index = (motor_1_status_buf_index + buf_size - 1 - i) % buf_size;
        u8 status = motor_1_status_buf[actual_index];
        
        if (status < 3)
        {
            bit_map |= (status << (i * 2)); // 每2位存储一个状态
        }
    }
    
    // 简单的多数投票逻辑
    u8 counts[3] = {0};
    for (u8 i = 0; i < sample_count; i++)
    {
        u8 status = (bit_map >> (i * 2)) & 0x03;
        if (status < 3) counts[status]++;
    }
    
    // 返回出现次数最多的状态
    if (counts[1] >= counts[0] && counts[1] >= counts[2])
        return MOTOR_STATUS_FORWARD;
    else if (counts[2] >= counts[0] && counts[2] >= counts[1])
        return MOTOR_STATUS_REVERSE;
    else
        return MOTOR_STATUS_STOP;
}
#endif

/*
    向蓝牙ic反馈电机的状态

    由于电机的状态经常变化，
    需要采用类似滑动平均再加权的方式，来获取当前电机的状态
    例如：10ms扫描一次，每100ms通过滑动平均再加权的方式，得到状态
*/
// 系统初始化时调用
void motor_status_feedback_init(void)
{
    // 当前实现不需要初始化任何表格
}

void motor_status_scan(void)
{
    motor_0_status_buf_push(motor_0_status);
    motor_1_status_buf_push(motor_1_status);
}

void motor_status_feedback(void)
{
    // 可以选择使用快速模式或加权平均模式
    motor_status_t motor_0_status = motor_0_status_buf_get_weighted_average();
    motor_status_t motor_1_status = motor_1_status_buf_get_weighted_average();

    printf("motor_0_status: %02d\n", (u16)motor_0_status);
    printf("motor_1_status: %02d\n", (u16)motor_1_status);

    // TODO: 将电机状态发送给蓝牙IC
    // bluetooth_send_motor_status(motor_0_status, motor_1_status);
}
