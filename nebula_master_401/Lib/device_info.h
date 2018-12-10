#ifndef __DEVICE_INFO_H_
#define __DEVICE_INFO_H_
#include "mxconstants.h"
#include "stm32f4xx_hal.h"

uint8_t  Load_device_info(uint8_t *info,uint8_t len);
uint8_t Update_device_info(uint8_t *info,uint8_t len);

void STMFLASH_Write(uint32_t WriteAddr,uint8_t *pBuffer,uint32_t NumToWrite)	;
void STMFLASH_Program(uint32_t WriteAddr,uint8_t *pBuffer,uint32_t NumToWrite)	;
void STMFLASH_Read(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t NumToRead)   	;
#endif

