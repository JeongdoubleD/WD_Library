/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_w6100_port.h
 * @brief   WD Library port layer for the WIZnet io6Library (W6100)
 *
 *  Included from Application/Application.h (WD-PATCH-2), therefore visible in every
 *  io6Library source through wizchip_conf.h. Keep this header free of WD Library includes :
 *  it only declares the port functions implemented in _wd_w6100_port.c.
 *
 *  Bounded waits (WD-PATCH-1, Ethernet/socket.c) :
 *    the five busy-wait loops of send / recv / sendto / recvfrom wait forever in the original
 *    library when the chip stops responding. The port gives each loop a timeout and returns
 *    SOCKERR_TIMEOUT when it expires.
 *
 *  Configuration (wd_config.h or compiler define) :
 *    WD_CONF_W6100_WAIT_TIMEOUT_US   timeout of one wait loop in microseconds (default 5 s)
 *                                    0 = wait forever (original io6Library behaviour)
 *
 *  See ../README_WD.md for the full patch list.
 ******************************************************************************/
#ifndef WD_THIRD_PARTY_WD_W6100_PORT_H
#define WD_THIRD_PARTY_WD_W6100_PORT_H

#include <stdbool.h>
#include <stdint.h>

#ifndef WD_CONF_W6100_WAIT_TIMEOUT_US
#define WD_CONF_W6100_WAIT_TIMEOUT_US 5000000u
#endif

/**
 * @brief  Start time of a bounded wait.
 * @return current time in microseconds (free running counter)
 */
uint32_t wd_w6100_port_wait_start(void);

/**
 * @brief  Check a bounded wait.
 * @param  start_us  value returned by wd_w6100_port_wait_start()
 * @param  op_name   socket operation name used in the timeout message
 * @return true when the wait exceeded WD_CONF_W6100_WAIT_TIMEOUT_US (message is printed once)
 */
bool wd_w6100_port_wait_expired(uint32_t start_us, const char *op_name);

#define WIZCHIP_PORT_WAIT_START(var) uint32_t var = wd_w6100_port_wait_start()

#define WIZCHIP_PORT_WAIT_CHECK(var, op_name)                                                     \
    do {                                                                                          \
        if (wd_w6100_port_wait_expired((var), (op_name))) {                                       \
            return SOCKERR_TIMEOUT;                                                               \
        }                                                                                         \
    } while (0)

#endif /* WD_THIRD_PARTY_WD_W6100_PORT_H */
