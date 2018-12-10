#ifndef WIFI_UART_H_
#define WIFI_UART_H_

#include "stm32f4xx_hal.h"
#include "mxconstants.h"
#include "config.h"
#include "stdint.h"

#define WIFI_BUF_NO                      32

#pragma pack(1)
typedef struct  {
	
	uint8_t HEAD;
	uint8_t CMD;
	uint8_t LEN;
	uint8_t message[30];
	

}PACKET_ST;



typedef struct  {
	
	PACKET_ST packet[16];
	uint8_t mesage_num;
	
}RESPONSE_BOX;

#pragma pack()
enum RESPONSE{

	RESPONSE_READY=0x20,  //响应就绪
	RESPONSE_OK,       //操作成功
  RESPONSE_STA,      //响应连接状态
	RESPONSE_HOST,     //HOST 传来数据
	RESPONSE_ERROR      //操作失败
};
enum ERROR{
	
	TX_BUFF_FULL,
	STATE_ERR,
	
};

void wifi_transmit(char * p_tx_data, uint8_t len);


void wifi_Response_callback(void);
uint32_t  send_msg_to_wifi(uint8_t *data ,uint8_t len);
uint8_t send_wifi_data_buffer_to_host(void);

#endif

