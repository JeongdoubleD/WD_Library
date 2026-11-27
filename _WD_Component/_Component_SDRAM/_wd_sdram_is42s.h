/* SPDX-License-Identifier: MIT AND BSD-3-Clause
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright (c) 2019 STMicroelectronics (IS42S32800G component, BSD-3-Clause)
 */
/*
 * _wd_sdram_is42s.h
 *
 *  Created on: Jan 23, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_SDRAM_IS42S_H
#define WD_COMPONENT_WD_SDRAM_IS42S_H

#include "_wd_sdram.h"

/* board HAL mapping : default CubeMX names, override in wd_board.h */
#ifndef RTE_COMPONENT_SDRAM_DRIVER_HANDLE
#define RTE_COMPONENT_SDRAM_DRIVER_HANDLE (&hsdram1)
#endif

#if 1 // 토글

#define WD_SDRAM_ADDRESS_BASE 0xD0000000
#define WD_SDRAM_ADDRESS_SIZE (32 * 1024 * 1024)

typedef struct wd_sdram_is42s32800g_capabilities_s {
    uint32_t Address;
    uint32_t Size;
} wd_sdram_is42s_capabilities_t;

typedef struct wd_sdram_is42s32800g_info_s {
    int32_t volatile Flags;
    FMC_SDRAM_CommandTypeDef Command;
} wd_sdram_is42s_info_t;

typedef const struct wd_sdram_is42s32800g_resources_s {
    wd_sdram_is42s_capabilities_t Capabilities;
    wd_sdram_is42s_info_t *Info;
    void *Handle;
    void *Reg;
} wd_sdram_is42s_resources_t;

extern wd_sdram_driver_sdram_t wd_drv_sdram;
extern wd_sdram_is42s_info_t *const wd_sdram_is42s_res_sdram;

#ifndef NDEBUG
typedef struct wd_debug_sdram_is42s32800g_s {
    wd_sdram_is42s_resources_t *Resource;
    wd_sdram_is42s_info_t *Info;
    wd_sdram_driver_sdram_t *Driver;
} const wd_sdram_is42s_debug_sdram_is42s32800g_t;

extern wd_sdram_is42s_debug_sdram_is42s32800g_t wd_sdram_is42s_dbg_sdram;
#endif

extern int32_t wd_sdram_is42s_init_sdram(void); // test
/* SDRAM MDMA interrupt : call from the MDMA IRQ handler of the project */
void SDRAM_IRQHandler(void);

#endif //	#if 1 // 토글

#endif /* WD_COMPONENT_WD_SDRAM_IS42S_H */
