#ifndef __WDT__H___
#define __WDT__H___
#include <stdlib.h>
#include "nrf_soc.h"
#include "app_error.h"
#include "app_util.h"



//#define ENLABLE_DOG
void V_InitWdt(uint32_t TimeOutMs);
void V_FeedWdog(void);


#endif


