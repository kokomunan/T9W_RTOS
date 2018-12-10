
#include "Time_interupt_callback.h"
#include "battery.h"
#include "RGB_LED.h"
#include "em9203.h"
//extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
//extern TIM_HandleTypeDef htim10;
//extern TIM_HandleTypeDef htim1;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
//	
//    if(htim==&htim10)  //10ms 中断
//    {

//        battery_timer_callback();  //电池检测 与电源管理
// //       led_display_thread_10ms(); //led驱动
////        LED_Indicate_Has_Date();   //指示有写入动作

//    }
//    else 
//    if(htim==&htim1)
//    {

//      //  HAL_GPIO_WritePin(JEDI_SPI_CS1_GPIO_Port,JEDI_SPI_CS1_Pin, GPIO_PIN_SET);
//        Hard_Timer_Us_Callback();
//        HAL_TIM_Base_Stop_IT(&htim1)	;

//    }
//	
	
	
	
}

void start_hard_timer(void)
{
  //  HAL_TIM_Base_Start_IT(&htim10);
	
}
void stop_hard_timer(void)
{
	
	//HAL_TIM_Base_Stop_IT(&htim10);
}


void Set_Hard_timer_us(uint16_t us)
{
//    uint16_t target_value;
//    float calibra=0.0;
//    calibra=us*0.008;
//    target_value=us+(uint16_t)calibra;
//        
//    //htim11.Init.Period = us;
//    //HAL_TIM_Base_Init(&htim11) ;
//    //HAL_TIM_Base_Start(&htim1);
//    htim1.Instance->ARR= target_value-1;
//    //htim1.Instance->SR=~(TIM_IT_UPDATE);		
//    htim1.Instance->CNT=0;	
//    HAL_TIM_Base_Start_IT(&htim1);

//    //__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
}

void test_wide(void)
{
    
//    HAL_GPIO_WritePin(JEDI_SPI_CS1_GPIO_Port,JEDI_SPI_CS1_Pin, GPIO_PIN_SET);
    printf("start loop\r\n");
    
    {
        uint16_t i=0;
        for(i=0;i<60;i++)
        {
            
        
            //HAL_GPIO_WritePin(JEDI_SPI_CS1_GPIO_Port,JEDI_SPI_CS1_Pin, GPIO_PIN_RESET);
            Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (i + 1));     
            HAL_Delay(100);	  
        }
        
        
    }
    
    
}



