
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
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
    int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
    x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
    while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
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
   
    Log_Queue=xQueueCreate( LOG_QUEUE_NUM, LOG_LEN ); //��������
  
    osThreadDef( log_thread , log_thread, osPriorityAboveNormal, 0, 256);   
    osThreadCreate(osThread(log_thread), NULL); 
  
  

}





