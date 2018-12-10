#ifndef NRF518XX_H_
#define NRF518XX_H_
#include "stdint.h"


#define OVERTINE_ERR   0xff
#define  OPEN_BLE_POWER()    HAL_GPIO_WritePin(BT_PWR_CTRL_GPIO_Port, BT_PWR_CTRL_Pin, GPIO_PIN_SET)
#define  CLOSE_BLE_POWER()   HAL_GPIO_WritePin(BT_PWR_CTRL_GPIO_Port,BT_PWR_CTRL_Pin, GPIO_PIN_RESET)
#define  BLE_SELECT()        HAL_GPIO_WritePin(BT_SPI_CS_GPIO_Port,BT_SPI_CS_Pin, GPIO_PIN_RESET)
#define  BLE_RELEASE()       HAL_GPIO_WritePin(BT_SPI_CS_GPIO_Port,BT_SPI_CS_Pin, GPIO_PIN_SET)
#define  IF_HAS_DATA()      HAL_GPIO_ReadPin(BT_INT_GPIO_Port,BT_INT_Pin)==GPIO_PIN_RESET
typedef struct  
{

	uint8_t ready;
	uint8_t status;
  void (* Open)      (void);
	void (*Close)     (void);
  uint8_t (* Ioctl)(uint8_t cmd,uint8_t *);
  uint8_t (* Read)(uint8_t *, uint8_t );   
	uint8_t (*Write)  (uint8_t opcode, uint8_t * data, uint16_t len) ;
	void    (*manager)   (void);
	void  (*clear_buff)  (void);
	uint8_t (*Write_data)  (uint8_t *data ,uint8_t len);
	
	
} BLE518XX_OPS;

typedef struct  {
	
	uint8_t HEAD;
	uint8_t CMD;
	uint8_t LEN;
	uint8_t message[30];
	

}PACKET_ST;



typedef struct  {
	
	PACKET_ST packet[16];
	uint8_t mesage_num;
	
}RESPONSE_BOX;




enum COMMANDS{
	
	QUERY_READY=0x10,  //��ѯ����
	QUERY_FW_VERSION,  //��ѯ�洢�Ĺ̼��汾
	SET_FW_VERSION,    //����mcu�Ĺ̼��汾
	SET_MODE_CONNECT,  //���ÿ�����ģʽ
	SET_MODE_PARING,    //�������ģʽ
  SEND_HOST,          //�������ݵ�host
	
};


enum RESPONSE{
	
	RESPONSE_READY=0x20,  //��Ӧ����
	RESPONSE_OK,       //�����ɹ�
  RESPONSE_STA,      //��Ӧ����״̬
	RESPONSE_HOST,     //HOST ��������
	RESPONSE_ERROR ,     //����ʧ��
	
	
	
};

enum ERROR{
	
	TX_BUFF_FULL,
	STATE_ERR,
	
};
void Open_ble(void);
void  Close_ble(void);
uint8_t Ioctl_ble(uint8_t cmd,uint8_t *para);
uint8_t Read_ble(uint8_t *msg,uint8_t len);
uint8_t Write_ble(uint8_t opcode, uint8_t * data, uint16_t len);
void ble_clear_buff(void);
void manager_ble(void);
uint32_t  send_msg_to_ble(uint8_t *data ,uint8_t len);
uint8_t ble_direct_write (uint8_t *data ,uint8_t len);

static void ble_rw(uint8_t * p_tx_data, uint8_t * p_rx_data);
static void Create_Packet(uint8_t cmd,uint8_t* pyload,uint8_t len);
uint8_t Wait_Response(uint32_t  time);
void Ble_Response_callback(void);
uint8_t send_ble_data_buffer_to_host(void);


#endif



