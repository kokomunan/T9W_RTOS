
#ifndef _EWM_3080B_H_
#define _EWM_3080B_H_


#include "stdint.h"

#define VERSION_LEN  16
#define RETURN_DATA_LEN    64
#define TRANS_MTU      64
#define TRANS_PALOAD_MAX_LEN  TRANS_MTU-4

#define QUEUE_MAX_NUM         100

#define OPEN_EWM3080_POWER()    HAL_GPIO_WritePin(WIFI_PWR_CTRL_GPIO_Port,WIFI_PWR_CTRL_Pin, GPIO_PIN_SET)  
#define CLOSE_EWM3080_POWER()  HAL_GPIO_WritePin(WIFI_PWR_CTRL_GPIO_Port,WIFI_PWR_CTRL_Pin, GPIO_PIN_RESET)  

#define LOW    HAL_GPIO_WritePin(WIFI_REVER_Port,WIFI_REVER_Pin, GPIO_PIN_RESET)  
#define HIGH    HAL_GPIO_WritePin(WIFI_REVER_Port,WIFI_REVER_Pin, GPIO_PIN_SET)  



#define AP_SSID_KEY   "robot_office,robot123"

//#define TCP_ADR         "0,tcp_client,192.168.1.221,6001" 
#define TCP_ADR         "0,tcp_client,192.168.1.177,8080" 


#define LOCAL_SERVER_IP    "192.168.1.254"
#define LOCAL_PORT         "8080"
#define TCP_ID              "0"

#define MQTT_ID 
#define MQTT_USERID
#define MQTT_KEY
#define MQTT_IP
#define MQTT_PORT
#define MQTT_HEART_RATE
#define MQTT_PUB_TOPIC
#define MQTT_SUB_TOPIC 


#define ALINK_PRODUCT_NAME
#define ALINK_PRODUCT_MOUDLE
#define ALINK_PRODUCT_KEY
#define ALINK_PRODUCT_SECRET
#define ALINK_DATA_FORMAT  
#define ALINK_DEVICE_TYPE
#define ALINK_DEVICE_CATEGORY
#define ALINK_DEVICE_MANUFACTURER   


typedef struct  
{

	uint8_t     ready;
	uint8_t     status;
    void        (* Open)        (void);
	void        (*Close)        (void);
    uint8_t     (* Ioctl)       (uint8_t cmd,uint8_t *);
    uint8_t     (* Read)        (uint8_t *, uint8_t );   
	uint8_t     (*Write)        (uint8_t opcode, uint8_t * data, uint16_t len) ;
	void        (*manager)      (void);
	void        (*clear_buff)   (void);
	uint8_t     (*Write_data)   (uint8_t *data ,uint8_t len);
	
	
} EWM3080_OPS;

typedef struct  
{
    
    uint8_t mac[6];
    uint8_t version[VERSION_LEN];
    uint32_t mem_free;
    uint8_t call_result;
    uint8_t wifi_status;
    uint8_t last_status;
    uint8_t return_data[RETURN_DATA_LEN];
    uint8_t     (* Recive_pipe)        (uint8_t *data, uint8_t len);   
	uint8_t     (*send_pipe)           (uint8_t * data, uint8_t len) ;
    uint8_t     (*close_pipe)(void);
    
    
    
}EWM3080_handle;

struct WIFI_PACKET
{
    uint8_t identifier;
    uint8_t opcode;
    uint8_t index;
    uint8_t length;
    uint8_t payload[TRANS_PALOAD_MAX_LEN];
};



enum{
   
  CONFIG_UART,      
  START_STATION,
  START_TCP,
  START_MQTT,
  START_ALINK,
    
  
    
};



enum{
    
    STATION_DOWN,
    STATION_CONNECTING,
    STATION_UP,    
    TCP_CONNECT,
    TCP_CONNECTING,
    TCP_DISCONNCET,  
    MQTT_CONNCET,
    MQTT_CONNCETING,
    MQTT_DISCONNCET,
   
      
    
};


enum{
    
  CREATE_INIT_PROCESS,  
    
    
};

enum{
    
PIPE_DISCONNECT,
PIPE_CONNECT,    
    
    
    
};


void Open_wifi(void);
void Close_wifi(void);
uint8_t Ioctl_wifi(uint8_t cmd,uint8_t *para);
uint8_t Read_wifi(uint8_t *msg,uint8_t len);
uint8_t Write_wifi(uint8_t opcode, uint8_t * data, uint16_t len);
void manager_wifi(void);
uint8_t wifi_direct_write (uint8_t *data ,uint8_t len);
void wifi_clear_buff(void);
void process_loop(const void *argument);

void STATION_UP_callback(void);

void STATION_DOWN_callback(void);

void TCP_SERVER_CONNECTED_callback(void);

void TCP_SERVER_CLOSED_callback(void);

void TCP_SERVER_DISCONNECTED_callback(void);
void MQTT_CONNECT_SUCCESS_callback(void);


void MQTT_CONNECT_FAIL_callback(void);


void MQTT_CONNECT_RECONNECTING_callback(void);


void MQTT_CLOSE_SUCCESS_callback(void);


void MQTT_CLOSE_FAIL_callback(void);


void MQTT_SUBSCRIBE_SUCCESS_callback(void);


void MQTT_SBUSCRIBE_FAIL_callback(void);

void MQTT_PUBLISH_SUCCESS_callback(void);


void MQTT_PUBLISH_FAIL_callback(void);


void ALINK_STATUS_callback(uint8_t status);


void OTA_START_callback(void);


void OTA_END_callback(void);




uint8_t start_station_up(void);
uint8_t stop_station_up(void);
uint8_t start_alink_client(void);
uint8_t stop_alink(void);
uint8_t  start_tcp_client(void);
uint8_t stop_tcp_client(void);
uint8_t start_mqtt_client(void);
uint8_t stop_mqtt_client(void);
uint8_t tcp_send(uint8_t * data,uint8_t len);
uint8_t tcp_recive(uint8_t *data,uint8_t len);   
uint8_t HAL_WIFI_data_recive(uint8_t *data,uint8_t len);
void HAL_WIFI_return_result(uint8_t *data,uint8_t len);





#endif
