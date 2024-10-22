/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */


/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <stdlib.h>

#include "gap_api.h"
#include "os_timer.h"
#include "os_msg_q.h"
#include "os_task.h"
#include "co_printf.h"
#include "sys_utils.h"
#include "driver_pmu.h"
#include "driver_system.h"
#include "driver_flash.h"
#include "driver_uart.h"
#include "aes.h"
#include "lte_protocol.h"
#include "simple_gatt_service.h"

#include "car_key_protocol.h"

/*
 * MACROS (宏定义)
 */
#define PHONE_CHECK_ID_LEN      16
#define ACCREDIT_PHONE_MAX      8

#define DEV_MSG_STORE_OFFSET    0x70000
#define RSSI_ADJUST_VAL         0
#define DERICTION_CONFIRM_TIME  100 // MS   
#define DERICTION_OPTION_TIME   300 // MS


//#define RSSI_LVL_MAX            128
/*
 * CONSTANTS (常量定义)
 */

/*
 * TYPEDEFS (类型定义)
 */
enum{
    RECV_CHECK_ID,
    CMD_OPEN_LOCK,
    CMD_CLOSE_LOCK,
    CMD_SEARCH_CAR,
    CMD_OPEN_TRUNK,
    CMD_OPEN_LOCK_LVL,
    CMD_DOOR_AUTO_CTRL,
    CMD_SET_SHARE = 0x0f,
};
#if 1
enum{
    RSSI_LVL_1 = 1,
    RSSI_LVL_2,
    RSSI_LVL_3,
    RSSI_LVL_MAX,
};
#endif
enum{
    RSSI_TREND_INIT,
    KEEP_AWAY_FROM_CAR,
    CLOSE_TO_CAR,
};

typedef struct dev_store_msg
{
    uint8_t rssi_forward_level;
    uint8_t rssi_backward_level;
    uint8_t auto_door_ctrl_flag;
    uint8_t accredit_id_valid[ACCREDIT_PHONE_MAX];
    uint8_t accredit_dev_id[ACCREDIT_PHONE_MAX][PHONE_CHECK_ID_LEN];
}dev_store_msg_t;
/*
 * GLOBAL VARIABLES (全局变量)
 */

/*
 * LOCAL VARIABLES (本地变量)
 */
static dev_store_msg_t dev_store_msg_p;

static uint8_t dev_con_flag = 0,dev_con_ready = 0;
static uint8_t accredit_check_ok = 0; // bit0-accredit ID valid     bit1-accredit ID check ok
static int rssi_cur = 0;
static uint8_t rssi_change_trend = RSSI_TREND_INIT; // 1--keep away from car  2--close to car
static uint8_t rssi_forward_cnt = 0,rssi_backward_cnt = 0;
static uint8_t car_door_open_flag = 0; // 0-close 1-open
static uint32_t rssi_change_time = 0;
static uint16_t rssi_car_door_task_id;
static uint16_t key_direction_confirm_cnt = 5,key_direction_option_cnt = 10;

car_key_con_s car_key_con;
/*
 * PUBLIC FUNCTIONS (全局函数)
 */

void start_car_door_ctrl_timer(uint8_t open_close);
void app_decrypt_data(uint8_t * data);

void dev_discon_ctrl(void)
{
    dev_con_ready = 0;
    key_direction_confirm_cnt = 5;
    key_direction_option_cnt = 10;
}

void dev_set_con_interval(uint16_t c_interval,uint8_t min_interval)
{
    if(min_interval < 20)
    {
        dev_con_ready = 1;
        co_printf("=dev_con_ready=\r\n");
    }
    else
    {
        dev_con_ready = 0;
        co_printf("=dev_con_notready=\r\n");
    }
    key_direction_confirm_cnt = DERICTION_CONFIRM_TIME/((float)5*c_interval/4);
    key_direction_option_cnt = DERICTION_OPTION_TIME/((float)5*c_interval/4);
    co_printf("confirm_cnt:%d,option_cnt:%d\r\n",key_direction_confirm_cnt,key_direction_option_cnt);
}



void dev_msg_store(void)
{
    flash_erase(DEV_MSG_STORE_OFFSET,0x1000);
    flash_write(DEV_MSG_STORE_OFFSET,sizeof(dev_store_msg_p),(uint8_t *)&dev_store_msg_p);
}

void dev_msg_init(void)
{
    flash_read(DEV_MSG_STORE_OFFSET,sizeof(dev_store_msg_p),(uint8_t *)&dev_store_msg_p);

    if(dev_store_msg_p.rssi_forward_level > RSSI_LVL_MAX)
        dev_store_msg_p.rssi_forward_level = RSSI_LVL_1;
    if(dev_store_msg_p.rssi_backward_level > RSSI_LVL_MAX)
        dev_store_msg_p.rssi_backward_level = RSSI_LVL_1;
    if(dev_store_msg_p.auto_door_ctrl_flag == 0xff)
        dev_store_msg_p.auto_door_ctrl_flag = 1;
    
    //for(uint8_t i = 0;i < ACCREDIT_PHONE_MAX;i++)
    {
        if(dev_store_msg_p.accredit_id_valid[0] == 0x01)
            accredit_check_ok = 0x01;
        else
        {
            accredit_check_ok = 0x00;
        }
    }

    co_printf("rssi forward lvl:%d,back lvl:%d,accredit_ID:%x\r\n",dev_store_msg_p.rssi_forward_level,\
        dev_store_msg_p.rssi_backward_level,accredit_check_ok);
}


__attribute__((section("ram_code"))) void dev_rssi_send_event(uint8_t open_close_door)
{
   car_door_open_flag = open_close_door;
	 if(open_close_door) {
			car_key_con.flag = 1;
			ble_send_cmd(LTE_OPEN_KEY, 0);
	 }
	 else {
		 car_key_con.flag = 2;
		 ble_send_cmd(LTE_CLOSE_KEY, 0);
	 }
	 car_key_con.t = 0;
	 car_key_con.cnt =3;
	 car_key_con.time = 3000;
	 car_key_con.cur_time = system_get_curr_time();
}

__attribute__((section("ram_code"))) void dev_rssi_trend_cal_func(int8_t rssi)
{
    uint8_t open_or_close_door = 0;
    uint32_t during_time = 0;
    if(!dev_con_ready || (dev_store_msg_p.auto_door_ctrl_flag==0))
    {
        rssi_forward_cnt = 0;
        rssi_backward_cnt = 0;
        return;
    }

    if(((rssi - rssi_cur) > RSSI_ADJUST_VAL) &&\
        ((rssi_change_trend == RSSI_TREND_INIT) || (rssi_change_trend == KEEP_AWAY_FROM_CAR)))
    {
//        uart_putc_noint_no_wait(UART1,'O');
        rssi_forward_cnt++;
        if(rssi_forward_cnt > key_direction_confirm_cnt)
            rssi_backward_cnt = 0;
        if(rssi_forward_cnt > key_direction_option_cnt)
        {
            rssi_forward_cnt = 0;

            /************change trend ctrl**************/
            if((rssi_change_trend == RSSI_TREND_INIT) || (rssi_change_trend == KEEP_AWAY_FROM_CAR))
            {
                rssi_change_trend = CLOSE_TO_CAR;
                rssi_change_time = system_get_curr_time();
            }
            /************change trend ctrl**************/

            
        }
    }
    else if(((rssi_cur - rssi) > RSSI_ADJUST_VAL) &&\
        ((rssi_change_trend == RSSI_TREND_INIT) || (rssi_change_trend == CLOSE_TO_CAR)))
    {
//        uart_putc_noint_no_wait(UART1,'C');
        //rssi_cur = rssi;
        rssi_backward_cnt++;
        if(rssi_backward_cnt > key_direction_confirm_cnt)
            rssi_forward_cnt = 0;
        if(rssi_backward_cnt > key_direction_option_cnt)
        {
            rssi_backward_cnt = 0;

            /************change trend ctrl**************/
            if((rssi_change_trend == RSSI_TREND_INIT) || (rssi_change_trend == CLOSE_TO_CAR))
            {
                rssi_change_trend = KEEP_AWAY_FROM_CAR;
                rssi_change_time = system_get_curr_time();
            }
            /************change trend ctrl**************/

            
        }
    }
    rssi_cur = rssi;

    switch(rssi_change_trend)
    {
        case CLOSE_TO_CAR:
        {
            #if 0
            if(rssi_cur > dev_store_msg_p.rssi_forward_level)
                open_or_close_door = 1;
            #else
            switch(dev_store_msg_p.rssi_forward_level)
            {
                case RSSI_LVL_1:
                    if(rssi_cur > -68)
                        open_or_close_door = 1;
                    break;
                case RSSI_LVL_2:
                    if(rssi_cur > -80)
                        open_or_close_door = 1;
                    break;
                case RSSI_LVL_3:
                    if(rssi_cur > -85)
                        open_or_close_door = 1;
                    break;
                default:
                    break;    
            }
            #endif
            if((!car_door_open_flag) && (open_or_close_door == 1))
            {
                during_time = system_get_curr_time();
                if(during_time < rssi_change_time)
                {
                    during_time = during_time+0x4FFFFFF-rssi_change_time;
                }
                else
                    during_time = during_time-rssi_change_time;
                if(during_time > 1000) // > 1s
                    dev_rssi_send_event(1);
            }
        }
        break;
        case KEEP_AWAY_FROM_CAR:
        {
            #if 0
            if(rssi_cur < dev_store_msg_p.rssi_backward_level)
                open_or_close_door = 2;
            #else
            switch(dev_store_msg_p.rssi_backward_level)
            {
                case RSSI_LVL_1:
                    if(rssi_cur < -80)
                        open_or_close_door = 2;
                    break;
                case RSSI_LVL_2:
                    if(rssi_cur < -90)
                        open_or_close_door = 2;
                    break;
                case RSSI_LVL_3:
                    if(rssi_cur < -95)
                        open_or_close_door = 2;
                    break;
                default:
                    break;    
            }
            #endif
            if((car_door_open_flag) && (open_or_close_door == 2))
            {
                during_time = system_get_curr_time();
                if(during_time < rssi_change_time)
                {
                    during_time = during_time+0x4FFFFFF-rssi_change_time;
                }
                else
                    during_time = during_time-rssi_change_time;
                if(during_time > 1000) // > 1s
                    dev_rssi_send_event(0);
            }
        }
        break;
    }
}



/*****************for aes***************/

void dev_ble_protocol_init(void)
{    
    dev_msg_init();
}



