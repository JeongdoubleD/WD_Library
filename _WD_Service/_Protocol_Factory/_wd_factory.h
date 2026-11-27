/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_factory.h
 *
 *  Created on: Jan 13, 2025
 *      Author: doubled
 */

/* umbrella header first (before the guard) : any include order expands in the umbrella order */
#include "wd.h"

#ifndef WD_SERVICE_WD_FACTORY_H
#define WD_SERVICE_WD_FACTORY_H


/* board HAL mapping : default CubeMX names, override in wd_board.h */
#ifndef RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE
#define RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE (&huart8)
#endif

#if 1 // 토글

/***************************************************************************************************************************************************************
 **********  XYZ Modem
 **************************************************************************************************************************************************************/
typedef enum {
    WD_PROTOCOL_Z_MODEM_CMD_SEND_16_BIT_CRC_BINARY_HEADER,
    WD_PROTOCOL_Z_MODEM_CMD_SEND_32_BIT_CRC_BINARY_HEADER,
    WD_PROTOCOL_Z_MODEM_CMD_SEND_HEX_HEADER,
} wd_factory_protocol_z_modem_cmd_t;

typedef struct {
    uint32_t Check_Sum;

} wd_factory_protocol_z_modem_capabilities_t;

typedef enum {
    WD_PROTOCOL_Z_MODEM_STATE_NONE = 0,
    WD_PROTOCOL_Z_MODEM_STATE_DISCONNECTED,
    WD_PROTOCOL_Z_MODEM_STATE_IDLE,
    WD_PROTOCOL_Z_MODEM_STATE_CONNECTING,
    WD_PROTOCOL_Z_MODEM_STATE_CONNECTED,
    WD_PROTOCOL_Z_MODEM_STATE_DISCONNECTING,
    WD_PROTOCOL_Z_MODEM_STATE_START,
    WD_PROTOCOL_Z_MODEM_STATE_WORK,
    WD_PROTOCOL_Z_MODEM_STATE_END,

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
    WD_PROTOCOL_Z_MODEM_STATE_ALL = (int)0xFFFFFFFF,
#elif defined(__ICCARM__)
    WD_PROTOCOL_Z_MODEM_STATE_ALL = 0xFFFFFFFF,
#endif
} wd_factory_protocol_z_modem_state_t;

typedef struct {
    wd_factory_protocol_z_modem_state_t State;

    wd_factory_protocol_z_modem_capabilities_t Capabilities;

    wd_message_t Tx_Msg;
    wd_message_t Rx_Msg;

    uint8_t Request_Header[4];
    uint8_t Response_Header[4];

    void *APDU_Set;
    uint16_t APDU_Set_Len;
    uint16_t APDU_Set_Offset;

    uint8_t APDU[256];
    uint16_t APDU_Len;
    uint16_t APDU_Offset;
} wd_factory_protocol_z_modem_t;

typedef struct {
    int32_t (*Initialize)(wd_network_port_t *port, uint32_t arg);
    int32_t (*Process)(wd_network_port_t *port, uint32_t arg);
    int32_t (*Make_Frame)(wd_network_port_t *port, uint8_t *ptr, uint32_t num,
                          wd_message_t *message, uint32_t arg);
    int32_t (*Parser)(wd_network_port_t *port, uint32_t head, uint32_t tail, uint32_t arg);
    int32_t (*Control)(wd_network_port_t *port, uint32_t arg);
} const wd_factory_protocol_driver_z_modem_t;

extern wd_factory_protocol_driver_z_modem_t wd_drv_protocol_z_modem;


typedef struct {
    wd_factory_protocol_z_modem_t Z_Modem;
    uint32_t dummy;
} wd_factory_t;


/***************************************************************************************************************************************************************
 **********  Y-Modem Receive (UART Console OTA)
 **************************************************************************************************************************************************************/
int32_t wd_factory_receive_ymodem_firmware(void);


#endif // #if 1 // 토글

#endif /* WD_SERVICE_WD_FACTORY_H */
