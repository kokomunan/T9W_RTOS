#ifndef __CONFIG_H_
#define __CONFIG_H_
#include "stdint.h"


//只试用2.4g
//#define   ONLY_SUPORT_LAN    
//打开flash fifo 用于存储异常时的数据
//#define ONLY_SUPORT_BLE 
#define    USE_FLASH_FIFO
//是否打开低功耗管理
#define ENABLE_LOW_POWER_MANAGER


#define SW_VERSION                      (39)
#define HW_VERSION                       12
#define MAX_OTA_PAYLOAD_LEN             16
#define OTA_RCV_MAX_BUFF_LEN            1024
#define BLE_BUF_NO                      32


#define ORG_POS_DATA_BUFF_COUNT         5   //坐标缓冲包个数
//进入低功耗的倒计时
#define LOW_POWER_TO_POWER_OFF_TIME     (20*60*1000/10)   // 20min
//电池电压检测周期
#define BATTERY_DET_COUNTDOWN           ((5*1000)/10) //5s

//异常超时时间
#ifdef USE_FLASH_FIFO
#define EXCEPTION_MAX_TIME                5*1000    //5s
#else
#define EXCEPTION_MAX_TIME                 30*1000     //30s
#endif

//过滤坐标防止飞笔
#define ORG_POSITION_MAX_CHANGE_NUM      1400


#define BLE_MAX_PAYLOAD                   20
//离线存储的最大笔记号
#define MAX_NOTE_ID                      100

//#define MAX_NOTE_ID                      10

//设备状态
//#define TX_POWER_3DB            
//#define SHOW_MODE

#define MAX_VOTE_NUM      6

enum
{
    DEVICE_POWER_OFF, //0
    DEVICE_STANDBY,
    DEVICE_INIT_BTN,
    DEVICE_OFFLINE,
    DEVICE_ACTIVE,
    DEVICE_LOW_POWER_ACTIVE,
    DEVICE_OTA_MODE,//06
    DEVICE_OTA_WAIT_SWITCH,  
    DEVICE_TRYING_POWER_OFF,
    DEVICE_NRF_TEST,
    DEVICE_SYNC_MODE,
    DEVICE_DFU_MODE, //11
    SENSOR_UPDATE,
    SENSOR_CALIBRA,
    
};
//蓝牙状态
enum
{
    INITIALIZATION      = 0x00,
    BLE_PAIRING         = 0x01,
    BLE_RECONNECTING    = 0x02,
    BLE_CONNECTED       = 0x03,
    BLE_DFU             = 0x04,
};
//蓝牙接口命令
enum
{
    CMD_STATUS              =   0x80,
    CMD_POS_DATA            =   0x81,
    BLE_CMD_NAME_SET        =   0x82,
    CMD_ERROR_MSG           =   0x83,
    CMD_GET_VERSION         =   0x84,
    CMD_AUTH_REQUEST        =   0x85,
    CMD_AUTH_RESPONSE       =   0x86,
    CMD_RTC_SET             =   0x87,
    CMD_BTN_EVENT           =   0x88,
    CMD_DISPALY_PAGE        =   0x89,
    CMD_REPORT_PAGE_AUTO    =   0x8a,
    CMD_REQUST_PAGE         =   0x8b,

    CMD_SYNC_MODE_ENTER     =   0xA0,
    CMD_SYNC_MODE_QUIT      =   0xA1,
    CMD_SYNC_FIRST_HEADER   =   0xA2,
    CMD_SYNC_ONCE_START     =   0xA3,
    CMD_SYNC_ONCE_CONTENT   =   0xA4,
    CMD_SYNC_IS_END         =   0xA5,    //结束
    CMD_SYNC_ONCE_STOP      =   0xA6,
    CMD_SYNC_DATA           =   0xa8,
    

	CMD_ENTER_OTA_MODE      =   0xB0,
    CMD_OTA_FILE_INFO       =   0xB1,
    CMD_OTA_RAW_DATA        =   0xB2,
    CMD_OTA_CHECKSUM        =   0xB3,
    CMD_OTA_RESULT          =   0xB4,
    CMD_OTA_SWTICH          =   0xB5,
    CMD_OTA_QUIT            =   0xB6, 
    
    CMD_REQUIRE_MAC          =	0xc0,
	CMD_REQUIRE_NAME         =	0xc1,
    
    CMD_ENTER_UPDATA_EMR_MODE   =   0xd0,
    CMD_OTA_EMR_FILE_INFO       =   0xd1,
    CMD_OTA_EMR_RAW_DATA        =   0xd2,
    CMD_OTA_EMR_CHECKSUM        =   0xd3,
    CMD_OTA_EMR_RESULT          =   0xd4,
    CMD_OTA_EMR_SWTICH          =   0xd5,
    CMD_OTA_EMR_QUIT            =   0xd6, 
    CMD_GET_PAD_VERSION         =   0xd7,
    CMD_ENTER_FRQ_ADJUST        =   0xd8,
    CMD_QUIT_FRQ_ADJUST         =   0xd9,
    
    
};
//usb 接口命令
enum
{
    USB_CMD_OPEN_USB        =   0x10,
    USB_CMD_CLOSE_USB       =   0x11,
    
    
    USB_CMD_GET_STATUS          =0x20,
    USB_CMD_REPORT_ERROR        =0x26,   
    USB_CMD_REPORT_MODE         =0x27,
    USB_CMD_GET_DEVICE_INFO     =0x28,
    USB_CMD_RESET_DEFAULT       =0x29,
    USB_CMD_RESET_BLE_DEFAULT   =0x2a,
    USB_CMD_OLED_TEST_ENTER     =0x2b,
    USB_CMD_OLED_FULL           =0x2c,
    USB_CMD_OLED_BLANK          =0x2d,
    USB_CMD_OLED_TOGLE          =0x2e,
    USB_CMD_OLED_TEST_QIUT      =0x2f,
    
    
    USB_CMD_CONFIG_ADDR          =0x30,   
    USB_CMD_ENTER_DFU            =0x31,
    USB_CMD_GET_FW_INFO          =0x32,
    USB_CMD_GET_RAW_DATA         =0x33,
    USB_CMD_GET_CHEKSUM         =0x34,
    USB_CMD_REPORT_RESULT       =0x35,
    USB_CMD_NOTIFY_RESET        =0x36,
    USB_CMD_QUIT_DFU            =0x37,
    
   
    USB_CMD_REPORT_POSITION     =0x42,
    USB_CMD_RTC_SET             =0x43,
    USB_CMD_BTN_EVENT           =0x44,
    USB_CMD_SET_NAME            =0x45,
    USB_CMD_DISPALY_PAGE        =0x46,
    USB_CMD_REPORT_PAGE         =0x47,
    USB_CMD_GET_PAD_VERSION     =0x4a,
    
    
    USB_CMD_SYNC_MODE_ENTER     =0x50,
    USB_CMD_SYNC_MODE_QUIT      =0x51,
    USB_CMD_SYNC_FIRST_HEADER   =0x52,
    USB_CMD_SYNC_START          =0x53,
    USB_CMD_SYNC_DATA           =0x54,
    USB_CMD_SYNC_IS_END         =0x55,

    USB_CMD_TEST_MODE_ENTER     =0x60,
    USB_CMD_TEST_MODE_QUIT      =0x61,
    USB_CMD_ENTER_FRQ_ADJUST    =0x62,
    USB_CMD_QUIT_FRQ_ADJUST     =0x63,
    
    USB_CMD_ENTER_UPDATA_EMR_MODE   =   0x70,
    USB_CMD_OTA_EMR_FILE_INFO       =   0x71,
    USB_CMD_OTA_EMR_RAW_DATA        =   0x72,
    USB_CMD_OTA_EMR_CHECKSUM        =   0x73,
    USB_CMD_OTA_EMR_RESULT          =   0x74,
    USB_CMD_OTA_EMR_SWTICH          =   0x75,
    USB_CMD_OTA_EMR_QUIT            =   0x76, 


    
   
    
};

//usb 接口命令
enum
{

    
    
    WIFI_CMD_GET_STATUS          =0x20,
    WIFI_CMD_REPORT_ERROR        =0x26,   
    WIFI_CMD_REPORT_MODE         =0x27,
    WIFI_CMD_GET_DEVICE_INFO     =0x28,
    WIFI_CMD_RESET_DEFAULT       =0x29,
    WIFI_CMD_RESET_BLE_DEFAULT   =0x2a,

    
    
    WIFI_CMD_CONFIG_ADDR          =0x30,   
    WIFI_CMD_ENTER_DFU            =0x31,
    WIFI_CMD_GET_FW_INFO          =0x32,
    WIFI_CMD_GET_RAW_DATA         =0x33,
    WIFI_CMD_GET_CHEKSUM         =0x34,
    WIFI_CMD_REPORT_RESULT       =0x35,
    WIFI_CMD_NOTIFY_RESET        =0x36,
    WIFI_CMD_QUIT_DFU            =0x37,
    
   
    WIFI_CMD_REPORT_POSITION     =0x42,
    WIFI_CMD_RTC_SET             =0x43,
    WIFI_CMD_BTN_EVENT           =0x44,
    WIFI_CMD_SET_NAME            =0x45,
    WIFI_CMD_DISPALY_PAGE        =0x46,
    WIFI_CMD_REPORT_PAGE         =0x47,
    WIFI_CMD_GET_PAD_VERSION     =0x4a,
    WIFI_CMD_REPORT_MAC          =0x4b,
    
    
    WIFI_CMD_SYNC_MODE_ENTER     =0x50,
    WIFI_CMD_SYNC_MODE_QUIT      =0x51,
    WIFI_CMD_SYNC_FIRST_HEADER   =0x52,
    WIFI_CMD_SYNC_START          =0x53,
    WIFI_CMD_SYNC_DATA           =0x54,
    WIFI_CMD_SYNC_IS_END         =0x55,

    WIFI_CMD_TEST_MODE_ENTER     =0x60,
    WIFI_CMD_TEST_MODE_QUIT      =0x61,
    WIFI_CMD_ENTER_FRQ_ADJUST    =0x62,
    WIFI_CMD_QUIT_FRQ_ADJUST     =0x63,
    
    WIFI_CMD_ENTER_UPDATA_EMR_MODE   =   0x70,
    WIFI_CMD_OTA_EMR_FILE_INFO       =   0x71,
    WIFI_CMD_OTA_EMR_RAW_DATA        =   0x72,
    WIFI_CMD_OTA_EMR_CHECKSUM        =   0x73,
    WIFI_CMD_OTA_EMR_RESULT          =   0x74,
    WIFI_CMD_OTA_EMR_SWTICH          =   0x75,
    WIFI_CMD_OTA_EMR_QUIT            =   0x76, 


    
   
    
};

//固件索引序号
enum
{
    
    GATEWAY_FW_NUM      =0x0,
    NODE_MCU_FW_NUM     =1,
    NODE_BLE_FW_NUM     =2,
    JEDI_A4_FW_NUM      =3,
    JEDI_A5_FW_NUM      =4,

};
//dfu 步骤
enum
{
    START_DFU,
    GET_BLE_FW_INFO,
    DOWNLOAD_BLE_FW,
    GET_BLE_FW_CHECK,
    GET_MCU_FW_INFO,
    DOWNLOAD_MCU_FW,
    GET_MCU_FW_CHECK,
    FINISH_DFU,
    
    
};

//按键事件
enum
{
	
	
	BUTTON_EVENT_CREATE_PAGE    =0x05,
    BUTTON_EVENT_UP_PAGE        =0x03,
	BUTTON_EVENT_DOWN_PAGE      =0x04,
	
		
};
//OTA步骤
enum
{
    OTA_DFU_INIT,
    OTA_WAIT_BLE,
    OTA_GET_MCU_LEN,
    OTA_DOWNLOAD_MCU_FW,
    OTA_GET_MCU_CHECKSUM,
    OTA_FINISH,
    
};
//错误码
enum
{
    ERROR_NONE,
    ERROR_FLOW_NUM,
    ERROR_FW_LEN,
    ERROR_FW_CHECKSUM,
    ERROR_STATUS,
    ERROR_VERSION,
    ERROR_NAME_CONTENT,
    ERROR_NO_NOTE,
    ERROR_TEST_FAILED,
};

//oled 测试指令
enum
{
	OLED_ON=1,
	OLED_HALF_TOP,
	OLED_HALF_BOTTOM,
	OLED_OFF,
};


#pragma pack(1)
//ble 传输包格式
struct BLEDataFormat
{
    uint8_t identifier;
    uint8_t opcode;
    uint8_t length;
    uint8_t payload[61];//00 button init 01 cmd init 02offline running
};
//usb 传输包格式
typedef struct {
    uint8_t identifier;
    uint8_t opcode;
    uint8_t device_id;
    uint8_t length;
    uint8_t payload[60];//00 button init 01 cmd init 02offline running 
    
    
}stUSB_PACKET;

typedef struct {
    uint8_t device_status;
    uint16_t mcu_version;
    uint16_t ble_version;
    uint8_t  custom_num;
    uint8_t  class_num;
    uint8_t  device_num;
   
}stUSB_STATUS_PACKET;



typedef struct {
    uint16_t mcu_version;
    uint16_t ble_version;
    uint8_t  custom_num;
    uint8_t  class_num;
    uint8_t  device_num;
    uint8_t  mac[6];
    uint8_t  hard_num;
   
}stUSB_DEVICE_BASIC_INFO;



//坐标格式
typedef struct
{
    uint8_t data[8];
}stPosdata;
//坐标缓冲池
typedef struct
{
    uint8_t data_count;  
    stPosdata pos_data[ORG_POS_DATA_BUFF_COUNT];
    
}stPosBuff;
//状态结构
typedef struct
{
    uint8_t server_state;
    uint8_t device_state;  
    uint8_t btn_hold_flag;
    
}stMainStatus;
//设备名结构
typedef struct  
{
    uint8_t device_name_flag;
    uint8_t device_name_length;
    uint8_t device_name[18];
} st_device_name;
//充电显示灯控制体
typedef struct  
{
    uint8_t display_flag;
    uint8_t display_type;
    uint8_t display_on_off;
    uint8_t display_cnt;
} st_charging_led_control;
//离线笔记存储结构
typedef struct  
{
    uint8_t y_h : 6;
    uint8_t store_flag : 2;
    uint8_t y_l;
    uint8_t x_h;
    uint8_t x_l;
    uint8_t press;
} st_store_info;

typedef struct  
{
    uint8_t data[5];

} st_store_packet;
//节点地址结构
typedef struct
{
    //customer: flag for customer
    uint8_t customer_number;
    //class: 0~19
    uint8_t class_number;
    uint8_t device_number;
    uint8_t resver;
}st_nebula_node_device_info;

//服务记录结构
typedef struct
{
  uint8_t last_serve;
  uint8_t second_serve;
  
}st_server_info;

//设备存储的信息

typedef struct
{
   uint16_t mcu_firmware_version; 
   uint8_t  ble_addr[6];//6 bytes   
   uint16_t hard_version;
    
}st_notify_ble_info;
typedef struct
{
   uint16_t jedi_version;
   uint8_t  surport_calibra_flag; 
    
}st_jedi_info;

typedef struct
{
    uint16_t identifier;//2
    uint8_t dtm_mode_flag;//test flag,used for record if the device has be connected  1
    st_device_name name; //20 bytes
    uint8_t  ble_addr[7];//7 bytes
    uint8_t auto_poweron;  //开机自启动  1
    //uint32_t hardware_version;//0x00028c20
    uint16_t mcu_firmware_version; //固件版本   2
    uint16_t ble_firmware_version;   //2
    uint32_t stored_total_note_num;  //未同步的笔记数量  4
    uint32_t note_read_start_sector;  // 4
    uint32_t note_read_end_sector;   //4
    uint8_t error_code;              //1
	uint32_t note_index;   //最新创建的笔记id 4
	//uint16_t  link_code;  //连接特征码
    st_nebula_node_device_info  node_info;   //4
    st_server_info serve_info;   //2
	uint16_t  hard_version  ;   //这个是为了4字节对齐  2
} st_device_data_storage;


#pragma pack()







#endif

