#include "timer.h"
#include "task.h"
#include "adc.h"
#include "uart_data_handle.h"

// timer0 用作系统定时器
void timer0_init(void)
{
	__EnableIRQ(TMR0_IRQn); // 使能timer0中断
	IE_EA = 1;				// 使能总中断

	// 设置timer0的计数功能，配置一个频率为的中断
	TMR_ALLCON = TMR0_CNT_CLR(0x1);								  // 清除计数值
	TMR0_PRH = TMR_PERIOD_VAL_H((TIMER0_PEROID_VAL >> 8) & 0xFF); // 周期值
	TMR0_PRL = TMR_PERIOD_VAL_L((TIMER0_PEROID_VAL >> 0) & 0xFF);
	TMR0_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);							 // 计数等于周期时允许发生中断
	TMR0_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x0) | TMR_MODE_SEL(0x1); // 选择系统时钟，128分频，计数模式
}

/*  ---------------------  定时中断服务函数 ---------------------- */

void TIMR0_IRQHandler(void) interrupt TMR0_IRQn
{
	// 进入中断设置IP，不可删除
	__IRQnIPnPush(TMR0_IRQn);

	// ---------------- 用户函数处理 -------------------

	// 周期中断
	if (TMR0_CONH & TMR_PRD_PND(0x1))
	{
		TMR0_CONH |= TMR_PRD_PND(0x1); // 清除pending

		task_schedule_tick();

		adc_scan(); // 至少要间隔1ms，才调用一次

		uart_data_recv_timeout_add();
	}

	// 退出中断设置IP，不可删除
	__IRQnIPnPop(TMR0_IRQn);
}