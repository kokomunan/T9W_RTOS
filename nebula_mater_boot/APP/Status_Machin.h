#ifndef _STATUS_MACHIN_H
#define _STATUS_MACHIN_H
#include "mxconstants.h"
#include "stm32f4xx_hal.h"

extern IWDG_HandleTypeDef hiwdg;
#define POWER_BUTTON_PRESS     HAL_GPIO_ReadPin(BUTTON_DET_GPIO_Port, BUTTON_DET_Pin)==0
#define FEED_DOG()  HAL_IWDG_Refresh(&hiwdg);
void status_machin_thread(void);
void init_status_machin(void);
#endif




