/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_template.h
 * @brief   <module> public interface (one cohesive responsibility, GEN-S-002)
 *
 * Layer      : Platform | Drivers | Component | Service | System
 * Depends on : wd_core.h, <direct dependencies only>
 * HAL handle : WD_<MODULE>_HANDLE (default in this header, override in wd_board.h)
 * Config     : RTE_xxx, WD_CONF_xxx
 ******************************************************************************/
#ifndef WD_LAYER_WD_TEMPLATE_H
#define WD_LAYER_WD_TEMPLATE_H

#include "wd_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------------- configuration */
#ifndef WD_TEMPLATE_TIMEOUT_MS
#define WD_TEMPLATE_TIMEOUT_MS (100U) /* default, override in wd_config.h */
#endif

/* ----------------------------------------------------------------------------- types */
typedef struct wd_template_config_s {
    uint32_t sampling_rate_hz; /* unit suffix (GEN-S-006) */
    uint16_t timeout_ms;
} wd_template_config_t;

/* ----------------------------------------------------------------------------- functions */
/**
 * @brief      Initialize the module.
 * @param[in]  config  non-NULL, values within documented range
 * @return     WD_CODE_OK, WD_CODE_ERROR_PARAMETER (config == NULL), WD_CODE_ERROR_TIMEOUT
 * @note       Side effects : configures peripheral. Not reentrant. Do NOT call from ISR.
 */
wd_code_t wd_template_init(const wd_template_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* WD_LAYER_WD_TEMPLATE_H */
