#ifndef __OLED_H_
#define __OLED_H_
#include "stdint.h"
#define SET_MEM_ADDR_MODE_CMD   0x20
#define VERTICAL_ADDR_MODE_CMD  0x01
#define SET_COLUMN_ADDR_CMD     0x21
#define SET_PAGE_ADDR_CMD       0x22
#define DISPLAY_ON_CMD          0xAF
#define DISPLAY_OFF_CMD         0xAE

#define 	OLED_VBAT_ON() ;    	HAL_GPIO_WritePin(DIS_12V_CTRL_GPIO_Port, DIS_12V_CTRL_Pin,  GPIO_PIN_SET);
#define  	OLED_VBAT_OFF();      HAL_GPIO_WritePin(DIS_12V_CTRL_GPIO_Port, DIS_12V_CTRL_Pin,  GPIO_PIN_RESET);
#define   OLED_VDD_ON();        HAL_GPIO_WritePin(VDD_OLED_GPIO_Port, VDD_OLED_Pin,  GPIO_PIN_SET);  
#define   OLED_VDD_OFF();       HAL_GPIO_WritePin(VDD_OLED_GPIO_Port, VDD_OLED_Pin,  GPIO_PIN_RESET);  

#define   OLED_SELECT()         HAL_GPIO_WritePin(OLED_SPI_CS_GPIO_Port, OLED_SPI_CS_Pin, GPIO_PIN_RESET);
#define   OLED_RELEASE()        HAL_GPIO_WritePin(OLED_SPI_CS_GPIO_Port, OLED_SPI_CS_Pin,  GPIO_PIN_SET);
#define   OLED_DISABLE()       HAL_GPIO_WritePin(OLED_RET_GPIO_Port, OLED_RET_Pin, GPIO_PIN_RESET);
#define   OLED_ENABLE()         HAL_GPIO_WritePin(OLED_RET_GPIO_Port, OLED_RET_Pin,  GPIO_PIN_SET);


void spi_send_recv(uint8_t * p_tx_data, uint8_t * p_rx_data, uint16_t len);
void CLR_GDDRAM(void) ;
void OLED_On_Init(void);
void OLED_Off_Init(void);
void OLED_On_Test_Mode(void);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Display_ON_PUMP(void);
void OLED_Display_OFF_PUMP(void);
void OLED_CLEAR_BUFF(void);
void OLED_Draw_BMP(uint8_t *Matrix_Buffer,uint8_t Start_Adr,uint8_t Use_Col_Num,uint8_t half);
void  OLED_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void OLED_Refresh_bank(uint8_t bank) ;
void SPI_Write_Command(uint8_t cmd);
void SPI_Write_Data(uint8_t *data,uint16_t lenth);
void DRAW_1_Area(uint8_t *Matrix_Buffer,uint8_t Start_Adr,uint8_t Use_Col_Num)  ;
void DRAW_2_Area(uint8_t *Matrix_Buffer,uint8_t Start_Adr,uint8_t Use_Col_Num)  ;
void DRAW_FULL_Area(uint8_t *Matrix_Buffer,uint8_t Start_Adr,uint8_t Use_Col_Num);
void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t size);
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2) ;
#endif


