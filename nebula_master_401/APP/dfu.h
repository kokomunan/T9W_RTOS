#ifndef __DFU_H_
#define __DFU_H_

#include "stdint.h"

#define DFU_INFO_ADR                   0x08008000
//应用程序所在地址
#define DFU_APP_ADR                    0x08020000
//备份区地址
#define DFU_APP_BACK_ADR               0x08040000

typedef  struct DFU_INFOR{
	
	
	uint32_t  dfu_update;     //非零时 说明需要更新
	uint32_t dfu_len;         //固件大小
	
}dfu_info_st;





void STMFLASH_Erase_bank0(void);
void STMFLASH_Erase_bank1(void);
void start_update_fw(void);
void jump_to_app(void);
uint8_t check_dfu(void);
void Dfu_Info_update(dfu_info_st*  info) ;
void Dfu_Info_load(dfu_info_st*  info);
#endif
