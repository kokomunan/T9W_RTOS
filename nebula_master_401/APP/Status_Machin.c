#include "Status_Machin.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "config.h"
#include "RGB_LED.h"
#include "General_Emr.h"
#include "battery.h"
#include "debug_log.h"
#include "display.h"
#include "input_event.h"
#include "norflash.h"
#include "note_manager.h"
#include "power_manager.h"
#include "server.h"
#include "device_info.h"
#include "usb_controler.h"
#include "page_detect.h"
#include "cmsis_os.h"


stMainStatus    g_main_status,g_main_status_back;
uint8_t battery_back=0;
extern st_device_data_storage g_device_info;


uint8_t Get_Device_Status(void)
{
    return g_main_status.device_state;
	
}
void Set_Device_Status(uint8_t new_sta)
{
	 
    g_main_status.device_state= new_sta;
	 
}

void init_status_machin(void)
{
    uint8_t rst;
    uint8_t add[6];
    uint16_t h_version;
    g_main_status.device_state = DEVICE_POWER_OFF;
    g_main_status.btn_hold_flag = 0;

    Load_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage)) ;  //加载关键的全局机器信息
    memcpy(add,g_device_info.ble_addr,6);
    h_version=g_device_info.hard_version;
    debug_log("hard version[%x]",h_version);
    debug_log("get device info[%x][%x][%d]", g_device_info.identifier, g_device_info.dtm_mode_flag, g_device_info.auto_poweron);
    //  hw_v = g_device_info.hardware_version;
    if(g_device_info.identifier != 0x1985)
    {

        memset((uint8_t *)&g_device_info, 0, sizeof(st_device_data_storage) - 8);
        g_device_info.identifier = 0x1985;
        g_device_info.dtm_mode_flag = 0;
        g_device_info.mcu_firmware_version = SW_VERSION;
        g_device_info.stored_total_note_num = 0;
        g_device_info.note_read_start_sector = 0;
        g_device_info.note_read_end_sector = 0;
        //g_device_info.hardware_version = hw_v;
        g_device_info.error_code = 0;
        g_device_info.note_index=0;
        g_device_info.node_info.class_number=8;
        g_device_info.node_info.customer_number=0xcc;
        g_device_info.node_info.device_number=8;
        g_device_info.serve_info.last_serve=NRF51822;
        g_device_info.serve_info.second_serve=WIFI_MODE;
//        set_first_boot_flag(1); //标记第一次启动
        memcpy(g_device_info.ble_addr,add,6);
        g_device_info.hard_version=h_version;
        rst = Update_device_info((uint8_t *)&g_device_info, sizeof(st_device_data_storage));
        debug_log("init storage [%d]", rst);
    }


//    init_display_currunt_note_index( g_device_info.stored_total_note_num);  //初始化显示的索引
    set_currunt_note_id(g_device_info.note_index); //提取笔记的最新的id

    debug_log("stored notes[%d]", g_device_info.stored_total_note_num);
    debug_log("sw version[0x%x]",  SW_VERSION);

    power_off_battery_Display_init();
	
}

static void state_machine_trying_poweroff(void)
{
   

    Dispalay_deinit();   // 关闭显示和oled
    server_stop();//关闭无线设备 
    flash_wake_up();
    close_note(1, 0);
	FEED_DOG();   
    osDelay(300);
    g_main_status.device_state = DEVICE_POWER_OFF; 
    TOUCH_PAD_OFF();	
    debug_log("trying to power off");
	OFF_SYS_POWER() ;
    if(POWER_BUTTON_PRESS)
    {
        g_main_status.btn_hold_flag = 1;
    }
    else
    {
        NVIC_SystemReset();
    }
}

static void state_machine_power_off(void)
{    

    if(POWER_BUTTON_PRESS)//button detect enabled, and the button is pressed
    {
        if(0 == g_main_status.btn_hold_flag)
        {
           
            if(POWER_UNCHARGING)
            {
                init_charging_display();
            }
            g_main_status.device_state = DEVICE_STANDBY;
            debug_log("enter standby!");
        }
    }
    else
    {
        if(g_main_status.btn_hold_flag)
        {
            g_main_status.btn_hold_flag = 0;
            //g_main_status.device_state = DEVICE_POWER_OFF;
            NVIC_SystemReset();
        }
        
      
    }
      

}


static uint8_t button_initialization_process(void)
{
   
    uint8_t server_mode=0;

    if(0 == power_on_battery_check())
    {
     
         debug_log("vlotage is too low");
         return 0;
    }
       
    
    server_mode=SELCET_MODE;
     
    debug_log("server init at mode%d",server_mode);
    if(server_init(server_mode))
    return 0;
    ON_SYS_POWER() ;  
    power_off_battery_Display_deinit();    
    debug_log("start menu");       
    Display_init()	;//创建显示线程
    input_init();  //创建输入线程        
    g_main_status.device_state = DEVICE_OFFLINE; 
    debug_log("enter offline!");	
    


    return 1;
}


static void state_machine_standby(void)
{
    if(POWER_BUTTON_PRESS)//button detect enabled, and the button is pressed
    {
              
        //turn on the devices successfully   
        g_main_status.device_state = DEVICE_INIT_BTN;       
        debug_log("enter btn init!");

    }
    
  
    
   
    
    
}

static void state_machine_button_initialization(void)
{
   
    if(false == button_initialization_process())
    {
        g_main_status.device_state =  DEVICE_POWER_OFF;
      
    }
}

static void state_machine_ota(void)
{
   
}
static void state_machine_offline(void)
{
 

}
static void state_machine_active(void)
{
    
	
	
	
}

void report_status_change()
{
    uint8_t temp[3];
    temp[0] = g_main_status.device_state;
    temp[1] =  get_battery_value();
    temp[2] = (uint8_t)g_device_info.stored_total_note_num;
    server_notify_host(CMD_STATUS, temp, 3);
 
	
}

void Status_poll()
{
    uint8_t bttery_temp;

    bttery_temp=get_battery_value();


    if(g_main_status.device_state != g_main_status_back.device_state)
    {
        if((DEVICE_ACTIVE == g_main_status.device_state) && (DEVICE_OFFLINE  == g_main_status_back.device_state))//由离线变成连接时
        {
             close_note(1, 0);
            if(get_server_mode()==NRF51822)
            {
               
                flash_enter_power_down();
            }
           page_det_moudule_reset();
        }
                      
        if((DEVICE_OFFLINE == g_main_status.device_state) && (DEVICE_OFFLINE != g_main_status_back.device_state))//由连接状态变成离线
        {
    
              page_det_moudule_reset() ;     
                       
        }
        if((DEVICE_ACTIVE == g_main_status.device_state)
           ||(DEVICE_SYNC_MODE == g_main_status.device_state)
           ||(DEVICE_OTA_MODE==g_main_status.device_state)
           ||(DEVICE_DFU_MODE==g_main_status.device_state)
           )
        {
           
            report_status_change();  
        }
  
          
        Reset_Low_Power_Timer();							 
        update_dis_all();//刷新整体界面	
        
        memcpy((uint8_t *)&g_main_status_back, (uint8_t *)&g_main_status, sizeof(stMainStatus));
      			
        debug_log("status:[%d]", g_main_status.device_state);
    }
    
    
    else if(battery_back !=	bttery_temp )
    {
       if((g_main_status.device_state!=DEVICE_OTA_MODE)&&(g_main_status.device_state!=DEVICE_DFU_MODE)&&(g_main_status.device_state!=SENSOR_UPDATE)) 
       {
            report_status_change();  
           battery_back=	bttery_temp;	
           
       }           
    }
}


void status_machin_thread(void const * argument)
{
    while(1)
    {
	
        if(DEVICE_POWER_OFF == g_main_status.device_state)
        {
            state_machine_power_off();
        }
        else if(DEVICE_STANDBY == g_main_status.device_state)
        {
            state_machine_standby();
        }
        else if(DEVICE_INIT_BTN == g_main_status.device_state)
        {
            state_machine_button_initialization();
        } 
        else if(DEVICE_OFFLINE == g_main_status.device_state)
        {
            state_machine_offline();
        }
        else if(DEVICE_ACTIVE == g_main_status.device_state)
        {
            state_machine_active();
        }
        else if(DEVICE_TRYING_POWER_OFF == g_main_status.device_state)
        {
            state_machine_trying_poweroff();
        }
        else if(DEVICE_OTA_MODE == g_main_status.device_state)
        {
            state_machine_ota();
        }

        Status_poll();  //跟踪状态
        osDelay(1);
   }
	
}

