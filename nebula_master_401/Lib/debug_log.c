
#include "debug_log.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "config.h"


#define LOG_LEN 100
#define LOG_QUEUE_NUM  10
osMutexId  log_mutex_handle;
osMutexDef(log_mutex);

QueueHandle_t Log_Queue;

#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
    int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
    x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
    while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (uint8_t) ch;      
    return ch;
}


void log_thread(const void *argument)
{
  
  char pcMessageToPrint[100];
  while(1)
  {
    
    xQueueReceive( Log_Queue, pcMessageToPrint, portMAX_DELAY );
    
    printf("%s",pcMessageToPrint);
    
    
  }
 
  
}

void init_log(void )
{
    
    log_mutex_handle=osMutexCreate (osMutex(log_mutex));
   
    Log_Queue=xQueueCreate( LOG_QUEUE_NUM, LOG_LEN ); //创建队列
  
    osThreadDef( log_thread , log_thread, osPriorityAboveNormal, 0, 256);   
    osThreadCreate(osThread(log_thread), NULL); 
  
  

}





