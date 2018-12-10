#ifndef __USB_CONTROLER_H_
#define __USB_CONTROLER_H_
#include "mxconstants.h"
#include "stm32f4xx_hal.h"

#define set_client_rcv_flag()       usb_client_rcv_flag=1
#define clear_client_rvc_flag()      usb_client_rcv_flag=0
#define if_client_rcv_flag()        usb_client_rcv_flag==1


enum {
    
DISCONNECT,
CONNECT,    
    
    
};



extern uint8_t usb_client_rx_buff[64];
extern uint8_t usb_cmd_tx_buff[64];
extern uint8_t  usb_client_rcv_flag;

void set_rcv_flag(void);
uint8_t if_rcv_flag(void);
void clear_rvc_flag(void);
void create_usb_packet(uint8_t opcode,uint8_t len,uint8_t *payload);
void usb_check_status(void);
void create_usb_packet(uint8_t opcode,uint8_t len,uint8_t *payload);
void set_send_flag(void);
void usb_controler_init(void);
void usb_controler_Thread(void);
uint8_t Is_usb_connect(void);
void clear_usb_hosplug(void);
uint8_t Is_usb_hotplug(void);
void usb_open(void);
void usb_close(void);
uint8_t usb_ioctl(uint8_t cmd,uint8_t *param);
uint8_t usb_read(uint8_t *buff, uint8_t len );
uint8_t usb_write (uint8_t opcode, uint8_t * data, uint16_t len);
uint8_t usb_write_dirct(uint8_t *data ,uint8_t len);
void usb_manager(void);
void  usb_clear_buff(void);
void usb_interface_manager(void);
void usb_main_task(void const * argument)  ;
void usb_controler_Task(void const * argument);
#endif


