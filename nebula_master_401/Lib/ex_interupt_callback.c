#include "ex_interupt_callback.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "nrf51822.h"
#include "em9203.h"
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
	
    if(GPIO_Pin==BT_INT_Pin)  //¿∂—¿”–œ˚œ¢
    {

        Ble_Response_callback();

    }

    else if(GPIO_Pin==IIC_ANT_INT_Pin)
    {
        EMR_pad_isr_callback();

    }
	
	
	
}


