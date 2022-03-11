/*!
 * \file      lr1_mac_core.c
 *
 * \brief     LoRaWan core definition
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
 *-----------------------------------------------------------------------------------
 * --- DEPENDENCIES -----------------------------------------------------------------
 */

#include "lr1mac_core.h"

#include "smtc_modem_hal_dbg_trace.h"
#include "lr1mac_utilities.h"
#include "smtc_modem_hal.h"
#include "smtc_real.h"
#include "smtc_real_defs.h"
#include "smtc_real_defs_str.h"
#include "smtc_duty_cycle.h"
#include "smtc_lbt.h"
#include "lr1mac_config.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

#define DBG_PRINT_WITH_LINE( ... )                                                     \
    do                                                                                 \
    {                                                                                  \
        SMTC_MODEM_HAL_TRACE_MSG( "\n  *************************************\n  * " ); \
        SMTC_MODEM_HAL_TRACE_PRINTF( __VA_ARGS__ );                                    \
        SMTC_MODEM_HAL_TRACE_MSG( "\n  *************************************\n" );     \
    } while( 0 );

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

#if( MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON )
static const char* smtc_name_bw[] = { "BW007", "BW010", "BW015", "BW020", "BW031", "BW041", "BW062",
                                      "BW125", "BW200", "BW250", "BW400", "BW500", "BW800", "BW1600" };
#endif

/*
 *-----------------------------------------------------------------------------------
 *--- PRIVATE VARIABLES -------------------------------------------------------------
 */
/*
 *-----------------------------------------------------------------------------------
 *--- PRIVATE FUNCTIONS DECLARATION -------------------------------------------------
 */
static uint32_t    failsafe_timstamp_get( lr1_stack_mac_t* lr1_mac_obj );
static rp_status_t rp_status_get( lr1_stack_mac_t* lr1_mac_obj );
static void        copy_user_payload( lr1_stack_mac_t* lr1_mac_obj, const uint8_t* data_in, const uint8_t size_in );
static void        lr1mac_mac_update( lr1_stack_mac_t* lr1_mac_obj, user_rx_packet_type_t* available_rx_packet );
static void        save_devnonce_rst( const lr1_stack_mac_t* lr1_mac_obj );
static void        load_devnonce_reset( lr1_stack_mac_t* lr1_mac_obj );
static void        try_recover_nvm( lr1_stack_mac_t* lr1_mac_obj );
/*
 *-----------------------------------------------------------------------------------
 *--- PUBLIC FUNCTIONS DEFINITIONS --------------------------------------------------
 */

void lr1mac_core_init( lr1_stack_mac_t* lr1_mac_obj, smtc_lbt_t* lbt_obj, smtc_dtc_t* dtc_obj, radio_planner_t* rp,
                       lr1mac_activation_mode_t activation_mode, smtc_real_region_types_t smtc_real_region_types,
                       void ( *push_callback )( void* push_context ), void*               push_context )
{
    memset( lr1_mac_obj, 0, sizeof( lr1_stack_mac_t ) );

    lr1_mac_obj->lr1mac_state                = LWPSTATE_IDLE;
    lr1_mac_obj->valid_rx_packet             = NO_MORE_VALID_RX_PACKET;
    lr1_mac_obj->receive_window_type         = RECEIVE_NONE;
    lr1_mac_obj->stack_id4rp                 = RP_HOOK_ID_LR1MAC_STACK;
    lr1_mac_obj->lbt_obj                     = lbt_obj;
    lr1_mac_obj->dtc_obj                     = dtc_obj;
    lr1_mac_obj->send_at_time                = false;
    lr1_mac_obj->push_callback               = push_callback;
    lr1_mac_obj->push_context                = push_context;
    lr1_mac_obj->crystal_error               = BSP_CRYSTAL_ERROR;
    lr1_mac_obj->device_time_invalid_delay_s = LR1MAC_DEVICE_TIME_DELAY_TO_BE_NO_SYNC;
    lr1_stack_mac_init( lr1_mac_obj, activation_mode, smtc_real_region_types );

    status_lorawan_t status = lr1mac_core_context_load( lr1_mac_obj );

    if( status != OKLORAWAN )
    {
        // Possible update of nvm organisation --> try to recover
        try_recover_nvm( lr1_mac_obj );
        status = lr1mac_core_context_load( lr1_mac_obj );
    }

    if( status == OKLORAWAN )
    {
        // Check if the region stored in flash is still valid
        status = lr1mac_core_is_supported_region( lr1_mac_obj, lr1_mac_obj->real.region_type );
    }

    if( status == ERRORLORAWAN )
    {
        SMTC_MODEM_HAL_TRACE_WARNING( "No valid lr1mac context --> Use default value\n" );

        lr1_mac_obj->adr_custom       = BSP_USER_DR_DISTRIBUTION_PARAMETERS;  // (dr0 only)
        lr1_mac_obj->real.region_type = ( smtc_real_region_types_t ) smtc_real_region_list[0];
        lr1mac_core_context_save( lr1_mac_obj );
    }

    load_devnonce_reset( lr1_mac_obj );
	
    	//20211216 by ysh devnoce can use random value
  //      lr1_mac_obj->dev_nonce   = smtc_modem_hal_get_random_nb();//ctx.devnonce;
	
    lr1_mac_obj->nb_of_reset += 1;  // increment reset counter when lr1mac_core_init is called, reset is saved when
                                    // devnonce is save (after a tx join)
    SMTC_MODEM_HAL_TRACE_PRINTF( " DevNonce = %d\n", lr1_mac_obj->dev_nonce );
    SMTC_MODEM_HAL_TRACE_PRINTF( " JoinNonce = 0x%02x %02x %02x, NetID = 0x%02x %02x %02x\n",
                                 lr1_mac_obj->join_nonce[0], lr1_mac_obj->join_nonce[1], lr1_mac_obj->join_nonce[2],
                                 lr1_mac_obj->join_nonce[3], lr1_mac_obj->join_nonce[4], lr1_mac_obj->join_nonce[5] );
    SMTC_MODEM_HAL_TRACE_PRINTF( " NbOfReset = %d\n", lr1_mac_obj->nb_of_reset );
    SMTC_MODEM_HAL_TRACE_PRINTF( " Region = %s\n", smtc_real_region_list_str[lr1_mac_obj->real.region_type] );

    lr1_mac_obj->rp                 = rp;
    lr1_mac_obj->no_rx_packet_count = NO_RX_PACKET_CNT;
    rp_hook_init( lr1_mac_obj->rp, lr1_mac_obj->stack_id4rp, ( void ( * )( void* ) )( lr1_stack_mac_rp_callback ),
                  ( lr1_mac_obj ) );
    SMTC_MODEM_HAL_TRACE_PRINTF( "rp_hook_init done\n" );

    smtc_real_config( lr1_mac_obj );
    SMTC_MODEM_HAL_TRACE_PRINTF( "smtc_real_config done\n" );
    smtc_real_init( lr1_mac_obj );
    SMTC_MODEM_HAL_TRACE_PRINTF( "smtc_real_init done\n" );
}

/***********************************************************************************************/
/*    LoraWanProcess Method                                                                    */
/***********************************************************************************************/

lr1mac_states_t lr1mac_core_process( lr1_stack_mac_t* lr1_mac_obj, user_rx_packet_type_t* available_rx_packet )
{
    uint8_t myhook_id;
    rp_hook_get_id( lr1_mac_obj->rp, ( void* ) ( ( lr1_mac_obj ) ), &myhook_id );
    *available_rx_packet = NO_LORA_RXPACKET_AVAILABLE;

#if !defined( TEST_BYPASS_JOIN_DUTY_CYCLE )
    if( lr1mac_core_certification_get( lr1_mac_obj ) == false )
    {
        if( ( lr1_mac_joined_status_get( lr1_mac_obj ) == NOT_JOINED ) &&
            ( ( int32_t )( lr1_mac_obj->next_time_to_join_seconds - smtc_modem_hal_get_time_in_s( ) ) > 0 ) )
        {
            SMTC_MODEM_HAL_TRACE_PRINTF( "TOO SOON TO JOIN time is  %d time target is : %d\n",
                                         smtc_modem_hal_get_time_in_s( ), lr1_mac_obj->next_time_to_join_seconds );
            lr1_mac_obj->lr1mac_state = LWPSTATE_IDLE;
        }
    }
#endif

    if( ( lr1_mac_obj->lr1mac_state != LWPSTATE_IDLE ) &&
        ( ( smtc_modem_hal_get_time_in_s( ) - failsafe_timstamp_get( lr1_mac_obj ) ) > 300 ) )
    {
        smtc_modem_hal_lr1mac_panic( "FAILSAFE EVENT OCCUR (lr1mac_state:0x%x)\n", lr1_mac_obj->lr1mac_state );
        lr1_mac_obj->lr1mac_state = LWPSTATE_ERROR;
    }
    if( lr1_stack_mac_radio_state_get( lr1_mac_obj ) == RADIOSTATE_ABORTED_BY_RP )
    {
        lr1mac_mac_update( lr1_mac_obj, available_rx_packet );
    }

    switch( lr1_mac_obj->lr1mac_state )
    {
    //**********************************************************************************
    //                                    STATE IDLE
    //**********************************************************************************
    case LWPSTATE_IDLE:
        break;

    //**********************************************************************************
    //                                    STATE TX
    //**********************************************************************************
    case LWPSTATE_SEND:
        switch( lr1_stack_mac_radio_state_get( lr1_mac_obj ) )
        {
        case RADIOSTATE_IDLE:
            lr1_mac_obj->radio_process_state = RADIOSTATE_PENDING;
            DBG_PRINT_WITH_LINE( "Send Payload  HOOK ID = %d", myhook_id );

            modulation_type_t tx_modulation_type =
                smtc_real_get_modulation_type_from_datarate( lr1_mac_obj, lr1_mac_obj->tx_data_rate );

            if( tx_modulation_type == LORA )
            {
                uint8_t            tx_sf;
                lr1mac_bandwidth_t tx_bw;
                smtc_real_lora_dr_to_sf_bw( lr1_mac_obj, lr1_mac_obj->tx_data_rate, &tx_sf, &tx_bw );

                SMTC_MODEM_HAL_TRACE_PRINTF(
                    "  Tx  LoRa at %u ms: freq:%d, SF%u, %s, len %u bytes %d dBm, fcnt_up %d, toa = %d\n",
                    lr1_mac_obj->rtc_target_timer_ms, lr1_mac_obj->tx_frequency, tx_sf, smtc_name_bw[tx_bw],
                    lr1_mac_obj->tx_payload_size, lr1_mac_obj->tx_power, lr1_mac_obj->fcnt_up,
                    lr1_stack_toa_get( lr1_mac_obj ) );
            }
            else if( tx_modulation_type == FSK )
            {
                SMTC_MODEM_HAL_TRACE_PRINTF(
                    "  Tx  FSK  at %u ms: freq:%d, len %u bytes %d dBm, fcnt_up %d, toa = %d\n",
                    lr1_mac_obj->rtc_target_timer_ms, lr1_mac_obj->tx_frequency, lr1_mac_obj->tx_payload_size,
                    lr1_mac_obj->tx_power, lr1_mac_obj->fcnt_up, lr1_stack_toa_get( lr1_mac_obj ) );
            }

            if( smtc_lbt_is_enable( lr1_mac_obj->lbt_obj ) == true )
            {
                smtc_lbt_listen_channel( ( lr1_mac_obj->lbt_obj ), lr1_mac_obj->tx_frequency, lr1_mac_obj->send_at_time,
                                         lr1_mac_obj->rtc_target_timer_ms - RP_MARGIN_DELAY,
                                         lr1_stack_toa_get( lr1_mac_obj ) );
            }
            else
            {
                lr1_stack_mac_tx_radio_start( lr1_mac_obj );
            }
            break;

        case RADIOSTATE_TX_FINISHED:
            lr1_mac_obj->lr1mac_state               = LWPSTATE_RX1;
            lr1_mac_obj->tx_duty_cycle_timestamp_ms = lr1_mac_obj->isr_tx_done_radio_timestamp;
            lr1_mac_obj->tx_duty_cycle_time_off_ms =
                ( lr1_mac_obj->rp->stats.tx_last_toa_ms[myhook_id] << lr1_mac_obj->max_duty_cycle_index ) -
                lr1_mac_obj->rp->stats.tx_last_toa_ms[myhook_id];

            if( lr1_mac_obj->join_status == NOT_JOINED )
            {
                // save devnonce after the end of TX
                save_devnonce_rst( lr1_mac_obj );
            }
            lr1_stack_mac_rx_timer_configure( lr1_mac_obj, RX1 );
            lr1_stack_mac_update_tx_done( lr1_mac_obj );
            smtc_duty_cycle_sum( lr1_mac_obj->dtc_obj, lr1_mac_obj->tx_frequency,
                                 lr1_mac_obj->rp->stats.tx_last_toa_ms[myhook_id] );
            break;

        default:
            // Do nothing
            break;
        }
        break;

    //**********************************************************************************
    //                                   STATE RX1
    //**********************************************************************************
    case LWPSTATE_RX1:
        if( lr1_stack_mac_radio_state_get( lr1_mac_obj ) == RADIOSTATE_RX_FINISHED )
        {
            if( rp_status_get( lr1_mac_obj ) == RP_STATUS_RX_PACKET )
            {
                lr1_mac_obj->receive_window_type = RECEIVE_ON_RX1;
                lr1_mac_obj->valid_rx_packet     = lr1_stack_mac_rx_frame_decode( lr1_mac_obj );
                if( lr1_mac_obj->valid_rx_packet == NO_MORE_VALID_RX_PACKET )
                {
                    lr1_mac_obj->lr1mac_state = LWPSTATE_RX2;
                    DBG_PRINT_WITH_LINE( "Receive a bad packet on RX1 for Hook Id = %d continue with RX2 ", myhook_id );
                    lr1_stack_mac_rx_timer_configure( lr1_mac_obj, RX2 );
                }
                else
                {
                    DBG_PRINT_WITH_LINE( "Receive a Valid downlink RX1 for Hook Id = %d", myhook_id );
                    lr1mac_mac_update( lr1_mac_obj, available_rx_packet );
                }
            }
            else
            {
                lr1_mac_obj->lr1mac_state = LWPSTATE_RX2;
                DBG_PRINT_WITH_LINE( "RX1 Timeout for Hook Id = %d", myhook_id );
                lr1_stack_mac_rx_timer_configure( lr1_mac_obj, RX2 );
            }
        }
        break;

    //**********************************************************************************
    //                                   STATE RX2
    //**********************************************************************************
    case LWPSTATE_RX2:
        if( lr1_stack_mac_radio_state_get( lr1_mac_obj ) == RADIOSTATE_RX_FINISHED )
        {
            if( rp_status_get( lr1_mac_obj ) == RP_STATUS_RX_PACKET )
            {
                lr1_mac_obj->receive_window_type = RECEIVE_ON_RX2;
                lr1_mac_obj->valid_rx_packet     = lr1_stack_mac_rx_frame_decode( lr1_mac_obj );
                if( lr1_mac_obj->valid_rx_packet == NO_MORE_VALID_RX_PACKET )
                {
                    DBG_PRINT_WITH_LINE( "Receive a bad packet on RX2 for Hook Id = %d", myhook_id );
                }
                else
                {
                    DBG_PRINT_WITH_LINE( "Receive a Valid downlink RX2 for Hook Id = %d", myhook_id );
                }
            }
            else
            {
                DBG_PRINT_WITH_LINE( "RX2 Timeout for Hook Id = %d", myhook_id );
            }
            lr1mac_mac_update( lr1_mac_obj, available_rx_packet );
        }
        break;

    //**********************************************************************************
    //                              STATE TXwait MAC
    //**********************************************************************************
    case LWPSTATE_TX_WAIT:
        SMTC_MODEM_HAL_TRACE_MSG( " ." );
        if( smtc_modem_hal_get_time_in_ms( ) > lr1_mac_obj->rtc_target_timer_ms )
        {
            lr1_mac_obj->lr1mac_state = LWPSTATE_SEND;  //@note the frame have already been prepare in Update Mac Layer
        }
        break;

    default:
        smtc_modem_hal_lr1mac_panic( "Illegal state in lorawan process\n" );
        break;
    }

    return ( lr1_mac_obj->lr1mac_state );
}

/***********************************************************************************************/
/*    End Of LoraWanProcess Method                                                             */
/***********************************************************************************************/

/**************************************************/
/*            LoraWan  Join  Method               */
/**************************************************/
lr1mac_states_t lr1mac_core_join( lr1_stack_mac_t* lr1_mac_obj, uint32_t target_time_ms )
{
    if( lr1_mac_obj->lr1mac_state != LWPSTATE_IDLE )
    {
        SMTC_MODEM_HAL_TRACE_ERROR( "LP STATE NOT EQUAL TO IDLE\n" );
        return ( LWPSTATE_ERROR );
    }
    if( lr1mac_core_get_activation_mode( lr1_mac_obj ) == ACTIVATION_MODE_ABP )
    {
        SMTC_MODEM_HAL_TRACE_ERROR( "ABP DEVICE CAN'T PROCCED A JOIN REQUEST\n" );
        return ( LWPSTATE_ERROR );
    }
    uint32_t current_timestamp       = smtc_modem_hal_get_time_in_s( );
    lr1_mac_obj->timestamp_failsafe  = current_timestamp;
    lr1_mac_obj->rtc_target_timer_ms = target_time_ms;
    lr1_mac_obj->join_status         = NOT_JOINED;

    smtc_real_init( lr1_mac_obj );

    lr1_mac_obj->rx2_data_rate = smtc_real_get_rx2_join_dr( lr1_mac_obj );

    // check adr mode to see if it is already set in join DR and it is the first join try or if adr was set to other DR
    if( ( lr1_mac_obj->adr_mode_select != JOIN_DR_DISTRIBUTION ) ||
        ( ( lr1_mac_obj->retry_join_cpt == 0 ) && ( lr1_mac_obj->adr_mode_select == JOIN_DR_DISTRIBUTION ) ) )
    {
        // if it is the case force dr distribution to join
        lr1_mac_obj->adr_mode_select_tmp = lr1_mac_obj->adr_mode_select;
        smtc_real_set_dr_distribution( lr1_mac_obj, JOIN_DR_DISTRIBUTION );
        lr1_mac_obj->adr_mode_select = JOIN_DR_DISTRIBUTION;
    }
    smtc_real_get_next_dr( lr1_mac_obj );
    smtc_duty_cycle_update( lr1_mac_obj->dtc_obj );
    if( smtc_real_get_join_next_channel( lr1_mac_obj ) != OKLORAWAN )
    {
        return ( LWPSTATE_ERROR );
    }

    lr1_stack_mac_join_request_build( lr1_mac_obj );
    lr1_stack_rx1_join_delay_set( lr1_mac_obj );
    lr1_stack_rx2_join_dr_set( lr1_mac_obj );

    // check if it first join try
    if( lr1_mac_obj->retry_join_cpt == 0 )
    {
        // take the timestamp reference for join duty cycle management
        lr1_mac_obj->first_join_timestamp = current_timestamp;
    }

    lr1_mac_obj->lr1mac_state = LWPSTATE_SEND;
    return ( lr1_mac_obj->lr1mac_state );
}

/**************************************************/
/*          LoraWan  IsJoined  Method             */
/**************************************************/
join_status_t lr1_mac_joined_status_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->join_status;
}

/**************************************************/
/*          LoraWan  ClearJoinStatus  Method      */
/**************************************************/
void lr1mac_core_join_status_clear( lr1_stack_mac_t* lr1_mac_obj )
{
    lr1_mac_obj->join_status = NOT_JOINED;
    lr1mac_core_abort( lr1_mac_obj );
    smtc_real_init_join_snapshot_channel_mask( lr1_mac_obj );
}

/**************************************************/
/*          LoraWan  IsJoined  Method             */
/**************************************************/
void lr1mac_core_new_join( lr1_stack_mac_t* lr1_mac_obj )
{
    lr1_mac_obj->join_status = NOT_JOINED;
}

/**************************************************/
/*         LoraWan  SendPayload  Method           */
/**************************************************/
lr1mac_states_t lr1mac_core_send_stack_cid_req( lr1_stack_mac_t* lr1_mac_obj, cid_from_device_t cid_req )
{
    uint8_t  data_in[242];
    uint8_t  size_in = 0;
    uint8_t  fport   = 0;
    uint32_t target_time_ms =
        smtc_modem_hal_get_time_in_ms( ) + ( smtc_modem_hal_get_random_nb_in_range( 1, 3 ) * 1000 );

    // If sticky are present add it to the payload only once
    if( ( lr1_mac_obj->link_check_user_req != USER_MAC_REQ_REQUESTED ) &&
        ( lr1_mac_obj->device_time_user_req != USER_MAC_REQ_REQUESTED ) &&
        ( lr1_mac_obj->ping_slot_info_user_req != USER_MAC_REQ_REQUESTED ) )

        if( lr1_mac_obj->tx_fopts_current_length > 0 )
        {
            size_in += lr1_mac_obj->tx_fopts_current_length;
            memcpy1( data_in, lr1_mac_obj->tx_fopts_datasticky, lr1_mac_obj->tx_fopts_current_length );
        }

    switch( cid_req )
    {
    case LINK_CHECK_REQ:
        if( lr1_mac_obj->link_check_user_req != USER_MAC_REQ_REQUESTED )
        {
            lr1_mac_obj->link_check_user_req = USER_MAC_REQ_REQUESTED;
            lr1_mac_obj->link_check_margin   = 0;
            lr1_mac_obj->link_check_gw_cnt   = 0;
            data_in[size_in++]               = cid_req;
        }
        break;

    case DEVICE_TIME_REQ:
        if( lr1_mac_obj->device_time_user_req != USER_MAC_REQ_REQUESTED )
        {
            lr1_mac_obj->device_time_user_req = USER_MAC_REQ_REQUESTED;
            data_in[size_in++]                = cid_req;
        }
        break;

    case PING_SLOT_INFO_REQ:
        if( lr1_mac_obj->ping_slot_info_user_req != USER_MAC_REQ_REQUESTED )
        {
            lr1_mac_obj->ping_slot_info_user_req = USER_MAC_REQ_REQUESTED;
            data_in[size_in++]                   = cid_req;
            data_in[size_in++]                   = lr1_mac_obj->ping_slot_periodicity & 0x7;
        }
        break;

    default:
        return LWPSTATE_INVALID;
        break;
    }

    return lr1mac_core_payload_send( lr1_mac_obj, fport, true, data_in, size_in, UNCONF_DATA_UP, target_time_ms );
}

lr1mac_states_t lr1mac_core_payload_send_at_time( lr1_stack_mac_t* lr1_mac_obj, uint8_t fport, bool fport_enabled,
                                                  const uint8_t* data_in, const uint8_t size_in, uint8_t packet_type,
                                                  uint32_t target_time_ms )
{
    lr1mac_states_t status =
        lr1mac_core_payload_send( lr1_mac_obj, fport, fport_enabled, data_in, size_in, packet_type, target_time_ms );

    if( status == LWPSTATE_SEND )
    {
        lr1_mac_obj->send_at_time = true;
        lr1_mac_obj->nb_trans_cpt = 1;  // Overwrite nb_trans_cpt, when downlink is At Time, repetitions are out dated
    }
    return status;
}

lr1mac_states_t lr1mac_core_payload_send( lr1_stack_mac_t* lr1_mac_obj, uint8_t fport, bool fport_enabled,
                                          const uint8_t* data_in, const uint8_t size_in, uint8_t packet_type,
                                          uint32_t target_time_ms )
{
    status_lorawan_t status = smtc_real_is_payload_size_valid( lr1_mac_obj, lr1_mac_obj->tx_data_rate, size_in,
                                                               lr1_mac_obj->uplink_dwell_time );
    if( status == ERRORLORAWAN )
    {
        SMTC_MODEM_HAL_TRACE_ERROR( "PAYLOAD SIZE TOO HIGH\n" );
        return ( LWPSTATE_INVALID );
    }
    if( lr1mac_core_get_activation_mode( lr1_mac_obj ) == ACTIVATION_MODE_OTAA )
    {
        if( lr1_mac_obj->join_status == NOT_JOINED )
        {
            SMTC_MODEM_HAL_TRACE_ERROR( "OTAA DEVICE NOT JOINED YET\n" );
            return ( LWPSTATE_INVALID );
        }
    }
    if( lr1_mac_obj->lr1mac_state != LWPSTATE_IDLE )
    {
        SMTC_MODEM_HAL_TRACE_ERROR( "LP STATE NOT EQUAL TO IDLE\n" );
        return ( LWPSTATE_ERROR );
    }
    // Decrement duty cycle before check the available DTC
    smtc_duty_cycle_update( lr1_mac_obj->dtc_obj );
    if( smtc_real_get_next_channel( lr1_mac_obj ) != OKLORAWAN )
    {
        return ( LWPSTATE_DUTY_CYCLE_FULL );
    }

    if( lr1mac_core_next_free_duty_cycle_ms_get( lr1_mac_obj ) > 0 )
    {
        SMTC_MODEM_HAL_TRACE_WARNING( "Duty Cycle is full\n" );
        return ( LWPSTATE_DUTY_CYCLE_FULL );
    }

    lr1_mac_obj->timestamp_failsafe  = smtc_modem_hal_get_time_in_s( );
    lr1_mac_obj->rtc_target_timer_ms = target_time_ms;
    lr1_mac_obj->app_payload_size    = size_in;
    lr1_mac_obj->tx_fport            = fport;
    lr1_mac_obj->tx_fport_present    = fport_enabled;
    lr1_mac_obj->tx_mtype            = packet_type;
    // Because Network payload are sent unconfirmed, we have to keep the Rx ACK bit for the App layer
    lr1_mac_obj->rx_ack_bit = 0;

    // copy payload after the update of the lr1mac object
    copy_user_payload( lr1_mac_obj, data_in, size_in );

    lr1_stack_mac_tx_frame_build( lr1_mac_obj );
    lr1_stack_mac_tx_frame_encrypt( lr1_mac_obj );
    if( packet_type == CONF_DATA_UP )
    {
        lr1_mac_obj->receive_window_type = RECEIVE_NONE;
    }
    else
    {
        lr1_mac_obj->nb_trans_cpt = lr1_mac_obj->nb_trans;
        // deprecated lr1_mac_obj->receive_window_type = RECEIVE_NACK;
    }
    lr1_mac_obj->nb_trans_cpt = lr1_mac_obj->nb_trans;
    lr1_mac_obj->lr1mac_state = LWPSTATE_SEND;

    return ( lr1_mac_obj->lr1mac_state );
}

/**************************************************/
/*        LoraWan  Receive  Method                */
/**************************************************/
status_lorawan_t lr1mac_core_payload_receive( lr1_stack_mac_t* lr1_mac_obj, uint8_t* user_rx_port,
                                              uint8_t* user_rx_payload, uint8_t* user_rx_payloadSize )
{
    if( lr1_mac_obj->available_app_packet == NO_LORA_RXPACKET_AVAILABLE )
    {
        return ERRORLORAWAN;
    }

    *user_rx_payloadSize = lr1_mac_obj->rx_payload_size;
    *user_rx_port        = lr1_mac_obj->rx_metadata.rx_fport;
    memcpy1( user_rx_payload, lr1_mac_obj->rx_payload, lr1_mac_obj->rx_payload_size );
    lr1_mac_obj->available_app_packet = NO_LORA_RXPACKET_AVAILABLE;

    return OKLORAWAN;
}

/**************************************************/
/*       LoraWan  AdrModeSelect  Method           */
/**************************************************/

status_lorawan_t lr1mac_core_dr_strategy_set( lr1_stack_mac_t* lr1_mac_obj, dr_strategy_t adr_mode_select )
{
    status_lorawan_t status;
    dr_strategy_t    adr_mode_select_cpy = lr1_mac_obj->adr_mode_select;
    lr1_mac_obj->adr_mode_select         = adr_mode_select;

    if( adr_mode_select == STATIC_ADR_MODE )
    {
        lr1_mac_obj->tx_data_rate_adr = smtc_real_get_min_tx_channel_dr( lr1_mac_obj );
    }
    else
    {
        lr1_mac_obj->tx_power = smtc_real_get_default_max_eirp( lr1_mac_obj );
    }

    smtc_real_set_dr_distribution( lr1_mac_obj, adr_mode_select );
    status = smtc_real_get_next_dr( lr1_mac_obj );

    if( status == ERRORLORAWAN )  // new adr profile no compatible with channel mask, retreive old adr profile
    {
        lr1_mac_obj->adr_mode_select = adr_mode_select_cpy;
        smtc_real_set_dr_distribution( lr1_mac_obj, adr_mode_select_cpy );
        smtc_real_get_next_dr( lr1_mac_obj );
    }
    return ( status );
}
/**************************************************/
/*       LoraWan  AdrModeSelect  Get Method       */
/**************************************************/
dr_strategy_t lr1mac_core_dr_strategy_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return ( lr1_mac_obj->adr_mode_select );
}

/*************************************************************/
/*       LoraWan  Set DataRate Custom for custom adr profile */
/*************************************************************/
void lr1mac_core_dr_custom_set( lr1_stack_mac_t* lr1_mac_obj, uint32_t DataRateCustom )
{
    lr1_mac_obj->adr_custom = DataRateCustom;
    lr1mac_core_context_save( lr1_mac_obj );
}

/**************************************************/
/*         LoraWan  GetDevAddr  Method            */
/**************************************************/
uint32_t lr1mac_core_devaddr_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return ( lr1_mac_obj->dev_addr );
}

/**************************************************/
/*         LoraWan  GetNextPower  Method          */
/**************************************************/
uint8_t lr1mac_core_next_power_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return ( lr1_mac_obj->tx_power );
}

/**************************************************/
/*    LoraWan  GetLorawanProcessState  Method     */
/**************************************************/
lr1mac_states_t lr1mac_core_state_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return ( lr1_mac_obj->lr1mac_state );
}

/**************************************************/
/*    LoraWan  StoreContext  Method             */
/**************************************************/
void lr1mac_core_context_save( lr1_stack_mac_t* lr1_mac_obj )
{
    // don't save in nvm if only nb of reset has been modified
    if( ( lr1_mac_obj->mac_context.adr_custom != lr1_mac_obj->adr_custom ) ||
        ( lr1_mac_obj->mac_context.region_type != lr1_mac_obj->real.region_type ) ||
        ( lr1_mac_obj->mac_context.certification_enabled != lr1_mac_obj->is_lorawan_modem_certification_enabled ) )
    {
        lr1_mac_obj->mac_context.adr_custom            = lr1_mac_obj->adr_custom;
        lr1_mac_obj->mac_context.region_type           = lr1_mac_obj->real.region_type;
        lr1_mac_obj->mac_context.certification_enabled = lr1_mac_obj->is_lorawan_modem_certification_enabled;
        lr1_mac_obj->mac_context.crc =
            lr1mac_utilities_crc( ( uint8_t* ) &( lr1_mac_obj->mac_context ), sizeof( lr1_mac_obj->mac_context ) - 4 );

        smtc_modem_hal_context_store( CONTEXT_LR1MAC, ( uint8_t* ) &( lr1_mac_obj->mac_context ),
                                      sizeof( lr1_mac_obj->mac_context ) );

        // dummy context reading to ensure context store is done before exiting the function
        mac_context_t dummy_context = { 0 };
        smtc_modem_hal_context_restore( CONTEXT_LR1MAC, ( uint8_t* ) &dummy_context,
                                        sizeof( lr1_mac_obj->mac_context ) );
    }
}
/**************************************************/
/*   LoraWan  lr1mac_core_next_max_payload_length_get  Method     */
/**************************************************/
uint32_t lr1mac_core_next_max_payload_length_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return ( smtc_real_get_max_payload_size( lr1_mac_obj, lr1_mac_obj->tx_data_rate, lr1_mac_obj->uplink_dwell_time ) -
             lr1_mac_obj->tx_fopts_current_length - 8 );
}

/**************************************************/
/*        LoraWan  lr1mac_core_next_dr_get  Method        */
/**************************************************/
uint8_t lr1mac_core_next_dr_get( lr1_stack_mac_t* lr1_mac_obj )
{  // note return datareate in case of adr
    return ( lr1_mac_obj->tx_data_rate );
}

uint32_t lr1mac_core_next_frequency_get( lr1_stack_mac_t* lr1_mac_obj )
{  // note return datareate in case of adr
    return ( lr1_mac_obj->tx_frequency );
}

void lr1mac_core_factory_reset( lr1_stack_mac_t* lr1_mac_obj )
{
    lr1_mac_obj->mac_context.adr_custom            = lr1_mac_obj->adr_custom;
    lr1_mac_obj->mac_context.region_type           = lr1_mac_obj->real.region_type;
    lr1_mac_obj->mac_context.certification_enabled = lr1_mac_obj->is_lorawan_modem_certification_enabled;
    lr1_mac_obj->mac_context.crc =
        lr1mac_utilities_crc( ( uint8_t* ) &( lr1_mac_obj->mac_context ), sizeof( lr1_mac_obj->mac_context ) - 4 ) + 1;

    smtc_modem_hal_context_store( CONTEXT_LR1MAC, ( uint8_t* ) &( lr1_mac_obj->mac_context ),
                                  sizeof( lr1_mac_obj->mac_context ) );

    // dummy context reading to ensure context store is done before exiting the function
    mac_context_t dummy_context = { 0 };
    smtc_modem_hal_context_restore( CONTEXT_LR1MAC, ( uint8_t* ) &dummy_context, sizeof( lr1_mac_obj->mac_context ) );
}

lr1mac_activation_mode_t lr1mac_core_get_activation_mode( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->activation_mode;
}

void lr1mac_core_set_activation_mode( lr1_stack_mac_t* lr1_mac_obj, lr1mac_activation_mode_t activation_mode )
{
    lr1_mac_obj->activation_mode = activation_mode;
}

radio_planner_t* lr1mac_core_rp_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->rp;
}

uint16_t lr1mac_core_nb_reset_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->nb_of_reset;
}

uint16_t lr1mac_core_devnonce_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->dev_nonce;
}

int16_t lr1mac_core_last_snr_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->rx_metadata.rx_snr;
}

int16_t lr1mac_core_last_rssi_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->rx_metadata.rx_rssi;
}

uint8_t lr1mac_core_min_tx_dr_get( lr1_stack_mac_t* lr1_mac_obj )
{
    uint8_t tmp = lr1_stack_mac_min_tx_dr_get( lr1_mac_obj );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Min DataRate = %d\n", tmp );
    return ( tmp );
}

uint8_t lr1mac_core_max_tx_dr_get( lr1_stack_mac_t* lr1_mac_obj )
{
    uint8_t tmp = lr1_stack_mac_max_tx_dr_get( lr1_mac_obj );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Max DataRate = %d\n", tmp );
    return ( tmp );
}

uint16_t lr1mac_core_mask_tx_dr_channel_up_dwell_time_check( lr1_stack_mac_t* lr1_mac_obj )
{
    uint8_t tmp = lr1_stack_mac_mask_tx_dr_channel_up_dwell_time_check( lr1_mac_obj );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Mask DataRate = 0x%x\n", tmp );
    return ( tmp );
}

status_lorawan_t lr1mac_core_context_load( lr1_stack_mac_t* lr1_mac_obj )
{
    smtc_modem_hal_context_restore( CONTEXT_LR1MAC, ( uint8_t* ) &( lr1_mac_obj->mac_context ),
                                    sizeof( lr1_mac_obj->mac_context ) );

    if( lr1mac_utilities_crc( ( uint8_t* ) &( lr1_mac_obj->mac_context ), sizeof( lr1_mac_obj->mac_context ) - 4 ) ==
        lr1_mac_obj->mac_context.crc )
    {
        lr1_mac_obj->adr_custom       = lr1_mac_obj->mac_context.adr_custom;
        lr1_mac_obj->real.region_type = ( smtc_real_region_types_t ) lr1_mac_obj->mac_context.region_type;
        lr1_mac_obj->is_lorawan_modem_certification_enabled = lr1_mac_obj->mac_context.certification_enabled;

        return OKLORAWAN;
    }
    else
    {  // == factory reset
        return ERRORLORAWAN;
    }
}
receive_win_t lr1mac_core_rx_window_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return ( lr1_mac_obj->receive_window_type );
}

uint32_t lr1mac_core_fcnt_up_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return ( lr1_mac_obj->fcnt_up );
}

uint32_t lr1mac_core_next_join_time_second_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return ( lr1_mac_obj->next_time_to_join_seconds );
}

int32_t lr1mac_core_next_free_duty_cycle_ms_get( lr1_stack_mac_t* lr1_mac_obj )
{
    int32_t  ret            = 0;
    uint8_t  number_of_freq = 0;
    uint8_t  max_size       = 16;
    uint32_t freq_list[16]  = { 0 };  // Generally region with duty cycle support 16 channels only

    int32_t region_dtc = 0;
    int32_t nwk_dtc    = lr1_stack_network_next_free_duty_cycle_ms_get( lr1_mac_obj );

    if( smtc_real_is_dtc_supported( lr1_mac_obj ) == true )
    {
        if( smtc_real_get_current_enabled_frequency_list( lr1_mac_obj, &number_of_freq, freq_list, max_size ) == true )
        {
            region_dtc = smtc_duty_cycle_get_next_free_time_ms( lr1_mac_obj->dtc_obj, number_of_freq, freq_list );
        }
    }

    if( nwk_dtc == 0 )
    {
        ret = region_dtc;
    }
    else
    {
        ret = MAX( nwk_dtc, region_dtc );
    }
    return ret;
}

uint8_t lr1mac_core_duty_cycle_enable_set( lr1_stack_mac_t* lr1_mac_obj, smtc_dtc_enablement_type_t enable )
{
    return smtc_duty_cycle_enable_set( lr1_mac_obj->dtc_obj, enable );
}

smtc_dtc_enablement_type_t lr1mac_core_duty_cycle_enable_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return smtc_duty_cycle_enable_get( lr1_mac_obj->dtc_obj );
}

uint8_t lr1mac_core_rx_ack_bit_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return ( lr1_mac_obj->rx_ack_bit );
}

status_lorawan_t lr1mac_core_is_supported_region( lr1_stack_mac_t* lr1_mac_obj, smtc_real_region_types_t region_type )
{
    for( uint8_t i = 0; i < SMTC_REAL_REGION_LIST_LENGTH; i++ )
    {
        if( smtc_real_region_list[i] == region_type )
        {
            return OKLORAWAN;
        }
    }

    SMTC_MODEM_HAL_TRACE_ERROR( "Invalid Region 0x%02x\n", region_type );
    return ERRORLORAWAN;
}

smtc_real_region_types_t lr1mac_core_get_region( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->real.region_type;
}

status_lorawan_t lr1mac_core_set_region( lr1_stack_mac_t* lr1_mac_obj, smtc_real_region_types_t region_type )
{
    if( lr1mac_core_is_supported_region( lr1_mac_obj, region_type ) == OKLORAWAN )
    {
        lr1_mac_obj->real.region_type = region_type;
        lr1mac_core_context_save( lr1_mac_obj );
        smtc_real_config( lr1_mac_obj );
        smtc_real_init( lr1_mac_obj );
        // After a region change a new join should happen, reset join counter
        lr1_mac_obj->retry_join_cpt = 0;
        return OKLORAWAN;
    }
    return ERRORLORAWAN;
}

status_lorawan_t lr1mac_core_set_no_rx_packet_count_config( lr1_stack_mac_t* lr1_mac_obj, uint16_t no_rx_packet_count )
{
    // 0 means this protection is deactivated
    // if( no_rx_packet_count > 0 )
    //  {
    lr1_mac_obj->no_rx_packet_count = no_rx_packet_count;
    // lr1_mac_obj->adr_ack_cnt                 = 0;  // reset adr counter,
    return OKLORAWAN;
    //  }
    // return ERRORLORAWAN;
}

uint16_t lr1mac_core_get_no_rx_packet_count_config( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->no_rx_packet_count;
}

uint16_t lr1mac_core_get_no_rx_packet_count_current( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->adr_ack_cnt;
}

uint16_t lr1mac_core_get_no_rx_packet_count_in_mobile_mode( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->no_rx_packet_count_in_mobile_mode;
}

void lr1mac_core_set_no_rx_packet_count_in_mobile_mode( lr1_stack_mac_t* lr1_mac_obj, uint16_t no_rx_packet_count )
{
    lr1_mac_obj->no_rx_packet_count_in_mobile_mode = no_rx_packet_count;
}

bool lr1mac_core_available_link_adr_get( lr1_stack_mac_t* lr1_mac_obj )
{
    if( lr1_mac_obj->available_link_adr == true )
    {
        lr1_mac_obj->available_link_adr = false;  // reset the internal flag when the function is called by upper layer
        return ( true );
    }

    return false;
}

void lr1mac_core_certification_set( lr1_stack_mac_t* lr1_mac_obj, uint8_t enable )
{
    lr1_mac_obj->is_lorawan_modem_certification_enabled = enable;
    lr1mac_core_context_save( lr1_mac_obj );
}

uint8_t lr1mac_core_certification_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->is_lorawan_modem_certification_enabled;
}

lr1mac_version_t lr1mac_core_get_lorawan_version( lr1_stack_mac_t* lr1_mac_obj )
{
    lr1mac_version_t version = { .major    = LORAWAN_VERSION_MAJOR,
                                 .minor    = LORAWAN_VERSION_MINOR,
                                 .patch    = LORAWAN_VERSION_PATCH,
                                 .revision = LORAWAN_VERSION_REVISION };
    return version;
}

bool lr1mac_core_convert_rtc_to_gps_epoch_time( lr1_stack_mac_t* lr1_mac_obj, uint32_t rtc_ms,
                                                uint32_t* seconds_since_epoch, uint32_t* fractional_second )
{
    uint32_t tmp_seconds_since_epoch;
    uint32_t tmp_fractional_second;

    uint32_t delta_tx_rx_ms = rtc_ms - lr1_mac_obj->timestamp_tx_done_device_time_req_ms;

    tmp_fractional_second = lr1_mac_obj->fractional_second + delta_tx_rx_ms;

    uint32_t tmp_s        = tmp_fractional_second / 1000;  // number of seconds without ms
    tmp_fractional_second = tmp_fractional_second - ( tmp_s * 1000 );

    tmp_seconds_since_epoch = lr1_mac_obj->seconds_since_epoch + tmp_s;

    // SMTC_MODEM_HAL_TRACE_PRINTF( "tx: %u, rx:%u, diff:%u\n",
    // lr1_mac->timestamp_tx_done_device_time_req_ms,
    //                              lr1_mac->timestamp_last_device_time_ans_s, delta_tx_rx_ms );
    SMTC_MODEM_HAL_TRACE_PRINTF( "DeviceTime GPS : %u.%u s\n", tmp_seconds_since_epoch, tmp_fractional_second );

    *seconds_since_epoch = tmp_seconds_since_epoch;
    *fractional_second   = tmp_fractional_second;

    return lr1mac_core_is_time_valid( lr1_mac_obj );
}

bool lr1mac_core_is_time_valid( lr1_stack_mac_t* lr1_mac_obj )
{
    uint32_t rtc_s = smtc_modem_hal_get_time_in_s( );

    if( ( lr1_mac_obj->timestamp_last_device_time_ans_s != 0 ) &&
        ( ( rtc_s - lr1_mac_obj->timestamp_last_device_time_ans_s ) < lr1_mac_obj->device_time_invalid_delay_s ) )
    {
        return true;
    }
    return false;
}

void lr1mac_core_set_device_time_callback( lr1_stack_mac_t* lr1_mac_obj,
                                           void ( *device_time_callback )( void* context, uint32_t rx_timestamp_s ),
                                           void* context, uint32_t rx_timestamp_s )
{
    if( device_time_callback != NULL )
    {
        lr1_mac_obj->device_time_callback         = device_time_callback;
        lr1_mac_obj->device_time_callback_context = context;
    }
}

status_lorawan_t lr1_mac_core_set_device_time_invalid_delay_s( lr1_stack_mac_t* lr1_mac_obj, uint32_t delay_s )
{
    if( delay_s > LR1MAC_DEVICE_TIME_DELAY_TO_BE_NO_SYNC )
    {
        return ERRORLORAWAN;
    }
    lr1_mac_obj->device_time_invalid_delay_s = delay_s;
    return OKLORAWAN;
}

uint32_t lr1_mac_core_get_device_time_invalid_delay_s( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->device_time_invalid_delay_s;
}

status_lorawan_t lr1mac_core_set_ping_slot_periodicity( lr1_stack_mac_t* lr1_mac_obj, uint8_t ping_slot_periodicity )
{
    if( ping_slot_periodicity > 7 )
    {
        return ERRORLORAWAN;
    }

    lr1_mac_obj->ping_slot_periodicity = ping_slot_periodicity;
    return OKLORAWAN;
}

uint8_t lr1mac_core_get_ping_slot_periodicity( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->ping_slot_periodicity;
}

status_lorawan_t lr1_mac_core_get_link_check_ans( lr1_stack_mac_t* lr1_mac_obj, uint8_t* margin, uint8_t* gw_cnt )
{
    status_lorawan_t ret = OKLORAWAN;

    if( lr1_mac_obj->link_check_user_req == USER_MAC_REQ_ACKED )
    {
        *margin = lr1_mac_obj->link_check_margin;
        *gw_cnt = lr1_mac_obj->link_check_gw_cnt;
    }
    else
    {
        *margin = 0;
        *gw_cnt = 0;
        ret     = ERRORLORAWAN;
    }

    return ret;
}

status_lorawan_t lr1_mac_core_get_device_time_req_status( lr1_stack_mac_t* lr1_mac_obj )
{
    if( lr1_mac_obj->device_time_user_req == USER_MAC_REQ_ACKED )
    {
        return OKLORAWAN;
    }
    return ERRORLORAWAN;
}

/*
 *-----------------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITIONS ------------------------------------------------
 */

static void copy_user_payload( lr1_stack_mac_t* lr1_mac_obj, const uint8_t* data_in, const uint8_t size_in )
{
    memcpy1(
        &( lr1_mac_obj->tx_payload[FHDROFFSET + lr1_mac_obj->tx_fport_present + lr1_mac_obj->tx_fopts_current_length] ),
        data_in, size_in );
}

static uint32_t failsafe_timstamp_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->timestamp_failsafe;
}

static rp_status_t rp_status_get( lr1_stack_mac_t* lr1_mac_obj )
{
    return lr1_mac_obj->planner_status;
}

void lr1mac_core_abort( lr1_stack_mac_t* lr1_mac_obj )
{
    lr1_mac_obj->valid_rx_packet     = NO_MORE_VALID_RX_PACKET;
    lr1_mac_obj->type_of_ans_to_send = NOFRAME_TOSEND;
    lr1_mac_obj->receive_window_type = RECEIVE_NONE;
    lr1_mac_obj->nb_trans_cpt        = 1;
    rp_task_abort( lr1_mac_obj->rp, lr1_mac_obj->stack_id4rp );
}

static void lr1mac_mac_update( lr1_stack_mac_t* lr1_mac_obj, user_rx_packet_type_t* available_rx_packet )
{
    lr1_mac_obj->radio_process_state = RADIOSTATE_IDLE;

    if( lr1_mac_obj->valid_rx_packet == JOIN_ACCEPT_PACKET )
    {
        SMTC_MODEM_HAL_TRACE_MSG( " update join procedure\n" );
        if( lr1_stack_mac_join_accept( lr1_mac_obj ) == OKLORAWAN )
        {
            //@note because datarate Distribution has been changed during join
            lr1_mac_obj->adr_mode_select = lr1_mac_obj->adr_mode_select_tmp;
            smtc_real_set_dr_distribution( lr1_mac_obj, lr1_mac_obj->adr_mode_select_tmp );
            save_devnonce_rst( lr1_mac_obj );
        }
        else
        {
            SMTC_MODEM_HAL_TRACE_WARNING( "Not Joined\n" );
        }
    }
    if( ( lr1_mac_obj->valid_rx_packet == NWKRXPACKET ) || ( lr1_mac_obj->valid_rx_packet == USERRX_FOPTSPACKET ) )
    {
        lr1_stack_mac_cmd_parse( lr1_mac_obj );
    }
    lr1_stack_mac_update( lr1_mac_obj );
    *available_rx_packet = lr1_mac_obj->available_app_packet;

    if( lr1_mac_obj->available_app_packet == LORA_RX_PACKET_AVAILABLE )
    {
        lr1_mac_obj->push_callback( lr1_mac_obj->push_context );
    }

    if( ( lr1_mac_obj->type_of_ans_to_send == NWKFRAME_TOSEND ) ||
        ( lr1_mac_obj->type_of_ans_to_send == USRFRAME_TORETRANSMIT ) )
    {  // @note ack send during the next tx|| ( packet.IsFrameToSend == USERACK_TOSEND ) ) {
        // Decrement duty cycle before check the available DTC
        smtc_duty_cycle_update( lr1_mac_obj->dtc_obj );
        if( smtc_real_get_next_channel( lr1_mac_obj ) != OKLORAWAN )
        {
            lr1_mac_obj->lr1mac_state = LWPSTATE_IDLE;
            if( lr1_mac_obj->type_of_ans_to_send == USRFRAME_TORETRANSMIT )
            {
                lr1_mac_obj->fcnt_up++;
                lr1_mac_obj->adr_ack_cnt++;  // increment adr counter each new uplink frame
            }
        }
        else
        {
            lr1_mac_obj->type_of_ans_to_send = NOFRAME_TOSEND;
            lr1_mac_obj->rtc_target_timer_ms =
                smtc_modem_hal_get_time_in_ms( ) + ( smtc_modem_hal_get_random_nb_in_range( 1, 3 ) * 1000 );
            lr1_mac_obj->lr1mac_state = LWPSTATE_TX_WAIT;
        }
    }
    else
    {
        lr1_mac_obj->lr1mac_state = LWPSTATE_IDLE;
    }
    lr1_mac_obj->valid_rx_packet = NO_MORE_VALID_RX_PACKET;
}

static void save_devnonce_rst( const lr1_stack_mac_t* lr1_mac_obj )
{
    lr1_counter_context_t ctx = { 0 };

    ctx.devnonce = lr1_mac_obj->dev_nonce;
    ctx.nb_reset = lr1_mac_obj->nb_of_reset;
    memcpy( ctx.join_nonce, lr1_mac_obj->join_nonce, sizeof( ctx.join_nonce ) );
    ctx.crc = lr1mac_utilities_crc( ( uint8_t* ) &ctx, sizeof( ctx ) - 4 );

    smtc_modem_hal_context_store( CONTEXT_DEVNONCE, ( uint8_t* ) &ctx, sizeof( ctx ) );
}

static void load_devnonce_reset( lr1_stack_mac_t* lr1_mac_obj )
{
    lr1_counter_context_t ctx = { 0 };
    smtc_modem_hal_context_restore( CONTEXT_DEVNONCE, ( uint8_t* ) &ctx, sizeof( ctx ) );

    if( lr1mac_utilities_crc( ( uint8_t* ) &ctx, sizeof( ctx ) - 4 ) == ctx.crc )
    {
        lr1_mac_obj->dev_nonce   = ctx.devnonce;
        lr1_mac_obj->nb_of_reset = ctx.nb_reset;
        memcpy( lr1_mac_obj->join_nonce, ctx.join_nonce, sizeof( lr1_mac_obj->join_nonce ) );
    }
    else
    {
        SMTC_MODEM_HAL_TRACE_WARNING( "No valid DevNonce in NVM, use default (0)\n" );
    }
}

static void try_recover_nvm( lr1_stack_mac_t* lr1_mac_obj )
{
    SMTC_MODEM_HAL_TRACE_PRINTF( "Lr1mac context : Try recover NVM\n" );

    typedef struct mac_context_010007_s
    {
        uint8_t  joineui[8];
        uint8_t  deveui[8];
        uint8_t  appkey[16];
        uint16_t devnonce;
        uint16_t nb_reset;
        uint32_t adr_custom;
        uint8_t  region_type;
        uint32_t crc;
    } mac_context_010007_t;

    mac_context_010007_t old_save_fmt;

    smtc_modem_hal_context_restore( CONTEXT_LR1MAC, ( uint8_t* ) &old_save_fmt, sizeof( old_save_fmt ) );

    if( lr1mac_utilities_crc( ( uint8_t* ) &( old_save_fmt ), sizeof( old_save_fmt ) - 4 ) == old_save_fmt.crc )
    {
        SMTC_MODEM_HAL_TRACE_PRINTF( "Recover success from 1.0.7 !\n" );

        // memcpy1( lr1_mac_obj->join_eui, old_save_fmt.joineui, 8 );
        // memcpy1( lr1_mac_obj->dev_eui, old_save_fmt.deveui, 8 );
        // memcpy1( lr1_mac_obj->app_key, old_save_fmt.appkey, 16 );
        lr1_mac_obj->dev_nonce        = old_save_fmt.devnonce + 10;  // in 1.0.7 dev nonce was save every 10
        lr1_mac_obj->adr_custom       = old_save_fmt.adr_custom;
        lr1_mac_obj->nb_of_reset      = old_save_fmt.nb_reset;
        lr1_mac_obj->real.region_type = ( smtc_real_region_types_t ) old_save_fmt.region_type;

        save_devnonce_rst( lr1_mac_obj );
        lr1mac_core_context_save( lr1_mac_obj );  // to save new number of reset
    }
    else
    {
        SMTC_MODEM_HAL_TRACE_WARNING( "Fail to recover\n" );
    }
}

/* --- EOF ------------------------------------------------------------------ */
