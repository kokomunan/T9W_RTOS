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
#include "debug_log.h"
#include "usb_controler.h"
#include "usb_host.h"
#include  "page_detect.h"
#include "misc.h"
#include "product_test.h"
#include "cmsis_os.h"

st_device_data_storage g_device_info;
osThreadId usb_main_task_Handle;
osThreadId status_machin_thread_Handle;
extern void write_note_header(uint16_t header_sector, uint8_t *pdata, uint16_t len);
void init(void)
{
	
    ON_SYS_POWER() ;
    TOUCH_PAD_OFF();
    OLED_VBAT_OFF(); 
    OLED_VDD_OFF();

    FLASH_RELEASE();
    OPEN_BLE_POWER();
    BLE_RELEASE() ;	

    OLED_RELEASE();	
    OLED_ENABLE();
    OLED_VDD_ON() ;
    osDelay(10);		
    OLED_DISABLE();  
    osDelay(10);	
    OLED_ENABLE();
    osDelay(10);	
	
	
}

void application_start(void const * argument)
{
    init_log();   
    init();                 //设置io        
    flash_hw_check();       //检测外设是否正常	
    EMR_pad_version();
    mmi_init();
    OLED_On_Init() ;
    init_status_machin();     
    osThreadDef( usb_main_task , usb_main_task, osPriorityNormal, 0, 256);
    usb_main_task_Handle=osThreadCreate(osThread(usb_main_task), NULL);     
    OFF_SYS_POWER() ;   
    half_product_test_check();   
  
  
    if(g_device_info.auto_poweron)
    {
        ON_SYS_POWER() ;
        g_device_info.auto_poweron=0;
        Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage));   
        if(0 == power_on_battery_check())
        {          
             debug_log("vlotage is too low");
             NVIC_SystemReset();       
            
        }
        power_off_battery_Display_deinit();
        server_init(AUTO_POWERON);
               
    }
    
    osThreadDef( status_machin_thread , status_machin_thread, osPriorityNormal, 0, 256);  
    status_machin_thread_Handle =osThreadCreate(osThread(status_machin_thread), NULL);    
    
    osThreadDef( battery_check_thread , battery_check_thread, osPriorityNormal, 0, 256);   
    osThreadCreate(osThread(battery_check_thread), NULL); 
    
    osThreadDef( listen_ble_device_thread , listen_ble_device_thread, osPriorityNormal, 0, 256); 
    osThreadCreate(osThread(listen_ble_device_thread), NULL); 
    
    while(1)
    {

   
          upload_stored_note_Thread(); //上传笔记
          offline_write_flash_Thread();//离线存储    
          server_thread();             //无线服务	                    
          read_position_thread();      //读取坐标                     
          osDelay(1);

              
    }
	
	
	
	
}
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    
    
   debug_log("task id %d,name %s",(uint32_t)xTask,pcTaskName);
    

}

void vApplicationIdleHook( void )
{
    
    
    
    uint32_t size;   
   // size=xPortGetFreeHeapSize();
  //  debug_log("stack free size %d",size);    
     FEED_DOG() ;           
  //  Sleep_start();	
    
}



