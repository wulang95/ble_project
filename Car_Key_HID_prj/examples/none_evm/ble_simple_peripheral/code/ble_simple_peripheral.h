/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
#ifndef BLE_SIMPLE_PERIPHERAL_H
#define BLE_SIMPLE_PERIPHERAL_H

 /*
 * INCLUDES (包含头文件)
 */

/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */
#define SP_TASK_EVT_NOTI    0x0001

/*
 * TYPEDEFS (类型定义)
 */
enum
{
    GATT_SVC_IDX_SP,
    GATT_SVC_NUM    
};



/*
 * GLOBAL VARIABLES (全局变量)
 */



/*
 * LOCAL VARIABLES (本地变量)
 */
 
/*
 * LOCAL FUNCTIONS (本地函数)
 */

/*
 * EXTERN FUNCTIONS (外部函数)
 */

/*
 * PUBLIC FUNCTIONS (全局函数)
 */


/** @function group ble peripheral device APIs (ble外设相关的API)
 * @{
 */
 
void app_gap_evt_cb(gap_event_t *p_event);
void set_dev_data(uint8_t *data, uint16_t len);
void set_dev_sacn_data(uint8_t *data, uint16_t len);
void set_ble_adv_param(uint16_t min, uint16_t max);
void set_ble_adv_con_param(uint16_t min, uint16_t max, uint16_t lay, uint16_t timeout);
/*********************************************************************
 * @fn      simple_peripheral_init
 *
 * @brief   Initialize simple peripheral profile, BLE related parameters.
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void simple_peripheral_init(void);
void sp_start_adv(void);
#endif
