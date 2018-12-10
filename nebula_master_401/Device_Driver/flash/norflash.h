#ifndef __NORFLASH_H__
#define __NORFLASH_H__
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#define FLASH_MID  0xC8
#define FLASH_DID  0x15

#define FLASH_MAX_SECTOR_NUM   1024

#define FLASH_SECTOR_SIZE   0x1000
#define FLASH_END_ADDR      (FLASH_MAX_SECTOR_NUM*FLASH_SECTOR_SIZE-1)
//flash cmd
#define FLASH_WRITE_ENABLE  0x06
#define FLASH_READ_STATUS   0x05

#define FLASH_READ_DATA     0x03
#define FLASH_WRITE_PAGE    0x02
#define FLASH_SECTOR_ERASE  0x20
#define FLASH_READ_ID       0x90
#define FLASH_DEEP_POWER_DOWN       0xB9
#define FLASH_WAKE_UP               0xAB


#define  FLASH_SELECT()        HAL_GPIO_WritePin(FLASH_SPI_CS_GPIO_Port,FLASH_SPI_CS_Pin,GPIO_PIN_RESET)
#define  FLASH_RELEASE()      HAL_GPIO_WritePin(FLASH_SPI_CS_GPIO_Port,FLASH_SPI_CS_Pin,GPIO_PIN_SET)

uint8_t flash_status_check(void);
void flash_write_enable(void);
void flash_rw(uint8_t * p_tx_data, uint8_t * p_rx_data, uint16_t len);
void flash_erase_sector(uint16_t sector_num);
void flash_wake_up(void);
void flash_enter_power_down(void);
uint8_t flash_hw_check(void);
void flash_write_page(uint32_t Addr,uint16_t len,uint8_t *data);

#endif 


