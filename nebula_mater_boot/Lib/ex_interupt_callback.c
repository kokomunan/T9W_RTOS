#include "ex_interupt_callback.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "nrf51822.h"
#include "em9203.h"
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
	
		if(GPIO_Pin==BT_INT_Pin)  //蓝牙有消息
		{

		    Ble_Response_callback();

		}
		else if(GPIO_Pin==EM_9203_IRQ_Pin) //em9203 有消息
		{

		    Em9203_Int_Callback();

		}
		else if(GPIO_Pin==IIC_ANT_INT_Pin)
		{
		    EMR_pad_isr_callback();

		}
	
	
	
}


