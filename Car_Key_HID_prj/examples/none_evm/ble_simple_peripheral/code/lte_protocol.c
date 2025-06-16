#include "lte_protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver_uart.h"
#include "gap_api.h"
#include "co_printf.h"
#include "FIFO.h"
#include "car_key_protocol.h"
#include "ble_simple_peripheral.h"
#include "simple_gatt_service.h"
#include "driver_system.h"
#include "os_task.h"
#include "driver_rtc.h"
#include "os_timer.h"
#include "app_ota.h"
#include "driver_flash.h"
#include "jump_table.h"
#include "sys_utils.h"
#include "driver_wdt.h"
#if DEBUG_EN == 1
#define BLE_DEBUG(a, b, c)	print_data(a, b, c)
#else
#define BLE_DEBUG(a, b,c)
#endif
uint8_t ack_flag;
extern void adv_disconnect();
extern void set_dev_name(uint8_t *name, uint16_t len);
extern void set_dev_data(uint8_t *data, uint16_t len);
extern void adv_stop();
void print_data(char *str, uint8_t *data, uint16_t len)
{
		uint16_t i = 0;
		co_printf("%s[%d]:\r\n", str, len);
		for(i = 0; i< len; i++){
				co_printf("%02x ", data[i]);
		}
}

/****************************************************
** 功能    :计算校验值
********************************************************/
uint16_t ble_Package_CheckSum(uint8_t* pdata, uint32_t len)
{
    uint16_t sum = 0;
    uint32_t i;

    for(i = 0; i < len; i++) 
        sum += pdata[i];
    sum = sum ^ 0xFFFF;             //异或0xFFFFF = 取反后取低16位
	return sum;
}


void ble_send_data(uint8_t cmd, uint8_t *data, uint16_t len)
{
		uint8_t buf[256];
		uint16_t lenth = 0, i;
		uint16_t check;
		buf[lenth++] = HEADERH;
		buf[lenth++] = HEADERL;
		buf[lenth++] = cmd;
		buf[lenth++] = len << 8;
		buf[lenth++] = len&0xff;
		memcpy(&buf[lenth], data, len);
		lenth += len;
		check = ble_Package_CheckSum(&buf[2], lenth-2);
		
		buf[lenth++]=check&0xff;
		buf[lenth++]=check>>8;
		BLE_DEBUG("ble_send", buf, lenth);
		uart_put_data_noint(UART0, buf, lenth);
}

const uint8_t test_dat[]={
		0x08, 
		GAP_ADVTYPE_LOCAL_NAME_COMPLETE,
  'E','N','G','W','E','Y','B',
		0x03,
	GAP_ADVTYPE_16BIT_MORE,
	0x18,0x20,
};
void test_protol()
{
		uint8_t buf[256];
		uint16_t len = 0;
		uint16_t check;
		buf[len++] = 0x55;
		buf[len++] = 0xaa;
		buf[len++] = LTE_GET_VER;
		
		buf[len++] = 0x00;
		buf[len++] = 0x00;
//		memcpy(&buf[len], &test_dat[0], sizeof(test_dat));
//		len += sizeof(test_dat);
	
		check = ble_Package_CheckSum(&buf[2], len-2);
		buf[len++] = check&0xff;
		buf[len++] =	check>>8;
		BLE_DEBUG("ble_send", buf, len);
}
void ble_send_cmd(uint8_t cmd, uint8_t ok)
{
		uint8_t data[36];
		uint16_t check;
		uint16_t lenth = 0;
		data[lenth++] = HEADERH;
		data[lenth++] = HEADERL;
		data[lenth++] = cmd;
		switch(cmd){
			case LTE_START_ADV:
			case LTE_STOP_ADV:
			case LTE_SET_ADV_DATA:
			case LTE_SCAN_RSP:
			case LTE_ADV_PARAM:
			case LTE_CON_PARAM:
			case LTE_DELETE_BONDE:
			case LTE_ENTER_SLEEP:
			case LTE_OTA_START:
			case LTE_OTA_DATA:
			case LTE_OTA_END:
			case LTE_CLOSE_CON:
			case CMD_BLE_SIGN:
			case CMD_BLE_HEART:
				data[lenth++] = 0x00;
				data[lenth++] = 0x01;
				if(ok == R_OK)
					data[lenth++] = 0x00;
				else 
					data[lenth++] = 0x01;
			break;
			case LTE_GET_MAC:
				mac_addr_t addr;
				gap_address_get(&addr);
				data[lenth++] = 0x00;
				data[lenth++] = 0x06;
				memcpy(&data[lenth], &addr.addr[0], 6);
				lenth += 6;
			break;	
			case LTE_GET_VER:
				data[lenth++] = 0x00;
				data[lenth++] = 0x06;
				memcpy(&data[lenth], BLE_VER, 6);
				lenth +=6;
			break;
			case LTE_CLOSE_KEY:
			case LTE_OPEN_KEY:
			case DFT_CMD_BLE_CONN_GPIO:
				data[lenth++] = 0x00;
				data[lenth++] = 0x00;
			break;	
		}
		check = ble_Package_CheckSum(&data[2], lenth - 2);
		data[lenth++] = check&0xff;
		data[lenth++] = check>>8;
		BLE_DEBUG("ble_send", data, lenth);
		while(lenth > 50){
				uart_put_data_noint(UART0, data, 50);
				lenth -= 50;
				co_delay_100us(500);
		} 
		uart_put_data_noint(UART0, data, lenth);
}
struct ble_ota_config_stu{
    uint32_t firm_ver;
    uint32_t ota_crc;
    uint32_t total;
};
struct ble_ota_control_stu{
		uint16_t pack_num;
		uint16_t last_num;
		uint8_t data_len;
		uint32_t offset;
		uint32_t firmware_offset;
		uint8_t data[128];
		uint8_t data_save[256];
		uint32_t ota_adress; 
};
uint8_t dft_gpio_flag;
struct ble_ota_config_stu ble_ota_config;
struct ble_ota_control_stu ble_ota_control;
uint8_t gb_delete_wl;
void ble_protol_process(uint8_t cmd, uint8_t *data, uint16_t len)
{
		co_printf("cmd:%02x\r\n", cmd);
		uint8_t res;
		switch (cmd)
		{
			case LTE_SET_ADV_DATA:
				if(len > 31){
					res = R_FAIL;
				}else {
					set_dev_data(data, strlen((char *)data));
					res = R_OK;
				}
				ble_send_cmd(LTE_SET_ADV_DATA, res);
			break;
			case LTE_GET_MAC:
				res = R_OK;
				ble_send_cmd(LTE_GET_MAC, res);
			break;
			case LTE_SCAN_RSP:
				if(len > 31){
						res = R_FAIL;
				}else{
						res = R_OK;
						set_dev_sacn_data(data, len);
				}
				ble_send_cmd(LTE_SCAN_RSP, res);	
			break;
			case LTE_GET_VER:
				res = R_OK;
				ble_send_cmd(LTE_GET_VER, res);
			break; 
			case LTE_START_ADV:
				sp_start_adv();
		//		sp_start_adv();
				res = R_OK;
				ble_send_cmd(LTE_START_ADV, res);
			break;
			case LTE_STOP_ADV:
				adv_stop();
				res = R_OK;
				ble_send_cmd(LTE_STOP_ADV, res);
			break;
			case LTE_CLOSE_CON:
				adv_disconnect();
				res = R_OK;
				ble_send_cmd(LTE_CLOSE_CON, res);
				break;
			case LTE_SEND_DATA:
				BLE_DEBUG("ble_trans", data, len);
				dev_ntf_data_to_app(ENG_R_CHAR1_VALUE, data, len);
			break;
			case LTE_VIRT_AT:
				BLE_DEBUG("ble_virt_at", data, len);
				dev_ntf_data_to_app(ENG_AT_CHAR3_VALUE, data, len);
				break;
			case LTE_OPEN_KEY:
				car_key_con.flag = 0;
			break;
			case LTE_CLOSE_KEY:
				car_key_con.flag = 0;
			break;
			case LTE_ADV_PARAM:
				struct adv_param_s adv_p;		
				if(len != 4) {
						res = R_FAIL;
				} else {	
						adv_p.adv_intv_min = data[0]<<8 | data[1];
						adv_p.adv_intv_max = data[2]<<8 | data[3];
						set_ble_adv_param(adv_p.adv_intv_min, adv_p.adv_intv_max);
						res = R_OK;		
				}	
				ble_send_cmd(LTE_ADV_PARAM, res);
				break;
			case LTE_CON_PARAM:
				struct adv_con_param_s adv_con_p;
				if(len != 8){
						res = R_FAIL;
				} else {
						adv_con_p.min_intv = data[0]<<8 | data[1];
						adv_con_p.max_intv = data[2]<<8 | data[3];
						adv_con_p.slave_latency = data[4]<<8|data[5];
						adv_con_p.timeout = data[6] << 8 | data[7];
						set_ble_adv_con_param(adv_con_p.min_intv, adv_con_p.max_intv, adv_con_p.slave_latency, adv_con_p.timeout);
						res = R_OK;
				}
				ble_send_cmd(LTE_CON_PARAM, res);
				break;		
			case LTE_DELETE_BONDE:
				co_printf("gap_bond_manager_delete_all\r\n");
				gb_delete_wl = 1;
				adv_disconnect();
				gap_bond_manager_delete_all();
				res = R_OK;
				ble_send_cmd(LTE_DELETE_BONDE, res);
				gap_stop_advertising();
			break;
			case LTE_ENTER_SLEEP:
				res = R_OK;
				ble_send_cmd(LTE_ENTER_SLEEP, res);
				system_sleep_enable();
				os_user_loop_event_clear();
				rtc_alarm(RTC_A,12000);
			break;
			case LTE_OTA_START:
				memcpy(&ble_ota_config, data, sizeof(ble_ota_config));
				co_printf("firm_ver:%0x, ota_crc:%0x, total:%d\r\n", ble_ota_config.firm_ver, ble_ota_config.ota_crc, ble_ota_config.total);
				res = R_OK;
				ble_ota_control.ota_adress = app_otas_get_storage_address();
				ble_ota_control.offset = 0;
				ble_ota_control.last_num = -1;
				co_printf("cur_adr,:%0x, storage_adr:%0x\r\n",  app_otas_get_curr_code_address(), ble_ota_control.ota_adress);
				app_set_ota_state(1);
				app_otas_flash_read(ble_ota_control.ota_adress, ble_ota_control.data_save,256);
        flash_erase(ble_ota_control.ota_adress, 0x1000);
        app_otas_save_data(ble_ota_control.ota_adress, ble_ota_control.data_save, 256);
				app_ota_erase(ble_ota_control.ota_adress + 0x1000, ble_ota_config.total - 0x1000);
				ble_send_cmd(LTE_OTA_START, res);
			break;
			case LTE_OTA_DATA:
				res = R_OK;
				ble_ota_control.pack_num = data[1] << 8 | data[0];
				ble_ota_control.data_len = len - 2;
				co_printf("pack_num:%d\r\n", ble_ota_control.pack_num);
				if(ble_ota_control.pack_num != ble_ota_control.last_num) {	
						ble_ota_control.last_num = ble_ota_control.pack_num;
						memcpy(ble_ota_control.data, &data[2], ble_ota_control.data_len);
						if(ble_ota_control.pack_num == 0) {
								memcpy(&ble_ota_control.data_save[0], ble_ota_control.data, ble_ota_control.data_len);
						} else if(ble_ota_control.pack_num == 1) {
								memcpy(&ble_ota_control.data_save[128], ble_ota_control.data, ble_ota_control.data_len);
								ble_ota_control.firmware_offset = (uint32_t)&((struct jump_table_t *)0x01000000)->firmware_version- 0x01000000;
								if(*(uint32_t *)((uint32_t)ble_ota_control.data_save + ble_ota_control.firmware_offset) <= app_otas_get_curr_firmwave_version()){
											uint32_t new_bin_ver = app_otas_get_curr_firmwave_version() + 1;
                      co_printf("old_ver:%08X\r\n",*(uint32_t *)((uint32_t)ble_ota_control.data_save + ble_ota_control.firmware_offset));
                      co_printf("new_ver:%08X\r\n",new_bin_ver);
                     *(uint32_t *)((uint32_t)ble_ota_control.data_save + ble_ota_control.firmware_offset) = new_bin_ver;
							}
						} else {
								app_otas_save_data(ble_ota_control.ota_adress + ble_ota_control.offset, ble_ota_control.data, ble_ota_control.data_len);
						}					
						ble_ota_control.offset += ble_ota_control.data_len;
						ble_send_cmd(LTE_OTA_DATA, res);
				} else {
						ble_send_cmd(LTE_OTA_DATA, res);
				}
			break;
			case LTE_OTA_END:
				if(app_otas_crc_cal(ble_ota_config.total, ble_ota_control.ota_adress, ble_ota_config.ota_crc)){
	//			if(ota_data_check(ble_ota_config.total - 256, ble_ota_control.ota_adress + 256) == ble_ota_config.ota_crc){
						co_printf("BLE OTA IS OK\r\n");
						res = R_OK;
						ble_send_cmd(LTE_OTA_END, res);
						wdt_feed();
						co_delay_100us(1000);
						app_otas_save_data(ble_ota_control.ota_adress, ble_ota_control.data_save, 256);
					//		app_otas_save_first_pkt(ble_ota_control.ota_adress, ble_ota_control.data_save, 256);
						
				} else {
						co_printf("CRC is fail!\r\n");
						res = 1;
						ble_send_cmd(LTE_OTA_END, res);
						co_delay_100us(1000);
				}
				app_set_ota_state(0);
				uart_finish_transfers(UART1_BASE);
				platform_reset_patch(0);
			break;
			case DFT_CMD_BLE_CONN_GPIO:
				res = R_OK;
				ble_send_cmd(DFT_CMD_BLE_CONN_GPIO, res);
				pmu_set_gpio_value(GPIO_PORT_D, BIT(5), 0);
				printf("DFT_CMD_BLE_CONN_GPIO\r\n");
				dft_gpio_flag = 1;
			break;
		}
}

void ble_rcv_parse()
{
		uint8_t res;
		static uint32_t ble_proto_time;
		static uint16_t len, i;
		static uint16_t check_sum = 0,rcv_check = 0;
		static uint8_t buf[256], cmd, data[256],data_len;
		static uint8_t step = 0;
		
		if(uart_get_data_nodelay_noint(UART0, &res, 1) == 1){
				switch(step){
					case 0:
						if(res == HEADERH){
								step = 1;
								i = 0;
								check_sum = 0;
								len = 0;
								memset(buf, 0, sizeof(buf));
								ble_proto_time = system_get_curr_time();
								data_len = 0;
								data[data_len++] = res;
						}
						break;
					case 1:
						if(res == HEADERL){
							step = 2;
						} else {
							step = 0;
						}
						data[data_len++] = res;
						break;
					case 2:
						cmd = res;
						step = 3;
						check_sum += res;
						data[data_len++] = res;
						break;
					case 3:
						len = res << 8;
						step = 4;
						check_sum += res;
						data[data_len++] = res;
						break;
					case 4:
						len |= res;
						if(len)
							step = 5;
						else step = 6;
						check_sum += res;
						data[data_len++] = res;
						break;
					case 5:
						buf[i++] = res;
						check_sum += res;
						if(i == len) step = 6;
						data[data_len++] = res;
						break;
					case 6:
						rcv_check = res;
						step = 7;
						data[data_len++] = res;
						break;
					case 7:
						data[data_len++] = res;
						rcv_check |= res << 8;
						check_sum = check_sum ^ 0xFFFF; 
						if(check_sum == rcv_check)
							ble_protol_process(cmd, &buf[0], len);
						else {
							for(i = 0; i < data_len; i++)
								co_printf("%02x\t", data[i]);
							co_printf("\r\n");
							co_printf("check_sum:%04x, rcv_check:%04x\r\n", check_sum, rcv_check);
						}
						step = 0;
						break;
				}
		}
				if(system_get_curr_time() - ble_proto_time > 2000) {
						step = 0;
				}
}













