#ifndef __TASK_H__
#define __TASK_H__

#include "include.h"

// 定义任务控制块
typedef struct
{  
    
    u8 is_task_enable;                // 调度标志 1：调度 0：挂起
    u16 task_time_slice;              // 时间片周期，用于递增计数
    const u16 target_task_time_slice; // 时间片目标值，用于递增计数
    void (*task_func)(void);          // 函数指针，保存任务函数地址

} task_ctl_block_t;

void task_handle(void);
void task_schedule_tick(void);

#endif
