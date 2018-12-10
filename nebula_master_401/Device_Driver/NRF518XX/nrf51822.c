#include "nrf51822.h"
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

extern st_device_data_storage g_device_info;
extern SPI_HandleTypeDef hspi3;
uint8_t usb_dfu_step;
extern uint8_t mcu_need_update;
extern uint8_t ota_step;
extern uint8_t JEDI_VERSION;
extern uint8_t JEDI_SURPORT_CALIBRA_FLAG;
#define  SPI_CONTROLER    &hspi3
#define BLE_SLAVE_BUFF      32//缓冲池大小
uint32_t ble_cmd_buffer_input_index = 0;
uint32_t ble_cmd_buffer_output_index = 0;
struct  BLEDataFormat ble_data_buffer[BLE_SLAVE_BUFF] = {0};//缓冲池


PACKET_ST  packet;
static	uint8_t to_ble_buf[TRNASFER_MTU];
static	uint8_t from_ble_buf[TRNASFER_MTU];               
static  uint8_t back_buf[TRNASFER_MTU];                    
static  uint8_t ble_opened_flag=0;
uint8_t Ble_Response=0;
uint8_t Host_Request_Flag=0;
uint8_t currunt_status=0;
uint8_t ble_device_ack=0;

TimerHandle_t resend_timer_handle;

BLE518XX_OPS  ble_ops={
	
    0,
    0,
    Open_ble,
    Close_ble,
    Ioctl_ble,
    Read_ble,
    Write_ble,
    manager_ble,
    ble_clear_buff,
    ble_direct_write ,	
};

void Open_ble(void)
{
	ble_opened_flag=1;
    Ble_Response=0;

}

void  Close_ble(void)
{

    ble_opened_flag=0;
    CLOSE_BLE_POWER() ;
    BLE_SELECT();    //拉低片选以免倒灌电流  
    osDelay(200);
    OPEN_BLE_POWER();	
    BLE_RELEASE() ;	
  //debug_log("reset ble ");
  //重启蓝牙
	
}

//操作蓝牙设备

uint8_t Ioctl_ble(uint8_t cmd,uint8_t *para)
{
    uint8_t res;
    //uint8_t test_addr[6]={1,2,3,4,5,6};
  
	switch(cmd)
	{
		case INIT://如果有事件 并且是就绪响应 将结果返回	
        {
           
            st_notify_ble_info  notify_packet;
            notify_packet.mcu_firmware_version=SW_VERSION;    
            if((g_device_info.ble_addr[0]!=27)||(g_device_info.ble_addr[1]!=HW_VERSION))//如果地址段非法
            {
              uint32_t cpuid[3];
              read_cpu_id(cpuid);
              g_device_info.ble_addr[0]=0x27 ;
              g_device_info.ble_addr[1]=HW_VERSION ;      
              g_device_info.ble_addr[2]=0;
              g_device_info.ble_addr[3]=cpuid[0];  
              g_device_info.ble_addr[4]=cpuid[1];  
              g_device_info.ble_addr[5]=cpuid[2];  
                                            
            }
            memcpy(notify_packet.ble_addr,g_device_info.ble_addr,6);
            notify_packet.hard_version=g_device_info.hard_version;
            //memcpy(notify_packet.ble_addr,test_addr,6);
            Create_Packet(QUERY_READY,(uint8_t *)&notify_packet,sizeof(st_notify_ble_info));  //发送自己的固件版本和MAC地址
            ble_rw(to_ble_buf, NULL);//发送命令包到ble 
            res=Wait_Response(500);  //等待响应 
            Ble_Response=0;
            if(res==0)
            {
                
                if(packet.CMD==RESPONSE_READY)
                {	
                    
                    memcpy(&g_device_info.ble_firmware_version,packet.message,2);
                    
                    ble_ops.status=INITIALIZATION;
                    return 0;
                    
                }
                else
                {
                    return 1;	
                }                
            }	
        }		
		break;		
        case START_WORK:

        if(*para)
        {

            Create_Packet(SET_MODE_PARING,0,0);
            ble_rw(to_ble_buf, NULL);//发送命令包到ble 
            res=Wait_Response(500);  //等待响应 
            Ble_Response=0;
            if(res==0)
            { 
                if(packet.CMD==RESPONSE_OK)		
                {		
                    ble_ops.status=BLE_PAIRING;
                    return 0;

                }
                else
                {
                    return 1;
                }


            }

        }
        else
        {
            Create_Packet(SET_MODE_CONNECT,0,0);
            ble_rw(to_ble_buf, NULL);//发送命令包到ble 
            res=Wait_Response(500);  //等待响应 
            Ble_Response=0;
            if(res==0)
            {
                if(packet.CMD==RESPONSE_OK)		
                {			
                    ble_ops.status= BLE_RECONNECTING;				
                    return 0;

                }
                else
                {
                    return 1;
                }


            }

        }

        break;
        case STOP_WORK:
            Create_Packet(SET_MODE_STOP,0,0);
            ble_rw(to_ble_buf, NULL);//发送命令包到ble 
            res=Wait_Response(500);  //等待响应 
            Ble_Response=0;
            if(res==0)
            {
                if(packet.CMD==RESPONSE_OK)		
                {			
                    ble_ops.status=  BLE_RECONNECTING;				
                    return 0;

                }
                else
                {
                    return 1;
                }


            }
            
        
        
        break;			 

        case SET_PARA:
            Create_Packet(SET_DEFAULT_HARDVERSION ,0,0);
            ble_rw(to_ble_buf, NULL);//发送命令包到ble 
            res=Wait_Response(500);  //等待响应 
            Ble_Response=0;
            if(res==0)
            {
                if(packet.CMD==RESPONSE_OK)		
                {						
                    return 0;

                }
                else
                {
                    return 1;
                }


            }   
    
        
        
        break;
        
        default: 
        break;

        }
        
        return 1;

}
uint8_t Read_ble(uint8_t *msg,uint8_t len)
{
	
	return 0;
}

//返回非零代表错误  将要写入的数据放入缓冲池中
uint8_t Write_ble(uint8_t opcode, uint8_t * data, uint16_t len)
{
	
    uint32_t i = 0;
    uint8_t buf_index = 0;
    if(ble_ops.status != BLE_CONNECTED)
    {
        //debug_log("err due to unconnect ");
        return 1;  

    }
    if(len > 18)
    {
        debug_log("err due to len ");
        return 1; 
    }

    i = ble_cmd_buffer_input_index - ble_cmd_buffer_output_index;
    if(i > BLE_BUF_NO)
    {   
        debug_log("err due to overspace ");
        ble_cmd_buffer_output_index = ble_cmd_buffer_input_index;
        return 1;
    }
    else
    {
        i = BLE_BUF_NO - i;//buffer left counts
        if((i*18) < len) //no much buffer
        {   
            debug_log("err due to nospace ");
            return 1;
        }
    }


    buf_index = ble_cmd_buffer_input_index%BLE_BUF_NO;
    ble_data_buffer[buf_index].identifier = 0xAA;
    ble_data_buffer[buf_index].opcode = opcode;
    ble_data_buffer[buf_index].length = len;
    if(len)
    {
        memcpy(ble_data_buffer[buf_index].payload, data, len);
    }
    ble_cmd_buffer_input_index ++;

    return 0;
	
	
}

void manager_ble(void)
{
	
    //处理客户端请求
    if(Host_Request_Flag)
    {	
        //debug_log("host request ");
        osDelay(3);//这里延时是为了让51822从发送变成接收需要一段时间
        host_client((uint8_t *)&packet);
        Host_Request_Flag=0;
    }
     
    if(currunt_status!=ble_ops.status)
    {
        ble_ops.status=currunt_status;
        server_status_call_back(currunt_status);  
        
    }
	

    if(	ble_ops.status== BLE_CONNECTED)
    {
        
        send_ble_data_buffer_to_host();  //将缓冲里的数据全部发出去 会阻塞
        
    }


}
//通过蓝牙直接传输纯数据到host
uint8_t ble_direct_write (uint8_t *data ,uint8_t len)
{

 
   Create_Packet(CMD_SYNC_DATA,data, len);
    
   return send_msg_to_ble(to_ble_buf , len);
	
}


void ble_clear_buff(void)
{
		
	ble_cmd_buffer_input_index = 0;
    ble_cmd_buffer_output_index = 0;	
		
}
//ble的响应中断回调
void Ble_Response_callback(void)
{
	  
    uint8_t *packet_buff;
    uint8_t i;
    uint16_t delay=200;
    if(ble_opened_flag)
    {
           
        while(delay--);
        if(IF_HAS_DATA() )
        {
            ble_rw(NULL,(uint8_t *)&from_ble_buf);	//读取ble的数据

            for(i=0;i<TRNASFER_MTU;i++)    //从缓冲里取得一个包
            {
                if(from_ble_buf[i]==0xaa)
                {	
                    packet_buff=&from_ble_buf[i];
                    break;

                }

            }
             if(i>2)
             {
                err_log("position %d",i);
             }
            if(i<TRNASFER_MTU)
            {
                       
                memcpy(&packet,packet_buff,sizeof(PACKET_ST));
                if(packet.CMD>=0x80)   //来自于host
                {

                    Host_Request_Flag=1;	

                }
                else  //来自ble的响应
                {
                 
                    Ble_Response=1;
                    ble_device_event();                    
               

                }

             
            }
            else
            {
                 err_log("error packet from ble");
            }
            
             memset(from_ble_buf,0,TRNASFER_MTU);
                            
        }
     
    }

		
}
uint8_t  check_ble(void)
{
    uint8_t res;
    Open_ble();
    res= Ioctl_ble(INIT,NULL); 
    Close_ble();
    if(res)
    {
        err_log("ble check error\r\n");	 
        return 1;
    }
    else
    {
        return 0;
        
    }
   
       
    
}

uint8_t start_test_ble_net(void)
{
    
    uint8_t res,para;  
    Open_ble(); 
    res= Ioctl_ble(INIT,NULL); 
    if(res)
    {
        debug_log("ble check error");	 
        return res;
    }

    osDelay(50);
    para=1;
    res=Ioctl_ble(START_WORK,&para);
    if(res)
    {
        
        debug_log("ble has error");
        return res;
    }
    return 0;
    
}

void qiut_test_ble_net(void)
{
     Close_ble();
}

//固定每次发送32字节 读写32字节
static void ble_rw(uint8_t * p_tx_data, uint8_t * p_rx_data)
{

    HAL_NVIC_DisableIRQ(EXTI0_IRQn);  //关闭模组中断
	BLE_SELECT() ; 
  
    if(p_rx_data != NULL)
    {
        
        HAL_SPI_Receive(SPI_CONTROLER,p_rx_data, 64, 10);
    }
    else
    {  
        
        HAL_SPI_Transmit(SPI_CONTROLER,p_tx_data, 64, 10);
    }
  
	BLE_RELEASE();
    
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);  //打开模组中断
}

//构成命令包 
static void Create_Packet(uint8_t cmd,uint8_t* pyload,uint8_t len)
{
	
    memset(to_ble_buf,0,TRNASFER_MTU);	
	to_ble_buf[0]=0xaa;
	to_ble_buf[1]=cmd;
	to_ble_buf[2]=len;
	if(len>0)
    {
        memcpy(&to_ble_buf[3], pyload,len);
    }
	
	
}
//阻塞模式等待响应  超时返回错误
uint8_t Wait_Response(uint32_t  time)
{
    uint32_t count=0;

    do{
        osDelay(1); 
        count++;
    }while((count<time)&&(Ble_Response==0));
    if(count>=time)
    {
        return 1;
    }
    else
    {
        return 0;
    }
	
}
//发送到蓝牙 等待蓝牙发送到host成功
uint32_t  send_msg_to_ble(uint8_t *data ,uint8_t len)
{
    uint8_t res;
    static uint8_t error_count=0;
    //debug_log("send ");
    ble_rw(data, NULL);
    res=Wait_Response(1000);  
    Ble_Response=0;
    if(res==0)
    {
        if(packet.CMD==RESPONSE_OK)		
        {	  error_count=0;
            //debug_log("response ok");
            return 0;	
        }
        else if(packet.CMD==RESPONSE_ERROR)
        {
            
            if(packet.message[0]==STATE_ERR) 
            {
                error_count=0;
                debug_log("response err");
                currunt_status= BLE_RECONNECTING;//这里应该考虑是不是应为断开了导致的
                return 0;
            }
            else
            {
                //debug_log("response full");
                
                error_count++;
                if( error_count>30)
                {
                    
                   debug_log("reset ble");
                   Close_ble();  
                   FEED_DOG() ;  
                   osDelay(1000);
                   error_count=0;
                   Open_ble();
                   uint8_t para=0;
                    
                  if( Ioctl_ble(INIT,0))
                  {
                      debug_log("init ble failed");
                      Set_Device_Status(DEVICE_TRYING_POWER_OFF);
                  }
                  else
                  {
                    
                      if(Ioctl_ble(START_WORK,&para))
                      {
                            debug_log("start ble server failed"); 
                            Set_Device_Status(DEVICE_TRYING_POWER_OFF);
                      }
                  }
                    
                   return 0;                   
                }
                                                       
                osDelay(3); //这里延时是为了让51822从发送变成接收需要一段时间
                return 1;
            }
            
        }
        return 1;
    
                
    }
    else
    {
        debug_log("over time ");
        return 1;
    }
	
	
}
//将缓冲里的数据通过ble发送给host
uint8_t send_ble_data_buffer_to_host(void)
{
    uint8_t buf_index = 0;
    do
    {
        

        if(ble_cmd_buffer_input_index > ble_cmd_buffer_output_index) 
        {   
            buf_index = ble_cmd_buffer_output_index%BLE_BUF_NO;
                
            if(0== send_msg_to_ble( (uint8_t *)(&ble_data_buffer[buf_index].identifier), (ble_data_buffer[buf_index].length + 3)))  //发送给蓝牙模块
            {
                ble_cmd_buffer_output_index ++; 
                osDelay(3);                                 
            }
            

             
                            
                    

        }
       
    }
    while(ble_cmd_buffer_input_index > ble_cmd_buffer_output_index);

    return 1;
}


void ble_device_event(void)
{
    if(packet.CMD==RESPONSE_STA)
    {
       // debug_log("response status ");
        currunt_status=packet.message[0];
        Ble_Response=0;
    }
      
    ble_device_ack=1;
   
    
}

void resend_timer_callback(const void *argument)
{
   ble_rw(back_buf, NULL);
    
}

void listen_ble_device_thread(const void* argument)
{
   
   osTimerDef(resend_timer,resend_timer_callback);
   resend_timer_handle=osTimerCreate (osTimer(resend_timer), osTimerOnce,NULL);       
    
    
    
    while(1)
    {
        if(ble_device_ack)
        {
            ble_device_ack=0;
            if(Get_Device_Status()==DEVICE_DFU_MODE) 
            {
                   
                    switch(packet.CMD)
                    {
                        
                        case GET_HW_LEN:               
                        {
                            uint8_t fw_num= NODE_BLE_FW_NUM;           
                            usb_write (USB_CMD_GET_FW_INFO,&fw_num, 1);  
                            debug_log("get fw info ");
                            usb_dfu_step= GET_BLE_FW_INFO;                            
                            osTimerStop ( resend_timer_handle);                            
                            Ble_Response=0;
                        }
                                   
                        break;           
                       case GET_HW_RAW:                                                  
                            usb_write (USB_CMD_GET_RAW_DATA,&packet.message[0], 1);   
                            osTimerStop ( resend_timer_handle);  
                            Ble_Response=0;
                                       
                       break;
                        
                       case GET_HW_CHECK:
                            usb_write (USB_CMD_GET_CHEKSUM,NULL, 0); 
                            Ble_Response=0;
                            usb_dfu_step= GET_BLE_FW_CHECK;
                            osTimerStop ( resend_timer_handle);  
                        
                       break;
                        
                       case REPORT_CHECK_RESULT:             
                             {
                                 uint8_t result=packet.message[0];
                                 usb_write (USB_CMD_REPORT_RESULT,&result, 1);  
                                 osDelay(10);
                                 if(result==0)
                                 {
                               
                                     if( mcu_need_update)
                                     {
                                          usb_dfu_step= GET_MCU_FW_INFO;
                                          uint8_t fw_num=  NODE_MCU_FW_NUM;                          
                                          usb_write (USB_CMD_GET_FW_INFO,&fw_num, 1);  
                                         
                                     }
                                     else
                                     {
                                         
                                         usb_write (USB_CMD_NOTIFY_RESET ,NULL, 0);  
                                         g_device_info.auto_poweron=1;
                                         Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage));
                                         Dispalay_deinit(); 
                                         server_stop();
                                         NVIC_SystemReset(); 
                                                                          
                                     }
                                   
                          
                                 }
                            
                               
                                 Ble_Response=0;   
                                 osTimerStop ( resend_timer_handle);  
                             }
                                                                 
                       break;
                             
                             
                       case REPORT_ERROR:
                           usb_write (USB_CMD_REPORT_ERROR ,&packet.message[0], 1);   
                           Ble_Response=0; 
                           osTimerStop ( resend_timer_handle);  
                           break;
                       
                        
                    }
                }
        }
        osDelay(50);
        
    }

  
}

uint8_t send_fw_packet_to_ble(uint8_t cmd,uint8_t* pyload,uint8_t len)
{
     
    Create_Packet(cmd,pyload,len);
    ble_rw(to_ble_buf, NULL);
  
   if(Get_Device_Status()==DEVICE_DFU_MODE) 
   {
       memcpy(back_buf,to_ble_buf,TRNASFER_MTU);  
       osTimerStart (resend_timer_handle, 100);
       
   }
    
    return 0;
       
}



