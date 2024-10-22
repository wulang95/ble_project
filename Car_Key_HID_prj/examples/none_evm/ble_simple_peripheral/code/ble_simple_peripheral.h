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
 * INCLUDES (����ͷ�ļ�)
 */

/*
 * MACROS (�궨��)
 */

/*
 * CONSTANTS (��������)
 */
#define SP_TASK_EVT_NOTI    0x0001

/*
 * TYPEDEFS (���Ͷ���)
 */
enum
{
    GATT_SVC_IDX_SP,
    GATT_SVC_NUM    
};



/*
 * GLOBAL VARIABLES (ȫ�ֱ���)
 */



/*
 * LOCAL VARIABLES (���ر���)
 */
 
/*
 * LOCAL FUNCTIONS (���غ���)
 */

/*
 * EXTERN FUNCTIONS (�ⲿ����)
 */

/*
 * PUBLIC FUNCTIONS (ȫ�ֺ���)
 */


/** @function group ble peripheral device APIs (ble������ص�API)
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
