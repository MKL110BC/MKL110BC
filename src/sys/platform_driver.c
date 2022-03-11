#include "platform_driver.h"
#include "app_timer.h"
#include "nrf_soc.h"
#include "wdt.h"


//系统定时器初始化
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
	
	//20210511 by ysh,修复17.0的SDK中出现调用浮点运算进入不了低功耗的问题
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

