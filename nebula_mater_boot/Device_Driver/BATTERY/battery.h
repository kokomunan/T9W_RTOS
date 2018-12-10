#ifndef _BATEERY_H
#define _BATEERY_H
#include "stdint.h"
#define bool  uint8_t
#define false 0
#define  true  1	
#define ADC_VDIV_R1_10K         20	//20K ohms
#define ADC_VDIV_R2_10K         62	//59K ohms
#define ADC_REF_VBG_10V         12	//1.2V
#define ADC_TO_VBAT_MV(adc)      ((adc)*(ADC_VDIV_R1_10K + ADC_VDIV_R2_10K)*ADC_REF_VBG_10V*1000)/(10*ADC_VDIV_R1_10K*1024)
#define POWER_CHARGING    HAL_GPIO_ReadPin(PGOOD_GPIO_Port, PGOOD_Pin)==0
#define POWER_UNCHARGING  HAL_GPIO_ReadPin(PGOOD_GPIO_Port, PGOOD_Pin)==1 

uint8_t get_battery_value(void);
void charging_display(void);
uint32_t batt_meas_init(void);
uint16_t batt_meas_update(void);
uint8_t batt_meas_is_lowbatt(void);
uint8_t batt_meas_is_shutdown(void);
uint8_t batt_meas_power_check(void);
uint16_t batt_meas_get_value(void);
uint8_t get_battery_percent(uint16_t BatteryVoltageInMV);
uint16_t batt_meas_get_mv_value(void);
void battery_timer_callback(void);
void init_charging_display(void);
void battery_check_thread(void);
void Reset_Low_Power_Timer(void);
uint8_t power_on_battery_check(void);
extern void Low_Power_manager(void);
#endif
