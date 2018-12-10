#ifndef __PAGE_DETECT_H_
#define __PAGE_DETECT_H_
#include "stdint.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"

#define 	PAGE_DET_POWER_ON()     	HAL_GPIO_WritePin(PAGE_DET_POWER_VTRL_Port, PAGE_DET_POWER_CTRL_pin,  GPIO_PIN_SET)
#define 	PAGE_DET_POWER_OFF()     	HAL_GPIO_WritePin(PAGE_DET_POWER_VTRL_Port, PAGE_DET_POWER_CTRL_pin,  GPIO_PIN_RESET)



void page_det_moudule_init(void);
void page_det_start(void);
void page_det_stop(void);
uint8_t request_pages(void);
uint16_t read_page_value(void);
void read_page_thread(const void *argument);
void page_det_moudule_reset(void);
#endif

