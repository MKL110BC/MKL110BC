#include "wdt.h"
#include "system_port.h"
#include "nrf_drv_wdt.h"

#ifdef ENLABLE_DOG
nrf_drv_wdt_channel_id m_channel_id;
static bool B_ReadWdtRunStatues(void)
{
    if(NRF_WDT->RUNSTATUS)
        return true;
    else
        return false;
}
#endif

void wdt_event_handler(void)
{
	SYS_LOG_INFO("-------WDT interrupt-----------\r\n");
}
void V_InitWdt(uint32_t TimeOutMs)
{
	#ifdef ENLABLE_DOG
	uint32_t err_code = NRF_SUCCESS;
	nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
	config.reload_value = TimeOutMs;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();
	#endif
}




void V_FeedWdog(void)
{
#ifdef ENLABLE_DOG
    NRF_WDT->RREN=1;
    NRF_WDT->RR[0]=0x6E524635;
    NRF_WDT->RREN=0;
#endif
}





