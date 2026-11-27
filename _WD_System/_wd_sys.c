/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_sys.c
 * @brief   System globals : reset reason, power state, system mode, debug values
 ******************************************************************************/

#include "wd.h"

/* ------------------------------------------------------------------------(Global variable) */
const uint8_t wd_sys_firmware_build_time[] = __TIMESTAMP__;
int32_t wd_sys_debug_value = 0;
int32_t wd_sys_debug_value2 = 0;
int32_t wd_sys_debug_value3 = 0;
int32_t wd_sys_debug_value4 = 0;


/* ------------------------------------------------------------------------(Local global variable) */

/* ------------------------------------------------------------------------(Local Function Prototype) */

/* ------------------------------------------------------------------------(IRQ ....) */

/* ------------------------------------------------------------------------(static, extern) */

/* ------------------------------------------------------------------------  */
#ifndef NDEBUG
wd_sys_power_reset_reason_t wd_sys_power_get_reset_reason(void) {
#ifdef RCC_FLAG_LPWRRST
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
        return WD_POWER_RESET_REASON_WAKE_LOW_POWER;
    }
#endif

#ifdef RCC_FLAG_LPWR1RST
    if ((__HAL_RCC_GET_FLAG(RCC_FLAG_LPWR1RST)) || (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWR2RST))) {
        return WD_POWER_RESET_REASON_WAKE_LOW_POWER;
    }
#endif

#ifdef RCC_FLAG_WWDGRST
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
        return WD_POWER_RESET_REASON_WATCHDOG;
    }
#endif

#ifdef RCC_FLAG_WWDG1RST
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST)) {
        return WD_POWER_RESET_REASON_WATCHDOG;
    }
#endif

#ifdef RCC_FLAG_IWDGRST
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
        return WD_POWER_RESET_REASON_WATCHDOG;
    }
#endif

#ifdef RCC_FLAG_IWDG1RST
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDG1RST)) {
        return WD_POWER_RESET_REASON_WATCHDOG;
    }
#endif

#ifdef RCC_FLAG_SFTRST
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
        return WD_POWER_RESET_REASON_SOFTWARE;
    }
#endif

#ifdef RCC_FLAG_PORRST
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)) {
        return WD_POWER_RESET_REASON_POWER_ON;
    }
#endif

#ifdef RCC_FLAG_BORRST
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST)) {
        return WD_POWER_RESET_REASON_BROWN_OUT;
    }
#endif

#ifdef RCC_FLAG_PINRST
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
        return WD_POWER_RESET_REASON_PIN_RESET;
    }
#endif

    return WD_POWER_RESET_REASON_UNKNOWN;
}

char *wd_sys_power_reset_reason_msg_box(wd_sys_power_reset_reason_t reason) {
    char *msg = "Unknown or unreadable reset reason.";

    switch (reason) {
    case WD_POWER_RESET_REASON_POWER_ON: // Set when power is initially applied to the board. The power-on-reset circuit causes a POWER_ON reset when this occurs.
        msg = "Set when power is initially applied to the board. The power-on-reset circuit causes "
              "a POWER_ON reset when this occurs.";
        break;

    case WD_POWER_RESET_REASON_PIN_RESET: // Set when a reset is triggered by the hardware pin on the board.
        msg = "Set when a reset is triggered by the hardware pin on the board.";
        break;

    case WD_POWER_RESET_REASON_BROWN_OUT: // Triggered when the voltage drops below the low voltage detect (LVD) threshold; the system is held in a reset until the voltage rises above the threshold.
        msg = "Triggered when the voltage drops below the low voltage detect (LVD) threshold; the "
              "system is held in a reset until the voltage rises above the threshold.";
        break;

    case WD_POWER_RESET_REASON_SOFTWARE: // Set during software reset, typically triggered by writing the SYSRESETREQ bit in the Application Interrupt and Reset Control register.
        msg = "Set during software reset, typically triggered by writing the SYSRESETREQ bit in "
              "the Application Interrupt and Reset Control register.";
        break;

    case WD_POWER_RESET_REASON_WATCHDOG: // Set when a running watchdog timer fails to be refreshed.
        msg = "Set when a running watchdog timer fails to be refreshed.";
        break;

    case WD_POWER_RESET_REASON_LOCKUP: // Set when the core is locked because of an unrecoverable exception.
        msg = "Set when the core is locked because of an unrecoverable exception.";
        break;

    case WD_POWER_RESET_REASON_WAKE_LOW_POWER: // Set when waking from deep sleep mode.
        msg = "Set when waking from deep sleep mode.";
        break;

    case WD_POWER_RESET_REASON_ACCESS_ERROR: // Umbrella value that encompasses any access related reset.
        msg = "Umbrella value that encompasses any access related reset.";
        break;

    case WD_POWER_RESET_REASON_BOOT_ERROR: // Umbrella value that encompasses any boot related reset.
        msg = "Umbrella value that encompasses any boot related reset.";
        break;

    case WD_POWER_RESET_REASON_MULTIPLE: // Set if multiple reset reasons are set within the board. Occurs when the reset reason registers aren't cleared between resets.
        msg = "Set if multiple reset reasons are set within the board. Occurs when the reset "
              "reason registers aren't cleared between resets.";
        break;

    case WD_POWER_RESET_REASON_PLATFORM: // Platform specific reset reason not captured in this enum.
        msg = "Platform specific reset reason not captured in this enum.";
        break;

    case WD_POWER_RESET_REASON_UNKNOWN: // Unknown or unreadable reset reason.
        msg = "Unknown or unreadable reset reason.";
        break;
    default:
        break;
    }

    return (msg);
}
#endif

void wd_sys_power_boot_up(void) {
#if (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_BOOTLOADER)
#elif (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
#endif
}

void wd_sys_power_sleep(void) {
#if (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_BOOTLOADER)
#elif (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
#endif

    HAL_PWR_EnterSTANDBYMode(); // Enter Stnadby Mode
}

void wd_sys_power_wake_up(void) {
#if (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_BOOTLOADER)
#elif (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
#endif
}

wd_power_state_t wd_sys_power_status(void) {
    /*
   * false : Normal
   * true : power fail
   */
    wd_power_state_t p_fail_condition = WD_POWER_FULL;
#if (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_BOOTLOADER)
#elif (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
#endif
    return (p_fail_condition);
}
