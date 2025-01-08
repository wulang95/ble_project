#ifndef __LTE_PROTOCOL_H
#define __LTE_PROTOCOL_H
#include <stdint.h>
#define HEADERH		0X55
#define HEADERL		0XAA
#define R_OK		0
#define R_FAIL  -1
#define BLE_VER "1.0"

#define  DEBUG_EN  1


struct adv_param_s {
		uint16_t adv_intv_min; 
		uint16_t adv_intv_max;	
};
struct adv_con_param_s {
		uint16_t min_intv;
		uint16_t max_intv;
		uint16_t slave_latency;
		uint16_t timeout;
};
struct ble_adv_con_param_s {
		struct adv_param_s adv_param;
		struct adv_con_param_s adv_con_param;
};

extern uint8_t key_close_open_flag;
enum {
	LTE_SET_ADV_DATA = 0x0C,
	LTE_GET_MAC = 0x01,
	LTE_SCAN_RSP = 0x02,
	LTE_GET_VER = 0x03,
	LTE_START_ADV = 0x04,
	LTE_CLOSE_CON = 0x05,
	LTE_SEND_DATA = 0x06,
	LTE_OPEN_KEY = 0x07,
	LTE_CLOSE_KEY = 0x08,
	LTE_ADV_PARAM = 0x09,
	LTE_CON_PARAM = 0X0A,
	LTE_STOP_ADV = 0X0B,
	LTE_VIRT_AT = 0X0D,
	LTE_DELETE_BONDE = 0X0F,
	LTE_ENTER_SLEEP = 0X0E,
	LTE_OTA_START = 0X10,
	LTE_OTA_DATA = 0X11,
	LTE_OTA_END = 0X12,
};
void ble_rcv_parse(void);
void ble_send_cmd(uint8_t cmd, uint8_t ok);
void ble_send_data(uint8_t cmd, uint8_t *data, uint16_t len);
#endif