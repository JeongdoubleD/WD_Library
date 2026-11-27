/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_nand.h
 *
 *  Winbond W25N04KVZEIR 4Gb SPI NAND Flash driver header
 *  Created on: 2026-06-17
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_NAND_H
#define WD_COMPONENT_WD_NAND_H

#include "wd_core.h"
#include "_wd_blockdev.h"

/* board HAL mapping : default CubeMX names, override in wd_board.h */
#ifndef RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE
#define RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE (&hqspi)
#endif

#if 1 // 토글

/***************************************************************************************************************************************************************
 **********  NVM NAND Flash — W25N04KVZEIR
 **************************************************************************************************************************************************************/

/* Kept same name/value as NOR header for _Memory_User.h MAP_NVM_FLASH compatibility */
#define RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE 4096

/* NAND physical geometry */
#define W25N_PAGE_SIZE       2048u
#define W25N_PAGES_PER_BLOCK 64u
#define W25N_BLOCK_SIZE      (W25N_PAGE_SIZE * W25N_PAGES_PER_BLOCK) /* 128 KB */
#define W25N_TOTAL_BLOCKS    2048u

/* JEDEC ID: 0x9F → 0xEF (Winbond), 0xAA, 0x23 */
#define W25N_JEDEC_MFR  0xEF
#define W25N_JEDEC_TYPE 0xAA
#define W25N_JEDEC_CAP  0x23

/*
 * Logical address mapping (for MAP_NVM_FLASH 4 KB sector compatibility):
 *   logical_block  = addr / RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE
 *   nand_block     = logical_block           (1 NAND block per 4 KB logical sector)
 *   page_in_block  = (addr % ERASE_BLOCK_SIZE) / W25N_PAGE_SIZE  (0 or 1)
 *   column         = addr % W25N_PAGE_SIZE
 *   nand_page_addr = nand_block * W25N_PAGES_PER_BLOCK + page_in_block
 */
#define W25N_ADDR_TO_BLOCK(addr) ((addr) / RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE)
#define W25N_ADDR_TO_PAGE(addr)                        \
    (W25N_ADDR_TO_BLOCK(addr) * W25N_PAGES_PER_BLOCK + \
     ((addr) % RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE) / W25N_PAGE_SIZE)
#define W25N_ADDR_TO_COL(addr) ((addr) % W25N_PAGE_SIZE)

/* Block device capability constants */
#ifndef RTE_BLOCK_DEVICE_NAND_FLASH_ADDR
#define RTE_BLOCK_DEVICE_NAND_FLASH_ADDR \
    ((uint32_t)0xC0000000U) /* memory mapped base (= MEMORY_DEVICE_NVM_FLASH_BASE) */
#endif
#define RTE_BLOCK_DEVICE_NAND_FLASH_SIZE (W25N_TOTAL_BLOCKS * RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE)
#define RTE_BLOCK_DEVICE_NAND_FLASH_PROGRAM_SIZE      W25N_PAGE_SIZE
#define RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_SECTOR_SIZE RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE
#define RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_BLOCK_SIZE  W25N_BLOCK_SIZE
#define RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_BLOCK_ADDR  0u

#define RTE_BLOCK_DEVICE_NAND_FLASH_PROGRAM_TIMEOUT      ((uint32_t)700u)   /* us */
#define RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_SECTOR_TIMEOUT ((uint32_t)10000u) /* us */
#define RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_BLOCK_TIMEOUT  ((uint32_t)10000u) /* us */
#define RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_CHIP_TIMEOUT   ((uint32_t)0u)

extern wd_blockdev_instance_t wd_drv_block_device_nand_flash;
extern wd_blockdev_instance_info_t *const wd_nand_res_block_device_nand_flash;

#ifndef NDEBUG
typedef struct wd_debug_block_device_nand_flash_s {
    wd_blockdev_instance_resources_t *Resource;
    wd_blockdev_instance_info_t *const *Info;
    wd_blockdev_instance_t *Driver;
} const wd_nand_debug_block_device_nand_flash_t;

extern wd_nand_debug_block_device_nand_flash_t wd_nand_dbg_block_device_nand_flash;
#endif

#endif // #if 1 // 토글

#endif /* WD_COMPONENT_WD_NAND_H */
