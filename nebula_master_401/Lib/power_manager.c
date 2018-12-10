#include "power_manager.h"
#include "battery.h"
#include "config.h"
#include "debug_log.h"

extern uint8_t Get_Device_Status(void);
extern void   Set_Device_Status(uint8_t new_sta);
uint32_t power_off_time_count=0;  //关机倒计时
void Reset_Low_Power_Timer(void)
{
	
    power_off_time_count=0;
	
}

void Low_Power_manager(void)
{
    #ifdef ENABLE_LOW_POWER_MANAGER
    uint8_t device_sta;
    device_sta=Get_Device_Status();
    if(POWER_UNCHARGING)
    {
        if(DEVICE_ACTIVE ==device_sta )
        {
            power_off_time_count++;
            if(power_off_time_count >= LOW_POWER_TO_POWER_OFF_TIME)
            {
							
			    Set_Device_Status(DEVICE_TRYING_POWER_OFF);						
                power_off_time_count = 0;
                debug_log("timeout power off");
            }
        }

		else if(DEVICE_OFFLINE == device_sta) 
        {
            power_off_time_count++;
            if(power_off_time_count >= LOW_POWER_TO_POWER_OFF_TIME)
            {
               Set_Device_Status(DEVICE_TRYING_POWER_OFF);
							 power_off_time_count = 0;
               debug_log("timeout power off");
            }
        }
    }
    
    #endif

}
void Sleep_start(void)
{
//  HAL_PWR_DisableSleepOnExit();  
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

    
}


