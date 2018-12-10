#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "RGB_LED.h"
#include "server.h"
extern uint8_t Get_Device_Status(void);


#define  true 1
#define false 0

static  unsigned long s_u32RedPreTime = 0;
static  unsigned long s_u32GreenPreTime = 0;
static  unsigned long s_u32BluePreTime = 0;

static  unsigned long s_u32CurTime = 0;
static  LEDAction* s_LedAction = NULL;
static  uint8_t u8RedCount = 0;
static  uint8_t u8GreenCount = 0;
static  uint8_t u8BlueCount = 0;

static  uint8_t u8RedRepeatCount = 0;
static  uint8_t u8GreenRepeatCount = 0;
static  uint8_t u8BlueRepeatCount = 0;

static  uint8_t u8RedStatus= 0;// 0: low 1:high
static  uint8_t u8GreenStatus = 0;
static  uint8_t u8BlueStatus = 0;

static  uint8_t u8Start = 0;
static  uint8_t u8EndFlag = 0;
extern uint8_t is_charging;
uint32_t LEDTick = 0; //LSB is 50ms
uint8_t current_ui_display = 0;
uint8_t Div_clk=0;
uint32_t write_signal_last_time=0;
uint32_t write_signal_count =0;
uint32_t write_interval=0; 
uint8_t hold_led_ui=0;
extern uint8_t Get_Device_Status(void);

int led_stop(void)
{
	u8Start = 0;
	RED_LED_OFF();
	BLUE_LED_OFF();    
	current_ui_display = UI_NONE;
	return 0;
}
//需要每10毫秒调用一次
int led_display_thread_10ms(void)
{
	LEDTick++;
	if(u8Start == 1)
	{
		s_u32CurTime = LEDTick;
		
		if((u8RedCount < s_LedAction->m_u16RedParaNum)&&(s_LedAction->m_u16RedParaNum > 0))
		{
			if((s_u32CurTime -s_u32RedPreTime >= s_LedAction->p_RedLedPara[u8RedCount].m_u32Interval)&&(u8RedStatus == 0))
			{
				RED_LED_ON();
				u8RedStatus = 1;
				s_u32RedPreTime = s_u32CurTime;				
			}
			else if((s_u32CurTime -s_u32RedPreTime >= s_LedAction->p_RedLedPara[u8RedCount].m_u32LastTime)&&(u8RedStatus ==1))
			{
				
				RED_LED_OFF();
				u8RedStatus = 0;
				s_u32RedPreTime = s_u32CurTime;

				u8RedCount ++;
				if((s_LedAction->m_u16RedRepeatMode == 1)&&(u8RedCount >= s_LedAction->m_u16RedParaNum))//one repeat cycle complete
				{
					if(0 == s_LedAction->m_u16RedRepeatCount) //Nonstop repeat
					{
						//continue repeat
						u8RedCount = s_LedAction->m_u16RedHeadCount;							
					}
					else
					{
                        u8RedRepeatCount ++;												
						if(u8RedRepeatCount < s_LedAction->m_u16RedRepeatCount) //repeat
						{
							//continue repeat
							u8RedCount = s_LedAction->m_u16RedHeadCount;							
						}
						else
						{
						    //red repeat end
						    u8EndFlag--;
						}
					}
					
                }
			  }
		}

		if((u8GreenCount < s_LedAction->m_u16GreenParaNum)&&(s_LedAction->m_u16GreenParaNum > 0))
		{
			if((s_u32CurTime -s_u32GreenPreTime >= s_LedAction->p_GreenLedPara[u8GreenCount].m_u32Interval)&&(u8GreenStatus == 0))
			{
				GREEN_LED_ON();
				u8GreenStatus = 1;
				s_u32GreenPreTime = s_u32CurTime;
			}
			else if((s_u32CurTime -s_u32GreenPreTime >= s_LedAction->p_GreenLedPara[u8GreenCount].m_u32LastTime)&&(u8GreenStatus == 1))
			{
				GREEN_LED_OFF();
				u8GreenStatus = 0;
				s_u32GreenPreTime = s_u32CurTime;
				
				u8GreenCount ++;
				if((s_LedAction->m_u16GreenRepeatMode == 1)&&(u8GreenCount >= s_LedAction->m_u16GreenParaNum))
				{
					if(0 == s_LedAction->m_u16GreenRepeatCount) //Nonstop repeat
					{
						//continue repeat
						u8GreenCount = s_LedAction->m_u16GreenHeadCount;							
					}
					else
					{
						u8GreenRepeatCount ++;						
						
						if(u8GreenRepeatCount <=s_LedAction->m_u16GreenRepeatCount) //repeat
						{
							//continue repeat
							u8GreenCount = s_LedAction->m_u16GreenHeadCount;							
						}
						else
						{
						    //green repeat end
						    u8EndFlag--;
						}
					}
				}
			}
		}

		if((u8BlueCount < s_LedAction->m_u16BlueParaNum)&&(s_LedAction->m_u16BlueParaNum > 0))
		{
			if((s_u32CurTime -s_u32BluePreTime >= s_LedAction->p_BlueLedPara[u8BlueCount].m_u32Interval)&&(u8BlueStatus == 0))
			{
				BLUE_LED_ON();
				u8BlueStatus = 1;
				s_u32BluePreTime = s_u32CurTime;
			}
			else if((s_u32CurTime -s_u32BluePreTime >= s_LedAction->p_BlueLedPara[u8BlueCount].m_u32LastTime)&&(u8BlueStatus == 1))
			{
				BLUE_LED_OFF();
				u8BlueStatus = 0;
				s_u32BluePreTime = s_u32CurTime;
				
				u8BlueCount ++;
				if((s_LedAction->m_u16BlueRepeatMode == 1)&&(u8BlueCount >= s_LedAction->m_u16BlueParaNum))
				{
					if(0 == s_LedAction->m_u16BlueRepeatCount) //Nonstop repeat
					{
						//continue repeat
						u8BlueCount = s_LedAction->m_u16BlueHeadCount;							
					}
					else
					{
						u8BlueRepeatCount ++;						
						
						if(u8BlueRepeatCount < s_LedAction->m_u16BlueRepeatCount) //repeat
						{
							//continue repeat
							u8BlueCount = s_LedAction->m_u16BlueHeadCount;							
						}
						else
						{
						    //blue repeat end
						    u8EndFlag--;
						}
					}
				}
			}
			
		}
		if(u8EndFlag == 0)
		{
		  //  DEBUG_LOG("led play end");
		    led_stop();
		    if(hold_led_ui)
		    {
		        LEDStart((UIDisplayType)hold_led_ui);
		        hold_led_ui = 0;
		    }
		}
	}
	return 0;
}

//Pairing
/***********************************************************************************/

static const LEDPara pairing_red[] = {
                                                                    {50,50}  ,            //white led1 on                      
                                                                    {50,50}              //white led1 on                      
                                                        };
                                                        
static const LEDPara pairing_blue[] = {
                                                                    {0,50}  ,            //white led1 on                      
                                                                    {50,50}              //white led1 on                      
                                                        };

static const LEDAction pairing = {
								(LEDPara*)pairing_red,				 //p_BlueLedPara
								(LEDPara*)pairing_blue,				 //p_BlueLedPara
								NULL,				 //p_GreenLedPara
								
								2,					 //m_u16RedParaNum
								2,					 //m_u16BlueParaNum
								0,					 //m_u16GreenParaNum
								
								1,					 //m_u16RedRepeatMode
								1,					 //m_u16BlueRepeatMode
								0,					 //m_u16GreenRepeatMode

								0,					 //m_u16RedRepeatCount
								0,					 //m_u16BlueRepeatCount // 0 = always play
								0,					 //m_u16GreenRepeatCount
								
								1,    				 //m_u16RedHeadCount
								1,					 //m_u16BlueHeadCount  //
								0,					 //m_u16GreenHeadCount
						   };

//Reconnecting
/***********************************************************************************/
static const LEDPara reconnecting_blue[] = { //1LSB is 50ms
								{10,10},
								{10,10},
                                			{190, 10},
						   	     };
static const LEDAction reconnecting = {
								NULL,  //p_RedLedPara
								(LEDPara*)reconnecting_blue,				 //p_BlueLedPara
								NULL,				 //p_GreenLedPara
								
								0,					 //m_u16RedParaNum
								3,					 //m_u16BlueParaNum
								0,					 //m_u16GreenParaNum
								
								0,					 //m_u16RedRepeatMode
								1,					 //m_u16BlueRepeatMode
								0,					 //m_u16GreenRepeatMode

								0,					 //m_u16RedRepeatCount
								0,					 //m_u16BlueRepeatCount
								0,					 //m_u16GreenRepeatCount
								
								1,    				 //m_u16RedHeadCount
								1,					 //m_u16BlueHeadCount
								1,					 //m_u16GreenHeadCount
						   };


/***********************************************************************************/

static const LEDPara connected_blue[] = {
                                                            {2,0xffffffff}              //white led1 on                      
                                                        };

static const LEDAction ble_connected =  {
								NULL,  //p_RedLedPara
								(LEDPara*)connected_blue, //p_BlueLedPara
								NULL,  //p_RedLedPara
								
								0,					 //m_u16RedParaNum
								1,					 //m_u16BlueParaNum
								0,					 //m_u16GreenParaNum
								
								0,					 //m_u16RedRepeatMode
								1,					 //m_u16BlueRepeatMode
								0,					 //m_u16GreenRepeatMode

								0,					 //m_u16RedRepeatCount
								0,					 //m_u16BlueRepeatCount
								0,					 //m_u16GreenRepeatCount
								
								0,    				 //m_u16RedHeadCount
								0,					 //m_u16BlueHeadCount
								0,					 //m_u16GreenHeadCount
						   };    
/***********************************************************************************/						   
static const LEDPara ota_blue[] = {
                                                            {2,0xffffffff},              //white led1 on                      
                                                        };
static const LEDPara ota_red[] = {
                                                            {2,0xffffffff},              //white led1 on                      
                                                        };

static const LEDAction ble_ota =  {
								(LEDPara*)ota_red,  //p_RedLedPara
								(LEDPara*)ota_blue, //p_BlueLedPara
								NULL,  //p_RedLedPara
								
								1,					 //m_u16RedParaNum
								1,					 //m_u16BlueParaNum
								0,					 //m_u16GreenParaNum
								
								1,					 //m_u16RedRepeatMode
								1,					 //m_u16BlueRepeatMode
								0,					 //m_u16GreenRepeatMode

								0,					 //m_u16RedRepeatCount
								0,					 //m_u16BlueRepeatCount
								0,					 //m_u16GreenRepeatCount
								
								0,    				 //m_u16RedHeadCount
								0,					 //m_u16BlueHeadCount
								0,					 //m_u16GreenHeadCount
						   }; 


/***********************************************************************************/

static const LEDPara ble_timeout_blue[] = {
                                                            {2,0xffffffff}              //white led1 on                      
                                                        };

static const LEDAction ble_timeout =  {
								NULL,  //p_RedLedPara
								(LEDPara*)ble_timeout_blue, //p_BlueLedPara
								NULL,  //p_RedLedPara
								
								0,					 //m_u16RedParaNum
								1,					 //m_u16BlueParaNum
								0,					 //m_u16GreenParaNum
								
								0,					 //m_u16RedRepeatMode
								0,					 //m_u16BlueRepeatMode
								0,					 //m_u16GreenRepeatMode

								0,					 //m_u16RedRepeatCount
								0,					 //m_u16BlueRepeatCount
								0,					 //m_u16GreenRepeatCount
								
								0,    				 //m_u16RedHeadCount
								0,					 //m_u16BlueHeadCount
								0,					 //m_u16GreenHeadCount
						   };         

/***********************************************************************************/

//Reconnecting
/***********************************************************************************/
static const LEDPara power_on_blue[] = { //1LSB is 50ms
                                                        {2,0xffffffff}              //white led1 on                      
						   	     };
static const LEDAction power_on = {
								NULL,  //p_RedLedPara
								(LEDPara*)power_on_blue,				 //p_BlueLedPara
								NULL,				 //p_GreenLedPara
								
								0,					 //m_u16RedParaNum
								1,					 //m_u16BlueParaNum
								0,					 //m_u16GreenParaNum
								
								0,					 //m_u16RedRepeatMode
								1,					 //m_u16BlueRepeatMode
								0,					 //m_u16GreenRepeatMode

								0,					 //m_u16RedRepeatCount
								0,					 //m_u16BlueRepeatCount
								0,					 //m_u16GreenRepeatCount
								
								0,    				 //m_u16RedHeadCount
								0,					 //m_u16BlueHeadCount
								0,					 //m_u16GreenHeadCount
						   };


/***********************************************************************************/

static const LEDPara battery_low_red[] = {
                                                {50, 100} ,
                                                //{50, 100},
                                                //{5, 10},
                                           };
                                                        

static const LEDAction battery_low =  {
								(LEDPara*)battery_low_red,  //p_RedLedPara
                                NULL,  //p_BlueLedPara								
								NULL, //p_GreenLedPara
								
								1,					 //m_u16RedParaNum
								0,					 //m_u16BlueParaNum
								0,					 //m_u16GreenParaNum
								
								1,					 //m_u16RedRepeatMode
								0,					 //m_u16BlueRepeatMode
								0,					 //m_u16GreenRepeatMode

								3,					 //m_u16RedRepeatCount
								0,					 //m_u16BlueRepeatCount
								0,					 //m_u16GreenRepeatCount
								
								0,    				 //m_u16RedHeadCount
								0,					 //m_u16BlueHeadCount
								0,					 //m_u16GreenHeadCount
						   }; 
/***********************************************************************************/

static const LEDPara note_store_red[] = {
                                                {30, 30} ,
                                           };
static const LEDPara note_store_blue[] = {
                                                {30, 30} ,
                                           };                                                        

static const LEDAction note_store =  {
								(LEDPara*)note_store_red,  //p_RedLedPara
                                (LEDPara*)note_store_blue,  //p_BlueLedPara								
								NULL, //p_GreenLedPara
								
								1,					 //m_u16RedParaNum
								1,					 //m_u16BlueParaNum
								0,					 //m_u16GreenParaNum
								
								1,					 //m_u16RedRepeatMode
								1,					 //m_u16BlueRepeatMode
								0,					 //m_u16GreenRepeatMode

								2,					 //m_u16RedRepeatCount
								2,					 //m_u16BlueRepeatCount
								0,					 //m_u16GreenRepeatCount
								
								0,    				 //m_u16RedHeadCount
								0,					 //m_u16BlueHeadCount
								0,					 //m_u16GreenHeadCount
						   }; 

                                                        
/***********************************************************************************/
static const LEDPara low_power_mode_blue[] = {
                                                {200, 50} ,
                                           };

static const LEDAction low_power_mode =  {
								NULL,  //p_RedLedPara
                                (LEDPara*)low_power_mode_blue,  //p_BlueLedPara								
								NULL, //p_GreenLedPara
								
								0,					 //m_u16RedParaNum
								1,					 //m_u16BlueParaNum
								0,					 //m_u16GreenParaNum
								
								0,					 //m_u16RedRepeatMode
								1,					 //m_u16BlueRepeatMode
								0,					 //m_u16GreenRepeatMode

								0,					 //m_u16RedRepeatCount
								0,					 //m_u16BlueRepeatCount
								0,					 //m_u16GreenRepeatCount
								
								0,    				 //m_u16RedHeadCount
								0,					 //m_u16BlueHeadCount
								0,					 //m_u16GreenHeadCount
						   }; 

/***********************************************************************************/
//Command list
/***********************************************************************************/
static const LEDAction * const CommandList[UI_COUNTS] =   {
														NULL, //UI_poweron
														&pairing, //UI_poweron
														&ble_connected, //UI_poweron
														&ble_timeout,
														&reconnecting,
														&power_on,
														&ble_ota,
														&low_power_mode,
														&battery_low,
														&note_store,
													};

uint8_t LEDStart(UIDisplayType led_display)
{
    if(led_display>UI_BATTERY_LOW)
    {
        return false;
    }

    if(current_ui_display == led_display)
    {
        return false;
    }

    RED_LED_OFF();
    GREEN_LED_OFF();
    BLUE_LED_OFF();
    if(CommandList[led_display] != NULL)
    {
        s_LedAction = (LEDAction*)CommandList[led_display]; 
        s_u32RedPreTime = LEDTick;
        s_u32GreenPreTime = LEDTick;
        s_u32BluePreTime = LEDTick;

        u8RedCount = 0;
        u8GreenCount = 0;
        u8BlueCount = 0;

        u8RedRepeatCount = 0;
        u8GreenRepeatCount = 0;
        u8BlueRepeatCount = 0;

        u8RedStatus= 0;
        u8GreenStatus = 0;
        u8BlueStatus = 0;

        u8Start = true;
        current_ui_display = led_display;
        u8EndFlag = 4;
        
        return true;
    }
    return false;
}
uint8_t LEDEventStart(UIDisplayType led_display)
{
    if((led_display>=UI_COUNTS) || (led_display < UI_BATTERY_LOW))
    {
        return false;
    }

    if(current_ui_display == led_display)
    {
        return false;
    }

    RED_LED_OFF();
    GREEN_LED_OFF();
    BLUE_LED_OFF();
    //nrf_delay_ms(10);
    if(CommandList[led_display] != NULL)
    {
        if(current_ui_display)
        {
            hold_led_ui = current_ui_display;
        }
        led_stop();
        s_LedAction = (LEDAction*)CommandList[led_display]; 
        s_u32RedPreTime = LEDTick;
        s_u32GreenPreTime = LEDTick;
        s_u32BluePreTime = LEDTick;

        u8RedCount = 0;
        u8GreenCount = 0;
        u8BlueCount = 0;

        u8RedRepeatCount = 0;
        u8GreenRepeatCount = 0;
        u8BlueRepeatCount = 0;

        u8RedStatus= 0;
        u8GreenStatus = 0;
        u8BlueStatus = 0;

        u8Start = true;
        current_ui_display = led_display;
        if(UI_BATTERY_LOW == led_display)
        {
            u8EndFlag = 1;
        }
        else if(UI_NOTE_STORE == led_display)
        {
            u8EndFlag = 2;
        }
        return true;
    }
    return false;
}

void Add_signal_count (void)
{
	
	write_signal_count ++;
}


void LED_Indicate_Has_Date(void)
{
	Div_clk++;
	if((0 == (Div_clk % 10)) && (DEVICE_ACTIVE ==  Get_Device_Status()))
    {
        //DEBUG_LOG("t[%d]", system_tick_counts);
        if(write_signal_count > write_signal_last_time)
        {
            write_interval = write_signal_count - write_signal_last_time;
        }
        else
        {
            write_interval = 0xffffffff - write_signal_last_time + write_signal_count;
        }
        if(write_interval < 30)
        {
            RED_LED_TOGLE();
		
        }
        else
        {
            
            RED_LED_OFF();
        }
        write_signal_last_time = write_signal_count;
    }
	
}

void low_BAT_power_on_led(void)
{
    uint8_t i = 0;

    RED_LED_OFF();
    //GREEN_LED_OFF();
    BLUE_LED_OFF();

    for(i = 0; i <= 3; i ++)
    {
        RED_LED_ON();
        HAL_Delay(200);
        RED_LED_OFF();
        HAL_Delay(200);
    }
}
