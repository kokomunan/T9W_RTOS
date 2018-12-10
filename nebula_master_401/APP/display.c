#include "display.h"
#include "oled.h"
#include "oled_icon.h"
#include "config.h"
#include "stdio.h"
#include "debug_log.h"
#include "Soft_Timer.h"
#include "server.h"
#include "input_event.h"
#include "usb_controler.h"
#include "battery.h"
#include "Status_Machin.h"
#include "ewm3080b.h"
#include "norflash.h"
#include "cmsis_os.h"
extern st_device_data_storage g_device_info;
extern uint8_t OLED_GRAM[128][4];
extern uint8_t Get_Device_Status(void);
extern uint32_t get_free_sector(void);

#define SETBIT(BYTE,BIT)         ( (BYTE) |= (uint8_t)((uint8_t)1<<(uint8_t)(BIT)) )
#define CLRBIT(BYTE,BIT)         ( (BYTE) &= (uint8_t)((uint8_t)((uint8_t)1<<(uint8_t)(BIT))^(uint8_t)0xff) )
#define FRESH_PERIOD           150
#define PRESENT_INTERVEL       4
#define TOTAL_INTERL         8

SOFT_TIMER_ST  *display_timer;

uint8_t* battery_icon[8]={BATEERY_LEVEL_1_ICON,BATEERY_LEVEL_2_ICON,BATEERY_LEVEL_3_ICON,BATEERY_LEVEL_4_ICON,BATEERY_LEVEL_5_ICON,BATEERY_LEVEL_6_ICON,BATEERY_LEVEL_7_ICON,BATEERY_LEVEL_8_ICON};
const uint8_t* charing_icon[6]={CHARGING_ICON_1,CHARGING_ICON_2,CHARGING_ICON_3,CHARGING_ICON_4,CHARGING_ICON_5,CHARGING_ICON_6};

uint8_t need_update_screen=0;
uint8_t online_display_cur_page=0;
uint8_t online_display_tal_page=0;
uint8_t offline_note_cur_page=0;
uint8_t offline_note_tal_page=0;
uint8_t write_icon_update_flag=0;   //已经显示了写入图标
uint8_t battery_value=0;
uint8_t oled_test_mode=0;
uint8_t need_redraw_flag=0;
uint8_t screen_num=MAIN_SCREEN;
uint8_t key_event=NON_KEY;
uint8_t chose_index=BLE_SERVER_INDEX;
uint8_t nebula_event=NO_EVENT;
uint8_t nebula_keys[MAX_VOTE_NUM];
uint8_t chose_max_index=0;
uint8_t currunt_server_mode;
uint8_t oled_test_value=0;
uint8_t power_off_charge_flag=0,power_off_charge_dis_flag=0;
uint8_t intervel_count=0;
uint8_t vote_num=0;
uint8_t vote_en=0;
uint8_t vote_finish_flag=0;
uint8_t base_num_icon,max_count;

osThreadId  display_thread_handle=NULL;

TimerHandle_t stop_display_charge_full_timer_handle;
TimerHandle_t delay_display_charge_timer_handle;
TimerHandle_t return_main_screem_timer_handle;


osThreadId power_off_battery_Display_Thread_Handle;
   
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
void update_oled_test_key(uint8_t key_value)
{
    oled_test_value=key_value;
    
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
void update_adr_icon(void)
{
   update_dis_event(NEED_UPDATE_ADR_INFO); 
    
}
void update_dis_all(void)
{
    update_dis_event(NEED_UPDATE_ALL);
}
void Display_update_addr(uint8_t class_id,uint8_t device_id)
{
    uint8_t temp[20];	
//	uint8_t len;
    OLED_Draw_BMP((uint8_t *)CLASS_ICON,2,8,1);	
    OLED_Draw_BMP((uint8_t *)DEVICE_NUM_ICON,80,10,1);	
    sprintf((char*)temp,"%d",class_id);
    //len=strlen((char*)temp);
	OLED_ShowString(20,0,temp,16);
    sprintf((char*)temp,"%d ",device_id+1);
    //len=strlen((char*)temp);
	OLED_ShowString(100,0,temp,16);
       
}
//要根据当前的服务模式显示不同的开机界面
void  Display_init(void)
{
		
    //OLED_On_Init();
    
    CLR_GDDRAM();
	OLED_CLEAR_BUFF(); //清除缓存
    currunt_server_mode=get_server_mode();  
    if(currunt_server_mode==NRF51822)	
    {
        DRAW_1_Area(BLE_ICON,2,12);	
    }
    else if(currunt_server_mode==WIFI_MODE)
    {
        DRAW_1_Area(PRAV_ICON,2,18);
    }
    else
    {
        DRAW_1_Area(USB_ICON,2,18);
    }

       
    OLED_Display_On(); //点亮oled设备  
    osThreadDef(Dsiplay_Thread,Dsiplay_Thread,osPriorityHigh,0,512);
    display_thread_handle=osThreadCreate(osThread(Dsiplay_Thread),NULL);
    
    osTimerDef(return_main_screem_timer,return_main_screem_timer);
    
    return_main_screem_timer_handle=osTimerCreate (osTimer(return_main_screem_timer), osTimerOnce,NULL);      

    
    
    
    if(display_thread_handle==NULL)
    {
        debug_log("display thread create error");
    }
    
     
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
          DRAW_1_Area(PEN_ICON,60,14); 
    }
    else
    {
          memset(zero,0,28);
          DRAW_1_Area(zero,60,14); 
        
    }
	
}

void Display_update_baterry(uint8_t baterry_level)
{
    if(POWER_UNCHARGING)  
    {
        if(baterry_level>7)
        {
            baterry_level=7;
        }
        else if(baterry_level<1)
        {
            baterry_level=1;
        }
        
        if((baterry_level==7)||(baterry_level==6))
        {
             DRAW_1_Area(battery_icon[6],127-18,18)	;
        }
        else if((baterry_level==2)||(baterry_level==3))
        {
             DRAW_1_Area(battery_icon[2],127-18,18)	;
            
        }
        else
        {
            DRAW_1_Area(battery_icon[baterry_level-1],127-18,18)	;
        }
                
        
    }
    else
    {
        
        if(IS_charge_full())
        {
            
            DRAW_1_Area(battery_icon[6],127-18,18)	;
        }
        else
        {
            DRAW_1_Area(battery_icon[7],127-18,18)	;
        }
    }
	
}
	
void Display_update_storage_space(uint32_t left_secoters)
{
  	uint8_t percent,block;
	float use=FLASH_MAX_SECTOR_NUM-left_secoters;
	float tatol=FLASH_MAX_SECTOR_NUM;	
	percent=(uint8_t)((use/tatol)*100);	
	block = percent / 4;
	if(block > 24)
	{
		block = 24;
	}
	
	//set 1 block of the bar if the percent is  1%
	if(percent >= 1 && percent <= 8)
	{
		block = 1;
	}
	
    #ifdef SHOW_MODE	
	block = 10;
    #endif
    OLED_Draw_BMP(SDCARD_ICON,8,9,1);	  
    OLED_DrawRectangle(22, 3, 120, 12);	 //绘制矩形框 
    
    
	for(uint8_t i = 0; i < block ; i++)
	{
		uint8_t temp = i * 4;
		OLED_Fill(24 + temp ,6,26 + temp,9) ;	// 设置进度条长度	
	}
	
}
void Display_update_offlinenote_page(uint32_t currunt_page,uint32_t total_pages)
{
    uint8_t icon_xpos;
    uint8_t temp[7];	
    uint8_t len;
    sprintf((char*)temp,"%d/%d",currunt_page,total_pages);
    len=strlen((char*)temp);
	icon_xpos = (X_TOTAL_LEN - ICON_LEN - len*10 - CENTER_BLANK)/2;   
    OLED_Draw_BMP((uint8_t *)&OFFLINE_NOTE_ICON,icon_xpos,32,0);	  
	OLED_ShowString(icon_xpos + ICON_LEN + CENTER_BLANK,3,temp,20);
    
	
	
}

void Display_update_onlinenote_page(uint32_t currunt_page,uint32_t total_pages)
{
    uint8_t icon_xpos;
    uint8_t temp[7];	
    uint8_t len;
    sprintf((char*)temp,"%d/%d",currunt_page,total_pages);
    len=strlen((char*)temp);

	icon_xpos = (X_TOTAL_LEN - ICON_LEN - len*10 - CENTER_BLANK)/2;   
    OLED_Draw_BMP((uint8_t *)&ONLINE_NOTE_ICON,icon_xpos,32,0);	
	OLED_ShowString(icon_xpos + ICON_LEN + CENTER_BLANK,3,temp,20);
	
	
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

void set_key_event(uint8_t key_value)
{
    key_event=key_value;
    
}

void auto_update_icon_init(void)
{
   intervel_count=0; 
     
}

void aotu_update_icon_thread (void)
{
    uint8_t server_mode;
    server_mode=get_server_mode();  
    intervel_count++;//计数加
    if(server_mode==NRF51822)
    {
        if(intervel_count<PRESENT_INTERVEL)
        {   

            if( get_server_status()== BLE_PAIRING)
            {
                DRAW_1_Area(BLE_ICON,2,12);
                DRAW_1_Area(DISCOVER_ICON,13,8); 
            }
            else
            {
                DRAW_1_Area(BLE_ICON,2,12);
            }
        }
        else
        {
            if( get_server_status()== BLE_PAIRING)
            {
                uint8_t buff[28];
                memset(buff,0,28);
                DRAW_1_Area(buff,2,12);
                DRAW_1_Area(buff,13,8); 
            }
            else
            {
                uint8_t buff[36];
                memset(buff,0,36);
                DRAW_1_Area(buff,2,18);
            }


        } 
        
    }
    else if(server_mode==WIFI_MODE)
    {
        
        if(intervel_count<PRESENT_INTERVEL)
        {   

           
            DRAW_1_Area(PRAV_ICON,2,18);
            
        }
        else
        {
     
            uint8_t buff[36];
            memset(buff,0,36);
            DRAW_1_Area(buff,2,18);
        
        } 
        
    }
    
        
    if(intervel_count>=TOTAL_INTERL)
    {
        intervel_count=0;
    }
    
}


void change_screen(uint8_t new_screen)
{
    if(new_screen!=screen_num)
    {
        screen_num=new_screen;    //设置刷新客户区
        need_redraw_flag=1;//设置绘制客户区标志
        CLR_GDDRAM();  //硬件清屏 
    }
   
    
}


uint8_t get_currunt_screen(void)
{
    return screen_num;
}



void redraw_main_sceen(void)
{
    
    update_dis_event(NEED_UPDATE_ALL);
    currunt_server_mode=get_server_mode();  
    CLR_GDDRAM();  //硬件清屏 
    OLED_CLEAR_BUFF();
    if(currunt_server_mode==NRF51822)	
    {
        DRAW_1_Area(BLE_ICON,2,12);	
    }
    else if(currunt_server_mode==WIFI_MODE)
    {
        DRAW_1_Area(PRAV_ICON,2,18);
    }
    else
    {
        DRAW_1_Area(USB_ICON,2,18);
    }
 	if((currunt_server_mode==NRF51822)||(currunt_server_mode==USB_DEVICE))
    {    
        OLED_CLEAR_BUFF();   
        Display_update_offlinenote_page(offline_note_cur_page,offline_note_tal_page);
        OLED_Refresh_bank(1) ; 	
        
    }
	

  
   
}

void screen_main(void)
{
    uint8_t device_sta;
    device_sta=Get_Device_Status();	
    currunt_server_mode=get_server_mode();  
    if(need_redraw_flag)
    {
        need_redraw_flag=0;
        redraw_main_sceen();
       
    }
  
    
    if(device_sta==DEVICE_OFFLINE)//offline
    {
        
        if(check_event(NEED_UPDATE_ALL))	 
        {
            clear_dis_event(NEED_UPDATE_ALL);                      
            Display_update_baterry( battery_value);	       
          
                      
            if(currunt_server_mode==NRF51822)
            {
                OLED_CLEAR_BUFF(); //清除缓存
                Display_update_offlinenote_page(offline_note_cur_page,offline_note_tal_page);
                OLED_Refresh_bank(1) ;
                OLED_CLEAR_BUFF(); //清除缓存      
                Display_update_storage_space(get_free_sector());  	
                OLED_Refresh_bank(2) ;
            }
            else if(currunt_server_mode==WIFI_MODE)
            {
                uint8_t buff[52];
                memset(buff,0,52);                
                DRAW_1_Area(buff,26,26) ;  
                OLED_CLEAR_BUFF(); 
                OLED_Refresh_bank(1) ; 
                OLED_CLEAR_BUFF(); 
                Display_update_addr(g_device_info.node_info.class_number,g_device_info.node_info.device_number);  
                OLED_Refresh_bank(2) ; 
                
            }
            auto_update_icon_init();
         
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
            if(currunt_server_mode==NRF51822)
            {
                Display_update_onlinenote_page(online_display_cur_page,online_display_tal_page);	
                OLED_Refresh_bank(1) ;
                OLED_CLEAR_BUFF(); //清除缓存
                Display_update_storage_space(get_free_sector());  	
                OLED_Refresh_bank(2) ;	
            }                

        }
    
        if(check_event(NEED_UPDATE_WRITING_ICON))
        {
            clear_dis_event(NEED_UPDATE_WRITING_ICON);	
            write_icon_update_flag=1;
            Display_Pen(write_icon_update_flag);
  

        }
        else 
        {
            if(write_icon_update_flag)
            {
                write_icon_update_flag=0;
                Display_Pen(write_icon_update_flag);
            }

        }
        
        
  
    aotu_update_icon_thread () ;   

  //  OLED_Display_On();   
     
    }
    else if(device_sta==DEVICE_ACTIVE)
    {

        if(check_event(NEED_UPDATE_ALL))
        {			 	 
            clear_dis_event(NEED_UPDATE_ALL);	
            Display_update_baterry( battery_value);	  
            if(currunt_server_mode==NRF51822)	
            {
             
               
                uint8_t buff[28];
                memset(buff,0,28);
                DRAW_1_Area(buff,13,8);   
                DRAW_1_Area(BLE_ICON,2,12);	
            }
            else if(currunt_server_mode==WIFI_MODE)
            {
                 DRAW_1_Area(PRAV_ICON,2,18);
            }
            else
            {
                 DRAW_1_Area(USB_ICON,2,18);
            }
            if((currunt_server_mode==NRF51822)||(currunt_server_mode==USB_DEVICE))
            {
                OLED_CLEAR_BUFF(); //清除缓存	
                Display_update_onlinenote_page(online_display_cur_page,online_display_tal_page);			 
                OLED_Refresh_bank(1) ;	            
                OLED_CLEAR_BUFF(); //清除缓存		                   		 
                OLED_Draw_BMP(LEFT_CURSOR,11,6,1);
                OLED_Draw_BMP(RIGHT_CURSOR,110,6,1);
                OLED_Refresh_bank(2) ;  
                
            }
            else  
            {
     
                OLED_CLEAR_BUFF(); 
                Display_update_addr(g_device_info.node_info.class_number,g_device_info.node_info.device_number);  
                OLED_Refresh_bank(2) ; 

            }                
         
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
            if((currunt_server_mode==NRF51822)||(currunt_server_mode==USB_DEVICE))
            {
               
               	 
                OLED_CLEAR_BUFF(); //清除缓存		 
                Display_update_onlinenote_page(online_display_cur_page,online_display_tal_page);			 
                OLED_Refresh_bank(1) ;	
            }                
            
        }

        if(check_event(NEED_UPDATE_WRITING_ICON))
        {
            clear_dis_event(NEED_UPDATE_WRITING_ICON);	
            write_icon_update_flag=1;
            Display_Pen(write_icon_update_flag);
  

        }
        else 
        {
            if(write_icon_update_flag)
            {
                write_icon_update_flag=0;
                Display_Pen(write_icon_update_flag);
            }

        }
  
       
    }
    
    else if((device_sta==DEVICE_OTA_MODE)||(device_sta==DEVICE_DFU_MODE)||(device_sta==SENSOR_UPDATE))
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
            OLED_Draw_BMP(DOWNLOAD_ICON,50,32,0);	  //载入连接时的icon 
            OLED_Refresh_bank(1) ;	
            OLED_CLEAR_BUFF(); //清除缓存	
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
            OLED_Draw_BMP(UPLOAD_ICON,50,32,0);	  //载入连接时的icon 
            OLED_Refresh_bank(1) ;	
            OLED_CLEAR_BUFF(); //清除缓存	
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
    else if(device_sta==SENSOR_CALIBRA)
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
            OLED_Draw_BMP(CALIBRA_ICON,50,32,0);	  //载入连接时的icon 
            
            
            OLED_Refresh_bank(1) ;	
            OLED_CLEAR_BUFF(); //清除缓存	
            OLED_Refresh_bank(2) ;	
            
            //OLED_DEBUG("update liquid to sync"); 

        }
        if(check_event(NEED_UPDATE_BATERRY))
        {
            Display_update_baterry( battery_value);	
            clear_dis_event(NEED_UPDATE_BATERRY);

        }
        
        
              
        
        
    }


    else
    {


    }

    
}


void return_main_screem_timer(const void* argument)
{
    
      change_screen(MAIN_SCREEN);  //返回主界面   
    
}

void screen_wait(void)
{
    uint8_t server_mode;
    server_mode=get_server_mode();
    if(need_redraw_flag)
    {      
       osTimerStart (return_main_screem_timer_handle, 1000);    
       if(server_mode==NRF51822) 
      {
         CLR_GDDRAM(); 
         DRAW_2_Area((uint8_t *)&WAIT_BLE_ICON,0,128);
      }
      else if(server_mode==WIFI_MODE)
      {
         CLR_GDDRAM(); 
         DRAW_2_Area((uint8_t *)&WAIT_NUB_ICON,0,128);
          
      }
      else
      {
         CLR_GDDRAM(); 
         DRAW_2_Area((uint8_t *)&WAIT_USB_ICON,0,128); 
          
      }   
      need_redraw_flag=0;
    }
      
    
}

void screen_low_power(void)
{
    
     if(need_redraw_flag)
    { 
   
       osTimerStart (return_main_screem_timer_handle, 2000);      
       need_redraw_flag=0;       
       CLR_GDDRAM(); 
       DRAW_2_Area((uint8_t *)&LOW_POWER_ICON,0,128); 

    }
 
     
    
}

void screen_test_nrf(void)
{
    
     if(need_redraw_flag)
    { 
                  
        char dis_buff[16];
        uint8_t len;       
        need_redraw_flag=0;            
        CLR_GDDRAM();    
        sprintf(dis_buff,"test nrf"); //打印第一个键值
        len=strlen(dis_buff);
        OLED_CLEAR_BUFF();	
        OLED_ShowString(64-((len*8)/2),0,(uint8_t *)dis_buff,16);
        OLED_Refresh_bank(0) ; 	           
               

    }
    
    if(oled_test_value!=0)
    {
       char dis_buff[16]; 
       uint8_t len;       
        sprintf(dis_buff,"key:%d",oled_test_value); //打印第一个键值
        len=strlen(dis_buff);
        OLED_CLEAR_BUFF();	
        OLED_ShowString(64-((len*8)/2),0,(uint8_t *)dis_buff,16);
        OLED_Refresh_bank(1) ; 	     
        oled_test_value=0;
        
    }
    
    
}


//显示处理层   根据状态和事件驱动  z这部分代码写的太垃圾 以后重写
void Dsiplay_Thread(const void * argument)
{
    
    while(1)
    {
        currunt_server_mode=get_server_mode();  //获取当前的模式
       
        if(screen_num==MAIN_SCREEN)
        {
            screen_main();

        }      
        else if(screen_num==WAIT_SCREEN)
        {
            screen_wait();
        }
   
        else if(screen_num==LOWPOER_SCREEN)
        {
            
          screen_low_power();  
        }
        else if(screen_num==TEST_SCREEN)
        {
            if(oled_test_mode)
            {
                
                Display_Test(oled_test_mode);
                oled_test_mode=0;	

            }  
            
        }
        else if(screen_num==TEST_NRF_SCREEN)
        {
              screen_test_nrf();
        }
       
        osDelay(FRESH_PERIOD);
    }
    
    
	
	
}


//显示充满的函数
void Display_chargefull(void)
{
    CLR_GDDRAM();
    DRAW_2_Area((uint8_t *) CHARGING_ICON_6,0,128);    
    OLED_Display_On(); //点亮oled设备 
    debug_log("oled charge full");    
}

void Display_charging_init(void)
{
    intervel_count=0;
    max_count=0;
    OLED_Display_On(); //点亮oled设备    
}
void  stop_display_charge_full(void const *argument)
{
    power_off_charge_dis_flag=0; 
    OLED_Display_Off();
    debug_log("clsoe oled");
    
}
//显示正在充电的函数
void delay_charge_full(void const *argument)  //一秒定时器的回调函数  到达后重新定时
{
    

    if(intervel_count==0)    
    {

        base_num_icon=1; 
        max_count=6-base_num_icon;

        CLR_GDDRAM(); 
        DRAW_2_Area((uint8_t *)charing_icon[base_num_icon-1],0,128);  //绘制相应的电压图标
        intervel_count++;  
    }
    else if(intervel_count>max_count)
    {
        intervel_count=0;
    }
    else
    {

        CLR_GDDRAM(); 
        DRAW_2_Area((uint8_t *)charing_icon[base_num_icon+intervel_count-1],0,128); 
        intervel_count++;  
    }    

   
}

void power_off_battery_Display_init(void)
{
    
   power_off_charge_flag=0;
   power_off_charge_dis_flag=0;
   CLR_GDDRAM();
   osTimerDef(stop_display_charge_full_timer,stop_display_charge_full);
   osTimerDef(delay_charge_full_timer,delay_charge_full);
    
   stop_display_charge_full_timer_handle=osTimerCreate (osTimer(stop_display_charge_full_timer), osTimerOnce,NULL);       
   delay_display_charge_timer_handle=osTimerCreate (osTimer(delay_charge_full_timer), osTimerPeriodic,NULL);

    
    
   osThreadDef(power_off_battery_Display_Thread, power_off_battery_Display_Thread, osPriorityNormal, 0, 512); 
   power_off_battery_Display_Thread_Handle=osThreadCreate(osThread(power_off_battery_Display_Thread), NULL);
  
   
}
void power_off_battery_Display_deinit(void)
{
    
   power_off_charge_flag=0;
   power_off_charge_dis_flag=0;
        
   osTimerDelete (stop_display_charge_full_timer_handle);
   osTimerDelete(delay_display_charge_timer_handle);
    
   osThreadTerminate (power_off_battery_Display_Thread_Handle); //关闭充电显示线程
    
    

}

void power_off_battery_Display_Thread(const void* argument)
{
    osStatus res; 
  
    while(1)
    {

        if(POWER_CHARGING)	
        {
            if(IS_charge_full())   //如果充满了   之前的状态不是充满  ， 开启定时器 显示充满界面
            {
                          
                if( power_off_charge_flag)
                {
                   
                    power_off_charge_flag=0;                               
                    res=osTimerStart (stop_display_charge_full_timer_handle, 3000);   
                    if(res)
                    {
                        debug_log("stop display charge full timer start ERROR" );
                    }
                    
                    Display_chargefull();
                    power_off_charge_dis_flag=1;
                    osTimerStop (delay_display_charge_timer_handle); 
                    
                    
                  
                    
                }
                    
                if(POWER_BUTTON_PRESS) //检测按键如果按键按下  
                {


                    if(!power_off_charge_dis_flag)
                    {

                        res=osTimerStart (stop_display_charge_full_timer_handle, 3000); 
                        if(res)
                        {
                            debug_log("stop display charge full timer start ERROR" );
                        }
                        Display_chargefull();
                        power_off_charge_dis_flag=1;
                    }
                }



            }
            else 
            {
                if(power_off_charge_flag!=1)
                {
                    power_off_charge_flag=1;
                    Display_charging_init();
                    res=osTimerStart (delay_display_charge_timer_handle, 1000); 
                   if(res)
                    {
                        debug_log("delay display charge  timer start ERROR" );
                    }
                                 
                }       

            }           



        }
        osDelay(100);
    }
    
    
    
}
void Display_waite_updata(void)
{
    char dis_buff[16];
    uint8_t len;
    CLR_GDDRAM();
    //DRAW_2_Area((uint8_t *) CHARGING_ICON_6,0,128);    
    
    sprintf(dis_buff,"updating"); //打印第一个键值
    len=strlen(dis_buff);
    OLED_CLEAR_BUFF();	
    OLED_ShowString(64-((len*8)/2),8,(uint8_t *)dis_buff,16);
    OLED_Refresh_bank(1) ; 	
       
    OLED_Display_On(); //点亮oled设备 
    debug_log("fw is updating");   
       
}

void notify_low_power(void)
{
	
    OLED_Display_On(); //点亮oled设备    
    CLR_GDDRAM(); 
    DRAW_2_Area((uint8_t *)&LOW_POWER_ICON,0,128); 

	
}

