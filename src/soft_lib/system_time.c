#include "app_timer.h"
#include "system_time.h"

uint32_t ms_to_rtc(uint32_t ms)
{
	return ( uint32_t )APP_TIMER_TICKS(ms);
}
uint32_t rtc_to_ms(uint32_t tick)
{
	return ((uint32_t)ROUNDED_DIV((tick) * ((uint64_t)(1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1))), (uint64_t)APP_TIMER_CLOCK_FREQ));
}
uint64_t get_rtc(void)
{
	return get_rtc_value();
}
uint8_t timepassed_rtc(uint32_t rtc,uint32_t  ms)
{
	if(((uint32_t)get_rtc()-rtc)>=ms_to_rtc(ms))
		return 1;
	return 0;
}

static uint32_t sys_time_ms = 0;
void sys_ms_add(void)
{
	sys_time_ms ++;
}
static uint32_t sys_time_sec = 0;
void sys_sec_add(void)
{
	sys_time_sec++;
}
//用于在低功耗应用中，此时系统时间分辨率较低，一些对时间要求
//严的应用通过get_sys_time_rtc不好处理,如系统时间分辨率100ms，要求100ms广播一次就出现可能200ms广播一次
//这个时候需要用中断计数做一些定时处理
static uint32_t sys_rtc_int_num = 0;
static void  sys_rtc_int_num_add(void)
{
	sys_rtc_int_num++;
}


//nordic 芯片处理
APP_TIMER_DEF(system_time_id);  
static void system_time_handle(void * p_context)
{
	UNUSED_PARAMETER(p_context);

//	sys_ms_add();
}
void systime_init(void)
{
	uint32_t err_code;
	 // Create timers.
    	err_code = app_timer_create(&system_time_id,APP_TIMER_MODE_REPEATED,system_time_handle);
	APP_ERROR_CHECK(err_code);
}
static uint16_t current_time_slice;
void systime_start(uint16_t time_slice)
{
	ret_code_t err_code;
	current_time_slice = time_slice;
    	err_code = app_timer_start(system_time_id, APP_TIMER_TICKS(time_slice) , NULL); // 1ms timer
    	APP_ERROR_CHECK(err_code);
}
void systime_stop(void)
{
	ret_code_t err_code;
	err_code = app_timer_stop(system_time_id);
	APP_ERROR_CHECK(err_code);
}
APP_TIMER_DEF(system_time_sec_id);  
static void system_time_sec_handle(void * p_context)
{
	UNUSED_PARAMETER(p_context);
	sys_sec_add();
}
void systime_sec_init(void)
{
	uint32_t err_code;
	 // Create timers.
    	err_code = app_timer_create(&system_time_sec_id,APP_TIMER_MODE_REPEATED,system_time_sec_handle);
	APP_ERROR_CHECK(err_code);
}
void systime_sec_start(void )
{
	ret_code_t err_code;
    	err_code = app_timer_start(system_time_sec_id, APP_TIMER_TICKS(1000) , NULL); 
    	APP_ERROR_CHECK(err_code);
}
void systime_sec_stop(void)
{
	ret_code_t err_code;
	err_code = app_timer_stop(system_time_sec_id);
	APP_ERROR_CHECK(err_code);
}

APP_TIMER_DEF(system_time_int_id);  
static void system_time_int_handle(void * p_context)
{
	UNUSED_PARAMETER(p_context);
	sys_rtc_int_num_add();
}
void systime_int_init(void)
{
	uint32_t err_code;
	 // Create timers.
    	err_code = app_timer_create(&system_time_int_id,APP_TIMER_MODE_REPEATED,system_time_int_handle);
	APP_ERROR_CHECK(err_code);
}
static uint16_t current_time_int_slice;
void systime_int_start(uint16_t time_slice)
{
	ret_code_t err_code;
	current_time_int_slice = time_slice;
    	err_code = app_timer_start(system_time_int_id, APP_TIMER_TICKS(time_slice) , NULL); // 1ms timer
    	APP_ERROR_CHECK(err_code);
}
void systime_int_stop(void)
{
	ret_code_t err_code;
	err_code = app_timer_stop(system_time_int_id);
	APP_ERROR_CHECK(err_code);
}





uint32_t get_sys_time(void)
{
	return sys_time_ms;
}

uint8_t timepassed_ms(uint32_t timer,uint32_t ms)
{
	if((get_sys_time()-timer)>=(ms/current_time_slice))
		return 1;
	return 0;
}

uint32_t get_sys_rtc_int_num(void)
{
	return sys_rtc_int_num;
}
//函数目前的用法就是比较中断次数来比较时间，对于时间分辨率较低的情况下 比get_sys_time_rtc好用
uint8_t timepassed_int(uint32_t timer_int,uint32_t ms)
{
	if((get_sys_rtc_int_num()-timer_int)>=(ms/current_time_int_slice))
		return 1;
	return 0;
}

uint32_t get_sys_time_sec(void)
{
	return sys_time_sec;
}
uint8_t timepassed_sec(uint32_t timer,uint32_t sec)
{
	if((get_sys_time_sec()-timer)>=sec)
		return 1;
	return 0;
}

