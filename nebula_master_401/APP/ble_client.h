#ifndef _BLE_CLIENT_H
#define _BLE_CLIENT_H
#include "stdint.h"

#define ENABLE_NORTIFY      nortify_enable=1
#define DISABLE_NORTIFY     nortify_enable=0
#define IS_ENABLE_NOTIFY    nortify_enable

void host_client(uint8_t *p_event_data);
void send_error_msg(uint8_t error);

typedef struct  
{
    uint32_t sw_version; 
    uint32_t img_length;
} st_jedi_fw_info;


extern uint8_t nortify_enable;

#endif

