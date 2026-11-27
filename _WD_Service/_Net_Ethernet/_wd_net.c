/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_net.c
 *
 *  Created on: Jan 7, 2025
 *      Author: doubled
 */

#include "wd.h"

#if (RTE_SERVICE_NET_CAN) || (RTE_SERVICE_NET_ETHERNET) /* common part of the network services */

//#if   (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_BOOTLOADER)
//#define WD_CONF_CONSOLE_LEVEL_COMPILE                                             WD_CONSOLE_LEVEL_INFO
//#elif (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
//#define WD_CONF_CONSOLE_LEVEL_COMPILE                                             WD_CONSOLE_LEVEL_INFO
//#endif


static const wd_net_user_protocol_t *Net_User_Protocol = NULL;

void wd_net_register_user_protocol(const wd_net_user_protocol_t *protocol) {
    Net_User_Protocol = protocol;
}

const wd_net_user_protocol_t *wd_net_get_user_protocol(void) {
    return Net_User_Protocol;
}

void wd_net_initialize_user_protocol(wd_network_port_t *port) {
    if ((Net_User_Protocol != NULL) && (Net_User_Protocol->Initialize != NULL)) {
        Net_User_Protocol->Initialize(port, 0);
    }
}

int32_t wd_net_protocol_parser(const void *port_ptr, uint8_t *head, uint8_t *tail, uint32_t arg) {
    wd_network_port_t *port = (wd_network_port_t *)port_ptr;
    wd_net_protocol_resources_t *protocol = (wd_net_protocol_resources_t *)port->Protocol;
    uint8_t *ptr = head;
    int32_t idx = 0;
    int find_protocol = false;

    UNUSED(protocol);
    UNUSED(head);
    UNUSED(tail);
    UNUSED(arg);
    UNUSED(find_protocol);

    while (ptr < tail) {
        if (idx > 0) {
            break;
        }

#if (RTE_COMPONENT_PROTOCOL_FACTORY)
        if (protocol->Profile->Current_Type & WD_PROTOCOL_TYPE_FACTORY) {
            idx = wd_drv_protocol_z_modem.Parser(port, (uint32_t)ptr, (uint32_t)tail, 0);
            if (idx > 0) {
                ptr += idx;
                find_protocol = true;
                continue;
            }
        }
#endif

        if ((protocol->Profile->Current_Type & WD_PROTOCOL_TYPE_USER) &&
            (Net_User_Protocol != NULL) && (Net_User_Protocol->Parser != NULL)) {
            idx = Net_User_Protocol->Parser(port, (uint32_t)ptr, (uint32_t)tail, 0);
            if (idx > 0) {
                ptr += idx;
                find_protocol = true;
                continue;
            }
        }


        if ((protocol->Profile->Current_Type & WD_PROTOCOL_TYPE_TEST_ECHO) || 0) {
            idx = (int32_t)(tail - head);
            memcpy(port->Info->Xfer.Tx_Buf, port->Info->Xfer.Rx_Buf, (uint32_t)(tail - head));
            port->Info->Tx_Msg.ID = port->ID;
            port->Info->Tx_Msg.Tyoe = 0;
            port->Info->Tx_Msg.Priority = 0;
            port->Info->Tx_Msg.Size = (uint16_t)(tail - head);
            port->Info->Tx_Msg.Ptr = port->Info->Xfer.Tx_Buf;
            if (osMessageQueuePut(port->Info->Msg_Queue, &port->Info->Tx_Msg, 0,
                                  RTE_RTOS_MESSAGEQUEUE_TIMEOUT) != osOK) {
                WD_LOG_ERROR("osMessageQueuePut Error !!!");
                osMessageQueueReset(port->Info->Msg_Queue);
            }

            if (idx > 0) {
                ptr += idx;
                find_protocol = true;
                continue;
            }
        }
        ptr++;
    }

    if (idx < 0) {
        WD_LOG_ERROR();
        idx = 0;
    }

    if (find_protocol) {
        if (head + idx > tail) {
            idx = tail - head;
        } else {
            idx = ptr - head;
        }
    } else {
        WD_PRINT_DEBUG(WD_MSG_ERROR_LOG, "Protocol Not Found");
        WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "<Unknown> %s", port->Capabilities->Info,
                       WD_MSG_MAGIC, WD_NET_RX_ASCII, head, tail - head);
        wd_time_wait_us(1); // DUMP_LOG 시 깨짐 발생_이건 USB 콘솔문제인거 같음.
        idx = 0;
    }

    return (idx);
}

#endif /* RTE_SERVICE_NET_CAN || RTE_SERVICE_NET_ETHERNET */
