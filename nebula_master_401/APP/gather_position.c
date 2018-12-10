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
#include "misc.h"
#include "gather_position.h"
#include "ble_client.h"
extern st_device_data_storage g_device_info;
extern uint8_t Get_Device_Status(void);



int32_t x_last_pos=0;                    //坐标滤波
int32_t y_last_pos=0;
uint32_t last_time;
uint32_t system_tick_counts = 0;
uint8_t read_iic_flag=0;
stPosBuff g_position_buffer;
uint8_t leave_flag=0;
uint8_t test_count=0x30;
//uint32_t test_count=0;
//uint32_t     data_count=0;
//返回0 正确 1错误
uint8_t check_position_data(int32_t x_pos, int32_t y_pos)
{
    uint32_t time_interval = 0;
    uint32_t x_pos_interval = 0;
    uint32_t y_pos_interval = 0;
    uint8_t device_sta;
	system_tick_counts= HAL_GetTick();
	
	device_sta=	Get_Device_Status();
    //debug_log("time:[%d][%d]", system_tick_counts, last_time);
    if(DEVICE_ACTIVE ==  device_sta)
    {	
      //  Add_signal_count ();  //累计写入信号
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
    //debug_log("time:[%d]", time_interval);
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
        //debug_log("pos:[%d][%d]", x_pos_interval, y_pos_interval);
        if(x_pos_interval > ORG_POSITION_MAX_CHANGE_NUM || (y_pos_interval > ORG_POSITION_MAX_CHANGE_NUM))
        {   
					  //debug_log("position error\r\n");
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
    uint8_t server_mode;
    device_sta=	Get_Device_Status();
    if((DEVICE_OFFLINE ==device_sta) && ( Is_enable_note_store()))
    {
		
        offline_store_data(buf,len);  //离线操作
				
    }
    else if(DEVICE_ACTIVE ==device_sta)  //如果是连接时
    {
        
            update_dis_Pen_icon();
            server_mode=get_server_mode();
            if(server_mode==WIFI_MODE)
            {

                if(g_position_buffer.data_count==0)
                {							 
                        memcpy(&g_position_buffer.pos_data[0], (uint8_t *)buf, len);
                        if(buf[1] != 0x00)
                        {
                                g_position_buffer.data_count++;
                        }
                        else
                        {
                                rst = server_Send_position_to_host(CMD_POS_DATA, (uint8_t *)&g_position_buffer.pos_data[0], 8);
                                if(rst)
                                {
                                 
                                        debug_log("x");
                                }
                                g_position_buffer.data_count = 0;
                        }
                }
                else if( g_position_buffer.data_count==1)
                {
                    
                  
                     memcpy(&g_position_buffer.pos_data[1], (uint8_t *)buf, len);
                    
                     if(buf[1] != 0x00)
                        {
                                g_position_buffer.data_count++;
                        }
                        else
                        {
                                rst = server_Send_position_to_host(CMD_POS_DATA, (uint8_t *)&g_position_buffer.pos_data[0], 16);
                                if(rst)
                                {
                                 
                                        debug_log("x");
                                }
                                g_position_buffer.data_count = 0;
                        }
                     
                }
                
                 else if(g_position_buffer.data_count==2)
                {
                        memcpy(&g_position_buffer.pos_data[2], (uint8_t *)buf, len);
                        
                        if(buf[1] != 0x00)
                        {
                                g_position_buffer.data_count++;
                        }
                        else
                        {
                                rst = server_Send_position_to_host(CMD_POS_DATA, (uint8_t *)&g_position_buffer.pos_data[0], 24);
                                if(rst)
                                {
                                 
                                        debug_log("x");
                                }
                                g_position_buffer.data_count = 0;
                        }
                       
                }
                 else if( g_position_buffer.data_count==3)
                {
                        memcpy(&g_position_buffer.pos_data[3], (uint8_t *)buf, len);
                        
                        if(buf[1] != 0x00)
                        {
                                g_position_buffer.data_count++;
                        }
                        else
                        {
                                rst = server_Send_position_to_host(CMD_POS_DATA, (uint8_t *)&g_position_buffer.pos_data[0], 32);
                                if(rst)
                                {
                                 
                                        debug_log("x");
                                }
                                g_position_buffer.data_count = 0;
                        }
                        
                }
                else if( g_position_buffer.data_count==4)
                {
                        memcpy(&g_position_buffer.pos_data[4], (uint8_t *)buf, len);
                        
                       
                        rst = server_Send_position_to_host(CMD_POS_DATA, (uint8_t *)&g_position_buffer.pos_data[0], 40);
                        if(rst)
                        {
                         
                                debug_log("x");
                        }
                        g_position_buffer.data_count = 0;
                        
                }
                
                    
                
            }
            else  //ble  usb服务
            {
            
                    if(server_mode==NRF51822)
                    {
                        //检查通知使能没有打开 返回
                       if(IS_ENABLE_NOTIFY==0)
                       {
                           return;
                       }
                        
                    }
                
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
                                    rst = server_Send_position_to_host(CMD_POS_DATA, (uint8_t *)&g_position_buffer.pos_data[0], 8);
                                    if(rst)
                                    {
                                     
                                            debug_log("x");
                                    }
                                    g_position_buffer.data_count = 0;
                            }
                    }
                    else
                    {
                            memcpy(&g_position_buffer.pos_data[1], (uint8_t *)buf, len);
                            
                            rst = server_Send_position_to_host(CMD_POS_DATA, (uint8_t *)&g_position_buffer.pos_data[0], 16);
                            if(rst)
                            {
                    
                                 debug_log("x");
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
    
    if(get_server_mode()!=WIFI_MODE)
    {
         if(read_iic_flag)
        { 
            read_iic_flag=0;
            read_position();
           
        }
         
    }
  	
}

void read_position(void)
{
	
    int16_t x_pos = 0, y_pos = 0;
 //   int16_t  press;
    uint8_t device_sta;
    uint8_t tmp[8], rst = 0;
    device_sta=Get_Device_Status();
      
    if((DEVICE_ACTIVE ==device_sta)|| ( DEVICE_OFFLINE ==device_sta)||(SENSOR_UPDATE ==device_sta)||( SENSOR_CALIBRA ==device_sta))   
    {

    
        rst = Read_EMR(tmp, 8);
        
        if(0 == rst)
        {
            if(tmp[0] == 0x02)   //position
            {
            
                if(tmp[1] != 0x00)
                {
                     x_pos = (uint16_t)tmp[3] << 8 | tmp[2];
                     y_pos = (uint16_t)tmp[5] << 8 | tmp[4];
               //      press=(uint16_t)tmp[7] << 8 | tmp[6];
                     rst = check_position_data(x_pos, y_pos);
                     if(rst==0)
                    {
                        write_pos_buffer(tmp, 8);
                       // update_dis_Pen_icon();
                        Reset_Low_Power_Timer();
                        
                    }
                }
                else
                {
                    write_pos_buffer(tmp, 8);
                   // update_dis_Pen_icon();
                    Reset_Low_Power_Timer();
                    
                }
            }
            else if(tmp[0] == 0xaa)
            {
                debug_log("i2c cmd %x",tmp[1]);
                ERM_cmd_handle(tmp);
                                                               
            }
            
            
            
        }
            
    }
    
    	

        
}
void init_position_buffer(void)
{
    x_last_pos=0;
    y_last_pos=0;
    memset((uint8_t *)&g_position_buffer, 0, sizeof(stPosBuff));
	
	
}

uint8_t If_update_sucsess(uint16_t new_version)
{
 
   
    if(check_version(new_version))
    {
        return 1;
    }
    
    return 0;
    
    
    
}

