/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_env.h
 * @brief   Persistent environment (1024 bytes, wd_toolkit_crc16 digest) stored through MMU
 *          to MAP->NVM->Environment (or MAP->ROM->Environment fallback).
 *
 *  Application specific persistent settings : use wd_env->User[] area
 *  (cast to an application structure, max WD_ENVIRONMENT_USER_SIZE bytes).
 ******************************************************************************/
/* umbrella header first (before the guard) : any include order expands in the umbrella order */
#include "wd.h"

#ifndef WD_SYSTEM_WD_ENV_H
#define WD_SYSTEM_WD_ENV_H

#if defined(__cplusplus)
extern "C" {
#endif

#define WD_ENVIRONMENT_SIZE      1024
#define WD_ENVIRONMENT_USER_SIZE 512

typedef __PACKED_STRUCT {
    char Platform[64];
    char Version[64];

    int Console_Level;

    struct wd_net_debug_config_s {
        uint32_t Packet_Analyzer_TCP_App : 1;   /* dump packets of application TCP port */
        uint32_t Packet_Analyzer_TCP_Debug : 1; /* dump packets of debug TCP port       */
        uint32_t : 30;
    } Net_Debug;

    int firmware_update_flag;
    uint32_t digipot_rdac;

    uint8_t User[WD_ENVIRONMENT_USER_SIZE]; /* application area */

    uint8_t Reserved[WD_ENVIRONMENT_SIZE - 2 - 64 - 64 - 4 - 4 - 4 - 4 - WD_ENVIRONMENT_USER_SIZE];

    uint16_t Digest;
}
wd_env_t; // 1024 Bytes

extern wd_env_t wd_env_data;
extern wd_env_t *const wd_env;

void wd_env_default(void);
void wd_env_init(uint32_t arg);
void wd_env_save(void);
void wd_env_print(wd_env_t *env, char *title);

/* Optional application hooks (weak, override in application) */
void wd_env_user_default(wd_env_t *env);
void wd_env_print_user(wd_env_t *env);

#if defined(__cplusplus)
}
#endif

#endif /* WD_SYSTEM_WD_ENV_H */
