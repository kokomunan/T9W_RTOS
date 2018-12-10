
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "norflash.h"
#include "debug_log.h"

extern SPI_HandleTypeDef hspi2;
#define  SPI_CONTROLER    &hspi2
#define  NORFLASH_LOG   debug_log


uint8_t ex_flash_ok=0;   //检测到flash

void flash_rw(uint8_t * p_tx_data, uint8_t * p_rx_data, uint16_t len)
{

    if(p_rx_data == NULL)
    {
       
        HAL_SPI_Transmit(SPI_CONTROLER,p_tx_data, len, 10);
			  
    }
    else
    {
      		
        HAL_SPI_TransmitReceive(SPI_CONTROLER,p_tx_data,p_rx_data, len, 10);
    }
  
}

void flash_hw_check(void)
{
   
    uint8_t send_buffer[6];
    uint8_t rcv_buffer[6];
    //write enable
    send_buffer[0] = FLASH_WAKE_UP;
    send_buffer[1] = 0;
    send_buffer[2] = 0;
    send_buffer[3] = 0;
    send_buffer[4] = 0;
	
    FLASH_SELECT();
    flash_rw(send_buffer, rcv_buffer, 5);
    FLASH_RELEASE() ;
   
    if(FLASH_DID == rcv_buffer[4])
    {
        ex_flash_ok = 1;
        UserLog("flash detected\r\n");
    }
    else
    {
        
        UserLog("flash has errors\r\n");
 
    }

}

uint8_t flash_status_check(void)
{
    uint8_t send_buffer[2];
    uint8_t rcv_buffer[2];
    //write enable
    send_buffer[0] = FLASH_READ_STATUS;
    
    FLASH_SELECT();
    flash_rw(send_buffer, rcv_buffer, 2);
    FLASH_RELEASE();
    return rcv_buffer[1];
}
//flash 写使能
void flash_write_enable(void)
{
    uint8_t send_buffer[1];
    //write enable
    send_buffer[0] = FLASH_WRITE_ENABLE;
    
    FLASH_SELECT();
    flash_rw(send_buffer, NULL, 1);
    FLASH_RELEASE();
}
//erase one sectot about 80ms in  real test
//擦除一个扇区
void flash_erase_sector(uint16_t sector_num)
{
    uint8_t send_buffer[4];//, read_buffer[4];
    uint32_t WriteAddr = sector_num * FLASH_SECTOR_SIZE;
    
    flash_write_enable();
    //earse
    send_buffer[0] = FLASH_SECTOR_ERASE;
    send_buffer[1] = (uint8_t)((WriteAddr&0x00ff0000)>>16);
    send_buffer[2] = (uint8_t)((WriteAddr&0x0000ff00)>>8);
    send_buffer[3] = (uint8_t)WriteAddr;
    
    FLASH_SELECT();
    flash_rw(send_buffer, NULL, 4);    
    FLASH_RELEASE();
	NORFLASH_LOG("erase flash sector%d",sector_num);
}
//flash 进入低功耗
void flash_enter_power_down(void)
{
    if(ex_flash_ok)
    {
        uint8_t send_buffer[1];
        //write enable
        send_buffer[0] = FLASH_DEEP_POWER_DOWN;   
        FLASH_SELECT();
        flash_rw(send_buffer, NULL, 1);
        FLASH_RELEASE();
    }
}
//from the sent over spi data, flash will return standby mode use 20us in spec
//唤醒flash
void flash_wake_up(void)
{
    if(ex_flash_ok)
    {
        uint8_t send_buffer[1];
        //write enable
        send_buffer[0] = FLASH_WAKE_UP;
        FLASH_SELECT();
        flash_rw(send_buffer, NULL, 1);
        FLASH_RELEASE();
    }
}

