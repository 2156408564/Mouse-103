/********************************************************************************************************
 * @file     aaa_public.c
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


#define COMB_BTN_PAIR  (MS_BTN_LEFT|MS_BTN_RIGHT)

#if DOUBLE_CLICK_LEFT_FUN_ENABLE
    _attribute_data_retention_user static u32 double_click_left_tick;
    _attribute_data_retention_user static u8 has_double_click_cnt;
    _attribute_data_retention_user u8 has_double_click_left = 0;
    #define DOUBLE_CLICK_LEFT_INTERVAL  40000

#endif
u8 auto_draw_flag=0;
_attribute_data_retention_user  u8 switch_type=TWO_SPEED_SWITCH;
_attribute_data_retention_user  u8 active_disconnect_reason=0;

_attribute_data_retention_user  u32 power_on_tick = 0;
_attribute_data_retention_user  u8 device_name_len = 0;
_attribute_data_retention_user  u8 connect_ok = 0;
u8 connect_ok_flag = 0;


_attribute_data_retention_user custom_cfg_t   user_cfg;
_attribute_data_retention_user u8 fun_mode = RF_1M_BLE_MODE;
_attribute_data_retention_user FLASH_DEV_INFO_AAA flash_dev_info;


_attribute_data_retention_user u8 suspend_wake_up_enable = 1;
_attribute_data_retention_user u8 deep_flag = POWER_ON_ANA_AAA;
_attribute_data_retention_user u8 pair_flag = 0;
_attribute_data_retention_user u8 ana_reg1_aaa = 0;
_attribute_data_retention_user u8 has_been_paired_flag = 0;


_attribute_data_retention_user u8 has_new_report_aaa;
_attribute_data_retention_user u8 has_new_key_event = 0;
_attribute_data_retention_user u8 dpi_value = 1;


_attribute_data_retention_user u8 combination_flag = 0;

_attribute_data_retention_user u16 btn_value = 0;
_attribute_data_retention_user u16 last_btn_value = 0;
_attribute_data_retention_user u8 mouse_btn_in_sensor;
_attribute_data_retention_user mouse_data_t ms_data;
_attribute_data_retention_user mouse_data_t ms_buf;

_attribute_data_retention_user u8 wheel_pre = 0;
_attribute_data_retention_user u8 wheel_status;
extern _attribute_data_retention_ u8 need_batt_data_notify;

extern _attribute_data_retention_	u8 key_voice_press;
u8 host_connected = 0;
static volatile u32 double_click_tick;
static volatile u8 has_double_click_cnt;

void flash_write_page_user(unsigned long addr, unsigned long len, unsigned char *buf)
{

    u16 write_cnt = len / 8;
    u8 last_write_len = len % 8;

    for(int i = 0; i < write_cnt; i++) {
        flash_write_page((addr + 8 * i), 8, (buf + 8 * i));
        sleep_us(20);
    }

    if (last_write_len > 0)
        flash_write_page((addr + 8 * write_cnt), last_write_len, (buf + 8 * write_cnt));
}

void save_dev_info_flash()
{
    u8 read_data[SAVE_MAX_IN_FLASH];

#if (0)
    user_battery_power_check();
    //3200mv
    if (batt_vol_mv < (min_vol_mv + 50)) {
        return;
    }
#endif

    dev_info_idx += SAVE_MAX_IN_FLASH;
	if(dev_info_idx>4000)
	{
		flash_erase_sector(CFG_DEVICE_MODE_ADDR);
		dev_info_idx=0;	
	}

    for(int i = 0; i < 8; i++) {
        flash_write_page_user(CFG_DEVICE_MODE_ADDR + dev_info_idx, SAVE_MAX_IN_FLASH, (u8 *)&flash_dev_info.dongle_id);
        flash_read_page(CFG_DEVICE_MODE_ADDR + dev_info_idx, SAVE_MAX_IN_FLASH, read_data);
        if (memcmp((u8 *)&flash_dev_info.dongle_id, read_data, SAVE_MAX_IN_FLASH) == 0) {
            break;
        }
    }
}

#if BUTTON_FUN_ENABLE_AAA

_attribute_data_retention_user  u32 btn_tick = 0;

_attribute_data_retention_user 	u32 btn_pins[BTN_NUM_AAA] = BTN_MATRIX;

void btn_set_wakeup_level_suspend(u8 enable)
{
#if 0
    gpio_write(col_pins[0], 0);
    gpio_write(col_pins[1], 0);
    gpio_setup_up_down_resistor(col_pins[0], PM_PIN_PULLDOWN_100K);
    gpio_setup_up_down_resistor(col_pins[1], PM_PIN_PULLDOWN_100K);
    for (u8 i = 0; i < (sizeof(row_pins)/ sizeof(row_pins[0])); i++)
    {
        if (gpio_read(row_pins[i]))
        {
            cpu_set_gpio_wakeup(row_pins[i], 0, enable); //low wakeup suspend
        }
        else
        {
            cpu_set_gpio_wakeup(row_pins[i], 1, enable);
        }
    }
#endif
    gpio_write(PIN_BTN_OUT_VCC, 0);
    gpio_setup_up_down_resistor(PIN_BTN_OUT_VCC, PM_PIN_PULLDOWN_100K);

	for (u8 i = 0; i < BTN_NUM_AAA; i++)
    {
        if (gpio_read(btn_pins[i]))
            cpu_set_gpio_wakeup(btn_pins[i], 0, enable); //low wakeup suspend
        else
            cpu_set_gpio_wakeup(btn_pins[i], 1, enable);
    }
}


void btn_set_wakeup_level_deep(void)
{
#if 0
    gpio_write(col_pins[0], 0);
    gpio_write(col_pins[1], 0);
    gpio_setup_up_down_resistor(col_pins[0], PM_PIN_PULLDOWN_100K);
    gpio_setup_up_down_resistor(col_pins[1], PM_PIN_PULLDOWN_100K);
    for (u8 i = 0; i < (sizeof(row_pins)/sizeof(row_pins[0])); i++)
    {
        if (gpio_read(row_pins[i]))
        {
            cpu_set_gpio_wakeup(row_pins[i], 0, 1);
        }
        else
        {
            cpu_set_gpio_wakeup(row_pins[i], 1, 1);
        }
    }
    #endif

    gpio_write(PIN_BTN_OUT_VCC, 0);
    gpio_setup_up_down_resistor(PIN_BTN_OUT_VCC, PM_PIN_PULLDOWN_100K);

	for (u8 i = 0; i < BTN_NUM_AAA; i++)
    {
        if (gpio_read(btn_pins[i]))
            cpu_set_gpio_wakeup(btn_pins[i], 0, 1); //low wakeup suspend
        else
            cpu_set_gpio_wakeup(btn_pins[i], 1, 1);
    }
}

//_attribute_data_retention_user u32 row_pins[3] = ROW_PINS;
//_attribute_data_retention_user u32 col_pins[2] = COL_PINS;

static void btn_init_hw()
{
    #if 0
    gpio_set_func(col_pins[0], AS_GPIO);
    gpio_set_func(col_pins[0], AS_GPIO);
    gpio_set_output_en(col_pins[0], 1);
    gpio_set_output_en(col_pins[1], 1);
    gpio_set_input_en(col_pins[0],  0);
    gpio_set_input_en(col_pins[1],  0);
    gpio_write(col_pins[0], 0);
    gpio_write(col_pins[1], 0);

    for (u8 i = 0; i < (sizeof(row_pins)/sizeof(row_pins[0])); i++)
    {
        gpio_set_func(row_pins[i], AS_GPIO);
        gpio_set_output_en(row_pins[i], 0);
        gpio_set_input_en(row_pins[i],  1);
        gpio_write(row_pins[i], 1);
        gpio_setup_up_down_resistor(row_pins[i], PM_PIN_PULLUP_1M);
        cpu_set_gpio_wakeup(row_pins[i], 0, 1);
    }
    #endif
}

void ble_start_pair()
{
	set_pair_flag();
	if(connect_ok)
	{
		active_disconnect_reason=BLE_PAIR_REBOOT_ANA_AAA;
	}
	else
	{
		user_reboot(BLE_PAIR_REBOOT_ANA_AAA);
	}
//telink public
}

#if 0
void three_speed_switch_pair()
{
	if(fun_mode==RF_1M_BLE_MODE)
	{
		if ((((btn_value&COMB_BTN_PAIR)==COMB_BTN_PAIR)||((btn_value&MS_BTN_PAIR)==MS_BTN_PAIR)) && (clock_time_exceed(btn_tick, 2100000))) 
		{
			ble_start_pair();
		}
	}
	else 
	{
		if ((((btn_value&COMB_BTN_PAIR)==COMB_BTN_PAIR)) && (clock_time_exceed(btn_tick, 2100000))) 
		{
			d24_start_pair();
		}
	}
}
void two_speed_switch_pair()
{
	if(fun_mode==RF_1M_BLE_MODE)
	{
		if ((((btn_value&COMB_BTN_PAIR)==COMB_BTN_PAIR)||((btn_value&MS_BTN_MODE)==MS_BTN_MODE)) && (clock_time_exceed(btn_tick, 2100000))) 
		{
			ble_start_pair();
		}
	}
	else
	{
		if ((((btn_value&COMB_BTN_PAIR)==COMB_BTN_PAIR)) && (clock_time_exceed(btn_tick, 2100000))) 
		{
			d24_start_pair();
		}
	}
}
#endif

void two_speed_switch_pair()
{
	if (((btn_value&COMB_BTN_PAIR)==COMB_BTN_PAIR) && (clock_time_exceed(btn_tick, 2100000))) {

        if (connect_ok == 0 && fun_mode == RF_1M_BLE_MODE) {
			ble_start_pair();
		} else {
            if (connect_ok == 0) {
                d24_start_pair();
            }
		}

	}
}


void switch_mode_change_poll(u8 event_new)
{
#if 1
    static u8 test_mode;

	if (!gpio_read(PIN_BTN_MODE))
	{
		test_mode = RF_1M_BLE_MODE;
	} else {
        test_mode = RF_2M_2P4G_MODE;
    }

    if (test_mode != fun_mode) {
        clear_pair_flag();
        if (fun_mode == RF_1M_BLE_MODE)
        {
            printf("switch to 2.4 mode\r\n");
            //flash_dev_info.mast_id++;
            if(connect_ok)
            {
                    active_disconnect_reason = MODE_CHANGE_REBOOT_ANA_AAA;
            }
            else
            {
                    flash_dev_info.mode = RF_2M_2P4G_MODE;
                    save_dev_info_flash();
                    user_reboot(MODE_CHANGE_REBOOT_ANA_AAA);
            }
        }
        else
        {
                printf("switch to ble mode\r\n");
                flash_dev_info.mode = RF_1M_BLE_MODE;
                flash_dev_info.mast_id = 0;
                save_dev_info_flash();
                user_reboot(MODE_CHANGE_REBOOT_ANA_AAA);
        }
    }

#else
    // key btn switch mode
    _attribute_data_retention_ static u8 mode_btn_release_cnt = 0;
    u8 need_change_mode = 0;

    if ((last_btn_value & MS_BTN_MODE) && event_new)
    {
		mode_btn_release_cnt++;
	#if ENTER_PAIR_WHEN_NEVER_PAIRED_ENABLE
		if ((pair_flag == 1)&&has_been_paired_flag)
	#else
		if (pair_flag == 1)
	#endif
		{
			if(mode_btn_release_cnt >=1)
			{
				need_change_mode=1;
			}
        }
		else
		{
			need_change_mode=1;
		}
		if(need_change_mode)
		{
			clear_pair_flag();

			if (fun_mode == RF_1M_BLE_MODE)
            {
            	flash_dev_info.mast_id++;
              	if(connect_ok)
				{
					if (flash_dev_info.mast_id > 1) {
						active_disconnect_reason = MODE_CHANGE_REBOOT_ANA_AAA;
					} else {
						active_disconnect_reason = MUTI_DEVICE_REBOOT_ANA_AAA;
					}
				}
				else
				{
                    if (flash_dev_info.mast_id > 1) {
                        flash_dev_info.mode = RF_2M_2P4G_MODE;
                        save_dev_info_flash();
                        user_reboot(MODE_CHANGE_REBOOT_ANA_AAA);
					} else {
						clear_pair_flag();
						save_dev_info_flash();
						user_reboot(MUTI_DEVICE_REBOOT_ANA_AAA);
					}
				}
            }
            else
            {
                flash_dev_info.mode = RF_1M_BLE_MODE;
                flash_dev_info.mast_id = 0;
                save_dev_info_flash();
                user_reboot(MODE_CHANGE_REBOOT_ANA_AAA);
            }
    	}
	}
    #endif
}



void muti_device_change()
{
    if (fun_mode == RF_1M_BLE_MODE)
    {
        if (last_btn_value == MS_BTN_DEVICE_1)
        {
            if (flash_dev_info.mast_id != 0) //1
            {
                flash_dev_info.mast_id = 0;
                if (connect_ok)
                {
                    active_disconnect_reason=MUTI_DEVICE_REBOOT_ANA_AAA;
                }
                else
                {
					clear_pair_flag();
                    save_dev_info_flash();
                    user_reboot(MUTI_DEVICE_REBOOT_ANA_AAA);
                }
            }
        }
        else if (last_btn_value == MS_BTN_DEVICE_2)
        {
            if (flash_dev_info.mast_id != 1) //2
            {
                flash_dev_info.mast_id = 1;
                if (connect_ok)
                {
                    active_disconnect_reason=MUTI_DEVICE_REBOOT_ANA_AAA;
                }
                else
                {
                    clear_pair_flag();
                    save_dev_info_flash();
                    user_reboot(MUTI_DEVICE_REBOOT_ANA_AAA);
                }
            }
        }
		 else if (last_btn_value == MS_BTN_DEVICE_3)
        {
            if (flash_dev_info.mast_id != 2) //2
            {
                flash_dev_info.mast_id = 2;
                if (connect_ok)
                {
                    active_disconnect_reason=MUTI_DEVICE_REBOOT_ANA_AAA;
                }
                else
                {
                    clear_pair_flag();
                    save_dev_info_flash();
                    user_reboot(MUTI_DEVICE_REBOOT_ANA_AAA);
                }
            }
        }
        else if (last_btn_value == MS_BTN_DEVICE_4)
        {
            if (flash_dev_info.mast_id != 3) //2
            {
                flash_dev_info.mast_id = 3;
                if (connect_ok)
                {
                    active_disconnect_reason=MUTI_DEVICE_REBOOT_ANA_AAA;
                }
                else
                {
                    clear_pair_flag();
                    save_dev_info_flash();
                    user_reboot(MUTI_DEVICE_REBOOT_ANA_AAA);
                }
            }
        }
    }
}

void slave_push_order(void)
{
	if (connect_ok_flag && connect_ok)
	{
		u8 data[21];
		memset(data, 0, sizeof(data));
		data[0] = USER_DEFINE_CMD;
		data[1] = PROTOCOL_HEARD;
		data[2] = 0x04;
		data[3] = HOST_SLAVE_CONNECT;
		data[4] = PROTOCOL_TAIL;
		my_fifo_push(&fifo_km, data, sizeof(data));
		printf("push connect order\r\n");
		connect_ok_flag = 0;
	}
}
void button_process(u8 event_new)
{
#if 0 //for test
    if((last_btn_value & MS_BTN_K4) && event_new)
    {
		auto_draw_flag^=0x01;
    }

	if((last_btn_value & MS_BTN_K5) && event_new)
    {
		report_rate=(report_rate&0xf)>>1;
		if(report_rate==1)
		{
			report_rate=8;//125
		}
    }
#endif
    //-----------------cpi ------------------------
#if 0
    if ((last_btn_value & MS_BTN_CPI) && event_new)
    {
#if SENSOR_FUN_ENABLE_AAA
        if (connect_ok)
        {
            btn_dpi_set();
        }
#endif
    }
#endif

    if(pair_flag == 0)
    {
        two_speed_switch_pair();
    }

    switch_mode_change_poll(event_new);

    miclink_key_handle(event_new);
	slave_push_order();
}


#if BLE_AUDIO_ENABLE
bool audio_status = false;
void audio_key_handle(u8 key_value)
{
    if (((key_value&MS_BTN_SEARCH) == MS_BTN_SEARCH) && audio_status == false) {
        btn_tick = clock_time();
        audio_status = true;
        printf("voice start\r\n");
        key_voice_is_press();
    } else if  (((key_value&MS_BTN_SEARCH) == MS_BTN_SEARCH) && audio_status == true) {
        audio_status = false;
        printf("voice stop\r\n");
		key_voice_is_release();
    }
}

void audio_ctrl_by_btn(void)
{
    if (audio_status == false) {
        audio_status = true;
        printf("voice start\r\n");
        key_voice_is_press();
    } else if(audio_status == true) {
        audio_status = false;
        printf("voice start\r\n");
        key_voice_is_release();
    }
}

void audio_stop_by_btn(void)
{
    if (audio_status == true) {
        audio_status = false;
        printf("voice start\r\n");
        key_voice_is_release();
    }
}

#endif


static inline u8 button_get_status(u32 pin)
{
    u8 value = 0; //no button press

    if (!gpio_read(pin)) //connect gnd
    {
        value = 1;
    } else {
        gpio_setup_up_down_resistor(pin, PM_PIN_PULLDOWN_100K);
        gpio_write(pin, 0);
        sleep_us(10);
        if (gpio_read(pin))
        {
            value = 2;
        }
        gpio_write(pin, 1);
        gpio_setup_up_down_resistor(pin, PM_PIN_PULLUP_1M);
    }

    return value;
}
u16 btn_scan()
{
    u16 now_value = 0;
#if 0
    // test connect gnd
    if (!gpio_read(row_pins[2]))
    {
        now_value |= MS_BTN_SEARCH;
    }

    gpio_write(col_pins[0], 1);
    gpio_write(col_pins[1], 1);

    if (!gpio_read(row_pins[1]))
    {
        now_value |= MS_BTN_RIGHT;
    }
    if (!gpio_read(row_pins[0]))
    {
        now_value |= MS_BTN_LEFT;
    }

	/*****************test col_pins[]*************************/
    gpio_write(col_pins[0], 1);
    gpio_write(col_pins[1], 0);

	if (button_get_status(row_pins[0]) == 2) {
		now_value |= MS_BTN_K5;
	}

	if (button_get_status(row_pins[1]) == 2){
		now_value |= MS_BTN_MODE;
	}

    gpio_write(col_pins[0], 0);
    gpio_write(col_pins[1], 1);

	if (button_get_status(row_pins[0]) == 2) {
		now_value |= MS_BTN_K4;
	}

	if (button_get_status(row_pins[1]) == 2){
		now_value |= MS_BTN_MIDDLE;
	}

    gpio_write(col_pins[1], 0);
	/******************************************/
#endif

	if (!gpio_read(PIN_BTN_LEFT))
	{
		now_value |= MS_BTN_LEFT;
		printf("Left mouse button is pressed\n");
	}

#if 1
	/******************************************/
	gpio_write(PIN_BTN_OUT_VCC, 1);

	if (button_get_status(PIN_BTN_MIDDLE) == 1){
		now_value |= MS_BTN_MIDDLE;
	} else if (button_get_status(PIN_BTN_MIDDLE) == 2){
		now_value |= MS_BTN_K5;
		 printf("F5 key is pressed\n");
	}

	if (button_get_status(PIN_BTN_RIGHT) == 1){
		now_value |= MS_BTN_RIGHT;
		printf("Right mouse button is pressed\n");
	} else if (button_get_status(PIN_BTN_RIGHT) == 2){
		now_value |= MS_BTN_K4;
		 printf("F4 key is pressed\n");
	}

	gpio_write(PIN_BTN_OUT_VCC, 0);
#endif
	/******************************************/

    return  now_value;
}

u8 btn_get_value()
{
    u8 ret = 0;
    _attribute_data_retention_user  static u8 debounce = 0;
    u16 now_value = 0;
    static u8 long_press = 0;
    static u16 last_btn = 0;
    _attribute_data_retention_user static u16 last_value = 0;
    //static u32 self_msg_tick = 0;
	static u8 miclink_btn[20] = {USER_DEFINE_CMD, PROTOCOL_HEARD, 0x04, 0, PROTOCOL_TAIL};

    now_value = btn_scan();

    if (last_value != now_value)
    {
        reset_idle_status();
        debounce = 1;
        last_value = now_value;
    }

    if (debounce)
    {
        debounce++;
        if (debounce == 3)
        {
            ret = NEW_KEY_EVENT_AAA;
            btn_value = now_value;
            btn_tick = clock_time();
            my_printf_aaa("btn_value %x \r\n", btn_value);

            if (btn_value == MS_BTN_KEY_WRITE || btn_value == MS_BTN_KEY_TRANSLATOR || btn_value == MS_BTN_SEARCH)
            {
                has_double_click_cnt++;
                double_click_tick = clock_time();
				btn_long_internet_flag = 1;
                last_btn = btn_value;
				//my_printf_aaa("btn %1x\n", btn_value);
				//top key also send K4
				if (btn_value == MS_BTN_SEARCH) {
                    now_value = MS_BTN_K4;
                }
                //ms_data.btn = now_value & 0X1F;
            } else {
				if(btn_value == MS_BTN_MIDDLE){
					btn_long_internet_flag = 1;
				}else{
					btn_long_internet_flag = 0;
				}
                ms_data.btn = now_value & 0X1F;
			}
#if BLE_AUDIO_ENABLE
            //audio_key_handle(btn_value);
#endif
            debounce = 0;
        }
    }

    button_process(ret);

#if 1
    if (has_double_click_cnt && (btn_value == MS_BTN_RELEASE) \
        && long_press != BTN_LONG_DOWN \
        && clock_time_exceed(double_click_tick, 150*1000)) {
        double_click_tick = clock_time();
        if (has_double_click_cnt == 1) {
            if (last_btn == MS_BTN_KEY_WRITE) {
                my_printf_aaa("write single click\n");
                miclink_btn[3] = WRITE_KEY_SINGLE_CLICK;
            } else if (last_btn == MS_BTN_KEY_TRANSLATOR) {
                miclink_btn[3] = TRANSLATOR_KEY_SINGLE_CLICK;
                my_printf_aaa("translator single click\n");
            } else if (last_btn == MS_BTN_SEARCH) {
                my_printf_aaa("search single click\n");
                miclink_btn[3] = SEARCH_KEY_SINGLE_CLICK;
            }
            //self_msg_tick = clock_time();
            
            //my_printf_aaa("self_msg_tick  %x\n", self_msg_tick);
            if (miclink_btn[3] != 0) {
                my_printf_aaa("push 4\n");
                my_fifo_push(&fifo_km, miclink_btn, sizeof(miclink_btn));
                miclink_btn[3] = 0;
            }
			btn_long_internet_flag = 0;
            //host_connected = 0;
        }
        has_double_click_cnt = 0;
    }

    if ((btn_value  == MS_BTN_KEY_WRITE || btn_value  == MS_BTN_KEY_TRANSLATOR || btn_value  == MS_BTN_SEARCH) && \
        (long_press == BTN_LONG_RELEASE)\
        && (clock_time_exceed(btn_tick, 300*1000))) {
        long_press = BTN_LONG_DOWN;
        btn_tick = clock_time();
        if (btn_value ==  MS_BTN_KEY_WRITE) {
            audio_ctrl_by_btn();
            my_printf_aaa("write long press\n");
            miclink_btn[3] = WRITE_KEY_LONG_DOWN;
        } else if (btn_value ==  MS_BTN_KEY_TRANSLATOR) {
            my_printf_aaa("translator long press\n");
            miclink_btn[3] = TRANSLATOR_KEY_LONG_DOWN;
            audio_ctrl_by_btn();
        } else if (btn_value ==  MS_BTN_SEARCH) {
            my_printf_aaa("search long press\n");
            audio_ctrl_by_btn();
            miclink_btn[3] = SEARCH_KEY_LONG_DOWN;
        }
        //self_msg_tick = clock_time();
        if (miclink_btn[3] != 0) {
            my_printf_aaa("push 2\n");
            my_fifo_push(&fifo_km, miclink_btn, sizeof(miclink_btn));
            miclink_btn[3] = 0;
        }

        //host_connected = 0;
    } else if (long_press == BTN_LONG_DOWN \
        && btn_value == MS_BTN_RELEASE) {
       if (last_btn_value == MS_BTN_KEY_WRITE) {
            miclink_btn[3] = WRITE_KEY_LONG_UP;
            my_printf_aaa("write long up\n");
            audio_stop_by_btn();
       } else if (last_btn_value == MS_BTN_KEY_TRANSLATOR) {
           miclink_btn[3] = TRANSLATOR_KEY_LONG_UP;
           my_printf_aaa("translator long up\n");
           audio_stop_by_btn();
       } else if (last_btn_value == MS_BTN_SEARCH) {
           miclink_btn[3] = SEARCH_KEY_LONG_UP;
           audio_stop_by_btn();
           my_printf_aaa("search long up\n");
       }
        long_press = BTN_LONG_RELEASE;
	   	btn_long_internet_flag = 0;
        has_double_click_cnt = 0;
        //self_msg_tick = clock_time();
        if (miclink_btn[3] != 0) {
            my_printf_aaa("push 1\n");
            my_fifo_push(&fifo_km, miclink_btn, sizeof(miclink_btn));
            miclink_btn[3] = 0;
        }
        //host_connected = 0;
    }

#if 0
//no use back home
/////////////////////////////////////////////////////////////////////
    if (self_msg_tick != 0 && clock_time_exceed(self_msg_tick, 200000)) {
        if (host_connected == 0) {
            // KEY A need send back home
            if (miclink_btn[3] == VOICE_KEY_SINGLE_CLICK || miclink_btn[3] == VOICE_KEY_DOUBLE_CLICK || miclink_btn[3] == VOICE_KEY_LONG_DOWN || \
                miclink_btn[3] == VOICE_KEY_LONG_UP ) {
                if (miclink_btn[3] == VOICE_KEY_SINGLE_CLICK) {
                    //printf("send A, need back to home\n");
                    /*Not connected host , need back home*/
                    // TODO:add back home
                    //miclink_btn[3] = NEED_BACK_HOME;
                    //my_fifo_push(&fifo_km, miclink_btn, sizeof(miclink_btn));
                }
            } else {
                 // KEY B
            #if SENSOR_FUN_ENABLE_AAA
                if (connect_ok)
                    btn_dpi_set();
            #endif
            }
        }
        self_msg_tick = 0;
        miclink_btn[3] = 0;
    }
#endif
#endif
    return ret;
}
#endif

#if WHEEL_FUN_ENABLE_AAA
void wheel_set_wakeup_level_suspend(u8 enable)
{
#if 1

    if (gpio_read(PIN_WHEEL_1))
    {
        cpu_set_gpio_wakeup(PIN_WHEEL_1, 0, enable); //low wakeup suspend
        //connected_idle_time_count_ykq=0;
    }
    else
    {
        cpu_set_gpio_wakeup(PIN_WHEEL_1, 1, enable);
    }

    if (gpio_read(PIN_WHEEL_2))
    {
        cpu_set_gpio_wakeup(PIN_WHEEL_2, 0, enable); //low wakeup suspend
        //connected_idle_time_count_ykq=0;
    }
    else
    {
        cpu_set_gpio_wakeup(PIN_WHEEL_2, 1, enable);
    }
#endif
}

void wheel_set_wakeup_level_deep()
{

#if 1
    if (gpio_read(PIN_WHEEL_1))
    {

        cpu_set_gpio_wakeup(PIN_WHEEL_1, 0, 1);
    }
    else
    {
        cpu_set_gpio_wakeup(PIN_WHEEL_1, 1, 1);
    }

    if (gpio_read(PIN_WHEEL_2))
    {
        cpu_set_gpio_wakeup(PIN_WHEEL_2, 0, 1);
    }
    else
    {
        cpu_set_gpio_wakeup(PIN_WHEEL_2, 1, 1);
    }
#endif
}
static void wheel_init_hw()
{

    write_reg8(0xd2, WHEEL_ADDRES_D2); // different gpio different value
    write_reg8(0xd3, WHEEL_ADDRES_D3);


    write_reg8(0xd7, 0x01);  //BIT(0) 0: 1¸ñ        1:  2¸ñ      	BIT(1)  wakeup enable

    write_reg8(0xd1, 0x01);  //filter   00-07     00 is best


	reg_rst0 |= FLD_RST0_QDEC;   // for8258  power on 
	reg_rst0 &= (~FLD_RST0_QDEC);
    rc_32k_cal();
    BM_SET(reg_clk_en0, FLD_CLK0_QDEC_EN);
}

u32 mouse_wheel_prepare_tick(void)
{

    write_reg8(0xd8, 0x01);
    return clock_time();
}

_attribute_ram_code_ s8 mouse_wheel_process(u32 wheel_prepare_tick)
{
    s8 ret = 0;
    while (read_reg8(0xd8) & 0x01)
    {
#if (MODULE_WATCHDOG_ENABLE)
		wd_clear(); //clear watch dog
#endif
        if (clock_time_exceed(wheel_prepare_tick, 260))  //4 cylce is enough: 4*1/32k = 1/8 ms
        {
            write_reg8(0xd6, 0x01); //reset  d6[0]
            write_reg8(0xd6, 0x00);
            break;
        }
    }

#if 1//(WHEEL_TWO_STEP_PROC)
    _attribute_data_retention_user static signed char accumulate_wheel_cnt;
    _attribute_data_retention_user static signed char wheel_cnt;
    wheel_cnt = read_reg8(0xd0);

    wheel_cnt += accumulate_wheel_cnt;

    if (wheel_cnt & 1) //Ææ
    {
        accumulate_wheel_cnt = wheel_cnt > 0 ? 1 : -1;
    }
    else  //Å¼
    {
        accumulate_wheel_cnt = 0;
    }
    ret = (wheel_cnt / 2);
#else
    ret = read_reg8(0xd0);
#endif
    return ret;
}
#if PM_DEEPSLEEP_RETENTION_ENABLE
s8 wheel_vaule = 0;
_attribute_ram_code_ u8 wheel_get_value_1()
{
    u8 ret = 0;

    u8 wheel_now = 0;
    wheel_vaule = 0;

    if (!gpio_read(PIN_WHEEL_1)) //left
    {
        wheel_now |= 0x01;
    }
    if (!gpio_read(PIN_WHEEL_2)) //left
    {
        wheel_now |= 0x02;
    }
    if (wheel_now != wheel_pre)
    {
        wheel_pre = wheel_now;
        ret = WHEEL_DATA_EVENT_AAA;
        wheel_status = ((wheel_status << 2) & 0x3F) + wheel_now;
        if ((wheel_status == 0x07) || (wheel_status == 0x38))
        {
            wheel_vaule++;
        }
        else if ((wheel_status == 0x0b) || (wheel_status == 0x34))
        {
            wheel_vaule--;

        }

    }
    return ret;
}
#endif
_attribute_ram_code_ u8 wheel_get_value(u32 wheel_prepare_tick)
{
    u8 ret = 0;

    s8 wheel_now = 0;
#if PM_DEEPSLEEP_RETENTION_ENABLE

    ret = wheel_get_value_1();
#endif
    wheel_now = mouse_wheel_process(wheel_prepare_tick);

#if PM_DEEPSLEEP_RETENTION_ENABLE
    if (wheel_now == 0)
    {
        wheel_now = wheel_vaule;
    }
#endif

    if (user_cfg.wheel_direct != U8_MAX)
    {
        wheel_now = -wheel_now;
    }

    ms_data.wheel = wheel_now;
    if (wheel_now != 0)
    {
        ret = WHEEL_DATA_EVENT_AAA;
    }

    return ret;
}
#endif


void hw_init()
{
#if WHEEL_FUN_ENABLE_AAA
    wheel_init_hw();
#endif

#if BUTTON_FUN_ENABLE_AAA
    btn_init_hw();
#endif

#if SENSOR_FUN_ENABLE_AAA

    OPTSensor_Init(1);
    OPTSensor_Shutdown();
    output_dev_info.sensor_type = sensor_type;
    output_dev_info.sensor_pd1 = product_id1;
    output_dev_info.sensor_pd2 = product_id2;
    output_dev_info.sensor_pd3 = product_id3;
#endif
#if BLT_APP_LED_ENABLE
	led_hw_init();
#endif

}


void ble_notify_data_proc_aaa()
{
    u8 need_ms_notify = 0;
    mouse_data_t buf;
	u8 txFifoNumber;
	u8 status = 0;
    u8 slef_msg[21];

    memset(slef_msg, 0, sizeof(slef_msg));

	if (need_pop_internet) {
		if(need_win_R_internet){
			five_sec_win_r_internet();
		}else{
			five_sec_pop_internet();
		}
	}

	if (need_back_to_desktop) {
		back_to_desktop();
	}

    if ((my_fifo_get(&fifo_km)!=0) || ms_data.x || ms_data.y || ms_data.wheel)
    {
        loop_cnt = 0;

        if (my_fifo_get(&fifo_km)!=0)
        {
            u8 *p =  my_fifo_get(&fifo_km);
			if (p[0] == USER_DEFINE_CMD) {
                //user define cmd
                memcpy(slef_msg, &p[0], sizeof(slef_msg));
			} else if (p[0] == VOICE_CMD){
                //voice
            } else {
                //mouse date
                buf.btn = p[0];
            }
        }
        else
        {
            buf.btn = ms_data.btn;
        }

        buf.x = ms_data.x;
        buf.y = ms_data.y;
        buf.wheel = ms_data.wheel;
        need_ms_notify = 1;
    }

	if(active_disconnect_reason)
	{
		 buf.btn =0;
		 need_ms_notify = 1;
         blc_gatt_pushHandleValueNotify(BLS_CONN_HANDLE, HID_MOUSE_REPORT_INPUT_DP_H, &buf.btn, MOUSE_DATA_LEN_AAA);
		 bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
		 return;
    }

    txFifoNumber = blc_ll_getTxFifoNumber();
    if (need_ms_notify)
    {
		if (txFifoNumber < 32)
        {
			if (slef_msg[0] == USER_DEFINE_CMD) {
                /*KEYA or KEYB*/
				if (slef_msg[3] == NEED_BACK_HOME) {
					need_back_to_desktop = 1;
				} else {
					printf("self msg %1x \n", slef_msg[3]);
					status = blc_gatt_pushHandleValueNotify(BLS_CONN_HANDLE, HID_ML_REPORT_INPUT_CMD_DP_H, &slef_msg[1], sizeof(slef_msg) - 1);
				}
			} /*else if (slef_msg[0] == VOICE_CMD) {
                voice data never through there
            }*/else {
                /* ble mouse data */
	            status = blc_gatt_pushHandleValueNotify(BLS_CONN_HANDLE, HID_MOUSE_REPORT_INPUT_DP_H, &buf.btn, MOUSE_DATA_LEN_AAA);
			}
            if (status == BLE_SUCCESS)
            {
                if (my_fifo_get(&fifo_km)!=0)
                    my_fifo_pop(&fifo_km);
                ms_data.wheel = 0;
            }
            else
            {
                printf("notify_fail_reason=%x\r\n", status);
            }
        }

    }
#if BATT_CHECK_ENABLE
    else if (need_batt_data_notify)
    {
        if (txFifoNumber < 9)
        {
            status = blc_gatt_pushHandleValueNotify(BLS_CONN_HANDLE, BATT_LEVEL_INPUT_DP_H, my_batVal, 1);
            if (status == BLE_SUCCESS)
            {
                need_batt_data_notify = 0;
            }
        }
    }
#endif

}




void mouse_xy_multiple()
{
#if MUTI_SENSOR_ENABLE
    s32 x = 0, y = 0;
    if (xy_multiple_flag == MULTIPIPE_1_DOT_5)
    {
        x = ms_data.x;
        y = ms_data.y;
        x = (x * 3) / 2;
        y = (y * 3) / 2;
        ms_data.x = x;
        ms_data.y = y;
    }else if(xy_multiple_flag == MULTIPIPE_2_DOT_0)
    {
		x = ms_data.x;
        y = ms_data.y;
        x = (x * 4) / 2;
        y = (y * 4) / 2;
        ms_data.x = x;
        ms_data.y = y;
	}else if(xy_multiple_flag == MULTIPIPE_2_DOT_5)
	{
		x = ms_data.x;
        y = ms_data.y;
        x = (x * 5) / 2;
        y = (y * 5) / 2;
        ms_data.x = x;
        ms_data.y = y;
	}
#endif
}

void mouse_task_when_rf()
{
#if SENSOR_FUN_ENABLE_AAA
    if (OPTSensor_motion_report(0))
    {
		if(ms_data.x||ms_data.y)
		{
        has_new_key_event |= SENSOR_DATA_EVENT_AAA;

        mouse_xy_multiple();
        check_sensor_dircet(user_cfg.sensor_direct);
        adaptive_smoother();
		}
    }
    else
    {
        ms_data.x = 0;
        ms_data.y = 0;
    }
#endif
    has_new_key_event |= Draw_a_square_test();
}

u8  Draw_a_square_test(void)
{
    _attribute_data_retention_user static int x = 0;
    _attribute_data_retention_user static u8 flag = 0;
    int step = 4;
   if(auto_draw_flag==0)
   {
   		return 0;
   }
    x++;
    if (x >= 50)
    {
        x = 0;
        flag++;
        if (flag > 3)
        {
            flag = 0;
        }

    }
    if (flag == 0)
    {
        ms_data.x = step;
        ms_data.y = 0;
    }
    else if (flag == 1)
    {
        ms_data.x = 0;
        ms_data.y = step;
    }
    else if (flag == 2)
    {
        ms_data.x = -step;
        ms_data.y = 0;
    }
    else if (flag == 3)
    {
        ms_data.x = 0;
        ms_data.y = -step;
    }
    return SENSOR_DATA_EVENT_AAA;
}

void clear_pair_flag()
{
    pair_flag = 0;
    analog_write(USED_DEEP_ANA_REG1, ana_reg1_aaa & CLEAR_PAIR_ANA_FLAG);
}

void set_pair_flag()
{
    pair_flag = 1;
    analog_write(USED_DEEP_ANA_REG1, ana_reg1_aaa | PAIR_ANA_FLG);
}

void write_deep_ana0(u8 buf)
{
    deep_flag = buf;
    analog_write(DEEP_ANA_REG0, buf);
}

void user_reboot(u8 reason)
{
    write_deep_ana0(reason);
    start_reboot();
}



u8 get_ble_data_report_aaa(void)
{
#if WHEEL_FUN_ENABLE_AAA
    u32 wheel_prepare_tick = mouse_wheel_prepare_tick();
#endif

#if BUTTON_FUN_ENABLE_AAA
    has_new_key_event |= btn_get_value();
#endif

#if SENSOR_FUN_ENABLE_AAA
    if (OPTSensor_motion_report(0))
    {
        has_new_key_event |= SENSOR_DATA_EVENT_AAA;
        mouse_xy_multiple();
        check_sensor_dircet(user_cfg.sensor_direct);
        adaptive_smoother();
    }
    else
    {
        ms_data.x = 0;
        ms_data.y = 0;
    }
#endif

#if TEST_DRAW_A_SQUARE
        if ((blc_ll_getCurrentState() == BLS_LINK_STATE_CONN) && ((ble_status_aaa == T5S_CONNECTED_STATUS_AAA)))
        {
            has_new_key_event |= Draw_a_square_test();
        }
#endif

#if WHEEL_FUN_ENABLE_AAA
    has_new_key_event |= wheel_get_value(wheel_prepare_tick);
#endif
    if (has_new_key_event & (NEW_KEY_EVENT_AAA))
    {
        has_new_report_aaa |= HAS_MOUSE_REPORT;
		my_fifo_push(&fifo_km, &ms_data.btn, sizeof(mouse_data_t));
    }
    return has_new_key_event;

}

void get_24g_data_report_aaa()
{
#if WHEEL_FUN_ENABLE_AAA
    u32 wheel_prepare_tick;
    wheel_prepare_tick = mouse_wheel_prepare_tick();
#endif


#if BUTTON_FUN_ENABLE_AAA
    has_new_key_event |= btn_get_value();
#endif

#if WHEEL_FUN_ENABLE_AAA
    has_new_key_event |= wheel_get_value(wheel_prepare_tick);
#endif
}

#if (APP_FLASH_LOCK_ENABLE)

_attribute_data_retention_ unsigned int mid;//= flash_read_mid();
_attribute_ram_code_ void flash_lock_init(void)
{

    printf("flash_lock_init\r\n");

	mid = flash_read_mid();

	printf("mid flash:%x\r\n",mid);

	switch(mid)
	{
	/*case 0x1160c8:
        if (0 == (flash_read_status_mid1160c8() & FLASH_WRITE_STATUS_BP_MID1160C8))
            flash_lock_mid1160c8(FLASH_LOCK_LOW_64K_MID1160C8);
		break;*/
	case 0x1360c8:
        if (0 ==  (flash_read_status_mid1360c8() & FLASH_WRITE_STATUS_BP_MID1360C8));
            flash_lock_mid1360c8(FLASH_LOCK_LOW_256K_MID1360C8);
		break;
	case 0x1460c8:
        if (0 == (flash_read_status_mid1460c8() & FLASH_WRITE_STATUS_BP_MID1460C8))
            flash_lock_mid1460c8(FLASH_LOCK_LOW_768K_MID1460C8);
		break;
	/*case 0x11325e:
        if (0 == (flash_read_status_mid11325e() & FLASH_WRITE_STATUS_BP_MID11325E))
            flash_lock_mid11325e(FLASH_LOCK_LOW_64K_MID11325E);
		break;*/
	case 0x13325e:
        if (0 == (flash_read_status_mid13325e() & FLASH_WRITE_STATUS_BP_MID13325E))
            flash_lock_mid13325e(FLASH_LOCK_LOW_256K_MID13325E);
		break;
	case 0x14325e:
        if (0 == (flash_read_status_mid14325e() & FLASH_WRITE_STATUS_BP_MID14325E))
            flash_lock_mid14325e(FLASH_LOCK_LOW_768K_MID14325E);
		break;
	default:
		break;
	}
}


_attribute_ram_code_ void flash_unlock_init(void)
{

	switch(mid)
	{
	/*case 0x1160c8:
        if (0 != (flash_read_status_mid1160c8() & FLASH_WRITE_STATUS_BP_MID1160C8))
            flash_unlock_mid1160c8();
		break;*/
	case 0x1360c8:
        if (0 !=  (flash_read_status_mid1360c8() & FLASH_WRITE_STATUS_BP_MID1360C8));
            flash_unlock_mid1360c8();
		break;
	case 0x1460c8:
        if (0 != (flash_read_status_mid1460c8() & FLASH_WRITE_STATUS_BP_MID1460C8))
            flash_unlock_mid1460c8();
		break;
	/*case 0x11325e:
        if (0 != (flash_read_status_mid11325e() & FLASH_WRITE_STATUS_BP_MID11325E))
            flash_unlock_mid11325e();
		break;*/
	case 0x13325e:
        if (0 != (flash_read_status_mid13325e() & FLASH_WRITE_STATUS_BP_MID13325E))
            flash_unlock_mid13325e();
		break;
	case 0x14325e:
        if (0 != (flash_read_status_mid14325e() & FLASH_WRITE_STATUS_BP_MID14325E))
            flash_unlock_mid14325e();
		break;
	default:
		break;
	}
}
#endif


