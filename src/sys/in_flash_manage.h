#ifndef __INFLASH__MANAGE__H
#define __INFLASH__MANAGE__H

#include <stdint.h>





#define FLASH_SIZE  (512*1024)
#define PAGE_SIZE  4096UL






void flash_page_erase(uint32_t addr);
void flash_write(uint32_t addr,const uint8_t *buf,uint32_t len);
void flash_page_erase_and_write(uint32_t addr,const uint8_t *buf,uint16_t len);
void flash_read(uint32_t addr,uint8_t *buf,uint32_t len);

#endif



