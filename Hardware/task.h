#ifndef __TASK_H__
#define __TASK_H__


#include "include.h"



typedef struct{

    u8 run;              // 调度标志 1：调度 0：挂起
    u16 TimCount;        // 时间片周期，用于递减计数
    u16 TimRload;        // 时间片周期，用于重载
    void (*pTaskFunc)(void);  // 函数指针，保存任务函数地址


}TaskComps_t;




void TaskHandler(void);
void TaskSchedule(void);

#endif

