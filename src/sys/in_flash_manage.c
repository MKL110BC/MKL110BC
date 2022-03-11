#include "in_flash_manage.h"
#include "nrf_delay.h"
#include "string.h"
#include "nrf_nvmc.h"




void flash_page_erase(uint32_t addr)
{
	if(addr%PAGE_SIZE)
		return;
	nrf_nvmc_page_erase(addr);
}
void flash_write(uint32_t addr,const uint8_t *buf,uint32_t len)
{
	nrf_nvmc_write_bytes(addr,buf,len);
}
void flash_page_erase_and_write(uint32_t addr,const uint8_t *buf,uint16_t len)
{
	if(addr%PAGE_SIZE)
		return;
	flash_page_erase(addr);
	nrf_nvmc_write_bytes(addr,buf,len);
}
void flash_read(uint32_t addr,uint8_t *buf,uint32_t len)
{
	uint16_t i;
	const uint8_t *pdata;
	pdata=(uint8_t*)addr;
	for(i=0; i<len; i++)
		buf[i]=pdata[i];
}


