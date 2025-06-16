/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include "driver_timer.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "os_timer.h"
#include "os_mem.h"
#include "sys_utils.h"
#include "jump_table.h"
#include "FIFO.h"
#include "user_task.h"
#include "driver_wdt.h"
#include "driver_plf.h"
#include "driver_system.h"
#include "driver_i2s.h"
#include "driver_pmu.h"
#include "driver_uart.h"
#include "driver_rtc.h"
#include "driver_flash.h"
#include "driver_efuse.h"
#include "flash_usage_config.h"
#include "car_key_protocol.h"
#include "ble_simple_peripheral.h"
#include "simple_gatt_service.h"
#include "version.h"
#include "lte_protocol.h"
#include "os_task.h"
#include "os_timer.h"
#include "app_ota.h"
void user_loop_callback(void *arg);
void user_entry_before_ble_init(void);
uint8_t week_flag;
/*
 * LOCAL VARIABLES
 */
const struct jump_table_version_t _jump_table_version __attribute__((section("jump_table_3"))) = 
{
    .firmware_version = 0x00000000,
};

const struct jump_table_image_t _jump_table_image __attribute__((section("jump_table_1"))) =
{
    .image_type = IMAGE_TYPE_APP,
    .image_size = 0x24000,    // 128K  
};


__attribute__((section("ram_code"))) void rtc_isr_ram(uint8_t rtc_idx)
{
    if(rtc_idx == RTC_A)
    {
				
    }
    if(rtc_idx == RTC_B)
    {
        ;
    }
}


__attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void)
{
		
//		if(pmu_get_gpio_value(GPIO_PORT_D,GPIO_BIT_4) == 1){
//			
//		}
		week_flag = 1;
		os_user_loop_event_set(&user_loop_callback);
		pmu_port_wakeup_func_clear(GPIO_PA0);
		rtc_disalarm(RTC_A);
}

/*********************************************************************
 * @fn      user_custom_parameters
 *
 * @brief   initialize several parameters, this function will be called 
 *          at the beginning of the program. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_custom_parameters(void)
{
    struct chip_unique_id_t id_data;

    efuse_get_chip_unique_id(&id_data);
    __jump_table.addr.addr[0] = 0xBD;
    __jump_table.addr.addr[1] = 0xAD;
    __jump_table.addr.addr[2] = 0xD0;
    __jump_table.addr.addr[3] = 0xF0;
    __jump_table.addr.addr[4] = 0x17;
    __jump_table.addr.addr[5] = 0x20;
    
    id_data.unique_id[5] |= 0xc0; // random addr->static addr type:the top two bit must be 1.
    memcpy(__jump_table.addr.addr,id_data.unique_id,6);
    __jump_table.system_clk = SYSTEM_SYS_CLK_48M;
    jump_table_set_static_keys_store_offset(JUMP_TABLE_STATIC_KEY_OFFSET);
    ble_set_addr_type(BLE_ADDR_TYPE_PUBLIC);
    retry_handshake();
}

/*********************************************************************
 * @fn      user_entry_before_sleep_imp
 *
 * @brief   Before system goes to sleep mode, user_entry_before_sleep_imp()
 *          will be called, MCU peripherals can be configured properly before 
 *          system goes to sleep, for example, some MCU peripherals need to be
 *          used during the system is in sleep mode. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void user_entry_before_sleep_imp(void)
{
	uart_putc_noint_no_wait(UART1, 's');
	pmu_set_pin_pull(GPIO_PORT_A, (1<<GPIO_BIT_0), true);
	pmu_port_wakeup_func_set(GPIO_PA0);
	wdt_feed();
	rtc_alarm(RTC_A,12000);
}

/*********************************************************************
 * @fn      user_entry_after_sleep_imp
 *
 * @brief   After system wakes up from sleep mode, user_entry_after_sleep_imp()
 *          will be called, MCU peripherals need to be initialized again, 
 *          this can be done in user_entry_after_sleep_imp(). MCU peripherals
 *          status will not be kept during the sleep. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void user_entry_after_sleep_imp(void)
{
    /* set PA2 and PA3 for AT command interface */
    //system_set_port_pull(GPIO_PA2, true);
    //system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);
    
    uart_init(UART1, BAUD_RATE_115200);
    //NVIC_EnableIRQ(UART1_IRQn);
		uart_putc_noint_no_wait(UART1, 'w');
    // Do some things here, can be uart print
		wdt_feed();
    NVIC_EnableIRQ(PMU_IRQn);
}

/*********************************************************************
 * @fn      user_entry_before_ble_init
 *
 * @brief   Code to be executed before BLE stack to be initialized.
 *          Power mode configurations, PMU part driver interrupt enable, MCU 
 *          peripherals init, etc. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_entry_before_ble_init(void)
{    
    /* set system power supply in BUCK mode */
    pmu_set_sys_power_mode(PMU_SYS_POW_BUCK);
#ifdef FLASH_PROTECT
    flash_protect_enable(1);
#endif
    pmu_enable_irq(PMU_ISR_BIT_ACOK
                   | PMU_ISR_BIT_ACOFF
                   | PMU_ISR_BIT_ONKEY_PO
                   | PMU_ISR_BIT_OTP
                   | PMU_ISR_BIT_LVD
                   | PMU_ISR_BIT_BAT
                   | PMU_ISR_BIT_ONKEY_HIGH);
    NVIC_EnableIRQ(PMU_IRQn);
		pmu_set_port_mux(GPIO_PORT_D,GPIO_BIT_5,PMU_PORT_MUX_GPIO);
		pmu_set_pin_to_PMU(GPIO_PORT_D, BIT(5) );
    pmu_set_pin_dir(GPIO_PORT_D, BIT(5), GPIO_DIR_OUT);
    pmu_set_pin_pull(GPIO_PORT_D, BIT(5), false);
		pmu_set_gpio_value(GPIO_PORT_D, BIT(5), 0);  //表示已连接。
		wdt_init(WDT_ACT_RST_CHIP, 16);
		wdt_start();
		
		
		system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_D4);
		pmu_set_pin_to_PMU(GPIO_PORT_D, BIT(4));
		pmu_set_pin_dir(GPIO_PORT_D, BIT(4), GPIO_DIR_IN);

		pmu_set_pin_pull(GPIO_PORT_D, BIT(4), false);
		
		
		
	  system_set_port_pull(GPIO_PA0, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_UART0_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_UART0_TXD);
		uart_init(UART0, BAUD_RATE_115200);  
		rtc_init();
//		
//		NVIC_EnableIRQ(UART0_IRQn);
    // Enable UART print.
    //system_set_port_pull(GPIO_PA2, true);
    //system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);
    uart_init(UART1, BAUD_RATE_115200);    
		wdt_feed();
}

uint32_t cur_time = 0;
void user_loop_callback(void *arg)
{
		if(week_flag == 1){
				week_flag = 0;
				pmu_set_pin_to_CPU(GPIO_PORT_A, GPIO_PA0 | GPIO_PA1);
				system_set_port_pull(GPIO_PA0, true);
				system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_UART0_RXD);
				system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_UART0_TXD);
				uart_init(UART0, BAUD_RATE_115200); 
				printf("cat1 is week");
		}
		
		
		if(dft_gpio_flag == 1){
				if(pmu_get_gpio_value(GPIO_PORT_D, GPIO_BIT_4) == 1) {
					 pmu_set_gpio_value(GPIO_PORT_D, BIT(5), 1);
						dft_gpio_flag = 0;	
						printf("PD5 OUT 1");
				}
		}
		ble_rcv_parse();	
		if(system_get_curr_time() - cur_time > 3000) {
			co_printf("ble heart\r\n");
			cur_time = system_get_curr_time();
			ble_send_cmd(CMD_BLE_HEART, 0);
		}
		if(car_key_con.flag== 1){   //Open
			if(system_get_curr_time() - car_key_con.cur_time > car_key_con.time) {
					car_key_con.cur_time = system_get_curr_time();
					ble_send_cmd(LTE_OPEN_KEY, 0);
					car_key_con.t++;
					if(car_key_con.t == car_key_con.cnt) {
							car_key_con.flag = 0;
					}
			}
		} else if(car_key_con.flag == 2){ //close
			if(system_get_curr_time() - car_key_con.cur_time > car_key_con.time) {
					car_key_con.cur_time = system_get_curr_time();
					ble_send_cmd(LTE_CLOSE_KEY, 0);
					car_key_con.t++;
					if(car_key_con.t == car_key_con.cnt) {
							car_key_con.flag = 0;
					}
			}
		}
		wdt_feed();
}

/*********************************************************************
 * @fn      user_entry_after_ble_init
 *
 * @brief   Main entrancy of user application. This function is called after BLE stack
 *          is initialized, and all the application code will be executed from here.
 *          In that case, application layer initializtion can be startd here. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_entry_after_ble_init(void)
{
    co_printf("BLE Peripheral\r\n");
		co_printf("OTATEST\r\n");
    char *data, *time;
    get_SDK_compile_date_time(&data, &time);
    co_printf("SDK Version:%s,%s\r\n",data,time);
	//	test_protol();
		co_printf("DATA TIME:%s_%s", __DATE__, __TIME__);
		co_printf("ver:%0x\r\n", app_otas_get_curr_firmwave_version());
#if 1
    system_sleep_disable();		//disable sleep 
#else
    if(__jump_table.system_option & SYSTEM_OPTION_SLEEP_ENABLE)  //if sleep is enalbed, delay 3s for JLINK 
    {
        co_printf("\r\na");
        co_delay_100us(10000);       
        co_printf("\r\nb");
        co_delay_100us(10000);
        co_printf("\r\nc");
        co_delay_100us(10000);
        co_printf("\r\nd");
    }
#endif
    // User task initialization, for buttons.
    user_task_init();
		week_flag = 0;
		dft_gpio_flag = 0;
    // Application layer initialization, can included bond manager init, 
    // advertising parameters init, scanning parameter init, GATT service adding, etc.    
    simple_peripheral_init();
		wdt_feed();
		os_user_loop_event_set(&user_loop_callback);
		ble_send_cmd(CMD_BLE_SIGN, 0);
}
