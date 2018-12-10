#ifndef __SERVER_H_
#define __SERVER_H_

#include "stdint.h"

typedef struct  
{

	uint8_t ready;
	uint8_t status;
  void (* Open)      (void);
	void (*Close)     (void);
  uint8_t (* Ioctl)(uint8_t cmd,uint8_t *);
  uint8_t (* Read)(uint8_t *, uint8_t );   
	uint8_t (*Write)     (uint8_t opcode, uint8_t * data, uint16_t len) ;
	void    (*manager)   (void);	
	void  (*clear_buff)  (void);
	uint8_t (*Write_direct)  (uint8_t *data ,uint8_t len);
	
} SERVIVE_OPS;

enum IOCTL_CMD{
	
	INIT=0x00,  //²éÑ¯¾ÍÐ÷
	START_WORK,
	STOP_WORK,
  
	
};

enum SERVER_MODE{
	
	NRF51822=0,
	EM9203,
	
	
};


void server_thread(void);
void server_init(uint8_t mode);
void server_stop(void);
uint8_t get_server_mode(void);
uint8_t server_Send_position_to_host(uint8_t opcode, uint8_t * data, uint16_t len);
uint8_t  server_notify_host(uint8_t opcode, uint8_t * data, uint16_t len);
void server_status_call_back(uint8_t sta);
uint8_t server_send_direct(uint8_t *data ,uint8_t len);
uint8_t get_server_status(void);
uint8_t get_server_ready(void);
void set_server_ready(uint8_t v);
#endif










