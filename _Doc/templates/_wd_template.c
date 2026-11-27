/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_template.c
 * @brief   <module> implementation
 ******************************************************************************/
#include "_wd_template.h"

/* ----------------------------------------------------------------------------- local constants */
enum { MAX_RETRY_COUNT = 3 }; /* no magic numbers (GEN-S-007) */

/* ----------------------------------------------------------------------------- local variables */
static wd_template_config_t s_config;
static bool s_initialized = false;

/* ----------------------------------------------------------------------------- local functions */
static wd_code_t apply_config(const wd_template_config_t *config) {
    wd_code_t st = WD_CODE_OK;

    if (config->timeout_ms == 0U) {
        st = WD_CODE_ERROR_PARAMETER;
    } else {
        s_config = *config;
    }
    return st;
}

/* ----------------------------------------------------------------------------- public functions */
wd_code_t wd_template_init(const wd_template_config_t *config) {
    wd_code_t st = WD_CODE_ERROR_PARAMETER;

    if (config != NULL) { /* explicit NULL comparison (GEN-M-009, M-016) */
        uint32_t retry = 0U;

        do {
            st = apply_config(config);
            retry++;
        } while ((st != WD_CODE_OK) && (retry < (uint32_t)MAX_RETRY_COUNT));

        s_initialized = (st == WD_CODE_OK);
    }
    return st; /* single exit (GEN-S-009) */
}
