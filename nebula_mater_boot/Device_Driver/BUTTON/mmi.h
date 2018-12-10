#ifndef __MMI_H_
#define __MMI_H_
#include "stdint.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"


extern  void power_button_long_press(void);
extern void power_button_short_release(void);
extern void power_button_double_press(void);
extern void up_page_button_short_press(void);
extern void down_page_button_short_press(void);


typedef enum _ButtonsNumber
{
   BTN_PWR=0,
   BTN_UP,
	 BTN_C,
	 BTN_D,
	 BTN_DWN,
	 BTN_A,
	 BTN_B,
	 BTN_TOTAL_NO,
}ButtonsNumber;

typedef enum buttonevents
{
   NO_BUTTON_EVENT,
   SHORT_PRESS,
   DOUBLE_PRESS,
   LONG_PRESS,
   VERY_LONG_PRESS,
   KEEP_PRESS
}Buttonevents;

enum{
	
	DIGITAL_IO,
	ANOLOG_IO,
	
	
};



typedef struct buttonstatus
{
   uint8_t StartEventDetect;
	 uint8_t type;
	 uint8_t anolog_channal;
	 uint16_t threshold;
   uint8_t pinStatus;
   uint8_t pinLastStatus;
   uint8_t short_press_counts;
   uint8_t short_press_interval;
   uint16_t buttonPressedTimeslots;
   uint16_t LongPressThreshold;  	
   void (* press_callback)(void);
   void (* short_release_callback)(void);
   void (* double_press_callback)(void);
   void (* long_release_callback)(void);
   void (* keep_press_callback)(void);
}PinStatus;


void mmi_init(void);
void ButtonProcessLoop(void);
static void ButtonEventProcess(void);
static Buttonevents CheckButtonEvent(ButtonsNumber buttonNO);




#endif

