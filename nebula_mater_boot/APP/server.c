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
#define DEBUG_LOG  debug_log
#define ERR_LOG    err_log
extern BLE518XX_OPS  ble_ops;
extern EM9203_OPS    em9203_ops;
extern stMainStatus  g_main_status;
extern st_device_data_storage g_device_info;

uint8_t Server_mode=0;


SERVIVE_OPS *server_ops;
//���߷����ʼ�� ����slider ������ͬ�������豸
void server_init(uint8_t mode)
{
		uint8_t res;
		uint16_t delay_ms  = 0;
		Server_mode=mode;
		if(mode==NRF51822) //Ĭ������
		{

				server_ops=(SERVIVE_OPS *)&ble_ops;
				server_ops->Open();  	//���豸
				//��ⰴ������ʱ�����벻ͬ��ģʽ

				delay_ms = 0;
				while(POWER_BUTTON_PRESS)
				{    
						//������Ҫι��
						HAL_Delay(5);
						delay_ms ++;
						if(delay_ms == 400) //2s
						{   
								GREEN_LED_OFF();					
								res=server_ops->Ioctl(INIT,0);	//��ʼ���豸
								if(res==0)
								{
										BLUE_LED_ON();	
										DEBUG_LOG("detected ble device");

								}
								else
								{
										ERR_LOG("undetected ble device");
										NVIC_SystemReset();   //������������� ����

								}

						}
						if(delay_ms>= 800)//4s delay
						{

						    break;
						}
				}

																			
				if(delay_ms>=800)
				{
						if(POWER_BUTTON_PRESS)
						{
						    g_main_status.btn_hold_flag = 1;
						}

						g_device_info.dtm_mode_flag = 0;

						Update_device_info((uint8_t *)&g_device_info, sizeof(st_device_data_storage));        //����
						DEBUG_LOG("prepare to paring\r\n");		
						{
								uint8_t param=1;
								res=server_ops->Ioctl(START_WORK,&param);	//�����������ģʽ
								if(res)
								{
										NVIC_SystemReset();   //������������� ����
								}
								led_stop();
								LEDStart(UI_PAIRING);
						}
				}
				else if((delay_ms>=400) && (delay_ms< 800))
				{
						DEBUG_LOG("prepare to connecting\r\n");
						{
								uint8_t param=0;
								res=server_ops->Ioctl(START_WORK,&param);	//�������������ģʽ
								if(res)
								{
								    NVIC_SystemReset();   //������������� ����
								}
								LEDStart(UI_RECONNECTING);
						}
				}
				else
				{
				    NVIC_SystemReset();
				}


		}

		else //em9203��ʼ�� 
		{  

		server_ops=(SERVIVE_OPS *)&em9203_ops;
		server_ops->Open();  	
		delay_ms = 0;
		while(POWER_BUTTON_PRESS)
		{    
				//������Ҫι��
				HAL_Delay(5);
				delay_ms ++;
				if(delay_ms == 400) //2s
				{   
						res=server_ops->Ioctl(INIT,0);	
						if(res==0)
						{
								BLUE_LED_ON();	
								//		DEBUG_LOG("detected em9203 device");
								break;
						}
						else
						{
								//		ERR_LOG("undetected em9203 device");
								NVIC_SystemReset();   

						}

				}

		}

		if(delay_ms < 400)	
		{			
		    NVIC_SystemReset();   
		}
		Disbale_note_store();   //��ֹʹ�����ߴ���


		}

}

void server_thread(void)
{
		if(server_ops!=NULL)
		server_ops->manager();
	
}


void server_stop(void)
{
		if(server_ops!=NULL)
		{
		    server_ops->Close();	
		}
		server_ops=NULL;
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
			
		return	server_ops->Write(opcode,data,len);
	
}
//����û�н��뻺��� ����ֱ�ӷ��͸�ble ���ͳ�ȥ
uint8_t server_send_direct(uint8_t *data ,uint8_t len)
{
		if(Server_mode==NRF51822)
		{
		    return ble_direct_write (data,len);
		}

		else//9203
		{
			
		    return 0;
		}
}

//�����豸״̬�Ļص��������˺����ɲ�ͬ�������豸���ã������������Լ���״̬
void server_status_call_back(uint8_t sta)
{
	
		if(Server_mode==NRF51822)
		{

				if( BLE_CONNECTED==sta)
				{     
						DEBUG_LOG("ble connected host");
						LEDStart(UI_CONNECTED);
						//  g_main_status.ble_state= BLE_CONNECTED;  
						TOUCH_PAD_ON();
						g_main_status.device_state = DEVICE_ACTIVE;         

						//g_main_status.flash_store_data_flag = 0;
						init_position_buffer();
						server_ops->clear_buff();  //������������Ļ���
						g_device_info.dtm_mode_flag = 1;
						Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage));
						
				}       
				else
				{
				    DEBUG_LOG("ble disconnected host");
						if((DEVICE_POWER_OFF != g_main_status.device_state)&& (DEVICE_TRYING_POWER_OFF != g_main_status.device_state))
						{

								g_main_status.device_state = DEVICE_OFFLINE;
								set_en_upload(0);  //��ֹ�ʼ��ϴ�                       
								LEDStart(UI_RECONNECTING);
								flash_wake_up();           
								init_position_buffer();            

						}


				}

		}

		else//9203
		{
				if(NEBULA_STATUS_OFFLINE==sta)
				{
						DEBUG_LOG("node disconnected");
						LEDStart(UI_RECONNECTING);
						TOUCH_PAD_OFF();
				}
				else
				{

						if(NEBULA_STATUS_VOTE==sta)
						{
							  DEBUG_LOG("node vote mode");
							
						}
						else if(NEBULA_STATUS_MASSDATA==sta)
						{
							
								DEBUG_LOG("node mass mode");
							
						}
						else
						{
							
							  DEBUG_LOG("node idle mode");
							
							
						}



						LEDStart(UI_CONNECTED);
						//  g_main_status.ble_state= BLE_CONNECTED;  
						TOUCH_PAD_ON();
						g_main_status.device_state = DEVICE_ACTIVE;         

						//g_main_status.flash_store_data_flag = 0;
						init_position_buffer();
						server_ops->clear_buff();  //������������Ļ���
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

//��ȡ��ǰ�����ģʽ
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
