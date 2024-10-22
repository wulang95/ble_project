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
#include <stdbool.h>
#include "os_timer.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "driver_gpio.h"
#include "simple_gatt_service.h"
#include "ota_service.h"
#include "ble_simple_peripheral.h"
#include "driver_pmu.h"
#include "sys_utils.h"
#include "flash_usage_config.h"
#include "car_key_protocol.h"
#include "dev_info_service.h"
#include "hid_service.h"
#include "driver_uart.h"
#include "lte_protocol.h"
/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
// GAP-广播包的内容,最长31个字节.短一点的内容可以节省广播时的系统功耗.
static uint8_t scan_rsp_data[] =
{
  // service UUID, to notify central devices what services are included
  // in this peripheral. 告诉central本机有什么服务, 但这里先只放一个主要的.
//  0x11,   // length of this data
//  GAP_ADVTYPE_128BIT_COMPLETE,      // some of the UUID's, but not all
//  0xa0, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07,
//  0x07, 0x83, 0xb0, 0x3e, 0x85, 0x35, 0xb5, 0xa0, 0x71, 0x40, 0xa3, 0x04, 0xd2, 0x49, 0x5c, 0xa0
    // appearance
    0x03,   // length of this data
    GAP_ADVTYPE_APPEARANCE,
    LO_UINT16(GAP_APPEARE_GENERIC_HID),
    HI_UINT16(GAP_APPEARE_GENERIC_HID),

    // service UUIDs, to notify central devices what services are included
    // in this peripheral. 告诉central本机有什么服务, 但这里先只放一个主要的.
    0x03,   // length of this data
    GAP_ADVTYPE_16BIT_COMPLETE,
    LO_UINT16(HID_SERV_UUID),
    HI_UINT16(HID_SERV_UUID),
//  0x09,
//  0xff,
//  0xff,0xff,
//  0x12,0x34,0x56,0x78,0xab,0xc0,
};

// GAP - Scan response data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
// GAP-Scan response内容,最长31个字节.短一点的内容可以节省广播时的系统功耗.

static uint8_t adv_data[] =
{
  // complete name 设备名字
  0x12,   // length of this data
  GAP_ADVTYPE_LOCAL_NAME_COMPLETE,
  'E','N','G','W','E','e','_','P','e','r','i','p','h','e','r','a','l',

  // Tx power level 发射功率
  0x02,   // length of this data
  GAP_ADVTYPE_POWER_LEVEL,
  0,	   // 0dBm
};
static struct adv_data_d{
		uint8_t data_d[31];
		uint16_t len;
} adv_d;
static struct adv_data_s{
		uint8_t data_s[31];
		uint16_t len;
}adv_s;
void set_dev_data(uint8_t *data, uint16_t len)
{
		uint16_t i = 0;
		memcpy(&adv_d.data_d[0], data, len);
	printf("adv_data:\r\n");
		for(i = 0; i < len; i++){
			co_printf("%02x\t", adv_d.data_d[i]);
		}
		adv_d.len = len;
}

void set_dev_sacn_data(uint8_t *data, uint16_t len)
{
		memcpy(&adv_s.data_s[0], data, len);
		adv_s.len = len;
}
void adv_disconnect()
{
		gap_disconnect_req(0);
}

static struct ble_adv_con_param_s ble_adv_con_param;

void ble_adv_param_init()
{
	
		ble_adv_con_param.adv_param.adv_intv_min = 600;
		ble_adv_con_param.adv_param.adv_intv_max = 600;
		ble_adv_con_param.adv_con_param.min_intv = 12;
		ble_adv_con_param.adv_con_param.max_intv = 12;
		ble_adv_con_param.adv_con_param.slave_latency = 0;
		ble_adv_con_param.adv_con_param.timeout = 800;
		memcpy(adv_d.data_d, adv_data, sizeof(adv_data));
		adv_d.len = sizeof(adv_data);
		memcpy(adv_s.data_s, scan_rsp_data, sizeof(scan_rsp_data));
		adv_s.len = sizeof(scan_rsp_data);
} 
void set_ble_adv_param(uint16_t min, uint16_t max)
{
		ble_adv_con_param.adv_param.adv_intv_min = min;
		ble_adv_con_param.adv_param.adv_intv_max = max;
}

void set_ble_adv_con_param(uint16_t min, uint16_t max, uint16_t lay, uint16_t timeout)
{
		ble_adv_con_param.adv_con_param.min_intv = min;
		ble_adv_con_param.adv_con_param.max_intv = max;
		ble_adv_con_param.adv_con_param.slave_latency = lay;
		ble_adv_con_param.adv_con_param.timeout = timeout;
}

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */

/*
 * LOCAL VARIABLES (本地变量)
 */
os_timer_t update_param_timer;
static uint8_t rssi_filter_id;
os_timer_t sp_timer;
 
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

void param_timer_func(void *arg)
{
    co_printf("param_timer_func\r\n");
    gatt_mtu_exchange_req(0);
    gap_conn_param_update(0, ble_adv_con_param.adv_con_param.min_intv,\
		ble_adv_con_param.adv_con_param.max_intv, \
		ble_adv_con_param.adv_con_param.slave_latency,\
		ble_adv_con_param.adv_con_param.timeout);
}
__attribute__((section("ram_code"))) void gap_rssi_ind(int8_t rssi, uint8_t conidx)
{
    int rssi_t;

    rssi_t = (int)Smoothing_Filter_handle(rssi_filter_id,rssi);
    dev_rssi_trend_cal_func(rssi_t);
    
    static uint16_t cnt = 0;
    cnt++;
    if(cnt > 39)
    {
        cnt = 0;
        //co_printf("rssi: link=%d, rssi=%d.\r\n", conidx, rssi_t);
   //     co_printf("r:%d  ",rssi_t);
        //if(rssi_t > check_rssi)
        //    co_printf("==rssi ok==\r\n");
    }
}
void adv_stop()
{
	gap_stop_advertising();
}

/*********************************************************************
 * @fn      app_gap_evt_cb
 *
 * @brief   Application layer GAP event callback function. Handles GAP evnets.
 *
 * @param   p_event - GAP events from BLE stack.
 *       
 *
 * @return  None.
 */
void app_gap_evt_cb(gap_event_t *p_event)
{
    switch(p_event->type)
    {
        case GAP_EVT_ADV_END:
        {
            co_printf("adv_end,status:0x%02x\r\n",p_event->param.adv_end.status);
            
        }
        break;
        
        case GAP_EVT_ALL_SVC_ADDED:
        {
            co_printf("All service added\r\n");
        }
        break;

        case GAP_EVT_SLAVE_CONNECT:
        {
            co_printf("slave[%d],connect. link_num:%d\r\n",p_event->param.slave_connect.conidx,gap_get_connect_num());
            //gatt_mtu_exchange_req(p_event->param.slave_connect.conidx);
            os_timer_start(&update_param_timer,2000,0);
						pmu_set_gpio_value(GPIO_PORT_D, BIT(5), 1);
            //gap_security_req(p_event->param.slave_connect.conidx);
        }
        break;

        case GAP_EVT_DISCONNECT:
        {
            co_printf("Link[%d] disconnect,reason:0x%02X\r\n",p_event->param.disconnect.conidx
                      ,p_event->param.disconnect.reason);
            dev_discon_ctrl();
						pmu_set_gpio_value(GPIO_PORT_D, BIT(5), 0);
						os_timer_stop(&update_param_timer);
            gap_start_advertising(0);
        }
        break;

        case GAP_EVT_LINK_PARAM_REJECT:
            co_printf("Link[%d]param reject,status:0x%02x\r\n"
                      ,p_event->param.link_reject.conidx,p_event->param.link_reject.status);
            break;

        case GAP_EVT_LINK_PARAM_UPDATE:
            co_printf("Link[%d]param update,interval:%d,latency:%d,timeout:%d\r\n",p_event->param.link_update.conidx
                      ,p_event->param.link_update.con_interval,p_event->param.link_update.con_latency,p_event->param.link_update.sup_to);
            if(p_event->param.link_update.con_latency)
            {
                dev_set_con_interval(p_event->param.link_update.con_interval*p_event->param.link_update.con_latency,\
                                    p_event->param.link_update.con_interval);
            }
            else
            {
                dev_set_con_interval(p_event->param.link_update.con_interval,p_event->param.link_update.con_interval);
            }
            break;

        case GAP_EVT_PEER_FEATURE:
            co_printf("peer[%d] feats ind\r\n",p_event->param.peer_feature.conidx);
            //show_reg((uint8_t *)&(p_event->param.peer_feature.features),8,1);
            break;

        case GAP_EVT_MTU:
            co_printf("mtu update,conidx=%d,mtu=%d\r\n"
                      ,p_event->param.mtu.conidx,p_event->param.mtu.value);
            break;
        
        case GAP_EVT_LINK_RSSI:
            co_printf("link rssi %d\r\n",p_event->param.link_rssi);
            break;
                
        case GAP_SEC_EVT_SLAVE_ENCRYPT:
            co_printf("slave[%d]_encrypted\r\n",p_event->param.slave_encrypt_conidx);
			os_timer_start(&update_param_timer,4000,0);
				
            break;

        default:
            break;
    }
}



/*********************************************************************
 * @fn      sp_start_adv
 *
 * @brief   Set advertising data & scan response & advertising parameters and start advertising
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void sp_start_adv(void)
{
		uint16_t i;
		uint8_t ble_name[20] = {0}; 
		uint8_t name_len;
    // Set advertising parameters
    gap_adv_param_t adv_param;
    adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_param.adv_intv_min = ble_adv_con_param.adv_param.adv_intv_min;
    adv_param.adv_intv_max = ble_adv_con_param.adv_param.adv_intv_max; 
        
    gap_set_advertising_param(&adv_param);

//    mac_addr_t g_addr;
//    uint8_t adv_data_len = sizeof(adv_data);
//    gap_address_get(&g_addr);
//    for(uint8_t i = 0;i < 6;i++)
//        adv_data[adv_data_len-1-i] = g_addr.addr[i];
    // Set advertising data & scan response data
	name_len = adv_d.data_d[0];
	name_len -= 1;
	memcpy(ble_name, &adv_d.data_d[2], name_len);
	gap_set_dev_name(ble_name, name_len);
		co_printf("adv_data[%d]:\r\n", adv_d.len);
		for(i = 0; i < adv_d.len; i++){
				co_printf("%02x\t", adv_d.data_d[i]);
		}
		gap_set_advertising_data(adv_d.data_d, adv_d.len);
		co_printf("adv_scan[%d]:\r\n", adv_s.len);
		for(i = 0; i < adv_s.len; i++){
				co_printf("%02x\t", adv_s.data_s[i]);
		}
		co_printf("\r\n");
		gap_set_advertising_rsp_data(adv_s.data_s, adv_s.len);
    // Start advertising
		co_printf("Start advertising...\r\n");
		gap_start_advertising(0);

		gap_set_link_rssi_report(true);
}


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
void simple_peripheral_init(void)
{
		char mac_str[24] = {0};
    // set local device name
    os_timer_init( &update_param_timer,param_timer_func,NULL);
		ble_adv_param_init();
#if 0		//security encryption
    gap_security_param_t param =
    {
        .mitm = true,		
        .ble_secure_conn = true,		//not enable security encryption
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,		//ble device has input ability, will input pin code. 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,	//need bond
        .bond_auth = true,	//need bond auth
    };
#endif 
#if 0
    gap_security_param_t param =
    {
        .mitm = true,		// use PIN code during bonding
        .ble_secure_conn = false,		//not enable security encryption
        .io_cap = GAP_IO_CAP_KEYBOARD_ONLY,		//ble device has input ability, will input pin code. 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,	//need bond
        .bond_auth = true,	//need bond auth
    };
#endif
#if 1
    gap_security_param_t param =
    {
        .mitm = true,		// use PIN code during bonding
        .ble_secure_conn = false,		//not enable security encryption
        .io_cap = GAP_IO_CAP_DISPLAY_ONLY,	//ble device has output ability, will show pin code. 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ, //need bond
        .bond_auth = true,	//need bond auth
        .password = 123456,	//set PIN code, it is a dec num between 100000 ~ 999999
    };
#endif
#if 0
    gap_security_param_t param =
    {
        .mitm = false,	// dont use PIN code during bonding
        .ble_secure_conn = false,	//not enable security encryption
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT, //ble device has neither output nor input ability, 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,		//need bond
        .bond_auth = true,	//need bond auth
        .password = 0,
    };
#endif
		// Initialize security related settings.
    gap_security_param_init(&param);
    
    gap_set_cb_func(app_gap_evt_cb);

    Smoothing_Filter_init(&rssi_filter_id);
		//enable bond manage module, which will record bond key and peer service info into flash. 
		//and read these info from flash when func: "gap_bond_manager_init" executes.
    gap_bond_manager_init(BLE_BONDING_INFO_SAVE_ADDR, BLE_REMOTE_SERVICE_SAVE_ADDR, 8, true);
    //gap_bond_manager_delete_all();
    gap_set_dev_appearance(GAP_APPEARE_GENERIC_HID);
    
    mac_addr_t addr;
    gap_address_get(&addr);
    co_printf("Local BDADDR: 0x%02X%02X%02X%02X%02X%02X\r\n", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);
		sprintf(mac_str, "mac:%02X%02X%02X%02X%02X%02X", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);
    co_printf(mac_str);
		uart_put_data_noint(UART0, mac_str, sizeof(mac_str));
		dev_ble_protocol_init();
    // Adding services to database
    sp_gatt_add_service();  
    hid_gatt_add_service();
	//	sp_start_adv();
}


