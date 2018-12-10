#include "em9203.h"
#include <stdarg.h>
#include <string.h>
#include "Soft_Timer.h"
#include "RGB_LED.h"
#include "server.h"
#include "Time_interupt_callback.h"

extern SPI_HandleTypeDef hspi2;
#define  SPI_CONTROLER    hspi2

__IO uint8_t send_module_int_flag = 0;



uint8_t test_buff[1024];

//customer: flag for customer
uint8_t customer_number = 0xCC;
//class: 0~19
uint8_t class_number = 0;
//group: 0-rcv0, 1-rcv1, 2-rcv2, 0xEE-gateway_send
uint8_t group_number = 0;
uint8_t device_number = 1;
st_nebula_node_device_info node_info;
uint8_t send_ch = 0;
uint8_t rcv_ch = 0;
uint8_t em9203_bak_status=0;
//uint8_t g_nebula_status = 0;
uint32_t g_nebula_status_tick_check = 0;
uint32_t check_status_count=0;
//buffer write data
__IO uint32_t md_write_buffer_input_index = 0;
__IO uint32_t md_write_buffer_output_index = 0;
__IO uint32_t md_last_transfer_len = 0;//used for ack
uint8_t start_check=0;
__IO uint8_t en_recive_flag=0;
uint8_t  packet_len=0;
//30k buffer
st_write_packet_data md_write_buffer[MD_WRITE_BUF_NO];

uint8_t rcv_len = 0;
__IO st_nebula_send_poll_data gateway_poll_data;
uint8_t nebula_back_data_flag = 0;
uint8_t nebula_send_data_flag = 0;
uint8_t module_status = 0;
SOFT_TIMER_ST     *em9203_sta_timer;

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


void em9203_open(void)
{
    OPEN_EM9203_POWER();
}

void em9203_close(void)
{
    CLOSE_EM9203_POWER(); 
    start_check=0;
	
}
uint8_t em9203_ioctl(uint8_t cmd,uint8_t *param)
{
	uint8_t res;
    switch(cmd)
		{
            case INIT://如果有事件 并且是就绪响应 将结果返回	
             res=nebula_init_node_device();
            
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
        nebula_vote_result_send(*data);
        return 0;
    }
        
    else if(opcode==MASS_OPS)
    {
        return nebula_fill_md_write_data_to_buf((st_write_packet_data *) data);
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
} 
//如果计数到达最大 说明没有host 对设备进行心跳同步了  10ms 一次
void nebula_status_check(void)
{
    if(start_check)
    {
        g_nebula_status_tick_check++;
        if(em9203_bak_status!=em9203_ops.status)  //对状态进行跟踪
        {

            em9203_bak_status=em9203_ops.status;
            server_status_call_back(em9203_ops.status);

        }

        if(g_nebula_status_tick_check > CONNECT_TICK_TIME )
        {
            em9203_ops.status=NEBULA_STATUS_OFFLINE;
            nebula_init_md_write_buffer();
        }
    }
}


//返回状态
//uint8_t check_nebula_status(void)
//{
//    return g_nebula_status;
//}
//将数据填入大数据缓冲池
uint8_t nebula_fill_md_write_data_to_buf(st_write_packet_data * data)
{
    uint32_t i = 0;
    uint32_t buf_index = 0;
    
    i = md_write_buffer_input_index - md_write_buffer_output_index;
    if(i > MD_WRITE_BUF_NO)  //怎么可能大于这个数
    {   
        md_write_buffer_output_index = md_write_buffer_input_index;  //溢出了重置？
        return NEBULA_FAIL;
    }
    buf_index = md_write_buffer_input_index%MD_WRITE_BUF_NO;
    
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
    uint32_t data_len = nebula_inquiry_md_data_len();  //取出坐标块数
    if(data_len > 0)
    {
        buf_index = md_write_buffer_output_index%MD_WRITE_BUF_NO;
        data_len = data_len * sizeof(st_write_packet_data);  //数据总长度
        if(data_len >= len)  //如果数据长度大于本次能读出的长度
        {
            memcpy(pdata, (uint8_t *)&md_write_buffer[buf_index], len);  //拷贝本次的长度
            md_last_transfer_len = len/sizeof(st_write_packet_data);  //本次要传输的块数
            return len;
        }
        else
        {
            memcpy(pdata, (uint8_t *)&md_write_buffer[buf_index], data_len);  //读取剩下的所有数据
            md_last_transfer_len = data_len/sizeof(st_write_packet_data);
            return data_len;
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

        RED_LED_OFF();
        RED_LED_ON();
        md_write_buffer_output_index += md_last_transfer_len;
        md_last_transfer_len = 0;
    }
}


uint8_t nebula_init_node_device(void)
{
       
    node_info.customer_number = 0xCC;   //暂时固定
    node_info.class_number = 1;
    node_info.device_number = 1;
    send_ch = node_info.class_number * NEBULA_GROUP_NUM_IN_CLASS + node_info.device_number%(NEBULA_GROUP_NUM_IN_CLASS - 1) + 1;
    //send_ch = 1;
    rcv_ch = node_info.class_number * NEBULA_GROUP_NUM_IN_CLASS;


    init_em_module();
    nebula_init_md_write_buffer();
    em9203_ops.status=NEBULA_STATUS_OFFLINE;
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
        while((SPI_CONTROLER.Instance->SR & SPI_FLAG_TXE)==0);

        SPI_CONTROLER.Instance->DR = send_buf[cnt];

        while((SPI_CONTROLER.Instance->SR & SPI_FLAG_RXNE)==0);

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
        while((SPI_CONTROLER.Instance->SR & SPI_FLAG_TXE)==0);

        SPI_CONTROLER.Instance->DR = send_buf[cnt];

        while((SPI_CONTROLER.Instance->SR & SPI_FLAG_RXNE)==0);

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
    //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);	  //不明所以 测量？
    //Asm_Delay_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1));
    //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    EM92013_SPI_write(0x04, 0x03);
    nebula_send_data_flag = 1;
}


static void init_em_module(void)
{
    EM92013_SPI_write(0x01, 0xFF);                                               //clear all IRQ
    EM92013_SPI_write(0x00, 0xFF);

    EM92013_SPI_write(0x1A, 0xB3);                                              //SW reset
    EM92013_SPI_write(0x1A, 0x5E);
    while(!send_module_int_flag);                                               //应该增加超时检测和重试
    send_module_int_flag = 0;
    EM92013_SPI_write(0x01, 0xFF);                                               //
    EM92013_SPI_write(0x00, 0xFF);

    EM92013_SPI_write(0x02, 0x00);                               //
    EM92013_SPI_write(0x03, 0x01);                               //enable auto-calibaration interrupt

    EM92013_SPI_write(0x07, 0x13);                               //set channel for autocalibration
    
    EM92013_SPI_write(0x06, 0x3F);                               //set maximum output power, set 2Mbps, and start auto-calibration, 0x37 for EM9201 1Mbps, 0x3F for EM9203 2Mbps
    while(!send_module_int_flag);
    send_module_int_flag = 0;
    EM92013_SPI_write(0x01, 0xFF);                               //clear all IRQ
    EM92013_SPI_write(0x00, 0xFF);

    EM92013_SPI_write(0x04, 0x00);                               //go back standby mode manually, not mandatory, as device will do this automatically.


    EM92013_SPI_write(0x07, rcv_ch);                           //set communication channel  设置接收信道
    EM92013_SPI_write(0x06, 0x1E);                                               //2Mbps , +3dBm
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
    send_module_int_flag = 0;
}

void start_em_recive(void)
{
	 EM92013_SPI_write(0x04, 0x02);  
	
}

//em9203等待中断 并清除标志
static void em_wait_and_clear_int()
{
    while(!send_module_int_flag);
    send_module_int_flag = 0;
    EM92013_SPI_write(0x00,0xff);
}

void nebula_vote_result_send(uint8_t rst)
{
    if(NEBULA_STATUS_VOTE == em9203_ops.status)   //如果是投票模式
    {
        st_nebula_report_data report;
        uint8_t module_status = 0;

        EM92013_SPI_write(0x04, 0x00);
        EM92013_SPI_write(0x07, send_ch);  //设置发射信道
        report.device_id = node_info.device_number;  //设置设备id
        report.report_id = NEBULA_REPORT_VOTE; //设置报告id 投票模式
        report.payload_lenth = 1;  //长度
        report.payload[0] = rst;  //内容

        EM92013_SPI_write_fifo((uint8_t *)&report, report.payload_lenth + 2); //长度对？
        em_wait_and_clear_int();
        EM92013_SPI_read(0x04, &module_status);  //读取模块状态
        while(module_status == 0x03)
        {
            EM92013_SPI_read(0x04, &module_status);
        }
        EM92013_SPI_write(0x07, rcv_ch);
        EM92013_SPI_write(0x04, 0x02);
    }
}

void nebula_poll_handle()
{
    if(IS_Elapsed(em9203_sta_timer))
    {

        nebula_status_check();
        start(em9203_sta_timer,10); //10毫秒

    }
	   
}

//中断回调函数
void Em9203_Int_Callback(void)
{
	
    uint8_t int_status = 0;   
    send_module_int_flag = 1;
    
    EM92013_SPI_read(0x00,&int_status);
    if(0x80 & int_status)
    {   
        EM92013_SPI_read(0x15,&rcv_len); 
        rcv_len++;
        EM92013_SPI_read_fifo((uint8_t *)&gateway_poll_data, rcv_len);
        EM92013_SPI_write(0x16,0x10);
        send_module_int_flag = 0;
        EM92013_SPI_write(0x00,0xff);

        em9203_ops.status = gateway_poll_data.status;
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
                Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1));  //这个最大是60ms
                                
                                
            }
        }
        else if(NEBULA_STATUS_VOTE == em9203_ops.status)
        {
            
        }
        else if(NEBULA_STATUS_MASSDATA == em9203_ops.status)
        {
            if(NEBULA_CMD_ACK == gateway_poll_data.command)
            {
                EM92013_SPI_write(0x04, 0x00);
                EM92013_SPI_write(0x07, send_ch);
                nebula_back_data_flag = 1;
                //RED_LED_ON();
                Set_Hard_timer_us(NEBULA_NODE_SEND_DELAY_UINT_US * (node_info.device_number + 1)); 
            }
            else
            {
                if(gateway_poll_data.ack_status_0_31 & (1 << node_info.device_number))
                {
                    nebula_move_buffer_point();
                  
                                
                }
                else
                {
              
                }
            }
        }
        
    }
    else if(0x40 & int_status)
    {
        if(NEBULA_STATUS_VOTE != em9203_ops.status)
        {
            send_module_int_flag = 0; 
            EM92013_SPI_write(0x00,0xff);
            if(nebula_send_data_flag)
            {
                //sent over
                EM92013_SPI_read(0x04, &module_status);
                while(module_status == 0x03)
                {
                        EM92013_SPI_read(0x04, &module_status);
                }
                EM92013_SPI_write(0x07, rcv_ch);
                EM92013_SPI_write(0x04, 0x02);
                nebula_send_data_flag = 0;
            }
        }
    }

	
	
}

void	Hard_Timer_Us_Callback(void)
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
            else if(NEBULA_STATUS_MASSDATA == em9203_ops.status)
            {
                report.device_id = node_info.device_number;
                report.report_id = NEBULA_REPORT_WRITING;
                report.payload_lenth = nebula_read_md_data(report.payload, 30);
                EM92013_SPI_write_fifo((uint8_t *)&report, report.payload_lenth + 2);

                //RF send but do not sent over
            }
            nebula_back_data_flag = 0;
        }



    }		
	
	
	
	
}

