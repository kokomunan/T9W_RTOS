#include "Soft_Timer.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "stdlib.h"
#include "string.h"
#include <stdarg.h>
#include "battery.h"
#include "debug_log.h"
#define DEBUG_LOG     debug_log
TIMER_LIST  timer_list;
uint32_t  time_div=0;
void start(SOFT_TIMER_ST * timer,uint32_t period)
{
    if(timer!=NULL)
    {
    
        timer->start_time= HAL_GetTick();
        timer-> period=period;
        timer->active=1;
        timer->elapsed=0;
    }
    else
    {
        DEBUG_LOG("timer start failed due to null point");
    }
	
}


uint8_t IS_Elapsed(SOFT_TIMER_ST * timer)
{
    if(timer!=NULL)
    {
        return timer->elapsed ;
    }
    else
    {
        return 0;
    }
	
}


void Soft_Timer_manager(void)
{
    uint8_t i;
    uint32_t curunt_time;
    curunt_time=HAL_GetTick();
    for(i=0;i< MAX_TIMER;i++)
    {
        if(timer_list.timers[i].position!=0xff)
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
	
	
	
	
}

void soft_timer_init(void)
{
    uint8_t i;
	memset(&timer_list,0,sizeof(TIMER_LIST));
    for(i=0;i<MAX_TIMER;i++)
    {
       timer_list.timers[i].position=0xff; 
    }
}

//以后完善 注销 和停止 重复等功能
SOFT_TIMER_ST * register_soft_timer(void)
{
		//__disable_irq(); 
    	uint8_t i;
		if(timer_list.timer_total> MAX_TIMER )
		{
            //__enable_irq(); //打开总中断
            return 0;
		}
		else
		{

          
            // __enable_irq(); //打开总中断
            for(i=0;i<MAX_TIMER;i++)
            {
                if(timer_list.timers[i].position==0xff)
                {
                    timer_list.timers[i].position=i;
                    timer_list.timer_total++;
                    return &timer_list.timers[i];
                }
            }
            return 0;
				
		}
	
}

void disregister_soft_timer(SOFT_TIMER_ST * timer)
{
    
    
  //  timer_list.position[]
    if(timer!=NULL)
    {
        memset(timer,0,sizeof(SOFT_TIMER_ST)); //清除
        timer->position=0xff;
        if(timer_list.timer_total>0)
        {
            timer_list.timer_total--;
        }     
    }
    

    
}

