/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_nor.h
 *
 *  Created on: Jan 15, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_NOR_H
#define WD_COMPONENT_WD_NOR_H

#include "wd_core.h"
#include "_wd_blockdev.h"

/* board HAL mapping : default CubeMX names, override in wd_board.h */
#ifndef RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE
#define RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE (&hqspi)
#endif

#if 1 // 토글


/***************************************************************************************************************************************************************
 **********  NVM NOR Flash
 **************************************************************************************************************************************************************/
//OLD
#define RTE_NVM_NOR_FLASH_DRIVER_NUM         0
#define RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE 256
#define RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE   4096
#define RTE_NVM_NOR_FLASH_SIZE               (256 * 1024 * 1024)
#define RTE_NVM_NOR_FLASH_TEST_ADDRESS \
    (RTE_NVM_NOR_FLASH_SIZE - (RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE * 2))

//NEW
#ifndef RTE_BLOCK_DEVICE_NOR_FLASH_ADDR
#define RTE_BLOCK_DEVICE_NOR_FLASH_ADDR \
    ((uint32_t)0xC0000000U) /* memory mapped base (= MEMORY_DEVICE_NVM_FLASH_BASE) */
#endif
#define RTE_BLOCK_DEVICE_NOR_FLASH_SIZE         ((uint32_t)(256 * 1024 * 1024)) // [ 256 Mbyte	]
#define RTE_BLOCK_DEVICE_NOR_FLASH_PROGRAM_SIZE 256                             // [ 256  byte	]
#define RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_SECTOR_SIZE 4096                       // [ 4   Kbyte	]
#define RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_BLOCK_SIZE  ((uint32_t)(64 * 1024))    // [ 64  Kbyte	]
#define RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_BLOCK_ADDR \
    ((uint32_t)(128 * 1024 * 1024)) // [ 4   Kbyte	]

#define RTE_BLOCK_DEVICE_NOR_FLASH_PROGRAM_TIMEOUT      ((uint32_t)100000)    /* us */
#define RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_SECTOR_TIMEOUT ((uint32_t)600000)    /* us */
#define RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_BLOCK_TIMEOUT  ((uint32_t)2500000)   /* us */
#define RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_CHIP_TIMEOUT   ((uint32_t)350000000) /* us */

extern wd_blockdev_instance_t wd_drv_block_device_nor_flash;
extern wd_blockdev_instance_info_t *const wd_nor_res_block_device_nor_flash;

#ifndef NDEBUG
typedef struct wd_debug_block_device_nor_flash_s {
    wd_blockdev_instance_resources_t *Resource;
    wd_blockdev_instance_info_t *Info;
    wd_blockdev_instance_t *Driver;
} const wd_nor_debug_block_device_nor_flash_t;

extern wd_nor_debug_block_device_nor_flash_t wd_nor_dbg_block_device_nor_flash;
#endif


#endif // #if 1 // 토글

#endif /* WD_COMPONENT_WD_NOR_H */
