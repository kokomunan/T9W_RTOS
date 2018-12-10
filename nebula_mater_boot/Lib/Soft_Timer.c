#include "Soft_Timer.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "stdlib.h"
#include "string.h"
#include <stdarg.h>
TIMER_LIST  timer_list;

void start(SOFT_TIMER_ST * timer,uint32_t period)
{
		timer->start_time= HAL_GetTick();
		timer-> period=period;
		timer->active=1;
		timer->elapsed=0;
	
}


uint8_t IS_Elapsed(SOFT_TIMER_ST * timer)
{
	  return timer->elapsed ;
	
}


void Soft_Timer_manager(void)
{
		uint8_t i;
		uint32_t curunt_time;
		curunt_time=HAL_GetTick();
		for(i=0;i<timer_list.timer_total;i++)
		{
				if(timer_list.timers[i].active)
				{
						if(abs(curunt_time-timer_list.timers[i].start_time)>timer_list.timers[i].period)
						{
								timer_list.timers[i].elapsed=1;
								timer_list.timers[i].active=0;
						}
				}
		}
	
	
	
	
}

void soft_timer_init(void)
{
	  memset(&timer_list,0,sizeof(TIMER_LIST));
}

//以后完善 注销 和停止 重复等功能
SOFT_TIMER_ST * register_soft_timer(void)
{
		__disable_irq(); 
		if(timer_list.timer_total> MAX_TIMER )
		{
				__enable_irq(); //打开总中断
				return 0;
		}
		else
		{

				timer_list.timer_total++;
				 __enable_irq(); //打开总中断
				return &timer_list.timers[timer_list.timer_total-1];
		}
	
}


void HAL_SYSTICK_Callback(void)
{
    Soft_Timer_manager();
	
	
}


