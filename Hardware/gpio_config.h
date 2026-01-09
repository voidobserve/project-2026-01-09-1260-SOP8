#ifndef __GPIO_CINFIG_H__
#define __GPIO_CINFIG_H__


#include "include.h"


#define motor_forward_pin  P21
#define check_forward_pin  P22

#define motor_backward_pin P06
#define check_backward_pin P05


typedef enum{

    MOTOR_STOP = 0,
    MOTOR_FORWARD = 1,
    MOTOR_BACKWARD = 2,

}MOTOR_STATE;



 


void ZD_GPIO_Init(void);
void MotorHadnler(void);
void keyHandler(void);




#endif

