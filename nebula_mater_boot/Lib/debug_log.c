#include "debug_log.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "config.h"


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

