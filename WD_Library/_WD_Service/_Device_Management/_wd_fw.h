/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_fw.h
 *
 *  Created on: Jan 13, 2025
 *      Author: doubled
 */

#ifndef WD_SERVICE_WD_FW_H
#define WD_SERVICE_WD_FW_H

#include "wd.h"

#if 1 // 토글


/***************************************************************************************************************************************************************
 **********  Firmware API
 **************************************************************************************************************************************************************/
typedef enum {
    WD_FIRMWARE_UPDATE_TRANSFER_BLOCK_SIZE = 512,

    WD_FIRMWARE_UPDATE_TRANSFER_READY = 0,
    WD_FIRMWARE_UPDATE_TRANSFER_INITIATE = 1,
    WD_FIRMWARE_UPDATE_TRANSFER_SEND = 2,
    WD_FIRMWARE_UPDATE_TRANSFER_VERIFICATION = 3,
    WD_FIRMWARE_UPDATE_TRANSFER_ACTIVATION = 4,
} wd_fw_update_transfer_table_t;

extern wd_firmware_t wd_fw_data;
extern wd_firmware_header_t *const wd_fw_header_app;
extern wd_firmware_header_t *const wd_fw_header_app_down;
extern wd_firmware_header_t *const wd_fw_header_app_factory;
extern wd_firmware_fota_info_t wd_fw_fota_info;

void wd_fw_default(void);
void wd_fw_print_image(wd_firmware_header_t *header, char *title);
int32_t wd_fw_check_image(wd_firmware_header_t *header, uint32_t addr, uint32_t *size);
void wd_fw_initialization(void);
void wd_fw_boot_success_mark(void);


#endif //#if 1 // 토글

#endif /* WD_SERVICE_WD_FW_H */
