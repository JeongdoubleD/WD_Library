/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_eth.h
 *
 *  Created on: Jan 6, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_ETH_H
#define WD_COMPONENT_WD_ETH_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "wd_core.h"

#ifndef RTE_NETWORK_MAC
#define RTE_NETWORK_MAC "4D-78-5A-5F-48-02" //"1E-30-6C-A2-45-5E"  // "4D-78-5A-5F-48-02"
#endif
#ifndef RTE_NETWORK_IP
#define RTE_NETWORK_IP "169.254.17.2"
#endif
#ifndef RTE_NETWORK_SUB_NET_MASK
#define RTE_NETWORK_SUB_NET_MASK "255.255.0.0"
#endif
#ifndef RTE_NETWORK_GATEWAY
#define RTE_NETWORK_GATEWAY "169.254.17.1"
#endif
#ifndef RTE_NETWORK_DNS
#define RTE_NETWORK_DNS "8.8.8.8"
#endif

#ifndef RTE_NETWORK_PORT
#define RTE_NETWORK_PORT "2000"
#endif
#ifndef RTE_NETWORK_DEBUG_PORT
#define RTE_NETWORK_DEBUG_PORT "20000"
#endif

#ifndef RTE_TELNET_IP
#define RTE_TELNET_IP RTE_NETWORK_IP
#endif
#ifndef RTE_TELNET_SUB_NET_MASK
#define RTE_TELNET_SUB_NET_MASK RTE_NETWORK_SUB_NET_MASK
#endif
#ifndef RTE_TELNET_GATEWAY
#define RTE_TELNET_GATEWAY RTE_NETWORK_GATEWAY
#endif
#ifndef RTE_TELNET_DNS
#define RTE_TELNET_DNS RTE_NETWORK_DNS
#endif
#ifndef RTE_TELNET_PORT
#define RTE_TELNET_PORT "54321"
#endif

//#define RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG                                  0  // 1 : Net, 2 : Terminal
#ifndef RTE_COMPONENT_INTERFACE_ETHERNET_KEEP_CONFIG
#define RTE_COMPONENT_INTERFACE_ETHERNET_KEEP_CONFIG \
    1 /* 1: skip W6100 re-init when already configured (soft reset) */
#endif
#ifndef RTE_COMPONENT_INTERFACE_ETHERNET_TELNET
#define RTE_COMPONENT_INTERFACE_ETHERNET_TELNET 1
#endif
#ifndef RTE_COMPONENT_INTERFACE_ETHERNET_BAUDRATE
#define RTE_COMPONENT_INTERFACE_ETHERNET_BAUDRATE 10000000u //30000000u
#endif
#ifndef RTE_COMPONENT_INTERFACE_ETHERNET_BUFFER_SIZE
#define RTE_COMPONENT_INTERFACE_ETHERNET_BUFFER_SIZE 2048
#endif
#ifndef RTE_COMPONENT_INTERFACE_ETHERNET_DRIVER_HANDLE
#define RTE_COMPONENT_INTERFACE_ETHERNET_DRIVER_HANDLE (&hspi6)
#endif
#ifndef RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP
#define RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP 0
#endif
#ifndef RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG
#define RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG 1
#endif
#ifndef RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE
#define RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE 2
#endif
#define RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_SIZE 3

//#define RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP              0
//#define RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG            1
//#define RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE                 2
//#define RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_SIZE                               3

#define WD_SOCKET_ETHERNET RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP
#define WD_SOCKET_DEBUG    RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG
#define WD_SOCKET_CONSOLE  RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE
#define WD_SOCKET_SIZE     RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_SIZE


extern char *wd_eth_socket_error_msg_box(int32_t error);
extern int32_t wd_eth_get_rx_count(uint8_t sock_num);
extern int32_t wd_eth_tcp_ops(uint8_t sock_num);


#if (RTE_COMPONENT_INTERFACE_ETHERNET_W6100)
/***************************************************************************************************************************************************************
 **********  W6100
 **************************************************************************************************************************************************************/
/*--------------- WizNet-W6100-EVB \ W6100_EVB_SPI_DMA \ wizchip_init.h ---------------*/
// SPI Raw Debug -> w6100.c WD_CONF_CONSOLE_LEVEL_COMPILE


#include "socket.h"
#include "wizchip_conf.h"


/* SPI2 */
#define W6100_SPI_TIMEOUT 10 //10
//#define DMA

#if (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_)
#ifdef DMA
#define WD_BUS_DMA
#endif
#elif (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_) || \
    (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_)
#ifdef DMA
#define WD_SPI_DMA
#endif
#endif

#define WD_SPI_DMA

void wd_eth_w6100_initialze(void);
void wd_eth_reset_w6100(void);

uint8_t wd_eth_w6100_spi_read_byte(void);
void wd_eth_w6100_spi_write_byte(uint8_t byte);

uint8_t W6100SpiDummyReadBurst(uint8_t *pBuf, uint16_t len);
void W6100SpiDummyWriteBurst(uint8_t *pBuf, uint16_t len);

void wd_eth_w6100_spi_read_burst(uint8_t *pBuf, int16_t len);
void wd_eth_w6100_spi_write_burst(uint8_t *pBuf, int16_t len);

void W6100BusWriteByte(uint32_t addr, iodata_t data);
iodata_t W6100BusReadByte(uint32_t addr);

#if defined WD_BUS_DMA

void W6100BusDmaWriteByte(uint32_t addr, iodata_t data);
iodata_t W6100BusDmaReadByte(uint32_t addr);

void W6100BusWriteBurst(uint32_t addr, uint8_t *pBuf, uint32_t len, uint8_t addr_inc);
void W6100BusReadBurst(uint32_t addr, uint8_t *pBuf, uint32_t len, uint8_t addr_inc);
#endif

void wd_eth_reset_w6100(void);
void wd_eth_enable_w6100_cs(void);
void wd_eth_disable_w6100_cs(void);
void wd_eth_w6100_reset_assert(void);
void wd_eth_w6100_reset_deassert(void);

#endif /*--------------- RTE_COMPONENT_INTERFACE_ETHERNET_W6100 ---------------*/

#if (RTE_COMPONENT_INTERFACE_ETHERNET) && \
    (RTE_COMPONENT_INTERFACE_ETHERNET_W6100) /* the declarations below need the W6100 types */

#if 0
/*--------------- AddressAutoConfig.h ---------------*/
#define AutoConfig_debug debug_on
#define debug_on         1
#define debug_off        0

#define PROTOCOL_NUM_ICMPv6 58

#define WD_ROUTER_ADVERTISEMENT 134

#define SUCCESS           0
#define WD_ERROR_DAD_FAIL -1
#define WD_ERROR_SLCMD    -2
#define WD_ERROR_TIMEOUT  -3

#define WD_SLAAC_RDNSS 0
#define WD_SLAAC_DHCP6 1
#define WD_SFAAC_DHCP6 3

#define WD_RAO_SLLA 1
#define WD_RAO_TLLA 2
#define WD_RAO_PI   3
#define WD_RAO_RH   4
#define WD_RAO_MTU  5
#define WD_RAO_RDNS 25

/**
 * @brief
 *
 * @param netinfo
 * @return uint8_t
 */
uint8_t AddressAutoConfig_Init(wiz_NetInfo* netinfo);

/**
 * @brief
 *
 * @param mac_addr
 * @param Link_Local_Addr
 */
void Generate_EUI64(uint8_t *mac_addr, uint8_t *Link_Local_Addr);

/**
 * @brief
 *
 * @param netinfo
 * @return uint8_t
 */
uint8_t Duplicate_Address_Detection(wiz_NetInfo* netinfo);

/**
 * @brief
 *
 * @param sn
 * @param icmpbuf
 * @param buf_size
 * @param netinfo
 * @return uint8_t
 */
uint8_t Address_Auto_Config_RA(uint8_t sn, uint8_t *icmpbuf, uint16_t buf_size, wiz_NetInfo* netinfo);

/**
 * @brief
 *
 * @param sn
 * @param test_buf
 * @return uint8_t
 */
uint8_t Address_Auto_Config_SLDHCP(uint8_t sn, uint8_t* test_buf);

/**
 * @brief
 *
 * @param sn
 * @param test_buf
 * @return uint8_t
 */
uint8_t Address_Auto_Config_SFDHCP(uint8_t sn, uint8_t* test_buf, wiz_NetInfo* netinfo);
#endif

/*--------------- loopback.h ---------------*/
#if 0
#define WD_LOOPBACK_DEBUG

/* WD_DATA_BUF_SIZE define for Loopback example */
#ifndef WD_DATA_BUF_SIZE
#define WD_DATA_BUF_SIZE 2048
#endif

int32_t loopback_udps(uint8_t sn, uint8_t* buf, uint16_t port, uint8_t loopback_mode);
int32_t loopback_tcps(uint8_t sn, uint8_t* buf, uint16_t port, uint8_t loopback_mode);
int32_t loopback_tcpc(uint8_t sn, uint8_t* buf, uint8_t* destip, uint16_t destport, uint8_t loopback_mode);
#endif

/***************************************************************************************************************************************************************
 **********  Driver
 **************************************************************************************************************************************************************/
typedef struct {
    uint32_t Octet_Timeout; // Tx 전송 이후 추가 딜레이
} wd_eth_extend_capabilities_t;

typedef struct {
#if 0
  uint8_t                              Role;
  uint8_t                              Family;
  uint8_t                              MAC[6];
  wd_ipv4_t                            IPv4;
#endif
    uint32_t IP_Protocol;
    uint8_t Sock_Descriptor;
    uint8_t Padding[3];
    wd_eth_extend_capabilities_t *Extend;
} wd_eth_capabilities_t;

typedef struct {
    uint32_t Dummy;
} wd_eth_info_t;

extern wd_network_driver_t wd_eth_data;

typedef wd_event_callback_t wd_eth_socket_signal_event_t;

/****** SOCKET Event *****/
#define ARM_SOCKET_EVENT_CONNECT    (1UL << 0)
#define ARM_SOCKET_EVENT_DISCONNECT (1UL << 1)
#define ARM_SOCKET_EVENT_RECV       (1UL << 2)
#define ARM_SOCKET_EVENT_TIMEOUT    (1UL << 3)
#define ARM_SOCKET_EVENT_SEND_OK    (1UL << 4)

typedef struct {
    void *Owner;
    wd_eth_socket_signal_event_t CB_Event;
    int32_t volatile State;
    uint32_t volatile Event;
    uint32_t Time_Stamp;
    uint32_t Wait_Time;
    uint32_t Timer;
    uint8_t Status;
    uint8_t Status_Old;
    uint16_t Port;
} wd_eth_socket_t;

typedef struct {
    int32_t (*Initialize)(uint32_t arg);
    int32_t (*Send)(uint8_t sock_num, void *data, uint32_t num);
    int32_t (*Receive)(uint8_t sock_num, void *data, uint32_t num);
    int32_t (*GetRxCount)(uint8_t sock_num);
    int32_t (*Ops)(uint8_t sock_num, uint32_t ipproto);
    int32_t *State;
    wiz_NetInfo *Net_Info;
} const wd_eth_t;

extern wd_eth_t wd_eth_driver_ethernet_module;
extern wd_eth_socket_t wd_eth_socket[RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_SIZE];

#if (RTE_PLATFORM_PRINT_TELNET) && (RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG == 2)
void Driver_Ethernet_Terminal_Write(const void *data, uint32_t num, uint32_t arg);
int32_t Driver_Ethernet_Terminal_Ops(uint8_t sock_num);
#endif

#if (RTE_COMPONENT_INTERFACE_ETHERNET_TELNET)
void wd_eth_write_telnet(const void *data, uint32_t num, uint32_t arg);
int32_t wd_eth_telnet_ops(uint8_t sock_num);
#endif


/* Application hook (weak) : fill network configuration. return true when filled. */
bool wd_eth_user_net_info(wiz_NetInfo *net_info, uint16_t *port_app, uint16_t *port_debug);

/**
 * @brief  Enable / disable packet dump logging of the application and debug TCP ports.
 * @note   Default : both enabled. The system applies wd_env->Net_Debug at start-up (Environment).
 */
void wd_eth_set_packet_dump(bool app_port, bool debug_port);

#endif /* RTE_COMPONENT_INTERFACE_ETHERNET && RTE_COMPONENT_INTERFACE_ETHERNET_W6100 */

#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/

#endif /* WD_COMPONENT_WD_ETH_H */
