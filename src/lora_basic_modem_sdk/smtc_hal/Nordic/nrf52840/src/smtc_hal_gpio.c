/*!
 * @file      smtc_hal_gpio.c
 *
 * @brief     Implements the gpio HAL functions
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

#include "smtc_hal.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

//这边通过IO口定义，实际上GPIOTE只有8通道
static hal_gpio_irq_t const* gpio_irq[PIN_MAX];
#include "system_port.h"
static void pin_irq(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	SYS_LOG_INFO("pin_irq\r\n");
	if(gpio_irq[pin] !=NULL && gpio_irq[pin]->callback != NULL)
	{
		
		 gpio_irq[pin]->callback( gpio_irq[pin]->context );
	}
}

void hal_gpio_init_out( const hal_gpio_pin_names_t pin, const hal_gpio_state_t value )
{
	nrf_gpio_cfg_output(pin);
	nrf_gpio_pin_write(pin,value);
}

void hal_gpio_deinit( const hal_gpio_pin_names_t pin )
{
    	nrf_gpio_cfg_default(pin);
}

/**
 * @brief MCU input pin Handling
 */

void hal_gpio_init_in( const hal_gpio_pin_names_t pin, const hal_gpio_pull_mode_t pull_mode,
                       const hal_gpio_irq_mode_t irq_mode, hal_gpio_irq_t* irq )
{
	nrf_gpio_cfg_input(pin,pull_mode);
	//如果配置中断，就开启中断
	if( irq_mode!=HAL_GPIO_IRQ_MODE_OFF)
	{
		static uint8_t flag = 0;
		if(flag ==0)
		{	
			flag = 1;
			nrf_drv_gpiote_init();
		}
		
		nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
		switch(irq_mode)
		{
			case HAL_GPIO_IRQ_MODE_RISING:

				break;
			case HAL_GPIO_IRQ_MODE_FALLING:
				in_config.sense =   NRF_GPIOTE_POLARITY_HITOLO;
				break;
			case HAL_GPIO_IRQ_MODE_RISING_FALLING:
				in_config.sense =   NRF_GPIOTE_POLARITY_TOGGLE;
				break;
		}
		nrf_drv_gpiote_in_init(pin,&in_config,pin_irq);
		nrf_drv_gpiote_in_event_enable(pin,1);
	}
	//如果配置回调函数
	if(irq)
	{
		irq->pin = pin;
		hal_gpio_irq_attach(irq);
	}
}
//设置中断回调
void hal_gpio_irq_attach( const hal_gpio_irq_t* irq )
{
    if( ( irq != NULL ) && ( irq->callback != NULL ) )
    {
        gpio_irq[ irq->pin ] = irq;
    }
}
//删除中断回调
void hal_gpio_irq_deatach( const hal_gpio_irq_t* irq )
{
    if( irq != NULL )
    {
        gpio_irq[ irq->pin ] = NULL;
    }
}

void hal_gpio_irq_enable( void )
{
	for(uint8_t i=0 ; i<PIN_MAX  ; i++)
	{
		//有回调函数
		if(gpio_irq[i])
		{
			//nrf_drv_gpiote_in_event_enable(i,1);
		}
	}
}

void hal_gpio_irq_disable( void )
{
	for(uint8_t i=0 ; i<PIN_MAX  ; i++)
	{
		//有回调函数
		if(gpio_irq[i])
		{
		//	nrf_drv_gpiote_in_event_disable(i);
		}
	}
}

/**
 * @brief MCU pin state control
 */

void hal_gpio_set_value( const hal_gpio_pin_names_t pin, const hal_gpio_state_t value )
{
    	nrf_gpio_pin_write(pin,value);
}

void hal_gpio_toggle( const hal_gpio_pin_names_t pin )
{
	nrf_gpio_pin_toggle(pin);
}

uint32_t hal_gpio_get_value( const hal_gpio_pin_names_t pin )
{
    return nrf_gpio_pin_read(pin);
}

bool hal_gpio_is_pending_irq( const hal_gpio_pin_names_t pin )
{
    bool pending = false;
    return ( bool ) pending;
}


