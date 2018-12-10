#ifndef _SOFT_TIMER_H
#define _SOFT_TIMER_H

#include "stdint.h"

#define  MAX_TIMER     12

typedef struct  
{

	uint32_t start_time;
	uint32_t elapsed;
	uint32_t period;
	uint8_t active;
    uint8_t position;

	
} SOFT_TIMER_ST;

typedef struct  
{
	
	SOFT_TIMER_ST timers[MAX_TIMER];
	uint8_t timer_total;
	
	
}TIMER_LIST;

SOFT_TIMER_ST * register_soft_timer(void);
void disregister_soft_timer(SOFT_TIMER_ST * timer);
void start(SOFT_TIMER_ST * timer,uint32_t period);
uint8_t IS_Elapsed(SOFT_TIMER_ST * timer);
void soft_timer_init(void);



#endif
