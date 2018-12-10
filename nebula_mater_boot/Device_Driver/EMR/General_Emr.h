#ifndef __GENNERAL_EMR_H_
#define __GENNERAL_EMR_H_
#include "mxconstants.h"
#include "stm32f4xx_hal.h"

#define     TOUCH_PAD_ON()        HAL_GPIO_WritePin(EM_PWR_CTRL_GPIO_Port,EM_PWR_CTRL_Pin,GPIO_PIN_SET)
#define     TOUCH_PAD_OFF()       HAL_GPIO_WritePin(EM_PWR_CTRL_GPIO_Port,EM_PWR_CTRL_Pin,GPIO_PIN_RESET)
void EMR_pad_version(void);
uint8_t Read_EMR(uint8_t *buf,uint8_t len);
extern void read_position(void);


#endif 
