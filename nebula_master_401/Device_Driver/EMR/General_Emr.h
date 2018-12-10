#ifndef __GENNERAL_EMR_H_
#define __GENNERAL_EMR_H_
#include "mxconstants.h"
#include "stm32f4xx_hal.h"

//#define     TOUCH_PAD_ON()        HAL_GPIO_WritePin(ANT_PWR_CTRL_GPIO_Port,ANT_PWR_CTRL_Pin,GPIO_PIN_SET);\
//                                  HAL_Delay(10);                                                            \
//                                  if (HAL_I2C_Init(&hi2c3) != HAL_OK)\
//                                  {                                                                        \
//                                      Error_Handler();                                                      \
//                                  }                                                                         \
//                                  __HAL_I2C_ENABLE(&hi2c3);  
//                             
//#define     TOUCH_PAD_OFF()       HAL_GPIO_WritePin(ANT_PWR_CTRL_GPIO_Port,ANT_PWR_CTRL_Pin,GPIO_PIN_RESET)
#define EMR_INT    HAL_GPIO_ReadPin(IIC_ANT_INT_GPIO_Port, IIC_ANT_INT_Pin)==0
void TOUCH_PAD_ON(void);
void TOUCH_PAD_OFF(void);
void EMR_pad_version(void);
uint8_t Read_EMR(uint8_t *buf,uint8_t len);
extern void read_position(void);
uint8_t ERM_cmd_handle(uint8_t *packet);
uint8_t Write_EMR(uint8_t cmd,uint8_t *data,uint8_t len);
uint8_t check_version(uint16_t version);
void clear_jedi_info(void);
void update_jedi_info(void);
void set_i2c_int(uint8_t enable);
uint8_t IS_emr_update_sucess(void);


#endif 
