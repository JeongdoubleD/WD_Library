/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_net_eth.h
 *
 *  Created on: Jan 7, 2025
 *      Author: doubled
 */

#ifndef WD_SERVICE_WD_NET_ETH_H
#define WD_SERVICE_WD_NET_ETH_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "wd.h"

#define WD_NET_ETHERNET_BUFFER_SIZE       2100u
#define WD_NET_ETHERNET_MESSAGE_POOL_SIZE 3UL

typedef struct {
    wd_network_info_t Drv_Info;
    wd_eth_info_t Drv_Info_Private;
    uint8_t Tx_Buf[WD_NET_ETHERNET_BUFFER_SIZE];
    uint8_t Rx_Buf[WD_NET_ETHERNET_BUFFER_SIZE];
} wd_net_eth_t;

extern wd_net_protocol_resources_t wd_net_eth_protocol;
extern wd_network_port_t wd_net_eth_net_port_ethernet;
#if (RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG == 1)
extern wd_network_port_t wd_net_eth_net_port_ethernet_debug;
#endif

void wd_net_eth_open(void);
void wd_net_eth_close(void);
void wd_net_eth_report(void);

#if (RTE_COMPONENT_INTERFACE_ETHERNET_TELNET)
void wd_eth_write_telnet(const void *data, uint32_t num, uint32_t arg);
int32_t wd_eth_telnet_ops(uint8_t sock_num);
#endif

#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/

#endif /* WD_SERVICE_WD_NET_ETH_H */
