#include "misc.h"



#define CPUID_ADDR_BASE   0x1fff7a10
uint32_t start_time;

void push_data_to_sum(uint32_t*  last_sum,uint8_t *p,uint16_t  len)
{
	
    uint16_t  i;
    for(i=0;i<len;i++)
    *last_sum+=*(p+i);
	
	
}

void read_cpu_id(uint32_t *id)
{
    
    id[0]=*(uint32_t *)CPUID_ADDR_BASE;
    id[1]=*(uint32_t *)(CPUID_ADDR_BASE+4);
    id[2]=*(uint32_t *)(CPUID_ADDR_BASE+8);

    
    
    
}
void calculate_time_init(void)
{
  start_time= HAL_GetTick();
   
}



uint32_t calculate_time(void)
{
  
    uint32_t time_ms;
    uint32_t currunt_time;
    currunt_time=HAL_GetTick();
    if(currunt_time>= start_time)        
    {
        time_ms=currunt_time-start_time;
    }
    else
    {
        time_ms=(0xffffffff-start_time)+currunt_time;
    }
    start_time=currunt_time;
    
    return time_ms;


    
}