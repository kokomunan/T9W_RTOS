#include "dfu.h"
#include "device_info.h"
#include "debug_log.h"
#include "Status_Machin.h"
#include "RGB_LED.h"
#define  ERR_LOG       err_log
typedef  void (*pFunction)(void);
dfu_info_st   dfu_info;
void Dfu_Info_load(dfu_info_st*  info)
{
	
   STMFLASH_Read(DFU_INFO_ADR,(uint32_t *)info,sizeof(dfu_info_st)/4) ;	
	
}

void Dfu_Info_update(dfu_info_st*  info)  //可能会有问题
	
{
		
	STMFLASH_Write(DFU_INFO_ADR,(uint32_t *)info,sizeof(dfu_info_st)/4)	;
	
}


void STMFLASH_Erase_bank0(void)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t SectorError=0;   
    HAL_FLASH_Unlock();             //解锁	
    FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;    
    FlashEraseInit.Sector=FLASH_SECTOR_5;   //擦除3~4
    FlashEraseInit.Banks=FLASH_BANK_1;
    FlashEraseInit.NbSectors=1;
    FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;          
    while(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK);
	HAL_FLASH_Lock();           //上锁
}



void STMFLASH_Erase_bank1(void)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t SectorError=0;   
    HAL_FLASH_Unlock();             //解锁	
    FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;    
    FlashEraseInit.Sector=FLASH_SECTOR_6;    //擦除5
    FlashEraseInit.Banks=FLASH_BANK_1;
    FlashEraseInit.NbSectors=1;
    FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;          
    while(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK);
	HAL_FLASH_Lock();           //上锁
}


void start_update_fw(void)
{
    uint32_t i,data;
    uint32_t count=0,seperate_len;
	uint32_t read_addr,write_addr;
	read_addr=DFU_APP_BACK_ADR;    
	write_addr=DFU_APP_ADR ;
    
    seperate_len=dfu_info.dfu_len/10;
     
    UserLog("start copy fw\r\n");
	for(i=0;i<dfu_info.dfu_len;i+=4)
    {
        
        
        STMFLASH_Read(read_addr,(uint32_t *)&data,1) ;	
              
        STMFLASH_Write(write_addr,(uint32_t *)&data,1);


        
        read_addr+=4;         
		write_addr+=4;                    
        count+=4;
        if(count>seperate_len)
        {
            count=0;
            UserLog("*");
            RED_LED_TOGLE();
            BLUE_LED_TOGLE();
            
        }
        
        FEED_DOG() ;      //清看门狗
        
    }
  
    UserLog("copy finished\r\n");
    dfu_info.dfu_update=0;
    Dfu_Info_update(&dfu_info);
    
}

void jump_to_app(void)
{
    pFunction JumpToApplication;
    uint32_t JumpAddress;
    if (((*(__IO uint32_t*)DFU_APP_ADR) & 0x2FFE0000 ) == 0x20000000)//没有检查代码段地址的合法性只检查了堆栈地址
    {
        UserLog("jump to app\r\n");
        HAL_Delay(10);
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t*) (DFU_APP_ADR + 4);//得到代码地址
        JumpToApplication = (pFunction) JumpAddress;

        __HAL_RCC_GPIOA_CLK_DISABLE();
        __disable_irq();
        __HAL_RCC_PWR_CLK_DISABLE();
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*) DFU_APP_ADR);//设置栈指针
        JumpToApplication(); //跳转
    }
    else
    {
        UserLog("fw has fatal err\r\n");
        
    }
    
    
}

uint8_t check_dfu(void)
{
    
    Dfu_Info_load(&dfu_info);
    
    if(1==dfu_info.dfu_update) 
    {
        UserLog("has a new fw to update\r\n");
        if(dfu_info.dfu_len>0x30000)
        {    
            ERR_LOG("fw lenth is err");
            return 1;
        }
	    UserLog("prepare to erase bank0\r\n");
		STMFLASH_Erase_bank0();
       
        return 0;
    }
    else
    {
        
        UserLog("no need to update\r\n");
        return 1;
    }
      
 //   return 0; 
    
}







