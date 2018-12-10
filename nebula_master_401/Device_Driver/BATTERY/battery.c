#include "battery.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "config.h"
#include "debug_log.h"
#include "RGB_LED.h"
#include "display.h"
#include "cmsis_os.h"
extern ADC_HandleTypeDef hadc1;
extern uint8_t Get_Device_Status(void);
extern void   Set_Device_Status(uint8_t new_sta);
#define ADC_CONTROLER   &hadc1
#define BATEERY_ADC_CHANNAL   ADC_CHANNEL_10  
#define SHUTDOWN_THRESHOLD      0
#define POWE_ON_THRESHOLD       1
#define LOW_BAT_THRESHOLD       2
#define OTA_BAT_THRESHOLD       4



typedef enum
{
       BATTERY_100P,
       BATTERY_80P,
       BATTERY_60P,
       BATTERY_40P,
       BATTERY_20P,
       BATTERY_05P,
       BATTERY_00P
}BatteryTableOffset;

st_charging_led_control g_charging_display;  //控制充电显示的结构
uint16_t battery_detect_counterdown = 0;  //电量测量周期
uint8_t battery_level=0;  
uint8_t battery_percent = 0;
uint8_t battery_last=0;
uint8_t plug_flag=0;
uint8_t charge_full_flag=0;
uint32_t power_check_countdown=0;

TimerHandle_t  bateery_period_timer_handle;

const uint16_t BatteryVoltageTable[7] = 
{
    4250,4050,3860,3740,3680,3600,3500//100,80,60,40,20,5,0
};
//const uint16_t BatteryVoltageTable[7] = 
//{
//    4500,4400,4400,4400,4400,4400,3600//100,70,30,8,5,0
//};
uint8_t get_battery_percent(uint16_t BatteryVoltageInMV)
{

    uint8_t  tmpBatteryPercent;

    if(BatteryVoltageInMV>= BatteryVoltageTable[BATTERY_100P])
    {
        tmpBatteryPercent = 7;
    }
    else if(BatteryVoltageInMV> BatteryVoltageTable[BATTERY_80P])
    {      
        tmpBatteryPercent = 6;
    }
    else if(BatteryVoltageInMV> BatteryVoltageTable[BATTERY_60P])
    {      
        tmpBatteryPercent = 5;
    }
    else if(BatteryVoltageInMV> BatteryVoltageTable[BATTERY_40P])
    {
        tmpBatteryPercent = 4;
    }
    else if(BatteryVoltageInMV> BatteryVoltageTable[BATTERY_20P])
    {
        tmpBatteryPercent = 3;
    }
    else if(BatteryVoltageInMV> BatteryVoltageTable[BATTERY_05P])
    {
        tmpBatteryPercent = 2;
    }
    else if(BatteryVoltageInMV> BatteryVoltageTable[BATTERY_00P])
    {
        tmpBatteryPercent = 1;
    }
    else
    {
        tmpBatteryPercent = 0;
    }
    return tmpBatteryPercent;
}


uint8_t get_battery_value(void)
{
	
	return battery_level;
	
}


uint16_t batt_meas_update(void)
{   

    uint16_t value;

    ADC_ChannelConfTypeDef ADC1_ChanConf;
    ADC1_ChanConf.Channel=BATEERY_ADC_CHANNAL;                                   
    ADC1_ChanConf.Rank=1 ;
    ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_480CYCLES;
    ADC1_ChanConf.Offset=0;                 
    HAL_ADC_ConfigChannel(ADC_CONTROLER,&ADC1_ChanConf);        

    HAL_ADC_Start(ADC_CONTROLER);
    HAL_ADC_PollForConversion(ADC_CONTROLER,10);               
    value=HAL_ADC_GetValue(ADC_CONTROLER);	
    return  ADC_TO_VBAT_MV(value);
}

 uint8_t power_on_battery_check(void)
{
    
  
    battery_level = get_battery_percent(batt_meas_update());	  
    battery_last=battery_level;
    debug_log("power on battery level %d",battery_level);
    update_dis_battery_value(battery_level);//记录真正的电压值

    if(POWER_UNCHARGING)//no USB
    {
        if(batt_meas_power_check())
        {
            debug_log("battery low, can not power on!");           
            notify_low_power();
            osDelay(1000);
            return false ;
        }
    }
    else
    {
        
        if(IS_CHARGE_FULL)
        {
           // battery_level = 0xff; //charging full
            //debug_log("charge full");
           // if(0==charge_full_flag)
           // {
                charge_full_flag=1;
           // }
        }
        else
        {
           // battery_level = 0xfe; //charging
          //  if(1==charge_full_flag)
           // {
                charge_full_flag=0;
           // }
        }
    }
    return true ;
}


void battry_period_timer(const void* argument)
{
     uint16_t value;

    if(battery_detect_counterdown > 0)  //为电池检测提供时钟
    {
        battery_detect_counterdown--;	
    }
    if(power_check_countdown>0)
    {
        power_check_countdown--;
    }
    
    if(battery_detect_counterdown==0)   //定时对电池电量就行检测无论是否有usb 
    {
              
        value=batt_meas_update();     
         battery_percent = get_battery_percent(value); //对电压进行检测
        if(POWER_CHARGING)	//当插入usb时要去掉上浮的充电电压0.06V	
        {

            battery_last=battery_percent;
        }
        else
        {   
          
           if( Get_Device_Status()> DEVICE_STANDBY)
           {
                if(battery_percent>battery_last)
                {
                    battery_percent= battery_last;          

                }
                else
                {
                    battery_last=battery_percent;

                }
               
           }
            
           
        }        
        
        update_dis_battery_value(battery_percent);
        if(battery_percent!=battery_level)
        {
            battery_level = battery_percent;
        }

        battery_detect_counterdown = BATTERY_DET_COUNTDOWN;

    }	
    
    Low_Power_manager();  //控制低功耗管理
    
}





void battery_check_thread(const void *argument)
{
    
 
    osTimerDef(battry_period_timer,battry_period_timer);
    bateery_period_timer_handle=osTimerCreate(osTimer(battry_period_timer),osTimerPeriodic,NULL);
    osTimerStart(bateery_period_timer_handle,10);
    
    while(1)
    {

        if(POWER_UNCHARGING)  //无usb插入
        {
            if(battery_percent!=battery_level)
            {
                battery_level = battery_percent;
            }   
            
            
            if(plug_flag)
            {
                plug_flag=0;
                battery_detect_counterdown =0;
                update_dis_battery_value(battery_percent);
            }
            charge_full_flag=0;
            if(g_charging_display.display_flag)
            {
                
                init_charging_display();
            }

            if(batt_meas_is_shutdown())
            {
                notify_low_power();          
                osDelay(1000);
                debug_log("battery is too low %d",battery_level);
                Set_Device_Status(DEVICE_TRYING_POWER_OFF); //设置设备状态

            }
            
            if(battery_not_enough())
            {
                
                 //如果当前在主界面
                if(get_currunt_screen()==MAIN_SCREEN)
                {
                    if(power_check_countdown==0)
                    {
                    
                        debug_log("battery shoude be charge");
                        change_screen(LOWPOER_SCREEN);
                        power_check_countdown=6000;
                    }
                }
            }


        }
        else
        {
         

            if(IS_CHARGE_FULL)        
            {
                if(0==charge_full_flag)
                {
                    charge_full_flag=1;              
                    debug_log("charge full");
                    init_charging_display();  //刷新led显示控制
                }
                battery_level=0xff;
            }

            else  //电压要跌落到4v以下才是充电
            {   
               
                if(1==charge_full_flag)
                {
                    charge_full_flag=0;
                  
                    debug_log("charging");
                }
                battery_level=0xfe;
                
            }
            if((0 == g_charging_display.display_type) && (false == g_charging_display.display_flag))
            {
                if(DEVICE_POWER_OFF ==Get_Device_Status())
                {
                    g_charging_display.display_flag = true;
                    g_charging_display.display_type = charge_full_flag;
                    g_charging_display.display_cnt = 0;
                    g_charging_display.display_on_off = 1;
                }
                else
                {
                    g_charging_display.display_flag = true;
                    g_charging_display.display_type =charge_full_flag;
                    if(0==plug_flag)
                   {
                        update_dis_battery_value(battery_percent);
                        plug_flag=1;
                   }
          
                }
            }

        }
        
        osDelay(5);
    }
    
}
//重置充电显示控制结构
void init_charging_display(void)
{
    g_charging_display.display_flag = false;
    g_charging_display.display_type = 0;
    g_charging_display.display_cnt = 0;
    g_charging_display.display_on_off = 0;
//    GREEN_LED_OFF();
    debug_log("init charging led");
}

bool batt_meas_is_lowbatt(void)
{
    if(battery_level <= POWE_ON_THRESHOLD)
    {
        return true;
    }
    return false;
}

bool battery_not_enough(void)
{
    if(battery_level <= LOW_BAT_THRESHOLD)
    {
        return true;
    }
    return false;
}

bool Is_battery_is_safe(void)
{
    if(battery_level >= OTA_BAT_THRESHOLD)
    {
        return true;
    }
    return false;
    
    
}
bool batt_meas_is_shutdown(void)
{
    if(battery_level <= SHUTDOWN_THRESHOLD)
    {
        return true;
    }
    return false;
}


bool batt_meas_power_check(void)
{
    if(battery_level <= POWE_ON_THRESHOLD)
    {
        return true;
    }
    return false;
}

uint8_t IS_charge_full(void)
{
    return charge_full_flag;
    
}

