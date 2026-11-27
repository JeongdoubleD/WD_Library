/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_env.c
 * @brief   Persistent environment (load / default / save / print)
 ******************************************************************************/
#include "wd.h"

_Static_assert(sizeof(wd_env_t) == WD_ENVIRONMENT_SIZE, "ENVIRONMENT must be 1024 bytes");

#ifndef RTE_CONFIG_PLATFORM_NAME
#define RTE_CONFIG_PLATFORM_NAME "WD-Platform"
#endif
#ifndef RTE_CONFIG_FIRMWARE_VERSION
#define RTE_CONFIG_FIRMWARE_VERSION "WD Application"
#endif

static const wd_env_t DEFAULT_ENVIRONMENT = {
    .Platform = RTE_CONFIG_PLATFORM_NAME,
    .Version = RTE_CONFIG_FIRMWARE_VERSION,
    .Console_Level = WD_CONF_CONSOLE_LEVEL_DEFAULT,
    .Net_Debug =
        {
            .Packet_Analyzer_TCP_App = 1,
            .Packet_Analyzer_TCP_Debug = 1,
        },
    .firmware_update_flag = false, // firmware update is off at default
    .digipot_rdac = 1000,          // used only when DigiPOT is used
};

wd_env_t wd_env_data;
wd_env_t *const wd_env = &wd_env_data;

/* ------------------------------------------------------------------------ (application hooks) */
__WEAK void wd_env_user_default(wd_env_t *env) {
    UNUSED(env);
}

__WEAK void wd_env_print_user(wd_env_t *env) {
    UNUSED(env);
}

/* ------------------------------------------------------------------------ */
void wd_env_default(void) {
    WD_LOG_INFO("Environment Information Set Default");

    memcpy(wd_env, &DEFAULT_ENVIRONMENT, sizeof(wd_env_t));
    wd_env_user_default(wd_env);

    wd_env->Digest = wd_toolkit_crc16(wd_env, sizeof(wd_env_t) - WD_FIELD_SIZEOF(wd_env_t, Digest));
}

#if (RTE_SERVICE_MMU)
static void Environment_Object(wd_mmu_multi_object_t *object) {
    memset(object, 0, sizeof(*object));
    object->Mask = 0;
    object->Object_Size = 1;
    object->Num = sizeof(wd_env_t);
    object->Data = &wd_env_data;
#if (RTE_COMPONENT_NVM_NOR_FLASH || RTE_COMPONENT_NVM_NAND_FLASH)
    object->Object[0] = &MAP->NVM->Environment;
#else
    object->Object[0] = &MAP->ROM->Environment; // internal flash fallback (no NOR/NAND)
#endif
}
#endif

/* apply environment values to the modules that keep their own settings */
static void environment_apply(void) {
#if (RTE_COMPONENT_INTERFACE_ETHERNET)
    wd_eth_set_packet_dump(wd_env->Net_Debug.Packet_Analyzer_TCP_App != 0U,
                           wd_env->Net_Debug.Packet_Analyzer_TCP_Debug != 0U);
#endif
}

void wd_env_init(uint32_t arg) {
#if (RTE_SERVICE_MMU)
    wd_mmu_multi_object_t object;

    Environment_Object(&object);

    if (wd_mmu_read_multi(WD_MMU_COMMAND_READ, &object)) {
        WD_LOG_ERROR();
#ifndef NDEBUG
        WD_BREAK();
#endif
    }

    if (wd_toolkit_crc16(&wd_env_data, sizeof(wd_env_data)) != WD_CRC16_RESULT) {
        WD_LOG_ERROR("Environment Resources Digest Error");
        object.Mask = WD_MMU_OBJECT_ALL;
    }

    if (arg) {
        object.Mask = WD_MMU_OBJECT_ALL;
    }

    if (object.Mask) {
        if (object.Mask == WD_MMU_OBJECT_ALL) {
            wd_env_default();
        }
        wd_mmu_write_multi(WD_MMU_COMMAND_WRITE_CRC, &object);
    }
#else
    UNUSED(arg);
    wd_env_default();
#endif

    environment_apply();
}

void wd_env_save(void) {
#if (RTE_SERVICE_MMU)
    wd_mmu_multi_object_t object;

    Environment_Object(&object);

    wd_env->Digest = wd_toolkit_crc16(wd_env, sizeof(wd_env_t) - WD_FIELD_SIZEOF(wd_env_t, Digest));
    object.Mask = WD_MMU_OBJECT_ALL;

    wd_mmu_write_multi(WD_MMU_COMMAND_WRITE_CRC, &object);
#endif

    environment_apply();
}

void wd_env_print(wd_env_t *env, char *title) {
    if (title != NULL) {
        WD_PRINT(WD_MSG_NOTI_CRLF, "%s ", title);
    }
    WD_PRINT(WD_MSG_INFO_CRLF, "Platform             : %s", env->Platform);
    WD_PRINT(WD_MSG_INFO_CRLF, "Version              : %s", env->Version);
    WD_PRINT(WD_MSG_INFO_CRLF, "Console Level        : %d",
             (env->Console_Level == WD_CONSOLE_LEVEL_MOTORMOUTH) ? 8 : env->Console_Level);
    WD_PRINT(WD_MSG_INFO_CRLF, "Packet Dump TCP App  : %d", env->Net_Debug.Packet_Analyzer_TCP_App);
    WD_PRINT(WD_MSG_INFO_CRLF, "Packet Dump TCP Debug: %d",
             env->Net_Debug.Packet_Analyzer_TCP_Debug);
    WD_PRINT(WD_MSG_INFO_CRLF, "firmware_update_flag : %d", env->firmware_update_flag);
    WD_PRINT(WD_MSG_INFO_CRLF, "digipot_rdac         : %u", env->digipot_rdac);
    wd_env_print_user(env);
}
