#include "task.h"

#include "gpio_config.h"

#define TASK_NUM_MAX 2 // 任务数量

/*
    定义任务列表，在这里注册任务
*/
static volatile task_ctl_block_t task_table[] = {
    {0, 1, 1, MotorHadnler},
    // =======================================================
    {0, 5, 5, keyHandler},
    // =======================================================

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
