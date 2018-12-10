#include "usb_client.h"
#include "EXT_RTC.h"
#include "General_Emr.h"
#include "battery.h"
#include "debug_log.h"
#include "display.h"
#include "input_event.h"
#include "norflash.h"
#include "note_manager.h"
#include "Status_Machin.h"
#include "power_manager.h"
#include  "device_info.h"
#include "server.h"
#include "nrf51822.h"
#include "debug_log.h"
#include "dfu.h"
#include "usb_controler.h"
#include "misc.h"
#include "RGB_LED.h"
#include "em9203.h"
#include "ble_client.h"




extern st_device_data_storage g_device_info;
extern st_note_header_info note_header;     //�ʼ�ͷ
extern st_note_header_info offline_note_header;     //�ʼ�ͷ
extern uint32_t note_store_start_sector ; //�洢��ʼ����
extern uint32_t note_upload_start_addr;     //�ϴ��ʼǵ���ʼ��ַ
extern uint32_t note_store_addr_offset ; //�ʼǴ洢���������ʼ��ƫ�Ƶ�ַ
extern uint32_t note_upload_len;          //�ϴ��ʼǵĳ���
extern uint8_t note_upload_flag;            //��ʼ�ϴ��ʼǵı�־
extern uint32_t new_note_of_start_sector; //���������õ�
extern st_device_data_storage g_device_info;
extern dfu_info_st   dfu_info;
extern stMainStatus  g_main_status;
extern uint32_t ota_checksum;
extern uint8_t flow_number;
extern st_jedi_fw_info fw_info;
extern uint16_t JEDI_VERSION;
extern uint8_t JEDI_SURPORT_CALIBRA_FLAG;
extern uint8_t out_really_value;
uint8_t ble_need_update;
uint8_t mcu_need_update;
uint8_t dfu_step;
uint32_t dfu_target_addr=0;
uint32_t download_len=0;
uint32_t dfu_total_len=0;
uint32_t dfu_checksum=0;
uint8_t dfu_folow_num=0;


void mcu_dfu_init(void)
{
    ble_need_update=0;
    mcu_need_update=0;
    dfu_step=0;
    download_len=0;
    dfu_checksum=0;
    dfu_target_addr=DFU_APP_BACK_ADR;
    dfu_folow_num=0;    
    dfu_total_len=0;
}

void usb_host_report_error(uint8_t code)
{
    uint8_t value=code;    
    usb_write ( USB_CMD_REPORT_ERROR ,&value, 1);    
       
}

void usb_client(uint8_t *p_event_data)
{
    

      stUSB_PACKET client_msg;
      memcpy((uint8_t *)&client_msg, (uint8_t *)&usb_client_rx_buff, sizeof(stUSB_PACKET));  
      uint8_t op_code=client_msg.opcode;
      uint8_t len =client_msg.length;    
      uint8_t *payload =client_msg.payload;
      uint8_t device_sta=Get_Device_Status();
      switch(op_code)
      {
            case USB_CMD_GET_STATUS:                     
            {              
                                                            
                uint8_t temp[3];
                temp[0] =Get_Device_Status();
                temp[1] =get_battery_value();
                temp[2] = (uint8_t)g_device_info.stored_total_note_num;                                                       
                usb_write (USB_CMD_GET_STATUS,temp, 3);           
                debug_log("get status%d",device_sta);	
            }	
            break;
           
            case USB_CMD_GET_DEVICE_INFO:
            {
                uint8_t res; 
                stUSB_DEVICE_BASIC_INFO   dev_info;
                Open_ble();
                res= Ioctl_ble(INIT,NULL); 
                if(res)
                {
                    debug_log("no response");	 
                    
                }
                dev_info.mcu_version=SW_VERSION;
                dev_info.ble_version=g_device_info.ble_firmware_version;   
                dev_info.custom_num=g_device_info.node_info.customer_number;
                dev_info.class_num=g_device_info.node_info.class_number;
                dev_info.device_num=g_device_info.node_info.device_number;            
                memcpy(dev_info.mac,g_device_info.ble_addr,6);
                dev_info.hard_num=g_device_info.hard_version;
                
                usb_write (USB_CMD_GET_DEVICE_INFO,(uint8_t *)&dev_info, sizeof(  stUSB_DEVICE_BASIC_INFO));                                
                //Close_ble();        
            
            
            }
            break;          
            case USB_CMD_CONFIG_ADDR:  
            {   
                uint8_t res;
                uint8_t data[4];               
                debug_log("set custom%d class%d device_num%d ",payload[0],payload[1],payload[2]);	                   
                memcpy(&g_device_info.node_info,payload,3);
                res=Update_device_info((uint8_t *)&g_device_info, sizeof(st_device_data_storage));
                data[0]=res;
                data[1]=g_device_info.node_info.customer_number;
                data[2]=g_device_info.node_info.class_number;
                data[3]=g_device_info.node_info.device_number;
                usb_write (USB_CMD_CONFIG_ADDR,data, 4);
               
            } 
            break;
            
            
            case USB_CMD_RESET_DEFAULT:
            {
                uint8_t res;
                g_device_info.stored_total_note_num = 0;
                g_device_info.note_read_start_sector = 0;
                g_device_info.note_read_end_sector = 0;
                g_device_info.hard_version=HW_VERSION;
                res=Update_device_info((uint8_t *)&g_device_info, sizeof(st_device_data_storage));
                usb_write (USB_CMD_RESET_DEFAULT,&res, 1);
                debug_log("reset device info");
            }
            break;
            
            case USB_CMD_RESET_BLE_DEFAULT:
            {
                uint8_t res; 
                Open_ble();
                res= Ioctl_ble(SET_PARA,NULL); 
                if(res)
                {
                    debug_log("no response");	 
                    
                }  
               // Close_ble();   
                usb_write (USB_CMD_RESET_BLE_DEFAULT,&res, 1);          

            }                                   
            break;
            
            case USB_CMD_OLED_TEST_ENTER:
                out_really_value=1;             
                change_screen(TEST_SCREEN);
                                
            break;
            case USB_CMD_OLED_FULL:
                
                update_oled_test_mode(OLED_ON); 
            
            break;
            
            case USB_CMD_OLED_BLANK:
                
                update_oled_test_mode(OLED_OFF);  
            
            break;
            
            
            case  USB_CMD_OLED_TOGLE:
            {  
                static uint8_t count=0;
                
                if(count==0)
                {			

                    update_oled_test_mode(OLED_HALF_TOP); 
                    count=1;	
                }
                else
                {

                    update_oled_test_mode(OLED_HALF_BOTTOM);
                    count=0;	
                }

            }                
                
            break;
            
            case USB_CMD_OLED_TEST_QIUT:
                
            change_screen(MAIN_SCREEN);
            break;
            case USB_CMD_ENTER_DFU:               
            {
                uint8_t fw_num;
                uint16_t ble_version,mcu_version;
                
                if(Get_Device_Status()!= DEVICE_OTA_MODE)//���������OTA����ͬ���������Խ���DFUģʽ
                {
                    debug_log("enter dfu mode");	           
                    mcu_version=payload[0]<<8|payload[1];
                    ble_version=payload[2]<<8|payload[3];
                    Set_Device_Status(DEVICE_DFU_MODE);//����״̬ΪDFUģʽ                   
                    Open_ble(); //������������DFUģʽ����ȡ�汾��
                    TOUCH_PAD_OFF() ;
                    mcu_dfu_init();
                    Ioctl_ble(INIT,NULL);
                    debug_log("get version is %d of ble",g_device_info.ble_firmware_version);	
                    if(g_device_info.ble_firmware_version<ble_version) //�Ƚϵ�ǰ�������汾
                    {
                        ble_need_update=1;        
                        debug_log("prepare update ble");	                        
                        
                    }
                    if(SW_VERSION<mcu_version) //�������MCU��Ҫ����  ��������MCU�̼�
                    {
                       mcu_need_update=1;                                  
                        debug_log("prepare update mcu");	 
                    }
                    if(ble_need_update) 
                    {
                       HAL_Delay(100);        //��BLE�л��ɽ���׼��ʱ��                      
                       send_fw_packet_to_ble(ENTER_DFU_MODE,NULL,0);
                       debug_log("get len of ble fw");
                    }
                    else if(mcu_need_update)
                    {
                       dfu_step= GET_MCU_FW_INFO;
                       fw_num=NODE_MCU_FW_NUM;
                       usb_write (USB_CMD_GET_FW_INFO,&fw_num, 1); 
                     //  HAL_Delay(10);
                       debug_log("get len of mcu ");
                    }                                       
                    else//����������Ҫ���� ����������ʾ
                    {
                        dfu_step= FINISH_DFU ;                                 
                        usb_host_report_error(ERROR_VERSION);        
                        debug_log("version is the newest");	
                    }
                    
                                            
                }
                else
                {                        
                    
                     usb_host_report_error(ERROR_STATUS);  
                }
                                                  
                
            }     
            break;
            case USB_CMD_GET_FW_INFO:
                 if(Get_Device_Status()==DEVICE_DFU_MODE)  //���״̬�ǲ���DFUģʽ������Ϊ��ȡ����
                 {
                     
                    if(dfu_step== GET_BLE_FW_INFO)     //�����������������
                    {
                         
                         send_fw_packet_to_ble(GET_HW_LEN,payload,len); //���͸�����ģ��
                         dfu_step= DOWNLOAD_BLE_FW;    //���ò���Ϊ������
                    }  
                    else if(dfu_step== GET_MCU_FW_INFO)
                    {
                         memcpy(&download_len,payload,4);   //��ȡҪ���صĳ��� ��ʼ����źͽ��ܳ��ȼ���
                         debug_log("fw len is %d for mcu ",download_len);
                         STMFLASH_Erase_bank1();                     // ������������MCU�ı�����   
                         debug_log("start download fw");
                         usb_write (USB_CMD_GET_RAW_DATA,&dfu_folow_num, 1);  //������������
                        // HAL_Delay(10);   
                         dfu_step= DOWNLOAD_MCU_FW;    //���ò���Ϊ������
                    }
                    else
                    {
                         usb_host_report_error(ERROR_STATUS);  
                    }
               
                     
                 }
                 else
                 {
                     usb_host_report_error(ERROR_STATUS);  
                 }
                      
            break;
            
            case USB_CMD_GET_RAW_DATA:
              if(Get_Device_Status()==DEVICE_DFU_MODE) 
              {
                  
                    if(dfu_step== DOWNLOAD_BLE_FW)     
                    {
                         send_fw_packet_to_ble(GET_HW_RAW,payload,len); 
                         HAL_Delay(10);   
                    }  
                    else if(dfu_step==DOWNLOAD_MCU_FW)
                    {
                        uint8_t *raw_data;
                        uint8_t folow_num=payload[0];
                        uint8_t raw_len = 0;
                        raw_len = len - 1;
                        raw_data=&payload[1];
                        if(folow_num!=dfu_folow_num)
                        {
                                                           
                             usb_host_report_error(ERROR_FLOW_NUM);  
                             HAL_Delay(10);
                             usb_write (USB_CMD_GET_RAW_DATA,&dfu_folow_num, 1);  //������������
                            
                        }
                        else
                        {
                            dfu_folow_num++;  
                            
                            
                             push_data_to_sum(&dfu_checksum,raw_data,raw_len);  //У��
                            
                            if(download_len>dfu_total_len)
                            {
                                
                                STMFLASH_Write(dfu_target_addr,raw_data, raw_len);                               
                                dfu_target_addr+=raw_len;
                                dfu_total_len+=raw_len;
                                usb_write (USB_CMD_GET_RAW_DATA,&dfu_folow_num, 1);  //������������
                                
                            }
                            else  //�������
                            {
                                
                                usb_write (USB_CMD_GET_CHEKSUM,&dfu_folow_num, 1);  //������������ 
                                
                                dfu_step= GET_MCU_FW_CHECK;    //���ò���Ϊ������ 
                            }
                            
                            
                        }
                                                                                
                        
                    }
                    else
                    {
                        usb_host_report_error(ERROR_STATUS);  
                    }
                  
                }
                else
                {
                    usb_host_report_error(ERROR_STATUS);  
                }
                
            break;
            
            case USB_CMD_GET_CHEKSUM:
              if(Get_Device_Status()==DEVICE_DFU_MODE) 
              {
                     uint32_t host_checksum = 0;   
                     memcpy((uint8_t *)&host_checksum, payload, 4);
                    if(dfu_step== GET_BLE_FW_CHECK)     
                    {
                        send_fw_packet_to_ble(GET_HW_CHECK,(uint8_t *)&host_checksum, 4);  
                       
                    }  
                    else if(dfu_step==GET_MCU_FW_CHECK)
                    {
                        
                        uint8_t result;
                        uint8_t fw_num;
                      
                        if(host_checksum != dfu_checksum)
                        {
                            result=1;
                            usb_write (  USB_CMD_REPORT_RESULT ,&result, 1);      
                            HAL_Delay(10);                  
                            dfu_step= GET_MCU_FW_INFO;//���� step ����mcu_fw
                            fw_num=NODE_MCU_FW_NUM;
                            usb_write (USB_CMD_GET_FW_INFO,&fw_num, 1);
                     
                        }
                        else
                        {
                   
                            result=0;
                            usb_write (  USB_CMD_REPORT_RESULT ,&result, 1);  
                            dfu_info.dfu_update=1;          //�����Ҫ���¹̼�  
                            dfu_info.dfu_len= download_len;   
                            Dfu_Info_update(&dfu_info);     //����OTA��Ϣ   
                            HAL_Delay(10);  
                            usb_write (USB_CMD_NOTIFY_RESET ,NULL, 0);  
                            g_device_info.auto_poweron=1;
                            Update_device_info((uint8_t *)&g_device_info,sizeof(st_device_data_storage));
                            Dispalay_deinit(); 
                            server_stop();
                            NVIC_SystemReset();  //�����豸                                                                                

                        }

                                           
                        
                    }
                    else
                    {
                        usb_host_report_error(ERROR_STATUS);  
                    }
                  
                  
              }
              else
              {
                  usb_host_report_error(ERROR_STATUS);  
              }
 
            break;
                
            
            case USB_CMD_QUIT_DFU:          
            if(Get_Device_Status()==DEVICE_DFU_MODE) 
            {
                
                 Set_Device_Status(DEVICE_ACTIVE);//����״̬ΪDFUģʽ
                 //Close_ble(); 
     
                 TOUCH_PAD_ON();
                
            }
            else
            {
                usb_host_report_error(ERROR_STATUS);  
            }   
            break;
   
                       
            case USB_CMD_RTC_SET:
            {
                
                uint8_t result;   
                st_RTC_info rtc_data;  
                if(DEVICE_ACTIVE ==  Get_Device_Status()) 
                {                
                    memcpy((uint8_t *)&rtc_data,payload, sizeof(st_RTC_info));
                    debug_log("rtc set[%d][%d][%d][%d][%d]", rtc_data.note_time_year,
                                                           rtc_data.note_time_month,
                                                           rtc_data.note_time_day,
                                                           rtc_data.note_time_hour,
                                                           rtc_data.note_time_min);
                    result=0;  
                    write_rtc_data(&rtc_data);
                    usb_write (USB_CMD_RTC_SET,&result, 1);
                }
                else
                {    
                    usb_host_report_error(ERROR_STATUS);  
                }
            }
            break;
          
          
          
            case USB_CMD_SET_NAME:
                if(DEVICE_ACTIVE ==  Get_Device_Status()) 
                {
                    debug_log("set name ");
                }
                else
                {
                    usb_host_report_error(ERROR_STATUS);    
                }
            break;

            case  USB_CMD_DISPALY_PAGE:     
            {
                if(DEVICE_ACTIVE ==  Get_Device_Status()) 
                {
                    if(len==2)
                    {
                             
//                        update_dis_online_page(payload[0],payload[1]);
                    }	
                }
                else
                {
                    usb_host_report_error(ERROR_STATUS);   
                }
								
            }break;
        
   
          case USB_CMD_SYNC_MODE_ENTER:
        {
            debug_log("enter sync mode");
            if(DEVICE_ACTIVE ==  Get_Device_Status()) 
            {
                Set_Device_Status(DEVICE_SYNC_MODE);
                TOUCH_PAD_OFF() ;//�ر�iic ��Դ
                memset(&offline_note_header,0,sizeof(st_note_header_info));
                flash_wake_up();
                
            }
            else
            {
                usb_host_report_error(ERROR_STATUS);     
            }
        }break;       
        case USB_CMD_SYNC_MODE_QUIT:
        {
            if(DEVICE_SYNC_MODE == Get_Device_Status())
            {
                debug_log("Quit sync!");
                note_upload_flag = 0;
                Set_Device_Status(DEVICE_ACTIVE);
                TOUCH_PAD_ON();
                init_offline_store_data();
            }
            else
            {
                usb_host_report_error(ERROR_STATUS);  
            }
        }break;
      
        case USB_CMD_SYNC_FIRST_HEADER:  //��ʼ����һ���ʼǵĵ�һ���ʼ�ͷ
        {
            if(DEVICE_SYNC_MODE == Get_Device_Status())
            {     
                //report note header
                if(g_device_info.stored_total_note_num)
                {
                    debug_log("search from[%d]to[%d]", g_device_info.note_read_start_sector, g_device_info.note_read_end_sector);
                    if(search_note_head(g_device_info.note_read_start_sector)==0)  
                    {

                        note_upload_start_addr = offline_note_header.note_start_sector * FLASH_SECTOR_SIZE + 256;  //�ϴ����ݵ���ʼ��ַ
                        // debug_log("note data start addr[%x]",note_upload_start_addr);
                        note_store_addr_offset = 0;
                        note_upload_flag = 0;
                        note_upload_len=0;
                        usb_write (USB_CMD_SYNC_FIRST_HEADER, (uint8_t *)&offline_note_header, sizeof(st_note_header_info));
                        debug_log("find new note num[%d][%d]", offline_note_header.note_number, offline_note_header.note_len);

                    }
																		
                }
                else
                {
            
                    usb_host_report_error(ERROR_NO_NOTE);  
                    
                }
            }
            else
            {

                usb_host_report_error(ERROR_STATUS);  
            }
        }break;
        case USB_CMD_SYNC_START:
        {
            if(DEVICE_SYNC_MODE == Get_Device_Status())
            {
                if(note_upload_start_addr)
                {
                 
                    debug_log("sync start!");
                    note_upload_flag = 1;

                }
            }
            else
            {
               usb_host_report_error(ERROR_STATUS); 
            }
        }break;
        
        case  USB_CMD_SYNC_IS_END :   //�����ͬ�ʼ�ID�Ŀ鷵������ʼ���һ�����ͷ�� ������������� �ͽ�֮ǰ����ʼ�ID���е�ͷ������� Ȼ�󷵻�2˵������ʼǴ����������һ���ʼ���
        {
             if((DEVICE_SYNC_MODE == Get_Device_Status()) && (0 == note_upload_flag))
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
                usb_write( USB_CMD_SYNC_IS_END , &end_content, 1);  //����  0
								
                
            }
            else
            {
                usb_host_report_error(ERROR_STATUS); 
            }

        }break;
        case USB_CMD_GET_PAD_VERSION:
        {
            st_jedi_info   jedi_info;
            jedi_info.jedi_version=JEDI_VERSION;
            jedi_info.surport_calibra_flag=JEDI_SURPORT_CALIBRA_FLAG;    
            debug_log("jedi info: version %d calibra %d",JEDI_VERSION,JEDI_SURPORT_CALIBRA_FLAG);            
            usb_write(USB_CMD_GET_PAD_VERSION , (uint8_t *)&jedi_info, sizeof(st_jedi_info)); 
            
            
        } 
        break;
        
        case USB_CMD_TEST_MODE_ENTER:
        {    
            uint8_t res;
            res=start_test_ble_net();
            if(res)
            {
            
                usb_host_report_error(ERROR_TEST_FAILED); //���ش���  
                qiut_test_ble_net();
                
            }
//            else
//            {
//                                
//               start_test_2g4_net();     
//               Set_Device_Status(DEVICE_NRF_TEST); 
//               change_screen(TEST_NRF_SCREEN); //�����������
//            }   
         
        
        }
        break;
        
        
        case  USB_CMD_TEST_MODE_QUIT:
                
          Set_Device_Status(DEVICE_ACTIVE);
          qiut_test_ble_net();
         // quit_test_2g4_net();
          change_screen(MAIN_SCREEN); //�����������
     
        break;
        
            
        case  USB_CMD_ENTER_UPDATA_EMR_MODE:
            if((DEVICE_ACTIVE == g_main_status.device_state)&&(Is_battery_is_safe()))
            {
                g_main_status.device_state = SENSOR_UPDATE;    
                /*ʹ�����ַ�����Ϊ�˱���ģ���������ģʽ������
                ����������IO��ʼ��������쳣�жϣ������ڽ���ģʽ
                ֮ǰ�Ȱ��жϴ���رգ�Ȼ���ټ���ж����Ŷ�������
                �����ڴ��жϴ���*/
                set_i2c_int(0);
                Write_EMR(CMD_ENTER_UPDATA_EMR_MODE-0x20, NULL, 0);              
                HAL_Delay(100);
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
                usb_host_report_error(ERROR_STATUS);
            }

        break;

        case USB_CMD_OTA_EMR_FILE_INFO:
            if((SENSOR_UPDATE == g_main_status.device_state)&&(Is_battery_is_safe()))
            {
                                     
            memcpy((uint8_t *)&fw_info, payload, sizeof(st_jedi_fw_info));  
            debug_log("ver:[%x]lenth[%d]", fw_info.sw_version, fw_info.img_length);              
            Write_EMR(CMD_OTA_EMR_FILE_INFO-0x20, payload, 8);   

                     

            }
            else
            {
                usb_host_report_error(ERROR_STATUS);
            }

        break;

        case  USB_CMD_OTA_EMR_RAW_DATA:
            if((SENSOR_UPDATE == g_main_status.device_state)&&(Is_battery_is_safe()))
            {
            uint8_t flow_num = payload[0];
            uint8_t* raw_data = &payload[1];
            uint8_t raw_len = 0;
            raw_len = len - 1;
            if(flow_num != flow_number)
            {
                debug_log("flow number error[%d][%d]",flow_num, flow_number);
                usb_host_report_error(ERROR_STATUS);
                usb_write(USB_CMD_OTA_EMR_RAW_DATA, &flow_number, 1);
                break;
            }
             push_data_to_sum(&ota_checksum,raw_data,raw_len);  //����У��ֻ�Ƿ�����Կ���HSOT����������û�ж�ʧ
             flow_number++;   //ӦΪģ�鲢û�ж���Ž����ۼ� �������豸��Ҫ�����ۼƣ�����������������͵ĵط��������
             Write_EMR(CMD_OTA_EMR_RAW_DATA-0x20, raw_data, raw_len );
            }
            else
            {
                usb_host_report_error(ERROR_STATUS);
            }

        break;
                
        case  USB_CMD_OTA_EMR_CHECKSUM:
            if((SENSOR_UPDATE == g_main_status.device_state)&&(Is_battery_is_safe()))
            {
            //                uint8_t result = 0;
                           
            uint32_t host_checksum = 0;


            memcpy((uint8_t *)&host_checksum, payload, 4);

             debug_log("host check sum %x local check sum %x",host_checksum,ota_checksum);
             set_i2c_int(0);
             Write_EMR(CMD_OTA_EMR_CHECKSUM-0x20, payload, len);
            HAL_Delay(100);
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
                usb_host_report_error(ERROR_STATUS);
            }

        break;
        case USB_CMD_OTA_EMR_QUIT:

            g_main_status.device_state = DEVICE_ACTIVE;     
            debug_log("quit jedi update");

            break;

     
        case USB_CMD_ENTER_FRQ_ADJUST:
            {

            Write_EMR(CMD_ENTER_FRQ_ADJUST, NULL, 0);

                  

            }break;
        
        
        
        default :  
               
        debug_log("undefine conmands");                           
          
          
      }
    
      
 
     
}
