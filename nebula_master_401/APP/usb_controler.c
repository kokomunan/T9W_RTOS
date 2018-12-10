
#include "usb_controler.h"
#include "config.h"
#include "string.h"
#include "debug_log.h"
#include "Status_Machin.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"
#include "device_info.h"
#include "server.h"
#include "usb_client.h"
#include "soft_timer.h"
#include "nrf51822.h"
#include "cmsis_os.h"



extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t Get_Device_Status(void);
extern st_device_data_storage g_device_info;


uint8_t usb_client_rx_buff[64];
uint8_t usb_cmd_tx_buff[64];

uint8_t device_status=0;
uint8_t device_old_status=0;
uint8_t usb_hotplug_flag=0;
uint8_t usb_hptplug_event=0;
uint8_t usb_client_rcv_flag =0;
uint8_t usb_mode_request_flag=0;
uint8_t usb_mode_request_mode=0;
uint8_t usb_en=0;



osThreadId usb_controler_Task_Handle;



SERVIVE_OPS   usb_ops={
    0,
    0,
    usb_open,
    usb_close,
    usb_ioctl,
    usb_read,  
    usb_write,
    usb_manager,	
    usb_clear_buff,
    usb_write_dirct,
    
       
};


void usb_open(void)
{
    usb_ops.status=device_status;
    usb_ops.clear_buff();
    usb_hotplug_flag=1;  //触发一次热插拔事件
    
//    
//    usb_sta_timer=register_soft_timer() ;  
//    start(usb_sta_timer,200);
    usb_en=1;
    
}

void usb_close(void)
{
    usb_en=0;
//    disregister_soft_timer(usb_sta_timer);
//    usb_sta_timer=NULL;
    
    
}

uint8_t usb_ioctl(uint8_t cmd,uint8_t *param)
{
  
    
    return  0;
    
       
}


 uint8_t usb_read(uint8_t *buff, uint8_t len )
{
    return  0;
    
}

uint8_t usb_write (uint8_t opcode, uint8_t * data, uint16_t len)
{
     uint8_t res=1;
    uint8_t usb_opcode;
    if(opcode>=0x80)
    {
        if(opcode==CMD_POS_DATA)  
        {
            usb_opcode=USB_CMD_REPORT_POSITION;
            create_usb_packet(usb_opcode, len,data);  
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
        }
        else if(opcode==CMD_STATUS)
        {
            
            usb_opcode=USB_CMD_GET_STATUS ;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
        
        }
        else if(opcode==CMD_BTN_EVENT)
        {
            usb_opcode=USB_CMD_BTN_EVENT;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
        }
        else if(opcode==CMD_SYNC_IS_END)
        {
            usb_opcode=USB_CMD_SYNC_IS_END;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
        }
        else if(opcode==CMD_REPORT_PAGE_AUTO)
        {
            usb_opcode= USB_CMD_REPORT_PAGE ;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
            
        }
        else if(opcode==CMD_OTA_EMR_FILE_INFO)
        {
            
            usb_opcode= USB_CMD_OTA_EMR_FILE_INFO ;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
            
            
        }
         else if(opcode==CMD_OTA_EMR_RAW_DATA)
        {
            
            usb_opcode= USB_CMD_OTA_EMR_RAW_DATA ;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
            
            
        }
        else if(opcode==CMD_OTA_EMR_CHECKSUM)
        {
            
            usb_opcode= USB_CMD_OTA_EMR_CHECKSUM ;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
            
            
        }
        else if(opcode==CMD_OTA_EMR_RESULT)
        {
            
            usb_opcode= USB_CMD_OTA_EMR_RESULT ;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
            
            
        } 
        else if(opcode== CMD_ENTER_FRQ_ADJUST)
        {
            
            usb_opcode= USB_CMD_ENTER_FRQ_ADJUST;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
                   
        }
        else if(opcode==CMD_QUIT_FRQ_ADJUST)
        {
            
            usb_opcode= USB_CMD_QUIT_FRQ_ADJUST;
            create_usb_packet(usb_opcode, len,data);     
            res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
            if(res)
            {
                debug_log("usb send error");
            }                
            return res;
            
            
        }            
        else 
        {
            return 0;
        }
    }
    else
    {
        create_usb_packet(opcode, len,data);     
        res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
        if(res)
        {
            debug_log("usb send error");
        }                
        return res;
    }
       
    
}
uint8_t usb_write_dirct(uint8_t *data ,uint8_t len)
{
    uint8_t res;
    if(len>64)
    {
        return 1;
    }
    
     create_usb_packet(USB_CMD_SYNC_DATA, len,data);       
    res=USBD_SendReport(&hUsbDeviceFS,usb_cmd_tx_buff,64,10);                
    if(res)
    {
        debug_log("usb send error");
    }                
    return res;
    
}



void usb_cmd_handle(void)
{
		     
    if(if_client_rcv_flag()) 
    {	
        clear_client_rvc_flag()  ;  
        usb_client(usb_client_rx_buff);   
        
    }
    
   
}
void usb_manager(void)
{
    if(usb_en)
    {
        usb_cmd_handle();
        usb_check_status();
    
    }
   
}
void usb_clear_buff(void)
{
   clear_client_rvc_flag();
   
}

void usb_check_status(void)
{
   if(usb_hotplug_flag)
   {
        usb_ops.status= device_status;
        server_status_call_back(usb_ops.status);
        usb_hotplug_flag=0;
             
   }  
    
    
}


void create_usb_packet(uint8_t opcode,uint8_t len,uint8_t *payload)
{
    stUSB_PACKET *tx_packet;
    tx_packet=( stUSB_PACKET *)&usb_cmd_tx_buff;
    tx_packet->identifier=0xaa;
    tx_packet->device_id=HW_VERSION;
    tx_packet->opcode=opcode;
    tx_packet->length=len;
    if(0<len)
    {   
        memcpy(tx_packet->payload,payload,len);
        
        
    }
  
    
}


uint8_t Is_usb_hotplug(void)
{
    return  usb_hptplug_event;
}
void clear_usb_hosplug(void)
{ 
    usb_hptplug_event=0;
}
uint8_t Is_usb_connect(void)
{
    return (device_status==CONNECT);
    
}

void usb_controler_init(void)
{
    
    device_status=0;
    device_old_status=0;
    usb_hotplug_flag=0;
    usb_hptplug_event=0;
    usb_client_rcv_flag =0;
       
     osThreadDef( usb_controler_Task , usb_controler_Task, osPriorityNormal, 0, 256);
     usb_controler_Task_Handle=osThreadCreate(osThread(usb_controler_Task), NULL);
    
}
void usb_main_task(void const * argument)  
{
    usb_controler_init();
    
    while(1)
    {
   
        if( hUsbDeviceFS.dev_state!=USBD_STATE_CONFIGURED)
        {
            device_status=DISCONNECT;
         
        }
        else    
        {
            device_status=CONNECT;
           
        }
        if(device_status!=device_old_status)
        {
            device_old_status=device_status;
            usb_hotplug_flag=1;    
            usb_hptplug_event=1;
            
            if(device_status==CONNECT)
            {
                debug_log("usb plug");
            }
            else
            {
               debug_log("usb unplug");  
            } 
            
        }        
        osDelay(10);
    }
    
}



void usb_interface_manager(void)
{
      
    if(usb_mode_request_flag)
    {
        usb_mode_request_flag=0;
         uint8_t mode=get_server_mode();  
        if(usb_mode_request_mode)
        {
           
           if(mode!=USB_DEVICE)
           {
               usb_server_open();
            
            
           }
                                                 
        }
        else
        {
            
           if(mode==USB_DEVICE)
           {
             usb_server_close();
            
            
           }
            
            
        }
       
        
    }
    
    
    
}
void usb_controler_Task(void const * argument)
{
        
    while(1)
    { 
        
        uint8_t mode=get_server_mode();
                   
        if( (Get_Device_Status()>DEVICE_INIT_BTN)&&(device_status==CONNECT)&&(Get_Device_Status()!=DEVICE_DFU_MODE)&&(Get_Device_Status()!=SENSOR_UPDATE))
        {     
            //debug_log("period report");         
            usb_write (USB_CMD_REPORT_MODE,&mode,1);
        }
        usb_interface_manager();
        
        osDelay(500);
    }
    
}




