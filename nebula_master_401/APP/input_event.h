#ifndef _INPUT_EVENT_H
#define _INPUT_EVENT_H
#include "stdint.h"


enum KEY_EVENT{
    
    NON_KEY,
    CONFIRM_KEY,
    RETURN_KEY,
    FORWAD_KEY,
    BACK_KEY,
    
    
};


#define SELCET_MODE    HAL_GPIO_ReadPin(BUTTON_MODE_SELECT_Port,BUTTON_MODE_SELECT_Pin)

   
void Add_page_index(void);

void input_init(void);
void input_thread(const void *argumen);
void select_mode_thread(const void *argumen);
void set_currunt_note_id(uint32_t ID);
void init_display_currunt_note_index(uint32_t index);


#endif








