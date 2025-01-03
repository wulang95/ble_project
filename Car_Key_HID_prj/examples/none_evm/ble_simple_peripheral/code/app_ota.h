#ifndef  __APP_OTA_H
#define  __APP_OTA_H
#include <stdint.h>
#include "driver_plf.h"
__attribute__((section("ram_code"))) uint8_t app_get_ota_state(void);
__attribute__((section("ram_code"))) void app_set_ota_state(uint8_t state_flag);
void app_otas_save_first_pkt(uint32_t dest,uint8_t *src,uint32_t len);
void app_ota_erase(uint32_t adr, uint32_t len);
uint32_t app_otas_get_storage_address(void);
void app_otas_save_data(uint32_t dest, uint8_t *src, uint32_t len);
unsigned int ota_data_check(uint32_t firmware_length,uint32_t new_bin_addr);
__attribute__((section("ram_code")))void app_otas_flash_read(uint32_t dest, uint8_t *src, uint32_t len);
uint32_t app_otas_get_curr_firmwave_version(void);
uint32_t app_otas_get_curr_code_address(void);














#endif