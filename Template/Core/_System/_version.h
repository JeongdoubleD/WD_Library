/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _version.h   (TEMPLATE - project owned)
 * @brief   Project version information / boot logo
 *
 *  Firmware version string used by the WD Library : RTE_CONFIG_FIRMWARE_VERSION (wd_config.h)
 *  WD Library version                               : WD_LIB_VERSION_STRING (wd_version.h)
 ******************************************************************************/
#ifndef WD_TEMPLATE_VERSION_H
#define WD_TEMPLATE_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 -------------------------------------------------
 H/W Version history
   yyyy.mm.dd  [x.x.xxx]  description
 -------------------------------------------------
 S/W Version history
   yyyy.mm.dd  [x.x.x]    description
 -------------------------------------------------
*/
#define VER_COMPILED_TIME __DATE__ " - " __TIME__
#define VER_STRING        "1.0.0"
#define VER_HW_4BYTE      100000L // 1.00.000
#define VER_SW_4BYTE      100000L // 1.00.000
#define VER_TYPE          RTE_CONFIG_PLATFORM_NAME
#define VER_VERSION       "V1.0.0"

#define MSG_LOGO                                                                               \
    "\r\n"                                                                                     \
    "  __        ______  \r\n"                                                                 \
    "  \\ \\      / /  _ \\ \r\n"                                                              \
    "   \\ \\ /\\ / /| | | |\r\n"                                                              \
    "    \\ V  V / | |_| |\r\n"                                                                \
    "     \\_/\\_/  |____/   " RTE_CONFIG_PLATFORM_NAME " / WD Library " WD_LIB_VERSION_STRING \
    "\r\n"                                                                                     \
    "\r\n"

#ifdef __cplusplus
}
#endif

#endif /* WD_TEMPLATE_VERSION_H */
