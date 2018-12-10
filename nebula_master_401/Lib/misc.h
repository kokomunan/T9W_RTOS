#ifndef _MISC_H_
#define _MISC_H_
#include "stm32f4xx_hal.h"


#define  OS_ENTER_CRITICAL()  {cpu_sr = OS_CPU_SR_Save();}
#define  OS_EXIT_CRITICAL()   {OS_CPU_SR_Restore(cpu_sr);}

void push_data_to_sum(uint32_t*  last_sum,uint8_t *p,uint16_t  len);
uint32_t  OS_CPU_SR_Save(void);
void       OS_CPU_SR_Restore(uint32_t cpu_sr);
void read_cpu_id(uint32_t *id);

uint32_t calculate_time(void);



#endif


