#include "task.h"

#define TASK_NUM_MAX 2

static TaskComps_t TaskComps[] =
    {

        {0, 1, 1, MotorHadnler},
        {0, 5, 5, keyHandler},

};

void TaskHandler(void)
{
    u8 i = 0;

    for (i = 0; i < TASK_NUM_MAX; i++)
    {

        if (TaskComps[i].run)
        {

            TaskComps[i].run = 0;
            TaskComps[i].pTaskFunc();
        }
    }
}

void TaskSchedule(void)
{

    u8 i = 0;

    for (i = 0; i < TASK_NUM_MAX; i++)
    {

        if (TaskComps[i].TimCount)
        {

            TaskComps[i].TimCount--;
            if (TaskComps[i].TimCount == 0)
            {

                TaskComps[i].TimCount = TaskComps[i].TimRload;
                TaskComps[i].run = 1;
            }
        }
    }
}
