#include "EXT_RTC.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "debug_log.h"
extern I2C_HandleTypeDef hi2c1;

#define IIC_CONTROLER   &hi2c1
#define RTC_debug_log   debug_log

uint8_t samll_to_big(uint8_t code)
{
    uint8_t tmp[4];
    tmp[0] = code >> 3;
    tmp[1] = (code & 0x04) >> 2;
    tmp[2] = (code & 0x02) >> 1;
    tmp[3] = (code & 0x01);
    tmp[0] = (tmp[3] << 3) | (tmp[2] << 2) | (tmp[1] << 1) | (tmp[0]);
    return tmp[0];
}
uint8_t big_to_small(uint8_t code)
{
    uint8_t tmp[4];
    tmp[0] = code >> 3;
    tmp[1] = (code & 0x04) >> 2;
    tmp[2] = (code & 0x02) >> 1;
    tmp[3] = (code & 0x01);
    tmp[0] = (tmp[3] << 3) | (tmp[2] << 2) | (tmp[1] << 1) | (tmp[0]);
    return tmp[0];
}
uint8_t BCD_to_int(uint8_t bcd_code)
{
    uint8_t tmp[2];

    tmp[0] = bcd_code & 0x0f;
    tmp[1] = bcd_code >> 4;
    tmp[0] = samll_to_big(tmp[0]);
    tmp[1] = samll_to_big(tmp[1]);
    tmp[0] = tmp[0] *10 + tmp[1];
    return tmp[0];
}
uint8_t int_to_BCD(uint8_t hex)
{
    uint8_t tmp[2];
    tmp[0] = hex/10;
    tmp[1] = hex%10;
    tmp[0] = big_to_small(tmp[0]);
    tmp[1] = big_to_small(tmp[1]);
    tmp[0] = tmp[0] | (tmp[1] << 4);
    return tmp[0];
}

void read_rtc_data(st_RTC_info * rtc_time)
{
    uint8_t tmp[8];
    uint8_t cmd = 0x60 | 0x04;
    uint8_t res;
    memset(tmp, 0, 8);

    // twi_master_transfer( cmd | TWI_READ_BIT, tmp, 7, TWI_ISSUE_STOP);

    res=HAL_I2C_Master_Receive(IIC_CONTROLER, cmd , tmp, 7, 100);  //读取时间

    RTC_debug_log("read rtc[%d]", res);

    // RTC_debug_log("y[%x]m[%x]d[%x]w[%x]h[%x]m[%x]s[%x]", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6]);
    tmp[0] = BCD_to_int(tmp[0]);
    tmp[1] = BCD_to_int(tmp[1]);
    tmp[2] = BCD_to_int(tmp[2]);
    tmp[3] = BCD_to_int(tmp[3]);
    tmp[4] = BCD_to_int(tmp[4] & 0xfd);
    tmp[5] = BCD_to_int(tmp[5]);
    tmp[6] = BCD_to_int(tmp[6]);
    RTC_debug_log("y[%d]m[%d]d[%d]w[%d]h[%d]m[%d]s[%d]", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6]);
    if(rtc_time != NULL)
    {
        rtc_time->note_time_year = tmp[0];
        rtc_time->note_time_month = tmp[1];
        rtc_time->note_time_day = tmp[2];
        rtc_time->note_time_hour = tmp[4];
        rtc_time->note_time_min = tmp[5];
    }

}
//write data must be 5 bytes, 
void write_rtc_data(st_RTC_info * rtc_time)
{
    uint8_t tmp[7];
    tmp[0] = 0x40;

    HAL_I2C_Master_Transmit(IIC_CONTROLER,0x60,tmp,1,100); //设置时间

    tmp[0] = int_to_BCD(rtc_time->note_time_year);
    tmp[1] = int_to_BCD(rtc_time->note_time_month);
    tmp[2] = int_to_BCD(rtc_time->note_time_day);
    tmp[3] = 0x00;
    tmp[4] = int_to_BCD(rtc_time->note_time_hour);
    tmp[5] = int_to_BCD(rtc_time->note_time_min);
    tmp[6] = 0x00;

    HAL_I2C_Master_Transmit(IIC_CONTROLER,0x64,tmp,7,100); //时间值
}

uint8_t rtc_hw_check(void)
{
    uint8_t test_time[5]={16,1,1,1,1};
    st_RTC_info     read_time;
    write_rtc_data((st_RTC_info *)test_time);	
    read_rtc_data(&read_time);
    if(strncmp((const char *)&read_time,(const char *)test_time,5)!=0)
    {
         err_log("rtc device has error\r\n");
         return 1;
    }
    else
    {
        return 0; 
    }
   
}

