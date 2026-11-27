/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_net.h
 *
 *  Created on: Jan 7, 2025
 *      Author: doubled
 */

/* umbrella header first (before the guard) : any include order expands in the umbrella order */
#include "wd.h"

#ifndef WD_SERVICE_WD_NET_H
#define WD_SERVICE_WD_NET_H


#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    WD_PROTOCOL_TYPE_IDLE = 0,
    WD_PROTOCOL_TYPE_FACTORY = WD_BIT0,
    WD_PROTOCOL_TYPE_USER = WD_BIT1, // application protocol (wd_net_register_user_protocol)
    WD_PROTOCOL_TYPE_TEST_ECHO = WD_BIT15,
} wd_net_protocol_type_t;

typedef enum {
    WD_PROTOCOL_STATE_NONE = 0,
    WD_PROTOCOL_STATE_INIT = 1,
    WD_PROTOCOL_STATE_IDLE = 2,
    WD_PROTOCOL_STATE_RUN = 3,
    WD_PROTOCOL_STATE_STOP = 4,
    WD_PROTOCOL_STATE_BUSY = 5,
} wd_net_protocol_state_t;

typedef enum { WD_PROTOCOL_OPTION_NONE = 0 } wd_net_protocol_option_t;

typedef struct {
    uint32_t State;
    uint32_t Current_Type;
    uint32_t Timer;
} wd_net_protocol_profile_t;

typedef struct {
    wd_system_symbol_t ID;
    uint32_t Available_Type;
} const wd_net_protocol_capabilities_t;

typedef struct {
    wd_net_protocol_capabilities_t *Capabilities;
    wd_net_protocol_profile_t *Profile;
#if (RTE_COMPONENT_PROTOCOL_FACTORY)
    wd_factory_t *Factory;
#endif
} const wd_net_protocol_resources_t;


/**
 * @brief Application protocol (registered once, shared by all network ports : Ethernet / Ethernet debug / CAN)
 *        Parser  : return number of consumed bytes (>0) when a frame was handled, 0 when not matched
 *        Initialize is called at Net_xxx_Open() for each port that enables WD_PROTOCOL_TYPE_USER
 */
typedef struct {
    int32_t (*Initialize)(wd_network_port_t *port, uint32_t arg);
    int32_t (*Parser)(wd_network_port_t *port, uint32_t head, uint32_t tail, uint32_t arg);
} wd_net_user_protocol_t;

void wd_net_register_user_protocol(const wd_net_user_protocol_t *protocol);
const wd_net_user_protocol_t *wd_net_get_user_protocol(void);
void wd_net_initialize_user_protocol(wd_network_port_t *port);

/** @brief frame parser of the network service : assigned to wd_network_port_t.Parser of every Net_xxx port */
int32_t wd_net_protocol_parser(const void *port_ptr, uint8_t *head, uint8_t *tail, uint32_t arg);

#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/


#endif /* WD_SERVICE_WD_NET_H */
