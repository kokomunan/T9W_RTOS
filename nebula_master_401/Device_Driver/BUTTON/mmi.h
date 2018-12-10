#ifndef __MMI_H_
#define __MMI_H_
#include "stdint.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"


extern void button_det_short_release(void);
extern void button_det_double_press(void);
extern void button_det_long_press(void);
extern void button1_short_press(void);
extern  void button2_short_press(void);
extern  void button3_short_press(void);
extern  void button4_short_press(void);
extern  void button5_short_press(void);
extern  void button6_short_press(void);
extern  void button7_short_press(void);
extern  void button8_short_press(void);
extern  void button9_short_press(void);
extern  void button10_short_press(void);
typedef enum _ButtonsNumber
{
    BTN_DET=0,
    BTN_1,
    BTN_2,
    BTN_3,
    BTN_4,
    BTN_5,
    BTN_6,
    BTN_7,
    BTN_8,   
    BTN_9, 
    BTN_10,     
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

