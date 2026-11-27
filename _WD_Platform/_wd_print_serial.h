/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 *//**
 *  \version         V1.0.0 [MDK 5.37]
 *  \author
 ** @{
 ******************************************************************************/

#ifndef WD_PLATFORM_WD_PRINT_SERIAL_H
#define WD_PLATFORM_WD_PRINT_SERIAL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "wd.h"

void wd_print_serial_write_serial(const void *data, uint32_t num, uint32_t arg);


extern int stdout_puts(const char *str);
extern char wd_itoa(uint8_t number);
extern void stdout_put_hex(char *msg, const uint8_t *hex, uint32_t len);


#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/
#endif /*--------------- WD_PLATFORM_WD_PRINT_SERIAL_H ---------------*/
/** @} end of file */
