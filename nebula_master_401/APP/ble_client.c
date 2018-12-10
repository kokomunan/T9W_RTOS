
#include "ble_client.h"
#include "Status_Machin.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "config.h"
//#include "RGB_LED.h"
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
#include "dfu.h"
#include "misc.h"
#include "page_detect.h"
#define DEVICE_NAME_FLAG            0x55


extern stMainStatus  g_main_status;
extern st_device_data_storage g_device_info;
extern BLE518XX_OPS  ble_ops;  //����ģ������豸
extern dfu_info_st   dfu_info;

extern st_note_header_info note_header;     //�ʼ�ͷ
extern st_note_header_info offline_note_header;     //�ʼ�ͷ
extern uint32_t note_store_start_sector ; //�洢��ʼ����
extern uint32_t note_upload_start_addr;     //�ϴ��ʼǵ���ʼ��ַ
extern uint32_t note_store_addr_offset ; //�ʼǴ洢���������ʼ��ƫ�Ƶ�ַ
extern uint32_t note_upload_len;          //�ϴ��ʼǵĳ���
extern uint8_t note_upload_flag;            //��ʼ�ϴ��ʼǵı�־
extern uint32_t new_note_of_start_sector; //���������õ�
extern uint16_t JEDI_VERSION;
extern uint8_t JEDI_SURPORT_CALIBRA_FLAG;
uint32_t fw_len;
uint32_t ota_buff_p;
uint8_t ota_step;
uint32_t ota_checksum=0;
uint8_t flow_number=0;
uint32_t ota_target_addr=0;
uint8_t ota_buffer[1024];
uint8_t mcu_need_ota=0;
uint8_t ble_need_ota=0;
st_jedi_fw_info fw_info;
 
 uint8_t nortify_enable=0;
void ota_init(void)
{
    fw_len=0;
    ota_step= OTA_DFU_INIT;
    ota_checksum=0;
    flow_number=0;
    ota_buff_p=0;
    mcu_need_ota=0;
    ble_need_ota=0;
    ota_target_addr=DFU_APP_BACK_ADR  ;
  
    
     
}


void ota_is_end(void)
{
    
       debug_log("recived len%d download [%d]",fw_len, (fw_len * 100)/dfu_info.dfu_len);
        if(fw_len >= dfu_info.dfu_len)
        {
            ota_step= OTA_GET_MCU_CHECKSUM;         
            ble_ops.Write(CMD_OTA_CHECKSUM,  NULL, 0);
     
        }
        else
        {
            ble_ops.Write(CMD_OTA_RAW_DATA, &flow_number, 1);

        }
        memset((uint8_t *)&ota_buffer[0],0, 0x400);
}



void send_error_msg(uint8_t error)
{
    ble_ops.Write(CMD_ERROR_MSG, &error, 1);
}

void host_client(uint8_t *p_event_data)
{
	
    uint8_t op_code = *((uint8_t *)p_event_data + 1);
    uint8_t len = *(uint8_t *)((uint8_t *)p_event_data + 2);
    uint8_t *payload = ((uint8_t *)p_event_data + 3);
	
    switch(op_code)
    {
        case CMD_STATUS:
        {
            debug_log("status check\n");
        }break;

        case CMD_RTC_SET:
        {
            
            st_RTC_info rtc_data;
            memcpy((uint8_t *)&rtc_data, payload, sizeof(st_RTC_info));
            debug_log("rtc set[%d][%d][%d][%d][%d]\n", rtc_data.note_time_year,
                                                       rtc_data.note_time_month,
                                                       rtc_data.note_time_day,
                                                       rtc_data.note_time_hour,
                                                       rtc_data.note_time_min);
               
            write_rtc_data(&rtc_data);
        }break;
        case CMD_GET_VERSION  :
            
         ENABLE_NORTIFY;//���ô��ϱ�֪ͨ
            
        break;
        case  CMD_DISPALY_PAGE:     //����host �˷�����ʾ������ʾ��Ӧ��ҳ��  ���һ��ʼ�����������͵�slaveʧ�ܻᵼ��ҳ��һ��ʼ����ʾ ��������Ҫ�޲�
        {
            if(DEVICE_ACTIVE == g_main_status.device_state) 
            {
                if(len==2)
                {
                    debug_log("page change");         
//                    update_dis_online_page(payload[0],payload[1]);
                }	
            }
								
        }break;
        
   
         case CMD_SYNC_MODE_ENTER:
        {
            debug_log("enter sync mode\n");
            if(DEVICE_ACTIVE == g_main_status.device_state)        
            {
                g_main_status.device_state = DEVICE_SYNC_MODE;  
                TOUCH_PAD_OFF() ;//�ر�iic ��Դ
                memset(&offline_note_header,0,sizeof(st_note_header_info));
                flash_wake_up();
                
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
        }break;
        case CMD_SYNC_MODE_QUIT:
        {
            if(DEVICE_SYNC_MODE == g_main_status.device_state)
            {
                debug_log("Quit sync!");
                note_upload_flag = 0;
                g_main_status.device_state = DEVICE_ACTIVE;
                TOUCH_PAD_ON();
                init_offline_store_data();
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
        }break;
        case CMD_SYNC_FIRST_HEADER:  //��ʼ����һ���ʼǵĵ�һ���ʼ�ͷ
        {
            if(DEVICE_SYNC_MODE == g_main_status.device_state)
            {     
                //report note header
                if(g_device_info.stored_total_note_num)
                {
                    debug_log("search from[%d]to[%d]", g_device_info.note_read_start_sector, g_device_info.note_read_end_sector);
                    if(search_note_head(g_device_info.note_read_start_sector)==0)   //�ӻ��ε�falsh���ҵ���һ���ʼ�ͷ �����������ʼ��������ַ
                    {

                        note_upload_start_addr = offline_note_header.note_start_sector * FLASH_SECTOR_SIZE + 256;  //�ϴ����ݵ���ʼ��ַ
                        // debug_log("note data start addr[%x]",note_upload_start_addr);
                        note_store_addr_offset = 0;
                        note_upload_flag = 0;
                        note_upload_len=0;
                        ble_ops.Write(CMD_SYNC_FIRST_HEADER, (uint8_t *)&offline_note_header, sizeof(st_note_header_info));
                        debug_log("find a note block[%d][%d]", offline_note_header.note_number, offline_note_header.note_len);


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
                send_error_msg(ERROR_STATUS);
            }
        }break;
        case CMD_SYNC_ONCE_START:
        {
            if(DEVICE_SYNC_MODE == g_main_status.device_state)
            {
                  debug_log("sync start request");
                if(note_upload_start_addr)
                {
                    //start to report raw data
                    debug_log("sync start!");
                    note_upload_flag = 1;
                    ble_ops.Write(CMD_SYNC_ONCE_CONTENT, NULL, 0);
					debug_log("notify the host!");
                }
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
        }break;
        case  CMD_SYNC_IS_END :   //�����ͬ�ʼ�ID�Ŀ鷵������ʼ���һ�����ͷ�� ������������� �ͽ�֮ǰ����ʼ�ID���е�ͷ������� Ȼ�󷵻�2˵������ʼǴ����������һ���ʼ���
        {
					

					
            if((DEVICE_SYNC_MODE == g_main_status.device_state) && (0 == note_upload_flag))
            {
			    
                uint32_t sec_offset = (offline_note_header.note_len+256) / FLASH_SECTOR_SIZE;	
                uint32_t if_has_more=0;	
                uint8_t end_content = 0x02;
                if(((offline_note_header.note_len+256) % FLASH_SECTOR_SIZE)!=0)
                {
                    if_has_more=1;
                
                }
                else
                {
                    if_has_more=0;
                }
            
                
                debug_log("more%d offset %d",if_has_more,sec_offset);                    
                g_device_info.note_read_start_sector =get_next_sector(g_device_info.note_read_start_sector,if_has_more+ sec_offset);	//�õ���һ��������		
                if(g_device_info.stored_total_note_num>0)
                {
                    g_device_info.stored_total_note_num -= 1;	
                }                    
                note_store_addr_offset = 0;
                note_upload_len=0;
                note_upload_start_addr = 0;
                memset((uint8_t *)&offline_note_header, 0, sizeof(st_note_header_info));  										
                debug_log("remove a note, total note[%d]", g_device_info.stored_total_note_num);
                debug_log("start sector [%d]",g_device_info.note_read_start_sector);	
                Update_device_info((uint8_t *)&g_device_info, sizeof(st_device_data_storage));						       
                ble_ops.Write( CMD_SYNC_IS_END , &end_content, 1);  //����  0					
                
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
        }break;
        case CMD_SYNC_ONCE_STOP:
        {
            if(DEVICE_SYNC_MODE == g_main_status.device_state)
            {
                //the content do not upload done, stop the upload
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
        }break;
        
        
        case CMD_ENTER_OTA_MODE:
        {   
            
            if((DEVICE_ACTIVE == g_main_status.device_state)&&(Is_battery_is_safe()))
            {
                uint16_t ble_version,mcu_version;
                g_main_status.device_state =DEVICE_OTA_MODE;  
                debug_log("enter OTA mode\n");
                ota_init();  //��ʼ��       
                mcu_version=payload[2]<<8|payload[3];
                ble_version=payload[0]<<8|payload[1];
                 debug_log("new fw ble%d mcu%d",ble_version,mcu_version);
                if(g_device_info.ble_firmware_version<ble_version) 
                {
                     
                    ble_need_ota=1;            
                    debug_log("prepare update ble");	                        

                }
                if(SW_VERSION<mcu_version) 
                {
                   
                     mcu_need_ota=1;
                     debug_log("prepare update mcu");	 
                }
                if(ble_need_ota)
                {
                    
                     ota_step= OTA_WAIT_BLE;          
                     debug_log("make ble enter ota mode");	                     
                     ble_ops.Write(ENTER_OTA_MODE, NULL, 0);   
           
                    
                }
                else if(mcu_need_ota)
                {
                    
                     uint8_t fw_num=NODE_MCU_FW_NUM;
                     ota_step= OTA_GET_MCU_LEN;     
                     debug_log("require mcu fw info");	                    
                     ble_ops.Write(CMD_OTA_FILE_INFO, &fw_num, 1);   
                    
                }
                else
                {
                    send_error_msg(ERROR_VERSION); 
                    
                }                                
               
//                LEDStart(UI_OTA);
                TOUCH_PAD_OFF()  ;               
               
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
            
     
            
        }break;
               
        case CMD_OTA_FILE_INFO:
        {
            
            if((DEVICE_OTA_MODE == g_main_status.device_state)
                &&(OTA_GET_MCU_LEN==ota_step))
            { 
                   
                memcpy(&dfu_info.dfu_len,payload,4);   //��ȡҪ���صĳ��� ��ʼ����źͽ��ܳ��ȼ���
                debug_log("fw len is %d for mcu ",dfu_info.dfu_len);
                STMFLASH_Erase_bank1();                     // ������������MCU�ı�����   
                debug_log("start download fw");
                ble_ops.Write (CMD_OTA_RAW_DATA,&flow_number, 1);  //������������          
                ota_step=OTA_DOWNLOAD_MCU_FW;
                
                
            }          
            else
            {
                send_error_msg(ERROR_STATUS); 
            }
    
            
        }break;
        case CMD_OTA_RAW_DATA :
        {
         
            if((DEVICE_OTA_MODE == g_main_status.device_state) 
                && (OTA_DOWNLOAD_MCU_FW == ota_step))
            {
                uint8_t flow_num = payload[0];
                uint8_t* raw_data = &payload[1];
                uint8_t raw_len = 0;
                raw_len = len - 1;
                
               // debug_log("flow number%d ",flow_num); 
                if(flow_num != flow_number)
                {
                    debug_log("flow number error[%d][%d]",flow_num, flow_number);
                    send_error_msg(ERROR_FLOW_NUM);
                    ble_ops.Write(CMD_OTA_RAW_DATA, &flow_number, 1);
                    break;
                }
              
                push_data_to_sum(&ota_checksum,raw_data,raw_len);  //У��
                if(raw_len < MAX_OTA_PAYLOAD_LEN)
                {
                  
                    //���һ������
                    debug_log("raw len%d ",raw_len); 
                    if((ota_buff_p + raw_len) < OTA_RCV_MAX_BUFF_LEN)  //����16�ֽ�  ����1k
                    {
                        memcpy((uint8_t *)&ota_buffer[ota_buff_p], raw_data, raw_len);
                        ota_buff_p += raw_len;  //ƫ���ۼ�
                        fw_len += raw_len;  //�ۼ��ܳ�                      
                        STMFLASH_Write(ota_target_addr,(uint8_t *)ota_buffer,ota_buff_p);
                        ota_is_end();
                    }
                    else
                    {
                        if((ota_buff_p + raw_len) == OTA_RCV_MAX_BUFF_LEN)
                        {
                            memcpy((uint8_t *)&ota_buffer[ota_buff_p], raw_data, raw_len);
                            fw_len += raw_len;
                            ota_buff_p += raw_len;
                            ota_buff_p = 0;
                            STMFLASH_Write(ota_target_addr,(uint8_t *)ota_buffer,1024); //��ҪԤ�㴦������
                            ota_is_end();
                        }
               
                    }
                }
                else
                {
                   
                    flow_number++;                
                    if((ota_buff_p + MAX_OTA_PAYLOAD_LEN) < OTA_RCV_MAX_BUFF_LEN)   //���������1kʱ
                    {
                        
                        memcpy((uint8_t *)&ota_buffer[ota_buff_p], raw_data, MAX_OTA_PAYLOAD_LEN);
                        ota_buff_p+= MAX_OTA_PAYLOAD_LEN; //�ƶ�ƫ��
                        fw_len += MAX_OTA_PAYLOAD_LEN;   //�ۼ��ܳ� 
                        ble_ops.Write(CMD_OTA_RAW_DATA, &(flow_number), 1); //������һ����������
                      
                    }
                    else
                    {
                        if((ota_buff_p+ MAX_OTA_PAYLOAD_LEN) == OTA_RCV_MAX_BUFF_LEN) //������1k
                        {
                            memcpy((uint8_t *)&ota_buffer[ota_buff_p], raw_data, MAX_OTA_PAYLOAD_LEN);
                            STMFLASH_Write(ota_target_addr,(uint8_t *)ota_buffer,1024);                           
                            ota_target_addr += 1024;
                            ota_buff_p = 0;
                            fw_len += MAX_OTA_PAYLOAD_LEN;
                            ota_is_end();
                        }
               
                    }
                }
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
            
            
            
        }break;
        
        case CMD_OTA_CHECKSUM:
        {
            
             if((DEVICE_OTA_MODE == g_main_status.device_state) && (OTA_GET_MCU_CHECKSUM == ota_step))
            {
                uint32_t host_checksum = 0;
                uint8_t result = 1;
                memcpy((uint8_t *)&host_checksum, payload, 4);
                debug_log("checksum[%d]-[%d]", host_checksum, ota_checksum);
                if(host_checksum != ota_checksum)
                {
                    uint8_t fw_num=NODE_MCU_FW_NUM;
                    ble_ops.Write(CMD_OTA_RESULT, &result, 1);//�˶�У���� ���������host
                    fw_len=0; 
                    ota_checksum=0;
                    flow_number=0;
                    ota_buff_p=0;
                    ota_target_addr=DFU_APP_BACK_ADR  ;                 
                    ota_step= OTA_GET_MCU_LEN;                   
                    ble_ops.Write(CMD_OTA_FILE_INFO, &fw_num, 1);   
                                                      
            
                }
                else
                {
                    result = 0;
                    debug_log("checksum ok")    
                    ble_ops.Write(CMD_OTA_RESULT, &result, 1);//�˶�У���� ���������host
                    ota_step= OTA_FINISH;	
                    dfu_info.dfu_update=1;                   
                    Dfu_Info_update(&dfu_info);                            
                   
                }                      
               
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
                               
            
        }break;
        
        
        case  CMD_OTA_SWTICH:
            
           if(DEVICE_OTA_MODE == g_main_status.device_state)
            {
                
                if(OTA_FINISH == ota_step)
                {
                    ble_ops.Write( CMD_OTA_SWTICH ,NULL, 0);  
                    debug_log("reset")  ;  
                    g_device_info.auto_poweron=1;   //�����Զ�����
                    Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage));  
                    send_ble_data_buffer_to_host();
                    osDelay(200);
                    debug_log("request switch") ;     
                    Dispalay_deinit(); 
                    server_stop();
                    NVIC_SystemReset();   
                }
                else if( OTA_WAIT_BLE== ota_step)
                {
                    
                    if(mcu_need_ota)
                    {
                        ota_step=OTA_GET_MCU_LEN;
                        uint8_t fw_num=  NODE_MCU_FW_NUM;      
                        debug_log("start download mcu fw")   ;     
                        ble_ops.Write(CMD_OTA_FILE_INFO ,&fw_num, 1);
                    }
                    else
                    {
                        ble_ops.Write( CMD_OTA_SWTICH ,NULL, 0);  
                        debug_log("reset");    
                        g_device_info.auto_poweron=1;   //�����Զ�����
                        Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage));  
                        send_ble_data_buffer_to_host();  
                        osDelay(200);     
                        debug_log("request switch") ; 
                        Dispalay_deinit(); 
                        server_stop();
                        NVIC_SystemReset();    
                    }
          
                
                }
                else
                {
                    send_error_msg(ERROR_STATUS);
                }
                
        
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
            
        
        break;

        
        case  CMD_OTA_QUIT:
        {
            if(DEVICE_OTA_MODE == g_main_status.device_state)
            {
                debug_log("Quit OTA!");
                if( ble_need_ota)
                {
                     ble_ops.Write(QUIT_OTA_MODE, NULL, 0);   //֪ͨ�����˳�
                    
                }
                g_main_status.device_state = DEVICE_ACTIVE;        
                TOUCH_PAD_ON();    
             
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
            
        }break;
        
        case CMD_REQUST_PAGE:
            
           if(DEVICE_ACTIVE == g_main_status.device_state)
            {
//                uint16_t page_value;
                debug_log("request page info");
//                page_value= read_page_value();
 //               ble_ops.Write(CMD_REQUST_PAGE, (uint8_t *)&page_value, 2);  
             
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
        
        
        
        break;
                     
        case  CMD_ENTER_UPDATA_EMR_MODE:
             if((DEVICE_ACTIVE == g_main_status.device_state)&&(Is_battery_is_safe()))
            {
                g_main_status.device_state = SENSOR_UPDATE;    
                /*ʹ�����ַ�����Ϊ�˱���ģ���������ģʽ������
                ����������IO��ʼ��������쳣�жϣ������ڽ���ģʽ
                ֮ǰ�Ȱ��жϴ���رգ�Ȼ���ټ���ж����Ŷ�������
                �����ڴ��жϴ���*/
                set_i2c_int(0);
                Write_EMR(CMD_ENTER_UPDATA_EMR_MODE-0x20, NULL, 0);              
                osDelay(100);
                if(EMR_INT)
                {
                    read_position();
                    
                }
                set_i2c_int(1);
                flow_number=0;
                ota_checksum=0;
                clear_jedi_info();
                debug_log("jedi dfu mode");
                
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
            
        break;
        
        case  CMD_OTA_EMR_FILE_INFO:
            if(SENSOR_UPDATE == g_main_status.device_state)
            {
                                         
                memcpy((uint8_t *)&fw_info, payload, sizeof(st_jedi_fw_info));  
                debug_log("ver:[%x]lenth[%d]", fw_info.sw_version, fw_info.img_length);              
                Write_EMR(CMD_OTA_EMR_FILE_INFO-0x20, payload, 8);   
    
                         
                
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
            
        break;
                
        case  CMD_OTA_EMR_RAW_DATA:
             if(SENSOR_UPDATE == g_main_status.device_state)
            {
                uint8_t flow_num = payload[0];
                uint8_t* raw_data = &payload[1];
                uint8_t raw_len = 0;
                raw_len = len - 1;
                if(flow_num != flow_number)
                {
                    debug_log("flow number error[%d][%d]",flow_num, flow_number);
                    send_error_msg(ERROR_STATUS);
                    ble_ops.Write(CMD_OTA_EMR_RAW_DATA, &flow_number, 1);
                    break;
                }
                 push_data_to_sum(&ota_checksum,raw_data,raw_len);  //����У��ֻ�Ƿ�����Կ���HSOT����������û�ж�ʧ
                 flow_number++;   //ӦΪģ�鲢û�ж���Ž����ۼ� �������豸��Ҫ�����ۼƣ�����������������͵ĵط��������
                 Write_EMR(CMD_OTA_EMR_RAW_DATA-0x20, raw_data, raw_len );
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
            
        break;
                        
        case  CMD_OTA_EMR_CHECKSUM:
            if(SENSOR_UPDATE == g_main_status.device_state)
            {
//                uint8_t result = 0;
                               
                uint32_t host_checksum = 0;
                

                memcpy((uint8_t *)&host_checksum, payload, 4);
                
                 debug_log("host check sum %x local check sum %x",host_checksum,ota_checksum);
                 set_i2c_int(0);
                 Write_EMR(CMD_OTA_EMR_CHECKSUM-0x20, payload, len);
                osDelay(100);
                if(EMR_INT)
                {
                    read_position();
                    
                }  
                set_i2c_int(1);
                if(IS_emr_update_sucess())
                {
                     debug_log("jedi update sucess");
                     update_jedi_info();
                    
                }
                              
            }
            else
            {
                send_error_msg(ERROR_STATUS);
            }
            
        break;
        case CMD_OTA_EMR_QUIT:
            
            g_main_status.device_state = DEVICE_ACTIVE;     
            debug_log("quit jedi update");
        
        break;
        
        case  CMD_GET_PAD_VERSION :
        {
            
            
            st_jedi_info   jedi_info;
            jedi_info.jedi_version=JEDI_VERSION;
            jedi_info.surport_calibra_flag=JEDI_SURPORT_CALIBRA_FLAG;    
            debug_log("jedi info: version %d calibra %d",JEDI_VERSION,JEDI_SURPORT_CALIBRA_FLAG);            
            ble_ops.Write(CMD_GET_PAD_VERSION , (uint8_t *)&jedi_info, 3);
        }
        
       break;   
        case CMD_ENTER_FRQ_ADJUST:
        {
            
            Write_EMR(CMD_ENTER_FRQ_ADJUST, NULL, 0);
        
                      
            
        }break;
 
        
    }
	
	
}
