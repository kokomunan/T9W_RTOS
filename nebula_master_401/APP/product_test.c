#include "product_test.h"
#include "mxconstants.h"
#include "stm32f4xx_hal.h"
#include "RGB_LED.h"
#include "Status_MAchin.h"
#include "power_manager.h"
#include "oled.h"
#include "General_Emr.h"
#include "EXT_RTC.h"
#include "norflash.h"
#include "em9203.h"
#include "nrf51822.h"
#include "debug_log.h"
#include "page_detect.h"
#include "cmsis_os.h"
#define   IS_ENTER_HALF_PRODUCT_TEST         HAL_GPIO_ReadPin(HALF_PRODUCT_TEST_Port, HALF_PRODUCT_TEST_Pin)==1 
#define   IS_NO_ENTER_HALF_PRODUCT_TEST      HAL_GPIO_ReadPin(HALF_PRODUCT_TEST_Port, HALF_PRODUCT_TEST_Pin)==0

void half_product_test_check(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    GPIO_InitStruct.Pin = HALF_PRODUCT_TEST_Pin;    //≤‚ ‘“˝Ω≈      
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(HALF_PRODUCT_TEST_Port, &GPIO_InitStruct);  
    
    if(IS_ENTER_HALF_PRODUCT_TEST)
    {
        uint32_t test_mode_cnt = 0;
        while(test_mode_cnt <= 500)
        {
            if(IS_NO_ENTER_HALF_PRODUCT_TEST)
            {
                break;
            }
            test_mode_cnt++;
            osDelay(1);
        }
        if(test_mode_cnt > 500)					
        {
            uint8_t res1,res2,res4;
            
            OFF_SYS_POWER();              
            OLED_VBAT_ON();
            TOUCH_PAD_ON();
            PAGE_DET_POWER_ON();
            
       
            res1=rtc_hw_check();
            res2=flash_hw_check();      
            FEED_DOG() ;            
          //  res3=check_em9203();         
            res4=check_ble();                                     
            if(res1||res2 ||res4  )
            {
                UserLog("test error")
            }     
            else
            {
                 UserLog("test ok")
            }    
            while(1)
            {
                
                FEED_DOG() ;             
                               
            }
       				
            
        }
        
    }
    
}

