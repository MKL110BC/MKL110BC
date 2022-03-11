#include "lbm_app.h"
#include "system_port.h"
#include "lr1110_system_types.h"
#include "lr1110_types.h"
#include "smtc_hal.h"
#include "lr1110_mb1lxks_board.h"
#include "lr1110_system.h"
#include "smtc_modem_utilities.h"
#include "smtc_board_ralf.h"


#include "apps_utilities.h"
#include "apps_modem_common.h"
#include "apps_modem_event.h"

#include "gnss_helpers.h"
#include "wifi_helpers.h"
#include "system_time.h"

static ralf_t* modem_radio;
static uint8_t join_flag = 0;
static uint8_t stack_id = 0;
static uint8_t wifi_can_scan_flag = 0;
static uint8_t time_syn_flag = 0;
static uint8_t gnss_can_scan_flag = 1;

#define WIFI_APP_PORT 		195
#define GNSS_APP_PORT 		195
#define MODEM_EXAMPLE_ASSISTANCE_POSITION_LAT (30.181454 )
#define MODEM_EXAMPLE_ASSISTANCE_POSITION_LONG ( 114.720893 )
static const lr1110_gnss_solver_assistance_position_t assistance_position = {
    .latitude  = MODEM_EXAMPLE_ASSISTANCE_POSITION_LAT,
    .longitude = MODEM_EXAMPLE_ASSISTANCE_POSITION_LONG,
};

#define GNSS_SCAN_TLV_SIZE 2
#define GNSS_SCAN_METADATA_SIZE 2
#define GNSS_SCAN_NB_SATELLITE_MIN 6

typedef struct
{
    uint8_t detected_sv; /* Number of satellites detected */
    uint8_t results_size;
    uint8_t results_buffer[GNSS_SCAN_TLV_SIZE + GNSS_SCAN_METADATA_SIZE + GNSS_RESULT_SIZE_MAX_MODE3];
} gnss_scan_t;

void on_modem_reset(uint16_t reset_count)
{
	SYS_LOG_INFO("LR1110 RESET,reset count is %d\r\n",reset_count);
}

void on_modem_join_deal(void)
{	
	join_flag= 1;
}
void on_modem_syn_time( smtc_modem_event_time_status_t time_status )
{
	switch(time_status)
	{
		case SMTC_MODEM_EVENT_TIME_VALID:
			time_syn_flag = 1;
			SYS_LOG_INFO("on_modem_syn_time time syn\r\n");
			break;
	}
}





static void on_modem_tx_done( smtc_modem_event_txdone_status_t status )
{

}

void wifi_scan_deal(void)
{
	wifi_can_scan_flag = 0;
	wifi_helpers_end_scan( );
	uint8_t nb_wifi_mac = 0;
		// Wi-Fi scan completed, get and display the results
		wifi_helpers_get_results( modem_radio->ral.context, &nb_wifi_mac );
	if( nb_wifi_mac > 0 )
	{
		uint8_t wifi_result_buffer[255] = { 0 };
		uint8_t wifi_buffer_size                                = 0;
		wifi_helpers_display_results( );
		wifi_helpers_prepare_result_payload( wifi_result_buffer, &wifi_buffer_size, 255 );

		if( smtc_modem_request_uplink(stack_id,WIFI_APP_PORT,0,wifi_result_buffer,wifi_buffer_size)!= SMTC_MODEM_RC_OK )
		{
			HAL_DBG_TRACE_ERROR( "SEND FRAME ERROR\n" );
		}
	}
}
void gnss_scan_deal(void)
{
	bool request_almanac_update = false;
	gnss_scan_t scan_results;
	smtc_gnss_scan_ended( );
	memset( &scan_results, 0, sizeof scan_results );
        smtc_gnss_get_results( modem_radio->ral.context, &scan_results.detected_sv, &scan_results.results_size,
                               &scan_results.results_buffer[GNSS_SCAN_TLV_SIZE + GNSS_SCAN_METADATA_SIZE],
                               &request_almanac_update );
	if(request_almanac_update)
	{	
		SYS_LOG_INFO("gnss_scan_deal request_almanac_update\r\n");
		 uint8_t dm_almanac_status = SMTC_MODEM_DM_FIELD_ALMANAC_STATUS;
            ASSERT_SMTC_MODEM_RC( smtc_modem_dm_request_single_uplink( &dm_almanac_status, 1 ) );
	}
	else
	{
		gnss_can_scan_flag = 1;
		if(scan_results.detected_sv<GNSS_SCAN_NB_SATELLITE_MIN)
			return;
		 smtc_gnss_display_results( scan_results.detected_sv,
                                       &scan_results.results_buffer[GNSS_SCAN_TLV_SIZE + GNSS_SCAN_METADATA_SIZE],
                                       scan_results.results_size );
		
		 scan_results.results_buffer[3] = 0X07;
		 scan_results.results_buffer[4] = scan_results.results_size -1;
		 smtc_modem_request_uplink(stack_id,GNSS_APP_PORT,1,scan_results.results_buffer+3, scan_results.results_size+1);

		SYS_LOG_INFO_BUF("gps send single is",scan_results.results_buffer+3,scan_results.results_size+1);
	}
}
void on_modem_user_radio_access(uint32_t timestamp_ms, smtc_modem_event_user_radio_access_status_t status )
{
	switch(status)
	{
    		case SMTC_MODEM_EVENT_USER_RADIO_ACCESS_WIFI_SCAN_DONE:
			wifi_scan_deal();
			break;
    		case SMTC_MODEM_EVENT_USER_RADIO_ACCESS_GNSS_SCAN_DONE:
			gnss_scan_deal();
			break;
	}
}
void lbm_init(void)
{
	lr1110_system_version_t            	lr1110_fw_version;
	smtc_modem_lorawan_version_t  lorawan_version;
	smtc_modem_version_t            	modem_version;
	
    	lr1110_status_t                    	status;

	static apps_modem_event_callback_t smtc_event_callback = {
        .adr_mobile_to_static  = NULL,
        .alarm                 = NULL,
        .almanac_update        =NULL,
        .down_data             = NULL,
        .join_fail             = NULL,
        .joined                = on_modem_join_deal,
        .link_status           = NULL,
        .mute                  = NULL,
        .new_link_adr          = NULL,
        .reset                 = on_modem_reset,
        .set_conf              = NULL,
        .stream_done           = NULL,
        .time_updated_alc_sync = on_modem_syn_time,
        .tx_done               = on_modem_tx_done,
        .upload_done           = NULL,
        .user_radio_access     = on_modem_user_radio_access,
    };

	HAL_DBG_TRACE_MSG("LR1110 start\r\n");
	modem_radio = smtc_board_initialise_and_get_ralf( );
//	hal_mcu_disable_irq();
	hal_mcu_init( );
    	apps_modem_event_init( &smtc_event_callback );
    	smtc_modem_init( modem_radio, &apps_modem_event_process );
//	hal_mcu_enable_irq( );


//	lr1110_system_get_version( modem_radio->ral.context, &lr1110_fw_version );
//	SYS_LOG_INFO("lr1110_system_get_version is %04x,USE CASE is %02x\r\n",lr1110_fw_version.fw,lr1110_fw_version.type,status);
	smtc_modem_get_lorawan_version(&lorawan_version);
	SYS_LOG_INFO( "smtc_modem_get_lorawan_version     : %d.%d.%d.%d\n", lorawan_version.major, lorawan_version.minor,
                        lorawan_version.patch, lorawan_version.revision );
	smtc_modem_get_modem_version(&modem_version);
	SYS_LOG_INFO( "smtc_modem_get_modem_version    : %d.%d.%d\n", modem_version.major, modem_version.minor,
                        modem_version.patch );
	smtc_modem_get_regional_params_version(&lorawan_version);
	SYS_LOG_INFO( "smtc_modem_get_regional_params_version     : %d.%d.%d.%d\n", lorawan_version.major, lorawan_version.minor,
                        lorawan_version.patch, lorawan_version.revision );
	
}
void lbm_lorawan_init(void)
{
	uint8_t app_eui[8] = {0x00, 0x16, 0xC0, 0x01, 0xFF, 0xFE, 0x00, 0x01};
	uint8_t dev_eui[8] = {0x00, 0x16, 0xC0, 0x01, 0xF0, 0x00, 0x00, 0x00 };
	uint8_t appkey[16] = {0XBE,0XBB,0X41,0X58,0X47,0X6D,0XA5,0XC0,0X0D,0X40,0X5E,0X4B,0X85,0X3D,0XC9,0X7C};


	smtc_modem_set_region(stack_id,SMTC_MODEM_REGION_EU_868);
	smtc_modem_set_class(stack_id,SMTC_MODEM_CLASS_A);
	smtc_modem_set_deveui(stack_id,dev_eui);
	smtc_modem_set_joineui(stack_id,app_eui);
	smtc_modem_set_nwkkey(stack_id,appkey);


//	smtc_modem_set_tx_power_offset_db(stack_id,smtc_board_get_tx_power_offset());
	
}
void lbm_lorawan_join(void)
{
	static uint8_t state = 0;
	static uint32_t tick;
	smtc_modem_return_code_t rc;
	smtc_modem_stack_state_t stack_state;
	uint8_t sendbuf[10];
	uint8_t sendlen;
	switch(state)
	{
		case 0:
			SYS_LOG_INFO("lbm_lorawan_join init\r\n");
			lbm_lorawan_init();
			state = 1;
			tick = get_rtc();
			break;
		case 1:
			rc = smtc_modem_join_network(stack_id);
			SYS_LOG_INFO("smtc_modem_join_network rc is %d\r\n",rc);
			if(rc == SMTC_MODEM_RC_OK)
			{
				state = 2;
			}
			break;
		case 2: //wait join
			break;		
	}
}
void lbm_lorawan_send_test()
{
	static uint8_t state = 0;
	static uint32_t tick;
	switch(state)
	{
		case 0:
			if(join_flag)
			{
				state = 1;
				tick = get_rtc();
			}
			break;
		case 1:
			if(timepassed_rtc(tick,5000))
			{
				tick = get_rtc();
				uint8_t sendbuf[10] = {0,1,2,3,4,5,6,7,8,9};
				smtc_modem_stack_state_t stack_state;
				smtc_modem_get_stack_state( stack_id, &stack_state ) ;
				SYS_LOG_INFO("smtc_modem_get_stack_state is %d\r\n",stack_state)
				if(stack_state==SMTC_MODEM_STACK_STATE_IDLE)
				{
					smtc_modem_request_uplink(stack_id,20,0,sendbuf,10);
				}
			}
			break;

		
	}
}
void lbm_wifi_fix_test(void)
{
	static uint8_t state = 0;
	static uint32_t tick;
	smtc_modem_stack_state_t stack_state;
	switch(state)
	{
		case 0:
			// wait join to send network
			if(join_flag)
			{
				state = 1;
				tick = get_rtc();
				SYS_LOG_INFO("lbm_wifi_fix_test start\r\n");
			}
			break;
		case 1:
			if(timepassed_rtc(tick,15000) && wifi_can_scan_flag==0)
			{
				smtc_modem_get_stack_state( stack_id, &stack_state ) ;
				if(stack_state==SMTC_MODEM_STACK_STATE_IDLE)
				{
					wifi_can_scan_flag = 1;
					wifi_helpers_start_scan( stack_id, modem_radio->ral.context );
					SYS_LOG_INFO("lbm_wifi_fix start\r\n");
				}
				tick = get_rtc();
			}
			break;
		case 2:
			break;
	}
}

void lbm_gnss_fix_test()
{
	static uint8_t state = 0;
	static uint32_t tick;
	uint32_t gps_time;
	uint32_t fractional_seconds = 0;
	switch(state)
	{
		case 0:
			// assist scan need config ,but atuo  need't config
			if(join_flag)
			{
				SYS_LOG_INFO("lbm_gnss_fix_test start\r\n");
				smtc_gnss_init( modem_radio->ral.context,assistance_position );
				smtc_gnss_almanac_update( modem_radio->ral.context );
				state = 1;
				ASSERT_SMTC_MODEM_RC( smtc_modem_time_set_sync_interval_s( 3600 ) );     /* keep call order */
    				ASSERT_SMTC_MODEM_RC( smtc_modem_time_set_sync_invalid_delay_s( 24*3600 ) ); /* keep call order */
    				/* Start the service */
    				ASSERT_SMTC_MODEM_RC( smtc_modem_time_start_sync_service( stack_id, SMTC_MODEM_TIME_ALC_SYNC ) );
				tick = get_rtc();
			}
			break;
		case 1:
			if(time_syn_flag && timepassed_rtc(tick,15000)&& gnss_can_scan_flag ==1)
			{
				gnss_can_scan_flag = 0;
				smtc_modem_get_time( &gps_time, &fractional_seconds );
				uint8_t ret = smtc_gnss_scan( stack_id, modem_radio->ral.context, gps_time );
				SYS_LOG_INFO("smtc_gnss_scan ret is %d\r\n",ret);
				tick = get_rtc();
			}
			break;
		case 2:
			break;
	}
}

void lbm_cycle_test(void)
{

	lbm_lorawan_join();
	
	lbm_lorawan_send_test();

//	lbm_wifi_fix_test();

//	lbm_gnss_fix_test();


	smtc_modem_run_engine( );
}
