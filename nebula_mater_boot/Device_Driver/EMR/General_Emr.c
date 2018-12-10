#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "General_Emr.h"
#include "debug_log.h"
#include "gather_position.h"
extern I2C_HandleTypeDef hi2c3;
#define IIC_CONTROLER   &hi2c3

uint8_t pad_version[] = {0x00, 0x01, 0x02, 0x06, 0x03, 0x02};
uint8_t response_flag=0;
void EMR_pad_version(void)
{
    uint8_t tmp[8];
    TOUCH_PAD_ON();
    HAL_Delay(10);
		
	response_flag=0;
    if(HAL_I2C_Master_Transmit(IIC_CONTROLER,0x88,pad_version,6,10)==HAL_OK)//发送查询版本命令
	{
		
        do{

            HAL_Delay(10);

        }			
        while(response_flag==0);//等待外部中断
        HAL_I2C_Master_Receive(IIC_CONTROLER,0x88,tmp,8,10); //读取版本号
                    
        UserLog("emr pad version: [%d.%d]\r\n", tmp[5], tmp[6]);
    }
    else
    {
        UserLog("do not detected EMR\r\n");
    }
    TOUCH_PAD_OFF();
}

uint8_t Read_EMR(uint8_t *buf,uint8_t len)
{
		
	 return HAL_I2C_Master_Receive(IIC_CONTROLER,0x88,buf,8,10);
		
}



void EMR_pad_isr_callback(void)   
{
    if(response_flag==0)
    response_flag=1;
    set_read_iic_enable();	
    //read_position();
}
