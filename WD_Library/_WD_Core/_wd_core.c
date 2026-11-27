/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_core.c
 * @brief   WD core state and default (weak) core hooks
 ******************************************************************************/
#include "wd_core.h"

/* ----------------------------------------------------------------------------- core state */
wd_system_mode_t wd_core_system_mode = WD_SYSTEM_MODE_BOOT;

/* ----------------------------------------------------------------------------- default hooks */
__WEAK void wd_os_tick_50ms_hook(void) {
}

__WEAK void wd_time_rtc_init_hook(void) {
}

__WEAK void wd_time_rtc_second_hook(void) {
}

/* ----------------------------------------------------------------------------- address validation */
typedef struct {
    uint32_t start;
    uint32_t end;
} wd_core_memory_range_t;

/* valid memory ranges of the board (wd_board.h : WD_BOARD_MEMORY_RANGES, {start, end} inclusive) */
static const wd_core_memory_range_t s_memory_ranges[] = {
#ifdef WD_BOARD_MEMORY_RANGES
    WD_BOARD_MEMORY_RANGES
#else
    {0x00000000U, 0xFFFFFFFFU},
#endif
};

bool wd_core_is_valid_address(uint32_t address) {
    bool valid = false;

    for (size_t i = 0U; i < (sizeof(s_memory_ranges) / sizeof(s_memory_ranges[0])); i++) {
        if ((address >= s_memory_ranges[i].start) && (address <= s_memory_ranges[i].end)) {
            valid = true;
            break;
        }
    }
    return valid;
}

/* ----------------------------------------------------------------------------- status report */
__WEAK void wd_core_status_hook(wd_status_source_t source, wd_status_t status) {
    UNUSED(source);
    UNUSED(status);
}

/* ----------------------------------------------------------------------------- shared scratch buffer */
static uint8_t s_core_scratch[WD_CONF_CORE_SCRATCH_SIZE] __ALIGNED(sizeof(uint32_t));

__WEAK uint8_t *wd_core_scratch_buffer(void) {
    return s_core_scratch;
}

__WEAK uint32_t wd_core_scratch_size(void) {
    return (uint32_t)sizeof(s_core_scratch);
}
