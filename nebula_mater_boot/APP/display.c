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
uint8_t write_icon_update_flag=0;   //�Ѿ���ʾ��д��ͼ��
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

//ˢ�²���ͼ��
void update_oled_test_mode(uint8_t mode)
{
	  oled_test_mode=mode;
	
}
//ˢ�µ���
void update_dis_battery_value(uint8_t level)
{
		battery_value=level;
		update_dis_event(NEED_UPDATE_BATERRY);
}
//ˢ�����߱ʼ���Ϣ
void update_dis_offline_page(uint8_t cur,uint8_t total)
{
		offline_note_cur_page=cur;
		offline_note_tal_page=total;
		update_dis_event(NEED_UPDATE_NOTE_INFOR_ICON);
}
//ˢ�����߱ʼ���Ϣ
void update_dis_online_page(uint8_t cur,uint8_t total)
{
		online_display_cur_page=cur;
		online_display_tal_page=total;
		update_dis_event(NEED_UPDATE_NOTE_INFOR_ICON);
}
//ˢ�±�ͼ��
void update_dis_Pen_icon(void)
{
    update_dis_event(NEED_UPDATE_WRITING_ICON);
	
}
void update_dis_all(void)
{
	  update_dis_event(NEED_UPDATE_ALL);
}

//Ҫ���ݵ�ǰ�ķ���ģʽ��ʾ��ͬ�Ŀ�������
void  Display_init(void)
{
		currunt_server_mode=get_server_mode();  //��ȡ��ǰ��ģʽ
		OLED_On_Init();
		if(currunt_server_mode==NRF51822)	//����ģʽ���Ʋ�ͬ�������豸ͼ��	
		{
				OLED_Draw_BMP(BLE_ICON,2,12);	//��������ͼ��	
		}
		else
		{
				OLED_Draw_BMP(PRAV_ICON,2,12);//����˽��Э��ͼ��
		}
		OLED_Refresh_bank(0) ; //�����ϰ벿			
		DRAW_2_Area((uint8_t *)&LOGO,19,90)  ; //Ӧ�ø���ģʽ���Ʋ�ͬ��logo
		OLED_Display_On(); //����oled�豸
		display_timer=register_soft_timer();
		start(display_timer,300); //300����
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
		DRAW_1_Area(battery_icon[baterry_level-1],127-18,18)	; //��ʾ���//���ݵ�صĵ�����ʾ������Ӧ��ͼƬ
	
}
	
void Display_update_storage_space(uint32_t left_secoters)
{
		uint8_t percent;
		float use=1024-left_secoters;
		float tatol=1024;	
		percent=(uint8_t)((use/tatol)*100)/2;
		OLED_Fill(12,6,12+percent,9) ;  // ���ý���������
	
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

		SPI_Write_Command(SET_MEM_ADDR_MODE_CMD);  //���õ�ַģʽ
		SPI_Write_Command(VERTICAL_ADDR_MODE_CMD);  //����
		SPI_Write_Command(SET_COLUMN_ADDR_CMD); //����������ʼ����ֹ��ַ
		SPI_Write_Command(2);  //��ʼ��ַ
		SPI_Write_Command(129);  //������ַ
		SPI_Write_Command(SET_PAGE_ADDR_CMD); //����ҳ��ַ
		SPI_Write_Command(0); //��ʼ                        
		SPI_Write_Command(7); //����
		SPI_Write_Data(&OLED_GRAM[0][0],512);   //һ����1K�ֽ�ram ��Ҫд����
		SPI_Write_Data(&OLED_GRAM[0][0],512);  
}


//��ʾ�����   ����״̬���¼�����  z�ⲿ�ִ���д��̫���� �Ժ���д
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
								OLED_CLEAR_BUFF(); //�������
								if(currunt_server_mode==NRF51822)  //ֻ��bleʱ����ʣ��洢����
								{		
										OLED_Draw_BMP(SDCARD_ICON,0,9);	  //����sd��
										OLED_DrawRectangle(12, 3, 62, 12);	 //���ƾ��ο�
										Display_update_storage_space(get_free_sector());  //���ƽ�����		 
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
								OLED_CLEAR_BUFF(); //�������		 
								OLED_Draw_BMP(CONNECTED_ICON,56,16);	  //��������ʱ��icon 
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
								OLED_CLEAR_BUFF(); //�������		 
								OLED_Draw_BMP(CONNECTED_ICON,56,16);	  //��������ʱ��icon 
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
								//����ʱ�־
								if(write_icon_update_flag)
								{
										write_icon_update_flag=0;
										Display_Pen(write_icon_update_flag);			 
										//OLED_DEBUG("clear writing icon");   
								}

								OLED_CLEAR_BUFF(); //�������		 
								OLED_Draw_BMP(DOWNLOAD_ICON,56,16);	  //��������ʱ��icon 
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

								OLED_CLEAR_BUFF(); //�������		 
								OLED_Draw_BMP(UPLOAD_ICON,56,16);	  //��������ʱ��icon 
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

				start(display_timer,300); //300����
    }
	
	
}
