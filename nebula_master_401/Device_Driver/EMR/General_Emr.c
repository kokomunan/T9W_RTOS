#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "General_Emr.h"
#include "debug_log.h"
#include "gather_position.h"
#include  "misc.h"
#include "server.h"
#include "Status_Machin.h"
#include "cmsis_os.h"
extern I2C_HandleTypeDef hi2c3;
extern uint8_t flow_number;
#define IIC_CONTROLER   &hi2c3
void reset_iic(void);
uint8_t pad_version[] = {0x00, 0x01, 0x02, 0x06, 0x03, 0x02};
uint8_t response_flag=0;
uint8_t i2c_module_open_flag=0;
uint16_t JEDI_VERSION=0;
uint8_t JEDI_SURPORT_CALIBRA_FLAG=0;
uint8_t jedi_info[8];
uint8_t emr_update_sucess=0;
uint8_t suport_update_flag=0;
void EMR_pad_version(void)
{
//    uint8_t tmp[8];
    uint8_t res;   
    TOUCH_PAD_ON();	
    
	response_flag=0;
    res=HAL_I2C_Master_Transmit(IIC_CONTROLER,0x88,pad_version,6,100);
    if(res==HAL_OK)//发送查询版本命令
	{
		
        do{

            osDelay(10);
        }			
        while(response_flag==0);//等待外部中断    
        HAL_I2C_Master_Receive(IIC_CONTROLER,0x88,jedi_info,8,10); //读取版本号 
       
        if((jedi_info[5]==0)||(jedi_info[6]==0))
        {
            jedi_info[5]=0x80;
            jedi_info[6]=0x01;
            
        }       
        UserLog("emr pad version: [%d.%d]", jedi_info[5], jedi_info[6]);
                  
        JEDI_VERSION=(jedi_info[5]<<8)|jedi_info[6];
        
        if((suport_update_flag==0)&&(JEDI_VERSION==0x8001))
        {           
            JEDI_VERSION=0x7001;
        }
        UserLog("JEDI_VERSION: [%x]", JEDI_VERSION);
        JEDI_SURPORT_CALIBRA_FLAG=jedi_info[7];
    }
    else
    {
            
        err_log("do not detected EMR\r\n");
                        
    }
    TOUCH_PAD_OFF();
}


uint8_t check_version(uint16_t version)
{
//    uint8_t tmp[8];
    uint8_t res;  
    uint16_t cur_version;
    response_flag=0;
    res=HAL_I2C_Master_Transmit(IIC_CONTROLER,0x88,pad_version,6,100);
    if(res==HAL_OK)//发送查询版本命令
	{
		
        do{

            osDelay(10);
        }			
        while(response_flag==0);//等待外部中断

        HAL_I2C_Master_Receive(IIC_CONTROLER,0x88,jedi_info,8,10); //读取版本号                    
        UserLog("emr pad version: [%d.%d]", jedi_info[5], jedi_info[6]);
        cur_version=(jedi_info[5]<<8)|jedi_info[6];
        JEDI_VERSION=(jedi_info[5]<<8)|jedi_info[6];
        JEDI_SURPORT_CALIBRA_FLAG=jedi_info[7];
        return 0; 
        
    }
    else
    {
         return 1;
    }
    
}

void set_update_flag(uint8_t flag)
{
    if(flag)
    {
        emr_update_sucess=1;
            
    }
    else
    {
        emr_update_sucess=0; 
    }
}
uint8_t IS_emr_update_sucess(void)
{
    return emr_update_sucess;
}


void clear_jedi_info(void)
{
   JEDI_VERSION=0x8001;
   JEDI_SURPORT_CALIBRA_FLAG=0;
   set_update_flag(0);
    
}

void update_jedi_info(void)
{
   check_version(0);
    
}

uint8_t Read_EMR(uint8_t *buf,uint8_t len)
{		
    uint8_t res;
    retry:       
    res=HAL_I2C_Master_Receive(IIC_CONTROLER,0x88,buf,len,50);
    if(res)
    {
        err_log("i2c recovery from fault\r\n");
        TOUCH_PAD_OFF();
        TOUCH_PAD_ON();	
        goto retry;
    }
        
    return res;

}

uint8_t Write_EMR(uint8_t cmd,uint8_t *data,uint8_t len)
{
    
    uint8_t res;
    uint8_t send_buff[32];
    //uint8_t re_buff[8];
    send_buff[0] = 0xAA;
    send_buff[1] = cmd;
    send_buff[2] = len;
    memcpy((void *)(send_buff+4),data,len);//为了对齐          
    retry:      
    res=HAL_I2C_Master_Transmit(IIC_CONTROLER,0x88,send_buff,len+4,10);
    if(res)
    {
        err_log("i2c recovery from fault\r\n");
        TOUCH_PAD_OFF();
        TOUCH_PAD_ON();	
        goto retry;
    }
        
    return res;
      
}


uint8_t ERM_cmd_handle(uint8_t *packet)
{
   uint8_t cmd=packet[1];
   uint8_t len=packet[2];
   uint8_t *data=&packet[3];
          
   switch(cmd)
   {
       case  CMD_OTA_FILE_INFO :
           
            cmd+=0x20;
            break;     
       
       case CMD_OTA_RAW_DATA:
            len=1;
            data[0]=flow_number;  
            cmd+=0x20;
       break;
       case  CMD_OTA_CHECKSUM:
           
            cmd+=0x20;
       break;
       
       case CMD_OTA_RESULT:
            
            set_update_flag(1);
            cmd+=0x20;
       break;
                 
       case CMD_ENTER_FRQ_ADJUST :
                    
            Set_Device_Status(SENSOR_CALIBRA) ;
       
       break;
       
       case  CMD_QUIT_FRQ_ADJUST :
           
       if(Get_Device_Status()==SENSOR_CALIBRA)
       {
            Set_Device_Status( DEVICE_ACTIVE) ;
       }
       break;
                  
   }  
   return server_notify_host(cmd, data,len) ;
    
}




void TOUCH_PAD_ON(void)
{
    static uint8_t count=0;
    uint8_t res;
    hi2c3.Instance = I2C3;
    hi2c3.Init.ClockSpeed = 100000;  //先用100k 
    hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_GPIO_WritePin(ANT_PWR_CTRL_GPIO_Port,ANT_PWR_CTRL_Pin,GPIO_PIN_SET);
    debug_log("i2c open");
    osDelay(200);         
    res=HAL_I2C_Init(&hi2c3);
    if(res!=0)
    {
        debug_log("fialed init");  
        i2c_module_open_flag=0;
    }
    else
    {
        i2c_module_open_flag=1;
    }
    suport_update_flag=0;
    
   while(EMR_INT)  //模组上有bUG 为了将模组一上电就发出的文件请求读出来，中断引脚变成高电平
   {
       uint8_t buff[8];
       count ++;
       HAL_I2C_Master_Receive(IIC_CONTROLER,0x88,buff,8,10); 
       if((buff[0]==0xaa)&&(buff[1]==CMD_OTA_FILE_INFO))
       {
           suport_update_flag=1;
           
       }
       
       osDelay(10); 
        if(count>3)
        {
            break;
        }  
   }

        
    
}
void TOUCH_PAD_OFF(void)
{   
    uint8_t res;
  
    HAL_GPIO_WritePin(ANT_PWR_CTRL_GPIO_Port,ANT_PWR_CTRL_Pin,GPIO_PIN_RESET);    
    debug_log("i2c close");
    osDelay(200);  
    res=HAL_I2C_DeInit(&hi2c3);
    if(res!=0)
    {
        err_log("fialed deinit\r\n");    
    }
    i2c_module_open_flag=0;
    
}
void reset_iic(void)
{
       
    HAL_I2C_DeInit(&hi2c3);    
    hi2c3.Instance = I2C3;
    hi2c3.Init.ClockSpeed = 100000;  //先用100k 
    hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;      
    HAL_I2C_Init(&hi2c3); 
       
}

void set_i2c_int(uint8_t enable)
{
    if(enable)
    {
        i2c_module_open_flag=1;
    }
    else
    {
        i2c_module_open_flag=0;
    }
    
}

void EMR_pad_isr_callback(void)   
{
    if( i2c_module_open_flag)
    {
        if(response_flag==0)
        {
            response_flag=1; 
        }
        
        if(get_server_mode()==WIFI_MODE)
        {
            
             read_position();
        }
        else
        {
              set_read_iic_enable();	  
        }
    
    }
  

}
