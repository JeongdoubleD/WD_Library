/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_sys.h
 * @brief   System globals : reset reason, power state, system mode, debug values
 ******************************************************************************/

/* umbrella header first (before the guard) : any include order expands in the umbrella order */
#include "wd.h"

#ifndef WD_SYSTEM_WD_SYS_H
#define WD_SYSTEM_WD_SYS_H


/* ------------------------------------------------------------------------(Macro or Structure definition) */
#ifndef NDEBUG
/** Definitions of different reset reasons
 */
typedef enum {
    WD_POWER_RESET_REASON_POWER_ON, /**< Set when power is initially applied to the board. The power-on-reset circuit causes a POWER_ON reset when this occurs */
    WD_POWER_RESET_REASON_PIN_RESET, /**< Set when a reset is triggered by the hardware pin on the board */
    WD_POWER_RESET_REASON_BROWN_OUT, /**< Triggered when the voltage drops below the low voltage detect (LVD) threshold; the system is held in a reset until the voltage rises above the threshold */
    WD_POWER_RESET_REASON_SOFTWARE, /**< Set during software reset, typically triggered by writing the SYSRESETREQ bit in the Application Interrupt and Reset Control register */
    WD_POWER_RESET_REASON_WATCHDOG, /**< Set when a running watchdog timer fails to be refreshed */
    WD_POWER_RESET_REASON_LOCKUP, /**< Set when the core is locked because of an unrecoverable exception */
    WD_POWER_RESET_REASON_WAKE_LOW_POWER, /**< Set when waking from deep sleep mode */
    WD_POWER_RESET_REASON_ACCESS_ERROR, /**< Umbrella value that encompasses any access related reset */
    WD_POWER_RESET_REASON_BOOT_ERROR, /**< Umbrella value that encompasses any boot related reset */
    WD_POWER_RESET_REASON_MULTIPLE, /**< Set if multiple reset reasons are set within the board. Occurs when the reset reason registers aren't cleared between resets */
    WD_POWER_RESET_REASON_PLATFORM, /**< Platform specific reset reason not captured in this enum */
    WD_POWER_RESET_REASON_UNKNOWN   /**< Unknown or unreadable reset reason **/
} wd_sys_power_reset_reason_t;

wd_sys_power_reset_reason_t wd_sys_power_get_reset_reason(void);
char *wd_sys_power_reset_reason_msg_box(wd_sys_power_reset_reason_t reason);
#endif

void wd_sys_power_boot_up(void);
void wd_sys_power_sleep(void);
void wd_sys_power_wake_up(void);
wd_power_state_t wd_sys_power_status(void);


/* ------------------------------------------------------------------------(Global variable) */
extern const uint8_t wd_sys_firmware_build_time[];
extern int32_t wd_sys_debug_value;
extern int32_t wd_sys_debug_value2;
extern int32_t wd_sys_debug_value3;
extern int32_t wd_sys_debug_value4;

/* ------------------------------------------------------------------------(Global Function) */


/* ------------------------------------------------------------------------ */

#endif /* WD_SYSTEM_WD_SYS_H */
