#include "device_info.h"
#define SECTOR_0_END   0x8003fff
#define SECTOR_1_END   0x8007fff
#define SECTOR_2_END   0x800bfff
#define SECTOR_3_END   0x800ffff
#define SECTOR_4_END   0x801ffff
#define SECTOR_5_END   0x803ffff

#define STM32_FLASH_BASE  0x08000000 	//STM32 FLASH的起始地址
#define DEVICE_INFO_ADDR  0x08004000    //设备信息存储地址

uint32_t Get_sector(uint32_t addr)
{
	
  if((addr>=STM32_FLASH_BASE)&&(addr<=SECTOR_0_END))
  {
      return FLASH_SECTOR_0;
  }
  else if((addr>SECTOR_0_END)&&(addr<=SECTOR_1_END))
  {
       return FLASH_SECTOR_1;
  }
   else if((addr>SECTOR_1_END)&&(addr<=SECTOR_2_END))
  {
       return FLASH_SECTOR_2;
  }
   else if((addr>SECTOR_2_END)&&(addr<=SECTOR_3_END))
  {
       return FLASH_SECTOR_3;
  }
   else if((addr>SECTOR_3_END)&&(addr<=SECTOR_4_END))
  {
       return FLASH_SECTOR_4;
  }
   else if((addr>SECTOR_4_END)&&(addr<=SECTOR_5_END))
  {
       return FLASH_SECTOR_5;
  }
	
}
//读取指定地址的字(32位数据) 
//faddr:读地址 
//返回值:对应数据.
uint32_t  STMFLASH_ReadWord(uint32_t faddr)
{
	  return *(__IO uint32_t *)faddr; 
}
 
//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToRead:字(32位)数
void STMFLASH_Read(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t NumToRead)   	
{
		uint32_t i;
		for(i=0;i<NumToRead;i++)
		{
				if(ReadAddr>=0x08080000)
				return;
				pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//读取4个字节.
				ReadAddr+=4;//偏移4个字节.	
		}
}


//页方式 （128字节）  编程  buff 长度是128字节的整数倍
FLASH_EraseInitTypeDef FlashEraseInit;

void STMFLASH_Write(uint32_t WriteAddr,uint32_t *pBuffer,uint32_t NumToWrite)	
{ 
   
    HAL_StatusTypeDef FlashStatus=HAL_OK;
    uint32_t SectorError=0;
	  uint32_t addrx=0;
	  uint32_t endaddr=0;	
    if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//非法地址
    
		HAL_FLASH_Unlock();             //解锁	
		addrx=WriteAddr;				//写入的起始地址
		endaddr=WriteAddr+NumToWrite*4;	//写入的结束地址
    
		if(addrx<0X8080000)
		{
				while(addrx<endaddr)		
				{
						if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//有非0XFFFFFFFF的地方,要擦除这个扇区
						{   
									FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;    
									FlashEraseInit.Sector= Get_sector(addrx);  
									FlashEraseInit.Banks=FLASH_BANK_1;
									FlashEraseInit.NbSectors=1;
									FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3; 
							 
									if(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK) 
									{
											break;//发生错误了	
									}
										
										
						}
						else 
						{
						    addrx+=4;
						}

								FLASH_WaitForLastOperation(10);                //等待上次操作完成
				}


				FlashStatus=FLASH_WaitForLastOperation(10);            //等待上次操作完成
				if(FlashStatus==HAL_OK)
				{
						while(WriteAddr<endaddr)//写数据
						{
								if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,WriteAddr,*pBuffer)!=HAL_OK)//写入数据
								{ 
								    break;	//写入异常
								}
								WriteAddr+=4;
								pBuffer++;
						} 
				}

		}
    HAL_FLASH_Lock();           //上锁
} 


uint8_t  Load_device_info(uint8_t *info,uint8_t len)
{
	
		STMFLASH_Read(DEVICE_INFO_ADDR,(uint32_t *)info,len/4);   	
		
		return 0;
}

uint8_t Update_device_info(uint8_t *info,uint8_t len)
{
		
		STMFLASH_Write(DEVICE_INFO_ADDR,(uint32_t *)info,len/4)	;
		return 0;
}

