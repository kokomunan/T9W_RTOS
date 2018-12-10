#ifndef  _NOTE_MANAGER_H__
#define   _NOTE_MANAGER_H__

#include  "stdint.h"



typedef struct  
{
    uint16_t note_identifier;
    uint16_t note_number;
    uint8_t flash_erase_flag;
	  uint8_t note_head_start;
    uint16_t note_start_sector;
    uint32_t note_len;
    uint8_t note_time_year;
    uint8_t note_time_month;
    uint8_t note_time_day;
    uint8_t note_time_hour;
    uint8_t note_time_min;
} st_note_header_info;


uint32_t get_free_sector(void);
uint32_t get_next_sector(uint32_t currunt_sector);
uint16_t create_note_id(uint16_t id);
uint16_t get_before_note_id(int16_t id,int16_t offset) ;
uint16_t get_next_note_id(uint16_t id,int16_t offset);
void read_note(uint32_t addr, uint8_t* pdata, uint16_t len);
uint8_t Is_Open_note(void);
void Disbale_note_store(void);
void Enable_note_store(void);
uint8_t Is_enable_note_store(void);
uint8_t search_first_note_head(uint32_t serch_start_sector,uint32_t* fond_start_sector);
uint8_t search_other_note_head(void);
uint8_t create_new_note_head(uint16_t num_index);
uint8_t create_next_note_head(uint16_t num_index);
void close_note(uint8_t is_store, uint8_t show);
void init_offline_store_data(void);
void upload_stored_note_Thread(void);
void offline_write_flash_Thread(void);
void set_en_upload(uint8_t en);
void offline_store_data(uint8_t *buf,uint8_t len);
void set_first_boot_flag(uint8_t value);
#endif 










