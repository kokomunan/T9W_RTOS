#ifndef _POWER_MANAGER_H
#define _POWER_MANAGER_H

#include "mxconstants.h"
#include "stm32f4xx_hal.h"



#define ON_SYS_POWER()         HAL_GPIO_WritePin(MCU_PWR_HOLD_GPIO_Port,MCU_PWR_HOLD_Pin,GPIO_PIN_SET)
#define OFF_SYS_POWER()        HAL_GPIO_WritePin(MCU_PWR_HOLD_GPIO_Port,MCU_PWR_HOLD_Pin,GPIO_PIN_RESET)



void Low_Power_manager(void);

#endif
