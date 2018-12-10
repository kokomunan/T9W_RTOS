#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "note_manager.h"
#include "norflash.h"
#include "RGB_LED.h"
#include "stdint.h"
#include "config.h"
#include "debug_log.h"
#include "EXT_RTC.h"
#include "display.h"
#include "input_event.h"
#include "server.h"
#include "device_info.h"
#include "page_detect.h"
#include "gather_position.h"
#define   NOTE_debug_log       debug_log

extern st_device_data_storage g_device_info;
extern uint8_t Get_Device_Status(void);
extern uint32_t currunt_note_id;         //��ǰ�ʼǵ�����ID

st_note_header_info note_header;     //�ʼ�ͷ
st_note_header_info offline_note_header;
uint32_t note_store_start_sector = 0; //�洢��ʼ����
uint32_t note_upload_start_addr;     //�ϴ��ʼǵ���ʼ��ַ
uint32_t note_store_addr_offset = 0; //�ʼǴ洢���������ʼ��ƫ�Ƶ�ַ
uint32_t note_upload_len=0;          //�ϴ��ʼǵĳ���
uint8_t note_upload_flag;            //��ʼ�ϴ��ʼǵı�־
uint32_t new_note_of_start_sector=0; //���������õ�
uint8_t create_new_note_flag=0;      //�����±ʼǵı�־
uint32_t next_start_search_sector=0; //��һ������������
uint32_t to_find_id=0;               //�ҵ����±ʼǵ�id
uint8_t note_store_enable=0;         //�Ƿ��ʹ�����߱ʼ�
volatile uint8_t note_buffer[2][256]; //�������߱ʼ�ƹ��buff
volatile uint8_t note_buffer_num = 0; //����ƹ��buff��id
volatile uint16_t note_buffer_offset=0;//���ڴ������߱ʼ�ʱ��ƫ��
uint32_t note_len_total_test=0;
volatile uint8_t note_buffer_store_num=0xff;  //���ƹ��buff��id
volatile uint8_t note_store_leave_flag=0; //�뿪�ʼǵı�־�����������ݵķֶ�
volatile uint8_t note_store_leave_flag_count;
volatile uint8_t note_store_leave_report_flag;
uint8_t  new_note_flag=0;

uint32_t note_store_cnt=0;           //ϡ������������
uint8_t  first_boot=0;               //��һ��������־

uint32_t wrtie_fifo_addr;
uint32_t read_fifo_addr;
uint32_t fifo_len;
uint32_t fifo_total_len;
uint32_t fifo_currunt_setor;
uint32_t fifo_offset;
uint32_t store_pos_x=0,store_pos_y=0;
void set_first_boot_flag(uint8_t value)
{

    first_boot=value;
	
}

void Disbale_note_store(void)
{
	
	  note_store_enable=0;
	
}
void Enable_note_store(void)
{
	  note_store_enable=1;
}
uint8_t Is_enable_note_store(void)
{
	  return note_store_enable;
	
}
//��ȡʣ��ռ�
 uint32_t get_free_sector(void)
{
	if(g_device_info.note_read_end_sector>=g_device_info.note_read_start_sector)
	{
		return (FLASH_MAX_SECTOR_NUM-1)-(g_device_info.note_read_end_sector - g_device_info.note_read_start_sector);
	}
	else
	{
		return  g_device_info.note_read_start_sector-g_device_info.note_read_end_sector-1;
	}
	
}
//��ȡ��һ������
uint32_t get_next_sector(uint32_t currunt_sector,uint32_t offset)
{
//	if(currunt_sector>=(FLASH_MAX_SECTOR_NUM-1))
//	{
//		return currunt_sector-(FLASH_MAX_SECTOR_NUM-1);
//	}
//	else	
//	{
//		return  ++currunt_sector;
//	}
	currunt_sector+=offset;
	if(currunt_sector>=FLASH_MAX_SECTOR_NUM)
	{
		currunt_sector-=FLASH_MAX_SECTOR_NUM;
		
	}
	
	return currunt_sector;
	
	
	
}
uint32_t get_before_sector(uint32_t currunt_sector)
{
    	if(currunt_sector==0)
		{
			  return (FLASH_MAX_SECTOR_NUM-1);
		}
		else	
		{
			  return  --currunt_sector;
		}
}

//�����ʼ�ID
uint16_t create_note_id(uint16_t id) 
{
		if(id==MAX_NOTE_ID)
		{
			  id=0;
		}
		else 
		{
			  id++;
		}
		
		return id;
	
	
}
//��������ID�õ�һ�����һ��ƫ�Ƶ�id
uint16_t get_next_note_id(uint16_t id,int16_t offset) //���ݵ�ǰ��id ����һ���µ�id
{
	
		if(id+offset>MAX_NOTE_ID)
		{
			  return id+offset-MAX_NOTE_ID;
		}
		else
		{
			  return id+offset;
		}
	
}
//��������ID�õ�һ����ǰһ��ƫ�Ƶ�id
uint16_t get_before_note_id(int16_t id,int16_t offset) //���ݵ�ǰ��id ����һ���µ�id
{
	
		if((id-offset)<0)
		{
			  return (id-offset)+MAX_NOTE_ID;
		}
		else
		{
			  return id-offset;
		}
	
}
void wait_write_en_ready(void)
{
    
uint8_t retry_counts=0;
flash_write_enable();  
while(!(flash_status_check() & 0x02))
{
        
        retry_counts++;
    
        flash_write_enable();  
        if(retry_counts>20)
        {
           retry_counts=0;          
           NOTE_debug_log("wait write_en overtime");         
        }
        read_position_thread();  
        osDelay(1);
}   
}
//���ʼ�ͷ�洢
void write_note_header(uint16_t header_sector, uint8_t *pdata, uint16_t len)
{
		uint8_t send_buffer[4];
		uint32_t WriteAddr = header_sector * FLASH_SECTOR_SIZE;
		uint8_t retry_counts=0;

        while(flash_status_check() & 0x01)
        {
            retry_counts++;
            if(retry_counts>20)
            {
               retry_counts=0;
               NOTE_debug_log("wait flash ok overtime");         
            }
            osDelay(1);
        }
		wait_write_en_ready();

		send_buffer[0] = FLASH_WRITE_PAGE;
		send_buffer[1] = (uint8_t)((WriteAddr&0x00ff0000)>>16);
		send_buffer[2] = (uint8_t)((WriteAddr&0x0000ff00)>>8);
		send_buffer[3] = (uint8_t)WriteAddr;

		FLASH_SELECT();
		flash_rw(send_buffer, NULL, 4);
		flash_rw(pdata, NULL, len);
		FLASH_RELEASE();
    
}
//������д��ʼ�����������
//write one page about 3.2ms in  real test
void write_note(uint8_t *pdata, uint16_t len)
{
    uint8_t send_buffer[4];
    uint32_t WriteAddr = note_store_start_sector * FLASH_SECTOR_SIZE + note_store_addr_offset;
    //if the addr is a new sector
        
    if(0 == note_store_addr_offset)
    {
        //erase the sector
        flash_erase_sector(note_store_start_sector);
        uint8_t retry_counts=0;
        while(flash_status_check() & 0x01)
        {
            retry_counts++;
            if(retry_counts>20)
            {
               retry_counts=0;
               NOTE_debug_log("wait write_note overtime");         
            }
            read_position_thread();  
            osDelay(1);
        }
    }
   // flash_write_enable();   
    wait_write_en_ready(); 
    send_buffer[0] = FLASH_WRITE_PAGE;
    send_buffer[1] = (uint8_t)((WriteAddr&0x00ff0000)>>16);
    send_buffer[2] = (uint8_t)((WriteAddr&0x0000ff00)>>8);
    send_buffer[3] = (uint8_t)WriteAddr;
    FLASH_SELECT();
    flash_rw(send_buffer, NULL, 4);
    flash_rw(pdata, NULL, 256);
    FLASH_RELEASE();    
    note_store_addr_offset += len;    
    note_header.note_len += len;  //�ۼƱʼǵĳ���  
    if(note_store_addr_offset >= FLASH_SECTOR_SIZE)
    {
			
         if( get_free_sector()>0)
		{		
			note_store_start_sector=get_next_sector(note_store_start_sector,1);  //�����¸��ʼǴ�����ʼ��λ�ô���
			g_device_info.note_read_end_sector=get_next_sector(g_device_info.note_read_end_sector,1);

			//note_store_start_sector++;
			//g_device_info.note_read_end_sector++;
			NOTE_debug_log("currunt note cotent at[%d] sector", note_store_start_sector);
			note_store_addr_offset = 0;
		}
		else
		{
			 Disbale_note_store();
			NOTE_debug_log("no space to stroe note");
		}
    }
}
//�رձʼ� ���ñʼǿ����������
void close_note(uint8_t is_store, uint8_t show)
{
    if(0x1985 == note_header.note_identifier)
    {
        //end one note, store it or ignore it
        if(is_store)
        {
            if(note_buffer_offset)
            {
                //NOTE_debug_log("store the last data[%d]", note_buffer_offset);
               if(get_free_sector())
				{ 
                    write_note((uint8_t *)&note_buffer[note_buffer_num][0], note_buffer_offset);
                    NOTE_debug_log("store the last data[%d] from buffer[%d] total%d", note_buffer_offset,note_buffer_num,note_len_total_test);
                    
                    note_buffer_offset = 0;
                }
              
            }
            read_rtc_data((st_RTC_info *)&note_header.note_time_year);
            if(note_header.note_time_year < 16)
            {
                note_header.note_time_year = 16;
                note_header.note_time_month = 9;
                note_header.note_time_day = 10;
                note_header.note_time_hour = 14;
                note_header.note_time_min = 34;
            }
            write_note_header(note_header.note_start_sector, (uint8_t *)&note_header, sizeof(st_note_header_info));
						//�������һ���µ�id�ʼ� �������ܵıʼ���
            
		    note_store_start_sector=get_next_sector(note_store_start_sector,1);  //��ʵ���ûʲô��
			g_device_info.stored_total_note_num++;							
            NOTE_debug_log("store note num[%d]from[%d]to[%d]sector len[%d] total note[%d]", note_header.note_number, note_header.note_start_sector, g_device_info.note_read_end_sector,note_header.note_len,g_device_info.stored_total_note_num);
            if(show)
            {
               // LEDEventStart(UI_NOTE_STORE);
            }
        }
        else
        {
            note_store_start_sector = note_header.note_start_sector;
            g_device_info.note_read_end_sector = note_store_start_sector;
            NOTE_debug_log("do not store note num[%d]from[%d]to[%d] sector", g_device_info.stored_total_note_num, note_header.note_start_sector, g_device_info.note_read_end_sector);
        }
        memset((uint8_t *)&note_header, 0 , sizeof(st_note_header_info));
							
		Update_device_info((uint8_t *)&g_device_info, sizeof(st_device_data_storage));
    }
}

void offline_write_flash_Thread(void)
{
		 
		if(get_server_mode()==NRF51822)
		{

				if((0x1985 == note_header.note_identifier) && (0 == note_header.flash_erase_flag))
				{
						flash_erase_sector(note_header.note_start_sector);
                         
						uint8_t retry_counts=0;

                        while(flash_status_check() & 0x01)
                        {
                            retry_counts++;
                            if(retry_counts>20)
                            {
                               retry_counts=0;
                               NOTE_debug_log("wait erase flash ok overtime");         
                            }
                            read_position_thread();  
                            osDelay(1);
                        }
						note_header.flash_erase_flag = 1;
                        NOTE_debug_log("erase flash finish");
				}
				if(0x1985 == note_header.note_identifier)
                {
                    if(note_buffer_store_num != 0xff) //��������Ҫ�洢��flash
                    {
                         if(Is_enable_note_store())
                         {
                            //NOTE_debug_log("need store buff[%d]", note_buffer_store_num);
                            write_note((uint8_t *)&note_buffer[note_buffer_store_num][0], 256);
                            note_buffer_store_num = 0xff;
                         }
                    }
                }

		}
}
//�ϴ����߱ʼǴ���

void set_en_upload(uint8_t en)
{
	  note_upload_flag=en;
	
}
void upload_stored_note_Thread(void)
{
    uint8_t cnt = 0;
    //uint8_t testcnt = 0;
    uint8_t end_content = 0x01;
    uint8_t tmp_buffer[20];
    uint32_t left_len;
    uint32_t  read_start_addr;
    uint32_t read_end_addr; 
    uint32_t  first_read_len;
    uint32_t flash_end_addr=FLASH_MAX_SECTOR_NUM*FLASH_SECTOR_SIZE;	

    
    if((note_upload_flag) && (DEVICE_SYNC_MODE ==Get_Device_Status()))
    {
			
        for(cnt = 0; cnt < 4; cnt++)
        {
		    read_start_addr=note_upload_start_addr + note_store_addr_offset;  //��ǰ������ʼ��ַ
            if((note_upload_len + BLE_MAX_PAYLOAD) < offline_note_header.note_len)  //���ʣ�µ�������������������
            {
						//	   NOTE_debug_log("full packet\r\n");
								read_end_addr=read_start_addr+BLE_MAX_PAYLOAD;  //���������յ�ַ
												
								if(read_end_addr>flash_end_addr)//���Ҫ����ȡ
								{     
										//NOTE_debug_log("over the end\r\n");
										first_read_len=flash_end_addr- read_start_addr;
										read_note(read_start_addr, (uint8_t *)tmp_buffer, first_read_len);  //�ȶ�����β��					
										left_len=read_end_addr-flash_end_addr;  //ʣ���ȡ�ĳ���											
										read_note(0, (uint8_t *)(tmp_buffer+first_read_len), left_len);//�ӿ�ͷ��ȡʣ������																
										if(0== server_send_direct((uint8_t *)tmp_buffer, BLE_MAX_PAYLOAD))  //������ͳɹ�
										{   
												note_upload_start_addr=0;     //��ʼ��ַ����Ϊͷ
												note_store_addr_offset = left_len; //ƫ������Ϊʣ��ĳ���
												note_upload_len+=20; //�Ѷ��ĳ����ۼ�
										}															
									
								}
								else//û�п��ʱ
								{
										read_note(read_start_addr, (uint8_t *)tmp_buffer, BLE_MAX_PAYLOAD);
										if(0 == server_send_direct((uint8_t *)tmp_buffer, BLE_MAX_PAYLOAD))
										{
												note_store_addr_offset += 20;  //ƫ���ۼ�
												note_upload_len+=20;   //�ۼƳ���
										}

								}
            }
            else
            {        
							// ����һ����
								if((offline_note_header.note_len - note_upload_len)==0)
								{
										//NOTE_debug_log("lenth is 0\r\n");
										note_upload_flag = 0;
										server_notify_host(CMD_SYNC_IS_END, &end_content, 1);											
										break;
								}											 
							  read_end_addr = read_start_addr + (offline_note_header.note_len - note_upload_len);  //���������յ�ַ       
								if(read_end_addr>flash_end_addr)//���Ҫд��ķ�Χ�����˴洢����ͷ
							  {
										//NOTE_debug_log("over the end\r\n");
										first_read_len=flash_end_addr- read_start_addr;
										read_note(read_start_addr, (uint8_t *)tmp_buffer, first_read_len);  //�ȶ�����β��		
										left_len=read_end_addr-flash_end_addr;  //ʣ���ȡ�ĳ���		
										read_note(0, (uint8_t *)(tmp_buffer+first_read_len), left_len);//�ӿ�ͷ��ȡʣ������
										if(0== server_send_direct((uint8_t *)tmp_buffer,offline_note_header.note_len - note_upload_len))
										{   
												note_upload_start_addr=0;     //��ʼ��ַ����Ϊͷ
												note_store_addr_offset = left_len; //ƫ������Ϊʣ��ĳ���
												note_upload_len+=(offline_note_header.note_len - note_upload_len); //�Ѷ��ĳ����ۼ�
												note_upload_flag = 0;
												server_notify_host(CMD_SYNC_IS_END, &end_content, 1);
												break;
										}
								
							  }
								else //�����
								{
										
																		
										read_note(read_start_addr, (uint8_t *)tmp_buffer, (offline_note_header.note_len - note_upload_len));  //ֱ�Ӷ�ȡʣ��ĳ���
									
										//NOTE_debug_log("read the flash addr%d,len%d\r\n",read_start_addr,(offline_note_header.note_len - note_upload_len));
									
										if(0 == server_send_direct((uint8_t *)tmp_buffer, (offline_note_header.note_len - note_upload_len)))
										{
												note_store_addr_offset += (offline_note_header.note_len - note_upload_len);
												note_upload_len+=(offline_note_header.note_len - note_upload_len); 
												NOTE_debug_log("send over");
                                                osDelay(5);  //����USB����ʱ��
												note_upload_flag = 0;
												server_notify_host(CMD_SYNC_IS_END, &end_content, 1);
												break;
										}

								}
            
            }
				    osDelay(5);		
				
        }
    }
		
}
//������һ����ʼ�ط���ȡһ���ʼ�ͷ��С�Ŀ�
//read one page about 3.4ms in  real test
void read_note(uint32_t addr, uint8_t* pdata, uint16_t len)
{
    uint8_t send_buffer[4];
    if(len==0)
    {
        return;
    }
    send_buffer[0] = FLASH_READ_DATA;
    send_buffer[1] = (uint8_t)((addr&0x00ff0000)>>16);
    send_buffer[2] = (uint8_t)((addr&0x0000ff00)>>8);
    send_buffer[3] = (uint8_t)addr;
    uint8_t retry_counts=0;

    while(flash_status_check() & 0x01)
    {
        retry_counts++;
        if(retry_counts>20)
        {
           retry_counts=0;
           NOTE_debug_log("wait flash ok overtime");         
        }
        osDelay(1);
    }
    FLASH_SELECT();
    flash_rw(send_buffer, NULL, 4);
    flash_rw(send_buffer, pdata, len);
    FLASH_RELEASE();
}



uint8_t search_note_head(uint32_t serch_start_sector)
{
	
	//�Ҳ�������1
		uint32_t ReadAddr = serch_start_sector;   //��ʼ��������ʼ����λ��
		while(ReadAddr != g_device_info.note_read_end_sector)
		{
	
				read_note(ReadAddr*FLASH_SECTOR_SIZE, (uint8_t *)&offline_note_header, sizeof(st_note_header_info));  //��ȡ����ͷ��  ��ͷ�ṹ��			
				if(offline_note_header.note_identifier == 0x1985)
				{   
					
                    NOTE_debug_log("find a  note block(page=[%d] len=[%d])", offline_note_header.note_number, offline_note_header.note_len);
                    NOTE_debug_log("y[%d]m[%d]d[%d]h[%d]m[%d]",     offline_note_header.note_time_year, 
																	offline_note_header.note_time_month,
																	offline_note_header.note_time_day,
																	offline_note_header.note_time_hour,
																	offline_note_header.note_time_min);
                    break;
				}
				ReadAddr =get_next_sector(ReadAddr,1);
		}
		if(ReadAddr !=g_device_info.note_read_end_sector )
		{
		    return 0;
		}
		return 1;
	
}

//������һ���ʼǵ����ݿ�
uint8_t create_next_note_head(uint16_t num_index)
{
		if(0x1985 != note_header.note_identifier)
		{
				init_offline_store_data();
				if(get_free_sector()>0)
				{
						note_store_start_sector = g_device_info.note_read_end_sector;
						NOTE_debug_log("free sectors[%d]",get_free_sector());
						note_header.note_identifier = 0x1985;
						note_header.note_number=num_index;   //����±ʼǵ�ID
						// note_header.note_number = g_device_info.stored_total_note_num;
						note_header.note_start_sector = note_store_start_sector;
						note_header.flash_erase_flag = 0;
						note_header.note_head_start=0;   
						NOTE_debug_log("create note num [%d] at [%d] sector", note_header.note_number, note_store_start_sector);				
						g_device_info.note_read_end_sector=get_next_sector(note_store_start_sector,1);
						NOTE_debug_log("note tail at sector[%d]",g_device_info.note_read_end_sector);
						note_store_addr_offset = 256;
						return 0;
				}
				else
				{
						Disbale_note_store();
						NOTE_debug_log("no more flash");
						return 1;
				}
		}
		return 1;
}


uint8_t Is_Open_note(void)
{
    return (note_header.note_identifier==0x1985);
}


void init_offline_store_data(void)
{
    note_store_start_sector = 0;
    note_store_addr_offset = 0;
    note_upload_len=0;
    note_buffer_offset = 0;
    note_buffer_store_num = 0xff;
    note_store_leave_flag = 0;
    note_store_leave_flag_count = 0;
    note_store_leave_report_flag = 0;
    note_store_cnt = 0;
    note_buffer_num = 0;
    note_len_total_test=0;
    memset((uint8_t *)&note_header, 0 , sizeof(st_note_header_info));   //��ձʼ�ͷ��
    memset((uint8_t *)note_buffer, 0 , 512); //����������ݻ���
    

		if(get_free_sector()==0)  //����洢�ռ䲻�� �رձʼǴ洢
		{					
		    Disbale_note_store();   
		}

		else
		{
		    Enable_note_store();
		}
	
}


void offline_store_data(uint8_t *buf,uint8_t len)
{ 
		//static uint16_t store_pos_x=0,store_pos_y=0;
		st_store_info store_info;
		uint16_t x_pos = 0, y_pos = 0, press = 0;
		memset((uint8_t *)&store_info, 0, sizeof(st_store_info));	
		len = 5;
        
		if(0x11 == buf[1]) //����ǰ��µ�
		{

           // if(g_device_info.stored_total_note_num==0)  //����Ӧ���ж��ǲ��ǵ�һ�ο��� ����ǵ�һ�ο��� �����ݵ��� Ӧ���Ǵ�0�����µıʼ�  ���ȫ����ͬ������ Ӧ�ô����µ�id�ıʼ�
			//{
         
                 uint16_t page_value=0;
                 page_value=read_page_value();
//                 if(page_value==0)
//                 { 
//                    return ;
//                 }
                  update_dis_Pen_icon();
                  create_next_note_head(page_value); //�����ʼ� 
                 
                 
                 
                              				 
				if(note_store_cnt >= 2)  //�ɼ����������ݰ�  ����һ��
				{

						note_store_cnt = 0;
						x_pos = (uint16_t)buf[3] << 8 | buf[2];
						y_pos = (uint16_t)buf[5] << 8 | buf[4];
						press = (uint16_t)buf[7] << 8 | buf[6];
						x_pos = x_pos/2;   //����ֵ��Сһ�� ѹ����С4��
						y_pos = y_pos/2;
						press = press/4;
						if((store_pos_x != 0) || (store_pos_y != 0))
						{
								if((store_pos_x == x_pos) && (store_pos_y == y_pos))
								{
								    return;
								}
						}
						store_pos_x = x_pos;
						store_pos_y = y_pos;
						if(0 == press)  //���û�а��� ��ǰ���
						{
						    press = 1;
						}

						store_info.store_flag|= 0x3;
						//		debug_log("%x %x %x %x %x",packet->data[0],packet->data[1],packet->data[2],packet->data[3],packet->data[4]);
						store_info.x_l = (uint8_t)(x_pos & 0x00ff);
						store_info.x_h = (uint8_t)((x_pos >> 8) & 0x00ff);

						store_info.y_l = (uint8_t)(y_pos & 0x00ff);
						store_info.y_h = (uint8_t)((y_pos >> 8) & 0x003f);  //y�ĸ�4λ���ʶ4bitռһ���ֽ�
						store_info.press = (uint8_t)(press & 0x00ff);  //ѹ��ֵ



                        if(note_store_leave_flag_count != 0)
                        {
                            store_info.store_flag|= 0x3;
                            //		debug_log("%x %x %x %x %x",packet->data[0],packet->data[1],packet->data[2],packet->data[3],packet->data[4]);
                            store_info.x_l = 0;
                            store_info.x_h = 0;

                            store_info.y_l = 0;
                            store_info.y_h = 0;  //y�ĸ�4λ���ʶ4bitռһ���ֽ�
                            store_info.press = 0;  //ѹ��ֵ
                            note_store_leave_report_flag = 0;
                            note_store_leave_flag_count = 0;
                            //	debug_log("leave1\r\n");

                        }
                        else if(!note_store_leave_flag) //press ֮����ܱ�hold��
                        {
                            note_store_leave_flag = 1;
                            note_store_leave_flag_count = 0;
                            //debug_log("press\r\n");

                        }
                        else
                        {
                            //debug_log("press\r\n");
                        }		
                                    

             }
              else
              {
                  note_store_cnt++;
                  return;
              }
        }
        else if(0x10 == buf[1])  //���������
        {
            //leave data
            if(note_store_leave_flag) 
            {
                note_store_leave_report_flag  = 1;              
							
                store_info.store_flag |= 0x3;             
                note_store_cnt = 0;
                //note_store_leave_flag = 0;
                note_store_leave_flag_count ++;
                
                //debug_log2("leave\r\n");
                //debug_log2("count:%d\r\n",note_store_leave_flag_count);

                x_pos = (uint16_t)buf[3] << 8 | buf[2];
                y_pos = (uint16_t)buf[5] << 8 | buf[4];
                press = (uint16_t)buf[7] << 8 | buf[6];
                x_pos = x_pos/2;   //����ֵ��Сһ�� ѹ����С4��
                y_pos = y_pos/2;
                press = press/4;
                //debug_log2("store_flag:%d\r\n",store_info.store_flag);
                //debug_log2("x:%d\r y:%d\r press:%d\r\n store_x:%d\r store_y:%d\r\n",x_pos,y_pos,press,store_pos_x,store_pos_y);
                if((store_pos_x != 0) || (store_pos_y != 0))
                {
                    if((store_pos_x == x_pos) && (store_pos_y == y_pos))
                    {
                        return;
                    }
                }
                store_pos_x = x_pos;
                store_pos_y = y_pos;

       
                store_info.store_flag|= 0x3;
                        //		debug_log("%x %x %x %x %x",packet->data[0],packet->data[1],packet->data[2],packet->data[3],packet->data[4]);
                store_info.x_l = (uint8_t)(x_pos & 0x00ff);
                store_info.x_h = (uint8_t)((x_pos >> 8) & 0x00ff);
                
                store_info.y_l = (uint8_t)(y_pos & 0x00ff);
                store_info.y_h = (uint8_t)((y_pos >> 8) & 0x003f);  //y�ĸ�4λ���ʶ4bitռһ���ֽ�
                store_info.press = 0;  //ѹ��ֵ
              //  debug_log("hold\r\n");
                if(note_store_leave_flag_count > 10)
                {
                    note_store_leave_flag_count = 0;
                    note_store_leave_flag = 0;
                    store_info.store_flag|= 0x3;
                        //		debug_log("%x %x %x %x %x",packet->data[0],packet->data[1],packet->data[2],packet->data[3],packet->data[4]);
                    store_info.x_l = 0;
                    store_info.x_h = 0;
                    
                    store_info.y_l = 0;
                    store_info.y_h = 0;  //y�ĸ�4λ���ʶ4bitռһ���ֽ�
                    store_info.press = 0;  //ѹ��ֵ
                  //  debug_log("leave2\r\n");
                    note_store_leave_report_flag  = 0; 

                    
                }
                

                
            }
            else
            {
                
                
                return;
            }
        }
        else
        {
            if(note_store_leave_flag && note_store_leave_flag_count != 0)
            {
                note_store_leave_flag_count = 0;
                note_store_leave_flag = 0;
                store_info.store_flag|= 0x3;
                        //		debug_log("%x %x %x %x %x",packet->data[0],packet->data[1],packet->data[2],packet->data[3],packet->data[4]);
                store_info.x_l = 0;
                store_info.x_h = 0;
                
                store_info.y_l = 0;
                store_info.y_h = 0;  //y�ĸ�4λ���ʶ4bitռһ���ֽ�
                store_info.press = 0;  //ѹ��ֵ
              //  debug_log("leave3\r\n");
                
            }
            else
            {
                return;
            }


        }
        
        
        
        
       // note_header.note_len += len;  //�ۼƱʼǵĳ���   
         note_len_total_test+=5;
		if((note_buffer_offset + len) <= 256)  //�����ǰ��ƹ��buff ��װ��
		{
				memcpy((uint8_t *)&note_buffer[note_buffer_num][note_buffer_offset], (uint8_t *)&store_info, len);
				note_buffer_offset += len;

				if(note_buffer_offset >= 256)  //���Ҫ����� ��buff
				{
						//buffer full
						note_buffer_store_num = note_buffer_num;
						if(0 == note_buffer_num)
						{
						    note_buffer_num = 1;
						}
						else if(1 == note_buffer_num)
						{
						    note_buffer_num = 0;
						}
						memset((uint8_t *)&note_buffer[note_buffer_num][0], 0, 256);
						note_buffer_offset = 0;
				}

		}
		else  //��buff
		{
				memcpy((uint8_t *)&note_buffer[note_buffer_num][note_buffer_offset], (uint8_t *)&store_info, (256 - note_buffer_offset));
				note_buffer_store_num = note_buffer_num;
				if(0 == note_buffer_num)
				{
						note_buffer_num = 1;
				}
				else if(1 == note_buffer_num)
				{
						note_buffer_num = 0;
				}
				memset((uint8_t *)&note_buffer[note_buffer_num][0], 0, 256);
				memcpy((uint8_t *)&note_buffer[note_buffer_num][0], ((uint8_t *)&store_info + (256 - note_buffer_offset)), (note_buffer_offset + len - 256));
				note_buffer_offset = (note_buffer_offset + len - 256);
				//debug_log("%d", note_buffer_offset);
		}
		return;
	
	
	
}


void init_flash_fifo(void)
{

       
    fifo_total_len=(get_free_sector()-1)*FLASH_SECTOR_SIZE;  //��һ��Ϊ�˶���д����ʼ�ղ�һ������
    wrtie_fifo_addr=g_device_info.note_read_end_sector*FLASH_SECTOR_SIZE;
    read_fifo_addr= wrtie_fifo_addr;
    fifo_currunt_setor=g_device_info.note_read_end_sector;
    fifo_len=0;   
    fifo_offset=0;
    flash_erase_sector(g_device_info.note_read_end_sector);
}



void push_data_to_flash(uint32_t len,uint8_t *data)
{
    uint8_t *data_p;
       
    
    if((fifo_len+len)<fifo_total_len)
    {
  
            
        if((fifo_offset+len)>FLASH_SECTOR_SIZE)
        {
             fifo_currunt_setor= get_next_sector( fifo_currunt_setor,1);
             if(fifo_currunt_setor==g_device_info.note_read_start_sector)
             {
                 fifo_currunt_setor=g_device_info.note_read_end_sector;
                 
             }
             flash_erase_sector(fifo_currunt_setor);
             fifo_offset=fifo_offset+len-FLASH_SECTOR_SIZE;
             
               if(fifo_currunt_setor==0)
                {
                                         
                    uint8_t lenth=  FLASH_END_ADDR- wrtie_fifo_addr;
                               
                    flash_write_page(wrtie_fifo_addr,lenth,data); 
                               
                    if(lenth<len) 
                    {
                        wrtie_fifo_addr=0;   
                        data_p=&data[lenth];  
                        flash_write_page(wrtie_fifo_addr,len-lenth,data_p);
                        wrtie_fifo_addr+=(len-lenth);
                    }
                   
            
                }
                else if(fifo_currunt_setor==g_device_info.note_read_end_sector) 
                {
                    uint8_t lenth=  (((g_device_info.note_read_start_sector-1)*FLASH_SECTOR_SIZE)-1)- wrtie_fifo_addr;
                               
                    flash_write_page(wrtie_fifo_addr,lenth,data);
                               
                    if(lenth<len) 
                    {
                        wrtie_fifo_addr=g_device_info.note_read_end_sector*FLASH_SECTOR_SIZE;   
                        data_p=&data[lenth];  
                        flash_write_page(wrtie_fifo_addr,len-lenth,data_p);
                        wrtie_fifo_addr+=(len-lenth);
                    }   


                }
                else
                {
                    
                    
                    flash_write_page(wrtie_fifo_addr,len,data);  
                    wrtie_fifo_addr+=len;
                    
                }
                
            
             
        }
        else
        {
            fifo_offset+=len;                 
            flash_write_page(wrtie_fifo_addr,len,data);                  
            wrtie_fifo_addr+=len;
            
        }
        
        fifo_len+=len;  
        NOTE_debug_log("infifo len %d",fifo_len);
    }

    
    
}
void pop_data_from_flash(uint32_t len,uint8_t *data)
{
    uint32_t lenth;
    uint8_t *data_p;
    if(fifo_len>=len)
    {

        if((read_fifo_addr+len)>FLASH_END_ADDR)
        {
            if(g_device_info.note_read_start_sector==0)
            {
                
                lenth=FLASH_END_ADDR-read_fifo_addr;
                data_p=&data[lenth];            
                read_note(read_fifo_addr, data, lenth);
                read_fifo_addr=g_device_info.note_read_end_sector*FLASH_SECTOR_SIZE;  
                read_note(read_fifo_addr, data_p, len-lenth);
                read_fifo_addr+=len-lenth;
                //NOTE_debug_log("1");
            }
            else
            {
                
                lenth=FLASH_END_ADDR-read_fifo_addr;
                data_p=&data[lenth];            
                read_note(read_fifo_addr, data, lenth);
                read_fifo_addr=0;
                read_note(read_fifo_addr, data_p, len-lenth);
                read_fifo_addr+=len-lenth;  
                //NOTE_debug_log("2");
            }
       
            
        }
        else if((read_fifo_addr+len)>(((g_device_info.note_read_start_sector-1)*FLASH_SECTOR_SIZE)-1))
        {
            if(g_device_info.note_read_start_sector>g_device_info.note_read_end_sector)
            {
                
                lenth=(((g_device_info.note_read_start_sector-1)*FLASH_SECTOR_SIZE)-1)-read_fifo_addr;
                data_p=&data[lenth];          
                read_note(read_fifo_addr, data, lenth);
                read_fifo_addr=g_device_info.note_read_end_sector*FLASH_SECTOR_SIZE;  
                read_note(read_fifo_addr, data_p, len-lenth);
                read_fifo_addr+=len-lenth;
               // NOTE_debug_log("3");  
                
            }
            else
            {
                read_note(read_fifo_addr, data,  len);
                read_fifo_addr+=len;
               // NOTE_debug_log("4");
                
            }
         
            
        }
        else
        {
            read_note(read_fifo_addr, data,  len);
            read_fifo_addr+=len;
            //NOTE_debug_log("4");
        } 


        fifo_len-=len;
        NOTE_debug_log("outfifo len %d",fifo_len);
    }
       
}

uint32_t get_lenth_of_fifo(void)
{
    return fifo_len;
}




