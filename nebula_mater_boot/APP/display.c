#include "display.h"
#include "oled.h"

#include "oled_icon.h"
#include "config.h"
#include "stdio.h"
#include "debug_log.h"
#include "Soft_Timer.h"
#include "server.h"



extern uint8_t OLED_GRAM[128][4];
extern uint8_t Get_Device_Status(void);
extern uint32_t get_free_sector(void);

#define SETBIT(BYTE,BIT)         ( (BYTE) |= (uint8_t)((uint8_t)1<<(uint8_t)(BIT)) )
#define CLRBIT(BYTE,BIT)         ( (BYTE) &= (uint8_t)((uint8_t)((uint8_t)1<<(uint8_t)(BIT))^(uint8_t)0xff) )


SOFT_TIMER_ST  *display_timer;
uint8_t* battery_icon[7]={BATEERY_LEVEL_1_ICON,BATEERY_LEVEL_2_ICON,BATEERY_LEVEL_3_ICON,BATEERY_LEVEL_4_ICON,BATEERY_LEVEL_5_ICON,BATEERY_LEVEL_6_ICON,BATEERY_LEVEL_7_ICON};


uint8_t need_update_screen=0;
uint8_t online_display_cur_page=0;
uint8_t online_display_tal_page=0;
uint8_t offline_note_cur_page=0;
uint8_t offline_note_tal_page=0;
uint8_t write_icon_update_flag=0;   //已经显示了写入图标
uint8_t battery_value=0;
uint8_t oled_test_mode=0;
static uint8_t currunt_server_mode=0;


void update_dis_event(uint8_t event)
{
	
    SETBIT(need_update_screen,event);
	
}
uint8_t check_event(uint8_t event)
{
	  return (need_update_screen&(1<<event))?1:0;
}
void clear_dis_event(uint8_t event)
{
	
	  CLRBIT(need_update_screen,event) ;
	
}

//刷新测试图画
void update_oled_test_mode(uint8_t mode)
{
	  oled_test_mode=mode;
	
}
//刷新电量
void update_dis_battery_value(uint8_t level)
{
		battery_value=level;
		update_dis_event(NEED_UPDATE_BATERRY);
}
//刷新离线笔记信息
void update_dis_offline_page(uint8_t cur,uint8_t total)
{
		offline_note_cur_page=cur;
		offline_note_tal_page=total;
		update_dis_event(NEED_UPDATE_NOTE_INFOR_ICON);
}
//刷新在线笔记信息
void update_dis_online_page(uint8_t cur,uint8_t total)
{
		online_display_cur_page=cur;
		online_display_tal_page=total;
		update_dis_event(NEED_UPDATE_NOTE_INFOR_ICON);
}
//刷新笔图标
void update_dis_Pen_icon(void)
{
    update_dis_event(NEED_UPDATE_WRITING_ICON);
	
}
void update_dis_all(void)
{
	  update_dis_event(NEED_UPDATE_ALL);
}

//要根据当前的服务模式显示不同的开机界面
void  Display_init(void)
{
		currunt_server_mode=get_server_mode();  //获取当前的模式
		OLED_On_Init();
		if(currunt_server_mode==NRF51822)	//根据模式绘制不同的网络设备图标	
		{
				OLED_Draw_BMP(BLE_ICON,2,12);	//绘制蓝牙图标	
		}
		else
		{
				OLED_Draw_BMP(PRAV_ICON,2,12);//绘制私有协议图标
		}
		OLED_Refresh_bank(0) ; //更新上半部			
		DRAW_2_Area((uint8_t *)&LOGO,19,90)  ; //应该根据模式绘制不同的logo
		OLED_Display_On(); //点亮oled设备
		display_timer=register_soft_timer();
		start(display_timer,300); //300毫秒
}

void Dispalay_deinit(void)
{
	
	  OLED_Off_Init();
}


void Display_Pen(uint8_t en)
{
		uint8_t zero[28];
		if(en)
		{
			  DRAW_1_Area(PEN_ICON,26,14); 
		}
		else
		{
			  memset(zero,0,28);
			  DRAW_1_Area(zero,26,14); 
			
		}
	
}
void Display_signal(uint8_t en)
{
		uint8_t zero[16];
		if(en)
		{
			  DRAW_1_Area(SIGNAL_ICON,13,8); 
		}
		else
		{
			  memset(zero,0,16);
			  DRAW_1_Area(zero,13,8); 
			
		}
	
}

void Display_update_baterry(uint8_t baterry_level)
{
		if(baterry_level>7)
		{
			  baterry_level=7;
		}
		DRAW_1_Area(battery_icon[baterry_level-1],127-18,18)	; //显示电池//根据电池的电量显示绘制响应的图片
	
}
	
void Display_update_storage_space(uint32_t left_secoters)
{
		uint8_t percent;
		float use=1024-left_secoters;
		float tatol=1024;	
		percent=(uint8_t)((use/tatol)*100)/2;
		OLED_Fill(12,6,12+percent,9) ;  // 设置进度条长度
	
}
void Display_update_note_page(uint32_t currunt_page,uint32_t total_pages)
{
		uint8_t temp[7];	
		uint8_t len;
		sprintf((char*)temp,"%d/%d",currunt_page,total_pages);
		len=strlen((char*)temp);
		OLED_ShowString(128-(len*8),0,temp,16);
	
}

void Display_Test(uint8_t mode)
{
		uint8_t content=0;
		switch(mode)
		{
				case OLED_ON:
					 content=0xff;

				break;

				case OLED_OFF:
					 content=0;
				break;

				case OLED_HALF_TOP:
					 content=0x0f;

				break;

				case OLED_HALF_BOTTOM:
					
				 content=0xf0;
				break;

				default:  content=0;
		}


		for(uint16_t i=0;i<128;i++)
		{
				for(uint16_t j=0;j<4;j++)
				OLED_GRAM[i][j]=content;
		}

		SPI_Write_Command(SET_MEM_ADDR_MODE_CMD);  //设置地址模式
		SPI_Write_Command(VERTICAL_ADDR_MODE_CMD);  //纵向
		SPI_Write_Command(SET_COLUMN_ADDR_CMD); //设置纵向起始和终止地址
		SPI_Write_Command(2);  //起始地址
		SPI_Write_Command(129);  //结束地址
		SPI_Write_Command(SET_PAGE_ADDR_CMD); //设置页地址
		SPI_Write_Command(0); //起始                        
		SPI_Write_Command(7); //结束
		SPI_Write_Data(&OLED_GRAM[0][0],512);   //一共是1K字节ram 需要写两边
		SPI_Write_Data(&OLED_GRAM[0][0],512);  
}


//显示处理层   根据状态和事件驱动  z这部分代码写的太垃圾 以后重写
void Dsiplay_Thread(void)
{
		uint8_t device_sta;
		if(IS_Elapsed(display_timer))
		{
			
		    device_sta=Get_Device_Status();	
		    if(device_sta==DEVICE_OFFLINE)
				{
				
						if(check_event(NEED_UPDATE_ALL))	 
						{
							  clear_dis_event(NEED_UPDATE_ALL);
						    Display_signal(0); 		 
						 
						}
						if(check_event(NEED_UPDATE_BATERRY))
						{		 	 
								clear_dis_event(NEED_UPDATE_BATERRY);
								Display_update_baterry( battery_value);		 
						 
						}


						if(check_event(NEED_UPDATE_NOTE_INFOR_ICON))
						{		

								clear_dis_event(NEED_UPDATE_NOTE_INFOR_ICON); 
								OLED_CLEAR_BUFF(); //清除缓存
								if(currunt_server_mode==NRF51822)  //只有ble时绘制剩余存储容量
								{		
										OLED_Draw_BMP(SDCARD_ICON,0,9);	  //绘制sd卡
										OLED_DrawRectangle(12, 3, 62, 12);	 //绘制矩形框
										Display_update_storage_space(get_free_sector());  //绘制进度条		 
								}
								Display_update_note_page(offline_note_cur_page,offline_note_tal_page);
								OLED_Refresh_bank(2) ;			 

						}
					
						if(check_event(NEED_UPDATE_WRITING_ICON))
						{
						    clear_dis_event(NEED_UPDATE_WRITING_ICON);
								if(write_icon_update_flag==0)
								{
										write_icon_update_flag=1;
										Display_Pen(write_icon_update_flag);
								}   

						}
						else 
						{
								if(write_icon_update_flag)
								{
										write_icon_update_flag=0;
										Display_Pen(write_icon_update_flag);
								}

						}

				    OLED_Display_On();
			 
        }
				else if(device_sta==DEVICE_ACTIVE)
				{

						if(check_event(NEED_UPDATE_ALL))
						{			 	 
						    clear_dis_event(NEED_UPDATE_ALL);	
								Display_signal(1);		
								OLED_CLEAR_BUFF(); //清除缓存		 
								OLED_Draw_BMP(CONNECTED_ICON,56,16);	  //载入连接时的icon 
								OLED_Refresh_bank(2) ;			  			 
								 //OLED_DEBUG("update liquid to connect"); 
						 
						}
						if(check_event(NEED_UPDATE_BATERRY))
						{
								clear_dis_event(NEED_UPDATE_BATERRY);
								Display_update_baterry( battery_value);	
								//OLED_DEBUG("update baterry icon");  
						}				 
						if(check_event(NEED_UPDATE_NOTE_INFOR_ICON))
						{		
								clear_dis_event(NEED_UPDATE_NOTE_INFOR_ICON);	 
								OLED_CLEAR_BUFF(); //清除缓存		 
								OLED_Draw_BMP(CONNECTED_ICON,56,16);	  //载入连接时的icon 
								Display_update_note_page(online_display_cur_page,online_display_tal_page);			 
								OLED_Refresh_bank(2) ;			 
								//OLED_DEBUG("update note [%d]/[%d]",display_currunt_note_index,g_device_info.stored_total_note_num); 
						 
						}

						if(check_event(NEED_UPDATE_WRITING_ICON))
						{
								clear_dis_event(NEED_UPDATE_WRITING_ICON);	
								if(write_icon_update_flag==0)
								{
										write_icon_update_flag=1;
										Display_Pen(write_icon_update_flag);
										//OLED_DEBUG("update writing icon");  
								}   

						}
						else 
						{
								if(write_icon_update_flag)
								{
										write_icon_update_flag=0;
										Display_Pen(write_icon_update_flag);
										//OLED_DEBUG("clear writing icon");   
								}
							
						}


				}
			
				else if(device_sta==DEVICE_OTA_MODE)
				{

						if(check_event(NEED_UPDATE_ALL))
						{			 	 
								clear_dis_event(NEED_UPDATE_ALL);	 
								//清除笔标志
								if(write_icon_update_flag)
								{
										write_icon_update_flag=0;
										Display_Pen(write_icon_update_flag);			 
										//OLED_DEBUG("clear writing icon");   
								}

								OLED_CLEAR_BUFF(); //清除缓存		 
								OLED_Draw_BMP(DOWNLOAD_ICON,56,16);	  //载入连接时的icon 
								OLED_Refresh_bank(2) ;		 
								//OLED_DEBUG("update liquid to ota"); 

						}
						if(check_event(NEED_UPDATE_BATERRY))
						{
								clear_dis_event(NEED_UPDATE_BATERRY);
								Display_update_baterry( battery_value);	
								//OLED_DEBUG("update baterry icon");  
						}



				}
				else if(device_sta==DEVICE_SYNC_MODE)
				{

						if(check_event(NEED_UPDATE_ALL))
						{			 	 
								clear_dis_event(NEED_UPDATE_ALL);	 
								if(write_icon_update_flag)
								{
										write_icon_update_flag=0;
										Display_Pen(write_icon_update_flag);			 
										//OLED_DEBUG("clear writing icon");   
								}

								OLED_CLEAR_BUFF(); //清除缓存		 
								OLED_Draw_BMP(UPLOAD_ICON,56,16);	  //载入连接时的icon 
								OLED_Refresh_bank(2) ;		
								//OLED_DEBUG("update liquid to sync"); 

						}
						if(check_event(NEED_UPDATE_BATERRY))
						{
								Display_update_baterry( battery_value);	
								clear_dis_event(NEED_UPDATE_BATERRY);
								//OLED_DEBUG("update baterry icon");  
						}

				}

				else if(device_sta == DEVICE_FINISHED_PRODUCT_TEST)
				{

						if(oled_test_mode)
						{
								//OLED_DEBUG("mode[%d]",oled_test_mode);  
								Display_Test(oled_test_mode);
								oled_test_mode=0;	

						}

				}

				else
				{


				}

				start(display_timer,300); //300毫秒
    }
	
	
}
