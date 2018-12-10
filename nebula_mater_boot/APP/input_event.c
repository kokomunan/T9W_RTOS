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
#include "em9203.h"
#define DEBUG_LOG   debug_log
extern void Set_Device_Status(uint8_t new_sta);
extern uint8_t Get_Device_Status(void);
extern st_device_data_storage g_device_info;

uint32_t currunt_note_id=0;          //当前笔记的所属ID
uint32_t display_currunt_note_index=0; //当前应该显示的页
static uint8_t currunt_server_mode=0;
SOFT_TIMER_ST     *input_timer;

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

void power_button_long_press(void)
{
	
	  Set_Device_Status(DEVICE_TRYING_POWER_OFF);
	
}

void power_button_short_release(void)
{
	
		uint8_t device_sta;
		uint8_t test_vote=0x31;
		device_sta=Get_Device_Status();
		if(DEVICE_FINISHED_PRODUCT_TEST==device_sta)
		{		

		    update_oled_test_mode(1);

		} 

		else if(DEVICE_ACTIVE==device_sta)
		{

				if(currunt_server_mode==EM9203) 
				{

						if(get_server_status()==NEBULA_STATUS_VOTE )
						{
								server_notify_host(VOTE_OPS, &test_vote, 1);
								DEBUG_LOG ("send vote data %d ",test_vote);

						}
				}


		}
	
	
}
void power_button_double_press(void)
{
		uint8_t device_sta;
		device_sta=Get_Device_Status();

		if(DEVICE_OFFLINE ==device_sta)  //如果是离线模式 并且flash可用
		{
				if(currunt_server_mode==NRF51822) 
				{

						if( g_device_info.stored_total_note_num<MAX_NOTE_ID) 

						{	
								close_note(1, 1);  //关闭上个笔记并且提示翻页显示led		  
								currunt_note_id= create_note_id(g_device_info.note_index);//根据全局的当前最新的id生成新的笔记id
								create_new_note_head(currunt_note_id); //这样的坏处是可能没有笔记也浪费了一个扇区
								g_device_info.note_index=currunt_note_id;   //更新最新的id存储当前的id  等待关闭笔记时写入
								g_device_info.stored_total_note_num++;      //

								display_currunt_note_index= g_device_info.stored_total_note_num; //更新显示索引


								update_dis_offline_page(g_device_info.stored_total_note_num,g_device_info.stored_total_note_num);

								DEBUG_LOG("id[%d] index[%d]/total[%d]\n",currunt_note_id,display_currunt_note_index,g_device_info.stored_total_note_num);
						}

						else
						{
						    DEBUG_LOG("can't create notes\n");
						}

				}

		}

		else if(DEVICE_ACTIVE ==device_sta)
		{
				if(currunt_server_mode==NRF51822) 
				{
						uint8_t btn = BUTTON_EVENT_CREATE_PAGE;
						server_notify_host(BLE_CMD_BTN_EVENT, &btn, 1);  

				}			
		}
		

}
void up_page_button_short_press(void)
{
		static uint8_t count=0;
		uint8_t device_sta;
		device_sta=Get_Device_Status();
		if(DEVICE_OFFLINE ==  device_sta)  //如果是离线模式 
		{
				if(currunt_server_mode==NRF51822) 
				{

						if(g_device_info.stored_total_note_num<1)
						{
						    return ;
						}

						if(Is_Open_note()) 	//已经存在创建的笔记要关闭笔记
						{

						    close_note(1, 0);

						}
						if(display_currunt_note_index>=g_device_info.stored_total_note_num)//看看是否超过最大限
						{ 

								display_currunt_note_index=1;//改变显示的笔记索引
								currunt_note_id= get_before_note_id(currunt_note_id,g_device_info.stored_total_note_num-1) ;	//改变笔记的id	

						}
						else
						{
								display_currunt_note_index++;
								currunt_note_id++;

						}

						update_dis_offline_page(display_currunt_note_index,g_device_info.stored_total_note_num);

						DEBUG_LOG("id[%d] index[%d]/total[%d]\n",currunt_note_id,display_currunt_note_index,g_device_info.stored_total_note_num);
				}
		}

		else if(DEVICE_ACTIVE == device_sta)
		{
				if(currunt_server_mode==NRF51822) 
				{
						uint8_t btn =  BUTTON_EVENT_UP_PAGE;
						server_notify_host(BLE_CMD_BTN_EVENT, &btn, 1);  
				}			
		}
		else if( device_sta == DEVICE_FINISHED_PRODUCT_TEST)
		{		

				if(count==0)
				{			

						update_oled_test_mode(2);
						count=1;	
				}
				else
				{

						update_oled_test_mode(3);
						count=0;	
				}

		}
	
	
}
void down_page_button_short_press(void)
{
		uint8_t device_sta;
		device_sta=Get_Device_Status();
		if(DEVICE_OFFLINE == device_sta)  //如果是离线模式 并且flash可用
		{
				if(currunt_server_mode==NRF51822) 
				{
						if(g_device_info.stored_total_note_num<1)
						{
						    return ;
						}
						if( Is_Open_note()) 	//已经存在创建的笔记要关闭笔记
						{

						    close_note(1, 0);

						}
						if(display_currunt_note_index<=1)//看看是否超过最大限
						{ 

								display_currunt_note_index=g_device_info.stored_total_note_num;//改变显示的笔记索引
								currunt_note_id= get_next_note_id(currunt_note_id,g_device_info.stored_total_note_num-1) ;	//改变笔记的id	

								//得到响应的id
						}
						else
						{
								display_currunt_note_index--;
								currunt_note_id--;

						}
						update_dis_offline_page(display_currunt_note_index,g_device_info.stored_total_note_num);
						DEBUG_LOG("id[%d] index[%d]/total[%d]\n",currunt_note_id,display_currunt_note_index,g_device_info.stored_total_note_num);
				}
		} 	 
		else if(DEVICE_ACTIVE == device_sta)
		{
				if(currunt_server_mode==NRF51822) 
				{
						uint8_t btn =  BUTTON_EVENT_DOWN_PAGE;
						server_notify_host(BLE_CMD_BTN_EVENT, &btn, 1);  
				}		
		}
		else if(device_sta == DEVICE_FINISHED_PRODUCT_TEST)
		{		

		    update_oled_test_mode(4);

		}
	
	
}


void input_init(void)
{
		currunt_server_mode=get_server_mode();  //获取当前的模式
		input_timer=register_soft_timer();
		start(input_timer,10); //10毫秒
}

void input_thread(void)
{
	
		if(IS_Elapsed(input_timer))
		{
				ButtonProcessLoop() ;
					
				start(input_timer,10); //10毫秒
			
		}
	
}


