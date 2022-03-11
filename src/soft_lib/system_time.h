#ifndef 	__SYSTEM__TIME__H
#define 	__SYSTEM__TIME__H
#include <stdint.h>

#define SLEEP_TIME_SLICE			500
#define WORK_TIME_SLICE			100


#pragma pack(1)
struct time_struct
{
	uint16_t year;
	uint8_t mon;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t sec;
};
#pragma pack()



void systime_init(void);
void systime_stop(void);
void systime_start(uint16_t time_slice);


/*
20210603 在nordic中此函数不在使用了
原因
1、时间存在偏差
2、如果要调用有而仅仅是因为定时唤醒而已
//后续ms的时间判断获取都可以通过get_rtc

*/
//uint32_t get_sys_time(void);
//uint8_t timepassed_ms(uint32_t timer,uint32_t ms);

void systime_sec_init(void);
void systime_sec_start(void );
void systime_sec_stop(void);
uint32_t get_sys_time_sec(void);
uint8_t timepassed_sec(uint32_t timer,uint32_t sec);



void systime_int_init(void);
void systime_int_start(uint16_t time_slice);
void systime_int_stop(void);
uint32_t get_sys_rtc_int_num(void);
uint8_t timepassed_int(uint32_t int_count,uint32_t ms);

uint32_t rtc_to_ms(uint32_t tick);
uint64_t get_rtc(void);
uint8_t timepassed_rtc(uint32_t rtc,uint32_t  ms);
#endif
