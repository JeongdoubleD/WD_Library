/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_eth_telnet.h
 * @brief   Telnet console over the W6100 Ethernet interface
 *
 * Layer      : Component (Ethernet)
 * Depends on : wd_core.h, _wd_eth.h
 * Config     : RTE_PLATFORM_PRINT_TELNET, RTE_COMPONENT_INTERFACE_ETHERNET_TELNET,
 *              RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE
 *
 * wd_eth_telnet_initialize() registers the Telnet console as an output sink and an input source of
 * the core console (wd_print_register_output / wd_print_register_input).
 * Data is sent only while wd_eth_telnet_state has WD_TELNET_STATE_ENABLE and the console socket is
 * established.
 ******************************************************************************/
#ifndef WD_COMPONENT_WD_ETH_TELNET_H
#define WD_COMPONENT_WD_ETH_TELNET_H

#include "wd_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (RTE_PLATFORM_PRINT_TELNET)

typedef enum {
    WD_TELNET_STATE_ENABLE = (1UL << 3),
} wd_eth_telnet_state_t;

extern int32_t wd_eth_telnet_state;
extern wd_cbuf_t wd_eth_telnet_tx_c_buf;
extern wd_cbuf_t wd_eth_telnet_rx_c_buf;
extern uint32_t wd_eth_telnet_tx_time_stamp;
extern uint32_t wd_eth_telnet_rx_time_stamp;

/**
 * @brief  Initialize Telnet buffers and register the Telnet console to the core console.
 * @return WD_CODE_OK
 * @note   Thread context, call once after the core time base is running.
 */
int32_t wd_eth_telnet_initialize(void);

void wd_eth_telnet_write_telnet(const void *data, uint32_t num, uint32_t arg);
void wd_eth_telnet_flush_telnet(uint32_t arg);
int32_t wd_eth_telnet_tstc_telnet(void);
int32_t wd_eth_telnet_read_telnet(void *data, uint32_t num, uint32_t arg);

#endif /* RTE_PLATFORM_PRINT_TELNET */

#ifdef __cplusplus
}
#endif

#endif /* WD_COMPONENT_WD_ETH_TELNET_H */
