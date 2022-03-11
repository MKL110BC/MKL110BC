/*!
 * @file      smtc_hal_i2c.c
 *
 * @brief     Implements the i2c HAL functions
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
#include "smtc_hal.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */


/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*!
 * @brief Write data buffer to the I2C device
 *
 * @param [in] id               I2C interface id [1:N]
 * @param [in] deviceAddr       device address
 * @param [in] addr             data address
 * @param [in] buffer           data buffer to write
 * @param [in] size             number of data bytes to write
 *
 * @returns status [SMTC_HAL_SUCCESS, SMTC_HAL_FAILURE]
 */
static uint8_t i2c_write_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer,
                                 uint16_t size );

/*!
 * @brief Write data buffer to the I2C device
 *
 * @param [in] id               I2C interface id [1:N]
 * @param [in] deviceAddr       device address
 * @param [in] addr             data address
 * @param [in] buffer           data buffer to write
 * @param [in] size             number of data bytes to write
 *
 * @returns status [SMTC_HAL_SUCCESS, SMTC_HAL_FAILURE]
 */
static uint8_t i2c_read_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void hal_i2c_init( const uint32_t id, const hal_gpio_pin_names_t sda, const hal_gpio_pin_names_t scl )
{

}

void hal_i2c_deinit( const uint32_t id )
{

}


uint8_t hal_i2c_write( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t data )
{

	return SMTC_HAL_SUCCESS;
}

uint8_t hal_i2c_write_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size )
{
	return SMTC_HAL_SUCCESS;
}

uint8_t hal_i2c_read( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* data )
{
    return ( i2c_read_buffer( id, device_addr, addr, data, 1 ) );
}

uint8_t hal_i2c_read_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size )
{
    return ( i2c_read_buffer( id, device_addr, addr, buffer, size ) );
}

void i2c_set_addr_size( i2c_addr_size addr_size ) {  }

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static uint8_t i2c_write_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size )
{
 

    return SMTC_HAL_SUCCESS;
}

static uint8_t i2c_read_buffer( const uint32_t id, uint8_t device_addr, uint16_t addr, uint8_t* buffer, uint16_t size )
{

    return SMTC_HAL_SUCCESS;
}
