/*!
 * \file      smtc_hal_spi.c
 *
 * \brief     SPI Hardware Abstraction Layer implementation
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
#include "modem_pinout.h"



static hal_gpio_pin_names_t spi_miso_pin;
static hal_gpio_pin_names_t spi_mosi_pin;
static hal_gpio_pin_names_t spi_sclk_pin;

void hal_spi_init( const uint32_t id, const hal_gpio_pin_names_t mosi, const hal_gpio_pin_names_t miso,
                   const hal_gpio_pin_names_t sclk )
{
	spi_miso_pin = miso;
	spi_mosi_pin = mosi;
	spi_sclk_pin = sclk;
	hal_gpio_init_out(sclk,HAL_GPIO_RESET);
	hal_gpio_init_out(mosi,HAL_GPIO_RESET);
	
	
}

void hal_spi_deinit( const uint32_t id )
{
	hal_gpio_deinit(spi_miso_pin);
	hal_gpio_deinit(spi_mosi_pin);
	hal_gpio_deinit(spi_sclk_pin);
}

uint16_t hal_spi_in_out( const uint32_t id, const uint16_t out_data )
{
	uint8_t read_data,write_data;
	write_data = out_data;

	hal_gpio_init_in(spi_miso_pin,HAL_GPIO_PULL_MODE_NONE,HAL_GPIO_IRQ_MODE_OFF,NULL);


	
	for(uint8_t i=0; i<8; i++) 
	{
		if(write_data&0x80)
		{
			hal_gpio_set_value(spi_mosi_pin,HAL_GPIO_SET);
		}
		else
		{
			hal_gpio_set_value(spi_mosi_pin,HAL_GPIO_RESET);
		}
		hal_gpio_set_value(spi_sclk_pin,HAL_GPIO_SET);

		write_data <<=1;
		read_data<<=1;
		if(hal_gpio_get_value(spi_miso_pin))
		read_data|=0x01;
		hal_gpio_set_value(spi_sclk_pin,HAL_GPIO_RESET);
	}

	//to decrease  power
	hal_gpio_deinit(spi_miso_pin);
	return read_data;
}

