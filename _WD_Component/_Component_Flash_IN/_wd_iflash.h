/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_iflash.h
 *
 *  Created on: Feb 7, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_IFLASH_H
#define WD_COMPONENT_WD_IFLASH_H

#include "wd_core.h"
#include "_wd_blockdev.h"

#if 1 // 토글

#define RTE_BLOCK_DEVICE_I_FLASH_ADDR              ((uint32_t)FLASH_BASE)
#define RTE_BLOCK_DEVICE_I_FLASH_SIZE              ((uint32_t)(2 * 1024 * 1024))
#define RTE_BLOCK_DEVICE_I_FLASH_PROGRAM_SIZE      (32u) // ★ 32 bytes
#define RTE_BLOCK_DEVICE_I_FLASH_ERASE_SECTOR_SIZE FLASH_SECTOR_SIZE
#define RTE_BLOCK_DEVICE_I_FLASH_ERASE_BLOCK_SIZE  FLASH_SECTOR_SIZE
#define RTE_BLOCK_DEVICE_I_FLASH_ERASE_BLOCK_ADDR  0UL

#define RTE_BLOCK_DEVICE_I_FLASH_PROGRAM_TIMEOUT      ((uint32_t)100000)   /* us */
#define RTE_BLOCK_DEVICE_I_FLASH_ERASE_SECTOR_TIMEOUT ((uint32_t)6000000)  /* us */
#define RTE_BLOCK_DEVICE_I_FLASH_ERASE_BLOCK_TIMEOUT  ((uint32_t)6000000)  /* us */
#define RTE_BLOCK_DEVICE_I_FLASH_ERASE_CHIP_TIMEOUT   ((uint32_t)30000000) /* us */

extern wd_blockdev_instance_t wd_drv_block_device_i_flash;
extern wd_blockdev_instance_info_t *const wd_iflash_res_block_device_i_flash;

#ifndef NDEBUG
typedef struct wd_debug_block_device_i_flash_s {
    wd_blockdev_instance_resources_t *Resource;
    wd_blockdev_instance_info_t *Info;
    wd_blockdev_instance_t *Driver;
} const wd_iflash_debug_block_device_i_flash_t;

extern wd_iflash_debug_block_device_i_flash_t wd_iflash_dbg_block_device_i_flash;
#endif

// 섹터 CRC를 “한 번” 최종화 (필요 시 상위에서 호출 가능)
int32_t wd_iflash_finalize_cr_cs(uint32_t addr, uint32_t len);

// Factory 이미지처럼 자체 wd_toolkit_crc32(wd_firmware_header_t)를 이미 갖고 있어 섹터별 예약 CRC(마지막 4바이트)가
// 필요/가능하지 않은 다중 섹터 원본 그대로 쓰기 전용 경로. 대상 섹터를 지우고 그대로 프로그램한다.
// MAP_ROM 쪽(wd_iflash_program_one_sector_merged, 섹터당 wd_toolkit_crc16 예약)과는 완전히 분리된 별도 경로.
int32_t wd_iflash_write_raw(uint32_t addr, const void *data, uint32_t num);

#endif // #if 1 // 토글

#endif /* WD_COMPONENT_WD_IFLASH_H */
