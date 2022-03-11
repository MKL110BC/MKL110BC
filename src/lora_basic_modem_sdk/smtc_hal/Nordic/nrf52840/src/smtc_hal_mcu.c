/*!
 * \file      smtc_hal_mcu.c
 *
 * \brief     MCU Hardware Abstraction Layer implementation
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

#include "smtc_hal_mcu.h"
#include "modem_pinout.h"



#include "smtc_hal.h"
#include "app_timer.h"

#if( HAL_DBG_TRACE == HAL_FEATURE_ON )
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#endif

#include "nrf_delay.h"
#include "system_port.h"
#include "app_util_platform.h"
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*!
 * Watchdog counter reload value during sleep
 *
 * \remark The period must be lower than MCU watchdog period
 */
#define WATCHDOG_RELOAD_PERIOD_SECONDS 20
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*!
 * @brief Low Power options
 */
typedef enum low_power_mode_e
{
    LOW_POWER_ENABLE,
    LOW_POWER_DISABLE
} low_power_mode_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

static volatile bool             exit_wait            = false;
static volatile low_power_mode_t lp_current_mode      = LOW_POWER_ENABLE;


/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */




/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void hal_mcu_critical_section_begin( uint32_t* mask )
{
    *mask = __get_PRIMASK( );
    __disable_irq( );
}

void hal_mcu_critical_section_end( uint32_t* mask ) { __set_PRIMASK( *mask ); }

uint8_t _mask;
void hal_mcu_disable_irq( void ) { app_util_critical_region_enter(&_mask); }

void hal_mcu_enable_irq( void ) { app_util_critical_region_exit(_mask);}

void hal_mcu_delay_ms( uint32_t delay_ms ) { hal_rtc_delay_in_ms( delay_ms ); }

uint32_t hal_mcu_get_tick( void ) { return get_rtc_value(); }


void hal_gpio_init(void)
{
	hal_gpio_init_out( SMTC_RADIO_NSS, HAL_GPIO_SET );
    	hal_gpio_init_in( SMTC_RADIO_BUSY, HAL_GPIO_PULL_MODE_NONE, HAL_GPIO_IRQ_MODE_OFF, NULL );
    // Here init only the pin as an exti rising and the callback will be attached later
    	hal_gpio_init_in( SMTC_RADIO_DIOX, HAL_GPIO_PULL_MODE_DOWN, HAL_GPIO_IRQ_MODE_RISING, NULL );
    	hal_gpio_init_out( SMTC_RADIO_NRST, HAL_GPIO_SET );
	
	hal_gpio_init_out( RADIO_LNA_CTRL, HAL_GPIO_RESET );
}

void hal_mcu_init( void )
{
	hal_rng_init();
    	/* Initialize low power timer */
   	hal_lp_timer_init( );
	hal_gpio_init();


    	/* Initialize SPI */
    	hal_spi_init( HAL_RADIO_SPI_ID, SMTC_RADIO_SPI_MOSI, SMTC_RADIO_SPI_MISO, SMTC_RADIO_SPI_SCLK );
}

void hal_mcu_reset( void )
{
    	__disable_irq( );
    	NVIC_SystemReset( );  // Restart system
}

void  hal_mcu_wait_us( const int32_t microseconds )
{
	nrf_delay_us(microseconds);
}

void hal_mcu_set_sleep_for_ms( const int32_t milliseconds )
{
  
}

uint16_t hal_mcu_get_vref_level( void ) { return hal_adc_get_vref_int( ); }

int16_t hal_mcu_get_temperature( void ) { return hal_adc_get_temp( ); }

void hal_mcu_disable_low_power_wait( void )
{

}

void hal_mcu_enable_low_power_wait( void )
{

}

void hal_mcu_trace_print( const char* fmt, ... )
{
}




