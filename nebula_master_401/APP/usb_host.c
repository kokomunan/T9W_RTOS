#include "usb_host.h"
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
#include "dfu.h"
#include "misc.h"
#include "RGB_LED.h"
#include "battery.h"
#include "display.h"
#define DEBUG_LOG  debug_log





void usb_host_thread(void)
{
     stUSB_PACKET cmd_buff;
     uint8_t device_sta=Get_Device_Status();
   
     if(if_host_rcv_flag())  
     {
         if(device_sta> DEVICE_INIT_BTN)
         {
      
            memcpy((uint8_t *)&cmd_buff, (uint8_t *)&usb_host_rx_buff, sizeof(stUSB_PACKET));  
            switch(cmd_buff.opcode)
            {
                

                                      
            }
                       
        
        }
        else //Î´¿ª»ú×´Ì¬ÏÂ
        {
           clear_host_rvc_flag();    
        }
        
       
    }
    resend_poll();
    
    
}

