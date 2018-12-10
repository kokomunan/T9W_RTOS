#include "battery.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "config.h"
#include "debug_log.h"
#include "RGB_LED.h"
#include "display.h"
extern ADC_HandleTypeDef hadc1;
extern uint8_t Get_Device_Status(void);
extern void   Set_Device_Status(uint8_t new_sta);
#define ADC_CONTROLER   &hadc1
#define BATEERY_ADC_CHANNAL   ADC_CHANNEL_10  
#define SHUTDOWN_THRESHOLD      0
#define POWE_ON_THRESHOLD       1
#define LOW_BAT_THRESHOLD       2
#define  DEBUG_LOG    debug_log


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

st_charging_led_control g_charging_display;  //���Ƴ����ʾ�Ľṹ
uint16_t battery_detect_counterdown = 0;  //������������
uint8_t battery_level=0;        



const uint16_t BatteryVoltageTable[7] = 
{
    4000,3900,3850,3750,3700,3650,3600//100,70,30,8,5,0
};
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
//��ؼ�ⶨʱ���ص�
void battery_timer_callback(void)
{
	if(battery_detect_counterdown > 0)  //Ϊ��ؼ���ṩʱ��
    {
        battery_detect_counterdown--;	
    }
	charging_display();  //���Ƴ����ʾ
	Low_Power_manager();  //���Ƶ͹��Ĺ���
}

 uint8_t power_on_battery_check(void)
{
    if(POWER_CHARGING)   
    {        
        battery_level = get_battery_percent(batt_meas_update()-100);
    }
    else
    {
        battery_level = get_battery_percent(batt_meas_update());	
    }


    update_dis_battery_value(battery_level);//��¼�����ĵ�ѹֵ

    if(POWER_UNCHARGING)//no USB
    {
        if(batt_meas_power_check())
        {
            DEBUG_LOG("battery low, can not power on!");
            return false ;
        }
    }
    else
    {
        if(battery_level==7)  //���������ж�
        {
            battery_level = 0xff; //charging full
            //DEBUG_LOG("charge full");
        }
        else
        {
            battery_level = 0xfe; //charging
        }
    }
    return true ;
}


void battery_check_thread(void)
{
    uint16_t battery_percent = 0;
    static uint8_t battery_level=0;

    if(battery_detect_counterdown==0)   //��ʱ�Ե�ص������м�������Ƿ���usb 
    {

        if(POWER_CHARGING)	//������usbʱҪȥ���ϸ��ĳ���ѹ0.06V	
        {
            battery_percent = get_battery_percent(batt_meas_update()-100); //�Ե�ѹ���м��
        }
        else
        {
            battery_percent = get_battery_percent(batt_meas_update()); 		
        }        

        update_dis_battery_value(battery_percent);
        //DEBUG_LOG("Bat level:[%d]", battery_percent);
        if(battery_percent!=battery_level)
        {
            battery_level = battery_percent;
        }

        battery_detect_counterdown = BATTERY_DET_COUNTDOWN;

    }	


    if(POWER_UNCHARGING)  //��usb����
    {   
        if(g_charging_display.display_flag)
        {
            init_charging_display();
        }

        if(batt_meas_is_shutdown())
        {

            Set_Device_Status(DEVICE_TRYING_POWER_OFF); //�����豸״̬

        }


    }
    else
    {

        //device_sta=Get_Device_Status();

        if(battery_level==7)   //ֻ�е�ѹ�ﵽ4.1���ϲ��ǳ�����		
        {
            if(battery_level!= 0xff)
            {
                battery_level = 0xff; //charging full
                DEBUG_LOG("charging full\n");
                init_charging_display();  //ˢ��led��ʾ����
            }
        }

        else  //��ѹҪ���䵽4v���²��ǳ��
        {
            battery_level = 0xfe; //charging
        }
        if((0 == g_charging_display.display_type) && (false == g_charging_display.display_flag))
        {
            if(DEVICE_POWER_OFF ==Get_Device_Status())
            {
                g_charging_display.display_flag = true;
                g_charging_display.display_type = battery_level;
                g_charging_display.display_cnt = 0;
                GREEN_LED_ON();
                g_charging_display.display_on_off = 1;
            }
            else
            {
                g_charging_display.display_flag = true;
                g_charging_display.display_type =battery_level;
                GREEN_LED_ON();
                HAL_Delay(200);
                GREEN_LED_OFF();
            }
        }

    }
}
//���ó����ʾ���ƽṹ
void init_charging_display(void)
{
g_charging_display.display_flag = false;
g_charging_display.display_type = 0;
g_charging_display.display_cnt = 0;
g_charging_display.display_on_off = 0;
GREEN_LED_OFF();
DEBUG_LOG("init charging led\r\n");
}
//�����ʾ
void charging_display(void)
{

    if((g_charging_display.display_flag) && (DEVICE_POWER_OFF ==Get_Device_Status()))
    {
        if(0xfe == g_charging_display.display_type)
        {
            g_charging_display.display_cnt++;
            if(g_charging_display.display_cnt >= 100)
            {
                g_charging_display.display_cnt = 0;
                if(g_charging_display.display_on_off)
                {
                    GREEN_LED_OFF();
                    g_charging_display.display_on_off = 0;
                }
                else
                {
                    GREEN_LED_ON();
                    g_charging_display.display_on_off = 1;
                }
            }
        }
    }
}




bool batt_meas_is_lowbatt(void)
{
    if(battery_level <= LOW_BAT_THRESHOLD)
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
