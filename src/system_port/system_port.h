#ifndef __SYSTEM__PORT__H
#define __SYSTEM__PORT__H
#include "SEGGER_RTT.h"




#define LOG_LEVEL_SYS					0
#define LOG_LEVEL_INFO				1
#define LOG_LEVEL_ERROR				2

#define SYS_LOG_LEVEL			LOG_LEVEL_INFO

#define SYS_LOG_EN				1

#if SYS_LOG_EN
#define SYS_LOG_PRINTF(...)		SEGGER_RTT_printf(0,__VA_ARGS__)
#else
#define SYS_LOG_PRINTF(...)
#endif


#if SYS_LOG_EN			
#define SYS_LOG(level,...)										\
if(level>=SYS_LOG_LEVEL) 												\
{																	\
	if(level==LOG_LEVEL_SYS || level==LOG_LEVEL_ERROR )							\
		SYS_LOG_PRINTF("file:%s,func:%s,line:%d  ",__FILE__,__func__,__LINE__);	\
	SYS_LOG_PRINTF(__VA_ARGS__);											\
}
											
#define SYS_LOG_DUMP_BUF(level,str,buf,len)							\
if(level>=SYS_LOG_LEVEL) 												\
{																	\
	if(level==LOG_LEVEL_SYS || level==LOG_LEVEL_ERROR )							\
		SYS_LOG_PRINTF("file:%s,func:%s,line:%d  ",__FILE__,__func__,__LINE__);	\
	SYS_LOG_PRINTF("%s  : ",str);												\
	for(uint16_t i=0 ; i<len ; i++) {if(i%16==0) SYS_LOG_PRINTF("\r\n"); SYS_LOG_PRINTF("0x%02x ",*((uint8_t *)buf+i));}	\
	SYS_LOG_PRINTF("\r\n");													\
}	
#define SYS_LOG_SYSTEM(...)				SYS_LOG(LOG_LEVEL_SYS,__VA_ARGS__)
#define SYS_LOG_INFO(...)				SYS_LOG(LOG_LEVEL_INFO,__VA_ARGS__)
#define SYS_LOG_ERROR(...)				SYS_LOG(LOG_LEVEL_ERROR,__VA_ARGS__)	
#define SYS_LOG_SYSTEM_BUF(str,buf,len)	SYS_LOG_DUMP_BUF(LOG_LEVEL_SYS,str,buf,len)
#define SYS_LOG_INFO_BUF(str,buf,len)		SYS_LOG_DUMP_BUF(LOG_LEVEL_INFO,str,buf,len)
#define SYS_LOG_ERROR_BUF(str,buf,len)	SYS_LOG_DUMP_BUF(LOG_LEVEL_ERROR,str,buf,len)
#else
#define SYS_LOG(level,...)
#define SYS_LOG_DUMP_BUF(level,str,buf,len)
#define SYS_LOG_SYSTEM(...)		
#define SYS_LOG_INFO(...)		
#define SYS_LOG_ERROR(...)	
#define SYS_LOG_SYSTEM_BUF(str,buf,len)	
#define SYS_LOG_INFO_BUF(str,buf,len)		
#define SYS_LOG_ERROR_BUF(str,buf,len)	
#endif



#endif
