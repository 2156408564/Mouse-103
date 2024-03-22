/********************************************************************************************************
 * @file     aaa_app.c
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2010, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *			 The information contained herein is confidential and proprietary property of Telink
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in.
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/

#include "AAA_public_config.h"
#include "../../application/audio/tl_audio.h"

#define     CONNECT_WITH_KMA_DONGLE                 1

#define     CUST_SAVE_MASTER_INFO_ENABLE_AAA        1
#define     LENGHT_USER_BOND_INF                    256


#define     MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_ALL
#define 	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_20MS
#define 	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_25MS
#define		USER_OWN_ADDRESS_TYPE				OWN_ADDRESS_RANDOM


#define MOUSE_TIMER_SHORT_T			8000
extern blt_soft_timer_t	blt_timer;
#define RX_FIFO_SIZE	64
#define RX_FIFO_NUM		8

#define TX_FIFO_SIZE	(112) //264 40
#define TX_FIFO_NUM		32

#define DLE_TX_SUPPORTED_DATA_LEN    100//MAX_OCTETS_DATA_LEN_EXTENSION //264-12 = 252 > Tx max:251
_attribute_data_retention_	u16  final_MTU_size = 23;
_attribute_data_retention_	u32 connect_event_occurTick = 0;
_attribute_data_retention_  u32 mtuExchange_check_tick = 0;
_attribute_data_retention_ 	int  dle_started_flg = 0;
_attribute_data_retention_ 	int  mtuExchange_started_flg = 0;

typedef enum
{
    INIT_DONE_AAA,
    BEGIN_CONNECT_AAA,
    SMP_FIRST_CONNECT_DONE_AAA,
    SMP_RECONNECT_DONE_AAA,
    CONNECTED_DONE_AAA,
    CONNECTED_LOOP_AAA,

} BLE_MODE_CONNECT_STEP_AAA;;

#define LONG_SUSPEND_TIMER_AAA 200


#define  DEFAULT_LATENCY  99

#define PARAMS_GROUP_LEN  4

#define DEFAULT_INTERVAL  6
#define DEFAULT_TIMEOUT  400

#define  REPORT_RATE_100   0  //0 =125~133

#if BLE_AUDIO_ENABLE
    extern int ui_mtu_size_exchange_req;
    extern _attribute_data_retention_  u8 ui_mic_enable;

    #if AUDIO_AUTO_OPEN_TEST_DEBUG
        _attribute_data_retention_user u8 app_mic_enable = 0;
        _attribute_data_retention_user u32 tick_app_mic = 0;
    #endif
#endif
_attribute_data_retention_user  u8 pair_success=0;


//_attribute_data_retention_user u8 exit_suspend_flag=1;
_attribute_data_retention_user u32 start_tick;

_attribute_data_retention_user u8 Switch_Adv_Type = 0;
//_attribute_data_retention_user u8 bonded_peer_addr[6] = {0, 0, 0, 0, 0, 0};


_attribute_data_retention_user u32 loop_cnt;
_attribute_data_retention_user u32 connect_begin_tick;

_attribute_data_retention_user u8 ui_ota_is_working = 0;
//_attribute_data_retention_user u8 conn_params_pending = 0;
_attribute_data_retention_user u8 conn_params_cout = 0;
_attribute_data_retention_user u32 conn_params_tick;

_attribute_data_retention_user u8 conn_step = 0;
_attribute_data_retention_user u32 adv_begin_tick;
_attribute_data_retention_user u32 adv_count = 0;

//_attribute_data_retention_user u32 connect_begin_tick;

_attribute_data_retention_user u8 ble_status_aaa;
_attribute_data_retention_user u32 idle_tick;
_attribute_data_retention_user u32  idle_count;
_attribute_data_retention_user u8 soft_time_flag = 0;

#if AUTO_CHECK_OS_TYPE
    //_attribute_data_retention_user u8 os_check=0;//0:unknown, 1:ios pending,2:android,3: ios
    _attribute_data_retention_user u8 os_type = 0;
    //_attribute_data_retention_user u8 peer_type=0;
    //_attribute_data_retention_user u16 peer_con_interval=0;
    //_attribute_data_retention_user u16 peer_con_timeout=0;
#endif

_attribute_data_retention_user u8 temp_master_addr[8] = {0};
#if CUST_SAVE_MASTER_INFO_ENABLE_AAA
//_attribute_data_retention_user STRUCT_USER_BOND_INF user_bond_inf[4];//muti device addr
_attribute_data_retention_user _attribute_aligned_(4) smp_param_save_t smp_param_inf[4];//muti device addr

_attribute_data_retention_user int binding_master_addr_idx;
#endif
u32 blt_ota_start_tick;


_attribute_data_retention_  u8 		 	blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_rxfifo =
{
    RX_FIFO_SIZE,
    RX_FIFO_NUM,
    0,
    0,
    blt_rxfifo_b,
};



_attribute_data_retention_  u8 		 	blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_txfifo =
{
    TX_FIFO_SIZE,
    TX_FIFO_NUM,
    0,
    0,
    blt_txfifo_b,
};


//#define		MTU_RX_BUFF_SIZE_MAX			ATT_ALLIGN4_DMA_BUFF(23)
//#define		MTU_TX_BUFF_SIZE_MAX			ATT_ALLIGN4_DMA_BUFF(23)

//_attribute_data_retention_ u8 mtu_rx_fifo[MTU_RX_BUFF_SIZE_MAX];
//_attribute_data_retention_ u8 mtu_tx_fifo[MTU_TX_BUFF_SIZE_MAX];


//////////Adv Packet, Response Packet//////////////////////////////////////////////

#if (Microsoft_Swift_Pairing_ENABLE&&(DEVICE_NAME_INCLUDE_MAC_DEBUG==0))
#define DEV_POSITION   20
#define USER_DEVICE_NAME_MAX_LEN  11
#define MS_soft_paring_index  11

_attribute_data_retention_user u8	tbl_advData_aaa[31] =
{

    0x02, 0x01, 0x05, 		// BLE limited discoverable mode and BR/EDR not supported
    0x03, 0x19, DEVICE_APPEARANCE&0XFF, (DEVICE_APPEARANCE>>8)&0xff, // 0xc2 mouse 0xc1 keyboard
    0x03, 0x03, 0x12, 0x18,	// incomplete list of service class UUIDs (0x1812-HID SERVICE, 180F-BATTERY)
    #if CONNECT_WITH_KMA_DONGLE
    (sizeof(DEVICE_ADV_NAME)), 0x09, 0x4d, 0x69, 0x63, 0x4c, 0x69, 0x6e, 0x6b,
    0x06, 0xff, 0x06, 0x00, 0x03, 0x00, 0x80,
    #else
    0x06, 0xff, 0x06, 0x00, 0x03, 0x00, 0x80,
    12, 0x09, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    #endif
};
#if CONNECT_WITH_KMA_DONGLE
const u8 tbl_scanRsp [] = {
		 //0x08, 0x09, 'F', 'Y', 't', 'e', 's', 't', 'e',
        //0x07, 0xFF, 0x11, 0x02, 0x01, 0x00, 0x58, 0x82
	};
#else
u8 tbl_scanRsp [20] = {
        (sizeof(DEVICE_NAME_AAA)), 0x09, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};
#endif


#else
#define DEV_POSITION   13
#define USER_DEVICE_NAME_MAX_LEN  18

_attribute_data_retention_user u8	tbl_advData_aaa[31] =
{

    0x02, 0x01, 0x05, 		// BLE limited discoverable mode and BR/EDR not supported
    0x03, 0x19, DEVICE_APPEARANCE&0XFF, (DEVICE_APPEARANCE>>8)&0xff, // 384, Keyboard, Generic category,
    0x03, 0x03, 0x12, 0x18,	// incomplete list of service class UUIDs (0x1812-HID SERVICE, 180F-BATTERY)
    19, 0x09,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
#endif

#if (DEVICE_NAME_INCLUDE_MAC_DEBUG)
void BufToHexString(u8 *rp, u8 *sp, u8 len)
{
    u8  hex[17] = "0123456789ABCDEF";
    u8 count = 0;
    u8      i;
    for (i = len; i > 0; i--)
    {
        rp[count++] = hex[sp[i - 1] >> 4];
        rp[count++] = hex[sp[i - 1] & 0x0F];
    }
}
#endif


void set_adv_scanRsp_data()
{
#if CONNECT_WITH_KMA_DONGLE
        bls_ll_setAdvData((u8 *)tbl_advData_aaa, sizeof(tbl_advData_aaa));
        bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));
#else
        if (device_name_len == 0)
        {
#if (DEVICE_NAME_INCLUDE_MAC_DEBUG)
            device_name_len = 16;
            memcpy(user_cfg.device_name, "58m_", 4);
            BufToHexString(&user_cfg.device_name[4], &bltMac.macAddress_public[0], 6);
#else
            device_name_len = sizeof(DEVICE_NAME_AAA) - 1;
            if (device_name_len > USER_DEVICE_NAME_MAX_LEN)
            {
                device_name_len = USER_DEVICE_NAME_MAX_LEN;
            }
            memcpy(user_cfg.device_name, DEVICE_NAME_AAA, device_name_len);
#endif
        }
        else
        {
            if (device_name_len > USER_DEVICE_NAME_MAX_LEN)
            {
                device_name_len = USER_DEVICE_NAME_MAX_LEN;
            }
        }
        //memset(&user_cfg.device_name[device_name_len], 0, 18 - device_name_len);
        //memcpy(&tbl_advData_aaa[DEV_POSITION], user_cfg.device_name, device_name_len);
        //tbl_advData_aaa[DEV_POSITION - 2] = device_name_len + 1;
        //bls_ll_setAdvData((u8 *)tbl_advData_aaa, device_name_len + DEV_POSITION);
        //bls_ll_setScanRspData(&tbl_advData_aaa[DEV_POSITION - 2], device_name_len + 2);
        bls_ll_setAdvData((u8 *)tbl_advData_aaa, DEV_POSITION - 2);
        memcpy(&tbl_scanRsp[2], DEVICE_NAME_AAA, sizeof(DEVICE_NAME_AAA) - 1);
        bls_ll_setScanRspData((u8 *)tbl_scanRsp, sizeof(DEVICE_NAME_AAA) + 1);
#endif
}

void set_reconnect_adv_scanRsp_data()
{
        device_name_len = sizeof(DEVICE_NAME_AAA) - 1;

        memcpy(&tbl_advData_aaa[MS_soft_paring_index], DEVICE_NAME_AAA, device_name_len);
        bls_ll_setAdvData((u8 *)tbl_advData_aaa, MS_soft_paring_index + 1 + device_name_len);
}


/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_CONN_PARA_REQ"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void	task_conn_update_req (u8 e, u8 *p, int n)
{
    printf("task_conn_update_req\n");

}




/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_CONN_PARA_UPDATE"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void	task_conn_update_done (u8 e, u8 *p, int n)
{
    printf("interval %d, latency %d, timeout %d\n", \
                             p[0]|p[1]<<8,p[2]|p[3]<<8,p[4]|p[5]<<8);
	//更新完连接参数之后,就更改配对信息
}

/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_DATA_LENGTH_EXCHANGE"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void task_dle_exchange (u8 e, u8 *p, int n)
{
	ll_data_extension_t* dle_param = (ll_data_extension_t*)p;
	printf("----- DLE exchange: -----\n");
	printf("connEffectiveMaxRxOctets: %d\n", dle_param->connEffectiveMaxRxOctets);
	printf("connEffectiveMaxTxOctets: %d\n", dle_param->connEffectiveMaxTxOctets);
	printf("connMaxRxOctets: %d\n", dle_param->connMaxRxOctets);
	printf("connMaxTxOctets: %d\n", dle_param->connMaxTxOctets);
	printf("connRemoteMaxRxOctets: %d\n", dle_param->connRemoteMaxRxOctets);
	printf("connRemoteMaxTxOctets: %d\n", dle_param->connRemoteMaxTxOctets);
	if(dle_param->connEffectiveMaxTxOctets < DLE_TX_SUPPORTED_DATA_LEN){
		audio_reat_setting = AUDIO_8K;
		flash_dev_info.audio_reat = 0;
	}else{
		audio_reat_setting = AUDIO_16K;
		flash_dev_info.audio_reat = 1;
	}

	dle_started_flg = 1;
}

/**
 * @brief      the function serves to sdle test in mainloop
 * @param[in]  n - data length of event
 * @return     0
 */
void feature_sdle_test_mainloop(void)
{
	if(connect_event_occurTick && clock_time_exceed(connect_event_occurTick, 1500000)){  //1.5 S after connection established
		connect_event_occurTick = 0;

		mtuExchange_check_tick = clock_time() | 1;
		if(!mtuExchange_started_flg){  //master do not send MTU exchange request in time
			blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
			printf("After conn 1.5s, S send  MTU size req to the Master.\n");
		}
	}



	if(mtuExchange_check_tick && clock_time_exceed(mtuExchange_check_tick, 500000 )){  //2 S after connection established
		mtuExchange_check_tick = 0;

		if(!dle_started_flg){ //master do not send data length request in time
			printf("Master hasn't initiated the DLE yet, S send DLE req to the Master.\n");
			blc_ll_exchangeDataLength(LL_LENGTH_REQ , DLE_TX_SUPPORTED_DATA_LEN);
			dle_started_flg = 1;
		}
	}

}


/**
 * @brief		call back function of connect parameter update response
 * @param[in]	id - connect id
 * @param[in]	result - connect parameter update response result
 * @return      0    - success
 *              else - reserved
 */
int app_conn_param_update_response(u8 id, u16  result)
{
	if(result == CONN_PARAM_UPDATE_ACCEPT){
        printf("task_conn_update_accept\n");

	}
	else if(result == CONN_PARAM_UPDATE_REJECT){
        printf("task_conn_update_reject\n");

	}

	return 0;
}

void connect_params_proc()
{
    if (bls_ll_getConnectionInterval() <= DEFAULT_INTERVAL)
    {
        conn_params_cout = 0;
    }
    else if ((conn_params_cout < PARAMS_GROUP_LEN) && clock_time_exceed(conn_params_tick, 600000))
    {
    	my_printf_aaa("conn param update--1 \n");
        conn_params_tick = clock_time() | 1;
        bls_l2cap_requestConnParamUpdate(DEFAULT_INTERVAL, DEFAULT_INTERVAL, DEFAULT_LATENCY, DEFAULT_TIMEOUT);
        conn_params_cout++;
    }
}

void app_switch_to_indirect_adv(u8 e, u8 *p, int n)
{
    Switch_Adv_Type += 1;

    if (Switch_Adv_Type & 0x01)
    {
        set_reconnect_adv_scanRsp_data();
        bls_ll_setAdvParam(MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                           ADV_TYPE_CONNECTABLE_UNDIRECTED, USER_OWN_ADDRESS_TYPE,
                           0,  NULL,
                           MY_APP_ADV_CHANNEL,
                           ADV_FP_ALLOW_SCAN_WL_ALLOW_CONN_WL);
    }
    else
    {
        bls_ll_setAdvParam(ADV_INTERVAL_3_75MS, ADV_INTERVAL_3_75MS,
                           ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, USER_OWN_ADDRESS_TYPE,
                           BLE_ADDR_PUBLIC, smp_param_inf[flash_dev_info.mast_id].peer_addr,
                           BLT_ENABLE_ADV_ALL,
                           ADV_FP_ALLOW_SCAN_WL_ALLOW_CONN_WL);
    }
    bls_ll_setAdvDuration(2000000, 1);
    bls_ll_setAdvEnable(1);  //must: set adv enable
}

_attribute_ram_code_ void user_set_rf_power(u8 e, u8 *p, int n)
{
    rf_set_power_level_index(user_cfg.tx_power);
}

int cccWrite(void *p)
{
    my_fifo_reset (&fifo_km);
    connect_begin_tick = clock_time() | 1;
    return 0;
}

void task_connect(u8 e, u8 *p, int n)
{

	bls_l2cap_requestConnParamUpdate(CONN_INTERVAL_7P5MS, CONN_INTERVAL_7P5MS, 0x2c, CONN_TIMEOUT_4S);
	bls_l2cap_setMinimalUpdateReqSendingTime_after_connCreate(1000);
    //blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);


    //blc_ll_exchangeDataLength(LL_LENGTH_REQ, 200);
    connect_begin_tick = clock_time() | 1;
    connect_event_occurTick = clock_time() | 1;
	mtuExchange_check_tick = clock_time() | 1;
	//MTU size reset to default 23 bytes every new connection, it can be only updated by MTU size exchange procedure
	final_MTU_size = 23;
    my_fifo_reset(&fifo_km);
    reset_idle_status();
    clear_pair_flag();
    connect_ok = 1;
    ui_ota_is_working = 0;
    //conn_params_pending=0;
    conn_params_cout = 0;
    conn_params_tick = 0;
    conn_step = BEGIN_CONNECT_AAA;
    bls_ll_setAdvDuration(0, 0);
    ble_status_aaa = BEGIN_CONNECTED_STATUS_AAA;

#if BLE_AUDIO_ENABLE
    ui_mtu_size_exchange_req = 1;
#if AUDIO_AUTO_OPEN_TEST_DEBUG
    tick_app_mic = clock_time();
    app_mic_enable = 0;
#endif
#endif

	my_printf_aaa("task_connect\n");


    //rf_packet_connect_t *pc = (rf_packet_connect_t *)(p - 6);
#if UART_PRINT_DEBUG_ENABLE
    // my_printf_aaa("conn req interval=%x,latency=%x,timeout=%x\r\n", pc->interval, pc->latency, pc->timeout);
    //my_printf_aaa("scan_addr=%02X ,%02X%02X%02X%02X%02X%02X\r\n", pc->txAddr, pc->initA[5], pc->initA[4], pc->initA[3], pc->initA[2], pc->initA[1], pc->initA[0]);
    // my_printf_aaa("adv_addr=%02X%02X%02X%02X%02X%02X\r\n", pc->advA[5], pc->advA[4], pc->advA[3], pc->advA[2], pc->advA[1], pc->advA[0]);
#endif

#if AUTO_CHECK_OS_TYPE

    //peer_type=pc->type;
    //peer_con_interval=pc->interval;
    //peer_con_timeout=pc->timeout;

    if (os_type == UNKNOW_OS_TYPE)
    {
        if (pc->txAddr)
        {
            if ((pc->interval >= 0x0c) && (pc->interval <= 0x18) && (pc->timeout <= 0xc8))
            {
                os_type = APPLE_OS_TYPE;
#if UART_PRINT_DEBUG_ENABLE
                my_printf_aaa("apple_os\r\n");
#endif
            }
            else
            {
                os_type = ANDROID_OS_TYPE;
#if UART_PRINT_DEBUG_ENABLE
                my_printf_aaa("android_os\r\n");
#endif
            }
        }
        else//public
        {
            // os_type maybe detect errors
            //because  some andriod phone is public for exampe huawei asw=4.4.3
            os_type = WINDOWS_OS_TYPE;
#if UART_PRINT_DEBUG_ENABLE
            my_printf_aaa("window_os\r\n");
#endif
        }
    }
#endif

}


void task_terminate(u8 e, u8 *p, int n) //*p is terminate reason
{

	connect_event_occurTick = 0;
	mtuExchange_check_tick = 0;
	//MTU size exchange and data length exchange procedure must be executed on every new connection,
	//so when connection terminate, relative flags must be cleared
	dle_started_flg = 0;
	mtuExchange_started_flg = 0;

	//MTU size reset to default 23 bytes when connection terminated
	final_MTU_size = 23;

	OPTSensor_Shutdown();
#if 1
	bls_ll_setAdvEnable(0);
	my_printf_aaa("task_terminate %x\r\n", *p);
    if ((ble_status_aaa == DEEP_TERMINATE_STATUS_AAA))
    {
        ble_status_aaa = DEEP_SLEEPE_STATUS_AAA;
    }
	else if(active_disconnect_reason==BLE_PAIR_REBOOT_ANA_AAA)
	{
		user_reboot(BLE_PAIR_REBOOT_ANA_AAA);
	}
	else if(active_disconnect_reason==MODE_CHANGE_REBOOT_ANA_AAA)
    {
        bls_ll_setAdvEnable(0);
        flash_dev_info.mode = RF_2M_2P4G_MODE;
        save_dev_info_flash();
        user_reboot(MODE_CHANGE_REBOOT_ANA_AAA);
    }
	else if(active_disconnect_reason==MUTI_DEVICE_REBOOT_ANA_AAA)
	{
		bls_ll_setAdvEnable(0);
		save_dev_info_flash();
		user_reboot(MUTI_DEVICE_REBOOT_ANA_AAA);
	}
    else
    {
        ble_status_aaa = POWER_ON_STATUS_AAA;
    }
#else
    bls_ll_setAdvEnable(0);
#endif
    active_disconnect_reason=0;
    conn_step = 0;
    ui_ota_is_working = 0;

    connect_ok = 0;
#if AUTO_CHECK_OS_TYPE
    os_type = UNKNOW_OS_TYPE;
#endif

#if (BLE_AUDIO_ENABLE)
    if (ui_mic_enable){
        ui_enable_mic(0);
    }
#endif

#if BLT_SOFTWARE_TIMER_ENABLE
    delet_soft_time();
#endif
}


void get_master_real_mac()
{
#if SHOW_MAST_REAL_MAC_DEBUG
    if (temp_master_addr[0] & 0x40) //OWN_ADDRESS_RANDOM;
    {
        smp_param_save_t  bondInfo = {0};
        u32 ret = blc_smp_param_loadByAddr(1, &temp_master_addr[2], &bondInfo);
        if (ret == 0)
        {
            memset(&output_dev_info.master_mac[0], 0, 6);
            return;
        }
        else
        {
            memcpy(&output_dev_info.master_mac[0], &bondInfo.peer_id_addr[0], 6);
        }
    }
    else
    {
        memcpy(&output_dev_info.master_mac[0], &temp_master_addr[2], 6);
    }
#endif
}

void task_gpio_eary_wakeup(u8 e, u8 *p, int n)
{
    if (e == BLT_EV_FLAG_GPIO_EARLY_WAKEUP)
    {
        ui_loop();
    }
}

void save_smp_inf()
{
    if (pair_success)
    {
		pair_success=0;
		u32 CurStartAddr=0;
		smp_param_save_t smp_param_new;
		extern int blc_smp_param_getCurStartAddr();
		CurStartAddr = blc_smp_param_getCurStartAddr();

		flash_read_page(CurStartAddr+bond_device_flash_cfg_idx, 64, &smp_param_new.flag);
		memcpy(&smp_param_inf[flash_dev_info.mast_id].flag, &smp_param_new.flag, 64);
		binding_master_addr_idx+=LENGHT_USER_BOND_INF;
		flash_write_page_user(CFG_MAST_ADDR + binding_master_addr_idx, LENGHT_USER_BOND_INF, (u8*)&smp_param_inf[0].flag);
#if 0
		printf("peer addr type:%01x--- 0x%01x%01x%01x%01x%01x%01x\n", smp_param_new.peer_addr_type,	smp_param_new.peer_addr[0], smp_param_new.peer_addr[1], smp_param_new.peer_addr[2], smp_param_new.peer_addr[3], smp_param_new.peer_addr[4],smp_param_new.peer_addr[5]);
		printf("peer id addr type:%01x--- 0x%01x%01x%01x%01%01x%01x\n", smp_param_new.peer_id_adrType, smp_param_new.peer_id_addr[0], smp_param_new.peer_id_addr[1], smp_param_new.peer_id_addr[2],smp_param_new.peer_id_addr[3], smp_param_new.peer_id_addr[4],smp_param_new.peer_id_addr[5]);
#endif
		if (deep_flag == BLE_PAIR_REBOOT_ANA_AAA)
	   {
		   write_deep_ana0(CLEAR_FLAG_ANA_AAA);
		   flash_dev_info.slave_mac_addr[flash_dev_info.mast_id]++;
		   save_dev_info_flash();
	   }
	}
}



_attribute_ram_code_ void  ble_remote_set_sleep_wakeup(u8 e, u8 *p, int n)
{
    if (suspend_wake_up_enable && (blc_ll_getCurrentState() == BLS_LINK_STATE_CONN) && (((u32)(bls_pm_getSystemWakeupTick() - clock_time())) > 20 * CLOCK_16M_SYS_TIMER_CLK_1MS)) //suspend time > 30ms.add gpio wakeup
    {
        bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio pad wakeup suspend/deepsleep
#if WHEEL_FUN_ENABLE_AAA
        wheel_set_wakeup_level_suspend(1);
#endif
#if BUTTON_FUN_ENABLE_AAA
        btn_set_wakeup_level_suspend(1);
#endif
#if SENSOR_FUN_ENABLE_AAA
        sensor_set_wakeup_level_suspend(1);
#endif
    }
}


#if (BLE_OTA_ENABLE_AAA)
void app_Version_ReqCb(void)
{
    my_printf_aaa("app_Version_ReqCb \r\n");
}

void app_enter_ota_mode(void)
{
	wd_stop();
    ui_ota_is_working = 1;
#if (BLT_APP_LED_ENABLE && BLE_OTA_LED_DEBUG)
    gpio_write(PIN_BLE_LED, 1);
#endif
#if UART_PRINT_DEBUG_ENABLE
    my_printf_aaa("---start ota---\r\n");
#endif

#if (APP_FLASH_LOCK_ENABLE)
	flash_unlock_init();
	WaitMs(15);
#endif

    blt_ota_start_tick = clock_time();  //mark time
	//usb_start_flag = 1;  //mark time
}
void app_debug_ota_result(int result)
{
    irq_disable();
    //flash_erase_sector(CFG_DEVICE_MODE_ADDR);
    wd_stop();
	
#if(BLT_APP_LED_ENABLE)
    if (result == OTA_SUCCESS)  //OTA success
    {
       flash_erase_sector(CFG_DEVICE_MODE_ADDR);
#if UART_PRINT_DEBUG_ENABLE
        my_printf_aaa("ota success\r\n");
#endif
    }
    else   //OTA fail
    {
#if UART_PRINT_DEBUG_ENABLE
        my_printf_aaa("ota fail=%x\r\n",result);
#endif
    }
#endif
}
#endif

int app_host_event_callback(u32 h, u8 *para, int n)
{
    u8 event = h & 0xFF;

    switch (event)
    {
        case GAP_EVT_SMP_PARING_FAIL:
        {
            gap_smp_paringFailEvt_t *p = (gap_smp_paringFailEvt_t *)para;

            if (p->reason == PARING_FAIL_REASON_UNSPECIFIED_REASON)
            {
                bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN); //push terminate cmd into ble TX buffer
                printf("HCI_ERR_REMOTE_USER_TERM_CONN");
            }
        }
        break;
        case GAP_EVT_SMP_PARING_SUCCESS:
        {
            gap_smp_paringSuccessEvt_t *p = (gap_smp_paringSuccessEvt_t *)para;
            if (p->bonding_result)
            {
			 	pair_success=1;
				#if UART_PRINT_DEBUG_ENABLE
                my_printf_aaa("pair success\r\n");
                #endif
                //blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
			 }
        }
        break;

        case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
        {

            gap_smp_connEncDoneEvt_t *p = (gap_smp_connEncDoneEvt_t *)para;
            if (p->re_connect == SMP_STANDARD_PAIR)  //first paring
            {
       			conn_step = SMP_FIRST_CONNECT_DONE_AAA;

                    //bls_smp_enableParing (SMP_PARING_CONN_TRRIGER);
#if UART_PRINT_DEBUG_ENABLE
                my_printf_aaa("first connected\r\n");
#endif
            }
            else if (p->re_connect == SMP_FAST_CONNECT)  //auto connect
            {
                my_fifo_reset(&fifo_km);
                conn_step = SMP_RECONNECT_DONE_AAA;
#if UART_PRINT_DEBUG_ENABLE
                my_printf_aaa("reconnected\r\n");
#endif
            }
            connect_begin_tick = clock_time() | 1;
			connect_ok = 1;
			Sensor_Concent_Init();
        }
        break;
        case GAP_EVT_ATT_EXCHANGE_MTU:
        {
#if 1
            gap_gatt_mtuSizeExchangeEvt_t *p = (gap_gatt_mtuSizeExchangeEvt_t *)para;
			final_MTU_size = p->effective_MTU;
            my_printf_aaa("peer_size %x\r\n", p->peer_MTU);
            my_printf_aaa("final_MTU_size %x\r\n", p->effective_MTU);
			mtuExchange_started_flg = 1;   //set MTU size exchange flag here
#endif
        }
        break;
        default:
            break;
    }

    return 0;
}

void change_ble_stack_smp_inf()
{
	smp_param_save_t tmp[4];
	u8* src=&smp_param_inf[0].flag;
	u8 *last=&tmp[0].flag;
	
	binding_master_addr_idx = flash_info_load_aaa(CFG_MAST_ADDR, src, LENGHT_USER_BOND_INF);
	if(binding_master_addr_idx<0)
	{
		set_pair_flag();
		return;
	}
	flash_read_page(SMP_PARAM_NV_ADDR_START, 256, last);
	for(u8 i=0;i<8;i++)
	{
		if(memcmp(last,src, 256))
		{
			flash_erase_sector(SMP_PARAM_NV_ADDR_START);
			flash_write_page(SMP_PARAM_NV_ADDR_START, 256, src);
			flash_read_page(SMP_PARAM_NV_ADDR_START, 256, last);
			
		}
		else
		{
			break;
		}
	}	
}

void user_init_normal(void)
{
        blt_txfifo.wptr = 0;
        blt_txfifo.rptr = 0;
        blt_rxfifo.wptr = 0;
        blt_rxfifo.rptr = 0;

        ////////////////// BLE stack initialization ////////////////////////////////////
#if BLE_SNIFF_DEBUG
        u8  tbl_mac [6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xc7};
        extern  u8  blc_adv_channel[];
        //_attribute_data_retention_    u8      blc_adv_channel[3] = {37, 38, 39};
        blc_adv_channel[0] = 37;
        blc_adv_channel[1] = 38;
        blc_adv_channel[2] = 39;
        memcpy(tbl_mac, (u8 *)&user_cfg.dev_mac, 6);
        tbl_mac[5] = 0XD0 | (flash_dev_info.mast_id + 1);
#else
        u8  tbl_mac [] = {0x12, 0x34, 0x56, 0x78, 0x11, 0x54};
        memcpy(tbl_mac, (u8 *)&user_cfg.dev_mac, 6);
    
        if (pair_flag)
        {
            tbl_mac[4] = flash_dev_info.slave_mac_addr[flash_dev_info.mast_id] + 1;
        }
        else
        {
            tbl_mac[4] = flash_dev_info.slave_mac_addr[flash_dev_info.mast_id];
        }
    
        tbl_mac[5] = 0XD0 | (flash_dev_info.mast_id + 1);
        
#endif
        //blc_smp_set_simple_multi_mac_en(1);
        //extern u8 device_mac_index;
        //device_mac_index=(flash_dev_info.mast_id + 1);

        change_ble_stack_smp_inf();

        blc_ll_setRandomAddr(tbl_mac);
    
        ////// Controller Initialization  //////////
        blc_ll_initBasicMCU();                      //mandatory
        blc_ll_initStandby_module(tbl_mac);             //mandatory
        blc_ll_initAdvertising_module(tbl_mac);     //adv module:        mandatory for BLE slave,
        blc_ll_initConnection_module();             //connection module  mandatory for BLE slave/master
        blc_ll_initSlaveRole_module();              //slave module:      mandatory for BLE slave,
        blc_ll_initPowerManagement_module();        //pm module:         optional
    
        ////// Host Initialization  //////////
        blc_gap_peripheral_init();    //gap initialization
        extern void my_att_init();
        my_att_init();  //gatt initialization

        //ATT initialization
        //If not set RX MTU size, default is: 23 bytes.  In this situation, if master send MtuSize Request before slave send MTU size request,
        //slave will response default RX MTU size 23 bytes, then master will not send long packet on host l2cap layer, link layer data length
        //extension feature can not be used.  So in data length extension application, RX MTU size must be enlarged when initialization.
        blc_att_setRxMtuSize(MTU_SIZE_SETTING);

        blc_l2cap_register_handler(blc_l2cap_packet_receive);   //l2cap initialization
    
        //// smp initialization ////
#if (BLE_REMOTE_SECURITY_ENABLE)
        bls_smp_configParingSecurityInfoStorageAddr(0x74000);
        blc_smp_peripheral_init();
    
        //Hid device on android7.0/7.1 or later version
        // New paring: send security_request immediately after connection complete
        // reConnect:  send security_request 1000mS after connection complete. If master start paring or encryption before 1000mS timeout, slave do not send security_request.
        blc_smp_configSecurityRequestSending(SecReq_NOT_SEND, SecReq_NOT_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )
    
        blc_gap_registerHostEventHandler(app_host_event_callback);
        blc_gap_setEventMask(GAP_EVT_MASK_SMP_PARING_FAIL               |  \
                             GAP_EVT_MASK_SMP_PARING_SUCCESS            |  \
                             GAP_EVT_MASK_ATT_EXCHANGE_MTU    |  \
                             GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE);
    
#else
        blc_smp_setSecurityLevel(No_Security);
#endif

        //HID_service_on_android7p0_init();  //hid device on android 7.0/7.1
        //blc_att_setServerDataPendingTime_upon_ClientCmd(50);
    
#if SIG_PROC_ENABLE
        blc_l2cap_reg_att_sig_hander(att_sig_proc_handler);         //register sig process handler
#endif
        //blc_l2cap_registerConnUpdateRspCb(app_conn_param_update_response);
    
        ///////////////////// USER application initialization ///////////////////
    
        set_adv_scanRsp_data();
    
        //set rf power index, user must set it after every suspend wakeup, cause relative setting will be reset in suspend
        user_set_rf_power(0, 0, 0);
        bls_app_registerEventCallback(BLT_EV_FLAG_SUSPEND_EXIT, &user_set_rf_power);
    
        //ble event call back
        bls_app_registerEventCallback(BLT_EV_FLAG_DATA_LENGTH_EXCHANGE, &task_dle_exchange);
        bls_app_registerEventCallback(BLT_EV_FLAG_CONNECT, &task_connect);
        bls_app_registerEventCallback(BLT_EV_FLAG_TERMINATE, &task_terminate);
        bls_app_registerEventCallback(BLT_EV_FLAG_CONN_PARA_REQ, &task_conn_update_req);
        bls_app_registerEventCallback(BLT_EV_FLAG_CONN_PARA_UPDATE, &task_conn_update_done);
        bls_app_registerEventCallback(BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &app_switch_to_indirect_adv);
        //bls_app_registerEventCallback(BLT_EV_FLAG_DATA_LENGTH_EXCHANGE, &task_data_length_exchange);

        bls_app_registerEventCallback(BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &task_gpio_eary_wakeup);
        //unit is 10ms
        blc_att_setServerDataPendingTime_upon_ClientCmd(10);
    
        ///////////////////// Power Management initialization///////////////////
#if(BLE_APP_PM_ENABLE)
        blc_ll_initPowerManagement_module();
    
#if (PM_DEEPSLEEP_RETENTION_ENABLE)
        bls_pm_setSuspendMask(SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
        blc_pm_setDeepsleepRetentionThreshold(95, 95);
        blc_pm_setDeepsleepRetentionEarlyWakeupTiming(400);
        blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);
#else
        bls_pm_setSuspendMask(SUSPEND_ADV | SUSPEND_CONN);
#endif
    
        bls_app_registerEventCallback(BLT_EV_FLAG_SUSPEND_ENTER, &ble_remote_set_sleep_wakeup);
#else
        bls_pm_setSuspendMask(SUSPEND_DISABLE);
#endif
    
#if (BLE_OTA_ENABLE_AAA)
#if MODULE_WATCHDOG_ENABLE
        wd_stop();
#endif
        ////////////////// OTA relative ////////////////////////
        blc_ota_initOtaServer_module();
        bls_ota_clearNewFwDataArea(); //must
        bls_ota_registerStartCmdCb(app_enter_ota_mode);
        bls_ota_registerResultIndicateCb(app_debug_ota_result);  //debug
        //bls_ota_registerVersionReqCb(app_Version_ReqCb);
        blc_ota_setOtaProcessTimeout(150);
#if	MODULE_WATCHDOG_ENABLE
        wd_start();
#endif
#endif

#if BLT_SOFTWARE_TIMER_ENABLE
        blt_soft_timer_init();
#endif
        start_tick = clock_time();
}

_attribute_ram_code_ void user_init_deepRetn(void)
{
#if (PM_DEEPSLEEP_RETENTION_ENABLE)

    blc_ll_initBasicMCU();   //mandatory
    rf_set_power_level_index(user_cfg.tx_power);

    blc_ll_recoverDeepRetention();

    DBG_CHN0_HIGH;    //debug
    irq_enable();

    //pm mask disable,in main_loop according to ui task will reset pm mask
    bls_pm_setSuspendMask(SUSPEND_DISABLE);

    //app_ui_init_deepRetn();
#endif
}

void enter_deep_aaa()
{
	printf("enter deep\n");

#if (BLE_AUDIO_ENABLE)
        if (ui_mic_enable){
            ui_enable_mic(0);
        }
#endif

#if (!TEST_LOST_RATE)
    write_deep_ana0(DEEP_SLEEP_ANA_AAA);
    analog_write(USED_DEEP_ANA_REG1, (fun_mode << 1));

#if BUTTON_FUN_ENABLE_AAA
    btn_set_wakeup_level_deep();
#endif

#if WHEEL_FUN_ENABLE_AAA
    wheel_set_wakeup_level_deep();
#endif
#if SENSOR_FUN_ENABLE_AAA
#if (SENSOR_SHUT_DOWN_ENABLE==0)
    sensor_set_wakeup_level_deepsleep(1);
#endif
    OPTSensor_Shutdown();
#endif

    clear_pair_flag();

    cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);

#endif
}

void set_adv_type()
{
	u32 *adr=(u32*)&smp_param_inf[flash_dev_info.mast_id].peer_addr;
	
    if ((binding_master_addr_idx >= 0) && (pair_flag == 0)) 	//at least 1 bonding device exist
    {
		if ((adr[0] == 0)||(adr[0]==U32_MAX)) 
		{ //check whether this mast_id has been bonded
			my_printf_aaa("channel %d not bonded\n", flash_dev_info.mast_id);//not bonded
			set_pair_flag();
			bls_ll_setAdvDuration(0, 0);
	        bls_ll_setAdvParam(MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
	                           ADV_TYPE_CONNECTABLE_UNDIRECTED, USER_OWN_ADDRESS_TYPE,
	                           0,  NULL,
	                           MY_APP_ADV_CHANNEL,
	                           ADV_FP_NONE);
		} 
		else if(smp_param_inf[flash_dev_info.mast_id].peer_addr_type == 0) 
		{ //public addr
			//memcpy(bonded_peer_addr, (u8 *)&user_bond_inf[flash_dev_info.mast_id].con_bt_addr[0], 6);
			my_printf_aaa("channel %d bonded a public addr, direct\n", flash_dev_info.mast_id);
			ll_whiteList_reset(); 	  //clear whitelist
			ll_resolvingList_reset(); //clear resolving list
			if( IS_RESOLVABLE_PRIVATE_ADDR(smp_param_inf[flash_dev_info.mast_id].peer_addr_type, smp_param_inf[flash_dev_info.mast_id].peer_addr) )
			{
				ll_resolvingList_add(0, smp_param_inf[flash_dev_info.mast_id].peer_id_addr, smp_param_inf[flash_dev_info.mast_id].peer_irk, NULL);  //no local IRK
				ll_resolvingList_setAddrResolutionEnable(1);
				my_printf_aaa("random addr wL is resolvable priver\n");
        	}
			else
			{
				//if not resolvable random address, add peer address to whitelist
				ll_whiteList_add(smp_param_inf[flash_dev_info.mast_id].peer_addr_type, smp_param_inf[flash_dev_info.mast_id].peer_addr);
				my_printf_aaa("random addr wL is not resolvable priver\n");
			}

            bls_ll_setAdvParam(ADV_INTERVAL_3_75MS, ADV_INTERVAL_3_75MS,
                               ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, USER_OWN_ADDRESS_TYPE,
                               BLE_ADDR_PUBLIC, smp_param_inf[flash_dev_info.mast_id].peer_addr,
                               BLT_ENABLE_ADV_ALL,
                               ADV_FP_ALLOW_SCAN_WL_ALLOW_CONN_WL);
            bls_ll_setAdvDuration(4000000, DIRECT_ADV_TO_UNDIRECT_ENABLE);
		} 
		else if (smp_param_inf[flash_dev_info.mast_id].peer_addr_type == 1) 
		{//random addr
			my_printf_aaa("channel %d bonded a random addr, undirect\n", flash_dev_info.mast_id);
			ll_whiteList_reset(); 	  //clear whitelist
			ll_resolvingList_reset(); //clear resolving list
			if( IS_RESOLVABLE_PRIVATE_ADDR(smp_param_inf[flash_dev_info.mast_id].peer_addr_type, smp_param_inf[flash_dev_info.mast_id].peer_addr) )
			{
				ll_resolvingList_add(0, smp_param_inf[flash_dev_info.mast_id].peer_id_addr, smp_param_inf[flash_dev_info.mast_id].peer_irk, NULL);  //no local IRK
				ll_resolvingList_setAddrResolutionEnable(1);
				my_printf_aaa("random addr wL is resolvable priver\n");
        	}
			else
			{
				//if not resolvable random address, add peer address to whitelist
				ll_whiteList_add(smp_param_inf[flash_dev_info.mast_id].peer_addr_type, smp_param_inf[flash_dev_info.mast_id].peer_addr);
				my_printf_aaa("random addr wL is not resolvable priver\n");
			}
				
            tbl_advData_aaa[2] = 0x04;
            bls_ll_setAdvDuration(0, 0);
			bls_ll_setAdvParam(MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX, \
							ADV_TYPE_CONNECTABLE_UNDIRECTED, USER_OWN_ADDRESS_TYPE, \
							0,  NULL, MY_APP_ADV_CHANNEL, ADV_FP_ALLOW_SCAN_WL_ALLOW_CONN_WL);
        }
    }
    else
    {
		my_printf_aaa("normal pairing adv channel %d\n", flash_dev_info.mast_id);
        bls_ll_setAdvDuration(0, 0);
        bls_ll_setAdvParam(MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                           ADV_TYPE_CONNECTABLE_UNDIRECTED, USER_OWN_ADDRESS_TYPE,
                           0,  NULL,
                           MY_APP_ADV_CHANNEL,
                           ADV_FP_NONE);
    }
}


void ble_status_proc_aaa(u8 is_new_key_event)
{
    if(ble_status_aaa == T5S_CONNECTED_STATUS_AAA)
    {
	#if (BLE_CONNECT_ENTER_DEEPSLEEP_AFTER_TIME_OUT_ENABLE_AAA && (!BLE_SNIFF_DEBUG))
		if ((idle_count >= BLE_CONNECT_TIME_OUT))
		{
            my_printf_aaa("HCI_ERR_REMOTE_USER_TERM_CONN \r\n");
			bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
			ble_status_aaa = DEEP_TERMINATE_STATUS_AAA;
		}
	#endif

       connect_params_proc();
    }
    else if (ble_status_aaa == POWER_ON_STATUS_AAA)
    {
        	//bls_smp_enableParing (SMP_PARING_DISABLE_TRRIGER);
        	bls_ll_setAdvEnable(0);
		
	#if (BLE_SNIFF_DEBUG)
	        bls_ll_setAdvDuration(0, 0);
	        bls_ll_setAdvParam(MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
	                           ADV_TYPE_CONNECTABLE_UNDIRECTED, USER_OWN_ADDRESS_TYPE,
	                           0,  NULL,
	                           MY_APP_ADV_CHANNEL,
	                           ADV_FP_NONE);
	#else
	        Switch_Adv_Type = 0;
	        set_adv_type();
	#endif

        set_adv_scanRsp_data();
        bls_ll_setAdvEnable(1);  //adv enable
        // reset_idle_status();
        adv_begin_tick = clock_time() | 1;
        adv_count = 0;
        ble_status_aaa = LOW_ADV_STATUS_AAA;

    }
    else if (ble_status_aaa == LOW_ADV_STATUS_AAA)
    {
	#if (BLE_ADV_ENTER_DEEPSLEEP_AFTER_TIME_OUT_ENABLE_AAA&&(!BLE_SNIFF_DEBUG))
        adv_count_poll();

        if (pair_flag)
        {
            if (adv_count >= BLE_ADV_TIMER_OUT)
            {
                ble_status_aaa = DEEP_SLEEPE_STATUS_AAA;
            }
        }
        else if (adv_count >= 30)
        {
            ble_status_aaa = DEEP_SLEEPE_STATUS_AAA;
            //no need auto start pair
            //ble_start_pair();
        }
	#endif
	
	#if BLT_APP_LED_ENABLE
        led_ble_Adv_poll();
	#endif
	#if 0
        if (is_new_key_event && (pair_flag == 0))
        {
            ble_status_aaa = POWER_ON_STATUS_AAA;
        }
	#endif
    }
    else if (ble_status_aaa == BEGIN_CONNECTED_STATUS_AAA)
    {
        if (conn_step == BEGIN_CONNECT_AAA)
        {
            //clear_fifo();
            if (clock_time_exceed(connect_begin_tick, 40000000))
            {
                ble_status_aaa = OK_CONNECTED_STATUS_AAA;
            }
        }
		else if(conn_step == SMP_FIRST_CONNECT_DONE_AAA)
		{
			if(pair_success)
			{
				ble_status_aaa = OK_CONNECTED_STATUS_AAA;
				save_smp_inf();
			}
		}
        else if (conn_step == SMP_RECONNECT_DONE_AAA)
        {
            ble_status_aaa = OK_CONNECTED_STATUS_AAA;
        }
	 #if BLT_APP_LED_ENABLE
        led_ble_Adv_poll();
	 #endif
    }
    else if (ble_status_aaa == OK_CONNECTED_STATUS_AAA)
    {
    // bls_l2cap_requestConnParamUpdate(6, 9, 0x2c, 300);
        conn_step = CONNECTED_DONE_AAA;
        conn_params_tick = clock_time() | 1;
        ble_status_aaa = T5S_CONNECTED_STATUS_AAA;
		
	#if SHOW_MAST_REAL_MAC_DEBUG
        get_master_real_mac();
	#endif

	#if BLT_APP_LED_ENABLE
        led_ble_ConnectedStatus();
	#endif
    }
    else if (ble_status_aaa == DEEP_SLEEPE_STATUS_AAA)
    {
        if ((ui_ota_is_working == 0) && (!blc_ll_isControllerEventPending()))
        {
            printf("enter_deep_aaa---2\r\n");
            enter_deep_aaa();
        }
    }
}


void reset_idle_status()
{
    if (pair_flag)
    {
        return;
    }
    idle_count = 0;
    loop_cnt = 0;
    idle_tick = clock_time();
}

void idle_status_poll()
{
    u32 n;
    n = ((u32)(clock_time() - idle_tick)) / CLOCK_16M_SYS_TIMER_CLK_1S;

    idle_tick += n * CLOCK_16M_SYS_TIMER_CLK_1S;

    idle_count += n;
}
void adv_count_poll()
{
    u8 n;
    n = ((u32)(clock_time() - adv_begin_tick)) / CLOCK_16M_SYS_TIMER_CLK_1S;

    adv_begin_tick += n * CLOCK_16M_SYS_TIMER_CLK_1S;

    adv_count += n;
}

void ui_loop()
{
        get_ble_data_report_aaa();
        loop_cnt++;
        if (has_new_key_event)
        {
            reset_idle_status();
        }

        if (conn_step >= CONNECTED_DONE_AAA)
        {
            ble_notify_data_proc_aaa();
        }
        else
        {
            ms_data.wheel = 0;
        }
#if BLT_APP_LED_ENABLE
        device_led_process();
#endif
}

void ble_pm_aaa()
{
#if BLE_AUDIO_ENABLE
    if (ui_mic_enable || need_pop_internet)
    {
        bls_pm_setSuspendMask(SUSPEND_DISABLE);
        return;
    }
#endif

#if(BLT_SOFTWARE_TIMER_ENABLE==0)
    if (soft_time_flag)
    {
        bls_pm_setSuspendMask(SUSPEND_DISABLE);
        return;
    }
#endif

#if BLE_APP_PM_ENABLE
#if BLT_APP_LED_ENABLE
    if (0 || btn_value  || ui_ota_is_working || (loop_cnt < LONG_SUSPEND_TIMER_AAA))
#else
	if (0 || btn_value || ui_ota_is_working || (loop_cnt < LONG_SUSPEND_TIMER_AAA))
#endif
	{
        bls_pm_setManualLatency(0);
		bls_pm_setWakeupSource(0);

        if (suspend_wake_up_enable)
        {

            bls_pm_setWakeupSource(0);
#if WHEEL_FUN_ENABLE_AAA
            wheel_set_wakeup_level_suspend(0);
#endif
#if BUTTON_FUN_ENABLE_AAA
            btn_set_wakeup_level_suspend(0);
#endif
#if SENSOR_FUN_ENABLE_AAA
            sensor_set_wakeup_level_suspend(0);
#endif
        }
        suspend_wake_up_enable = 0;
    }
    else
    {
        loop_cnt = LONG_SUSPEND_TIMER_AAA + 1;
        bls_pm_setWakeupSource(PM_WAKEUP_PAD);
	
        if (suspend_wake_up_enable == 0)
        {
#if WHEEL_FUN_ENABLE_AAA
            wheel_set_wakeup_level_suspend(1);
#endif
#if BUTTON_FUN_ENABLE_AAA
            btn_set_wakeup_level_suspend(1);
#endif
#if SENSOR_FUN_ENABLE_AAA
            sensor_set_wakeup_level_suspend(1);
#endif
        }

        suspend_wake_up_enable = 1;
    }

#if (PM_DEEPSLEEP_RETENTION_ENABLE)
    if (suspend_wake_up_enable && (idle_count > 3))
    {
        bls_pm_setSuspendMask(SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
    }
    else
    {
        bls_pm_setSuspendMask(SUSPEND_ADV | SUSPEND_CONN);
    }
#else
    bls_pm_setSuspendMask(SUSPEND_ADV | SUSPEND_CONN);
#endif

#endif
}

/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////

void main_loop(void)
{
    static u8 fifo_num;

    fifo_num = blc_ll_getTxFifoNumber();
    ////////////////////////////////////// BLE entry /////////////////////////////////
    blt_sdk_main_loop();

    ////////////////////////////////////// UI entry /////////////////////////////////

    u16 time_interval = 4500;

    u16 interval = bls_ll_getConnectionInterval();
    if (interval > 0)
    {
        time_interval = interval * 1250;
    }
#if 1
    if (fifo_num > 10 && fifo_num < 15) {
        time_interval = interval * 1250 * + 2000;
    } else if (fifo_num >= 15 && fifo_num < 22){
        time_interval = interval * 1250 * + 5000;
    } else if (fifo_num >= 22 && fifo_num < 28){
        time_interval = interval * 1250 * + 7000;
    }else if (fifo_num > 28) {
        time_interval = interval * 1250 * + 10000;
    }
#endif
    if (clock_time_exceed(start_tick, time_interval))
    {
        start_tick = clock_time();
        ui_loop();
    }
    feature_sdle_test_mainloop();

    idle_status_poll();
    ble_status_proc_aaa(has_new_key_event);

    has_new_key_event = 0;
#if BLE_APP_PM_ENABLE
    ble_pm_aaa();
#else
    bls_pm_setSuspendMask(SUSPEND_DISABLE);
#endif
}


