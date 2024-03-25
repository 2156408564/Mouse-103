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
    u8 value = 0; //no button press	没有按键按下

    if (!gpio_read(pin)) //connect gnd		引脚处于低电平状态
    {
        value = 1;
    } else {
        gpio_setup_up_down_resistor(pin, PM_PIN_PULLDOWN_100K);//确保引脚在未连接时保持低电平状态
        gpio_write(pin, 0);//输出低电平
        sleep_us(10);//等待引脚状态稳定。
        if (gpio_read(pin))//如果引脚处于高电平状态
        {
            value = 2;
        }
        gpio_write(pin, 1);//输出高电平
        gpio_setup_up_down_resistor(pin, PM_PIN_PULLUP_1M);//确保引脚在未连接时，处于高电平状态
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

	if (!gpio_read(PIN_BTN_LEFT))// 检测PIN_BTN_LEFT 的 GPIO 引脚是否处于低电平状态。
	{
		now_value |= MS_BTN_K5;
	}

#if 1					//条件始终为真，包含在其中的代码将被编译。
	/******************************************/
	gpio_write(PIN_BTN_OUT_VCC, 1);//输出高电平

	if (button_get_status(PIN_BTN_MIDDLE) == 1){
		now_value |= MS_BTN_MIDDLE;
	} else if (button_get_status(PIN_BTN_MIDDLE) == 2){
		now_value |= MS_BTN_LEFT;
	}

	if (button_get_status(PIN_BTN_RIGHT) == 1){
		now_value |= MS_BTN_K4;
	} else if (button_get_status(PIN_BTN_RIGHT) == 2){
		now_value |=MS_BTN_RIGHT;
	}

	gpio_write(PIN_BTN_OUT_VCC, 0);//输出低电平
#endif
	/******************************************/

    return  now_value;
}
//用于获取按钮的状态
u8 btn_get_value()
{
    u8 ret = 0;
    _attribute_data_retention_user  static u8 debounce = 0;//debounce 消抖处理
    u16 now_value = 0;// 存储当前按钮的状态值
    static u8 long_press = 0;//用于记录按钮是否为长按状态。	 
    static u16 last_btn = 0;//记录上一个按钮的状态值
    _attribute_data_retention_user static u16 last_value = 0;//记录上一次按钮状态的值，以便与当前值进行比较，判断是否有按键事件发生
    //static u32 self_msg_tick = 0;
	static u8 miclink_btn[20] = {USER_DEFINE_CMD, PROTOCOL_HEARD, 0x04, 0, PROTOCOL_TAIL};

    now_value = btn_scan();
/*在检测到按钮状态发生改变时，执行了重置空闲状态、消抖处理和记录当前按钮状态值的操作。*/
    if (last_value != now_value)
    {
        reset_idle_status();
        debounce = 1;
        last_value = now_value;
    }

    if (debounce)//按钮按下事件经过了消抖处理
    {
        debounce++;//递增消抖计数器
        if (debounce == 3)//确认按钮按下事件有效
        {
            ret = NEW_KEY_EVENT_AAA;//新的按键事件发生
            btn_value = now_value;// 记录当前按钮的状态值
            btn_tick = clock_time();// 记录按键事件发生时的时间戳
            my_printf_aaa("btn_value %x \r\n", btn_value);//将当前按键的指打印出来
				/*在特定按钮值的情况下，执行一系列的操作，
				包括增加双击计数、记录双击时间戳、设置长按互联网标志以及记录上一个按钮值。*/
            if (btn_value == MS_BTN_KEY_WRITE || btn_value == MS_BTN_KEY_TRANSLATOR || btn_value == MS_BTN_SEARCH)
            {
                has_double_click_cnt++;
                double_click_tick = clock_time();
				btn_long_internet_flag = 1;
                last_btn = btn_value;//记录上一次的按钮值
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
/*根据WHEEL_1和WHEEL_2引脚的状态，设置它们的唤醒方式为低电平或高电平，并可以选择是否启用唤醒功能。*/
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
        cpu_set_gpio_wakeup(PIN_WHEEL_1, 1, enable);//高电平唤醒
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
/*对一些寄存器进行写入操作，设置了轮子相关硬件的一些参数和初始化操作*/
static void wheel_init_hw()
{

    write_reg8(0xd2, WHEEL_ADDRES_D2); // different gpio different value	向地址为0xD2的寄存器写入WHEEL_ADDRES_D2的值
    write_reg8(0xd3, WHEEL_ADDRES_D3);//向地址为0xD3的寄存器写入WHEEL_ADDRES_D3的值。
	 


    write_reg8(0xd7, 0x01);  //BIT(0) 0: 1¸ñ        1:  2¸ñ      	BIT(1)  wakeup enable

    write_reg8(0xd1, 0x01);  //filter   00-07     00 is best


	reg_rst0 |= FLD_RST0_QDEC;   // for8258  power on 对reg_rst0寄存器执行按位或操作，将FLD_RST0_QDEC位置1，用于8258的上电。
	reg_rst0 &= (~FLD_RST0_QDEC);//对reg_rst0寄存器执行按位与操作，将FLD_RST0_QDEC位清零，取消QDEC的复位状态。
    rc_32k_cal();//执行RC 32K校准的操作。
    BM_SET(reg_clk_en0, FLD_CLK0_QDEC_EN);//设置reg_clk_en0寄存器中FLD_CLK0_QDEC_EN位，使能QDEC时钟。
	
}
/*在进行鼠标滚轮操作之前，首先对硬件进行一些准备工作，并返回当前的时间戳*/
u32 mouse_wheel_prepare_tick(void)
{

    write_reg8(0xd8, 0x01);
    return clock_time();//调用 clock_time 函数来获取当前的时间戳，并将其作为函数的返回值返回给调用者。
}
/*对鼠标滚轮进行处理，并返回处理后的结果。同时，在处理过程中会根据一定的条件来执行特定的操作。*/
//函数接受一个参数 wheel_prepare_tick，处理鼠标滚轮的时间戳。
_attribute_ram_code_ s8 mouse_wheel_process(u32 wheel_prepare_tick)
{
    s8 ret = 0;
    while (read_reg8(0xd8) & 0x01)
    {
#if (MODULE_WATCHDOG_ENABLE)
		wd_clear(); //clear watch dog
#endif
			//检查时间是否超过了 260 个时钟周期（约为 1/8 毫秒），如果超过则执行一系列操作，并跳出循环。
        if (clock_time_exceed(wheel_prepare_tick, 260))  //4 cylce is enough: 4*1/32k = 1/8 ms
        {
            write_reg8(0xd6, 0x01); //reset  d6[0]
            write_reg8(0xd6, 0x00);
            break;
        }
    }
//根据宏定义来选择不同的鼠标滚轮处理方式，并返回相应的结果。
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
//读取两个引脚（PIN_WHEEL_1 和 PIN_WHEEL_2）的状态,分别表示鼠标滚轮向左或向右滚动的情况，并将读取的状态值存储在变量 wheel_now 中。
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

//处理蓝牙通知数据的逻辑
void ble_notify_data_proc_aaa()
{
    u8 need_ms_notify = 0;
    mouse_data_t buf;
	u8 txFifoNumber;
	u8 status = 0;
    u8 slef_msg[21];

    memset(slef_msg, 0, sizeof(slef_msg));//使用 memset 函数将 slef_msg 数组清零
//根据条件判断，如果需要弹出互联网窗口 (need_pop_internet)，则根据是否需要回到桌面（need_back_to_desktop），调用相应的函数 five_sec_win_r_internet() 或 five_sec_pop_internet()。
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
	//判断条件：如果键鼠 FIFO 非空、鼠标数据中的 x、y 或滚轮数据不为零，则进入条件判断。
    if ((my_fifo_get(&fifo_km)!=0) || ms_data.x || ms_data.y || ms_data.wheel)
    {
        loop_cnt = 0;//将 loop_cnt 置零。
			//如果键鼠 FIFO 非空，获取 FIFO 中的数据并进行处理：

/*如果数据的第一个字节是用户定义的命令（USER_DEFINE_CMD），则将数据拷贝到 slef_msg 数组中。如果数据的第一个
字节是语音命令（VOICE_CMD），则暂时不做处理。否则，表示是鼠标数据，将数据的第一个字节保存为按钮状态 buf.btn。*/
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
            buf.btn = ms_data.btn;//如果键鼠 FIFO 为空，则将鼠标数据中的按钮状态赋给 buf.btn。
        }
		//将鼠标数据中的 x、y 和滚轮数据分别赋给 buf.x、buf.y、buf.wheel。
        buf.x = ms_data.x;
        buf.y = ms_data.y;
        buf.wheel = ms_data.wheel;
        need_ms_notify = 1;
    }

	if(active_disconnect_reason)
	{
		 buf.btn =0;//将 buf.btn 置为 0。
		 need_ms_notify = 1;//将 need_ms_notify 设置为 1，表示需要通知蓝牙发送数据。
    txFifoNumber = blc_ll_getTxFifoNumber();//获取当前可用的传输 FIFO 数量 txFifoNumber。
    /*如果需要进行蓝牙通知（need_ms_notify 为真），并且当前可用的传输 FIFO 数量小于 32*/
    if (need_ms_notify)
    {
		if (txFifoNumber < 32)
        {
        /*如果 slef_msg 中的第一个字节是用户定义命令（USER_DEFINE_CMD），则根据具体命令做相应处理：
        如果是需要返回桌面的命令（NEED_BACK_HOME），则将 need_back_to_desktop 置为 1；
        否则，调用 blc_gatt_pushHandleValueNotify 函数通知蓝牙发送数据。*/
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
            }*/else {	//如果 slef_msg 中的第一个字节不是用户定义命令，则将鼠标数据作为 BLE 鼠标数据发送给蓝牙。
                /* ble mouse data */
	            status = blc_gatt_pushHandleValueNotify(BLS_CONN_HANDLE, HID_MOUSE_REPORT_INPUT_DP_H, &buf.btn, MOUSE_DATA_LEN_AAA);
			}
				/*根据通知的状态 status 判断通知是否发送成功：*/
//如果通知发送成功（status == BLE_SUCCESS），则从键鼠 FIFO 中弹出数据，并将鼠标滚轮数据清零。
            if (status == BLE_SUCCESS)
            {
                if (my_fifo_get(&fifo_km)!=0)
                    my_fifo_pop(&fifo_km);
                ms_data.wheel = 0;
            }
				//如果通知发送失败，则打印通知失败的原因。
            else
            {
                printf("notify_fail_reason=%x\r\n", status);
            }
        }

    } 
	 //在发送完蓝牙通知后，立即终止当前蓝牙连接，并退出当前函数。
		blc_gatt_pushHandleValueNotify(BLS_CONN_HANDLE, HID_MOUSE_REPORT_INPUT_DP_H, &buf.btn, MOUSE_DATA_LEN_AAA);
		 bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);//导致蓝牙连接被远程用户终止。
		 return;
    }
//在处理电池信息通知的逻辑，针对电池数据进行通知给蓝牙连接的设备
//首先检查 BATT_CHECK_ENABLE 宏是否定义，如果定义了，则继续执行下面的逻辑。
#if BATT_CHECK_ENABLE
//如果需要发送电池数据通知（need_batt_data_notify 为真）
    else if (need_batt_data_notify)
    {
    //当前可用的传输 FIFO 数量小于 9
        if (txFifoNumber < 9)
        {
        //调用 blc_gatt_pushHandleValueNotify 函数，向指定的连接句柄（BLS_CONN_HANDLE）发送电池电量数据（my_batVal）到 BATT_LEVEL_INPUT_DP_H 属性，数据长度为 1。
            status = blc_gatt_pushHandleValueNotify(BLS_CONN_HANDLE, BATT_LEVEL_INPUT_DP_H, my_batVal, 1);
				//如果通知发送成功（status == BLE_SUCCESS），则将 need_batt_data_notify 置为 0，表示电池数据已成功发送。
				if (status == BLE_SUCCESS)
            {
                need_batt_data_notify = 0;
            }
        }
    }
#endif

}



//处理鼠标的 XY 坐标倍增逻辑
void mouse_xy_multiple()
{
//首先检查 MUTI_SENSOR_ENABLE 宏是否定义，如果定义了，则继续执行下面的逻辑。
#if MUTI_SENSOR_ENABLE
//定义了两个整型变量 x 和 y，用于存储鼠标的 X 和 Y 坐标。
    s32 x = 0, y = 0;
//如果 xy_multiple_flag 等于 MULTIPIPE_1_DOT_5，则将鼠标的 X 和 Y 坐标分别乘以 3/2。
    if (xy_multiple_flag == MULTIPIPE_1_DOT_5)
    {
        x = ms_data.x;
        y = ms_data.y;
        x = (x * 3) / 2;
        y = (y * 3) / 2;
        ms_data.x = x;
        ms_data.y = y;
	//如果 xy_multiple_flag 等于 MULTIPIPE_2_DOT_0，则将鼠标的 X 和 Y 坐标分别乘以 4/2（即乘以 2）。
    }else if(xy_multiple_flag == MULTIPIPE_2_DOT_0)
    {
		x = ms_data.x;
        y = ms_data.y;
        x = (x * 4) / 2;
        y = (y * 4) / 2;
        ms_data.x = x;
        ms_data.y = y;
//如果 xy_multiple_flag 等于 MULTIPIPE_2_DOT_5，则将鼠标的 X 和 Y 坐标分别乘以 5/2。
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
//在 RF 模式下处理鼠标任务
void mouse_task_when_rf()
{
//首先检查 SENSOR_FUN_ENABLE_AAA 宏是否定义，如果定义了，则继续执行下面的逻辑。
#if SENSOR_FUN_ENABLE_AAA
//在条件判断中调用 OPTSensor_motion_report(0) 函数，如果返回真（非零）
    if (OPTSensor_motion_report(0))
    {
    //检查 ms_data 结构体中的 X 或 Y 坐标是否不为零
		if(ms_data.x||ms_data.y)
		{
        has_new_key_event |= SENSOR_DATA_EVENT_AAA;//将 has_new_key_event 标志位按位或上 SENSOR_DATA_EVENT_AAA，表示发生了传感器数据事件 AAA。

        mouse_xy_multiple();//对鼠标的 X 和 Y 坐标进行倍增操作。
        check_sensor_dircet(user_cfg.sensor_direct);//检查传感器方向并进行处理。
        adaptive_smoother();//进行自适应平滑处理。
		}
    }
	 //如果 OPTSensor_motion_report(0) 返回假（零），则将 ms_data 结构体中的 X 和 Y 坐标都设置为 0。
    else
    {
        ms_data.x = 0;
        ms_data.y = 0;
    }
#endif
//将 Draw_a_square_test() 的返回值与 has_new_key_event 进行按位或运算，并将结果赋值给 has_new_key_event。
    has_new_key_event |= Draw_a_square_test();
}
//模拟绘制一个正方形的过程，通过控制 flag 和 step 来改变鼠标数据的 x 和 y 值，最终返回一个表示传感器数据事件的标志。
u8  Draw_a_square_test(void)
{
//在函数内部定义了两个静态变量 x 和 flag，分别用于记录状态和计数。其中，x 用于控制计数，flag 用于标记当前状态。
    _attribute_data_retention_user static int x = 0;
    _attribute_data_retention_user static u8 flag = 0;
    int step = 4;//初始化变量 step 为 4。
    //如果 auto_draw_flag 等于 0，则直接返回 0，不执行后续逻辑。
   if(auto_draw_flag==0)
   {
   		return 0;
   }
	//每次函数调用，x 自增 1。
    x++;
	// x 大于等于 50 
    if (x >= 50)
    {
        x = 0;//将 x 重置为 0
        flag++;//增加 flag 的值
		  //如果 flag 大于 
        if (flag > 3)
        {
            flag = 0;//将其重置为 0
        }

    }
	 //当 flag 为 0 时，将 x 设为 step，y 设为 0。
    if (flag == 0)
    {
        ms_data.x = step;
        ms_data.y = 0;
    }
	 //当 flag 为 1 时，将 x 设为 0，y 设为 step。
    else if (flag == 1)
    {
        ms_data.x = 0;
        ms_data.y = step;
    }
	 //当 flag 为 2 时，将 x 设为 -step，y 设为 0。
    else if (flag == 2)
    {
        ms_data.x = -step;
        ms_data.y = 0;
    }
	 //当 flag 为 3 时，将 x 设为 0，y 设为 -step。
    else if (flag == 3)
    {
        ms_data.x = 0;
        ms_data.y = -step;
    }
    return SENSOR_DATA_EVENT_AAA;//最后返回 SENSOR_DATA_EVENT_AAA，表示发生了传感器数据事件 AAA。
}
//清除配对标志位
void clear_pair_flag()
{
    pair_flag = 0;
    analog_write(USED_DEEP_ANA_REG1, ana_reg1_aaa & CLEAR_PAIR_ANA_FLAG);
}
//设置配对标志位
void set_pair_flag()
{
    pair_flag = 1;
    analog_write(USED_DEEP_ANA_REG1, ana_reg1_aaa | PAIR_ANA_FLG);
}
//向DEEP_ANA_REG0 寄存器写入数据
void write_deep_ana0(u8 buf)
{
    deep_flag = buf;
    analog_write(DEEP_ANA_REG0, buf);
}
//用户重启
void user_reboot(u8 reason)
{
    write_deep_ana0(reason);
    start_reboot();
}


//获取蓝牙数据报告
u8 get_ble_data_report_aaa(void)
{
#if WHEEL_FUN_ENABLE_AAA
    u32 wheel_prepare_tick = mouse_wheel_prepare_tick();//获取滚轮准备时间
#endif

#if BUTTON_FUN_ENABLE_AAA
    has_new_key_event |= btn_get_value();//获取按钮的值，并将结果与 has_new_key_event 进行按位或操作。
#endif

#if SENSOR_FUN_ENABLE_AAA
//判断是否有传感器数据报告；
    if (OPTSensor_motion_report(0))
    {
        has_new_key_event |= SENSOR_DATA_EVENT_AAA;//表示发生了传感器数据事件 AAA；
        mouse_xy_multiple();
        check_sensor_dircet(user_cfg.sensor_direct);//检查传感器方向；
        adaptive_smoother();
    }
	 //如果没有传感器数据报告，则将 ms_data.x 和 ms_data.y 设置为 0
    else
    {
        ms_data.x = 5;
        ms_data.y = 5;
    }
#endif
//测试绘制一个正方形的功能
#if TEST_DRAW_A_SQUARE
/*检查当前蓝牙链路状态是否为连接状态，
通过调用 blc_ll_getCurrentState() 函数获取当前的链路状态，如果状态为 BLS_LINK_STATE_CONN；
检查蓝牙连接状态是否为已连接状态 T5S_CONNECTED_STATUS_AAA；*/
        if ((blc_ll_getCurrentState() == BLS_LINK_STATE_CONN) && ((ble_status_aaa == T5S_CONNECTED_STATUS_AAA)))
        {
            has_new_key_event |= Draw_a_square_test();//绘制一个正方形,并将返回值与 has_new_key_event 进行按位或操作。
        }
#endif
/*这段代码用于处理滚轮功能、生成鼠标事件报告，并在有新键盘事件发生时更新相关状态和数据。*/
#if WHEEL_FUN_ENABLE_AAA
    has_new_key_event |= wheel_get_value(wheel_prepare_tick);//获取滚轮值，并将返回值与 has_new_key_event 进行按位或操作。
#endif
//代码检查是否有新的键盘事件发生（通过按位与运算符 & 判断是否存在 NEW_KEY_EVENT_AAA 事件）
//如果存在新的键盘事件
    if (has_new_key_event & (NEW_KEY_EVENT_AAA))
    {
        has_new_report_aaa |= HAS_MOUSE_REPORT;//将 HAS_MOUSE_REPORT 标记位设置到 has_new_report_aaa 中，表示有鼠标报告；
		my_fifo_push(&fifo_km, &ms_data.btn, sizeof(mouse_data_t));//将鼠标数据 ms_data.btn 推入 fifo_km 队列中，使用 my_fifo_push 函数；
    }
    return has_new_key_event;//函数返回 has_new_key_event，即更新后的事件标志位。

}

//获取24G数据报告，并在满足条件编译宏的情况下处理滚轮和按钮事件。
void get_24g_data_report_aaa()
{
#if WHEEL_FUN_ENABLE_AAA
    u32 wheel_prepare_tick;//声明并定义了一个 u32 类型的变量 wheel_prepare_tick；
    wheel_prepare_tick = mouse_wheel_prepare_tick();//获取鼠标滚轮的准备时间，并将返回值赋给 wheel_prepare_tick 变量。
#endif


#if BUTTON_FUN_ENABLE_AAA
    has_new_key_event |= btn_get_value();//函数获取按钮的值，并将返回值与 has_new_key_event 进行按位或操作。
#endif

#if WHEEL_FUN_ENABLE_AAA
    has_new_key_event |= wheel_get_value(wheel_prepare_tick);//函数获取滚轮的值，并将返回值与 has_new_key_event 进行按位或操作。
#endif
}
//根据 Flash 制造商 ID 不同，对 Flash 的特定区域进行锁定操作，以确保数据的安全性和完整性。
#if (APP_FLASH_LOCK_ENABLE)
//数据保持属性的声明，
_attribute_data_retention_ unsigned int mid;//= flash_read_mid();
_attribute_ram_code_ void flash_lock_init(void)
{

    printf("flash_lock_init\r\n");

	mid = flash_read_mid();//函数获取 Flash 的制造商 ID，并将其存储在 mid 变量中，然后打印出来。

	printf("mid flash:%x\r\n",mid);

	switch(mid)
	{
	/*case 0x1160c8:
        if (0 == (flash_read_status_mid1160c8() & FLASH_WRITE_STATUS_BP_MID1160C8))
            flash_lock_mid1160c8(FLASH_LOCK_LOW_64K_MID1160C8);
		break;*/
	//如果 mid 的值为 0x1360c8，则检查 Flash 状态是否满足特定条件，若满足则 锁定 Flash 的低 256K 区域。
	case 0x1360c8:
        if (0 ==  (flash_read_status_mid1360c8() & FLASH_WRITE_STATUS_BP_MID1360C8));
            flash_lock_mid1360c8(FLASH_LOCK_LOW_256K_MID1360C8);
		break;
//如果 mid 的值为 0x1460c8，同样检查 Flash 状态是否满足特定条件，若满足则锁定 Flash 的低 768K 区域。
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


