#include "em9203.h"
#include <stdarg.h>
#include <string.h>
#include "Soft_Timer.h"
#include "RGB_LED.h"
#include "server.h"
#include "note_manager.h"
#include "debug_log.h"
#include "Time_interupt_callback.h"
#include "misc.h"
#include "display.h"
TIM_HandleTypeDef htim1;
SPI_HandleTypeDef hspi2;

extern SPI_HandleTypeDef hspi2;
#define  SPI_CONTROLER    hspi2
#define DEBUG_LOG  debug_log
__IO uint8_t send_module_int_flag = 0;

st_nebula_network_ch network_ch[NEBULA_NETWORK_CLASS_MAX_NUM] = 
{
    {0, 10, 20, 30}, //0
    {1, 11, 21, 31}, //1
    {2, 12, 22, 32}, //2
    {3, 13, 23, 33}, //3
    {4, 14, 24, 34}, //4
    {5, 15, 25, 35}, //5
    {6, 16, 26, 36}, //6
    {7, 17, 27, 37}, //7
    {8, 18, 28, 38}, //8
    {9, 19, 29, 39}, //9
};
//customer: flag for customer
uint8_t customer_number = 0xCC;
//group: 0-rcv0, 1-rcv1, 2-rcv2, 0xEE-gateway_send
uint8_t group_number = 0;

st_nebula_node_device_info node_info;
uint8_t send_ch = 0;
uint8_t rcv_ch = 0;
uint8_t em9203_bak_status=0;
uint8_t em9203_exception=0;
uint32_t g_nebula_status_tick_check = 0;
uint32_t em9203_fault_count=0;

//buffer write data
__IO uint32_t md_write_buffer_input_index = 0;
__IO uint32_t md_write_buffer_output_index = 0;
__IO uint32_t md_last_transfer_len = 0;//used for ack
uint8_t start_check=0;
st_nebula_report_data back_report;
uint8_t  report_busy_flag=0;
uint8_t  use_fifo_flash_flag=0;
uint8_t  calibra_finish_flag=0;
uint8_t  vote_send_flag=0;

//30k buffer
st_write_packet_data md_write_buffer[MD_WRITE_BUF_NO];

st_nebula_report_data hight_speed_fifo_buffer[8];


uint8_t rcv_len = 0;
__IO st_nebula_send_poll_data gateway_poll_data;
uint8_t nebula_back_data_flag = 0;
uint8_t nebula_send_data_flag = 0;
uint8_t module_status = 0;
uint8_t *vote_result = NULL;
SOFT_TIMER_ST     *em9203_sta_timer;
SOFT_TIMER_ST     *em9203_exception_timer;

EM9203_OPS    em9203_ops=
{
	
0,
0,
em9203_open,
em9203_close,
em9203_ioctl,
em9203_read,
em9203_write,
em9203_manage,
em9203_clearbuff,
NULL,
};


/* SPI2 init function */
static void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity =SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;  //32/4=8M
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    while(1);
  }
  __HAL_SPI_ENABLE(&hspi2);
}
/* TIM11 init function */
static void MX_TIM1_Init(void)
{

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 63;  //  64M/31+1=2M
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP ;  //向上
  htim1.Init.Period = 0;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    while(1);
  }
	
	HAL_TIM_Base_Start_IT(&htim1)	;	
  
}

void em9203_open(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = SPI2_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(SPI2_CS_GPIO_Port, &GPIO_InitStruct);
    
    
    MX_SPI2_Init();
    MX_TIM1_Init();
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);   
    EM9203_RELEASE();
    OPEN_EM9203_POWER();
    //DEBUG_LOG("em9203 open");
    em9203_ops.status=0;
    em9203_bak_status=0;
    em9203_clearbuff();
}

void em9203_close(void)
{
   
    start_check=0;
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);   
    HAL_Delay(50);
    HAL_TIM_Base_DeInit(&htim1);
    CLOSE_EM9203_POWER(); 
    HAL_GPIO_DeInit(SPI2_CS_GPIO_Port, SPI2_CS_Pin);
    
    HAL_SPI_DeInit(&hspi2);
    DEBUG_LOG("em9203 close");
    
   
 
	disregister_soft_timer(em9203_sta_timer);
    em9203_sta_timer=NULL;
}
uint8_t em9203_ioctl(uint8_t cmd,uint8_t *param)
{
	uint8_t res;
    switch(cmd)
		{
            case INIT://如果有事件 并且是就绪响应 将结果返回	
             res=nebula_init_node_device((st_nebula_node_device_info*)param);
            
            break;		
            case START_WORK:

            break;

            case STOP_WORK:
                
            break;			 

            default: 
            break;
			
		}

		return res;

	
}

uint8_t em9203_read(uint8_t *data, uint8_t len)
{
    return 0;
}

uint8_t em9203_write(uint8_t opcode, uint8_t * data, uint16_t len)
{
	
    if(opcode==VOTE_OPS)
    {
        
        nebula_vote_result_send(data);
        return 0;
    }
        
    else if(opcode==MASS_OPS)
    {
        return nebula_fill_md_write_data_to_buf((st_write_packet_data *) data);
    }
    else if(opcode==CMD_REPORT_PAGE_AUTO)
    {
        uint8_t buff[5];
        buff[2]=data[0];
        buff[3]=data[1];
        buff[0]=0xff;
        buff[1]=0xff;
        buff[4]=0xff;
        return nebula_fill_md_write_data_to_buf((st_write_packet_data *) buff); 
        
    }
    else
    {
        return 1;
    }
		
}

void em9203_manage(void)
{
	nebula_poll_handle();
}
void em9203_clearbuff(void)
{
	
	nebula_init_md_write_buffer();
	
}

//大数据量时 清除缓冲
void nebula_init_md_write_buffer(void)
{
    md_write_buffer_output_index = 0;
    md_write_buffer_input_index = 0;
    md_last_transfer_len = 0;
	em9203_ops.ready=0;  
    #ifdef USE_FLASH_FIFO
    report_busy_flag=0;
    init_flash_fifo();//初始化flash FIFO
    #endif
} 


uint8_t get_em9203_last_status(void)
{
    return em9203_exception;
    
}

//如果计数到达最大 说明没有host 对设备进行心跳同步了  10ms 一次
void nebula_status_check(void)
{
    if(start_check)
    {
        g_nebula_status_tick_check++;
        if(em9203_bak_status!=em9203_ops.status)  //对状态进行跟踪
        {
            if(em9203_bak_status==NEBULA_STATUS_EXCEPTION)
            {
                DEBUG_LOG("exception timer cancell");
                disregister_soft_timer(em9203_exception_timer);
                em9203_exception_timer=NULL;
                em9203_exception=0;
            }
            
            em9203_bak_status=em9203_ops.status;           
            server_status_call_back(em9203_ops.status);
            if(em9203_ops.status==NEBULA_STATUS_EXCEPTION)
            {
                
                em9203_exception_timer=register_soft_timer();
                start(em9203_exception_timer,EXCEPTION_MAX_TIME);  
                DEBUG_LOG("exception timer start");
                em9203_exception=1;
                               
            }
            else if((em9203_ops.status==NEBULA_STATUS_VOTE)||(em9203_ops.status==NEBULA_STATUS_MUX_VOTE)||(em9203_ops.status== NEBULA_STATUS_FIRST_ANSWER))
            {
                init_vote_value();
               
            }
              
            send_event_to_nebula_screen(STATUS_CHANGE);
        }

        if(g_nebula_status_tick_check > CONNECT_TICK_TIME )
        {
            
            
             
  
            if(em9203_ops.status==NEBULA_STATUS_MASSDATA)
            {
               //DEBUG_LOG("tick 1overflow");
               em9203_ops.status=NEBULA_STATUS_EXCEPTION; 
            }
            else if((em9203_ops.status!=NEBULA_STATUS_MASSDATA)&&(em9203_ops.status!=NEBULA_STATUS_EXCEPTION))
            {
                em9203_ops.status=NEBULA_STATUS_OFFLINE; 
                //DEBUG_LOG("tick 2overflow");
            }
                     
            
        }
        if(IS_Elapsed(em9203_exception_timer))
        {
             if(em9203_bak_status==NEBULA_STATUS_EXCEPTION)
            {
                 DEBUG_LOG("exception timer over");
                em9203_ops.status=NEBULA_STATUS_OFFLINE; 
                em9203_bak_status=em9203_ops.status;           
                server_status_call_back(em9203_ops.status);
                disregister_soft_timer(em9203_exception_timer);
                em9203_exception_timer=NULL;
                em9203_exception=0;
                send_event_to_nebula_screen(STATUS_CHANGE);
            }
    
            
        }
        
        
       restore_from_fault();
             
    }
}


//将数据填入大数据缓冲池
uint8_t nebula_fill_md_write_data_to_buf(st_write_packet_data * data)
{
    uint32_t i = 0;
    uint32_t buf_index = 0;
    
    i = md_write_buffer_input_index - md_write_buffer_output_index;
    if(i >= MD_WRITE_BUF_NO)  
    {   
        DEBUG_LOG("fifo overflow");
        return NEBULA_FULL;  
    }
    buf_index = md_write_buffer_input_index%MD_WRITE_BUF_NO;
    
     //DEBUG_LOG("input index=%d",buf_index);
    memcpy((uint8_t *)&md_write_buffer[buf_index], (uint8_t *)data, sizeof(st_write_packet_data));
    md_write_buffer_input_index ++;

    return NEBULA_SUCESS;
}
//查看大数据缓冲池的要发送出去的数量
uint32_t nebula_inquiry_md_data_len(void)
{
    uint32_t i = 0;
    if(md_write_buffer_input_index > md_write_buffer_output_index)
    {
        i = md_write_buffer_input_index - md_write_buffer_output_index;
        return i;
    }
    return 0;
}
//从缓冲池里取出一定长度的数据
//  
uint8_t nebula_read_md_data(uint8_t *pdata, uint8_t len)
{
    uint32_t buf_index = 0;
    uint32_t index;
    uint32_t data_len = nebula_inquiry_md_data_len();  //取出坐标块数
    len/=5;
    if(data_len > 0)
    {
        buf_index = md_write_buffer_output_index%MD_WRITE_BUF_NO;
        index=md_write_buffer_output_index;
     //   DEBUG_LOG("output index=%d",buf_index);
        //data_len = data_len * sizeof(st_write_packet_data);  //数据总长度
        if(data_len >= len)  //如果数据长度大于本次能读出的长度
        {
           // memcpy(pdata, (uint8_t *)&md_write_buffer[buf_index], len);  //拷贝本次的长度
            
            for(uint8_t i=0;i<len;i++)
            {
                memcpy(pdata,(uint8_t *)&md_write_buffer[buf_index],sizeof(st_write_packet_data));
                pdata+=sizeof(st_write_packet_data);
                index++;
                buf_index = index%MD_WRITE_BUF_NO;
                
            }
            
            
            md_last_transfer_len = len;  //本次要传输的块数
            return len*sizeof(st_write_packet_data);
        }
        else
        {
           // memcpy(pdata, (uint8_t *)&md_write_buffer[buf_index], data_len);  //读取剩下的所有数据
            
            for(uint8_t i=0;i<data_len;i++)
            {
                memcpy(pdata,(uint8_t *)&md_write_buffer[buf_index],sizeof(st_write_packet_data));
                pdata+=sizeof(st_write_packet_data);
                index++;
                buf_index = index%MD_WRITE_BUF_NO;
                
            }
            
            
            md_last_transfer_len = data_len;
            return data_len*sizeof(st_write_packet_data);
        }
    }
    else
    {
        md_last_transfer_len = 0;
        return 0;
    }
}

void nebula_move_buffer_point(void)
{
    if(md_last_transfer_len)  //如果上次已经有数据传出 移动缓冲指针
    {

        md_write_buffer_output_index += md_last_transfer_len;
        md_last_transfer_len = 0;
    }
}


uint8_t nebula_init_node_device(st_nebula_node_device_info *info)
{
    uint8_t *p_ch;   
    
    memcpy(&node_info,info,sizeof(st_nebula_node_device_info));
    
    
    
    DEBUG_LOG("custom %d class %d device%d",node_info.customer_number,node_info.class_number,node_info.device_number);
    p_ch = (uint8_t *)&network_ch[node_info.class_number];

    send_ch = p_ch[node_info.device_number%(NEBULA_GROUP_NUM_IN_CLASS - 1) + 1];

    rcv_ch = network_ch[node_info.class_number].TX_CH;
    em9203_ops.status=NEBULA_STATUS_OFFLINE;
    init_em_module();
    nebula_init_md_write_buffer();
    g_nebula_status_tick_check = 0;
    start_check=1;  //开启定时状态检测处理
    em9203_sta_timer=register_soft_timer();
    start(em9203_sta_timer,10); //10毫秒
    return NEBULA_SUCESS;

}

static void spi_send_data(uint8_t *send_buf, uint16_t len)
{
    uint8_t cnt, rcv_tmp;
    
    EM9203_SELECT();
    for(cnt = 0; cnt < len; cnt++)
    {
        uint8_t retry_count=0;
        while((SPI_CONTROLER.Instance->SR & SPI_FLAG_TXE)==0)
        {
            retry_count++;
            if(retry_count>200)
            {
                retry_count=0;
                DEBUG_LOG("em9203 spi send overtime");
            }
            
        }

        SPI_CONTROLER.Instance->DR = send_buf[cnt];
        retry_count=0;
        while((SPI_CONTROLER.Instance->SR & SPI_FLAG_RXNE)==0)
        {
            retry_count++;
            if(retry_count>200)
            {
                retry_count=0;
                DEBUG_LOG("em9203 spi send overtime");
            }
                        
        }

        rcv_tmp = SPI_CONTROLER.Instance->DR;
    }
    EM9203_RELEASE();
}
static void spi_send_rcv_data(uint8_t *send_buf, uint8_t *rcv_buf, uint16_t len)
{
    uint8_t cnt;
    EM9203_SELECT();
    for(cnt = 0; cnt < len; cnt++)
    {
        uint8_t retry_count=0;
        while((SPI_CONTROLER.Instance->SR & SPI_FLAG_TXE)==0)
        {
            retry_count++;
            if(retry_count>200)
            {
                retry_count=0;
                DEBUG_LOG("em9203 spi rcv overtime");
            }
            
        }

        SPI_CONTROLER.Instance->DR = send_buf[cnt];
        retry_count=0;
        while((SPI_CONTROLER.Instance->SR & SPI_FLAG_RXNE)==0)
        {
            retry_count++;
            if(retry_count>200)
            {
                retry_count=0;
                DEBUG_LOG("em9203 spi rcv overtime");
            }
            
        }

        rcv_buf[cnt] = SPI_CONTROLER.Instance->DR;
    }
    EM9203_RELEASE();
}

static void EM92013_SPI_write (uint8_t spi_addr, uint8_t spi_data)
{
    uint8_t send_buffer[2];
    //write enable
    send_buffer[0] = spi_addr & 0x7F;
    send_buffer[1] = spi_data;
    spi_send_data(send_buffer, 2);
}
static void EM92013_SPI_read (uint8_t spi_addr, uint8_t* spi_data)
{
    uint8_t send_buffer[2];
    uint8_t rcv_buffer[2];
    //write enable
    send_buffer[0] = spi_addr | 0x80;
    spi_send_rcv_data(send_buffer, rcv_buffer, 2);
    *spi_data = rcv_buffer[1];
}
static void EM92013_SPI_read_fifo(uint8_t* spi_data, uint8_t len)
{
    uint8_t send_buffer[33];
    uint8_t rcv_buffer[33];
    send_buffer[0] = 0x60 | 0x80;   
    spi_send_rcv_data(send_buffer, rcv_buffer, len + 1);
    memcpy(spi_data, rcv_buffer + 1, len);
}
static void EM92013_SPI_write_fifo(uint8_t* spi_data, uint8_t len)
{
    uint8_t send_buffer[33];
    send_buffer[0] = 0x40 & 0x7F;
    memcpy(&send_buffer[1], spi_data, len);
    EM92013_SPI_write(0x14, len - 1); //设置发送长度
    spi_send_data(send_buffer, len + 1); //写入到缓冲里
    EM92013_SPI_write(0x16, 0x80);  //启动传输？
    EM92013_SPI_write(0x04, 0x03);
    nebula_send_data_flag = 1;
}


static void init_em_module(void)
{
   uint32_t count=0;
    uint8_t status2;
    
    retry:
    send_module_int_flag=0;
    EM92013_SPI_write(0x1A, 0xB3);                                              //SPI reset
    EM92013_SPI_write(0x1A, 0x5E);
    count=0;
    while(send_module_int_flag==0)
    {
        count++;
        HAL_Delay(5);
        
   
       
        if(count>100)
        {
             DEBUG_LOG("waite 9203 overtime");
             goto retry;
        }
        
    }

    EM92013_SPI_write(0x01, 0xFF);                         //clear int                                        
    EM92013_SPI_write(0x00, 0xFF);
    calibra_finish_flag=0;             //清除校准完成标志
    EM92013_SPI_write(0x02, 0x00);                               
    EM92013_SPI_write(0x03, 0x01);                               //enable auto-calibaration interrupt

    EM92013_SPI_write(0x07, 0x13);                               //set channel for autocalibration
    #ifdef TX_POWER_3DB  
    EM92013_SPI_write(0x06, 0x37);        
    #else
    EM92013_SPI_write(0x06, 0x36);                               //set maximum output power, set 2Mbps, and start auto-calibration, 0x37 for EM9201 1Mbps, 0x3F for EM9203 2Mbps
    #endif
    
    count=0;
    while(calibra_finish_flag==0)                                              //等待完成
    {
        count++;
        HAL_Delay(5);
        
        
        EM92013_SPI_read(0x01,&status2);
        if(status2&0x01)
        {
              DEBUG_LOG("celibra 9203 finish");
              break;
            
        }
        if(count>100)
        {
             DEBUG_LOG("celibra 9203 failed retry");
             goto retry;
        }
        
    }

    DEBUG_LOG("config em9203");
    EM92013_SPI_write(0x01, 0xFF);                               //clear all IRQ
    EM92013_SPI_write(0x00, 0xFF);

    EM92013_SPI_write(0x04, 0x00);                               //go back standby mode manually, not mandatory, as device will do this automatically.


    EM92013_SPI_write(0x07, rcv_ch);                           //set communication channel  设置接收信道
    #ifdef TX_POWER_3DB  
    EM92013_SPI_write(0x06, 0x17);    
    #else
    EM92013_SPI_write(0x06, 0x16);                                               //2Mbps , +3dBm
    #endif
    EM92013_SPI_write(0x08, 0xAD);                             ////setRF startup time 150us, RX<->TX switching time 150us

    group_number = 0xEE;
    EM92013_SPI_write(0x0E,  node_info.customer_number);                               //byte0 //RX address 设置接收地址
    EM92013_SPI_write(0x0F, (0xC0 |  node_info.class_number));                              //byte1
    EM92013_SPI_write(0x10,  group_number);                               //byte2
    group_number = node_info.device_number%(NEBULA_GROUP_NUM_IN_CLASS - 1) + 1;
    EM92013_SPI_write(0x11,  node_info.customer_number);                               //byte0 //tx address 发送地址
    EM92013_SPI_write(0x12, (0xC0 |  node_info.class_number));                              //byte1
    EM92013_SPI_write(0x13,  group_number);                               //byte2

    EM92013_SPI_write(0x02, 0xC0);                              // enble rxDR and txDS interrupt
    EM92013_SPI_write(0x03, 0x00);

    EM92013_SPI_write(0x0B, 0x1D);                              // disable Auto_acknowledge
    EM92013_SPI_write(0x0C, 0x70);                              // disable auto re-transmit

    EM92013_SPI_write(0x04, 0x02);                               //go to standby mode, not madantory

}

uint8_t  check_em9203(void)
{
    em9203_open();
    HAL_Delay(500);   
    uint32_t count=0;   
    send_module_int_flag=0;
    EM92013_SPI_write(0x1A, 0xB3);                                              //SW reset
    EM92013_SPI_write(0x1A, 0x5E);     
    count=0;
    while(send_module_int_flag==0)
    {
        count++;
        HAL_Delay(5);
              
        if(count>100)
        {
             UserLog("waite 9203 overtime\r\n");
             return 1;
        }
        
    }
    
    EM92013_SPI_write(0x01, 0xFF);                                              
    EM92013_SPI_write(0x00, 0xFF);
    calibra_finish_flag=0;
    EM92013_SPI_write(0x02, 0x00);                               
    EM92013_SPI_write(0x03, 0x01);                               //enable auto-calibaration interrupt
    EM92013_SPI_write(0x07, 0x13);                               //set channel for autocalibration
    #ifdef TX_POWER_3DB  
    EM92013_SPI_write(0x06, 0x37);        
    #else
    EM92013_SPI_write(0x06, 0x36);                               //set maximum output power, set 2Mbps, and start auto-calibration, 0x37 for EM9201 1Mbps, 0x3F for EM9203 2Mbps
    #endif   
    count=0;
    while(calibra_finish_flag==0)                                              //应该增加超时检测和重试
    {
        count++;
        HAL_Delay(5);
        if(count>100)
        {
             UserLog("em9203 init error\r\n");
             CLOSE_EM9203_POWER(); 
             return 1;
        }
        
    }

   // DEBUG_LOG("em9203 init ok");
    CLOSE_EM9203_POWER(); 
     return 0;
}


uint8_t start_test_2g4_net(void)
{
   
    uint8_t *p_ch;   
    em9203_open();
    HAL_Delay(100);
    
    node_info.customer_number=0xcc;
    node_info.class_number=8;
    node_info.device_number=0;
    DEBUG_LOG("custom %d class %d device%d",node_info.customer_number,node_info.class_number,node_info.device_number);
    p_ch = (uint8_t *)&network_ch[node_info.class_number];

    send_ch = p_ch[node_info.device_number%(NEBULA_GROUP_NUM_IN_CLASS - 1) + 1];

    rcv_ch = network_ch[node_info.class_number].TX_CH;
    em9203_ops.status=NEBULA_STATUS_OFFLINE;
    init_em_module();
   // nebula_init_md_write_buffer();
  //  g_nebula_status_tick_check = 0;
    start_check=1;  //开启定时状态检测处理

    return 0;
    
    
}

void quit_test_2g4_net(void)
{
    em9203_close();
       
}
void restore_from_fault(void)
{
    
    uint8_t status;
//    uint8_t int_status,int2_status;
    uint32_t cpu_sr;
    
    OS_ENTER_CRITICAL();   
    EM92013_SPI_read(0x17,&status);   
//     EM92013_SPI_read(0x00,&int_status);
//    EM92013_SPI_read(0x01,&int2_status);
    OS_EXIT_CRITICAL() ;
    
//     DEBUG_LOG("status %x",status);
//     DEBUG_LOG("int status %x",int_status);
//     DEBUG_LOG("int2 status %x",int2_status);
    
    
    if(status==0x0d)
    {
        
        DEBUG_LOG("recovery from fault");
        init_em_module();
        

    }
        
    
}


void nebula_vote_result_send(uint8_t *rst)
{
   if((NEBULA_STATUS_VOTE == em9203_ops.status)||(NEBULA_STATUS_MUX_VOTE == em9203_ops.status)||( NEBULA_STATUS_FIRST_ANSWER == em9203_ops.status))
    {
        vote_result = rst;
        vote_send_flag=1;
    }
}

void nebula_poll_handle()
{

    uint8_t i;
    if(IS_Elapsed(em9203_sta_timer))
    {

        nebula_status_check();
        start(em9203_sta_timer,10); //10毫秒

    }
    #ifdef  USE_FLASH_FIFO
    
    if(nebula_inquiry_md_data_len()>(MD_WRITE_BUF_NO/2))//如果长度大于安全线
    {
         uint32_t cpu_sr;   
         
        for(i=0;i<8;i++)
        {
             hight_speed_fifo_buffer[i].device_id=node_info.device_number;
             hight_speed_fifo_buffer[i].report_id= NEBULA_REPORT_WRITING;  
              hight_speed_fifo_buffer[i].payload_lenth=30;
             OS_ENTER_CRITICAL();      
             nebula_read_md_data(hight_speed_fifo_buffer[i].payload, 30);
             nebula_move_buffer_point(); 
             OS_EXIT_CRITICAL() ;

                    
        }
        
        push_data_to_flash(256,(uint8_t *)&hight_speed_fifo_buffer); 
        use_fifo_flash_flag=1;
        
        
    }
        
    if(get_lenth_of_fifo()>=32)
    {         
        if(report_busy_flag==1)
         {

            memset(&back_report,0,sizeof(st_nebula_report_data));                         
            pop_data_from_flash(32,(uint8_t *)&back_report);
            if((back_report.device_id!=node_info.device_number)||(back_report.report_id!=NEBULA_REPORT_WRITING))
            {
                DEBUG_LOG("read error");
                 report_busy_flag=0;
            } 
            else 
            {
                // DEBUG_LOG("read ok");
                 report_busy_flag=2;
            }                
           
   
         
        }
    }
    else
    {
        use_fifo_flash_flag=0;
      // init_flash_fifo();
    }
          
    #endif
    
    
    
    
}

//中断回调函数
void Em9203_Int_Callback(void)
{
	
    uint8_t int_status = 0;   
    uint8_t int2_status=0;
    send_module_int_flag = 1;
    
    EM92013_SPI_read(0x00,&int_status);
    EM92013_SPI_read(0x01,&int2_status);
    if(0x80 & int_status)
    {   
        
        EM92013_SPI_read(0x15,&rcv_len); 
        rcv_len++;
        EM92013_SPI_read_fifo((uint8_t *)&gateway_poll_data, rcv_len);
        EM92013_SPI_write(0x16,0x10);      
        EM92013_SPI_write(0x00,0xff);
        em9203_ops.status = gateway_poll_data.status;
        // DEBUG_LOG("int");
        g_nebula_status_tick_check = 0;
        if((NEBULA_STATUS_OFFLINE == em9203_ops.status))
        {
            
        }
        else if(NEBULA_STATUS_STANDBY == em9203_ops.status)
        {
            if(NEBULA_CMD_ACK == gateway_poll_data.command)
            {
                EM92013_SPI_write(0x04, 0x00);
                EM92013_SPI_write(0x07, send_ch);
                nebula_back_data_flag = 1;
                Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1));  
                                
                                
            }
        }
        else if(NEBULA_STATUS_VOTE == em9203_ops.status)
        {
            if(NEBULA_CMD_ACK == gateway_poll_data.command)
            {
                EM92013_SPI_write(0x04, 0x00);
                EM92013_SPI_write(0x07, send_ch);
                nebula_back_data_flag = 1;
                Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1)); 
            }
            else
            {
                if(node_info.device_number < 32)
                {
                    if(gateway_poll_data.ack_status_0_31 & (1 << node_info.device_number))                        
                    {
                        if( vote_send_flag)
                        {
                            send_event_to_nebula_screen(CHOOSE_OVER);
                            vote_send_flag=0;
                            vote_result=0;
                        }
                       
                        
                    }
                }
                else if((node_info.device_number >= 32) && (node_info.device_number < 64))
                {
                    if(gateway_poll_data.ack_status_32_63 & (1 << (node_info.device_number - 32)))                   
                    {  
                        if( vote_send_flag)
                        {
                            send_event_to_nebula_screen(CHOOSE_OVER);
                            vote_send_flag=0;
                            vote_result=0;
                            
                        }
                    }
                }
                else if((node_info.device_number >= 64) && (node_info.device_number < 95))
                {
                   if(gateway_poll_data.ack_status_64_95 & (1 << (node_info.device_number - 64)))
                   {
                        if( vote_send_flag)
                        {
                            send_event_to_nebula_screen(CHOOSE_OVER);
                            vote_send_flag=0;
                            vote_result=0;
                            
                        }
                   }
                }
            }
        }
        else if(NEBULA_STATUS_MUX_VOTE == em9203_ops.status)
        {
            if(NEBULA_CMD_ACK == gateway_poll_data.command)
            {
                EM92013_SPI_write(0x04, 0x00);
                EM92013_SPI_write(0x07, send_ch);
                nebula_back_data_flag = 1;
                Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1)); 
            }
            else
            {
                if(node_info.device_number < 32)
                {
                    if(gateway_poll_data.ack_status_0_31 & (1 << node_info.device_number))
                    {
                        if( vote_send_flag)
                        {
                            send_event_to_nebula_screen(CHOOSE_OVER);
                            vote_result=0;
                            vote_send_flag=0;
                            
                        }
                        
                    }
                }
                else if((node_info.device_number >= 32) && (node_info.device_number < 64))
                {
                    if(gateway_poll_data.ack_status_32_63 & (1 << (node_info.device_number - 32)))
                    {
                        if( vote_send_flag)
                        {
                            send_event_to_nebula_screen(CHOOSE_OVER);
                            vote_result=0;
                            vote_send_flag=0;
                            
                        }
                    }
                }
                else if((node_info.device_number >= 64) && (node_info.device_number < 95))
                {
                    if(gateway_poll_data.ack_status_64_95 & (1 << (node_info.device_number - 64)))
                    {
                       if( vote_send_flag)
                        {
                            send_event_to_nebula_screen(CHOOSE_OVER);
                            vote_result=0;
                            vote_send_flag=0;
                            
                        }
                    }
                }
            }
        }
        else if(NEBULA_STATUS_MASSDATA == em9203_ops.status)
        {
            if(NEBULA_CMD_ACK == gateway_poll_data.command)
            {
               #ifdef  USE_FLASH_FIFO   
                               
                if(use_fifo_flash_flag)
                {
                    if(report_busy_flag==0)
                    {
                                                                           
                       report_busy_flag=1;
                    }
                    else if(report_busy_flag==2)
                    {
                        
                        EM92013_SPI_write(0x04, 0x00);
                        EM92013_SPI_write(0x07, send_ch);                         
                        nebula_back_data_flag = 1;                                                            
                        Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1));                          
                        
                    }
                    
                }
                else
                {
                      EM92013_SPI_write(0x04, 0x00);
                      EM92013_SPI_write(0x07, send_ch);
                      nebula_back_data_flag = 1;             
                      Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1));  
                    
                }
                
                #else
                
                      EM92013_SPI_write(0x04, 0x00);
                      EM92013_SPI_write(0x07, send_ch);
                      nebula_back_data_flag = 1;             
                      Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1));  
                
                #endif
                            
                
            }
            else
            {
                if(node_info.device_number < 32)
                {
                    if(gateway_poll_data.ack_status_0_31 & (1 << node_info.device_number))
                    {
                        nebula_move_buffer_point();
                        report_busy_flag=0;
                    }
                }
                else if((node_info.device_number >= 32) && (node_info.device_number < 64))
                {
                    if(gateway_poll_data.ack_status_32_63 & (1 << (node_info.device_number - 32)))
                    {
                        nebula_move_buffer_point();
                        report_busy_flag=0;
                    }
                }
                else if((node_info.device_number >= 64) && (node_info.device_number < 95))
                {
                    if(gateway_poll_data.ack_status_64_95 & (1 << (node_info.device_number - 64)))
                    {
                        nebula_move_buffer_point();
                        report_busy_flag=0;
                    }
                }
            }
        }
        else if(NEBULA_STATUS_FIRST_ANSWER == em9203_ops.status)
        {
            
            if(NEBULA_CMD_ACK == gateway_poll_data.command)
            {
                EM92013_SPI_write(0x04, 0x00);
                EM92013_SPI_write(0x07, send_ch);
                nebula_back_data_flag = 1;
                Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1)); 
          
            }
            
        }
        
    }
    else if(0x40 & int_status)
    {
        
//      send_module_int_flag = 0; 
        EM92013_SPI_write(0x00,0xff);
        if(nebula_send_data_flag)
        {
            //sent over
            EM92013_SPI_read(0x04, &module_status);
            uint8_t count=0;
            while(module_status == 0x03)
            {
               
                count++;
                if(count>200)
                {
                    DEBUG_LOG("wait em9203 overtime");
                }
                EM92013_SPI_read(0x04, &module_status);
            }
            EM92013_SPI_write(0x07, rcv_ch);
            EM92013_SPI_write(0x04, 0x02);
            nebula_send_data_flag = 0;
        }
        
    }
    if(0x01 & int2_status)       
    {
       EM92013_SPI_write(0x01,0xff);
       calibra_finish_flag=1; 
        
    }

	
	
}

void  Hard_Timer_Us_Callback(void)
{
	
    if(start_check)
    {

        if(nebula_back_data_flag)
        {
            st_nebula_report_data report;
            if(NEBULA_STATUS_STANDBY == em9203_ops.status)
            {
                report.device_id = node_info.device_number;
                report.report_id = NEBULA_REPORT_SYSTICK;
                report.payload_lenth = 0;
                EM92013_SPI_write_fifo((uint8_t *)&report, report.payload_lenth + 2);
                //RF send but do not sent over
            }
            else if(NEBULA_STATUS_VOTE == em9203_ops.status)
            {
                report.device_id = node_info.device_number;
                report.report_id = NEBULA_REPORT_VOTE;
                if(vote_result)
                {
                     
                    report.payload_lenth =1;                  
                    memcpy(report.payload,vote_result,report.payload_lenth);

                }
                else
                {
                    report.payload_lenth = 0;
                }
                EM92013_SPI_write_fifo((uint8_t *)&report, report.payload_lenth + 2);
            //RF send but do not sent over
            }
            else if(NEBULA_STATUS_MUX_VOTE == em9203_ops.status)
            {
                report.device_id = node_info.device_number;
                report.report_id =NEBULA_REPORT_MULTI_VOTE      ;
                if(vote_result)
                {
                    
                    report.payload_lenth = MAX_VOTE_NUM;               
                    memcpy(report.payload,vote_result,report.payload_lenth);
                }
                else
                {
                    report.payload_lenth = 0;
                }
                EM92013_SPI_write_fifo((uint8_t *)&report, report.payload_lenth + 2);
                
            }
            else if(NEBULA_STATUS_FIRST_ANSWER == em9203_ops.status)
            {
                report.device_id = node_info.device_number;
                report.report_id = NEBULA_REPORT_FIRST_ANSWER;
                if(vote_result)
                {
                    
                  //  report.payload_lenth = MAX_VOTE_NUM;   
                     report.payload_lenth =1; 
                     report.payload[0]=vote_result[0];
                     vote_result=0;
                    // send_event_to_nebula_screen(CHOOSE_OVER);//设置发送完成
                }
                else
                {
                    report.payload_lenth = 0;
                }
                EM92013_SPI_write_fifo((uint8_t *)&report, report.payload_lenth + 2);
                
                
            }
                  
            else if(NEBULA_STATUS_MASSDATA == em9203_ops.status)
            {
               #ifdef  USE_FLASH_FIFO              
                     
               if(report_busy_flag)
               {
                    EM92013_SPI_write_fifo((uint8_t *)&back_report, back_report.payload_lenth + 2);  
               }                                                                
               else
               {  
                    report.device_id = node_info.device_number;
                    report.report_id = NEBULA_REPORT_WRITING;
                    report.payload_lenth = nebula_read_md_data(report.payload, 30);
                    EM92013_SPI_write_fifo((uint8_t *)&report, report.payload_lenth + 2);
               }
               #else
               
                    report.device_id = node_info.device_number;
                    report.report_id = NEBULA_REPORT_WRITING;
                    report.payload_lenth = nebula_read_md_data(report.payload, 30);
                    EM92013_SPI_write_fifo((uint8_t *)&report, report.payload_lenth + 2);
               
               #endif
                
                

            }
            nebula_back_data_flag = 0;
        }



    }
    else
    {
         DEBUG_LOG("set rev");
         EM92013_SPI_write(0x07, rcv_ch);
         EM92013_SPI_write(0x04, 0x02);
    }        
	
	
	
	
}

