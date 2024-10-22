/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */

#ifndef SP_GATT_PROFILE_H
#define SP_GATT_PROFILE_H

/*
 * INCLUDES (����ͷ�ļ�)
 */
#include <stdio.h>
#include <string.h>
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"


/*
 * MACROS (�궨��)
 */

/*
 * CONSTANTS (��������)
 */
// Simple Profile attributes index. 
enum
{
    ENG_IDX_SERVICE,

    ENG_R_IDX_CHAR1_DECLARATION,
    ENG_R_CHAR1_VALUE,
		ENG_R_CHAR1_USER_CFG,
    ENG_R_CHAR1_USER_DESCRIPTION,

    ENG_W_CHAR2_DECLARATION,
    ENG_W_CHAR2_VALUE,
    ENG_W_CHAR2_USER_DESCRIPTION,
    
		ENG_AT_IDX_CHAR3_DECLARATION,
		ENG_AT_CHAR3_VALUE,
		ENG_AT_CHAR3_USER_CFG,
    ENG_AT_CHAR3_USER_DESCRIPTION,

    ENG_IDX_NB,
};

// Simple GATT Profile Service UUID
// Simple GATT Profile Service UUID
#define ENG_SVC_UUID              0x1820

#define ENG_CHAR2_TX_UUID            0X1822
#define ENG_CHAR1_RX_UUID            0X1821

#define ENG_CHAR3_AT_UUID				0X1823
#define SP_CHAR3_UUID            0xFFF3
#define SP_CHAR4_UUID            0xFFF4
#define SP_CHAR5_UUID            0xFFF5

/*
 * TYPEDEFS (���Ͷ���)
 */

/*
 * GLOBAL VARIABLES (ȫ�ֱ���)
 */
extern uint8_t sp_svc_id;
/*
 * LOCAL VARIABLES (���ر���)
 */


/*
 * PUBLIC FUNCTIONS (ȫ�ֺ���)
 */

void dev_ntf_data_to_app(uint16_t att_idx, uint8_t * data,uint8_t len);

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
void sp_gatt_add_service(void);



#endif







