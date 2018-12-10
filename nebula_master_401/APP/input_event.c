#include "input_event.h"
#include "mmi.h"
#include "config.h"
#include "RGB_LED.h"
#include "display.h"
#include "note_manager.h"
#include "debug_log.h"
#include "General_Emr.h"
#include "Soft_Timer.h"
#include "server.h"
#include "ewm3080b.h"
#include "usb_controler.h"
#include "device_info.h"

extern void Set_Device_Status(uint8_t new_sta);
extern uint8_t Get_Device_Status(void);
extern st_device_data_storage g_device_info;
extern uint8_t nebula_keys[MAX_VOTE_NUM];
extern uint8_t  new_note_flag;
uint32_t currunt_note_id=0;          //当前笔记的所属ID
uint32_t display_currunt_note_index=0; //当前应该显示的页

uint8_t press_count=0;
void set_currunt_note_id(uint32_t ID)
{	
    currunt_note_id=ID ;
}
void Add_page_index(void)
{
    display_currunt_note_index++;
	
}
void init_display_currunt_note_index(uint32_t index)
{
		display_currunt_note_index=index;
}


void button_det_long_press(void)
{
    
     Set_Device_Status(DEVICE_TRYING_POWER_OFF);
    debug_log("try to close");  
    
    
}


void button_det_short_release(void)
{
	
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();    
    device_sta=Get_Device_Status();
    
    
    if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(11);


         }               
        
        
    }
    else if(screen_num==MAIN_SCREEN)
    {
       
        if(DEVICE_ACTIVE ==device_sta)
        {
            if((server_mode==NRF51822)||(server_mode==USB_DEVICE)) 
            {
                uint8_t btn = BUTTON_EVENT_CREATE_PAGE;
                server_notify_host(CMD_BTN_EVENT, &btn, 1);  

            }
 			
        }
    }
    
    

}
void button_det_double_press(void)
{

		
}


void button1_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
       if(DEVICE_ACTIVE == device_sta)
       {
           if(server_mode==WIFI_MODE) 
           {
               
               
           }          
           else if(server_mode==NRF51822)
           {
               debug_log("key1");
           }
           
       }
            
        
    }
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
                       
             update_oled_test_key(1);//显示 1


         }               
        
        
    }
    
    
    
}
void button2_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
       if(DEVICE_ACTIVE == device_sta)
       {
           if(server_mode==WIFI_MODE) 
           {
           
               
           }
           else if(server_mode==NRF51822)
           {
               debug_log("key2");
           }
           
       }
            
        
    }
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(2);//显示 1


         }               
        
        
    }
}
void button3_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
       if(DEVICE_ACTIVE == device_sta)
       {
           if(server_mode==WIFI_MODE) 
           {
   
               
           }
           else if(server_mode==NRF51822)
           {
               debug_log("key3");
           }
           
       }
            
        
    }
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(3);


         }               
        
        
    }    
}
void button4_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
       if(DEVICE_ACTIVE == device_sta)
       {
           if(server_mode==WIFI_MODE) 
           {
  
               
           }
           else if(server_mode==NRF51822)
           {
               debug_log("key4");
           }
           
       }
            
        
    }
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(4);


         }               
        
        
    }     
}
void button5_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
       if(DEVICE_ACTIVE == device_sta)
       {
           if(server_mode==WIFI_MODE) 
           {

               
           }
           else if(server_mode==NRF51822)
           {
               debug_log("key5");
           }
           
       }
            
        
    }
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(5);


         }               
        
        
    }    
}
void button6_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
       if(DEVICE_ACTIVE == device_sta)
       {
           if(server_mode==WIFI_MODE) 
           {
  
               
           }
           else if(server_mode==NRF51822)
           {
               debug_log("key6");
           }
           
       }
            
        
    }
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(6);


         }               
        
        
    }     
}
void button7_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
       if(DEVICE_ACTIVE == device_sta)
       {
           if(server_mode==WIFI_MODE) 
           {

           
            }
       }
            
        
    } 
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(7);


         }               
        
        
    }    
}
void button8_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
       if(DEVICE_ACTIVE == device_sta)
       {
           if(server_mode==WIFI_MODE) 
           {
          
               
           }
           else 
           {
               uint8_t btn =  BUTTON_EVENT_DOWN_PAGE;
               server_notify_host(CMD_BTN_EVENT, &btn, 1);  
               debug_log("key8");
           }
           
       }
                    
        
    }
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(8);


         }               
        
        
    }    
}
void button9_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
       if(DEVICE_ACTIVE == device_sta)
       {
        
            if(server_mode==WIFI_MODE) 
            {
                                                      
                                    
            
            }
        
       }
                     
    }
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(9);

         }               
        
        
    }    
}
void button10_short_press(void)
{
    uint8_t screen_num;
    uint8_t device_sta; 
    uint8_t server_mode=get_server_mode();
    screen_num=get_currunt_screen();
    device_sta=Get_Device_Status();
    if(MAIN_SCREEN==screen_num)
    {
                       
        if(DEVICE_ACTIVE ==device_sta)
        {
            if(server_mode==WIFI_MODE) 
            {
                               
          
                           
              
            }
                      
        }
               
    }
    else if(TEST_NRF_SCREEN==screen_num)
    {
       if(server_mode==USB_DEVICE)          
         {
             
             update_oled_test_key(10);


         }               
        
        
    }     
}

void input_init(void)
{
    osThreadDef(input_thread, input_thread, osPriorityNormal, 0, 256);
   if(osThreadCreate(osThread(input_thread), NULL)==NULL)
   {
       err_log("create input thread error");
       
   }
    
    osThreadDef(select_mode_thread,select_mode_thread,osPriorityNormal,0,256);
    if(osThreadCreate(osThread(select_mode_thread), NULL)==NULL)
    {
        err_log("create select mode thread error");
        
    }
    
     
}

void input_thread(const void *argumen)
{
    uint8_t mode;
    uint8_t sta;
    uint8_t screen_num;
    
  while(1)
    {
            
      
        ButtonProcessLoop() ;           
        osDelay(10);
   }
	
}

void select_mode_thread(const void *argumen)
{
    uint8_t mode;
    uint8_t sta;
    uint8_t screen_num;
    while(1)
    {
            mode=get_server_mode();        
            if(mode!=USB_DEVICE)  
            {
                mode=SELCET_MODE;
                if(mode!=get_server_mode())
                {            
                    sta=Get_Device_Status();
                    screen_num=get_currunt_screen();
                    if((sta==DEVICE_OFFLINE)||(sta==DEVICE_ACTIVE))
                    {
                        if(screen_num==MAIN_SCREEN)
                        {  
                              
                                server_stop();//关闭当前的服务     
                            
                      
                                change_screen(WAIT_SCREEN);  
                                server_start(mode);
                                
                                debug_log("change server :%d",mode);   
                              
                         }
                       
                        
                     }


                }
                
            }
            osDelay(500);
      }
       
}


