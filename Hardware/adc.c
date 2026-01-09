#include "adc.h"


void ZD_ADC_Init(void)
{
    // 配置P04为模拟输入模式
    P0_MD1   |= GPIO_P05_MODE_SEL(0x3);                       // P21设为模拟模式

    //ADC配置
    ADC_ACON1 &= ~(ADC_VREF_SEL(0x7) | ADC_EXREF_SEL(0x1));   // 关闭外部参考电压
    ADC_ACON1 |= ADC_VREF_SEL(0x5) |                          // 选择内部参考电压4.2v
                 ADC_TEN_SEL(0x3);


    ADC_ACON0  = ADC_CMP_EN(0x1)  |                           // 打开ADC中的CMP使能信号
                 ADC_BIAS_EN(0x1) |                           // 打开ADC偏置电流能使信号
                 ADC_BIAS_SEL(0x1);
    ADC_CHS0   = ADC_ANALOG_CHAN(0x05) |                      // P04通路     选择对应的引脚配置
                 ADC_EXT_SEL(0x0);                            // 选择外部通路



    ADC_CFG0  |= ADC_CHAN0_EN(0x1) |                          // 使能通道0转换
                 ADC_EN(0x1);                                 // 使能A/D转换
    delay_ms(1);       



}


u16 current_arr[16] = {0};
u8 current_index = 0;
u32 current_sum = 0;
u16 current_average = 0;
void CheckCurrentFunc(void)
{

  static u16 adc0_val = 0;
  u8 i = 0;
  ADC_CFG0 |= ADC_CHAN0_TRG(0x1);                       // 触发ADC0转换
  if(ADC_STA & ADC_CHAN0_DONE(0x1)){

    ADC_STA  = ADC_CHAN0_DONE(0x1);                       // 清除ADC0转换完成标志位
    adc0_val = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4);     // 读取ADC0的值

  }
    current_arr[current_index] = adc0_val;

#if 0

/*  这个滤波，波形不行  */
    if(++current_index >= 16){
        current_index = 15;
        for(i = 0; i < 15; i++){

            current_arr[i] = current_arr[i+1];


        }


    }

#else
    if(++current_index >= 16)current_index = 0;
 #endif   
    current_sum = 0;
    for( i = 0; i < 16; i++)
    {
        current_sum += current_arr[i];
    }
    current_average = current_sum >> 4;


    printf("a=%d,",adc0_val);
    printf("v=%d,",current_average);
 

}



