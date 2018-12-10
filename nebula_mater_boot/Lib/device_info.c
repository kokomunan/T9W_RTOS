#include "device_info.h"
#define SECTOR_0_END   0x8003fff
#define SECTOR_1_END   0x8007fff
#define SECTOR_2_END   0x800bfff
#define SECTOR_3_END   0x800ffff
#define SECTOR_4_END   0x801ffff
#define SECTOR_5_END   0x803ffff

#define STM32_FLASH_BASE  0x08000000 	//STM32 FLASH����ʼ��ַ
#define DEVICE_INFO_ADDR  0x08004000    //�豸��Ϣ�洢��ַ

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
//��ȡָ����ַ����(32λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
uint32_t  STMFLASH_ReadWord(uint32_t faddr)
{
	  return *(__IO uint32_t *)faddr; 
}
 
//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToRead:��(32λ)��
void STMFLASH_Read(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t NumToRead)   	
{
		uint32_t i;
		for(i=0;i<NumToRead;i++)
		{
				if(ReadAddr>=0x08080000)
				return;
				pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//��ȡ4���ֽ�.
				ReadAddr+=4;//ƫ��4���ֽ�.	
		}
}


//ҳ��ʽ ��128�ֽڣ�  ���  buff ������128�ֽڵ�������
FLASH_EraseInitTypeDef FlashEraseInit;

void STMFLASH_Write(uint32_t WriteAddr,uint32_t *pBuffer,uint32_t NumToWrite)	
{ 
   
    HAL_StatusTypeDef FlashStatus=HAL_OK;
    uint32_t SectorError=0;
	  uint32_t addrx=0;
	  uint32_t endaddr=0;	
    if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//�Ƿ���ַ
    
		HAL_FLASH_Unlock();             //����	
		addrx=WriteAddr;				//д�����ʼ��ַ
		endaddr=WriteAddr+NumToWrite*4;	//д��Ľ�����ַ
    
		if(addrx<0X8080000)
		{
				while(addrx<endaddr)		
				{
						if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
						{   
									FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;    
									FlashEraseInit.Sector= Get_sector(addrx);  
									FlashEraseInit.Banks=FLASH_BANK_1;
									FlashEraseInit.NbSectors=1;
									FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3; 
							 
									if(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK) 
									{
											break;//����������	
									}
										
										
						}
						else 
						{
						    addrx+=4;
						}

								FLASH_WaitForLastOperation(10);                //�ȴ��ϴβ������
				}


				FlashStatus=FLASH_WaitForLastOperation(10);            //�ȴ��ϴβ������
				if(FlashStatus==HAL_OK)
				{
						while(WriteAddr<endaddr)//д����
						{
								if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,WriteAddr,*pBuffer)!=HAL_OK)//д������
								{ 
								    break;	//д���쳣
								}
								WriteAddr+=4;
								pBuffer++;
						} 
				}

		}
    HAL_FLASH_Lock();           //����
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

