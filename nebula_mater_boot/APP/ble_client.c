
#include "ble_client.h"
#include "Status_Machin.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "config.h"
#include "RGB_LED.h"
#include "EXT_RTC.h"
#include "General_Emr.h"
#include "battery.h"
#include "debug_log.h"
#include "display.h"
#include "input_event.h"
#include "norflash.h"
#include "note_manager.h"
#include "power_manager.h"
#include  "device_info.h"
#include "server.h"
#include "nrf51822.h"
#include "debug_log.h"
#define DEVICE_NAME_FLAG            0x55

#define DEBUG_LOG  debug_log
extern stMainStatus  g_main_status;
extern st_device_data_storage g_device_info;
extern BLE518XX_OPS  ble_ops;  //蓝牙模块操作设备


extern st_note_header_info note_header;     //笔记头
extern uint32_t note_store_start_sector ; //存储起始扇区
extern uint32_t note_upload_start_addr;     //上传笔记的起始地址
extern uint32_t note_store_addr_offset ; //笔记存储相对扇区开始的偏移地址
extern uint32_t note_upload_len;          //上传笔记的长度
extern uint8_t note_upload_flag;            //开始上传笔记的标志
extern uint32_t new_note_of_start_sector; //用于搜索用的



void send_error_msg(uint8_t error)
{
    ble_ops.Write(BLE_CMD_ERROR_MSG, &error, 1);
}

void host_client(uint8_t *p_event_data)
{
	
	 uint8_t op_code = *((uint8_t *)p_event_data + 1);
    uint8_t len = *(uint8_t *)((uint8_t *)p_event_data + 2);
    uint8_t *payload = ((uint8_t *)p_event_data + 3);

		
	 DEBUG_LOG("<<data[0x%x 0x%x 0x%x 0x%x]",0xaa,op_code,len,payload[0]);
	
	
    switch(op_code)
    {
        case BLE_CMD_STATUS:
        {
            DEBUG_LOG("status check\n");
        }break;
        case BLE_CMD_NAME_SET:
        {
            uint8_t name_f = 0;
            DEBUG_LOG("name set[%d]\n", len);
            DEBUG_LOG("[%s]", ((char *)payload));

            if(len > 0)
            {
                g_device_info.identifier = 0x1985;
                name_f = *payload;
                if((name_f < 48) || (name_f > 122))
                {
                    send_error_msg(ERROR_NAME_CONTENT);
                    break;
                }
                memcpy(g_device_info.name.device_name, payload, len);
                g_device_info.name.device_name_flag = DEVICE_NAME_FLAG;
                g_device_info.name.device_name_length = len;				
								Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage));  //存储上名字        
         
                ble_ops.Write(BLE_CMD_NAME_SET, NULL, 0);
            }
        }break;
        case BLE_CMD_GET_VERSION:
        {
            DEBUG_LOG("get version\n");
            st_version_info version;
            version.hw_version = g_device_info.hardware_version;
            version.fw_version = SW_VERSION;
					  g_device_info.link_code=payload[0];
					  DEBUG_LOG("get link code[%x]\n", g_device_info.link_code);
					  Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage)); 
            DEBUG_LOG("get version0x[%x]\n", g_device_info.hardware_version);
            ble_ops.Write(BLE_CMD_GET_VERSION, (uint8_t *)&version, sizeof(st_version_info));
        }break;
        case BLE_CMD_RTC_SET:
        {
            
            st_RTC_info rtc_data;
            memcpy((uint8_t *)&rtc_data, payload, sizeof(st_RTC_info));
            DEBUG_LOG("rtc set[%d][%d][%d][%d][%d]\n", rtc_data.note_time_year,
                                                       rtc_data.note_time_month,
                                                       rtc_data.note_time_day,
                                                       rtc_data.note_time_hour,
                                                       rtc_data.note_time_min);
               
            write_rtc_data(&rtc_data);
        }break;
        case  BLE_CMD_DISPALY_PAGE:     //根据host 端发送显示命令显示响应的页码  如果一开始连接这个命令发送到slave失败会导致页码一开始不显示 可能这里要修补
        {
					if(DEVICE_ACTIVE == g_main_status.device_state) 
					{
           if(len==2)
					 {
									 
						 update_dis_online_page(payload[0],payload[1]);
					 }	
				 }
								
        }break;
        
   
        case BLE_CMD_SYNC_MODE_ENTER:
        {
            DEBUG_LOG("enter sync mode\n");
            if((DEVICE_ACTIVE == g_main_status.device_state) 
                || (DEVICE_LOW_POWER_ACTIVE == g_main_status.device_state))
            {
                g_main_status.device_state = DEVICE_SYNC_MODE;
                //Send changed status       
                flash_wake_up();
                
            }
            else
            {
                send_error_msg(ERROR_OTA_STATUS);
            }
        }break;
        case BLE_CMD_SYNC_MODE_QUIT:
        {
            if(DEVICE_SYNC_MODE == g_main_status.device_state)
            {
                DEBUG_LOG("Quit sync!");
                g_main_status.device_state = DEVICE_ACTIVE;
                init_offline_store_data();
            }
            else
            {
                send_error_msg(ERROR_OTA_STATUS);
            }
        }break;
        case BLE_CMD_SYNC_FIRST_HEADER:  //开始搜索一个笔记的第一个笔记头
        {
            if(DEVICE_SYNC_MODE == g_main_status.device_state)
            {     
                //report note header
                if(g_device_info.stored_total_note_num)
                {
									 DEBUG_LOG("search from[%d]to[%d]", g_device_info.note_read_start_sector, g_device_info.note_read_end_sector);
									if(search_first_note_head(g_device_info.note_read_start_sector,&new_note_of_start_sector)==0)   //从环形的falsh里找到第一个笔记头 并返回这个起始的扇区地址
									{
										
										    note_upload_start_addr = note_header.note_start_sector * FLASH_SECTOR_SIZE + 256;  //上传数据的起始地址
											 // DEBUG_LOG("note data start addr[%x]",note_upload_start_addr);
                        note_store_addr_offset = 0;
                        note_upload_flag = 0;
											  note_upload_len=0;
                        ble_ops.Write(BLE_CMD_SYNC_FIRST_HEADER, (uint8_t *)&note_header, sizeof(st_note_header_info));
                        DEBUG_LOG("find new note num[%d][%d]", note_header.note_number, note_header.note_len);
										
										
									}
									
									// send_error_msg(ERROR_NO_NOTE);
                }
                else
                {
                    send_error_msg(ERROR_NO_NOTE);
                }
            }
            else
            {
                send_error_msg(ERROR_OTA_STATUS);
            }
        }break;
        case BLE_CMD_SYNC_ONCE_START:
        {
            if(DEVICE_SYNC_MODE == g_main_status.device_state)
            {
                if(note_upload_start_addr)
                {
                    //start to report raw data
                    DEBUG_LOG("sync start!");
                    note_upload_flag = 1;
                    ble_ops.Write(BLE_CMD_SYNC_ONCE_CONTENT, NULL, 0);
									  DEBUG_LOG("notify the host!");
                }
            }
            else
            {
                send_error_msg(ERROR_OTA_STATUS);
            }
        }break;
        case  BLE_CMD_SYNC_IS_END :   //如果相同笔记ID的块返回这个笔记下一个块的头部 如果搜索不到了 就将之前这个笔记ID所有的头部清除掉 然后返回2说明这个笔记传输完可以下一个笔记了
        {
					

					
            if((DEVICE_SYNC_MODE == g_main_status.device_state) && (0 == note_upload_flag))
            {
								
	              if(payload[0]==3)
							{
										  
										if(g_device_info.stored_total_note_num)
										{
														 DEBUG_LOG("search from[%d]to[%d]", g_device_info.note_read_start_sector, g_device_info.note_read_end_sector);
														if(search_other_note_head()==0)   //如果能继续找到相同id的笔记
														{
															
																	note_upload_start_addr = note_header.note_start_sector * FLASH_SECTOR_SIZE + 256;  //上传数据的起始地址
																 // DEBUG_LOG("note data start addr[%x]",note_upload_start_addr);
																	note_store_addr_offset = 0;
																	note_upload_flag = 0;
																	note_upload_len=0;
																	ble_ops.Write( BLE_CMD_SYNC_FIRST_HEADER , (uint8_t *)&note_header, sizeof(st_note_header_info));  //返回笔记头
																	DEBUG_LOG("find a note block[%d][%d]", note_header.note_number, note_header.note_len);
															
															
														}
														else
														{
																				
															uint8_t end_content = 0x01;               										       
															ble_ops.Write( BLE_CMD_SYNC_IS_END , &end_content, 1);  //返回  0
															DEBUG_LOG("find other block failed");
														}
											
										}
										else
										{
												send_error_msg(ERROR_NO_NOTE);
										}
							}

              else  if(payload[0]==2)
						{
                  uint8_t end_content = 0x02;       
              		g_device_info.note_read_start_sector=get_next_sector(new_note_of_start_sector);  //移动到这个笔记起始头的下一个扇区  这里以后可以优化成移动到起始头的数据区后的扇区
									g_device_info.stored_total_note_num -= 1;														
									note_store_addr_offset = 0;
									note_upload_len=0;
									note_upload_start_addr = 0;
									memset((uint8_t *)&note_header, 0, sizeof(st_note_header_info));  										
									DEBUG_LOG("remove a note, total note[%d]", g_device_info.stored_total_note_num);
							
							    Update_device_info((uint8_t *)&g_device_info, sizeof(st_device_data_storage));						       
									ble_ops.Write( BLE_CMD_SYNC_IS_END , &end_content, 1);  //返回  0

						}									
                
            }
            else
            {
                send_error_msg(ERROR_OTA_STATUS);
            }
        }break;
        case BLE_CMD_SYNC_ONCE_STOP:
        {
            if(DEVICE_SYNC_MODE == g_main_status.device_state)
            {
                //the content do not upload done, stop the upload
            }
            else
            {
                send_error_msg(ERROR_OTA_STATUS);
            }
        }break;
    }
	
	
}
