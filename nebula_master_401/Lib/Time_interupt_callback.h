#ifndef TIME_INTERUPT_CALLBACK_H
#define TIME_INTERUPT_CALLBACK_H
#include "mxconstants.h"
#include "stm32f4xx_hal.h"


void start_hard_timer(void);
void stop_hard_timer(void);
void Set_Hard_timer_us(uint16_t us);
void test_wide(void);
#endif


