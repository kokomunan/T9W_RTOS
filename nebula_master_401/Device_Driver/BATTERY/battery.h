#ifndef _BATEERY_H
#define _BATEERY_H
#include "stdint.h"
#define bool  uint8_t
#define false 0
#define  true  1	
#define ADC_VDIV_R1_10K         20	//20K ohms
#define ADC_VDIV_R2_10K         62	//59K ohms
#define ADC_REF_VBG_10V         12	//1.2V
//#define ADC_TO_VBAT_MV(adc)      ((adc)*(ADC_VDIV_R1_10K + ADC_VDIV_R2_10K)*ADC_REF_VBG_10V*1000)/(10*ADC_VDIV_R1_10K*4096)
#define ADC_LOW_NOISE    100


#define ADC_TO_VBAT_MV(adc)        ((((adc)*3300)*(20+62))/(4096*20))-ADC_LOW_NOISE


#define POWER_CHARGING    HAL_GPIO_ReadPin(PGOOD_GPIO_Port, PGOOD_Pin)==0
#define POWER_UNCHARGING  HAL_GPIO_ReadPin(PGOOD_GPIO_Port, PGOOD_Pin)==1 

#define IS_CHARGE_FULL    HAL_GPIO_ReadPin(CHG_STA_GPIO_Port, CHG_STA_Pin)==1 

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

void init_charging_display(void);
void battery_check_thread(const void *argument);
void Reset_Low_Power_Timer(void);
uint8_t power_on_battery_check(void);
uint8_t IS_charge_full(void);
extern void Low_Power_manager(void);
bool battery_not_enough(void);
bool Is_battery_is_safe(void);
void battry_period_timer(const void* argument);
#endif
