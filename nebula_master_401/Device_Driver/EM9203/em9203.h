#ifndef _EM9203_H_
#define _EM9203_H_

#include "stm32f4xx_hal.h"
#include "mxconstants.h"
#include "config.h"

#define NEBULA_STATUS_OFFLINE          0
#define NEBULA_STATUS_STANDBY          1
#define NEBULA_STATUS_VOTE             2
#define NEBULA_STATUS_MASSDATA         3
#define NEBULA_STATUS_EXCEPTION        5
#define NEBULA_STATUS_MUX_VOTE         6
#define NEBULA_STATUS_FIRST_ANSWER     7

#define NEBULA_CMD_NACK             0xA0
#define NEBULA_CMD_ACK              0xA1

#define NEBULA_REPORT_SYSTICK               0
#define NEBULA_REPORT_VOTE                  1
#define NEBULA_REPORT_WRITING               2
#define NEBULA_REPORT_MULTI_VOTE            3
#define NEBULA_REPORT_FIRST_ANSWER          4

#define MD_WRITE_BUF_NO     6000
//#define MD_WRITE_BUF_NO     100
#define NEBULA_GROUP_NUM_IN_CLASS   4
#define NEBULA_NODE_SEND_DELAY_UINT_US   250
#define CONNECT_TICK_TIME      100
#define FAULT_TICK_TIME      300

#define NEBULA_SUCESS   0
#define NEBULA_FAIL     1
#define NEBULA_FULL      2
#define OPEN_EM9203_POWER()    HAL_GPIO_WritePin(EM_PWR_CTRL_GPIO_Port, EM_PWR_CTRL_Pin, GPIO_PIN_SET)  
#define CLOSE_EM9203_POWER()  HAL_GPIO_WritePin(EM_PWR_CTRL_GPIO_Port, EM_PWR_CTRL_Pin, GPIO_PIN_RESET)  

#define EM9203_SELECT()   HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET)
#define EM9203_RELEASE()  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET)

#define NEBULA_NETWORK_CLASS_MAX_NUM   10

typedef struct  
{

	uint8_t     ready;   //用于标记下笔和太笔用的标志
	uint8_t     status;
    void        (* Open)        (void);
	void        (*Close)        (void);
    uint8_t     (* Ioctl)       (uint8_t cmd,uint8_t *);
    uint8_t     (* Read)        (uint8_t *, uint8_t );   
	uint8_t     (*Write)        (uint8_t opcode, uint8_t * data, uint16_t len) ;
	void        (*manager)      (void);
	void        (*clear_buff)   (void);
	uint8_t     (*Write_data)   (uint8_t *data ,uint8_t len);
	
	
} EM9203_OPS;

enum NODE_OPCODE{
	
	VOTE_OPS,
	MASS_OPS,
    PAGE_OPS,
	
	
};



#pragma pack(1)

typedef struct
{
    uint8_t TX_CH;
    uint8_t RX0_CH;
    uint8_t RX1_CH;
    uint8_t RX2_CH;
}st_nebula_network_ch;

typedef struct
{
    uint8_t status;
    uint8_t command;
    uint32_t ack_status_0_31;
    uint32_t ack_status_32_63;
    uint32_t ack_status_64_95;
}st_nebula_send_poll_data;  //状态查询类的数据包

typedef struct
{
    uint8_t device_id;
    uint8_t report_id : 3;
    uint8_t payload_lenth : 5;
    uint8_t payload[30];
}st_nebula_report_data; //报告类的数据包

typedef struct
{
    uint8_t x_l;
    uint8_t x_h;
    uint8_t y_l;
    uint8_t y_h;
    uint8_t press;
}st_write_packet_data;  //报告坐标类的数据包




#pragma pack()

static  void init_em_module(void);
void    em9203_open(void);
void    em9203_close(void);
uint8_t em9203_ioctl(uint8_t cmd,uint8_t *param);
uint8_t em9203_read(uint8_t *data, uint8_t len);
uint8_t em9203_write(uint8_t opcode, uint8_t * data, uint16_t len);
void    em9203_manage(void);
void    em9203_clearbuff(void);
void    restore_from_fault(void);



void    nebula_init_md_write_buffer(void);
void    nebula_status_check(void);
uint8_t nebula_init_node_device(st_nebula_node_device_info *info);
uint8_t nebula_fill_md_write_data_to_buf(st_write_packet_data * data);
uint8_t check_nebula_status(void);
void    nebula_poll_handle(void);
void    nebula_vote_result_send(uint8_t *rst);
void    Em9203_Int_Callback(void);
void    Hard_Timer_Us_Callback(void);
uint8_t  check_em9203(void);
uint8_t  start_test_2g4_net(void);
void     quit_test_2g4_net(void);
uint8_t  get_em9203_last_status(void);
#endif

