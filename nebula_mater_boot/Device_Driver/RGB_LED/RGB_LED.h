#ifndef  _RGB_LED_H_
#define  _RGB_LED_H_

#include "stdint.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "config.h"
typedef enum UIdisplay
{
/*0*/   UI_NONE = 0,
        UI_PAIRING,
        UI_CONNECTED,
        UI_BLE_TIMEOUT,
        UI_RECONNECTING,
        UI_POWER_ON,
        UI_OTA,
        UI_LOW_POWER_MODE,
        //EVENT
        UI_BATTERY_LOW,
        UI_NOTE_STORE,
        UI_COUNTS
}UIDisplayType ;

#define RED_LED_ON()     HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET)
#define RED_LED_OFF()	   HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET)

#define GREEN_LED_ON()	 HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET)
#define GREEN_LED_OFF()	 HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET)

#define BLUE_LED_ON()		 HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_RESET)
#define BLUE_LED_OFF()	 HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET)

#define RED_LED_TOGLE()  HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin)
#define BLUE_LED_TOGLE()   HAL_GPIO_TogglePin(LED_B_GPIO_Port,LED_B_Pin)


typedef struct LEDpara 
{
	unsigned long m_u32Interval;    			//uint 10ms //light off time
	unsigned long m_u32LastTime; 			    //unit 10ms     //light on time    

} LEDPara;

typedef struct LEDAction
{
	LEDPara* p_RedLedPara;
	LEDPara* p_BlueLedPara;
	LEDPara* p_GreenLedPara;
	
	uint8_t m_u16RedParaNum;
	uint8_t m_u16BlueParaNum;
	uint8_t m_u16GreenParaNum;
	
	uint8_t m_u16RedRepeatMode:1; //0: no repeat   1:repeat
	uint8_t m_u16BlueRepeatMode:1; //0: no repeat   1:repeat
	uint8_t m_u16GreenRepeatMode:1; //0: no repeat   1:repeat

	uint8_t m_u16RedRepeatCount;
	uint8_t m_u16BlueRepeatCount;
	uint8_t m_u16GreenRepeatCount;
		
	uint8_t m_u16RedHeadCount;
	uint8_t m_u16BlueHeadCount;
	uint8_t m_u16GreenHeadCount;
}LEDAction;


extern LEDPara ahs_vibration_red;
extern LEDPara ahs_vibration_motor;					   	     
extern LEDAction ahs_vibration;

void leds_init(void);
int led_stop(void);
uint8_t LEDStart(UIDisplayType led_display);
int led_display_thread_10ms(void);
uint8_t LEDEventStart(UIDisplayType led_display);
void LED_Indicate_Has_Date(void);
void Add_signal_count (void);
void low_BAT_power_on_led(void);
#endif
