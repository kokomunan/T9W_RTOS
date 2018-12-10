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
#define DEBUG_LOG  debug_log

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
		uint32_t hw_v = 4;
		g_main_status.device_state = DEVICE_POWER_OFF;
		g_main_status.btn_hold_flag = 0;

		Load_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage)) ;  //加载关键的全局机器信息

		DEBUG_LOG("get device info[%x][%x][%d]", g_device_info.identifier, g_device_info.dtm_mode_flag, g_device_info.auto_poweron);
		//  hw_v = g_device_info.hardware_version;
		if(g_device_info.identifier != 0x1985)
		{

				memset((uint8_t *)&g_device_info, 0, sizeof(st_device_data_storage) - 8);
				g_device_info.identifier = 0x1985;
				g_device_info.dtm_mode_flag = 0;
				g_device_info.firmware_version = SW_VERSION;
				g_device_info.stored_total_note_num = 0;
				g_device_info.note_read_start_sector = 0;
				g_device_info.note_read_end_sector = 0;
				g_device_info.hardware_version = hw_v;
				g_device_info.error_code = 0;
				g_device_info.note_index=0;
				set_first_boot_flag(1); //标记第一次启动
				rst = Update_device_info((uint8_t *)&g_device_info, sizeof(st_device_data_storage));
				DEBUG_LOG("init storage [%d]", rst);
		}

		init_display_currunt_note_index( g_device_info.stored_total_note_num);  //初始化显示的索引
		set_currunt_note_id(g_device_info.note_index); //提取笔记的最新的id

		DEBUG_LOG("stored notes[%d]", g_device_info.stored_total_note_num);
		DEBUG_LOG("hw version[0x%x]sw version[0x%x][0x%x]", g_device_info.hardware_version, g_device_info.firmware_version, SW_VERSION);
	
	
}

static void state_machine_trying_poweroff(void)
{
   

    led_stop();
    Dispalay_deinit();   // 关闭显示和oled

    DEBUG_LOG("close oled");


    server_stop();//关闭无线设备
  
    flash_wake_up();
	  BLUE_LED_ON(); 
    HAL_Delay(300);
    RED_LED_OFF();
    BLUE_LED_OFF();
    close_note(1, 0);
	  FEED_DOG();   
    HAL_Delay(300);
    g_main_status.device_state = DEVICE_POWER_OFF; 
    TOUCH_PAD_OFF();	
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
            led_stop();
            if(POWER_UNCHARGING)
            {
                init_charging_display();
            }
            g_main_status.device_state = DEVICE_STANDBY;
            DEBUG_LOG("enter standby!\r\n");
            return;
        }
    }
    else
    {
        if(g_main_status.btn_hold_flag)
        {
            g_main_status.btn_hold_flag = 0;
            NVIC_SystemReset();
        }
    }
}

static void state_machine_standby(void)
{
    if(POWER_BUTTON_PRESS)//button detect enabled, and the button is pressed
    {
        //turn on the devices successfully
       	ON_SYS_POWER() ;
     
        g_main_status.device_state = DEVICE_INIT_BTN;
     
        DEBUG_LOG("enter btn init!\n");
        return;//this return is must for switch the state machine
    }
}


static uint8_t button_initialization_process(void)
{
   
		uint8_t slider_value;

		if(0 == power_on_battery_check())
		{
				//battery is low
				low_BAT_power_on_led();
				return 0;
		}

		slider_value=HAL_GPIO_ReadPin(MODE_SELECT_GPIO_Port,MODE_SELECT_Pin);	// 检查要进入的模式 slider
		DEBUG_LOG("server init at mode%d",slider_value);
		//server_init(slider_value);
		server_init(1);

		DEBUG_LOG("start menu");
		Display_init()	;//开启display
		input_init();  //开启输入设备

		//	if(get_server_mode()==NRF51822)
		//	{
		g_main_status.device_state = DEVICE_OFFLINE; 
		DEBUG_LOG("enter offline!");

		//	}
		TOUCH_PAD_ON();


		return 1;
}

static void state_machine_button_initialization(void)
{
    if(false == button_initialization_process())
    {
        g_main_status.device_state = DEVICE_STANDBY;
        NVIC_SystemReset();
    }
}


static void state_machine_ota(void)
{
   
}
static void state_machine_offline(void)
{
    if(g_main_status.btn_hold_flag)
    {
        if(POWER_BUTTON_PRESS)
        {
            g_main_status.btn_hold_flag = 0;
        }
    }

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

		server_notify_host(BLE_CMD_STATUS, temp, 3);
	
}

void Status_poll()
{
		uint8_t bttery_temp;

		bttery_temp=get_battery_value();


		if((g_main_status.device_state != g_main_status_back.device_state) 
			 || (battery_back !=	bttery_temp ))
		{
				if((DEVICE_ACTIVE == g_main_status.device_state) && (DEVICE_ACTIVE != g_main_status_back.device_state))
				{
				    close_note(1, 0);
						flash_enter_power_down();
				}
				if((DEVICE_OFFLINE == g_main_status.device_state) && (DEVICE_OFFLINE != g_main_status_back.device_state))
				{
				
				
					   update_dis_online_page(g_device_info.stored_total_note_num,g_device_info.stored_total_note_num);
						
					   set_currunt_note_id(g_device_info.note_index);
					
				
				}

				
								
				update_dis_all();;//刷新整体界面
				
				memcpy((uint8_t *)&g_main_status_back, (uint8_t *)&g_main_status, sizeof(stMainStatus));
				battery_back=	bttery_temp;
				report_status_change();
				DEBUG_LOG("status:[%d]", g_main_status.device_state);
		}
}


void status_machin_thread(void)
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
		else if( DEVICE_FINISHED_PRODUCT_TEST == g_main_status.device_state)
		{

				if(POWER_UNCHARGING)//no USB	// 如果没有连接了 
				{   

						TOUCH_PAD_OFF(); // 将i2c模组掉电
						HAL_Delay(100);
						Dispalay_deinit();
						OFF_SYS_POWER(); //主电源掉电

				}
		}


		Status_poll();  //跟踪状态
	
}

