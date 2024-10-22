/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */

#ifndef CAR_KEY_PROTOCOL_H
#define CAR_KEY_PROTOCOL_H

/*
 * INCLUDES (包含头文件)
 */
#include <stdint.h>
typedef struct {
		uint8_t flag;
		uint8_t cnt;
		uint8_t t;
		uint32_t time;
		uint32_t cur_time;
} car_key_con_s;

extern car_key_con_s car_key_con;


/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */

/*
 * LOCAL VARIABLES (本地变量)
 */


/*
 * PUBLIC FUNCTIONS (全局函数)
 */

void dev_discon_ctrl(void);

void dev_set_con_interval(uint16_t c_interval,uint8_t min_interval);

void dev_rssi_trend_cal_func(int8_t rssi);

void app_cmd_send_analyze_task(uint8_t * data,uint8_t len);

void dev_ble_protocol_init(void);


#endif



