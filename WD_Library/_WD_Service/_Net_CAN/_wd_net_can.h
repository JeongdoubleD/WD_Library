/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_net_can.h
 *
 *  Created on: Nov 17, 2025
 *      Author: doubled
 */

#ifndef WD_SERVICE_WD_NET_CAN_H
#define WD_SERVICE_WD_NET_CAN_H

#include "wd.h"

#if (RTE_SERVICE_NET_CAN)

#define RTE_SERVICE_NET_CAN_BUF_SIZE 1024U
#define WD_NET_CAN_MESSAGE_POOL_SIZE 3UL

typedef struct {
    wd_network_info_t Drv_Info;
    wd_fdcan_info_t Drv_Info_Private;
    uint8_t Tx_Buf[RTE_SERVICE_NET_CAN_BUF_SIZE];
    uint8_t Rx_Buf[RTE_SERVICE_NET_CAN_BUF_SIZE];
} wd_net_can_t;

/*
 * Net 계층에서 사용할 CAN Port 핸들
 * (Ethernet의 wd_net_eth_net_port_ethernet 과 동일 역할)
 */
extern wd_network_port_t wd_net_can_net_port_can;

/* 서비스 제어 함수 */
void wd_net_can_open(void);
void wd_net_can_close(void);
void wd_net_can_report(void);

#endif

#endif /* WD_SERVICE_WD_NET_CAN_H */
