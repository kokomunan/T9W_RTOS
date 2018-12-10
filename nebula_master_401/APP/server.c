#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "server.h"
#include "nrf51822.h"
#include "Status_Machin.h"
#include "RGB_LED.h"
#include "norflash.h"
#include "debug_log.h"
#include "device_info.h"
#include "General_Emr.h"
#include "gather_position.h"
#include "note_manager.h"
#include "em9203.h"
#include "usb_controler.h"
#include "display.h"
#include "input_event.h"
#include "page_detect.h"
#include "ble_client.h"
#include "oled.h"
#include "power_manager.h"
#include "ewm3080b.h"

extern BLE518XX_OPS  ble_ops;
extern EM9203_OPS    em9203_ops;
extern EWM3080_OPS  wifi_ops;
extern SERVIVE_OPS   usb_ops;
extern stMainStatus  g_main_status;
extern st_device_data_storage g_device_info;

uint8_t Server_mode=0;
SERVIVE_OPS *server_ops;
//���߷����ʼ�� ����slider ������ͬ�������豸

uint8_t server_init(uint8_t mode)
{
		uint8_t res;
        uint16_t delay_ms  = 0;
		Server_mode=mode;
		if(mode==NRF51822) 
		{

            server_ops=(SERVIVE_OPS *)&ble_ops;
            server_ops->Open(); 
           // osDelay(100); 
            delay_ms = 0;   
           	while(POWER_BUTTON_PRESS)
			{    
                          
                osDelay(5);
                delay_ms ++;
                    
                if(delay_ms==400)   
                {
                    res=server_ops->Ioctl(INIT,0);	                                                       
                    if(res==0)
                    {
                                               
                        OLED_Display_On(); //����oled�豸    
                        redraw_main_sceen();//���ƿ�������
                        ON_SYS_POWER() ;  
                        debug_log("detected ble device");
                    }
                    else
                    {
                        err_log("undetected ble device");
                        NVIC_SystemReset();  

                    }
                }
                else if(delay_ms>600)
                {
                    break;              
                }
            }
            
            
            if(delay_ms>600)
            {
                debug_log("prepare to paring");
                {
                    uint8_t param=1;
                    res=server_ops->Ioctl(START_WORK,&param);	
                    if(res)
                    {
                        err_log("ble has error");
                        NVIC_SystemReset();  
                    }

                }
                
            }
            else if(delay_ms>=400)
            {
                 debug_log("prepare to connecting");
                {
                    uint8_t param=0;
                    res=server_ops->Ioctl(START_WORK,&param);	
                    if(res)
                    {
                        err_log("ble has error");
                        NVIC_SystemReset();  
                    }

                }
            }
            else
            {
                return 1;
            }
            
            
            init_offline_store_data();
            TOUCH_PAD_ON();
            page_det_start();
            return 0;
		}

		else if(mode==WIFI_MODE)
		{  

              server_ops=(SERVIVE_OPS *)&wifi_ops;
              server_ops->Open();  	
      
            
            delay_ms = 0;
            while(POWER_BUTTON_PRESS)
            {    
                //������Ҫι��
                osDelay(5);
                delay_ms ++;
                if(delay_ms == 400) //2s
                {
					
					
     					FEED_DOG() ; 
					    OLED_Display_On(); //����oled�豸    
						redraw_main_sceen();//���ƿ�������
						ON_SYS_POWER() ;
                        res=server_ops->Ioctl(CREATE_INIT_PROCESS,0);	
                        if(res==0)
                        {
							
                                debug_log("detect WIFI device");
                                break;
                        }
                        else
                        {
                                debug_log("undetected WIFI device");
                                NVIC_SystemReset();   

                        }

                }

            }

            if(delay_ms < 400)	
            {			
                //NVIC_SystemReset();   
                if(server_ops!=NULL)
                {
                    server_ops->Close();	
                }
                server_ops=NULL;
                g_main_status.device_state =  DEVICE_POWER_OFF;
                return 1;
            }

            Disbale_note_store();  
            return 0;

		}
        else if(USB_DEVICE==mode)
        {
            server_ops=&usb_ops;
            server_ops->Open();  
            init_offline_store_data();
            TOUCH_PAD_ON();  
            page_det_start();
            return 0;              
        }
        
        else  //�Զ�����
        {
            
            uint8_t server_mode;  
            FEED_DOG() ;           
            debug_log("auto power on");	           
            Display_waite_updata();//��ʾ�ȴ�
            osDelay(1000);  //�ȴ�BLE�������
            FEED_DOG() ; 
            osDelay(1000);      
            FEED_DOG() ; 
            osDelay(1000);      
                             
                       
            server_mode=NRF51822;   //����һ������
 
        
            debug_log("server init at mode%d",server_mode);
      
            server_start(server_mode);
            Display_init()	;
            input_init();  
            Set_Device_Status(DEVICE_OFFLINE);
            debug_log("start menu");
            page_det_start();
            return 0;   
        }

}

void server_thread(void)
{
    if(server_ops!=NULL)
    server_ops->manager();
	
}
//�����п���һ���µķ���
void server_start(uint8_t mode)
{
        uint8_t res;
		Server_mode=mode;
		if(mode==NRF51822) 
		{

				server_ops=(SERVIVE_OPS *)&ble_ops;
				server_ops->Open();  	
                osDelay(1000);                                                      
                res=server_ops->Ioctl(INIT,0);	
                if(res==0)
                {
                        debug_log("detected ble device");
                        debug_log("prepare to connecting");
                        {
                                uint8_t param=0;
                                res=server_ops->Ioctl(START_WORK,&param);
                                if(res)
                                {
                                    err_log("ble has error");
                                    NVIC_SystemReset();  
                                }
                        }

                }
                else 
                {
                        err_log("undetected ble device");
                        NVIC_SystemReset(); 

                }
                page_det_start();
                init_offline_store_data();
                TOUCH_PAD_ON();
                    
		}

		else if(mode==WIFI_MODE)
		{  
                     
            server_ops=(SERVIVE_OPS *)&wifi_ops;
            server_ops->Open();  	
            osDelay(2000);     
                   
            res=server_ops->Ioctl(CREATE_INIT_PROCESS,0);	
            if(res==0)
            {
                  debug_log("detected WIFI device");
          
            }
            else
            {
                debug_log("undetected WIFI device");   
                NVIC_SystemReset();   

            }
            TOUCH_PAD_OFF();
            Disbale_note_store();   
		}
        else if(USB_DEVICE==mode)
        {
            server_ops=&usb_ops;
            server_ops->Open();  
            init_offline_store_data();
            TOUCH_PAD_ON();
            page_det_start();
           
                      
        } 
    
}
//ֹͣһ������
void server_stop(void)
{
    if(server_ops!=NULL)
    {
        server_ops->Close();	
    }
    server_ops=NULL;
    DISABLE_NORTIFY;
//    if(Server_mode!=g_device_info.serve_info.last_serve)
//    {
//         g_device_info.serve_info.second_serve=g_device_info.serve_info.last_serve;
//         g_device_info.serve_info.last_serve=Server_mode;
//         Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage));
//    }
    close_note(1, 0);
    page_det_stop();
    Set_Device_Status(DEVICE_OFFLINE);//�����豸״̬Ϊ����
}

void usb_server_open(void)
{
    uint8_t mode;
    server_stop();
    mode=USB_DEVICE;
    change_screen(WAIT_SCREEN);   
    server_start(mode);
    debug_log("change server :%d",mode);   
    
    
    
}


void usb_server_close(void)
{
    uint8_t mode;
    server_stop();
    mode=SELCET_MODE;
    change_screen(WAIT_SCREEN);   
    server_start(mode);
    debug_log("change server :%d",mode);   
   
   
    
   
}



//��host �˷���֪ͨ��Ϣ  Ϊʲô�ͷ�������ֳ���������
//���ǵ�2.4g��һ�����й����host �ͻ��˷������Էֲ������������
//����ble ��һ����
uint8_t  server_notify_host(uint8_t opcode, uint8_t * data, uint16_t len)
{
    uint8_t res;

    if(server_ops!=NULL)
    {
        res=server_ops->Write(opcode,data,len);	 //��Ҫ���͵����ݷ��绺���
    }
    return	res;
		
	
}
//����������д�뵽���ͻ������
uint8_t server_Send_position_to_host(uint8_t opcode, uint8_t * data, uint16_t len)
{
    uint8_t res;
    if(server_ops!=NULL)
    {
        res=server_ops->Write(opcode,data,len);
    }
    return res;
}
//����û�н��뻺��� ����ֱ�ӷ��͸�ble ���ͳ�ȥ
uint8_t server_send_direct(uint8_t *data ,uint8_t len)
{
    uint8_t res;
    if(server_ops!=NULL)
    {
        res=server_ops->Write_direct (data,len);
    }
    return res;

	
}

//�����豸״̬�Ļص��������˺����ɲ�ͬ�������豸���ã������������Լ���״̬
void server_status_call_back(uint8_t sta)
{
		if(Server_mode==NRF51822)
		{

				if( BLE_CONNECTED==sta)
				{     
						debug_log("ble connected host");
						TOUCH_PAD_ON();
						g_main_status.device_state = DEVICE_ACTIVE;         
						init_position_buffer();
						server_ops->clear_buff();  
						g_device_info.dtm_mode_flag = 1;
						Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage)); 
//                        update_dis_online_page(0,0);                    
                      //  set_key_event(RETURN_KEY);  //�˳���Զ���
                       
						
				}       
				else if(BLE_DFU==sta)
                {
                        
                    
                }
                else if( BLE_PAIRING ==sta)
                {
                      debug_log("paring"); 
                    //  change_screen(PARING_SCREEN);      
                  
                }
                else
				{
				    debug_log("ble disconnected host");
                    if((DEVICE_POWER_OFF != g_main_status.device_state)&& (DEVICE_TRYING_POWER_OFF != g_main_status.device_state))
                    {
                        DISABLE_NORTIFY ;
                        g_main_status.device_state = DEVICE_OFFLINE;
                        set_en_upload(0);                       
                        TOUCH_PAD_ON();
                        flash_wake_up();           
                        init_position_buffer();                           

                    }

				}

		}
        

		else if(Server_mode==WIFI_MODE)
		{
				if(PIPE_DISCONNECT==sta)
				{
						debug_log("wifi  pipe disconnected");
                        g_main_status.device_state = DEVICE_OFFLINE;                   					
                        server_ops->clear_buff(); 
                        TOUCH_PAD_OFF();
                        page_det_stop();
                     
                      
				}
                else if(PIPE_CONNECT==sta)
                {
                        debug_log("wifi  pipe connected");
                    	g_main_status.device_state = DEVICE_ACTIVE;   
                        server_ops->clear_buff(); 
                         TOUCH_PAD_ON();
                        page_det_start();  //����ҳ����  
               
                }

                flash_wake_up();  


		}
        else
        {
            
           if(CONNECT==sta)
           {
                debug_log("usb connect");     
                g_main_status.device_state = DEVICE_ACTIVE;         
                init_position_buffer();
                server_ops->clear_buff();  
                TOUCH_PAD_ON(); 
        
           }
           else
           {
                debug_log("usb disconnect");
                g_main_status.device_state = DEVICE_OFFLINE;   
                TOUCH_PAD_OFF();
                usb_server_close();
      
           }

            
        }
	

}
//��ȡ�����豸��״̬
uint8_t get_server_status(void)
{
	
		if(server_ops!=NULL)
		{
			  return server_ops->status;
		}
		else
		{
			  return INITIALIZATION;
		}


}

uint8_t server_ioctl(uint8_t cmd,uint8_t *data)
{
    
    return server_ops->Ioctl(cmd,data);
}

//��ȡ��ǰ������豸
uint8_t get_server_mode(void)
{
	
	  return Server_mode;
	
}

uint8_t get_server_ready(void)
{
	  return server_ops->ready;
}

void set_server_ready(uint8_t v)
{
	  server_ops->ready=v;
}
