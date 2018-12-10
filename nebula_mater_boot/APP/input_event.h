#ifndef _INPUT_EVENT_H
#define _INPUT_EVENT_H
#include "stdint.h"


void Add_page_index(void);

void input_init(void);
void input_thread(void);
void set_currunt_note_id(uint32_t ID);
void init_display_currunt_note_index(uint32_t index);
#endif








