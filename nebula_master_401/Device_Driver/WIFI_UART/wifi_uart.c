#include "wifi_uart.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "config.h"
#include "server.h"
#include "ble_client.h"
#include "device_info.h"
#include "usb_host.h"
#include "usb_controler.h"
#include "Status_Machin.h"
#include "debug_log.h"
#include "Soft_Timer.h"
#include "display.h"
#include "misc.h"
#include "wifi_api.h"
#include "ewm3080b.h"

PACKET_ST  wifi_packet;
#define DEBUG  debug_log
extern st_device_data_storage g_device_info;

UART_HandleTypeDef huart6;
#define UART_WIFI_CONTROLER  &huart6

uint8_t RxBuffer[1];
char aRxBuffer[1];
char bRxBuffer[1];

uint8_t aRx_count=0;
uint8_t	write_packge_flag=0;
uint8_t write_packge_finish_flag=0;
uint8_t labble_count=0;
uint8_t debug_uart_buff[20];
uint8_t debug_uart_buff_index=0;




extern	 uint8_t	 wait_tcp_send_data_flag;
extern   uint8_t     wait_return_flag;
extern   uint8_t	 wifi_response;
extern 	 char from_wifi_buf[TRNASFER_MTU];
extern	 uint8_t	 Response_Status;


void wifi_transmit(char * p_tx_data, uint8_t len)  //WIFI AT指令/数据发送
{

    if(p_tx_data != NULL)
    {

        HAL_UART_Transmit(UART_WIFI_CONTROLER,(uint8_t *)p_tx_data, len,10);
    }
  
}


void  UART6_Callback(void)   
{
		aRxBuffer[0]=(uint8_t)(huart6.Instance->DR & (uint8_t)0x00FFU);
        debug_uart_buff[debug_uart_buff_index++]=aRxBuffer[0];
        if(debug_uart_buff_index==20)
        {
            debug_uart_buff_index=0;
        }

		if(write_packge_flag)
		{	
			
						
				 from_wifi_buf[aRx_count++]=aRxBuffer[0];		
		}
		else if((wait_tcp_send_data_flag)&&(aRxBuffer[0]==0x3E))  //CIPSEND先发送数据长度，等wifi模块回复‘>’(0x3E)后,再发送数据
		{
				wait_tcp_send_data_flag=0;
				Response_Status=AT_TCP_START_SEND_DATA;
				wifi_response=1;
		}
														
		if((aRxBuffer[0]==0x0a)&&(bRxBuffer[0]==0x0d))   //当前接收字节为‘\n’，下一字节为'\r'  则为包头或包尾
		{
				if(write_packge_flag)
				{
					
//					    if(wait_return_flag)//如果是等待查询结果 
//						{ 
//					
//							if(labble_count>1)
//							{
//								labble_count=0;
//								write_packge_flag=0;
//								write_packge_finish_flag=1;
//								
//								
//							}
//							else
//							{
//								
//								labble_count++;
//								
//							}
//							
//							
//						}
//						else
//						{
							labble_count=0;
							write_packge_flag=0;
							write_packge_finish_flag=1;
							
						//}
				
				}
				else
				{
						write_packge_flag=1;
						aRx_count=0;
				}
		}  
		
		bRxBuffer[0]=aRxBuffer[0];               			
		if(write_packge_finish_flag)
		{
				write_packge_finish_flag=0;
				wifi_Response_callback();
				aRxBuffer[0]=0;
				bRxBuffer[0]=0;
		}
}

