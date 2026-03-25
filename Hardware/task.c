#include "task.h"
#include "user_config.h"
#include "adc.h"
#include "key.h"
#include "motor_handle.h"
#include "motor_status_feedback.h"

// #define TASK_NUM_MAX 2 // 任务数量

/*
    定义任务列表，在这里注册任务
*/
void task_debug(void);
static volatile task_ctl_block_t task_table[] = {
    {0, 0, 1, motor_handle_func_0}, //
    {0, 0, 1, motor_handle_func_1},
    // // =======================================================
    {0, 0, 5, key0_scan_handle},
    {0, 0, 5, key1_scan_handle},
    // =======================================================
    {0, 0, MOTOR_STATUS_SCAN_PERIOD, motor_status_scan}, // USER_TO_DO 测试时屏蔽

    // {0, 0, 1000, motor_status_feedback}, // USER_TO_DO 测试时使用 1000ms
    {0, 0, MOTOR_STATUS_FEEDBACK_PERIOD, motor_status_feedback}, // USER_TO_DO 测试时屏蔽，实际需要恢复

#if USER_DEBUG_ENABLE
// {0, 0, 100, task_debug},
#endif
};

// 任务处理，由主循环调用
void task_handle(void)
{
    u8 i = 0;

    for (i = 0; i < (sizeof(task_table) / sizeof(task_ctl_block_t)); i++)
    {
        // 如果有任务就绪，调用该任务
        if (task_table[i].is_task_enable)
        {
            task_table[i].is_task_enable = 0;
            task_table[i].task_func();
        }
    }
}

// 任务调度，由定时器中断调用（跑时间片计数）
void task_schedule_tick(void)
{
    u8 i = 0;
    for (i = 0; i < (sizeof(task_table) / sizeof(task_ctl_block_t)); i++)
    {
        task_table[i].task_time_slice++;
        if (task_table[i].task_time_slice >= task_table[i].target_task_time_slice)
        {
            task_table[i].task_time_slice = 0;
            task_table[i].is_task_enable = 1;
        }
    }
}

#if USER_DEBUG_ENABLE
void task_debug(void)
{
}
#endif
