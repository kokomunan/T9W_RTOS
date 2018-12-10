#include "gather_position.h"
#include "stm32f4xx_hal.h"
#include "General_Emr.h"
#include "stdlib.h"
#include "config.h"
#include "note_manager.h"
#include "RGB_LED.h"
#include "input_event.h"
#include "string.h"
#include "battery.h"
#include "display.h"
#include "debug_log.h"
#include "server.h"
#include "em9203.h"

extern st_device_data_storage g_device_info;
extern uint8_t Get_Device_Status(void);


#define DEBUG_LOG   err_log
int32_t x_last_pos=0;                    //坐标滤波
int32_t y_last_pos=0;
uint32_t last_time;
uint32_t system_tick_counts = 0;
uint8_t read_iic_flag=0;
stPosBuff g_position_buffer;
uint32_t test_count=0;

//返回0 正确 1错误
uint8_t check_position_data(int32_t x_pos, int32_t y_pos)
{
    uint32_t time_interval = 0;
    uint32_t x_pos_interval = 0;
    uint32_t y_pos_interval = 0;
    uint8_t device_sta;
	system_tick_counts= HAL_GetTick();
	
	device_sta=	Get_Device_Status();
    //DEBUG_LOG("time:[%d][%d]", system_tick_counts, last_time);
    if(DEVICE_ACTIVE ==  device_sta)
    {	
        Add_signal_count ();  //累计写入信号
    }
    if(system_tick_counts >= last_time)
    {
        time_interval = system_tick_counts - last_time;
    }
    else
    {
        time_interval = 0xffffffff - last_time + system_tick_counts;
    }
    last_time = system_tick_counts;
    //DEBUG_LOG("time:[%d]", time_interval);
    if(time_interval > 10)  //大于10ms
    {
        
        x_last_pos = x_pos;
        y_last_pos = y_pos;
        return 0;
    }
    else
    {
        //online mode data
        x_pos_interval = abs(x_pos - x_last_pos);
        y_pos_interval = abs(y_pos - y_last_pos);
        //DEBUG_LOG("pos:[%d][%d]", x_pos_interval, y_pos_interval);
        if(x_pos_interval > ORG_POSITION_MAX_CHANGE_NUM || (y_pos_interval > ORG_POSITION_MAX_CHANGE_NUM))
        {   
					  //DEBUG_LOG("position error\r\n");
            return 1;
        }
        x_last_pos = x_pos;
        y_last_pos = y_pos;
        return 0;
    }
}

void write_pos_buffer(uint8_t *buf, uint8_t len)
{
    uint8_t rst = 0;
    uint8_t device_sta;
    st_write_packet_data   packet;
    uint16_t  press = 0;
    device_sta=	Get_Device_Status();
    if((DEVICE_OFFLINE ==device_sta) && ( Is_enable_note_store()))
    {
		
        offline_store_data(buf,len);  //离线操作
				
    }
		else if(DEVICE_ACTIVE ==device_sta)  //如果是连接时
		{
				if(get_server_mode()==EM9203)
				{
					
					   if(0x11 == buf[1]) //如果是按下的
					   {
								packet.x_l= buf[2];
								packet.x_h= buf[3];
								packet.y_l= buf[4];
								packet.y_h= buf[5];
								press = (uint16_t)buf[7] << 8 | buf[6];
								packet.press=press/4;
							  test_count++;
								if(get_server_ready()==0)
								{
								    set_server_ready(1);
								}
							
					   }
					   else if(0x10 == buf[1])  //如果是悬浮  
					   {
						
								if(get_server_ready())
								{
                                      set_server_ready(0);						
                                      packet.x_l= buf[2];
                                      packet.x_h= buf[3];
                                      packet.y_l= buf[4];
                                      packet.y_h= buf[5];
                                      packet.press=0;
									  test_count++;
									  printf("data count %d\r\n",test_count*5);
									  test_count=0;
								}					
								else   //过滤后面的悬浮
								return ;
						
					  }
					  else return;
					  
				//  printf("X:%d-Y%d-Press:%d\r\n ", (packet.x_l|(packet.x_h<<8)),(packet.y_l|(packet.y_h<<8)),packet.press);
					
					
						if((packet.x_l==0)&&(packet.x_h==0)&&(packet.y_l==0)&&(packet.y_h==0))
						{

						    DEBUG_LOG("0");

						}
						rst = server_Send_position_to_host(MASS_OPS, (uint8_t *)&packet, 5);//写入缓冲中		
						if(rst)
						{
							  DEBUG_LOG("x");
						}	
					
						
					
				}
				else  //ble 服务
				{
				
						//连接时将坐标数据发送到缓冲
						if(0 == g_position_buffer.data_count)
						{							 
								memcpy(&g_position_buffer.pos_data[0], (uint8_t *)buf, len);
								if(buf[1] != 0x00)
								{
										g_position_buffer.data_count++;
								}
								else
								{
										rst = server_Send_position_to_host(BLE_CMD_POS_DATA, (uint8_t *)&g_position_buffer.pos_data[0], 8);
										if(rst)
										{
										 
												DEBUG_LOG("x");
										}
										g_position_buffer.data_count = 0;
								}
						}
						else
						{
				        memcpy(&g_position_buffer.pos_data[1], (uint8_t *)buf, len);
								
								rst = server_Send_position_to_host(BLE_CMD_POS_DATA, (uint8_t *)&g_position_buffer.pos_data[0], 16);
								if(rst)
								{
						
									 DEBUG_LOG("x");
								}
								g_position_buffer.data_count = 0;
						}	
			 }
		
    }
}

void set_read_iic_enable(void)
{
    read_iic_flag=1;
}

void read_position_thread(void)
{
    if(read_iic_flag)
    {

        read_position();
        read_iic_flag=0;
    }
	
	
}


void read_position(void)
{
	
    int32_t x_pos = 0, y_pos = 0;
    uint8_t device_sta;
    uint8_t tmp[8], rst = 0;
    device_sta=Get_Device_Status();
        
    if((DEVICE_ACTIVE ==device_sta)|| (DEVICE_FINISHED_PRODUCT_TEST ==device_sta)|| ( DEVICE_OFFLINE ==device_sta))
    {

        if((get_server_mode()==EM9203)&&(get_server_status()!=NEBULA_STATUS_MASSDATA))  //私有协议时必须是大数据状态下
        {
                        
            return;
            
        }
        rst = Read_EMR(tmp, 8);
        x_pos = (uint16_t)tmp[3] << 8 | tmp[2];
        y_pos = (uint16_t)tmp[5] << 8 | tmp[4];
        
        //	printf("X:%d-Y%d-press:%d \r\n ", x_pos, y_pos);
        if(rst==0)
        {
            if(tmp[1] != 0x00)
            {
                    rst = check_position_data(x_pos, y_pos);
            }
    
        }
        if((0 == rst) && (tmp[0] == 0x02))
        {
                                    
            write_pos_buffer(tmp, 8);
            update_dis_Pen_icon();
            Reset_Low_Power_Timer();
        }
            
    }
    	
}
void init_position_buffer(void)
{
    x_last_pos=0;
    y_last_pos=0;
    memset((uint8_t *)&g_position_buffer, 0, sizeof(stPosBuff));
	
	
}



