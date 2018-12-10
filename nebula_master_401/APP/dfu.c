
#include "dfu.h"
#include "device_info.h"
#include "debug_log.h"
#include "Status_Machin.h"

typedef  void (*pFunction)(void);
dfu_info_st   dfu_info;
void Dfu_Info_load(dfu_info_st*  info)
{
	
   STMFLASH_Read(DFU_INFO_ADR,(uint32_t *)info,sizeof(dfu_info_st)/4) ;	
	
}

void Dfu_Info_update(dfu_info_st*  info)  	
{
		
	STMFLASH_Write(DFU_INFO_ADR,(uint8_t *)info,sizeof(dfu_info_st))	;
	
}


void STMFLASH_Erase_bank0(void)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t SectorError=0;   
    HAL_FLASH_Unlock();             
    FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;    
    FlashEraseInit.Sector=FLASH_SECTOR_5;   //²Á³ý5
    FlashEraseInit.Banks=FLASH_BANK_1;
    FlashEraseInit.NbSectors=1;
    FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;          
    while(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK);
	HAL_FLASH_Lock();           
}



void STMFLASH_Erase_bank1(void)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t SectorError=0;   
    HAL_FLASH_Unlock();            
    FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;    
    FlashEraseInit.Sector=FLASH_SECTOR_6;    
    FlashEraseInit.Banks=FLASH_BANK_1;
    FlashEraseInit.NbSectors=1;
    FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;          
    while(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK);
	HAL_FLASH_Lock();           
}


void start_update_fw(void)
{
    uint32_t i,data;
    uint32_t count=0,seperate_len;
	uint32_t read_addr,write_addr;
	read_addr= DFU_APP_ADR ;
	write_addr=DFU_APP_BACK_ADR;
    
    seperate_len=dfu_info.dfu_len/10;
     
    debug_log("start copy fw\r\n");
	for(i=0;i<dfu_info.dfu_len;i+=4)
    {
        
        STMFLASH_Read(read_addr,(uint32_t *)&data,1) ;	
        while(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,write_addr,data)==HAL_OK);	  
        read_addr+=4;         
		write_addr+=4;
        count+=4;
        if(count>seperate_len)
        {
            count=0;
            debug_log("*");
            
            
        }
        
        FEED_DOG() ;  
        
    }
  
    debug_log("copy finished\r\n");
    dfu_info.dfu_update=0;
    Dfu_Info_update(&dfu_info);
    
}

void jump_to_app(void)
{
    pFunction JumpToApplication;
    uint32_t JumpAddress;
    if (((*(__IO uint32_t*)DFU_APP_ADR) & 0x2FFE0000 ) == 0x20000000)
    {
        debug_log("jump to app\r\n");
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t*) (DFU_APP_ADR + 4);
        JumpToApplication = (pFunction) JumpAddress;

        __HAL_RCC_GPIOA_CLK_DISABLE();
        __disable_irq();
        __HAL_RCC_PWR_CLK_DISABLE();
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*) DFU_APP_ADR);
        JumpToApplication(); 
    }
    else
    {
        debug_log("fw has fatal err\r\n");
        
    }
    
    
}

uint8_t check_dfu(void)
{
    
    Dfu_Info_load(&dfu_info);
    
    if(1==dfu_info.dfu_update) 
    {
        debug_log("has a new fw to update\r\n");
        if(dfu_info.dfu_len>0x20000)
        {    
            err_log("fw lenth is err");
            return 1;
        }
	    debug_log("prepare to erase bank0\r\n");
		STMFLASH_Erase_bank0();
       
        return 0;
    }
    else
    {
        
        debug_log("no need to update\r\n");
        return 1;
    }
      
    
    
}







