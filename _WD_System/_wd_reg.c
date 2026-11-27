/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_reg.c
 * @brief   WD system register model / global shared buffers / address validation
 ******************************************************************************/
#include "wd.h"

/* ------------------------------------------------------------------------(Global variable) */
__USED uint32_t wd_reg_region[32] WD_LOCATION(0x24001000); /* WD_REGISTER_REGION_ADDRESS */
uint32_t wd_reg_alias[32][32] WD_LOCATION(0x24002000);     /* WD_REGISTER_ALIAS_ADDRESS  */

/* register size guard : each register block must fit in one alias row (32 x uint32_t) */
_Static_assert(sizeof(wd_reg_system_t) <= sizeof(wd_reg_alias[0]),
               "WD_REGISTER_SYSTEM exceeds 128 bytes");
_Static_assert(sizeof(wd_reg_status_t) <= sizeof(wd_reg_alias[0]),
               "WD_REGISTER_STATUS exceeds 128 bytes");
_Static_assert(sizeof(wd_reg_timer_t) <= sizeof(wd_reg_alias[0]),
               "WD_REGISTER_TIMER exceeds 128 bytes");

/***************************************************************************************************************************************************************
 **********  Register
 **************************************************************************************************************************************************************/
wd_reg_system_t *const wd_reg_system = (wd_reg_system_t *)&wd_reg_alias[0];
wd_reg_status_t *const wd_reg_status = (wd_reg_status_t *)&wd_reg_alias[2];
wd_reg_timer_t *const wd_reg_timer = (wd_reg_timer_t *)&wd_reg_alias[4];
wd_reg_timer_t *const wd_reg_r_timer_thread_source = (wd_reg_timer_t *)&wd_reg_alias[5]; //  5 ~ 14
wd_reg_timer_t *const wd_reg_r_timer_thread =
    (wd_reg_timer_t *)&wd_reg_alias[5 + RTE_RTOS_THREAD_NUMBER_MAX]; // 15 ~ 24
wd_reg_debug_t *const wd_reg_debug = (wd_reg_debug_t *)&wd_reg_alias[31];

/***************************************************************************************************************************************************************
 **********  Global
 **************************************************************************************************************************************************************/
wd_reg_global_t wd_global __ALIGNED(sizeof(uint32_t));


/***************************************************************************************************************************************************************
 **********  Core hooks -> system register model (overrides the weak defaults of _wd_core.c)
 **************************************************************************************************************************************************************/
void wd_os_tick_50ms_hook(void) {
    wd_reg_timer->Tick = true;
}

void wd_time_rtc_init_hook(void) {
    wd_reg_system->EVT_RTC_Initialize = true;
}

void wd_time_rtc_second_hook(void) {
    wd_reg_timer->RTC_Tick = true;
}

void wd_core_status_hook(wd_status_source_t source, wd_status_t status) {
    switch (source) {
    case WD_STATUS_SOURCE_POWER:
        wd_reg_status->Status_Power = (uint32_t)status;
        break;
    case WD_STATUS_SOURCE_ADC:
        wd_reg_status->Status_ADC = (uint32_t)status;
        break;
    case WD_STATUS_SOURCE_SDRAM:
        wd_reg_status->Status_SDRAM = (uint32_t)status;
        break;
    case WD_STATUS_SOURCE_NVM_FLASH:
        wd_reg_status->Status_NVM_NOR_Flash = (uint32_t)status;
        break;
    case WD_STATUS_SOURCE_TEMPERATURE_SENSOR:
        wd_reg_status->Status_Temperature_Sensor = (uint32_t)status;
        break;
    default:
        break;
    }
}

/* shared scratch buffer = wd_global.Shared union (shared with the MMU multi read buffers) */
uint8_t *wd_core_scratch_buffer(void) {
    return wd_global.Shared.Buf;
}

uint32_t wd_core_scratch_size(void) {
    return (uint32_t)sizeof(wd_global.Shared.Buf);
}

/* the memory mapped NVM base of the NOR / NAND drivers must match the project memory map */
#if (RTE_COMPONENT_NVM_NOR_FLASH)
_Static_assert(RTE_BLOCK_DEVICE_NOR_FLASH_ADDR == (uint32_t)MEMORY_DEVICE_NVM_FLASH_BASE,
               "NOR base != MEMORY_DEVICE_NVM_FLASH_BASE");
#endif
#if (RTE_COMPONENT_NVM_NAND_FLASH)
_Static_assert(RTE_BLOCK_DEVICE_NAND_FLASH_ADDR == (uint32_t)MEMORY_DEVICE_NVM_FLASH_BASE,
               "NAND base != MEMORY_DEVICE_NVM_FLASH_BASE");
#endif
