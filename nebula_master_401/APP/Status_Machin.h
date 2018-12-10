#ifndef _STATUS_MACHIN_H
#define _STATUS_MACHIN_H
#include "mxconstants.h"
#include "stm32f4xx_hal.h"

extern IWDG_HandleTypeDef hiwdg;
#define POWER_BUTTON_PRESS     HAL_GPIO_ReadPin(BUTTON_DET_GPIO_Port, BUTTON_DET_Pin)
#define FEED_DOG()  HAL_IWDG_Refresh(&hiwdg);
uint8_t Get_Device_Status(void);
void status_machin_thread(void const * argument);
void init_status_machin(void);
void Set_Device_Status(uint8_t new_sta);
#endif




