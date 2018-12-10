#ifndef  _DISPLAY_H_
#define  _DISPLAY_H_
#include "stdint.h"



enum{
	
	NEED_UPDATE_BATERRY,
	NEED_UPDATE_CONNECT_ICON,
	NEED_UPDATE_WRITING_ICON,
	NEED_UPDATE_NOTE_INFOR_ICON,
	NEED_UPDATE_ALL,
		
};


void Display_init(void);
void Dsiplay_Thread(void);
void Dispalay_deinit(void);
void update_oled_test_mode(uint8_t mode);
void update_dis_battery_value(uint8_t level);
void update_dis_offline_page(uint8_t cur,uint8_t total);
void update_dis_online_page(uint8_t cur,uint8_t total);
void update_dis_Pen_icon(void);
void update_dis_all(void);


#endif
