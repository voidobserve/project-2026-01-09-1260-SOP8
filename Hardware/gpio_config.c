#include "gpio_config.h"



volatile u8 motorState = MOTOR_STOP;
#define STMR0_PEROID_VAL               (SYSCLK/1/20000 - 1)                  // 周期值=系统时钟/分频/频率 - 1
#define STMR1_PEROID_VAL               (SYSCLK/1/20000 - 1)                  // 周期值=系统时钟/分频/频率 - 1



#define no_pwm_fun 0



void ZD_GPIO_Init(void)
{


#if (no_pwm_fun )
    P2_MD0   &= ~(GPIO_P21_MODE_SEL(0x03) | GPIO_P22_MODE_SEL(0x03));
    P2_MD0   |=   GPIO_P21_MODE_SEL(0x01) | GPIO_P22_MODE_SEL(0x00);


    P0_MD1   &= ~(GPIO_P06_MODE_SEL(0x03) | GPIO_P05_MODE_SEL(0x03));
    P0_MD1   |=   GPIO_P06_MODE_SEL(0x01) | GPIO_P05_MODE_SEL(0x00);  

    motor_forward_pin = 0;
    motor_backward_pin = 0;
#else

// 配置STIMER0的PWM端口：P21--STMR0_PWM
    P2_MD0   &= ~GPIO_P21_MODE_SEL(0x03);
    P2_MD0   |=  GPIO_P21_MODE_SEL(0x01);
    FOUT_S21  =  GPIO_FOUT_STMR0_PWMOUT;                                    // 选择stmr0_pwmout
    // STIMER0配置1kHz PWM
    STMR0_PSC     =  STMR_PRESCALE_VAL(0x00);                               // 不分频
    STMR0_PRH     =  STMR_PRD_VAL_H((STMR0_PEROID_VAL >> 8) & 0xFF);        // 周期高八位寄存器
    STMR0_PRL     =  STMR_PRD_VAL_L((STMR0_PEROID_VAL >> 0) & 0xFF);        // 周期低八位寄存器
    STMR0_CMPAH   =  STMR_CMPA_VAL_H(((0) >> 8) & 0xFF);   // 比较值A点高八位寄存器
    STMR0_CMPAL   =  STMR_CMPA_VAL_L(((0) >> 0) & 0xFF);   // 比较值A点低八位寄存器
    STMR_PWMVALA &= ~STMR_0_PWMVALA(0x1);                                   // 计数CNT小于比较值A,PWM输出1,大于等于输出0
    STMR_PWMEN   |=  STMR_0_PWM_EN(0x1);                                    // PWM输出使能
    STMR_CNTMD   |=  STMR_0_CNT_MODE(0x1);                                  // 选择连续计数模式
    STMR_LOADEN  |=  STMR_0_LOAD_EN(0x1);                                   // 自动装载使能
    STMR_CNTCLR  |=  STMR_0_CNT_CLR(0x1);                                   // 计数清零
    STMR_CNTEN   |=  STMR_0_CNT_EN(0x1);                                    // 计数使能
    
    // 配置STIMER1的PWM端口：P22--STMR1_PWM
    P0_MD1   &= ~GPIO_P06_MODE_SEL(0x03);
    P0_MD1   |=  GPIO_P06_MODE_SEL(0x01);
    FOUT_S06  =  GPIO_FOUT_STMR1_PWMOUT;                                    // 选择stmr1_pwmout 
    // STIMER1配置1kHz PWM
    STMR1_PSC     =  STMR_PRESCALE_VAL(0x00);                               // 不分频
    STMR1_PRH     =  STMR_PRD_VAL_H((STMR1_PEROID_VAL >> 8) & 0xFF);        // 周期高八位寄存器
    STMR1_PRL     =  STMR_PRD_VAL_L((STMR1_PEROID_VAL >> 0) & 0xFF);        // 周期低八位寄存器
    STMR1_CMPAH   =  STMR_CMPA_VAL_H(((0) >> 8) & 0xFF);   // 比较值A点高八位寄存器
    STMR1_CMPAL   =  STMR_CMPA_VAL_L(((0) >> 0) & 0xFF);   // 比较值A点低八位寄存器
    STMR_PWMVALA &= ~STMR_1_PWMVALA(0x1);                                   // 计数CNT小于比较值A,PWM输出1,大于等于输出0
    STMR_PWMEN   |=  STMR_1_PWM_EN(0x1);                                    // PWM输出使能
    STMR_CNTMD   |=  STMR_1_CNT_MODE(0x1);                                  // 选择连续计数模式
    STMR_LOADEN  |=  STMR_1_LOAD_EN(0x1);                                   // 自动装载使能
    STMR_CNTCLR  |=  STMR_1_CNT_CLR(0x1);                                   // 计数清零
    STMR_CNTEN   |=  STMR_1_CNT_EN(0x1);                                    // 计数使能




P2_MD0   &= ~( GPIO_P22_MODE_SEL(0x03));
P2_MD0   |=    GPIO_P22_MODE_SEL(0x00);
P0_MD1   &= ~(GPIO_P05_MODE_SEL(0x03));
P0_MD1   |=   GPIO_P05_MODE_SEL(0x00);  
#endif



    /* 自锁开关 */
    P3_MD0   &= ~(GPIO_P31_MODE_SEL(0x03));
    P3_PU |= GPIO_P31_PULL_UP(0x01);

    //ad按键  电量检测  充电检测
    P0_MD1 &= ~GPIO_P05_MODE_SEL(0x03);
    P0_MD1 |=  GPIO_P05_MODE_SEL(0x3);                       // P02设为模拟模式

    P2_MD0 &= ~GPIO_P22_MODE_SEL(0x03);
    P2_MD0 |=  GPIO_P22_MODE_SEL(0x3);                       // P05设为模拟模式


    //ADC配置
    ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) | ADC_EXREF_SEL(0x1));   // 关闭外部参考电压
    ADC_ACON1 |= ADC_VREF_SEL(0x5) |                          // 选择内部参考电压VCC
                 ADC_TEN_SEL(0x3);

    ADC_ACON0  = ADC_CMP_EN(0x1)  |                           // 打开ADC中的CMP使能信号
                ADC_BIAS_EN(0x1) |                            // 打开ADC偏置电流能使信号
                ADC_BIAS_SEL(0x1);
   
    ADC_CHS0   = ADC_ANALOG_CHAN(0x05) |                      // P02通路     选择对应的引脚配置
                ADC_EXT_SEL(0x0);                             // 选择外部通路
    ADC_CFG0  |= ADC_CHAN0_EN(0x1) |                          // 使能通道0转换
                ADC_EN(0x1);                                  // 使能A/D转换
    
  
    ADC_CHS1   = ADC_ANALOG_CHAN(0x12) |                      // P05通路     选择对应的引脚配置
                ADC_EXT_SEL(0x0);                             // 选择外部通路
    ADC_CFG0  |= ADC_CHAN1_EN(0x1) |                          // 使能通道1转换
                ADC_EN(0x1);                                  // 使能A/D转换

   
    delay_ms(1);                                              // 等待ADC模块配置稳定，需要等待20us以上

    P2_DRV1  = GPIO_P21_DRV_SEL(0x1F);
    P2_DRV2  = GPIO_P22_DRV_SEL(0x1F);
    P1_DRV7  = GPIO_P17_DRV_SEL(0x1F);

} 

void forward_pwm(u16 val)
{
    STMR0_CMPAH   =  STMR_CMPA_VAL_H((((val)) >> 8) & 0xFF);   // 比较值A点高八位寄存器
    STMR0_CMPAL   =  STMR_CMPA_VAL_L((((val)) >> 0) & 0xFF);   // 比较值A点低八位寄存器
    STMR_LOADEN  |=  STMR_0_LOAD_EN(0x1);                                   // 自动装载使能


}


void backward_pwm(u16 val)
{
    STMR1_CMPAH   =  STMR_CMPA_VAL_H((((val)) >> 8) & 0xFF);   // 比较值A点高八位寄存器
    STMR1_CMPAL   =  STMR_CMPA_VAL_L((((val)) >> 0) & 0xFF);   // 比较值A点低八位寄存器
    STMR_LOADEN  |=  STMR_1_LOAD_EN(0x1);                                   // 自动装载使能

    
}



u16 g_delay_change = 0;
u8 allow_change = 0;
u8 allow_check = 0;
u16 check_overCurrent_time = 0;

#define change_delay_time 300
#define overCurrent      (1657) // 1.7v
#define  qudongDuty (2040)  //20k 85  
#define  work_deadline  (2500) 


void keyHandler(void)
{

    //使用边沿检测
    u8 curr = P31;
    static u8 last = (u8)-1;

    if (curr != last)
    {
        if (last != (u8)-1)
        {
            if (curr)
            {
                // 上升沿



                motorState = MOTOR_FORWARD;
                g_delay_change = change_delay_time;
               
                #if (no_pwm_fun )
                motor_forward_pin = 0;
                motor_backward_pin = 0;
                #else 
                forward_pwm(0);
                backward_pwm(0);
                #endif


                allow_change = 1;
                allow_check = 0;

            }
            else
            {
                // 下降沿

//电机不转

              




            }
        }
        else
        {
            // 第一次上电，忽略
            motorState = MOTOR_STOP;
        }

        last = curr;
      
    }


}



u16  get_adc_value(void) 
{


    u16 adc0_val;
 
    ADC_CFG0 |= ADC_CHAN0_TRG(0x1);                       // 触发ADC0转换
    if(!(ADC_STA & ADC_CHAN0_DONE(0x1)))return;
    ADC_STA  = ADC_CHAN0_DONE(0x1);                       // 清除ADC0转换完成标志位
    adc0_val = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4);     // 读取ADC0的值
    
    return adc0_val;

}

u16  get_adc_value1(void) 
{


    u16 adc1_val;
 
    ADC_CFG0 |= ADC_CHAN1_TRG(0x1);                       // 触发ADC0转换
    if(!(ADC_STA & ADC_CHAN1_DONE(0x1)))return;
    ADC_STA  = ADC_CHAN1_DONE(0x1);                       // 清除ADC0转换完成标志位
    adc1_val = (ADC_DATAH1 << 4) | (ADC_DATAL1 >> 4);     // 读取ADC0的值
    
    return adc1_val;

}

u8  qiangzhi_back = 0;
u16  qiangzhi_cnt = 0;
void MotorHadnler(void)
{

	
    static u16 ad1 = 0;
    static u16 ad2 = 0;
    static u16 working_timing = 0;




    //  ====================  延时切换电机方向  ========================
    //  ====================  需求电机停止几百ms再启动  ========================
    if(g_delay_change > 0) {
        g_delay_change--;

    } else {

        if(allow_change){

            allow_change = 0;
            if(motorState == MOTOR_FORWARD ) {

                #if (no_pwm_fun )
                motor_forward_pin = 1;
                motor_backward_pin = 0; 

                #else
                forward_pwm(qudongDuty);
                backward_pwm(0);
                #endif

                working_timing = 0;
                allow_check = 1;
                check_overCurrent_time = 0;

            } else if(motorState == MOTOR_BACKWARD ) {
               

                #if (no_pwm_fun )      
                motor_forward_pin = 0;
                motor_backward_pin = 1;
                #else 

                forward_pwm(0);
                backward_pwm(qudongDuty);
                #endif


                working_timing = 0;
                allow_check = 1;
                check_overCurrent_time = 0;


                qiangzhi_back = 0;
            }

        }

    }
    
    
    
    
    // =================== ADC检测  ========================
    ad1 = get_adc_value();
    ad2 = get_adc_value1();  //向前时，碰到过流检测




    // =================== 电机工作过程  ========================
    //当电机工作时，检测是否有过流
    if(allow_check) {

   
        //向前过流，需要反向
        if(ad2 > overCurrent ){
            
            check_overCurrent_time++;
            if(check_overCurrent_time > 200) {

                //反向
                motorState = MOTOR_BACKWARD;
                g_delay_change = change_delay_time;
                allow_check = 0;  //停止电机流程
                allow_change = 1;
                
                //反向结束


                
            }
        }

        //回来原始位置停止工作
        if(ad1 > overCurrent) {

            
            check_overCurrent_time++;
            if(check_overCurrent_time > 200) {

                motorState = MOTOR_STOP;
                
            }
        }

    
  

        // 工作过程中，防止检测不到过流。够时间就强制复位
        if( working_timing < work_deadline) {

            working_timing++;

        } else {


            //反向
            motorState = MOTOR_BACKWARD;
            g_delay_change = change_delay_time;
            allow_check = 0;  //停止电机流程
            allow_change = 1;
            //反向结束
            
        }


    }  //allow_check

    


    // =================== 过流后，全部拉低，停止工作  ========================
    
    if(motorState == MOTOR_STOP) {

        allow_check = 0;  
        
	
        #if (no_pwm_fun )
            motor_forward_pin = 0;
            motor_backward_pin = 0;
        #else 

            forward_pwm(0);
            backward_pwm(0);

        #endif
    }

	
}

