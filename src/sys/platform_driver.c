#include "platform_driver.h"
#include "app_timer.h"
#include "nrf_soc.h"
#include "wdt.h"


//ϵͳ��ʱ����ʼ��
static void timers_init(void)
{
	ret_code_t err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);
}

/****************************
*sleep manager
***************************/
void power_manage(void)
{
	#ifdef SOFTDEVICE_PRESENT
	//SOFTDEVICE_PRESENT   
	ret_code_t err_code = sd_app_evt_wait();
	APP_ERROR_CHECK(err_code);
	
	//20210511 by ysh,�޸�17.0��SDK�г��ֵ��ø���������벻�˵͹��ĵ�����
	__set_FPSCR(__get_FPSCR()&(~(0X0000009F)));
	(void)__get_FPSCR();
	NVIC_ClearPendingIRQ(FPU_IRQn);
	#else
		__WFE();
	#endif

}


void Platform_driver_init(void)
{
	V_InitWdt(30000);
	
	timers_init();
}

