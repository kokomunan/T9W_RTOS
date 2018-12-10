/**
  ******************************************************************************
  * File Name          : mxconstants.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MXCONSTANT_H
#define __MXCONSTANT_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/


//ble
#define BT_INT_Pin GPIO_PIN_13
#define BT_INT_GPIO_Port GPIOC
#define BT_SPI_CS_Pin GPIO_PIN_6
#define BT_SPI_CS_GPIO_Port GPIOB
#define BT_PWR_CTRL_Pin GPIO_PIN_7
#define BT_PWR_CTRL_GPIO_Port GPIOB
//i2c ฤฃื้
#define ANT_PWR_CTRL_Pin GPIO_PIN_0
#define ANT_PWR_CTRL_GPIO_Port GPIOA
#define IIC_ANT_INT_Pin GPIO_PIN_0
#define IIC_ANT_INT_GPIO_Port GPIOH


//oled
#define DIS_12V_CTRL_Pin GPIO_PIN_5
#define DIS_12V_CTRL_GPIO_Port GPIOC
#define VDD_OLED_Pin GPIO_PIN_2
#define VDD_OLED_GPIO_Port GPIOB
#define OLED_RET_Pin GPIO_PIN_0
#define OLED_RET_GPIO_Port GPIOB
#define OLED_D_C_Pin GPIO_PIN_1
#define OLED_D_C_GPIO_Port GPIOB
#define OLED_SPI_CS_Pin GPIO_PIN_1
#define OLED_SPI_CS_GPIO_Port GPIOA

//flash
#define FLASH_SPI_CS_Pin GPIO_PIN_8
#define FLASH_SPI_CS_GPIO_Port GPIOC
//power 
#define LOW_BAT_DET_Pin GPIO_PIN_0
#define LOW_BAT_DET_GPIO_Port GPIOC
#define MCU_PWR_HOLD_Pin GPIO_PIN_3
#define MCU_PWR_HOLD_GPIO_Port GPIOC
#define PGOOD_Pin GPIO_PIN_1
#define PGOOD_GPIO_Port GPIOC
#define CHG_STA_Pin GPIO_PIN_2
#define CHG_STA_GPIO_Port GPIOC


//key

//#define DOWN_BUTTON_Pin GPIO_PIN_0
//#define DOWN_BUTTON_GPIO_Port GPIOB
//#define UP_BUTTON_Pin GPIO_PIN_1
//#define UP_BUTTON_GPIO_Port GPIOB


#define BUTTON_DET_Pin GPIO_PIN_10
#define BUTTON_DET_GPIO_Port GPIOA

//#define BUTTON_BT_MODE_Pin  GPIO_PIN_12
//#define BUTTON_BT_MODE_GPIO_Port  GPIOC
//#define BUTTON_2G4_MODE_Pin  GPIO_PIN_2
//#define BUTTON_2G4_MODE_GPIO_Port  GPIOD

#define BUTTON_MODE_SELECT_Pin   GPIO_PIN_12
#define BUTTON_MODE_SELECT_Port  GPIOC




//ewm3080b

#define WIFI_PWR_CTRL_Pin       GPIO_PIN_12
#define WIFI_PWR_CTRL_GPIO_Port GPIOB
#define WIFI_TX_Pin             GPIO_PIN_6
#define WIFI_TX_GPIO_Port       GPIOC
#define WIFI_RX_Pin             GPIO_PIN_7
#define WIFI_RX_GPIO_Port       GPIOC
#define WIFI_WAKEUP_Pin         GPIO_PIN_10
#define WIFI_WAKEUP_Port        GPIOB
#define WIFI_EASYLINK_Pin       GPIO_PIN_15
#define WIFI_EASYLINK_Port      GPIOB
#define WIFI_BOOT_Pin           GPIO_PIN_14
#define WIFI_BOOT_Port          GPIOB
#define WIFI_RESET_Pin          GPIO_PIN_4
#define WIFI_RESET_Port         GPIOC


#define WIFI_REVER_Port      GPIOC
#define WIFI_REVER_Pin      GPIO_PIN_11    


//rgb

//#define LED_R_Pin GPIO_PIN_3
//#define LED_R_GPIO_Port GPIOC
//#define LED_G_Pin GPIO_PIN_2
//#define LED_G_GPIO_Port GPIOC
//#define LED_B_Pin GPIO_PIN_1
//#define LED_B_GPIO_Port GPIOC

//mux key
#define KEY_PAD1_Pin GPIO_PIN_4
#define KEY_PAD1_Port  GPIOA
#define KEY_PAD2_Pin GPIO_PIN_5
#define KEY_PAD2_Port  GPIOA
#define KEY_PAD3_Pin GPIO_PIN_6
#define KEY_PAD3_Port  GPIOA
#define KEY_PAD4_Pin GPIO_PIN_7
#define KEY_PAD4_Port  GPIOA


//page detect 
#define PAGE_DET_POWER_CTRL_pin   GPIO_PIN_14
#define PAGE_DET_POWER_VTRL_Port  GPIOC
//half test
#define HALF_PRODUCT_TEST_Pin  GPIO_PIN_15
#define HALF_PRODUCT_TEST_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MXCONSTANT_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
