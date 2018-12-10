#include "Application_Main_Thread.h"
#include "display.h"
#include "note_manager.h"
#include "power_manager.h"
#include "RGB_LED.h"
#include "General_Emr.h"
#include "oled.h"
#include "norflash.h"
#include "mmi.h"
#include "EXT_RTC.h"
#include "Soft_Timer.h"
#include "Time_interupt_callback.h"
#include "input_event.h"
#include "Status_Machin.h"
#include "device_info.h"
#include "server.h"
#include "nrf51822.h"
#include "em9203.h"
#include "battery.h"
#include "gather_position.h"
#include "dfu.h"
#define BOOT_VERSION   3
void init(void)
{
	
    ON_SYS_POWER() ;
    RED_LED_OFF();
    GREEN_LED_OFF();
    BLUE_LED_OFF();
    TOUCH_PAD_OFF();
    OLED_VBAT_OFF(); 
    OLED_VDD_OFF();

    //拉高所有片选
    FLASH_RELEASE();
  //  BLE_RELEASE();//释放蓝牙片选
    //释放em9203
    OLED_RELEASE();	

}

void application_start(void)
{
	
    init();                 //设置io                    
    printf("boot version %d\r\n",BOOT_VERSION);
    if(check_dfu()==0)
    {  
      
        start_update_fw();  
        
    }
  
    jump_to_app();
    while(1)
    {

       HAL_Delay(1000);

      	

    }
	
	
	
	
	
}

