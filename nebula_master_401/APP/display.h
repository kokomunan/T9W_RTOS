#ifndef  _DISPLAY_H_
#define  _DISPLAY_H_
#include "stdint.h"
#include "cmsis_os.h"


enum{
	
	NEED_UPDATE_BATERRY,
	NEED_UPDATE_CONNECT_ICON,
	NEED_UPDATE_WRITING_ICON,
	NEED_UPDATE_NOTE_INFOR_ICON,
    NEED_UPDATE_ADR_INFO,
	NEED_UPDATE_ALL,
		
};


enum{
    MAIN_SCREEN,
    CHOSE_SCREEN,
    HOTPLUG_SCREEN,
    WAIT_SCREEN,
    PARING_SCREEN,
    LOWPOER_SCREEN,
    TEST_SCREEN, 
    TEST_NRF_SCREEN,
      
    
};

enum CHOSE_INDEX{
    
   BLE_SERVER_INDEX,
   LAN_SERVER_INDEX,
   USB_SERVER_INDEX,
    
};


enum NEBULA_EVENT{
   NO_EVENT,
   STATUS_CHANGE,
   KEY_CHANGE,
   CHOOSE_OVER,
   REDRAW,
   
 
};



#define CENTER_BLANK    10
#define X_TOTAL_LEN     128
#define ICON_LEN        32

void Display_init(void);
void Dsiplay_Thread(const void * argument);
void Dispalay_deinit(void);
void update_oled_test_mode(uint8_t mode);
void update_dis_battery_value(uint8_t level);
void update_dis_offline_page(uint8_t cur,uint8_t total);
void update_dis_online_page(uint8_t cur,uint8_t total);
void update_dis_Pen_icon(void);
void update_adr_icon(void);
void update_dis_all(void);
void set_key_event(uint8_t key_value);
uint8_t get_currunt_screen(void);
void change_screen(uint8_t new_screen);
void power_off_battery_Display_init(void);
void power_off_battery_Display_deinit(void);
void power_off_battery_Display_Thread(const void* argument);
void send_event_to_nebula_screen(uint8_t event);
void init_vote_value(void);
void add_vote_value(uint8_t value,uint8_t mode);
void delete_vote_value(void);
void stop_vote_operate(void);
void Display_waite_updata(void);
uint8_t If_vote_en(void);
void redraw_main_sceen(void);
void update_oled_test_key(uint8_t key_value);
void notify_low_power(void);
uint8_t If_has_vote(void);
void  stop_display_charge_full(void const *argument);
void delay_charge_full(void const *argument);
void return_main_screem_timer(const void* argument);
#endif
