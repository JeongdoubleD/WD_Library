/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_w6100_port.c
 * @brief   WD Library port layer for the WIZnet io6Library (W6100)
 *          - bounded waits used by Ethernet/socket.c (WD-PATCH-1)
 *          - symbols the io6Library expects the application to provide
 ******************************************************************************/
#include <stdio.h>
#include "_wd_w6100_port.h"
#include "_wd_time.h"

/* Internet/DNS/dns.c : "extern uint8_t IP_TYPE;" (record type of the query, set by the caller of
 * DNS_run()). The io6Library example application defines it, the library itself does not. */
uint8_t IP_TYPE;

uint32_t wd_w6100_port_wait_start(void)
{
    return (uint32_t)wd_time_get_us();
}

bool wd_w6100_port_wait_expired(uint32_t start_us, const char *op_name)
{
    bool expired = false;

#if (WD_CONF_W6100_WAIT_TIMEOUT_US > 0u)
    clock_t start = (clock_t)start_us;

    if ((uint32_t)wd_time_calc_us(&start) > WD_CONF_W6100_WAIT_TIMEOUT_US) {
        (void)printf("\x1b[1;37m\x1b[1;41m"
                     "W6100 %s Timeout 1!!\r\n"
                     "\x1b[0m",
                     op_name);
        expired = true;
    }
#else
    (void)start_us;
    (void)op_name;
#endif

    return expired;
}
