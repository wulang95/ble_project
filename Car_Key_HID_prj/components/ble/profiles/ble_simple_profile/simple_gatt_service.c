/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
 
/*
 * INCLUDES (����ͷ�ļ�)
 */
#include <stdio.h>
#include <string.h>
#include "co_printf.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"

#include "simple_gatt_service.h"
#include "lte_protocol.h"


/*
 * MACROS (�궨��)
 */

/*
 * CONSTANTS (��������)
 */


// Simple GATT Profile Service UUID: 0xFFF0
const uint8_t sp_svc_uuid[] = UUID16_ARR(ENG_SVC_UUID);

/******************************* Characteristic 1 defination *******************************/
// Characteristic 1 UUID: 0xFFF1
// Characteristic 1 data 
#define SP_CHAR1_VALUE_LEN  20
uint8_t sp_char1_value[SP_CHAR1_VALUE_LEN] = {0};
// Characteristic 1 User Description
#define SP_CHAR1_DESC_LEN   17
const uint8_t sp_char1_desc[SP_CHAR1_DESC_LEN] = "Characteristic 1";

/******************************* Characteristic 2 defination *******************************/
// Characteristic 2 UUID: 0xFFF2
// Characteristic 2 data 
#define SP_CHAR2_VALUE_LEN  20
uint8_t sp_char2_value[SP_CHAR2_VALUE_LEN] = {0};
// Characteristic 2 User Description
#define SP_CHAR2_DESC_LEN   17
const uint8_t sp_char2_desc[SP_CHAR2_DESC_LEN] = "Characteristic 2";

/******************************* Characteristic 3 defination *******************************/
#define SP_CHAR3_VALUE_LEN  20
uint8_t sp_char3_value[SP_CHAR3_VALUE_LEN] = {0};
// Characteristic 1 User Description
#define SP_CHAR3_DESC_LEN   17
const uint8_t sp_char3_desc[SP_CHAR3_DESC_LEN] = "Characteristic 3";
/*
 * TYPEDEFS (���Ͷ���)
 */

/*
 * GLOBAL VARIABLES (ȫ�ֱ���)
 */
uint8_t sp_svc_id = 0;
uint8_t ntf_char1_enable = 0;

/*
 * LOCAL VARIABLES (���ر���)
 */
static gatt_service_t simple_profile_svc;

/*********************************************************************
 * Profile Attributes - Table
 * ÿһ���һ��attribute�Ķ��塣
 * ��һ��attributeΪService �ĵĶ��塣
 * ÿһ������ֵ(characteristic)�Ķ��壬�����ٰ�������attribute�Ķ��壻
 * 1. ����ֵ����(Characteristic Declaration)
 * 2. ����ֵ��ֵ(Characteristic value)
 * 3. ����ֵ������(Characteristic description)
 * �����notification ����indication �Ĺ��ܣ��������ĸ�attribute�Ķ��壬����ǰ�涨���������������һ������ֵ�ͻ�������(client characteristic configuration)��
 *
 */

const gatt_attribute_t simple_profile_att_table[ENG_IDX_NB] =
{
    // Simple gatt Service Declaration
    [ENG_IDX_SERVICE]                        =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID) },     /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    UUID_SIZE_2,                                                /* Max size of the value */     /* Service UUID size in service declaration */
                                                    (uint8_t*)sp_svc_uuid,                                      /* Value of the attribute */    /* Service UUID value in service declaration */
                                                },

        // Characteristic 1 Declaration           
        [ENG_R_IDX_CHAR1_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 1 Value                  
        [ENG_R_CHAR1_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(ENG_CHAR1_RX_UUID)},                 /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_NOTI,                           /* Permissions */
                                                    300,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },        
        // Characteristic 4 client characteristic configuration
        [ENG_R_CHAR1_USER_CFG]                  =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
                                                    2,                                           /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                }, 
																								
        // Characteristic 1 User Description
        [ENG_R_CHAR1_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR1_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char1_desc,                                   /* Value of the attribute */
                                                },


        // Characteristic 2 Declaration
        [ENG_W_CHAR2_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 2 Value   
        [ENG_W_CHAR2_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(ENG_CHAR2_TX_UUID)},                 /* UUID */
                                                    GATT_PROP_WRITE,                                             /* Permissions */
                                                    300,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */	/* Can assign a buffer here, or can be assigned in the application by user */
                                                },   
        // Characteristic 2 User Description
        [ENG_W_CHAR2_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },       /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR2_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char2_desc,                                   /* Value of the attribute */
                                                },
				
				[ENG_AT_IDX_CHAR3_DECLARATION]      = 		{
																										{ UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
																								},
				[ENG_AT_CHAR3_VALUE]                =   {
																										{ UUID_SIZE_2, UUID16_ARR(ENG_CHAR3_AT_UUID)},                 /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_NOTI|GATT_PROP_WRITE,                           /* Permissions */
                                                    300,                                         /* Max size of the value */
                                                    NULL,      
																								},
				[ENG_AT_CHAR3_USER_CFG]                  =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
                                                    2,                                           /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                }, 
			  [ENG_AT_CHAR3_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR3_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char3_desc,                                   /* Value of the attribute */
                                                },
};

static void show_reg(uint8_t *data,uint32_t len,uint8_t dbg_on)
{
    uint32_t i=0;
    if(len == 0 || (dbg_on==0)) return;
    for(; i<len; i++)
    {
        co_printf("0x%02X,",data[i]);
    }
    co_printf("\r\n");
}

void dev_ntf_data_to_app(uint16_t att_idx, uint8_t * data,uint8_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = att_idx;
    ntf_att.conidx = 0;
    ntf_att.svc_id = sp_svc_id;
    ntf_att.data_len = len;
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
    show_reg(data,len,0);
}

void ntf_data(uint8_t con_idx,uint8_t att_idx,uint8_t *data,uint16_t len)
{
		gatt_ntf_t ntf_att;
		ntf_att.att_idx = att_idx;
		ntf_att.conidx = con_idx;
		ntf_att.svc_id = sp_svc_id;
		ntf_att.data_len = len;
		ntf_att.p_data = data;
		gatt_notification(ntf_att);
}
/*********************************************************************
 * @fn      sp_gatt_msg_handler
 *
 * @brief   Simple Profile callback funtion for GATT messages. GATT read/write
 *			operations are handeled here.
 *
 * @param   p_msg       - GATT messages from GATT layer.
 *
 * @return  uint16_t    - Length of handled message.
 */
static uint16_t sp_gatt_msg_handler(gatt_msg_t *p_msg)
{
    co_printf("evt:%x,att_idx:%x\r\n",p_msg->msg_evt,p_msg->att_idx);
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_READ_REQ:
				{
/*********************************************************************
 * @brief   Simple Profile user application handles read request in this callback.
 *			Ӧ�ò�������ص��������洦���������
 *
 * @param   p_msg->param.msg.p_msg_data  - the pointer to read buffer. NOTE: It's just a pointer from lower layer, please create the buffer in application layer.
 *					  ָ�����������ָ�롣 ��ע����ֻ��һ��ָ�룬����Ӧ�ó����з��仺����. Ϊ�������, ���Ϊָ���ָ��.
 *          p_msg->param.msg.msg_len     - the pointer to the length of read buffer. Application to assign it.
 *                    ���������ĳ��ȣ��û�Ӧ�ó���ȥ������ֵ.
 *          p_msg->att_idx - index of the attribute value in it's attribute table.
 *					  Attribute��ƫ����.
 *
 * @return  ������ĳ���.
 */					
					
					if(p_msg->att_idx == ENG_R_CHAR1_VALUE)
					{
						memcpy(p_msg->param.msg.p_msg_data, "ENG_R_CHAR1_VALUE", strlen("ENG_R_CHAR1_VALUE"));
						return strlen("ENG_R_CHAR1_VALUE");
					}
				}
            break;
        
        case GATTC_MSG_WRITE_REQ:
				{
/*********************************************************************
 * @brief   Simple Profile user application handles write request in this callback.
 *			Ӧ�ò�������ص��������洦��д������
 *
 * @param   p_msg->param.msg.p_msg_data   - the buffer for write
 *			              д����������.
 *					  
 *          p_msg->param.msg.msg_len      - the length of write buffer.
 *                        д�������ĳ���.
 *          att_idx     - index of the attribute value in it's attribute table.
 *					      Attribute��ƫ����.
 *
 * @return  д����ĳ���.
 */				
                    
					if (p_msg->att_idx == ENG_AT_CHAR3_VALUE)
					{
							ble_send_data(LTE_VIRT_AT, p_msg->param.msg.p_msg_data, p_msg->param.msg.msg_len);
							co_printf("char3_recv:");
							show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,0);
					}
					else if (p_msg->att_idx == ENG_W_CHAR2_VALUE)
					{
							ble_send_data(LTE_SEND_DATA, p_msg->param.msg.p_msg_data, p_msg->param.msg.msg_len);
							co_printf("char2_recv:");
							show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,0);
					}
				}
            break;
        case GATTC_MSG_LINK_CREATE:
						co_printf("link_created\r\n");
            break;
        case GATTC_MSG_LINK_LOST:
						co_printf("link_lost\r\n");
						ntf_char1_enable = 0;
            break;    
        default:
            break;
    }
    return p_msg->param.msg.msg_len;
}

/*********************************************************************
 * @fn      sp_gatt_add_service
 *
 * @brief   Simple Profile add GATT service function.
 *			���GATT service��ATT�����ݿ����档
 *
 * @param   None. 
 *        
 *
 * @return  None.
 */
void sp_gatt_add_service(void)
{
	simple_profile_svc.p_att_tb = simple_profile_att_table;
	simple_profile_svc.att_nb = ENG_IDX_NB;
	simple_profile_svc.gatt_msg_handler = sp_gatt_msg_handler;
	
	sp_svc_id = gatt_add_service(&simple_profile_svc);
}





