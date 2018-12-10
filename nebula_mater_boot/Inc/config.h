#ifndef __CONFIG_H_
#define __CONFIG_H_
#include "stdint.h"


typedef struct OTA_INFO_
{
    unsigned int   firmware_checksum;
    unsigned int   firmware_length;
    unsigned int   version;
    unsigned char ota_state;
    unsigned char nordic_ota_flag : 1;
    unsigned char rk_ota_finish : 1;
    unsigned char ti_ota_finish : 1;
}OTA_INFO;

#define MAX_OTA_PAYLOAD_LEN             16
#define OTA_RCV_MAX_BUFF_LEN            1024

#define SW_VERSION                      (0x00000001)
#define BLE_BUF_NO                      32
#define ORG_POS_DATA_BUFF_COUNT         3   //坐标缓冲包个数
#define LOW_POWER_TO_POWER_OFF_TIME     (20*60*1000/10)   // 20min
#define BATTERY_DET_COUNTDOWN           ((5*1000)/10) //1 MIN

//过滤坐标防止飞笔
#define ORG_POSITION_MAX_CHANGE_NUM      1400


#define BLE_MAX_PAYLOAD                   20
//离线存储的最大笔记号
#define MAX_NOTE_ID                      100
enum
{
    OTA_STATE_NONE,
    OTA_CLEAR_FLASH,
    OTA_INIT,
    OTA_TI_DOWNLOAD,
    OTA_RK_DOWNLOAD,
    OTA_BT_DOWNLOAD,
    OTA_BT_WIRTE_LAST,
    OTA_BT_DOWNLOAD_DONE,
    OTA_BT_CHECKSUM_VERIFY,
    OTA_SWITCH_RK,
    OTA_SWITCH_NORDIC,
    OTA_FINISH,
};


enum
{
    DEVICE_POWER_OFF,
    DEVICE_STANDBY,
    DEVICE_INIT_BTN,
    DEVICE_OFFLINE,
    DEVICE_ACTIVE,
    DEVICE_LOW_POWER_ACTIVE,
    DEVICE_OTA_MODE,//06
    DEVICE_OTA_WAIT_SWITCH,
    DEVICE_TRYING_POWER_OFF,
    DEVICE_FINISHED_PRODUCT_TEST,
    DEVICE_SYNC_MODE,
    DEVICE_SEMI_FINISHED_PRODUCT_TEST,
    
};
enum
{
    INITIALIZATION = 0x00,
    BLE_PAIRING          = 0x01,
    BLE_RECONNECTING          = 0x02,
    BLE_CONNECTED  = 0x03,
    BLE_STANDBY  = 0x04
};

enum
{
    BLE_CMD_STATUS              =   0x80,
    BLE_CMD_POS_DATA            =   0x81,
    BLE_CMD_NAME_SET            =   0x82,
    BLE_CMD_ERROR_MSG           =   0x83,
    BLE_CMD_GET_VERSION         =   0x84,
    BLE_CMD_AUTH_REQUEST        =   0x85,
    BLE_CMD_AUTH_RESPONSE       =   0x86,
    BLE_CMD_RTC_SET             =   0x87,
    BLE_CMD_BTN_EVENT           =   0x88,
    BLE_CMD_DISPALY_PAGE        =   0x89,

    BLE_CMD_SYNC_MODE_ENTER     =   0xA0,
    BLE_CMD_SYNC_MODE_QUIT      =   0xA1,
    BLE_CMD_SYNC_FIRST_HEADER   =   0xA2,
    BLE_CMD_SYNC_ONCE_START     =   0xA3,
    BLE_CMD_SYNC_ONCE_CONTENT   =   0xA4,
    BLE_CMD_SYNC_IS_END         =   0xA5,    //结束
    BLE_CMD_SYNC_ONCE_STOP      =   0xA6,
	  BLE_CMD_SYNC_NEXT_HEADER    =   0xa7,   //查询这个笔记是否还有下一个头
    


    BLE_CMD_ENTER_OTA_MODE      =   0xB0,
    BLE_CMD_OTA_FILE_INFO       =   0xB1,
    BLE_CMD_OTA_RAW_DATA        =   0xB2,
    BLE_CMD_OTA_CHECKSUM        =   0xB3,
    BLE_CMD_OTA_RESULT          =   0xB4,
    BLE_CMD_OTA_SWTICH          =   0xB5,
    BLE_CMD_OTA_QUIT            =   0xB6, 
};


enum
{
	
	
	BUTTON_EVENT_CREATE_PAGE =0x05,
    BUTTON_EVENT_UP_PAGE     =0x03,
	BUTTON_EVENT_DOWN_PAGE   =0x04,
	
		
};



enum
{
    OTA_DFU_INIT,
    OTA_DFU_DOWNLOAD,
    OTA_DFU_VERIFY_CHECKSUM,
    OTA_DFU_WAIT_SWTICH,
    
};
enum
{
    ERROR_NONE,
    ERROR_OTA_FLOW_NUM,
    ERROR_OTA_LEN,
    ERROR_OTA_CHECKSUM,
    ERROR_OTA_STATUS,
    ERROR_OTA_VERSION,
    ERROR_NAME_CONTENT,
    ERROR_NO_NOTE,
};


enum
{
	OLED_ON=1,
	OLED_HALF_TOP,
	OLED_HALF_BOTTOM,
	OLED_OFF,
};


#pragma pack(1)

struct BLEDataFormat
{
    uint8_t identifier;
    uint8_t opcode;
    uint8_t length;
    uint8_t payload[18];//00 button init 01 cmd init 02offline running
};
typedef struct
{
    uint8_t data[8];
}stPosdata;
typedef struct
{
    uint8_t  ota_substatus;
    uint8_t  flow_num;
    uint16_t ota_store_buff_len;
    uint32_t ota_file_total_len;
    uint32_t ota_downloaded_len;
    uint32_t ota_checksum;
}st_ota_record;

typedef struct
{
    uint8_t data_count;  
    stPosdata pos_data[ORG_POS_DATA_BUFF_COUNT];
    
}stPosBuff;
typedef struct
{
    uint8_t netcard_state;
    uint8_t device_state;  
    uint8_t btn_hold_flag;
    
}stMainStatus;
typedef struct  
{
    uint8_t device_name_flag;
    uint8_t device_name_length;
    uint8_t device_name[18];
} st_device_name;
typedef struct  
{
    uint8_t display_flag;
    uint8_t display_type;
    uint8_t display_on_off;
    uint8_t display_cnt;
} st_charging_led_control;

typedef struct  
{
    uint32_t sw_version; 
    uint32_t img_length;
} st_ota_info;
typedef struct  
{
    uint16_t hw_version; 
    uint32_t fw_version;
} st_version_info;

typedef struct  
{
    uint8_t y_h : 5;
    uint8_t store_flag : 3;
    uint8_t y_l;
    uint8_t x_h;
    uint8_t x_l;
    uint8_t press;
} st_store_info;

typedef struct  
{
    uint8_t data[5];

} st_store_packet;


typedef struct
{
    uint16_t identifier;
    uint8_t dtm_mode_flag;//test flag,used for record if the device has be connected 
    st_device_name name; //20 bytes
    uint8_t  ble_addr[7];//7 bytes
    uint8_t auto_poweron;  //开机自启动
    uint32_t hardware_version;//0x00028c20
    uint32_t firmware_version;
    uint32_t stored_total_note_num;  //未同步的笔记数量
    uint32_t note_read_start_sector;
    uint32_t note_read_end_sector;
    uint8_t error_code;
	uint32_t note_index;   //最新创建的笔记id
	uint16_t  link_code;  //连接特征码
	uint16_t  align  ;   //这个是为了4字节对齐
    
} st_device_data_storage;


#pragma pack()







#endif

