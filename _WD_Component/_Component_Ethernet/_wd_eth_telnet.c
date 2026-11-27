/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_eth_telnet.c
 * @brief   Telnet console over the W6100 Ethernet interface (moved from the core _PF_Print)
 ******************************************************************************/
#include "_wd_eth.h"
#include "_wd_eth_telnet.h"

#if (RTE_PLATFORM_PRINT_TELNET) && (RTE_COMPONENT_INTERFACE_ETHERNET)

#ifndef WD_CONF_TELNET_TX_BUF_SIZE
#define WD_CONF_TELNET_TX_BUF_SIZE (4096U)
#endif
#ifndef WD_CONF_TELNET_RX_BUF_SIZE
#define WD_CONF_TELNET_RX_BUF_SIZE (256U)
#endif

int32_t wd_eth_telnet_state = 0;
static uint8_t Telnet_Tx_Buf[WD_CONF_TELNET_TX_BUF_SIZE];
static uint8_t Telnet_Rx_Buf[WD_CONF_TELNET_RX_BUF_SIZE];
wd_cbuf_t wd_eth_telnet_tx_c_buf;
wd_cbuf_t wd_eth_telnet_rx_c_buf;
uint32_t wd_eth_telnet_tx_time_stamp;
uint32_t wd_eth_telnet_rx_time_stamp;

/* ----------------------------------------------------------------------------- local functions */
static bool telnet_is_connected(void) {
    return ((((uint32_t)wd_eth_telnet_state & (uint32_t)WD_TELNET_STATE_ENABLE) != 0U) &&
            (wd_eth_socket[RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE].Status ==
             SOCK_ESTABLISHED));
}

/* core console output sink (was the Telnet branch of wd_print_hal_uart1_put_char) */
static void telnet_putc(uint8_t ch) {
    if (telnet_is_connected() == true) {
        wd_eth_telnet_write_telnet(&ch, 1U, 0U);
    }
}

/* core console input source (was the Telnet branch of wd_print_hal_uart1_get_char) */
static int32_t telnet_getc(void) {
    int32_t ch = -1;

    if ((telnet_is_connected() == true) && (wd_eth_telnet_tstc_telnet() != 0)) {
        uint8_t data = 0U;

        (void)wd_eth_telnet_read_telnet(&data, 1U, 0U);
        ch = (int32_t)data;
    }
    return ch;
}

/* ----------------------------------------------------------------------------- public functions */
int32_t wd_eth_telnet_initialize(void) {
    wd_eth_telnet_state = WD_STATE_INITIALIZED;

    wd_cbuf_init(&wd_eth_telnet_tx_c_buf, Telnet_Tx_Buf, sizeof(Telnet_Tx_Buf));
    wd_cbuf_init(&wd_eth_telnet_rx_c_buf, Telnet_Rx_Buf, sizeof(Telnet_Rx_Buf));

    wd_eth_telnet_tx_time_stamp = wd_time_get_us();
    wd_eth_telnet_rx_time_stamp = wd_time_get_us();

    (void)wd_print_register_output(telnet_putc);
    (void)wd_print_register_input(telnet_getc);

    return (WD_CODE_OK);
}

void wd_eth_telnet_write_telnet(const void *data, uint32_t num, uint32_t arg) {
    UNUSED(arg);

    wd_eth_telnet_tx_time_stamp = wd_time_get_us();
    wd_cbuf_push(&wd_eth_telnet_tx_c_buf, (uint8_t *)((uint32_t)data), num);
}

void wd_eth_telnet_flush_telnet(uint32_t arg) {
    UNUSED(arg);

#if (RTE_COMPONENT_INTERFACE_ETHERNET_TELNET)
    wd_cbuf_print_pop(&wd_eth_telnet_tx_c_buf, wd_eth_write_telnet, wd_eth_telnet_tx_c_buf.Size);
    wd_eth_telnet_tx_time_stamp = wd_time_get_us();
#endif
}

int32_t wd_eth_telnet_tstc_telnet(void) {
    int32_t size = 0;

    if (wd_eth_telnet_rx_c_buf.Size != 0U) {
        size = (int32_t)wd_eth_telnet_rx_c_buf.Size;
    }
    return size;
}

int32_t wd_eth_telnet_read_telnet(void *data, uint32_t num, uint32_t arg) {
    UNUSED(arg);

    wd_cbuf_pop(&wd_eth_telnet_rx_c_buf, (uint8_t *)((uint32_t)data), num);

    return ((int32_t)num);
}

#endif /* RTE_PLATFORM_PRINT_TELNET && RTE_COMPONENT_INTERFACE_ETHERNET */
