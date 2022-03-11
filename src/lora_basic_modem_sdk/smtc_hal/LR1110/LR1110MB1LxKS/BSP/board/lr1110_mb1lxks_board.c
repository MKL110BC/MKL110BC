/*!
 * @file      lr1110_mb1lxks_board.c
 *
 * @brief     Target board LR1110 MB1LxKS shield board board driver implementation
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

#include <stdlib.h>


#include "lorawan_key_config.h"
#include "smtc_board_ralf.h"
#include "lr1110_mb1lxks_board.h"
#include "ralf_lr1110.h"
#include "modem_pinout.h"
#include "lr1110_hal_context.h"

#include "smtc_hal.h"
#include "modem_pinout.h"
#include "smtc_modem_api_lr1110_system.h"
#include "smtc_modem_test_api.h"
#include "lr1110_types.h"
#include "smtc_modem_hal_dbg_trace.h"

void lna_on( void ) { hal_gpio_set_value( RADIO_LNA_CTRL, 1 ); }

void lna_off( void ) { hal_gpio_set_value( RADIO_LNA_CTRL, 0 ); }

void smtc_board_lna_on( void ) { lna_on( ); }

void smtc_board_lna_off( void ) { lna_off( ); }

ralf_t* smtc_board_initialise_and_get_ralf( void )
{
    static lr1110_hal_context_t context = {
        .nss    = SMTC_RADIO_NSS,
        .busy   = SMTC_RADIO_BUSY,
        .reset  = SMTC_RADIO_NRST,
        .spi_id = HAL_RADIO_SPI_ID,
    };
    static ralf_t local_ralf = { 0 };
    local_ralf               = ( ralf_t ) RALF_LR1110_INSTANTIATE( &context );
    return &local_ralf;
}
int smtc_board_get_tx_power_offset( void )
{
    return BOARD_TX_POWER_OFFSET;
}
