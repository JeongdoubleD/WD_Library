/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_mmu.h
 *
 *  Created on: Jan 7, 2025
 *      Author: doubled
 */

/* umbrella header first (before the guard) : any include order expands in the umbrella order */
#include "wd.h"

#ifndef WD_SERVICE_WD_MMU_H
#define WD_SERVICE_WD_MMU_H

#if defined(__cplusplus)
extern "C" {
#endif


#if 1 // 토글

#if (RTE_SERVICE_MMU)

#define WD_MMU_MULTI_READ_BUFFER_SIZE 4100U // 4096 + 4(CRC)

typedef enum {
/*--------------- MMU option : Objects ---------------*/
#define WD_MMU_OBJECT_POS  0
#define WD_MMU_OBJECT_MSK  (0xFFUL << WD_MMU_OBJECT_POS)
#define WD_MMU_OBJECT_SIZE 3U
    WD_MMU_OBJECT_AUTO = (0UL << WD_MMU_OBJECT_POS),
    WD_MMU_OBJECT_RAM = (1UL << WD_MMU_OBJECT_POS),
    WD_MMU_OBJECT_ROM = (2UL << WD_MMU_OBJECT_POS),
    WD_MMU_OBJECT_NVM_FLASH = (3UL << WD_MMU_OBJECT_POS),
    WD_MMU_OBJECT_NVM_SERIAL_EEPROM = (4UL << WD_MMU_OBJECT_POS),
    WD_MMU_OBJECT_NONE = (100UL << WD_MMU_OBJECT_POS),
    WD_MMU_OBJECT_ALL = (0xFFUL << WD_MMU_OBJECT_POS),

/*--------------- MMU option : Command ---------------*/
#define WD_MMU_COMMAND_POS 8
#define WD_MMU_COMMAND_MSK (0xFFUL << WD_MMU_COMMAND_POS)
    WD_MMU_COMMAND_READ = ((1UL << 0) << WD_MMU_COMMAND_POS),
    WD_MMU_COMMAND_WRITE = ((1UL << 1) << WD_MMU_COMMAND_POS),
    WD_MMU_COMMAND_INIT = ((1UL << 2) << WD_MMU_COMMAND_POS),
    WD_MMU_COMMAND_INIT_0xFF = ((1UL << 3) << WD_MMU_COMMAND_POS),
    WD_MMU_COMMAND_INIT_0xFF_BULK = ((1UL << 4) << WD_MMU_COMMAND_POS),

/*--------------- MMU option : Mode ---------------*/
#define WD_MMU_MODE_POS 16
#define WD_MMU_MODE_MSK (0xFFUL << WD_MMU_MODE_POS)
#define WD_MMU_MODE(n)  (((n) & 0xFF) << WD_MMU_MODE_POS)
    WD_MMU_MODE_BACKUP = (1UL << WD_MMU_MODE_POS),

/*--------------- MMU option : Argument ---------------*/
#define WD_MMU_ARGUMENT_POS 24
#define WD_MMU_ARGUMENT_MSK (0xFFUL << WD_MMU_ARGUMENT_POS)
    WD_MMU_ARGUMENT_CRC = ((1UL << 0) << WD_MMU_ARGUMENT_POS),
    WD_MMU_ARGUMENT_EXTERNAL_PTR = ((1UL << 1) << WD_MMU_ARGUMENT_POS),

    /*--------------- Pre-defined ---------------*/
    WD_MMU_COMMAND_WRITE_CRC = (WD_MMU_COMMAND_WRITE | WD_MMU_ARGUMENT_CRC),
    WD_MMU_COMMAND_READ_CRC = (WD_MMU_COMMAND_READ | WD_MMU_ARGUMENT_CRC),
    WD_MMU_COMMAND_INIT_CRC = (WD_MMU_COMMAND_INIT | WD_MMU_ARGUMENT_CRC),
    WD_MMU_COMMAND_INIT_0xFF_CRC = (WD_MMU_COMMAND_INIT_0xFF | WD_MMU_ARGUMENT_CRC),
} wd_mmu_control_table_t;

typedef struct {
    uint8_t Mask; // 0:OK, 1:Error
    uint8_t Object_Size;
    uint16_t Num;
    void *Data;
    void *Object[3];
} wd_mmu_multi_object_t;

int32_t wd_mmu_get_object(uint32_t addr);
/**
 * @brief  Bind a block device to an MMU memory object (address range from wd_memory_map.h).
 * @param  object  WD_MMU_OBJECT_ROM, WD_MMU_OBJECT_NVM_FLASH or WD_MMU_OBJECT_NVM_SERIAL_EEPROM
 * @param  device  initialized block device instance, NULL to detach
 * @return WD_CODE_OK, WD_CODE_ERROR_PARAMETER (other object)
 * @note   Call once at start-up after the block device Initialize(). Not thread safe.
 */
int32_t wd_mmu_attach(uint32_t object, wd_blockdev_instance_t *device);
wd_blockdev_instance_t *wd_mmu_get_intereface(uint32_t addr);
int32_t wd_mmu_access(void *dst, void *src, uint32_t num, uint32_t arg);
int32_t wd_mmu_write(void *dst, void *src, uint32_t num);
bool wd_mmu_user_write_map(void *dst, void **src, uint32_t *num, void *job[],
                           uint32_t *arg); /* application hook (weak) */
int32_t wd_mmu_read_multi(uint32_t arg, wd_mmu_multi_object_t *object);
int32_t wd_mmu_write_multi(uint32_t arg, wd_mmu_multi_object_t *object);
void wd_mmu_validation(void);


#endif // (RTE_SERVICE_MMU)

#endif // #if 0		// 토글

#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/

#endif /* WD_SERVICE_WD_MMU_H */
