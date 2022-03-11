/**
 * @file      ral_lr1110.c
 *
 * @brief     Radio abstraction layer definition
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

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "lr1110_system.h"
#include "lr1110_radio.h"
#include "lr1110_regmem.h"
#include "ral_lr1110.h"
#include "ral_lr1110_bsp.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */
#define LR1110_LP_MIN_OUTPUT_POWER -17
#define LR1110_LP_MAX_OUTPUT_POWER 15

#define LR1110_HP_MIN_OUTPUT_POWER -9
#define LR1110_HP_MAX_OUTPUT_POWER 22

#define LR1110_LP_CONVERT_TABLE_INDEX_OFFSET 17
#define LR1110_HP_CONVERT_TABLE_INDEX_OFFSET 9

static const uint32_t ral_lr1110_convert_tx_dbm_to_ua_reg_mode_dcdc_lp_vreg[] = {
    10820,  // -17 dBm
    10980,  // -16 dBm
    11060,  // -15 dBm
    11160,  // -14 dBm
    11300,  // -13 dBm
    11430,  // -12 dBm
    11550,  // -11 dBm
    11680,  // -10 dBm
    11930,  //  -9 dBm
    12170,  //  -8 dBm
    12420,  //  -7 dBm
    12650,  //  -6 dBm
    12900,  //  -5 dBm
    13280,  //  -4 dBm
    13600,  //  -3 dBm
    14120,  //  -2 dBm
    14600,  //  -1 dBm
    15090,  //   0 dBm
    15780,  //   1 dBm
    16490,  //   2 dBm
    17250,  //   3 dBm
    17850,  //   4 dBm
    18720,  //   5 dBm
    19640,  //   6 dBm
    20560,  //   7 dBm
    21400,  //   8 dBm
    22620,  //   9 dBm
    23720,  //  10 dBm
    25050,  //  11 dBm
    26350,  //  12 dBm
    27870,  //  13 dBm
    28590,  //  14 dBm
    37820,  //  15 dBm
};

static const uint32_t ral_lr1110_convert_tx_dbm_to_ua_reg_mode_ldo_lp_vreg[] = {
    14950,  // -17 dBm
    15280,  // -16 dBm
    15530,  // -15 dBm
    15770,  // -14 dBm
    16020,  // -13 dBm
    16290,  // -12 dBm
    16550,  // -11 dBm
    16760,  // -10 dBm
    17280,  //  -9 dBm
    17770,  //  -8 dBm
    18250,  //  -7 dBm
    18750,  //  -6 dBm
    19250,  //  -5 dBm
    19960,  //  -4 dBm
    20710,  //  -3 dBm
    21620,  //  -2 dBm
    22570,  //  -1 dBm
    23570,  //   0 dBm
    24990,  //   1 dBm
    26320,  //   2 dBm
    27830,  //   3 dBm
    29070,  //   4 dBm
    30660,  //   5 dBm
    32490,  //   6 dBm
    34220,  //   7 dBm
    35820,  //   8 dBm
    38180,  //   9 dBm
    40220,  //  10 dBm
    42800,  //  11 dBm
    45030,  //  12 dBm
    47900,  //  13 dBm
    51220,  //  14 dBm
    66060,  //  15 dBm
};

static const uint32_t ral_lr1110_convert_tx_dbm_to_ua_reg_mode_dcdc_hp_vbat[] = {
    27750,   //  -9 dBm
    29100,   //  -8 dBm
    30320,   //  -7 dBm
    31650,   //  -6 dBm
    34250,   //  -5 dBm
    35550,   //  -4 dBm
    36770,   //  -3 dBm
    39250,   //  -2 dBm
    41480,   //  -1 dBm
    43820,   //   0 dBm
    46000,   //   1 dBm
    49020,   //   2 dBm
    50900,   //   3 dBm
    54200,   //   4 dBm
    56330,   //   5 dBm
    59050,   //   6 dBm
    62210,   //   7 dBm
    65270,   //   8 dBm
    68600,   //   9 dBm
    71920,   //  10 dBm
    75500,   //  11 dBm
    79500,   //  12 dBm
    84130,   //  13 dBm
    88470,   //  14 dBm
    92200,   //  15 dBm
    94340,   //  16 dBm
    96360,   //  17 dBm
    98970,   //  18 dBm
    102220,  //  19 dBm
    106250,  //  20 dBm
    111300,  //  21 dBm
    113040,  //  22 dBm
};

static const uint32_t ral_lr1110_convert_tx_dbm_to_ua_reg_mode_ldo_hp_vbat[] = {
    31310,   //  -9 dBm
    32700,   //  -8 dBm
    33970,   //  -7 dBm
    35270,   //  -6 dBm
    37900,   //  -5 dBm
    39140,   //  -4 dBm
    40380,   //  -3 dBm
    42860,   //  -2 dBm
    45150,   //  -1 dBm
    47400,   //   0 dBm
    49600,   //   1 dBm
    52600,   //   2 dBm
    54460,   //   3 dBm
    57690,   //   4 dBm
    59840,   //   5 dBm
    62550,   //   6 dBm
    65750,   //   7 dBm
    68520,   //   8 dBm
    72130,   //   9 dBm
    75230,   //  10 dBm
    78600,   //  11 dBm
    82770,   //  12 dBm
    87450,   //  13 dBm
    91700,   //  14 dBm
    95330,   //  15 dBm
    97520,   //  16 dBm
    99520,   //  17 dBm
    102080,  //  18 dBm
    105140,  //  19 dBm
    109300,  //  20 dBm
    114460,  //  21 dBm
    116530,  //  22 dBm
};

// TODO: check values
#define LR1110_GFSK_RX_CONSUMPTION_DCDC 5400
#define LR1110_GFSK_RX_BOOSTED_CONSUMPTION_DCDC 7500

#define LR1110_GFSK_RX_CONSUMPTION_LDO 5400
#define LR1110_GFSK_RX_BOOSTED_CONSUMPTION_LDO 7500

#define LR1110_LORA_RX_CONSUMPTION_DCDC 5700
#define LR1110_LORA_RX_BOOSTED_CONSUMPTION_DCDC 7800

#define LR1110_LORA_RX_CONSUMPTION_LDO 5700
#define LR1110_LORA_RX_BOOSTED_CONSUMPTION_LDO 7800

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/**
 * @brief Convert interrupt flags from LR1110 context to RAL context
 *
 * @param [in] lr1110_irq  LR1110 interrupt status
 *
 * @returns RAL interrupt status
 */
static ral_irq_t ral_lr1110_convert_irq_flags_to_ral( lr1110_system_irq_mask_t lr1110_irq );

/**
 * @brief Convert interrupt flags from RAL context to LR1110 context
 *
 * @param [in] ral_irq RAL interrupt status
 *
 * @returns LR1110 interrupt status
 */
static lr1110_system_irq_mask_t ral_lr1110_convert_irq_flags_from_ral( ral_irq_t ral_irq );

/**
 * @brief Convert GFSK modulation parameters from RAL context to LR1110 context
 *
 * @param [in] ral_mod_params     RAL modulation parameters
 * @param [out] radio_mod_params  Radio modulation parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_lr1110_convert_gfsk_mod_params_from_ral( const ral_gfsk_mod_params_t*    ral_mod_params,
                                                                 lr1110_radio_mod_params_gfsk_t* radio_mod_params );

/**
 * @brief Convert GFSK packet parameters from RAL context to LR1110 context
 *
 * @param [in] ral_pkt_params     RAL packet parameters
 * @param [out] radio_pkt_params  Radio packet parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_lr1110_convert_gfsk_pkt_params_from_ral( const ral_gfsk_pkt_params_t*    ral_pkt_params,
                                                                 lr1110_radio_pkt_params_gfsk_t* radio_pkt_params );

/**
 * @brief Convert LoRa modulation parameters from RAL context to LR1110 context
 *
 * @param [in] ral_mod_params     RAL modulation parameters
 * @param [out] radio_mod_params  Radio modulation parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_lr1110_convert_lora_mod_params_from_ral( const ral_lora_mod_params_t*    ral_mod_params,
                                                                 lr1110_radio_mod_params_lora_t* radio_mod_params );

/**
 * @brief Convert LoRa packet parameters from RAL context to LR1110 context
 *
 * @param [in] ral_pkt_params     RAL packet parameters
 * @param [out] radio_pkt_params  Radio packet parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_lr1110_convert_lora_pkt_params_from_ral( const ral_lora_pkt_params_t*    ral_pkt_params,
                                                                 lr1110_radio_pkt_params_lora_t* radio_pkt_params );

/**
 * @brief Convert LoRa CAD parameters from RAL context to LR1110 context
 *
 * @param [in] ral_lora_cad_params     RAL LoRa CAD parameters
 * @param [out] radio_lora_cad_params  Radio LoRa CAD parameters
 *
 * @returns Operation status
 */
static ral_status_t ral_lr1110_convert_lora_cad_params_from_ral( const ral_lora_cad_params_t* ral_lora_cad_params,
                                                                 lr1110_radio_cad_params_t*   radio_lora_cad_params );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

bool ral_lr1110_handles_part( const char* part_number )
{
    return strcmp( "lr1110", part_number ) == 0;
}

ral_status_t ral_lr1110_reset( const void* context )
{
    return ( ral_status_t ) lr1110_system_reset( context );
}

ral_status_t ral_lr1110_init( const void* context )
{
    ral_status_t                        status = RAL_STATUS_ERROR;
    lr1110_system_tcxo_supply_voltage_t tcxo_supply_voltage;
    lr1110_system_reg_mode_t            reg_mode;
    lr1110_system_rfswitch_cfg_t        system_rf_switch_cfg     = { 0 };
    bool                                tcxo_is_radio_controlled = false;
    bool                                crc_is_activated         = false;
    uint32_t                            startup_time_in_tick     = 0;

    ral_lr1110_bsp_get_crc_state( context, &crc_is_activated );
    if( crc_is_activated == true )
    {
        status = ( ral_status_t ) lr1110_system_enable_spi_crc( context, true );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
    }

    ral_lr1110_bsp_get_reg_mode( context, &reg_mode );
    status = ( ral_status_t ) lr1110_system_set_reg_mode( context, reg_mode );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    ral_lr1110_bsp_get_rf_switch_cfg( context, &system_rf_switch_cfg );
    status = ( ral_status_t ) lr1110_system_set_dio_as_rf_switch( context, &system_rf_switch_cfg );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    ral_lr1110_bsp_get_xosc_cfg( context, &tcxo_is_radio_controlled, &tcxo_supply_voltage, &startup_time_in_tick );
    if( tcxo_is_radio_controlled == true )
    {
        status = ( ral_status_t ) lr1110_system_set_tcxo_mode( context, tcxo_supply_voltage, startup_time_in_tick );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }

        status = ( ral_status_t ) lr1110_system_calibrate(
            context, LR1110_SYSTEM_CALIB_LF_RC_MASK | LR1110_SYSTEM_CALIB_HF_RC_MASK | LR1110_SYSTEM_CALIB_PLL_MASK |
                         LR1110_SYSTEM_CALIB_ADC_MASK | LR1110_SYSTEM_CALIB_IMG_MASK |
                         LR1110_SYSTEM_CALIB_PLL_TX_MASK );
        if( status != RAL_STATUS_OK )
        {
            return status;
        }
    }

    return status;
}

ral_status_t ral_lr1110_wakeup( const void* context )
{
    return ( ral_status_t ) lr1110_system_wakeup( context );
}

ral_status_t ral_lr1110_set_sleep( const void* context, const bool retain_config )
{
    ral_status_t              status = RAL_STATUS_ERROR;
    lr1110_system_sleep_cfg_t radio_sleep_cfg;

    radio_sleep_cfg.is_warm_start  = retain_config;
    radio_sleep_cfg.is_rtc_timeout = false;

    // Workaround: force LF clock to RC otherwise the wake-up sequence will not work
    status = ( ral_status_t ) lr1110_system_cfg_lfclk( context, LR1110_SYSTEM_LFCLK_RC, true );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) lr1110_system_set_sleep( context, radio_sleep_cfg, 0 );
}

ral_status_t ral_lr1110_set_standby( const void* context, ral_standby_cfg_t ral_standby_cfg )
{
    lr1110_system_standby_cfg_t radio_standby_cfg;

    switch( ral_standby_cfg )
    {
    case RAL_STANDBY_CFG_RC:
    {
        radio_standby_cfg = LR1110_SYSTEM_STANDBY_CFG_RC;
        break;
    }
    case RAL_STANDBY_CFG_XOSC:
    {
        radio_standby_cfg = LR1110_SYSTEM_STANDBY_CFG_XOSC;
        break;
    }
    default:
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    return ( ral_status_t ) lr1110_system_set_standby( context, radio_standby_cfg );
}

ral_status_t ral_lr1110_set_fs( const void* context )
{
    return ( ral_status_t ) lr1110_system_set_fs( context );
}

ral_status_t ral_lr1110_set_tx( const void* context )
{
    return ( ral_status_t ) lr1110_radio_set_tx( context, 0 );
}

ral_status_t ral_lr1110_set_rx( const void* context, const uint32_t timeout_in_ms )
{
    if( timeout_in_ms == RAL_RX_TIMEOUT_CONTINUOUS_MODE )
    {
        return ( ral_status_t ) lr1110_radio_set_rx_with_timeout_in_rtc_step( context, 0x00FFFFFF );
    }
    else
    {  // max timeout is 0xFFFFFE -> 511999 ms (0xFFFFFE / 32768 * 1000) - Single reception mode set if timeout_ms is 0
        if( timeout_in_ms < 512000 )
        {
            return ( ral_status_t ) lr1110_radio_set_rx( context, timeout_in_ms );
        }
        else
        {
            return RAL_STATUS_ERROR;
        }
    }

    return RAL_STATUS_ERROR;
}

ral_status_t ral_lr1110_cfg_rx_boosted( const void* context, const bool enable_boost_mode )
{
    return ( ral_status_t ) lr1110_radio_cfg_rx_boosted( context, enable_boost_mode );
}

ral_status_t ral_lr1110_set_rx_tx_fallback_mode( const void* context, const ral_fallback_modes_t ral_fallback_mode )
{
    lr1110_radio_fallback_modes_t radio_fallback_mode;

    switch( ral_fallback_mode )
    {
    case RAL_FALLBACK_STDBY_RC:
    {
        radio_fallback_mode = LR1110_RADIO_FALLBACK_STDBY_RC;
        break;
    }
    case RAL_FALLBACK_STDBY_XOSC:
    {
        radio_fallback_mode = LR1110_RADIO_FALLBACK_STDBY_XOSC;
        break;
    }
    case RAL_FALLBACK_FS:
    {
        radio_fallback_mode = LR1110_RADIO_FALLBACK_FS;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return ( ral_status_t ) lr1110_radio_set_rx_tx_fallback_mode( context, radio_fallback_mode );
}

ral_status_t ral_lr1110_stop_timer_on_preamble( const void* context, const bool enable )
{
    return ( ral_status_t ) lr1110_radio_stop_timeout_on_preamble( context, enable );
}

ral_status_t ral_lr1110_set_rx_duty_cycle( const void* context, const uint32_t rx_time_in_ms,
                                           const uint32_t sleep_time_in_ms )
{
    return ( ral_status_t ) lr1110_radio_set_rx_duty_cycle( context, rx_time_in_ms, sleep_time_in_ms,
                                                            LR1110_RADIO_RX_DUTY_CYCLE_MODE_RX );
}

ral_status_t ral_lr1110_set_lora_cad( const void* context )
{
    return ( ral_status_t ) lr1110_radio_set_cad( context );
}

ral_status_t ral_lr1110_set_tx_cw( const void* context )
{
    return ( ral_status_t ) lr1110_radio_set_tx_cw( context );
}

ral_status_t ral_lr1110_set_tx_infinite_preamble( const void* context )
{
    return ( ral_status_t ) lr1110_radio_set_tx_infinite_preamble( context );
}

ral_status_t ral_lr1110_set_tx_cfg( const void* context, const int8_t output_pwr_in_dbm, const uint32_t rf_freq_in_hz )
{
    ral_status_t                               status = RAL_STATUS_ERROR;
    ral_lr1110_bsp_tx_cfg_output_params_t      tx_cfg_output_params;
    const ral_lr1110_bsp_tx_cfg_input_params_t tx_cfg_input_params = {
        .freq_in_hz               = rf_freq_in_hz,
        .system_output_pwr_in_dbm = output_pwr_in_dbm,
    };

    ral_lr1110_bsp_get_tx_cfg( context, &tx_cfg_input_params, &tx_cfg_output_params );

    status = ( ral_status_t ) lr1110_radio_set_pa_cfg( context, &tx_cfg_output_params.pa_cfg );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    status = ( ral_status_t ) lr1110_radio_set_tx_params(
        context, tx_cfg_output_params.chip_output_pwr_in_dbm_configured, tx_cfg_output_params.pa_ramp_time );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return status;
}

ral_status_t ral_lr1110_set_pkt_payload( const void* context, const uint8_t* buffer, const uint16_t size )
{
    return ( ral_status_t ) lr1110_regmem_write_buffer8( context, buffer, size );
}

ral_status_t ral_lr1110_get_pkt_payload( const void* context, uint16_t max_size_in_bytes, uint8_t* buffer,
                                         uint16_t* size_in_bytes )
{
    ral_status_t                    status                 = RAL_STATUS_ERROR;
    lr1110_radio_rx_buffer_status_t radio_rx_buffer_status = { 0 };

    status = ( ral_status_t ) lr1110_radio_get_rx_buffer_status( context, &radio_rx_buffer_status );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    if( size_in_bytes != 0 )
    {
        *size_in_bytes = radio_rx_buffer_status.pld_len_in_bytes;
    }

    if( radio_rx_buffer_status.pld_len_in_bytes <= max_size_in_bytes )
    {
        status = ( ral_status_t ) lr1110_regmem_read_buffer8(
            context, buffer, radio_rx_buffer_status.buffer_start_pointer, radio_rx_buffer_status.pld_len_in_bytes );
    }
    else
    {
        status = RAL_STATUS_ERROR;
    }

    return status;
}

ral_status_t ral_lr1110_get_irq_status( const void* context, ral_irq_t* irq )
{
    ral_status_t             status         = RAL_STATUS_ERROR;
    lr1110_system_irq_mask_t radio_irq_mask = LR1110_SYSTEM_IRQ_NONE;

    status = ( ral_status_t ) lr1110_system_get_irq_status( context, &radio_irq_mask );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    *irq = ral_lr1110_convert_irq_flags_to_ral( radio_irq_mask );

    return status;
}

ral_status_t ral_lr1110_clear_irq_status( const void* context, const ral_irq_t irq )
{
    const lr1110_system_irq_mask_t radio_irq = ral_lr1110_convert_irq_flags_from_ral( irq );

    return ( ral_status_t ) lr1110_system_clear_irq_status( context, radio_irq );
}

ral_status_t ral_lr1110_get_and_clear_irq_status( const void* context, ral_irq_t* irq )
{
    ral_status_t             status    = RAL_STATUS_ERROR;
    lr1110_system_irq_mask_t radio_irq = LR1110_SYSTEM_IRQ_NONE;

    status = ( ral_status_t ) lr1110_system_get_and_clear_irq_status( context, &radio_irq );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    if( irq != 0 )
    {
        *irq = ral_lr1110_convert_irq_flags_to_ral( radio_irq );
    }

    return status;
}

ral_status_t ral_lr1110_set_dio_irq_params( const void* context, const ral_irq_t irq )
{
    lr1110_system_irq_mask_t lr1110_irq = ral_lr1110_convert_irq_flags_from_ral( irq );

    return ( ral_status_t ) lr1110_system_set_dio_irq_params( context, lr1110_irq, LR1110_SYSTEM_IRQ_NONE );
}

ral_status_t ral_lr1110_set_rf_freq( const void* context, const uint32_t freq_in_hz )
{
    return ( ral_status_t ) lr1110_radio_set_rf_freq( context, freq_in_hz );
}

ral_status_t ral_lr1110_set_pkt_type( const void* context, const ral_pkt_type_t ral_pkt_type )
{
    lr1110_radio_pkt_type_t radio_pkt_type;

    switch( ral_pkt_type )
    {
    case RAL_PKT_TYPE_GFSK:
    {
        radio_pkt_type = LR1110_RADIO_PKT_TYPE_GFSK;
        break;
    }
    case RAL_PKT_TYPE_LORA:
    {
        radio_pkt_type = LR1110_RADIO_PKT_TYPE_LORA;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return ( ral_status_t ) lr1110_radio_set_pkt_type( context, radio_pkt_type );
}

ral_status_t ral_lr1110_set_gfsk_mod_params( const void* context, const ral_gfsk_mod_params_t* ral_mod_params )
{
    ral_status_t                   status           = RAL_STATUS_ERROR;
    lr1110_radio_mod_params_gfsk_t radio_mod_params = { 0 };

    status = ral_lr1110_convert_gfsk_mod_params_from_ral( ral_mod_params, &radio_mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) lr1110_radio_set_gfsk_mod_params( context, &radio_mod_params );
}

ral_status_t ral_lr1110_set_gfsk_pkt_params( const void* context, const ral_gfsk_pkt_params_t* ral_pkt_params )
{
    ral_status_t                   status           = RAL_STATUS_ERROR;
    lr1110_radio_pkt_params_gfsk_t radio_pkt_params = { 0 };

    status = ral_lr1110_convert_gfsk_pkt_params_from_ral( ral_pkt_params, &radio_pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) lr1110_radio_set_gfsk_pkt_params( context, &radio_pkt_params );
}

ral_status_t ral_lr1110_set_lora_mod_params( const void* context, const ral_lora_mod_params_t* ral_mod_params )
{
    ral_status_t                   status = RAL_STATUS_ERROR;
    lr1110_radio_mod_params_lora_t radio_mod_params;

    status = ral_lr1110_convert_lora_mod_params_from_ral( ral_mod_params, &radio_mod_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) lr1110_radio_set_lora_mod_params( context, &radio_mod_params );
}

ral_status_t ral_lr1110_set_lora_pkt_params( const void* context, const ral_lora_pkt_params_t* ral_pkt_params )
{
    ral_status_t                   status           = RAL_STATUS_ERROR;
    lr1110_radio_pkt_params_lora_t radio_pkt_params = { 0 };

    status = ral_lr1110_convert_lora_pkt_params_from_ral( ral_pkt_params, &radio_pkt_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) lr1110_radio_set_lora_pkt_params( context, &radio_pkt_params );
}

ral_status_t ral_lr1110_set_lora_cad_params( const void* context, const ral_lora_cad_params_t* ral_lora_cad_params )
{
    ral_status_t              status = RAL_STATUS_ERROR;
    lr1110_radio_cad_params_t radio_lora_cad_params;

    status = ral_lr1110_convert_lora_cad_params_from_ral( ral_lora_cad_params, &radio_lora_cad_params );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    return ( ral_status_t ) lr1110_radio_set_cad_params( context, &radio_lora_cad_params );
}

ral_status_t ral_lr1110_set_lora_symb_nb_timeout( const void* context, const uint8_t nb_of_symbs )
{
    return ( ral_status_t ) lr1110_radio_set_lora_sync_timeout( context, nb_of_symbs );
}

ral_status_t ral_lr1110_set_flrc_mod_params( const void* context, const ral_flrc_mod_params_t* params )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_lr1110_set_flrc_pkt_params( const void* context, const ral_flrc_pkt_params_t* params )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_lr1110_get_gfsk_rx_pkt_status( const void* context, ral_gfsk_rx_pkt_status_t* ral_rx_pkt_status )
{
    ral_status_t                   status = RAL_STATUS_ERROR;
    lr1110_radio_pkt_status_gfsk_t radio_rx_pkt_status;

    status = ( ral_status_t ) lr1110_radio_get_gfsk_pkt_status( context, &radio_rx_pkt_status );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    uint8_t rx_status = 0;
    rx_status |= ( radio_rx_pkt_status.is_sent == true ) ? RAL_RX_STATUS_PKT_SENT : 0x00;
    rx_status |= ( radio_rx_pkt_status.is_received == true ) ? RAL_RX_STATUS_PKT_RECEIVED : 0x00;
    rx_status |= ( radio_rx_pkt_status.is_abort_err == true ) ? RAL_RX_STATUS_ABORT_ERROR : 0x00;
    rx_status |= ( radio_rx_pkt_status.is_len_err == true ) ? RAL_RX_STATUS_LENGTH_ERROR : 0x00;
    rx_status |= ( radio_rx_pkt_status.is_crc_err == true ) ? RAL_RX_STATUS_CRC_ERROR : 0x00;
    rx_status |= ( radio_rx_pkt_status.is_addr_err == true ) ? RAL_RX_STATUS_ADDR_ERROR : 0x00;

    ral_rx_pkt_status->rx_status = rx_status;

    ral_rx_pkt_status->rssi_sync_in_dbm = radio_rx_pkt_status.rssi_sync_in_dbm;
    ral_rx_pkt_status->rssi_avg_in_dbm  = radio_rx_pkt_status.rssi_avg_in_dbm;

    return status;
}

ral_status_t ral_lr1110_get_lora_rx_pkt_status( const void* context, ral_lora_rx_pkt_status_t* ral_rx_pkt_status )
{
    ral_status_t                   status = RAL_STATUS_ERROR;
    lr1110_radio_pkt_status_lora_t radio_rx_pkt_status;

    status = ( ral_status_t ) lr1110_radio_get_lora_pkt_status( context, &radio_rx_pkt_status );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    ral_rx_pkt_status->rssi_pkt_in_dbm        = radio_rx_pkt_status.rssi_pkt_in_dbm;
    ral_rx_pkt_status->snr_pkt_in_db          = radio_rx_pkt_status.snr_pkt_in_db;
    ral_rx_pkt_status->signal_rssi_pkt_in_dbm = radio_rx_pkt_status.signal_rssi_pkt_in_dbm;

    return status;
}

ral_status_t ral_lr1110_get_flrc_rx_pkt_status( const void* context, ral_flrc_rx_pkt_status_t* rx_pkt_status )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_lr1110_get_rssi_inst( const void* context, int16_t* rssi_in_dbm )
{
    int8_t          local_rssi_in_dbm;
    lr1110_status_t status = lr1110_radio_get_rssi_inst( context, &local_rssi_in_dbm );
    *rssi_in_dbm           = ( int16_t ) local_rssi_in_dbm;

    return ( ral_status_t ) status;
}

uint32_t ral_lr1110_get_lora_time_on_air_in_ms( const ral_lora_pkt_params_t* pkt_p, const ral_lora_mod_params_t* mod_p )
{
    lr1110_radio_mod_params_lora_t radio_mod_params;
    lr1110_radio_pkt_params_lora_t radio_pkt_params;

    ral_lr1110_convert_lora_mod_params_from_ral( mod_p, &radio_mod_params );
    ral_lr1110_convert_lora_pkt_params_from_ral( pkt_p, &radio_pkt_params );

    return lr1110_radio_get_lora_time_on_air_in_ms( &radio_pkt_params, &radio_mod_params );
}

uint32_t ral_lr1110_get_gfsk_time_on_air_in_ms( const ral_gfsk_pkt_params_t* pkt_p, const ral_gfsk_mod_params_t* mod_p )
{
    lr1110_radio_mod_params_gfsk_t radio_mod_params;
    lr1110_radio_pkt_params_gfsk_t radio_pkt_params;

    ral_lr1110_convert_gfsk_mod_params_from_ral( mod_p, &radio_mod_params );
    ral_lr1110_convert_gfsk_pkt_params_from_ral( pkt_p, &radio_pkt_params );

    return lr1110_radio_get_gfsk_time_on_air_in_ms( &radio_pkt_params, &radio_mod_params );
}

uint32_t ral_lr1110_get_flrc_time_on_air_in_ms( const ral_flrc_pkt_params_t* pkt_p, const ral_flrc_mod_params_t* mod_p )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_lr1110_set_gfsk_sync_word( const void* context, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    uint8_t sync_word_loc[8] = { 0 };

    for( uint8_t i = 0; i < sync_word_len; i++ )
    {
        sync_word_loc[i] = sync_word[i];
    }

    return ( ral_status_t ) lr1110_radio_set_gfsk_sync_word( context, sync_word_loc );
}

ral_status_t ral_lr1110_set_lora_sync_word( const void* context, const uint8_t sync_word )
{
    return ( ral_status_t ) lr1110_radio_set_lora_sync_word( context, sync_word );
}

ral_status_t ral_lr1110_set_flrc_sync_word( const void* context, const uint8_t* sync_word, const uint8_t sync_word_len )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_lr1110_set_gfsk_crc_params( const void* context, const uint16_t seed, const uint16_t polynomial )
{
    return ( ral_status_t ) lr1110_radio_set_gfsk_crc_params( context, seed, polynomial );
}

ral_status_t ral_lr1110_set_flrc_crc_params( const void* context, const uint32_t seed )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_lr1110_set_gfsk_whitening_seed( const void* context, const uint16_t seed )
{
    return ( ral_status_t ) lr1110_radio_set_gfsk_whitening_seed( context, seed );
}

ral_status_t ral_lr1110_get_lora_rx_pkt_cr_crc( const void* context, ral_lora_cr_t* cr, bool* is_crc_present )
{
    return RAL_STATUS_UNSUPPORTED_FEATURE;
}

ral_status_t ral_lr1110_get_tx_consumption_in_ua( const void* context, const int8_t output_pwr_in_dbm,
                                                  const uint32_t rf_freq_in_hz, uint32_t* pwr_consumption_in_ua )
{
    lr1110_system_reg_mode_t                   radio_reg_mode;
    ral_lr1110_bsp_tx_cfg_output_params_t      tx_cfg_output_params;
    const ral_lr1110_bsp_tx_cfg_input_params_t tx_cfg_input_params = {
        .freq_in_hz               = rf_freq_in_hz,
        .system_output_pwr_in_dbm = output_pwr_in_dbm,
    };

    ral_lr1110_bsp_get_tx_cfg( context, &tx_cfg_input_params, &tx_cfg_output_params );

    ral_lr1110_bsp_get_reg_mode( context, &radio_reg_mode );

    if( tx_cfg_output_params.pa_cfg.pa_sel == LR1110_RADIO_PA_SEL_LP )
    {
        if( tx_cfg_output_params.pa_cfg.pa_reg_supply == LR1110_RADIO_PA_REG_SUPPLY_VREG )
        {
            uint8_t index = 0;

            if( tx_cfg_output_params.chip_output_pwr_in_dbm_expected > LR1110_LP_MAX_OUTPUT_POWER )
            {
                index = LR1110_LP_MAX_OUTPUT_POWER + LR1110_LP_CONVERT_TABLE_INDEX_OFFSET;
            }
            else if( tx_cfg_output_params.chip_output_pwr_in_dbm_expected < LR1110_LP_MIN_OUTPUT_POWER )
            {
                index = LR1110_LP_MIN_OUTPUT_POWER + LR1110_LP_CONVERT_TABLE_INDEX_OFFSET;
            }
            else
            {
                index = tx_cfg_output_params.chip_output_pwr_in_dbm_expected + LR1110_LP_CONVERT_TABLE_INDEX_OFFSET;
            }

            if( radio_reg_mode == LR1110_SYSTEM_REG_MODE_DCDC )
            {
                *pwr_consumption_in_ua = ral_lr1110_convert_tx_dbm_to_ua_reg_mode_dcdc_lp_vreg[index];
            }
            else
            {
                *pwr_consumption_in_ua = ral_lr1110_convert_tx_dbm_to_ua_reg_mode_ldo_lp_vreg[index];
            }
        }
        else
        {
            return RAL_STATUS_UNSUPPORTED_FEATURE;
        }
    }
    else if( tx_cfg_output_params.pa_cfg.pa_sel == LR1110_RADIO_PA_SEL_HP )
    {
        if( tx_cfg_output_params.pa_cfg.pa_reg_supply == LR1110_RADIO_PA_REG_SUPPLY_VBAT )
        {
            uint8_t index = 0;

            if( tx_cfg_output_params.chip_output_pwr_in_dbm_expected > LR1110_HP_MAX_OUTPUT_POWER )
            {
                index = LR1110_HP_MAX_OUTPUT_POWER + LR1110_HP_CONVERT_TABLE_INDEX_OFFSET;
            }
            else if( tx_cfg_output_params.chip_output_pwr_in_dbm_expected < LR1110_HP_MIN_OUTPUT_POWER )
            {
                index = LR1110_HP_MIN_OUTPUT_POWER + LR1110_HP_CONVERT_TABLE_INDEX_OFFSET;
            }
            else
            {
                index = tx_cfg_output_params.chip_output_pwr_in_dbm_expected + LR1110_HP_CONVERT_TABLE_INDEX_OFFSET;
            }

            if( radio_reg_mode == LR1110_SYSTEM_REG_MODE_DCDC )
            {
                *pwr_consumption_in_ua = ral_lr1110_convert_tx_dbm_to_ua_reg_mode_dcdc_hp_vbat[index];
            }
            else
            {
                *pwr_consumption_in_ua = ral_lr1110_convert_tx_dbm_to_ua_reg_mode_ldo_hp_vbat[index];
            }
        }
        else
        {
            return RAL_STATUS_UNSUPPORTED_FEATURE;
        }
    }
    else
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }

    return RAL_STATUS_OK;
}

ral_status_t ral_lr1110_get_gfsk_rx_consumption_in_ua( const void* context, const uint32_t br_in_bps,
                                                       const uint32_t bw_dsb_in_hz, const bool rx_boosted,
                                                       uint32_t* pwr_consumption_in_ua )
{
    lr1110_system_reg_mode_t radio_reg_mode;

    ral_lr1110_bsp_get_reg_mode( context, &radio_reg_mode );

    if( radio_reg_mode == LR1110_SYSTEM_REG_MODE_DCDC )
    {
        *pwr_consumption_in_ua =
            ( rx_boosted ) ? LR1110_GFSK_RX_BOOSTED_CONSUMPTION_DCDC : LR1110_GFSK_RX_CONSUMPTION_DCDC;
    }
    else
    {
        // TODO: find the good values
        *pwr_consumption_in_ua =
            ( rx_boosted ) ? LR1110_GFSK_RX_BOOSTED_CONSUMPTION_LDO : LR1110_GFSK_RX_CONSUMPTION_LDO;
    }

    return RAL_STATUS_OK;
}

ral_status_t ral_lr1110_get_lora_rx_consumption_in_ua( const void* context, const ral_lora_bw_t bw,
                                                       const bool rx_boosted, uint32_t* pwr_consumption_in_ua )
{
    lr1110_system_reg_mode_t radio_reg_mode;

    ral_lr1110_bsp_get_reg_mode( context, &radio_reg_mode );

    if( radio_reg_mode == LR1110_SYSTEM_REG_MODE_DCDC )
    {
        *pwr_consumption_in_ua =
            ( rx_boosted ) ? LR1110_LORA_RX_BOOSTED_CONSUMPTION_DCDC : LR1110_LORA_RX_CONSUMPTION_DCDC;
    }
    else
    {
        // TODO: find the good values
        *pwr_consumption_in_ua =
            ( rx_boosted ) ? LR1110_LORA_RX_BOOSTED_CONSUMPTION_LDO : LR1110_LORA_RX_CONSUMPTION_LDO;
    }

    return RAL_STATUS_OK;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static ral_irq_t ral_lr1110_convert_irq_flags_to_ral( lr1110_system_irq_mask_t lr1110_irq_status )
{
    ral_irq_t ral_irq = RAL_IRQ_NONE;

    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_TX_DONE ) != 0 )
    {
        ral_irq |= RAL_IRQ_TX_DONE;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_RX_DONE ) != 0 )
    {
        ral_irq |= RAL_IRQ_RX_DONE;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_PREAMBLE_DETECTED ) != 0 )
    {
        ral_irq |= RAL_IRQ_RX_PREAMBLE_DETECTED;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_TIMEOUT ) != 0 )
    {
        ral_irq |= RAL_IRQ_RX_TIMEOUT;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_SYNC_WORD_HEADER_VALID ) != 0 )
    {
        ral_irq |= RAL_IRQ_RX_HDR_OK;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_HEADER_ERROR ) != 0 )
    {
        ral_irq |= RAL_IRQ_RX_HDR_ERROR;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_CRC_ERROR ) != 0 )
    {
        ral_irq |= RAL_IRQ_RX_CRC_ERROR;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_CAD_DONE ) != 0 )
    {
        ral_irq |= RAL_IRQ_CAD_DONE;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_CAD_DETECTED ) != 0 )
    {
        ral_irq |= RAL_IRQ_CAD_OK;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_WIFI_SCAN_DONE ) != 0 )
    {
        ral_irq |= RAL_IRQ_WIFI_SCAN_DONE;
    }
    if( ( lr1110_irq_status & LR1110_SYSTEM_IRQ_GNSS_SCAN_DONE ) != 0 )
    {
        ral_irq |= RAL_IRQ_GNSS_SCAN_DONE;
    }

    return ral_irq;
}

static lr1110_system_irq_mask_t ral_lr1110_convert_irq_flags_from_ral( ral_irq_t ral_irq )
{
    lr1110_system_irq_mask_t lr1110_irq_status = LR1110_SYSTEM_IRQ_NONE;

    if( ( ral_irq & RAL_IRQ_TX_DONE ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_TX_DONE;
    }
    if( ( ral_irq & RAL_IRQ_RX_DONE ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_RX_DONE;
    }
    if( ( ral_irq & RAL_IRQ_RX_PREAMBLE_DETECTED ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_PREAMBLE_DETECTED;
    }
    if( ( ral_irq & RAL_IRQ_RX_TIMEOUT ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_TIMEOUT;
    }
    if( ( ral_irq & RAL_IRQ_RX_HDR_OK ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_SYNC_WORD_HEADER_VALID;
    }
    if( ( ral_irq & RAL_IRQ_RX_HDR_ERROR ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_HEADER_ERROR;
    }
    if( ( ral_irq & RAL_IRQ_RX_CRC_ERROR ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_CRC_ERROR;
    }
    if( ( ral_irq & RAL_IRQ_CAD_DONE ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_CAD_DONE;
    }
    if( ( ral_irq & RAL_IRQ_CAD_OK ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_CAD_DETECTED;
    }
    if( ( ral_irq & RAL_IRQ_WIFI_SCAN_DONE ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_WIFI_SCAN_DONE;
    }
    if( ( ral_irq & RAL_IRQ_GNSS_SCAN_DONE ) != 0 )
    {
        lr1110_irq_status |= LR1110_SYSTEM_IRQ_GNSS_SCAN_DONE;
    }

    return lr1110_irq_status;
}

static ral_status_t ral_lr1110_convert_gfsk_mod_params_from_ral( const ral_gfsk_mod_params_t*    ral_mod_params,
                                                                 lr1110_radio_mod_params_gfsk_t* radio_mod_params )
{
    ral_status_t           status = RAL_STATUS_ERROR;
    lr1110_radio_gfsk_bw_t bw_dsb_param;

    status = ( ral_status_t ) lr1110_radio_get_gfsk_rx_bandwidth( ral_mod_params->bw_dsb_in_hz, &bw_dsb_param );
    if( status != RAL_STATUS_OK )
    {
        return status;
    }

    radio_mod_params->br_in_bps    = ral_mod_params->br_in_bps;
    radio_mod_params->fdev_in_hz   = ral_mod_params->fdev_in_hz;
    radio_mod_params->bw_dsb_param = bw_dsb_param;

    switch( ral_mod_params->pulse_shape )
    {
    case RAL_GFSK_PULSE_SHAPE_OFF:
    {
        radio_mod_params->pulse_shape = LR1110_RADIO_GFSK_PULSE_SHAPE_OFF;
        break;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_03:
    {
        radio_mod_params->pulse_shape = LR1110_RADIO_GFSK_PULSE_SHAPE_BT_03;
        break;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_05:
    {
        radio_mod_params->pulse_shape = LR1110_RADIO_GFSK_PULSE_SHAPE_BT_05;
        break;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_07:
    {
        radio_mod_params->pulse_shape = LR1110_RADIO_GFSK_PULSE_SHAPE_BT_07;
        break;
    }
    case RAL_GFSK_PULSE_SHAPE_BT_1:
    {
        radio_mod_params->pulse_shape = LR1110_RADIO_GFSK_PULSE_SHAPE_BT_1;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return status;
}

static ral_status_t ral_lr1110_convert_gfsk_pkt_params_from_ral( const ral_gfsk_pkt_params_t*    ral_pkt_params,
                                                                 lr1110_radio_pkt_params_gfsk_t* radio_pkt_params )
{
    radio_pkt_params->preamble_len_in_bits = ral_pkt_params->preamble_len_in_bits;

    switch( ral_pkt_params->preamble_detector )
    {
    case RAL_GFSK_PREAMBLE_DETECTOR_OFF:
    {
        radio_pkt_params->preamble_detector = LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_OFF;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_8BITS:
    {
        radio_pkt_params->preamble_detector = LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_8BITS;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_16BITS:
    {
        radio_pkt_params->preamble_detector = LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_16BITS;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_24BITS:
    {
        radio_pkt_params->preamble_detector = LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_24BITS;
        break;
    }
    case RAL_GFSK_PREAMBLE_DETECTOR_MIN_32BITS:
    {
        radio_pkt_params->preamble_detector = LR1110_RADIO_GFSK_PREAMBLE_DETECTOR_MIN_32BITS;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_pkt_params->sync_word_len_in_bits = ral_pkt_params->sync_word_len_in_bits;

    switch( ral_pkt_params->address_filtering )
    {
    case RAL_GFSK_ADDRESS_FILTERING_DISABLE:
    {
        radio_pkt_params->address_filtering = LR1110_RADIO_GFSK_ADDRESS_FILTERING_DISABLE;
        break;
    }
    case RAL_GFSK_ADDRESS_FILTERING_NODE_ADDRESS:
    {
        radio_pkt_params->address_filtering = LR1110_RADIO_GFSK_ADDRESS_FILTERING_NODE_ADDRESS;
        break;
    }
    case RAL_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES:
    {
        radio_pkt_params->address_filtering = LR1110_RADIO_GFSK_ADDRESS_FILTERING_NODE_AND_BROADCAST_ADDRESSES;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    switch( ral_pkt_params->header_type )
    {
    case RAL_GFSK_PKT_FIX_LEN:
    {
        radio_pkt_params->header_type = LR1110_RADIO_GFSK_PKT_FIX_LEN;
        break;
    }
    case RAL_GFSK_PKT_VAR_LEN:
    {
        radio_pkt_params->header_type = LR1110_RADIO_GFSK_PKT_VAR_LEN;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_pkt_params->pld_len_in_bytes = ral_pkt_params->pld_len_in_bytes;

    switch( ral_pkt_params->crc_type )
    {
    case RAL_GFSK_CRC_OFF:
    {
        radio_pkt_params->crc_type = LR1110_RADIO_GFSK_CRC_OFF;
        break;
    }
    case RAL_GFSK_CRC_1_BYTE:
    {
        radio_pkt_params->crc_type = LR1110_RADIO_GFSK_CRC_1_BYTE;
        break;
    }
    case RAL_GFSK_CRC_2_BYTES:
    {
        radio_pkt_params->crc_type = LR1110_RADIO_GFSK_CRC_2_BYTES;
        break;
    }
    case RAL_GFSK_CRC_1_BYTE_INV:
    {
        radio_pkt_params->crc_type = LR1110_RADIO_GFSK_CRC_1_BYTE_INV;
        break;
    }
    case RAL_GFSK_CRC_2_BYTES_INV:
    {
        radio_pkt_params->crc_type = LR1110_RADIO_GFSK_CRC_2_BYTES_INV;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    switch( ral_pkt_params->dc_free )
    {
    case RAL_GFSK_DC_FREE_OFF:
    {
        radio_pkt_params->dc_free = LR1110_RADIO_GFSK_DC_FREE_OFF;
        break;
    }
    case RAL_GFSK_DC_FREE_WHITENING:
    {
        radio_pkt_params->dc_free = LR1110_RADIO_GFSK_DC_FREE_WHITENING;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    return RAL_STATUS_OK;
}

static ral_status_t ral_lr1110_convert_lora_mod_params_from_ral( const ral_lora_mod_params_t*    ral_mod_params,
                                                                 lr1110_radio_mod_params_lora_t* radio_mod_params )
{
    radio_mod_params->sf = ( lr1110_radio_lora_sf_t ) ral_mod_params->sf;

    switch( ral_mod_params->bw )
    {
    case RAL_LORA_BW_010_KHZ:
    {
        radio_mod_params->bw = LR1110_RADIO_LORA_BW_10;
        break;
    }
    case RAL_LORA_BW_015_KHZ:
    {
        radio_mod_params->bw = LR1110_RADIO_LORA_BW_15;
        break;
    }
    case RAL_LORA_BW_020_KHZ:
    {
        radio_mod_params->bw = LR1110_RADIO_LORA_BW_20;
        break;
    }
    case RAL_LORA_BW_031_KHZ:
    {
        radio_mod_params->bw = LR1110_RADIO_LORA_BW_31;
        break;
    }
    case RAL_LORA_BW_041_KHZ:
    {
        radio_mod_params->bw = LR1110_RADIO_LORA_BW_41;
        break;
    }
    case RAL_LORA_BW_062_KHZ:
    {
        radio_mod_params->bw = LR1110_RADIO_LORA_BW_62;
        break;
    }
    case RAL_LORA_BW_125_KHZ:
    {
        radio_mod_params->bw = LR1110_RADIO_LORA_BW_125;
        break;
    }
    case RAL_LORA_BW_250_KHZ:
    {
        radio_mod_params->bw = LR1110_RADIO_LORA_BW_250;
        break;
    }
    case RAL_LORA_BW_500_KHZ:
    {
        radio_mod_params->bw = LR1110_RADIO_LORA_BW_500;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_mod_params->cr = ( lr1110_radio_lora_cr_t ) ral_mod_params->cr;

    radio_mod_params->ldro = ral_mod_params->ldro;

    return RAL_STATUS_OK;
}

static ral_status_t ral_lr1110_convert_lora_pkt_params_from_ral( const ral_lora_pkt_params_t*    ral_pkt_params,
                                                                 lr1110_radio_pkt_params_lora_t* radio_pkt_params )
{
    radio_pkt_params->preamble_len_in_symb = ral_pkt_params->preamble_len_in_symb;

    switch( ral_pkt_params->header_type )
    {
    case( RAL_LORA_PKT_EXPLICIT ):
    {
        radio_pkt_params->header_type = LR1110_RADIO_LORA_PKT_EXPLICIT;
        break;
    }
    case( RAL_LORA_PKT_IMPLICIT ):
    {
        radio_pkt_params->header_type = LR1110_RADIO_LORA_PKT_IMPLICIT;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_pkt_params->pld_len_in_bytes = ral_pkt_params->pld_len_in_bytes;
    radio_pkt_params->crc =
        ( ral_pkt_params->crc_is_on == false ) ? LR1110_RADIO_LORA_CRC_OFF : LR1110_RADIO_LORA_CRC_ON;
    radio_pkt_params->iq =
        ( ral_pkt_params->invert_iq_is_on == false ) ? LR1110_RADIO_LORA_IQ_STANDARD : LR1110_RADIO_LORA_IQ_INVERTED;

    return RAL_STATUS_OK;
}

static ral_status_t ral_lr1110_convert_lora_cad_params_from_ral( const ral_lora_cad_params_t* ral_lora_cad_params,
                                                                 lr1110_radio_cad_params_t*   radio_lora_cad_params )
{
    switch( ral_lora_cad_params->cad_symb_nb )
    {
    case RAL_LORA_CAD_01_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = 1;
        break;
    }
    case RAL_LORA_CAD_02_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = 2;
        break;
    }
    case RAL_LORA_CAD_04_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = 4;
        break;
    }
    case RAL_LORA_CAD_08_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = 8;
        break;
    }
    case RAL_LORA_CAD_16_SYMB:
    {
        radio_lora_cad_params->cad_symb_nb = 16;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_lora_cad_params->cad_detect_peak = ral_lora_cad_params->cad_det_peak_in_symb;
    radio_lora_cad_params->cad_detect_min  = ral_lora_cad_params->cad_det_min_in_symb;

    switch( ral_lora_cad_params->cad_exit_mode )
    {
    case RAL_LORA_CAD_ONLY:
    {
        radio_lora_cad_params->cad_exit_mode = LR1110_RADIO_CAD_EXIT_MODE_STANDBYRC;
        break;
    }
    case RAL_LORA_CAD_RX:
    {
        radio_lora_cad_params->cad_exit_mode = LR1110_RADIO_CAD_EXIT_MODE_RX;
        break;
    }
    case RAL_LORA_CAD_LBT:
    {
        radio_lora_cad_params->cad_exit_mode = LR1110_RADIO_CAD_EXIT_MODE_TX;
        break;
    }
    default:
    {
        return RAL_STATUS_UNKNOWN_VALUE;
    }
    }

    radio_lora_cad_params->cad_timeout = ral_lora_cad_params->cad_timeout_in_ms;

    return RAL_STATUS_OK;
}

/* --- EOF ------------------------------------------------------------------ */
