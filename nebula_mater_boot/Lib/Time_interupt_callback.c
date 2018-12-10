
#include "Time_interupt_callback.h"
#include "battery.h"
#include "RGB_LED.h"
#include "em9203.h"
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim1;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	
		if(htim==&htim10)  //10ms 中断
		{

				battery_timer_callback();  //电池检测 与电源管理
				//	 led_display_thread_10ms(); //led驱动
				// LED_Indicate_Has_Date();   //指示有写入动作
				//	 nebula_status_check();   //9203 状态更新
		}
		else if(htim==&htim1)
		{


				Hard_Timer_Us_Callback();
				HAL_TIM_Base_Stop_IT(&htim1)	;

		}
	
	
	
	
}


void start_hard_timer(void)
{
    HAL_TIM_Base_Start_IT(&htim10);
	
}
void stop_hard_timer(void)
{
	
	  HAL_TIM_Base_Stop_IT(&htim10);
}


void Set_Hard_timer_us(uint32_t us)
{
	

		//htim11.Init.Period = us;
		//HAL_TIM_Base_Init(&htim11) ;
		//HAL_TIM_Base_Start(&htim1);
		htim1.Instance->ARR=us-1;
		//htim1.Instance->SR=~(TIM_IT_UPDATE);		
		htim1.Instance->CNT=0;	
		HAL_TIM_Base_Start_IT(&htim1);
	
	 //__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
}




