#include "ewm3080b.h"
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
#include "wifi_client.h"

#include "wifi_uart.h"
#include "wifi_api.h"




uint8_t  packet_index=0;
uint32_t wifi_send_queue_input_index = 0;   //以后将队列抽象成对象！
uint32_t wifi_send_queue_output_index = 0;
uint32_t wifi_recive_queue_input_index = 0;
uint32_t wifi_recive_queue_output_index = 0;

struct WIFI_PACKET  wifi_send_queue[QUEUE_MAX_NUM];
struct WIFI_PACKET  wifi_recive_queue[QUEUE_MAX_NUM];

extern uint8_t RxBuffer[1];
extern st_device_data_storage g_device_info;
extern UART_HandleTypeDef huart6;

EWM3080_handle  ewm3080_handle;

osThreadId  wifi_process_thread_handle=NULL;



EWM3080_OPS  wifi_ops={
	
    0,
    0,
    Open_wifi,
    Close_wifi,
    Ioctl_wifi,
    Read_wifi,
    Write_wifi,
    manager_wifi,
    wifi_clear_buff,
    wifi_direct_write ,	
};

void MX_USART6_UART_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    debug_log("init uart6 failed");
  }
	__HAL_UART_ENABLE(&huart6);
  
  //设置为输出后拉高
    GPIO_InitStruct.Pin =WIFI_PWR_CTRL_Pin;        //EWM3080
    GPIO_InitStruct.Mode =GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed =GPIO_SPEED_FREQ_LOW ;
    HAL_GPIO_Init(WIFI_PWR_CTRL_GPIO_Port, &GPIO_InitStruct);
  
    GPIO_InitStruct.Pin =WIFI_BOOT_Pin;        
    GPIO_InitStruct.Mode =GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed =GPIO_SPEED_FREQ_LOW ;
    HAL_GPIO_Init(WIFI_BOOT_Port, &GPIO_InitStruct);

   
    GPIO_InitStruct.Pin = WIFI_RESET_Pin;
    GPIO_InitStruct.Mode =GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(WIFI_RESET_Port, &GPIO_InitStruct);
  
   
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_SET);  //boot
  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4, GPIO_PIN_SET);   //reset
  
}

void Open_wifi(void)
{
  
    MX_USART6_UART_Init();    
    SET_BIT(huart6.Instance->CR1, USART_CR1_RXNEIE);    
    OPEN_EWM3080_POWER();   
    memset(&ewm3080_handle,0,sizeof(ewm3080_handle));
	//	HAL_UART_Receive_IT(&huart6,RxBuffer,1);
    
}
void Close_wifi(void)
{
    
    if(ewm3080_handle.close_pipe!=NULL) 
    {
       ewm3080_handle.close_pipe(); 
    }
   
    stop_station_up();
    CLOSE_EWM3080_POWER();
    HAL_UART_DeInit(&huart6);

    osThreadTerminate (wifi_process_thread_handle);    
    
    
}
uint8_t Ioctl_wifi(uint8_t cmd,uint8_t *para)
{
    uint8_t res;
    switch(cmd)
    {
        
        
        case   CREATE_INIT_PROCESS:
                  
        res=wifi_AT_commond_set(UART_SET,"115200,8,1,NONE,NONE"); //配置串口      
        if(res==0)
        {
             
           
            osThreadDef( process_loop ,process_loop , osPriorityNormal, 0, 256); 
            wifi_process_thread_handle=osThreadCreate(osThread(process_loop ), NULL); 
                       
            
        }
        else
        {
            return 1;
        }


        break;
        
                 
    }
      
    return 0;
    
    
}

uint8_t Read_wifi(uint8_t *msg,uint8_t len)
{
  
    
    uint32_t buf_index = 0;
    
    
        if(wifi_recive_queue_input_index > wifi_recive_queue_output_index) 
        {   
            buf_index = wifi_recive_queue_output_index%QUEUE_MAX_NUM;
                
            memcpy(msg,&wifi_recive_queue[buf_index],len);
            
            wifi_recive_queue_output_index++;
            return 0;
        }
        else
        {
            return 1;
            
        }

    

}

uint8_t Write_wifi(uint8_t opcode, uint8_t * data, uint16_t len)
{
    
       
    uint32_t i = 0;
    uint8_t buf_index = 0;
    if(wifi_ops.status != PIPE_CONNECT)
    {
       
        return 1;  

    }
    if(len > TRANS_PALOAD_MAX_LEN)
    {
        debug_log("err due to len ");
        return 1; 
    }

    i = wifi_send_queue_input_index - wifi_send_queue_output_index;
    if(i >= QUEUE_MAX_NUM)
    {   
        debug_log("err due to overspace ");
        return 1;
    }

    buf_index = wifi_send_queue_input_index%QUEUE_MAX_NUM;
    wifi_send_queue[buf_index].identifier = 0xAA;
    
   
    if(opcode>=0x80)
    {
        if(opcode==CMD_POS_DATA)  
        {
            opcode=WIFI_CMD_REPORT_POSITION;

        }
        else if(opcode==CMD_STATUS)
        {
            
            opcode=WIFI_CMD_GET_STATUS ;
        
        }
        else if(opcode==CMD_BTN_EVENT)
        {
            opcode=WIFI_CMD_BTN_EVENT;

        }
        else if(opcode==CMD_SYNC_IS_END)
        {
            opcode=WIFI_CMD_SYNC_IS_END;

        }
        else if(opcode==CMD_REPORT_PAGE_AUTO)
        {
            opcode= WIFI_CMD_REPORT_PAGE ;

            
        }
        else if(opcode==CMD_OTA_EMR_FILE_INFO)
        {
            
            opcode= WIFI_CMD_OTA_EMR_FILE_INFO ;

                       
        }
         else if(opcode==CMD_OTA_EMR_RAW_DATA)
        {
            
            opcode= WIFI_CMD_OTA_EMR_RAW_DATA ;

                  
        }
        else if(opcode==CMD_OTA_EMR_CHECKSUM)
        {
            
            opcode= WIFI_CMD_OTA_EMR_CHECKSUM ;
           
            
        }
        else if(opcode==CMD_OTA_EMR_RESULT)
        {
            
            opcode= WIFI_CMD_OTA_EMR_RESULT ;
             
        } 
        else if(opcode== CMD_ENTER_FRQ_ADJUST)
        {
            
            opcode= WIFI_CMD_ENTER_FRQ_ADJUST;

                   
        }
        else if(opcode==CMD_QUIT_FRQ_ADJUST)
        {
            
            opcode= WIFI_CMD_QUIT_FRQ_ADJUST;
                       
        }            
        else 
        {
            return 0;
        }
    }

      
    wifi_send_queue[buf_index].opcode = opcode;
    wifi_send_queue[buf_index].length = len;
  
    wifi_send_queue[buf_index].index=packet_index;
    
    if(len)
    {
        memcpy(wifi_send_queue[buf_index].payload, data, len);
    }
    wifi_send_queue_input_index ++;
    packet_index++;
    return 0;

}


uint8_t Send_wifi(void)    
{
        uint32_t buf_index = 0;      
        if(wifi_send_queue_input_index > wifi_send_queue_output_index) 
        {   
            buf_index = wifi_send_queue_output_index%QUEUE_MAX_NUM;
                
            if(ewm3080_handle.send_pipe((uint8_t *)&wifi_send_queue[ buf_index],wifi_send_queue[ buf_index].length+4)==0)
            {
               wifi_send_queue_output_index++;
               return 0; 
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
            
        }
}
uint8_t Revice_wifi(uint8_t * data, uint16_t len)
{
  
    uint32_t i = 0;
    uint8_t buf_index = 0;
    if(wifi_ops.status != PIPE_CONNECT)
    {
       
        return 1;  

    }
    if(len > TRANS_PALOAD_MAX_LEN)
    {
        debug_log("err due to len ");
        return 1; 
    }

    i = wifi_recive_queue_input_index - wifi_recive_queue_output_index;
    if(i >= QUEUE_MAX_NUM)
    {   
        debug_log("err due to overspace ");
        return 1;
    }

    buf_index = wifi_recive_queue_input_index%QUEUE_MAX_NUM;  
    memcpy((uint8_t *)&wifi_recive_queue[buf_index], data, len);  
    wifi_recive_queue_input_index ++;

    
    return 0; 
}

void process_loop(const void *argument)
{
    uint8_t res;
  
   while(1)
   {       
    
    
       if(ewm3080_handle.wifi_status==STATION_DOWN) 
       {  
            res=start_station_up();
           if(res)
           {
               debug_log("start station up faild")

           }
           else
           {
               ewm3080_handle.wifi_status=STATION_CONNECTING;
           }
           
       }
       else if(ewm3080_handle.wifi_status==STATION_UP) 
       {
           res=start_tcp_client();
           if(res)
           {
               debug_log("start tcp faild")
           
           
           }
           else
           {
                ewm3080_handle.wifi_status=TCP_CONNECTING;
           }
               
       }
       else if(ewm3080_handle.wifi_status==TCP_DISCONNCET)
       {
			ewm3080_handle.wifi_status=STATION_UP ;
           
       }
                    
       osDelay(100);
   }
   
   
   
}
void wifi_status_loop(void)
{

    
    if(ewm3080_handle.last_status!=wifi_ops.status)
    {
        ewm3080_handle.last_status=wifi_ops.status;
        server_status_call_back(wifi_ops.status);                    
    }   
    
     
}

void manager_wifi(void)
{
    uint8_t data_temp[TRANS_MTU];

    if(Read_wifi(data_temp,TRANS_PALOAD_MAX_LEN)==0)
    {          
        wifi_client(data_temp); 
    }

    Send_wifi();      
    wifi_status_loop();  

    
}
uint8_t wifi_direct_write (uint8_t *data ,uint8_t len)
{
    return ewm3080_handle.send_pipe(data,len);
}

void wifi_clear_buff(void)
{
    wifi_send_queue_input_index = 0; 
    wifi_send_queue_output_index = 0;
    wifi_recive_queue_input_index = 0;
    wifi_recive_queue_output_index = 0;

		
}
void PIPE_CREATE_SUCCESS_callback(void) //接收到服务器应答
{
     
    
    
}

void STATION_UP_callback(void)
{
    ewm3080_handle.wifi_status=STATION_UP;//设置状态为已连接AP
  //  debug_log("station up");
    
}
void STATION_DOWN_callback(void)
{
    ewm3080_handle.wifi_status=STATION_CONNECTING;//设置状态为未连接
    ewm3080_handle.send_pipe=NULL;
    ewm3080_handle.Recive_pipe=NULL;//清空数据管道函数指针
       
    wifi_ops.status=PIPE_DISCONNECT;
  //  debug_log("station down");

}
void TCP_SERVER_CONNECTED_callback(void)
{
   // debug_log("tcp connect");
    ewm3080_handle.wifi_status= TCP_CONNECT;//设置状态为已连接 //设置数据管道为TCP的接收和发送函数   
    ewm3080_handle.send_pipe=tcp_send;
    ewm3080_handle.Recive_pipe=tcp_recive;
    ewm3080_handle.close_pipe=stop_tcp_client;
    wifi_ops.status=PIPE_CONNECT;
    packet_index=0;
    Write_wifi(WIFI_CMD_REPORT_MAC, (uint8_t *)g_device_info.ble_addr,6);
   


}
void TCP_SERVER_CLOSED_callback(void)
{
    ewm3080_handle.wifi_status=TCP_DISCONNCET;//设置TCP断开
    ewm3080_handle.send_pipe=NULL;
    ewm3080_handle.Recive_pipe=NULL;//清空数据管道函数指针
    ewm3080_handle.close_pipe=NULL;
    wifi_ops.status=PIPE_DISCONNECT;
 //   debug_log("tcp close");
    
}

void TCP_SERVER_DISCONNECTED_callback(void)
{
    ewm3080_handle.wifi_status=TCP_DISCONNCET;//设置TCP断开
    ewm3080_handle.send_pipe=NULL;
    ewm3080_handle.Recive_pipe=NULL;//清空数据管道函数指针
    ewm3080_handle.close_pipe=NULL;
    wifi_ops.status=PIPE_DISCONNECT;
 //   debug_log("tcp disconnceted");   
    
    
    
}






void MQTT_CONNECT_SUCCESS_callback(void)
{
     debug_log("Mqtt connect");
    
}

void MQTT_CONNECT_FAIL_callback(void)
{
      debug_log("Mqtt connect fail");
    
}

void MQTT_CONNECT_RECONNECTING_callback(void)
{
     debug_log("Mqtt reconnecting");
    
}

void MQTT_CLOSE_SUCCESS_callback(void)
{
     debug_log("Mqtt clsoe success");
}

void MQTT_CLOSE_FAIL_callback(void)
{
    debug_log("Mqtt clsoe fail");
}

void MQTT_SUBSCRIBE_SUCCESS_callback(void)
{
     debug_log("Mqtt subscribe success");
}

void MQTT_SBUSCRIBE_FAIL_callback(void)
{
     debug_log("Mqtt subscribe fail");
}
void MQTT_PUBLISH_SUCCESS_callback(void)
{
     debug_log("Mqtt publish success");
}

void MQTT_PUBLISH_FAIL_callback(void)
{
     debug_log("Mqtt publish fail");
}

void ALINK_STATUS_callback(uint8_t status)
{
     debug_log("Alink status");
}

void OTA_START_callback(void)
{
     debug_log("OTA start");
}

void OTA_END_callback(void)
{
     debug_log("OTA end");
}




uint8_t start_station_up(void)
{
   uint8_t res;
    
   wifi_AT_commond_set(WDHCP_SET,"ON");   					 //设置WDHCP   
   res= wifi_AT_commond_set(WJAP_SET,AP_SSID_KEY);   //设置为station
  
   return res;
    
}

uint8_t stop_station_up(void)
{
    
   uint8_t res;
   res= wifi_AT_commond_excute(WJAPQ);   //设置为station
  
   return res;

}


uint8_t start_alink_client(void)
{
    
   return 0;
    
}
uint8_t stop_alink(void)
{
   return 0; 
}


uint8_t  start_tcp_client(void)
{
		if(!(wifi_AT_commond_set(CIPAUTOCONN_SET,"0,1")))  //关闭自动连接 
		{
				if(!(wifi_AT_commond_set(CIPSTOP_SET,"0")))  //关闭TCP连接
				{
						if(!(wifi_AT_commond_set(CIPSTART_SET,TCP_ADR)))  //开启tcp连接 
						{
								return 0;
						}
				}
		}   
    return 1;      
}


uint8_t stop_tcp_client(void)
{
   uint8_t res;
   res=wifi_AT_commond_set(CIPSTOP_SET,TCP_ID);//开启tcp连接  
   return res; 
    
}




uint8_t start_mqtt_client(void)
{
//wifi_AT_commond_set(MQTTEVENT_SET,TCP_ID);//使能推送
//设置用户信息
//设置sockt
//关闭证书
//关闭ssl
//设置 client  id
//设置心跳
//设置自动重连
//启动服务
//订阅主题
//发布设置
//并将mqtt 接收 传递给    Recive_pipe
//将mqtt 发送 传送给  send_pipe   
    ewm3080_handle.close_pipe=stop_mqtt_client;  
    return 0;    
}

uint8_t stop_mqtt_client(void)
{
  //取消订阅
  //关闭mqtt   
  return 0;
    
    
}



uint8_t tcp_send(uint8_t * data,uint8_t len)
{
    
    uint8_t res;

    res = wifi_data_transmit(data, len);
    if(!res)
    {
        return 0;  
    }            
    return 1;
}


uint8_t tcp_recive(uint8_t *data,uint8_t len)  
{
             
   return Revice_wifi(data, len);
 
}

uint8_t HAL_WIFI_data_recive(uint8_t *data,uint8_t len)
{
    
    return ewm3080_handle.Recive_pipe(data,len);
    
    
}
void HAL_WIFI_return_result(uint8_t *data,uint8_t len)
{
    
    memcpy(ewm3080_handle.return_data,data,len);
    
}

