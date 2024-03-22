/********************************************************************************************************
 * @file     aaa_main.c
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
_attribute_data_retention_user u8 report_rate=8;

_attribute_data_retention_user int dev_info_idx;

_attribute_data_retention_user OUTPUT_DEV_INFO_AAA output_dev_info =
{
    0,//u32 bin_crc;
    {3, 4, 2,1,4,15}, //u8
    0,//u8 sensor_type;
    0,//u8 sensor_pd1;
    0,//u8 sensor_pd2;
    0,//u8 sensor_pd3;
};
_attribute_data_retention_user u8 ic_inf[20];



_attribute_ram_code_ void irq_handler(void)
{
    if (fun_mode == RF_2M_2P4G_MODE)
    {
        irq_handle_private_2m();
    }
    else
    {
        irq_blt_sdk_handler();
    }
}

//len must 4 bytes bei beisu  beacuse *(volatile u32 *)(s_addr + idx) must 4 bytes bei beisu
int flash_info_load_aaa(u32 s_addr, u8 *d_addr,  int len)
{
    int idx;
    u32 buf;
    for (idx = 0; idx < (4096 - len); idx += len)
    {
        flash_read_page((u32)(s_addr + idx), 4, (u8 *)(&buf));
        if (buf == U32_MAX)
        {
            break;
        }
    }
    idx -= len;
    if (idx < 0) 		// no binding
    {
        return idx;
    }
    flash_read_page((u32)(s_addr + idx), len, d_addr);

    if (idx > 3000) 			//3k, erase flash
    {
        flash_erase_sector((u32)s_addr);

        sleep_us(10);

        flash_write_page((u32)s_addr, len, d_addr);
        idx = 0;
    }
    return idx;
}


void user_config()
{
    random_generator_init();  //this is must
    flash_read_page(CFG_ADR_MAC, sizeof(custom_cfg_t), (u8 *)&user_cfg.dev_mac);
    if (user_cfg.dev_mac == U32_MAX)
    {
        generateRandomNum(4, (u8 *) &user_cfg.dev_mac);
        flash_write_page(CFG_ADR_MAC, 4, (u8 *)&user_cfg.dev_mac);
    }


    if (user_cfg.paring_tx_power == U8_MAX)
    {
        user_cfg.paring_tx_power = DEFAULT_PAIR_TX_POWER;
    }
    if (user_cfg.tx_power == U8_MAX)
    {
        user_cfg.tx_power = DEFAULT_NORMAL_TX_POWER;
    }
    if (user_cfg.emi_tx_power == U8_MAX)
    {
        user_cfg.emi_tx_power = DEFAULT_EMI_TX_POWER;
    }


    if (user_cfg.sensor_direct == U8_MAX || user_cfg.sensor_direct > 3)
    {
        user_cfg.sensor_direct = SENSOR_DIRECTION_CLOCK_12;
    }
    else if (user_cfg.sensor_direct == 0)
    {
        user_cfg.sensor_direct = SENSOR_DIRECTION_CLOCK_3;
    }
    else if (user_cfg.sensor_direct == 1)
    {
        user_cfg.sensor_direct = SENSOR_DIRECTION_CLOCK_6;
    }
    else if (user_cfg.sensor_direct == 2)
    {
        user_cfg.sensor_direct = SENSOR_DIRECTION_CLOCK_9;
    }
    else if (user_cfg.sensor_direct == 3)
    {
        user_cfg.sensor_direct = SENSOR_DIRECTION_CLOCK_12;
    }

    if (user_cfg.mouse_type == U8_MAX) {
        user_cfg.mouse_type = MOUSE_TYPE;
    }

    if (user_cfg.pop_int == U8_MAX) {
        user_cfg.pop_int = 1;// start from 1
    }

    if (user_cfg.audio_reat == U8_MAX || user_cfg.audio_reat > 1) {
        user_cfg.audio_reat = 1;// start from 0
    }

    if(user_cfg.sn[0] == U8_MAX) {
        user_cfg.sn[0] = 0x4d;
        user_cfg.sn[1] = 0x4c;
        user_cfg.sn[2] = 0x20;
        user_cfg.sn[3] = 0x23;
        user_cfg.sn[4] = 0x04;
        user_cfg.sn[5] = 0x06;
        user_cfg.sn[6] = 0x01;
        user_cfg.sn[7] = 0x01;
        user_cfg.sn[8] = 0x00;
        user_cfg.sn[9] = 0x00;
    }

    for (u8 i = 0; i < 18; i++)
    {
        if (user_cfg.device_name[i] != 0xff)
        {
            device_name_len++;
        }
        else
        {
            break;
        }
    }


}

int deepRetWakeUp;

_attribute_ram_code_ int main(void)     //must run in ramcode
{
    blc_pm_select_internal_32k_crystal();

	#if(MCU_CORE_TYPE == MCU_CORE_825x)
		cpu_wakeup_init();
	#elif(MCU_CORE_TYPE == MCU_CORE_827x)
		cpu_wakeup_init(LDO_MODE,EXTERNAL_XTAL_24M);
	#endif

    deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp

    gpio_init(!deepRetWakeUp);    //analog resistance will keep available in deepSleep mode, so no need initialize again

    bls_ota_set_fwSize_and_fwBootAddr(124, 0x20000);

#if (CLOCK_SYS_CLOCK_HZ == 16000000)
    clock_init(SYS_CLK_16M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
    clock_init(SYS_CLK_24M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
	clock_init(SYS_CLK_32M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
	clock_init(SYS_CLK_48M_Crystal);
#endif

	if(!deepRetWakeUp){//read flash size
		blc_readFlashSize_autoConfigCustomFlashSector();
	}


	u32 offset=ota_program_bootAddr-ota_program_offset;
	u32 bin_size = 0;
	flash_read_page(offset+0x18, 4, (u8 *)&bin_size);
	flash_read_page(offset+(bin_size - 4), 4, (u8 *)&output_dev_info.bin_crc);
		

    //my_printf_aaa("deepRetWakeUp %d\r\n", deepRetWakeUp);
    if (!deepRetWakeUp)
    {
        deep_flag = analog_read(DEEP_ANA_REG0);
        ana_reg1_aaa = analog_read(USED_DEEP_ANA_REG1);
        pair_flag = ana_reg1_aaa & PAIR_ANA_FLG;

        user_config();

        dev_info_idx = flash_info_load_aaa(CFG_DEVICE_MODE_ADDR, (u8 *)&flash_dev_info.dongle_id, SAVE_MAX_IN_FLASH);
        if (dev_info_idx < 0)
        {
            flash_dev_info.mode = RF_2M_2P4G_MODE;
            flash_dev_info.mast_id = 0;
            flash_dev_info.slave_mac_addr[0] = 0;
            flash_dev_info.slave_mac_addr[1] = 0;
            flash_dev_info.slave_mac_addr[2] = 0;
            flash_dev_info.slave_mac_addr[3] = 0;
            flash_dev_info.dongle_id = 0;
            // 第一次上电使用user_cfg进行初始化
            flash_dev_info.sensor_direct = user_cfg.sensor_direct;
            flash_dev_info.mouse_type = user_cfg.mouse_type;
            flash_dev_info.pop_int = user_cfg.pop_int;
            flash_dev_info.audio_reat = user_cfg.audio_reat;
            memcpy(&flash_dev_info.host_set_sn[0], &user_cfg.sn[0], 10);
        }

        //blc_app_setExternalCrystalCapEnable(0);
        blc_app_loadCustomizedParameters();  //load customized freq_offset cap value

        ui_enable_mic(0);


#if (BLE_SNIFF_DEBUG)
			flash_dev_info.mode = RF_1M_BLE_MODE;
#endif
            //flash_dev_info.mode = RF_2M_2P4G_MODE;

#if (ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG==1)
			flash_dev_info.mode = RF_1M_BLE_MODE;
#endif	
			fun_mode = flash_dev_info.mode;
    }
    //fun_mode = RF_1M_BLE_MODE;

    hw_init();
	if (deep_flag == POWER_ON_ANA_AAA)
    {
        power_on_tick = clock_time() | 1;
#if EMI_TEST_FUN_ENABLE_AAA
        emi_process();
#endif
    }

#if (BATT_CHECK_ENABLE)  //battery check must do before OTA relative operation
    //user_batt_check_init();
#endif

    if (fun_mode == RF_1M_BLE_MODE)
    {
        varb_ADPCM_PACKET_LEN = ADPCM_PACKET_LEN;
        varb_TL_MIC_ADPCM_UNIT_SIZE = TL_MIC_ADPCM_UNIT_SIZE;
        varb_TL_MIC_BUFFER_SIZE = TL_MIC_BUFFER_SIZE;
        audio_reat_setting = flash_dev_info.audio_reat;

        rf_drv_init(RF_MODE_BLE_1M);
        if (deepRetWakeUp)
        {
            user_init_deepRetn();
        }
        else
        {
            printf("flash_dev_info.mast_id %d\r\n", flash_dev_info.mast_id);
            user_init_normal();
        }
    }
    else
    {
        varb_ADPCM_PACKET_LEN = P24G_ADPCM_PACKET_LEN;
        varb_TL_MIC_ADPCM_UNIT_SIZE = P24G_TL_MIC_ADPCM_UNIT_SIZE;
        varb_TL_MIC_BUFFER_SIZE = P24G_TL_MIC_BUFFER_SIZE;
        audio_reat_setting = flash_dev_info.audio_reat;

        rf_drv_private_2m_init();
        if (!deepRetWakeUp)
        {
            d24_user_init();
        }
    }
    blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);


	flash_read_mid_uid_with_check((u32 *)&ic_inf[0], (u8*)&ic_inf[4]);
	printf("8253m init ok\r\n");
    printf("******ota_program_offset = 0x%03x ******\r\n", ota_program_offset);
    irq_enable();
#if (APP_FLASH_LOCK_ENABLE) //flash lock en
	flash_lock_init();//flash lock init
#endif


    while (1)
    {
#if (MODULE_WATCHDOG_ENABLE)
        wd_clear();
#endif

#if DEBUG_TOGGLE_GPIO_ENABLE
        debug_loop_toggle();
#endif

#if (BLE_AUDIO_ENABLE)
        proc_audio();
        check_host_status();
#endif

        if (fun_mode == RF_1M_BLE_MODE)
        {
            main_loop();
        }
        else
        {
            d24_main_loop();
            #if D24G_OTA_ENABLE_AAA
             d24g_ota_loop();
            #endif
            mode_led_switch();
        }
#if (BATT_CHECK_ENABLE)
        user_batt_check_proc();
#endif

        last_btn_value = btn_value;
#if UART_PRINT_DEBUG_ENABLE
        _attribute_data_retention_user  static u32 tick = 0;
        if (clock_time_exceed(tick, 10000000))
        {
            tick = clock_time();
            my_printf_aaa("fifoNum=%x interval %d \r\n",  blc_ll_getTxFifoNumber(), bls_ll_getConnectionInterval());
            // my_printf_aaa("poll mode=%x,ble_status=%x,fifoNum=%x,interval=%x,latency=%x,timeout=%x\r\n", fun_mode, ble_status_aaa, blc_ll_getTxFifoNumber(), bls_ll_getConnectionInterval(), bls_ll_getConnectionLatency(), bls_ll_getConnectionTimeout());
        }
#endif
    }
}

