/*!
 * \file      smtc_hal_lp_timer.c
 *
 * \brief     Implements Low Power Timer utilities functions.
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type

#include "smtc_hal_lp_timer.h"

#include "smtc_hal_mcu.h"

//nrf52833
#include "app_timer.h"


APP_TIMER_DEF(lora_time_id);


static uint8_t timer_init_flag = 0;
static hal_lp_timer_irq_t lptim_tmr_irq = { .context = NULL, .callback = NULL };


uint32_t RtcMs2Tick( uint32_t milliseconds )
{
    return ( uint32_t )APP_TIMER_TICKS(milliseconds);
}

static void lora_time_Handle(void * p_context)
{
	UNUSED_PARAMETER(p_context);
	if( lptim_tmr_irq.callback != NULL )
	{
	    lptim_tmr_irq.callback( lptim_tmr_irq.context );
	}
}


void hal_lp_timer_init( void )
{
	if(timer_init_flag==0)
	{
		uint32_t err_code;
		// Create timers.
		err_code = app_timer_create(&lora_time_id,
		                            APP_TIMER_MODE_SINGLE_SHOT,
		                            lora_time_Handle);
		APP_ERROR_CHECK(err_code);
		timer_init_flag = 1;
	}
}

void hal_lp_timer_start( const uint32_t milliseconds, const hal_lp_timer_irq_t* tmr_irq )
{
	lptim_tmr_irq = *tmr_irq;
	app_timer_stop(lora_time_id);
    	app_timer_start(lora_time_id, RtcMs2Tick(milliseconds), NULL);
    	
}

void hal_lp_timer_stop( void ) 
{ 
	app_timer_stop(lora_time_id);
}

void hal_lp_timer_irq_enable( void ) 
{ }

void hal_lp_timer_irq_disable( void ) 
{  }

