/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_net_can.c
 *
 *  Created on: Nov 17, 2025
 *      Author: doubled
 */

#include "wd.h"

#if (RTE_SERVICE_NET_CAN)


#undef LOCAL_PREFIX_NAME
#define LOCAL_PREFIX_NAME "CAN"

/*********************************************************************************************************************
 *  Message Queue Attribute
 *********************************************************************************************************************/

static uint8_t Net_CAN_Queue_stack_mem[(sizeof(wd_message_t) * RTE_SERVICE_NET_QUEUE_SIZE)];
static osStaticMutexDef_t Net_CAN_Queue_cb_mem;
static const osMessageQueueAttr_t NET_CAN_QUEUE_ATTR = {.name = "CAN Message Queue",
                                                        .cb_mem = &Net_CAN_Queue_cb_mem,
                                                        .cb_size = sizeof(Net_CAN_Queue_cb_mem),
                                                        .mq_mem = Net_CAN_Queue_stack_mem,
                                                        .mq_size = sizeof(Net_CAN_Queue_stack_mem)};
/*********************************************************************************************************************
 *  내부 구조체
 *********************************************************************************************************************/
/*********************************************************************************************************************
 *  Driver Capabilities / Network Capabilities
 *********************************************************************************************************************/
static wd_fdcan_capabilities_t NET_CAN_CAPABILITIES = {
    .Dummy = 0,
};
static const wd_network_capabilities_t NET_PORT_CAN_CAPABILITIES = {
    .Info = LOCAL_PREFIX_NAME,
    .Tx_Wait_Timer = 0,
    .Rx_Inter_Octet_Timeout = 0,
    .Rx_Frame_Timeout = 0,
    .Private = (void *)(&NET_CAN_CAPABILITIES)};


static wd_net_can_t Net_CAN = {
    0,
};


static const wd_net_protocol_capabilities_t NET_PORT_CAN_PROTOCOL_CAPABILITIES = {
    .ID = WD_SYMBOL_02_NET_CAN, /* TODO: wd_system_symbol_t enum에 CAN용 ID 추가 */
    .Available_Type =
        WD_PROTOCOL_TYPE_USER, /* application protocol (wd_net_register_user_protocol) */
};

static wd_net_protocol_profile_t Net_CAN_Protocol_Profile;

static const wd_net_protocol_resources_t Net_CAN_Protocol = {
    .Capabilities = (wd_net_protocol_capabilities_t *)&NET_PORT_CAN_PROTOCOL_CAPABILITIES,
    .Profile = &Net_CAN_Protocol_Profile,
#if (RTE_COMPONENT_PROTOCOL_FACTORY)
    .Factory = NULL, /* 나중에 _Protocol_FDCAN 에서 Factory/FDCAN 포인터 연결 */
#endif
};


/*********************************************************************************************************************
 *  이벤트 콜백
 *********************************************************************************************************************/

static void Net_CAN_Event_Callback(uint32_t event) {
    Net_CAN.Drv_Info.Event |= event;
}

/*********************************************************************************************************************
 *  wd_network_port_t 인스턴스
 *********************************************************************************************************************/

wd_network_port_t wd_net_can_net_port_can = {
    .ID = (wd_system_symbol_t)
        WD_SYMBOL_02_NET_CAN, /* TODO: wd_system_symbol_t enum에 CAN용 ID 추가 */
    .Capabilities = &NET_PORT_CAN_CAPABILITIES,
    .Signal_Event = Net_CAN_Event_Callback,
    .Tx_Buf = Net_CAN.Tx_Buf,
    .Tx_Buf_Size = sizeof(Net_CAN.Tx_Buf),
    .Rx_Buf = Net_CAN.Rx_Buf,
    .Rx_Buf_Size = sizeof(Net_CAN.Rx_Buf),
    .Msg_Queue_Attr = &NET_CAN_QUEUE_ATTR,
    .Msg_Queue_Size = RTE_SERVICE_NET_QUEUE_SIZE,
    .Drv = (void const *)&wd_fdcan_module,
    .Handle = (void *)RTE_COMPONENT_CAN_DRIVER_HANDLE,
    .Protocol = (void const *)&Net_CAN_Protocol,
    .Parser = wd_net_protocol_parser,
    .Info = (void *)&Net_CAN.Drv_Info,
    .Private = (void *)&Net_CAN.Drv_Info_Private};

/*********************************************************************************************************************
 *  Public 함수 구현
 *********************************************************************************************************************/

void wd_net_can_open(void) {
    wd_fdcan_data.Register(&wd_net_can_net_port_can);
    wd_net_can_net_port_can.Info->Status.Power = true;
    wd_fdcan_data.Check(&wd_net_can_net_port_can);
    wd_net_can_net_port_can.Info->Status.Configure = true;

    Net_CAN_Protocol.Profile->Current_Type = Net_CAN_Protocol.Capabilities->Available_Type;

    if (Net_CAN_Protocol.Profile->Current_Type & WD_PROTOCOL_TYPE_USER) {
        wd_net_initialize_user_protocol(&wd_net_can_net_port_can);
    }
    WD_LOG_INFO("Net_CAN: Open");
}

void wd_net_can_close(void) {
    wd_fdcan_data.Unregister(&wd_net_can_net_port_can);
    wd_net_can_net_port_can.Info->Status.Power = false;
    wd_net_can_net_port_can.Info->Status.Configure = false;

    WD_LOG_INFO("Net_CAN: Close");
}

void wd_net_can_report(void) {
    /* 필요하다면 CAN 상태/통계 출력 */
    WD_LOG_INFO("Net_CAN: Tx[%u] Rx[%u] Err[%u]", (unsigned long)Net_CAN.Drv_Info_Private.TxCount,
                (unsigned long)Net_CAN.Drv_Info_Private.RxCount,
                (unsigned long)Net_CAN.Drv_Info_Private.ErrorCount);
}

#endif
