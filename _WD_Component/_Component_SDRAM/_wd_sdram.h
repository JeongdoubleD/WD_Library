/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_sdram.h
 *
 *  Created on: Jan 23, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_SDRAM_H
#define WD_COMPONENT_WD_SDRAM_H

#include "wd_core.h"

#if 1 // 토글

typedef struct wd_driver_sdram_s {
    int32_t (*Initialize)(void);
    int32_t (*Uninitialize)(void);
    void (*Validation)(void);
} const wd_sdram_driver_sdram_t;


#endif //	#if 1 // 토글

#endif /* WD_COMPONENT_WD_SDRAM_H */
