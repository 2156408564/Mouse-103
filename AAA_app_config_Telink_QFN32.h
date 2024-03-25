#ifndef _TELINK_QFN32_AAA_H_
#define _TELINK_QFN32_AAA_H_

#if (PRJ_NAME==Telink_QFN32_PRJ)
	#define ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG    0
    //-----------------------------for debug-----------------------------
	#define TEST_LOST_RATE		 0   //test 2.4g rf lost rate
	#define APP_FLASH_LOCK_ENABLE	1
	
	#define  BLE_SNIFF_DEBUG    0   //for ti packet sniff   if enable mouse do not enter sleep for ever
    #define  BLE_OTA_LED_DEBUG   0   //led show  ota result
    #define DEBUG_GPIO_AAA		 0  //
    #define TEST_DRAW_A_SQUARE	 (ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG||0)   //when mouse  connected auto draw  a square
    
    #define AUDIO_AUTO_OPEN_TEST_DEBUG  0 //823x not support
    #define DEVICE_NAME_INCLUDE_MAC_DEBUG 0 //for debug
    #define MUTI_DEVICE_SWITCH_DEBUG     0 //for debug

    #define ADC_BATT_DEBUG   0  //enbale : in ble mode £¬notify adc value.

    // test  mcu current ,in the mode,sensor shut down ,
    // when both left and right  button are pressed £¬mouse auto draw a square in conencted
    #define TEST_MCU_CURRENT_DEBUG       0

    #define SHOW_MAST_REAL_MAC_DEBUG    0//for test get master real addr

    #define DEBUG_TOGGLE_GPIO_ENABLE    0
    //-----------------------------PM --------------------------------

    #define BLE_APP_PM_ENABLE					1
    #define PM_DEEPSLEEP_RETENTION_ENABLE		0//must set 0
    #define BLE_ADV_ENTER_DEEPSLEEP_AFTER_TIME_OUT_ENABLE_AAA      ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&&1)
    #define BLE_CONNECT_ENTER_DEEPSLEEP_AFTER_TIME_OUT_ENABLE_AAA  ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&&1)


    #define D24G_ADV_ENTER_DEEPSLEEP_AFTER_TIME_OUT_ENABLE_AAA     ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&&1)
    #define D24G_CONNECT_ENTER_DEEPSLEEP_AFTER_TIME_OUT_ENABLE_AAA  ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&&1)


    #define BLE_ADV_TIMER_OUT  60 //unit 1s
    #define D24G_ADV_TIMER_OUT  60 //unit 1s

    #define BLE_CONNECT_TIME_OUT  300 //unit 1s
    #define D24G_CONNECT_TIME_OUT  300 //unit 1s

    #if PM_DEEPSLEEP_RETENTION_ENABLE
        #define _attribute_data_retention_user    _attribute_data_retention_
    #else
        #define _attribute_data_retention_user
    #endif

    //-----------------------product function referance---------------------------------------
    //----------------------for debug--------------
    
    #define UART_PRINT_DEBUG_ENABLE 	1//not support
        
    #if UART_PRINT_DEBUG_ENABLE
        #define PRINT_BAUD_RATE     1000000
        #define DEBUG_INFO_TX_PIN   GPIO_PA7 //GPIO_SWS
        #define PA7_INPUT_ENABLE    0
        #define PA7_OUTPUT_ENABLE   1
        #define PA7_FUNC    AS_GPIO

        #define my_printf_aaa	 		u_printf
    #else
        static inline void no_printf(const char *format, ...){    return;}
        #define my_printf_aaa     no_printf 
    #endif

	#define DEVICE_APPEARANCE      0X03C2// 0X0XC1 KEYBOARD 0X03C2 MOUSE
    #define DEVICE_ADV_NAME   "MicLink"
    #define DEVICE_NAME_AAA   "MicLink Mouse"

    #define MOUSE_PIPE1_DATA_WITH_DID	1//If you want to fit the old hamster dongle (firmware version :v5.3) it must set 0

    #define MOUSE_DATA_LEN_AAA	6   //If you want to fit the old hamster dongle (firmware version :v5.3)  it must set 4

    #define BLE_OTA_ENABLE_AAA     1  //BLE OTA Enable
    #define D24G_OTA_ENABLE_AAA    1  //2.4G-OTA Enable

    #define EMI_TEST_FUN_ENABLE_AAA   ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&&0)//If the flash  is 128k, must be set 0£¬save 2k

    #define BATT_CHECK_ENABLE      ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&& 1)

    #define WHEEL_FUN_ENABLE_AAA   ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&&1)

    #define BUTTON_FUN_ENABLE_AAA   ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&&1)

    #define SENSOR_FUN_ENABLE_AAA   ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&&1)
    #define SENSOR_CS_ENABLE       0
    #define SENSOR_SHUT_DOWN_ENABLE 1

    #define BLT_APP_LED_ENABLE   ((!ADAPT_ALL_HW_AUTO_DRAW_IN_D24G_MODE_DEBUG)&&1)
    #define	LED_OFF_AAA	1
    #define LED_ON_AAA	0



    #define DIRECT_ADV_TO_UNDIRECT_ENABLE  1

    #define DOUBLE_CLICK_LEFT_FUN_ENABLE    0

    #define BLE_AUDIO_ENABLE     1
    #define AUTO_CHECK_OS_TYPE   0
    #define Microsoft_Swift_Pairing_ENABLE 1//if enable then device_name_len<=11

    #define	BLT_SOFTWARE_TIMER_ENABLE	0

    #define MUTI_SENSOR_ENABLE  1
    #define DPI_SAVE_FLASH      1
	#define ENTER_PAIR_WHEN_NEVER_PAIRED_ENABLE 1//For the better production of the factory

    // aes method
    //0:no aes encryption   no key					old method
    //1:aes128 encryption   User burn fixed key   	old method  not used
    //2:aes128 encryption	Dynamic random key
    //3:aes128 decryption	Dynamic random key  for 8366 dongle
    
   // If the dongle  is 8366, it is recommended to set it to 0 or 3
   // if it is set to 3 , the algorithm will consume less time for 8366
   //If the dongle  is 8355, it can be set to any value
   //attention :Keyboard, mouse and receiver must have the same parameters
    #define AES_METHOD 0

	#define HOPING_METHOD   1
    #define MOTION_PIN      1
    //---------------------------gpio---------------------------------------------
    #if SENSOR_FUN_ENABLE_AAA
        #if MOTION_PIN
        #define PIN_SIF_MOTION 	GPIO_PA0//39a30_v1.0 j4 S-tx
        #define PA0_INPUT_ENABLE	1
        #define PA0_OUTPUT_ENABLE	0
        #define PA0_DATA_OUT    1
        #define	PULL_WAKEUP_SRC_PA0	PM_PIN_PULLUP_1M
        #endif

        #define PIN_SIF_SCL 	GPIO_PD7//39a30_v1.0  j15
        #define PD7_INPUT_ENABLE	0
        #define PD7_OUTPUT_ENABLE	1
        #define PD7_DATA_OUT	1
        #define PULL_WAKEUP_SRC_PD7 PM_PIN_PULLUP_1M

        #define PIN_SIF_SDA 	GPIO_PD4//39a30_v1.0  j15
        #define PD4_INPUT_ENABLE	1
        #define PD4_OUTPUT_ENABLE	1
        #define PD4_DATA_OUT	1
        #define PULL_WAKEUP_SRC_PD4 PM_PIN_PULLUP_10K


        #if SENSOR_CS_ENABLE
            #define PIN_SENSOR_CS   GPIO_PA7//39a30_v1.0  SWS
            #define PA7_INPUT_ENABLE	0
            #define PA7_OUTPUT_ENABLE	1
            #define PA7_DATA_OUT	1
            #define PULL_WAKEUP_SRC_PA7 PM_PIN_PULLUP_1M
        #endif

    #endif

    #if BUTTON_FUN_ENABLE_AAA
    
        #define BTN_NUM_AAA             4

        #define PIN_BTN_LEFT           GPIO_PB6
        #define PB6_INPUT_ENABLE        1
        #define PB6_OUTPUT_ENABLE       0
        #define PB6_DATA_OUT            1
        #define PULL_WAKEUP_SRC_PB6     PM_PIN_PULLUP_1M

        #define PIN_BTN_RIGHT           GPIO_PB4
        #define PB4_INPUT_ENABLE        1
        #define PB4_OUTPUT_ENABLE       0
        #define PB4_DATA_OUT            1
        #define PULL_WAKEUP_SRC_PB4     PM_PIN_PULLUP_1M

        #define PIN_BTN_MIDDLE          GPIO_PB5
        #define PB5_INPUT_ENABLE        1
        #define PB5_OUTPUT_ENABLE       0
        #define PB5_DATA_OUT            1
        #define PULL_WAKEUP_SRC_PB5     PM_PIN_PULLUP_1M

        #define PIN_BTN_MODE            GPIO_PD2
        #define PD2_INPUT_ENABLE        1
        #define PD2_OUTPUT_ENABLE       0
        #define PD2_DATA_OUT            0
        #define PULL_WAKEUP_SRC_PD2     PM_PIN_PULLUP_1M


        #define PIN_BTN_OUT_VCC          GPIO_PD3
        #define PD3_INPUT_ENABLE        0
        #define PD3_OUTPUT_ENABLE       1
        #define PD3_DATA_OUT            0
        #define PULL_WAKEUP_SRC_PD3     PM_PIN_PULLUP_1M

        #define BTN_MATRIX {PIN_BTN_LEFT,PIN_BTN_RIGHT,PIN_BTN_MIDDLE, PIN_BTN_MODE}

        //#define COL_PINS    {GPIO_PA0, GPIO_PD3}
        //#define ROW_PINS    {GPIO_PD2, GPIO_PA1, GPIO_PB4}
    #endif

    #if WHEEL_FUN_ENABLE_AAA
        #define PIN_WHEEL_1         GPIO_PC2
        #define PC2_INPUT_ENABLE	1
        #define PC2_OUTPUT_ENABLE	0
        #define PC2_DATA_OUT        0
        #define PULL_WAKEUP_SRC_PC2 PM_PIN_PULLUP_1M


        #define PIN_WHEEL_2         GPIO_PC3
        #define PC3_INPUT_ENABLE	1
        #define PC3_OUTPUT_ENABLE	0
        #define PC3_DATA_OUT        0
        #define PULL_WAKEUP_SRC_PC3 PM_PIN_PULLUP_1M

        #define WHEEL_ADDRES_D2   0X04
        #define WHEEL_ADDRES_D3   0X05
    #endif


    #if BLT_APP_LED_ENABLE
        #define PIN_24G_LED         GPIO_PB1
        #define PB1_INPUT_ENABLE	0
        #define PB1_OUTPUT_ENABLE	1
        #define PB1_DATA_OUT        LED_OFF_AAA
        #define PULL_WAKEUP_SRC_PB1 PM_PIN_UP_DOWN_FLOAT

        #if UART_PRINT_DEBUG_ENABLE
        #define PIN_BLE_LED_1	    PIN_24G_LED
        #else
        #define PIN_BLE_LED_1       GPIO_PA7
        #endif
        #define PA7_INPUT_ENABLE	0
        #define PA7_OUTPUT_ENABLE	1
        #define PA7_DATA_OUT        LED_OFF_AAA
        #define PA7_FUNC    AS_GPIO
        #define PULL_WAKEUP_SRC_PA7 PM_PIN_UP_DOWN_FLOAT

        #define PIN_BAT_LED         GPIO_PA1
        #define PA1_INPUT_ENABLE    0
        #define PA1_OUTPUT_ENABLE   1
        #define PA1_DATA_OUT        LED_OFF_AAA
        #define PULL_WAKEUP_SRC_PA1 PM_PIN_UP_DOWN_FLOAT


        #if 0
        #define PIN_BLE_LED_2       GPIO_PA1
        #define PA1_INPUT_ENABLE    0
        #define PA1_OUTPUT_ENABLE   1
        #define PA1_DATA_OUT        LED_OFF_AAA
        #define PULL_WAKEUP_SRC_PA1 PM_PIN_UP_DOWN_FLOAT
        #endif
    #endif


    #if (BATT_CHECK_ENABLE)
        //telink device: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
        //use PB7 output high level, then adc measure this high level voltage
        #define GPIO_VBAT_DETECT				GPIO_PB7
        #define PB7_FUNC						AS_GPIO
        #define PB7_INPUT_ENABLE				0
        #define ADC_INPUT_PCHN					B7P    //corresponding  ADC_InputPchTypeDef in adc.h
    #endif


/**
 *  @brief  AUDIO Configuration
 */
#if (BLE_AUDIO_ENABLE)

	#define BLE_DMIC_ENABLE					0  //0: Amic   1: Dmic
	#define IIR_FILTER_ENABLE				0

	#define GPIO_AMIC_BIAS					GPIO_PC4
	#define GPIO_AMIC_SP					GPIO_PC0
	#define GPIO_AMIC_SN					GPIO_PC1
	/* RCU Audio MODE:
	 * TL_AUDIO_RCU_ADPCM_GATT_TLEINK
	 * TL_AUDIO_RCU_ADPCM_GATT_GOOGLE
	 * TL_AUDIO_RCU_ADPCM_HID
	 * TL_AUDIO_RCU_SBC_HID						//need config 32k retention
	 * TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB
	 * TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB		//need config 32k retention
	 * TL_AUDIO_RCU_MSBC_HID					//need config 32k retention
	 */
	#define TL_AUDIO_MODE  						TL_AUDIO_RCU_ADPCM_GATT_TLEINK

#endif



#endif

#endif
