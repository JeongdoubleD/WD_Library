/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_eth.c
 *
 *  Created on: Jan 6, 2025
 *      Author: doubled
 */

//#if 0
//#undef  WD_CONF_CONSOLE_LEVEL_COMPILE
//#undef  _CONF_CONSOLE_LEVEL_LOCAL_VV
//#elif  (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_BOOTLOADER)
//#define WD_CONF_CONSOLE_LEVEL_COMPILE                                             WD_CONSOLE_LEVEL_NOTICE
//#elif (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
//#define WD_CONF_CONSOLE_LEVEL_COMPILE                                             WD_CONSOLE_LEVEL_NOTICE // WD_CONSOLE_LEVEL_INFO
///*
//  WD_BIT0 : Socket Event SEND_OK / RECV
//*/
//#define _CONF_CONSOLE_LEVEL_LOCAL_VV                                            NULL
//#endif
//

#include "_wd_eth.h"
#include "_wd_eth_telnet.h"

#if 1
#if (RTE_COMPONENT_INTERFACE_ETHERNET)
/**************************************************************************************************************************************************************/
#if 0 /// Config *********************************************************************************************************************************************{
/**************************************************************************************************************************************************************/
#define LOCAL_NAME "[Ethernet]"

#define LOCAL_QUEUE_SIZE   8
#define LOCAL_LOCK_TIMEOUT 1000

#define LOCAL_IF_INTER_OCTET_TIMEOUT 5   // [ms]
#define LOCAL_IF_FRAME_TIMEOUT       250 // [ms]

#define LOCAL_IP6_HEADER_LEN 40 // IPv6 Header Length in bytes
#define LOCAL_IP6_DATA_OFFS  (PHY_HEADER_LEN + LOCAL_IP6_HEADER_LEN)
#define LOCAL_IP6_MTU        (ETH_MAX_FRAME_SIZE - LOCAL_IP6_DATA_OFFS)

#define LOCAL_UDP_NUM_SOCKS  1
#define LOCAL_UDP_HEADER_LEN 8 // UDP Header Length in bytes
#define LOCAL_UDP_DATA_OFFS  (LOCAL_IP6_DATA_OFFS + LOCAL_UDP_HEADER_LEN)
#define LOCAL_UDP_MTU        2000 // UDP default max. transmission unit

#define LOCAL_TCP_NUM_SOCKS          6
#define LOCAL_TCP_MAX_RETRY          5
#define LOCAL_TCP_RETRY_TOUT         4   // [s]
#define LOCAL_TCP_DEFAULT_TOUT       120 // [s]
#define LOCAL_TCP_MAX_SEG_SIZE       1440
#define LOCAL_TCP_RECEIVE_WIN_SIZE   4320
#define LOCAL_TCP_INITIAL_RETRY_TOUT 1 // [s]
#define LOCAL_TCP_SYN_RETRY_TOUT     2 // [s]
#define LOCAL_TCP_CONNECT_RETRY      7

#define LOCAL_TCP_HEADER_LEN  20 // TCP Header Length in bytes
#define LOCAL_TCP_OPT_LEN     40 // Max TCP Header option field length
#define LOCAL_TCP_MAX_HLEN    (LOCAL_TCP_HEADER_LEN + LOCAL_TCP_OPT_LEN)
#define LOCAL_TCP_DATA_OFFS   (LOCAL_IP6_DATA_OFFS + LOCAL_TCP_HEADER_LEN)
#define LOCAL_TCP_MTU         1440 // TCP default max. transmission unit
#define LOCAL_TCP_DEF_MSS     536  // Standard/default MSS value for TCP
#define LOCAL_TCP_DEF_WINSIZE 4320 // TCP default window size

#define LOCAL_SOKET_TIMEOUT      1000 // [ms]
#define LOCAL_SYS_TICK_INTERVAL  1    // [ms]
#define LOCAL_NETOS_WAIT_FOREVER 0xFFFFFFFF

#define LOCAL_DEBUGF(fmt, ...) WD_PRINT_INFO(WD_MSG_INFO, LOCAL_NAME " " fmt, ##__VA_ARGS__);
#define LOCAL_ERRORF(fmt, ...) WD_PRINT_INFO(WD_MSG_ERROR_DEBUG, LOCAL_NAME " " fmt, ##__VA_ARGS__);
#define LOCAL_SOCKADDR4(addr)  ((SOCKADDR_IN *)((uint32_t)addr))
#define LOCAL_SOCKADDR6(addr)  ((SOCKADDR_IN6 *)((uint32_t)addr))
#define LOCAL_FD_SET(fd, set)  (set)->fd_bits[(fd - 1) >> 5] |= (1U << ((fd - 1) & 0x1F))
#define LOCAL_FD_CLR(fd, set)  (set)->fd_bits[(fd - 1) >> 5] &= ~(1U << ((fd - 1) & 0x1F))
#define LOCAL_FD_ISSET(fd, set) ((set)->fd_bits[(fd - 1) >> 5] & (1U << ((fd - 1) & 0x1F)))


#ifndef LOCAL_FALLTHROUGH
#define LOCAL_FALLTHROUGH __attribute__((fallthrough))
#endif

static WD_INTERFACE_ETHERNET_CAPABILITIES s_private_capabilities =
{
  .IP_Protocol     = IPPROTO_TCP,
  .Sock_Descriptor = NULL,
  .Extend          = NULL
};

static const wd_network_capabilities_t s_capabilities =
{
  .Name                   = LOCAL_NAME,
  .Tx_Time_Wait           = 0,
  .Rx_Inter_Octet_Timeout = LOCAL_IF_INTER_OCTET_TIMEOUT,
  .Rx_Frame_Timeout       = LOCAL_IF_FRAME_TIMEOUT,                           // [ms]
  .Private                = (void *)(&s_private_capabilities)
};

__USED static WDIF_NETWORK_INTERFACE * const __IF     = &Interface_Ethernet;
__USED static WDIF_SOCKET            * const __SOCKET = &Socket_Ethernet;
#endif /// Config *********************************************************************************************************************************************}


/**************************************************************************************************************************************************************/
#if 1 /// W6100 **********************************************************************************************************************************************{
/**************************************************************************************************************************************************************/
#if (RTE_COMPONENT_INTERFACE_ETHERNET_W6100)
#define SPI_HANDLE RTE_COMPONENT_INTERFACE_ETHERNET_DRIVER_HANDLE

// port->Info->Event 내부 비트플래그 (이 파일 안에서만 set/check/clear, 기존 ARM_USART_EVENT_* 와 동일한 값 유지)
#define WD_USART_EVENT_RECEIVE_COMPLETE (1UL << 1)
#define WD_USART_EVENT_TX_COMPLETE      (1UL << 3)
#define WD_USART_EVENT_RX_OVERFLOW      (1UL << 5)

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-sign"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#endif
/*--------------- WizNet-W6100-EVB \ W6100_EVB_SPI_DMA \ wizchip_init.c ---------------*/
#ifdef USE_STDPERIPH_DRIVER
DMA_InitTypeDef DMA_RX_InitStructure, DMA_TX_InitStructure;
#elif defined USE_HAL_DRIVER
#endif
void wd_eth_w6100_initialze(void) {
    //	wd_gpio_config(RTE_PORT_W6100_RESET, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
    //	wd_gpio_config(RTE_PORT_W6100_CS, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);

    wd_eth_reset_w6100();
    wd_time_wait_ms(300);

#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
    /* SPI method callback registration */
    //  WD_LOG_INFO("SPI DMA setting \r\n");
    //  reg_wizchip_spi_cbfunc(wd_eth_w6100_spi_read_byte, wd_eth_w6100_spi_write_byte, wd_eth_w6100_spi_read_burst, wd_eth_w6100_spi_write_burst);
    reg_wizchip_spi_cbfunc(wd_eth_w6100_spi_read_byte, wd_eth_w6100_spi_write_byte, 0, 0);
    /* CS function register */
    reg_wizchip_cs_cbfunc(wd_eth_enable_w6100_cs, wd_eth_disable_w6100_cs);
#else
    /* Indirect bus method callback registration */
#if defined WD_BUS_DMA
    //  WD_LOG_INFO("BUS DMA setting \r\n");
    reg_wizchip_bus_cbfunc(W6100BusDmaReadByte, W6100BusDmaWriteByte, 0, 0);
#else
    WD_LOG_INFO("no BUS DMA setting \r\n");
    reg_wizchip_bus_cbfunc(W6100BusReadByte, W6100BusWriteByte, 0, 0);
#endif
#endif
    if ((WIZCHIP_READ(_CIDR_) != 0x61) || (WIZCHIP_READ(_VER_) != 0x46)) {
        WD_LOG_ERROR("W6100 SPI Error");
    }
}

#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
inline void wd_eth_enable_w6100_cs(void) {
    wd_gpio_write(RTE_PORT_W6100_CS, WD_GPIO_ON);
}

inline void wd_eth_disable_w6100_cs(void) {
    wd_gpio_write(RTE_PORT_W6100_CS, WD_GPIO_OFF);
}

uint8_t wd_eth_w6100_spi_read_byte(void) {
    uint8_t tx = 0xFF;
    uint8_t rx = 0x00;

    WD_WAIT_WHILE(SPI_HANDLE->State != HAL_SPI_STATE_READY, 10000);

    if (HAL_SPI_TransmitReceive(SPI_HANDLE, &tx, &rx, 1, W6100_SPI_TIMEOUT) != HAL_OK) {
        WD_LOG_ERROR();
    }

    return rx;
}

void wd_eth_w6100_spi_write_byte(uint8_t byte) {
    uint8_t rx;

    WD_WAIT_WHILE(SPI_HANDLE->State != HAL_SPI_STATE_READY, 10000);

    if (HAL_SPI_TransmitReceive(SPI_HANDLE, &byte, &rx, 1, W6100_SPI_TIMEOUT) != HAL_OK) {
        WD_LOG_ERROR();
    }
}

void wd_eth_w6100_spi_read_burst(uint8_t *pBuf, int16_t len) {
    WD_WAIT_WHILE(HAL_SPI_GetState(SPI_HANDLE) != HAL_SPI_STATE_READY &&
                      HAL_DMA_GetState(SPI_HANDLE->hdmarx) != HAL_DMA_STATE_READY &&
                      HAL_DMA_GetState(SPI_HANDLE->hdmatx) != HAL_DMA_STATE_READY,
                  10000);

    if (HAL_SPI_Receive_DMA(SPI_HANDLE, pBuf, (uint16_t)len) != HAL_OK) {
        WD_LOG_ERROR();
    }

    WD_WAIT_WHILE(HAL_DMA_GetState(SPI_HANDLE->hdmarx) == HAL_DMA_STATE_BUSY ||
                      HAL_DMA_GetState(SPI_HANDLE->hdmarx) == HAL_DMA_STATE_RESET,
                  10000);
    WD_WAIT_WHILE(HAL_DMA_GetState(SPI_HANDLE->hdmatx) == HAL_DMA_STATE_BUSY ||
                      HAL_DMA_GetState(SPI_HANDLE->hdmatx) == HAL_DMA_STATE_RESET,
                  10000);
}

void wd_eth_w6100_spi_write_burst(uint8_t *pBuf, int16_t len) {
    WD_WAIT_WHILE(HAL_SPI_GetState(SPI_HANDLE) != HAL_SPI_STATE_READY &&
                      HAL_DMA_GetState(SPI_HANDLE->hdmarx) != HAL_DMA_STATE_READY &&
                      HAL_DMA_GetState(SPI_HANDLE->hdmatx) != HAL_DMA_STATE_READY,
                  10000);

    if (HAL_SPI_Transmit_DMA(SPI_HANDLE, pBuf, (uint16_t)len) != HAL_OK) {
        WD_LOG_ERROR();
    }

    WD_WAIT_WHILE(HAL_DMA_GetState(SPI_HANDLE->hdmarx) == HAL_DMA_STATE_BUSY ||
                      HAL_DMA_GetState(SPI_HANDLE->hdmarx) == HAL_DMA_STATE_RESET,
                  10000);
    WD_WAIT_WHILE(HAL_DMA_GetState(SPI_HANDLE->hdmatx) == HAL_DMA_STATE_BUSY ||
                      HAL_DMA_GetState(SPI_HANDLE->hdmatx) == HAL_DMA_STATE_RESET,
                  10000);
}
#else
void W6100BusWriteByte(uint32_t addr, iodata_t data) {
    (*(volatile uint8_t *)(addr)) = data;
}

iodata_t W6100BusReadByte(uint32_t addr) {
    return (*((volatile uint8_t *)(addr)));
}

void W6100BusDmaWriteByte(uint32_t addr, iodata_t data) {
#if defined USE_HAL_DRIVER
    while (HAL_DMA_GetState(&W6100_DMA_TX) != HAL_DMA_STATE_READY)
        ;
    HAL_DMA_Start_IT(&W6100_DMA_TX, &data, (uint32_t)addr, 1);
    while (HAL_DMA_GetState(&W6100_DMA_TX) == HAL_DMA_STATE_RESET)
        ;
#endif
}

iodata_t W6100BusDmaReadByte(uint32_t addr) {
#if defined USE_HAL_DRIVER
    iodata_t ret;

    while (HAL_DMA_GetState(&W6100_DMA_RX) != HAL_DMA_STATE_READY)
        ;
    HAL_DMA_Start_IT(&W6100_DMA_RX, (uint32_t)addr, &ret, 1);
    while (HAL_DMA_GetState(&W6100_DMA_RX) == HAL_DMA_STATE_RESET)
        ;

    return ret;
#endif
}

void W6100BusWriteBurst(uint32_t addr, uint8_t *pBuf, uint32_t len, uint8_t addr_inc) {
#ifdef USE_STDPERIPH_DRIVER

    if (addr_inc) {
        DMA_TX_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

    } else {
        DMA_TX_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    }


    DMA_TX_InitStructure.DMA_BufferSize = len;
    DMA_TX_InitStructure.DMA_MemoryBaseAddr = addr;
    DMA_TX_InitStructure.DMA_PeripheralBaseAddr = pBuf;

    DMA_Init(W6100_DMA_CHANNEL_TX, &DMA_TX_InitStructure);

    DMA_Cmd(W6100_DMA_CHANNEL_TX, ENABLE);

    /* Enable SPI Rx/Tx DMA Request*/


    /* Waiting for the end of Data Transfer */
    while (DMA_GetFlagStatus(DMA_TX_FLAG) == RESET)
        ;


    DMA_ClearFlag(DMA_TX_FLAG);

    DMA_Cmd(W6100_DMA_CHANNEL_TX, DISABLE);

#elif defined USE_HAL_DRIVER

#endif
}

void W6100BusReadBurst(uint32_t addr, uint8_t *pBuf, uint32_t len, uint8_t addr_inc) {
#ifdef USE_STDPERIPH_DRIVER

    DMA_RX_InitStructure.DMA_BufferSize = len;
    DMA_RX_InitStructure.DMA_MemoryBaseAddr = pBuf;
    DMA_RX_InitStructure.DMA_PeripheralBaseAddr = addr;

    DMA_Init(W6100_DMA_CHANNEL_RX, &DMA_RX_InitStructure);

    DMA_Cmd(W6100_DMA_CHANNEL_RX, ENABLE);
    /* Waiting for the end of Data Transfer */
    while (DMA_GetFlagStatus(DMA_RX_FLAG) == RESET)
        ;


    DMA_ClearFlag(DMA_RX_FLAG);


    DMA_Cmd(W6100_DMA_CHANNEL_RX, DISABLE);

#elif defined USE_HAL_DRIVER


#endif
}
#endif // W6100 ***

inline void wd_eth_w6100_reset_assert(void) {
    wd_gpio_write(RTE_PORT_W6100_RESET, WD_GPIO_ON);
}

inline void wd_eth_w6100_reset_deassert(void) {
    wd_gpio_write(RTE_PORT_W6100_RESET, WD_GPIO_OFF);
}

void wd_eth_reset_w6100(void) {
    wd_eth_w6100_reset_deassert();
    wd_time_wait_ms(5);
    wd_eth_w6100_reset_assert();
    wd_time_wait_ms(5);
    wd_eth_w6100_reset_deassert();
    wd_time_wait_ms(5);
}

#endif //#if 1  /// W6100

/***************************************************************************************************************************************************************
 **********  Driver
 **************************************************************************************************************************************************************/
#undef LOCAL_PREFIX
#define LOCAL_PREFIX_NAME  wd_eth_data
#define LOCAL_PREFIX(name) driver_ethernet_##name

#undef LOCAL_DRV_TYPEDEF
#define LOCAL_DRV_TYPEDEF             wd_eth_t
#define LOCAL_DRV_CASTING(name, port) LOCAL_DRV_TYPEDEF *name = (LOCAL_DRV_TYPEDEF *)((port)->Drv)

#undef LOCAL_CAPABILITIES_TYPEDEF
#define LOCAL_CAPABILITIES_TYPEDEF wd_eth_capabilities_t
#define LOCAL_CAPABILITIES_CASTING(name, port) \
    LOCAL_CAPABILITIES_TYPEDEF *name = (LOCAL_CAPABILITIES_TYPEDEF *)((port)->Capabilities->Private)

#undef LOCAL_INFO_TYPEDEF
#define LOCAL_INFO_TYPEDEF wd_eth_info_t
#define LOCAL_INFO_CASTING(name, port) \
    LOCAL_INFO_TYPEDEF *name = (LOCAL_INFO_TYPEDEF *)((port)->Private)

static char *Socket_Status_Msg_Box(uint8_t status) {
    switch (status) {
    default:
        return ("Socket Unknown");

    /*--------------- Normal status ---------------*/
    case SOCK_CLOSED:
        return ("[SOCK_CLOSED      ]");
    case SOCK_INIT:
        return ("[SOCK_INIT        ]");
    case SOCK_LISTEN:
        return ("[SOCK_LISTEN      ]");
    case SOCK_ESTABLISHED:
        return ("[SOCK_ESTABLISHED ]");
    case SOCK_CLOSE_WAIT:
        return ("[SOCK_CLOSE_WAIT  ]");
    case SOCK_UDP:
        return ("[SOCK_UDP         ]");
    case SOCK_IPRAW:
        return ("[SOCK_IPRAW       ]");
    case SOCK_IPRAW6:
        return ("[SOCK_IPRAW6      ]");
    case SOCK_MACRAW:
        return ("[SOCK_MACRAW      ]");
    /*--------------- Temporary status during changing the status of SOCKETn ---------------*/
    case SOCK_SYNSENT:
        return ("[SOCK_SYNSENT     ]");
    case SOCK_SYNRECV:
        return ("[SOCK_SYNRECV     ]");
    case SOCK_FIN_WAIT:
        return ("[SOCK_FIN_WAIT    ]");
    case SOCK_TIME_WAIT:
        return ("[SOCK_TIME_WAIT   ]");
    case SOCK_LAST_ACK:
        return ("[SOCK_LAST_ACK    ]");
    }
}

static char *Socket_Status_VV_Msg_Box(uint8_t status) {
    switch (status) {
    default:
        return ("None");

    /*--------------- Normal status ---------------*/
    case SOCK_CLOSED:
        return ("Closed");
    case SOCK_INIT:
        return ("Initiate state");
    case SOCK_LISTEN:
        return ("Listen state");
    case SOCK_ESTABLISHED:
        return ("Success to connect");
    case SOCK_CLOSE_WAIT:
        return ("Closing state");
    case SOCK_UDP:
        return ("UDP socket");
    case SOCK_IPRAW:
        return ("IPRAW socket");
    case SOCK_IPRAW6:
        return ("IPv6 IPRAW socket");
    case SOCK_MACRAW:
        return ("MAC raw mode socket");
    /*--------------- Temporary status during changing the status of SOCKETn ---------------*/
    case SOCK_SYNSENT:
        return ("This indicates SOCKETn sent the connect-request packet (SYN packet) to a peer");
    case SOCK_SYNRECV:
        return ("It indicates SOCKETn successfully received the connect-request packet (SYN "
                "packet) from a peer");
    case SOCK_FIN_WAIT:
        return ("Connection state");
    case SOCK_TIME_WAIT:
        return ("Closing state");
    case SOCK_LAST_ACK:
        return ("Closing state");
    }
}

extern char *wd_eth_socket_error_msg_box(int32_t error) {
    switch (error) {
    default:
        return ("None");

    case SOCK_BUSY:
        return ("Socket is busy on processing the operation. Valid only Non-block IO Mode.");
    case SOCK_FATAL:
        return ("Result is fatal error about socket process.");

    case SOCKERR_SOCKNUM:
        return ("Invalid socket number");
    case SOCKERR_SOCKOPT:
        return ("Invalid socket option");
    case SOCKERR_SOCKINIT:
        return ("Socket is not initialized or SIPR is Zero IP address when Sn_MR_TCP");
    case SOCKERR_SOCKCLOSED:
        return ("Socket unexpectedly closed.");
    case SOCKERR_SOCKMODE:
        return ("Invalid socket mode for socket operation.");
    case SOCKERR_SOCKFLAG:
        return ("Invalid socket flag");
    case SOCKERR_SOCKSTATUS:
        return ("Invalid socket status for socket operation.");
    case SOCKERR_ARG:
        return ("Invalid argument.");
    case SOCKERR_PORTZERO:
        return ("Port number is zero");
    case SOCKERR_IPINVALID:
        return ("Invalid source or destination IP address");
    case SOCKERR_TIMEOUT:
        return ("Timeout occurred");
    case SOCKERR_DATALEN:
        return ("Invalid data length");
    case SOCKERR_BUFFER:
        return ("Socket buffer is not enough for data communication");
    }
}

static void print_PHY_Status_Register(uint8_t status) {
    char *msg;

    switch (status & PHYSR_MODE) {
    default:
    case PHYSR_MODE_AUTO:
        msg = "[Auto Negotiation]";
        break;
    case PHYSR_MODE_100F:
        msg = "[100BASE-TX FDX  ]";
        break;
    case PHYSR_MODE_100H:
        msg = "[100BASE-TX HDX  ]";
        break;
    case PHYSR_MODE_10F:
        msg = "[10BASE-T FDX    ]";
        break;
    case PHYSR_MODE_10H:
        msg = "[10BASE-T HDX    ]";
        break;
    }

    if (status & WD_BIT0) {
        WD_PRINT_INFO(WD_MSG_INFO, "[PHY OK ] %s %s %s %s\r\n",
                      (status & WD_BIT1) ? "[10Mbps ]" : "[100Mbps]",
                      (status & WD_BIT2) ? "[Full]" : "[Half]", msg,
                      (status & WD_BIT7) ? "[Cable OK ]" : "[Cable OFF]");
    } else {
        WD_PRINT_INFO(WD_MSG_INFO, "[PHY OFF] %s %s\r\n", msg,
                      (status & WD_BIT7) ? "[Cable OK ]" : "[Cable OFF]");
    }
}

static int32_t Driver_Ethernet_Module_State = 0;
static wiz_NetInfo Driver_Ethernet_Module_Net_Info;

/* packet dump switches (formerly wd_env->Net_Debug, set by the system through wd_eth_set_packet_dump) */
static bool s_packet_dump_app = true;
static bool s_packet_dump_debug = true;
/* W6100 configuration kept across MCU soft reset (formerly s_config_skip) */
static bool s_config_skip = false;

void wd_eth_set_packet_dump(bool app_port, bool debug_port) {
    s_packet_dump_app = app_port;
    s_packet_dump_debug = debug_port;
}

static bool eth_packet_dump_enabled(uint32_t sock_num) {
    bool enabled = false;

    if (sock_num == (uint32_t)RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP) {
        enabled = s_packet_dump_app;
    } else if (sock_num ==
               (uint32_t)RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG) {
        enabled = s_packet_dump_debug;
    } else {
        enabled = false;
    }
    return enabled;
}

/**
 * @brief  Application hook : provide network configuration (IP/MAC/ports, e.g. from NVM).
 *         Default (weak) returns false -> RTE_NETWORK_xxx defaults of _wd_eth.h are used.
 * @return true when net_info / ports were filled by the application
 */
/* parse "a<sep>b<sep>c..." into bytes (MAC : base 16, IPv4 : base 10). Replaces sscanf("%x"/"%u") that
 * wrote 4-byte values into uint8_t / uint16_t fields. Missing fields are left unchanged (sscanf behaviour). */
static void eth_parse_u8_list(const char *text, uint8_t *out, uint32_t count, int base) {
    const char *p = text;

    for (uint32_t i = 0U; i < count; i++) {
        char *end = NULL;
        unsigned long value = strtoul(p, &end, base);

        if (end == p) {
            break;
        }
        out[i] = (uint8_t)value;
        if (*end == '\0') {
            break;
        }
        p = end + 1;
    }
}

static void eth_parse_u16(const char *text, uint16_t *out) {
    char *end = NULL;
    unsigned long value = strtoul(text, &end, 10);

    if (end != text) {
        *out = (uint16_t)value;
    }
}

__WEAK bool wd_eth_user_net_info(wiz_NetInfo *net_info, uint16_t *port_app, uint16_t *port_debug) {
    UNUSED(net_info);
    UNUSED(port_app);
    UNUSED(port_debug);
    return false;
}
wd_eth_socket_t wd_eth_socket[WD_SOCKET_SIZE];

static void print_ipv6_addr(uint8_t *name, uint8_t *ip6addr) {
    WD_PRINT_INFO(WD_MSG_INFO, "%s : ", name);
    WD_PRINT_INFO(WD_MSG_INFO, "%04X:%04X", ((uint16_t)ip6addr[0] << 8) | ((uint16_t)ip6addr[1]),
                  ((uint16_t)ip6addr[2] << 8) | ((uint16_t)ip6addr[3]));
    WD_PRINT_INFO(WD_MSG_INFO, ":%04X:%04X", ((uint16_t)ip6addr[4] << 8) | ((uint16_t)ip6addr[5]),
                  ((uint16_t)ip6addr[6] << 8) | ((uint16_t)ip6addr[7]));
    WD_PRINT_INFO(WD_MSG_INFO, ":%04X:%04X", ((uint16_t)ip6addr[8] << 8) | ((uint16_t)ip6addr[9]),
                  ((uint16_t)ip6addr[10] << 8) | ((uint16_t)ip6addr[11]));
    WD_PRINT_INFO(WD_MSG_INFO, ":%04X:%04X\r\n",
                  ((uint16_t)ip6addr[12] << 8) | ((uint16_t)ip6addr[13]),
                  ((uint16_t)ip6addr[14] << 8) | ((uint16_t)ip6addr[15]));
}

static int32_t print_network_information(wiz_NetInfo *info) {
    int32_t ret = WD_CODE_OK;

    wizchip_getnetinfo(info);

    if (memcmp(&Driver_Ethernet_Module_Net_Info, info, WD_FIELD_OFFSET(wiz_NetInfo, lla, mac))) {
        WD_LOG_ERROR("Net Info Error !!!");
        ret = WD_CODE_ERROR;
    }

    WD_PRINT_INFO(WD_MSG_INFO, "Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n\r", info->mac[0],
                  info->mac[1], info->mac[2], info->mac[3], info->mac[4], info->mac[5]);
    WD_PRINT_INFO(WD_MSG_INFO, "IP address : %d.%d.%d.%d\n\r", info->ip[0], info->ip[1],
                  info->ip[2], info->ip[3]);
    WD_PRINT_INFO(WD_MSG_INFO, "SM Mask    : %d.%d.%d.%d\n\r", info->sn[0], info->sn[1],
                  info->sn[2], info->sn[3]);
    WD_PRINT_INFO(WD_MSG_INFO, "Gate way   : %d.%d.%d.%d\n\r", info->gw[0], info->gw[1],
                  info->gw[2], info->gw[3]);
    WD_PRINT_INFO(WD_MSG_INFO, "DNS Server : %d.%d.%d.%d\n\r", info->dns[0], info->dns[1],
                  info->dns[2], info->dns[3]);

    print_ipv6_addr((uint8_t *)"GW6 ", info->gw6);
    print_ipv6_addr((uint8_t *)"LLA ", info->lla);
    print_ipv6_addr((uint8_t *)"GUA ", info->gua);
    print_ipv6_addr((uint8_t *)"SUB6", info->sn6);

    WD_PRINT_INFO(WD_MSG_INFO, "NETCFGLOCK : %x\r\n", getNETLCKR());

    return (ret);
}

static int32_t Driver_Ethernet_Module_Initialize(uint32_t arg) {
    wiz_NetInfo *net_info = &Driver_Ethernet_Module_Net_Info;

    if ((Driver_Ethernet_Module_State & WD_STATE_INITIALIZED) && (arg == WDIF_ARG_NONE)) {
        return (WD_CODE_OK);
    }

    if (arg != 0U) {
        wd_eth_w6100_initialze();
    }

    {
        unsigned char W6100_AdrSet[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
        if (ctlwizchip(CW_INIT_WIZCHIP, (void *)W6100_AdrSet) == -1) {
            WD_LOG_ERROR("W6100 initialized fail.\r\n");
        }
    }

    {
        uint8_t temp;
        char *msg;

        if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1) {
            WD_LOG_INFO("Unknown PHY link status.\r\n");
        }

        switch (temp) {
        case PHYSR_LNK_UP:
            msg = "[PHY OK ] Ethernet PHY is successfully established.";
            break;
        case PHYSR_LNK_DOWN:
            msg = "[PHY ERR] Ethernet PHY is not established yet.";
            break;
        default:
            msg = "wizphy_getphylink Exception.";
            break;
        }
        WD_LOG_INFO("%s", msg);
    }

    {
        uint8_t syslock = SYS_NET_LOCK;
        ctlwizchip(CW_SYS_UNLOCK, &syslock);
        ctlnetwork(CN_SET_NETINFO, net_info);
    }

    {
        char id[6];
        uint16_t ver;
        ctlwizchip(CW_GET_ID, id);
        ctlwizchip(CW_GET_VER, &ver);
        WD_PRINT_INFO(WD_MSG_INFO, "[ID : %s] [Ver : %d]\r\n", id, ver);
    }

    WD_PRINT_INFO(WD_MSG_INFO, "Register value after W6100 initialize!\r\n");

#if 0
  /* Address Auto Configuration */
  if(1 != AddressAutoConfig_Init(net_info))
  {
    ctlnetwork(CN_SET_NETINFO, net_info);
  }
#endif

    {
        wiz_NetInfo driver_ethernet_module_net_info;

        print_network_information(&driver_ethernet_module_net_info);
    }

    Driver_Ethernet_Module_State = WD_STATE_INITIALIZED;

    return (WD_CODE_OK);
}

static int32_t Driver_Ethernet_Module_Send(uint8_t sock_num, void *data, uint32_t num) {
    int32_t ret;
    datasize_t sentsize = 0;
    uint8_t *buf = (uint8_t *)data;
    uint32_t __time_start = wd_time_get_us();

    if (wd_eth_socket[sock_num].Status != SOCK_ESTABLISHED) {
        WD_LOG_ERROR("Socket is not ESTABLISHED");
        return (WD_CODE_ERROR);
    }

    while ((datasize_t)num != sentsize) {
        if (wd_time_calc_us(&__time_start) > 50000) {
            WD_LOG_ERROR("Socket Send TimeOut");
            return (WD_CODE_ERROR_TIMEOUT);
        }

        ret = send(sock_num, buf + sentsize, (datasize_t)num - sentsize);
        if (ret < 0) {
            close(sock_num);
            WD_LOG_ERROR("Socket Error [%s]", wd_eth_socket_error_msg_box(ret));
            return (WD_CODE_ERROR);
        }
        sentsize = (datasize_t)(sentsize + ret); // Don't care SOCKERR_BUSY, because it is zero.
    }

    return (WD_CODE_OK);
}

static int32_t Driver_Ethernet_Module_Receive(uint8_t sock_num, void *data, uint32_t num) {
    UNUSED(data);
    UNUSED(num);

    if (wd_eth_socket[sock_num].Status != SOCK_ESTABLISHED) {
        WD_LOG_ERROR("Socket is not ESTABLISHED");
        return (WD_CODE_ERROR);
    }

    return (WD_CODE_OK);
}

int32_t wd_eth_get_rx_count(uint8_t sock_num) {
    datasize_t received_size;

    if (wd_eth_socket[sock_num].Status != SOCK_ESTABLISHED) {
        return (0);
    }

    getsockopt(sock_num, SO_RECVBUF, &received_size);

    return (received_size);
}

int32_t wd_eth_tcp_ops(uint8_t sock_num) {
    wd_eth_socket_t *socket_info = &wd_eth_socket[sock_num];

    int32_t ret;
    int8_t inter;
    int8_t tmp = 0;
    datasize_t received_size;
    uint8_t arg_tmp8;

    uint8_t dst_ip[sizeof(wiz_IPAddress)], ext_status;
    uint16_t dst_port;

    static uint8_t PHY_Status_Register = 0;
    static uint8_t PHY_Status_Register_Old = 0;

    PHY_Status_Register = getPHYSR();
    if (PHY_Status_Register_Old != PHY_Status_Register) {
        print_PHY_Status_Register(PHY_Status_Register);
        PHY_Status_Register_Old = PHY_Status_Register;
    }

    getsockopt(sock_num, SO_STATUS, &socket_info->Status);
    if (socket_info->Status_Old != socket_info->Status) {
        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "%s -> %s\r\n",
                      Socket_Status_Msg_Box(socket_info->Status_Old),
                      Socket_Status_Msg_Box(socket_info->Status));
        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "%s -> %s\r\n",
                      Socket_Status_VV_Msg_Box(socket_info->Status_Old),
                      Socket_Status_VV_Msg_Box(socket_info->Status));
        socket_info->Status_Old = socket_info->Status;

        if (socket_info->Status == SOCK_ESTABLISHED) {
            uint8_t keepalive_time;

            socket_info->Time_Stamp = wd_time_get_ms();

            keepalive_time = 0;
            setsockopt(sock_num, SO_KEEPALIVEAUTO, &keepalive_time);
        }
    }

    ctlsocket(sock_num, CS_GET_INTERRUPT, &inter);


    if (inter == (int8_t)0xFF) {
        return (WD_CODE_ERROR);
    }

    if (inter & Sn_IR_CON) {
        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] CONNECT\r\n");
    }
    if (inter & Sn_IR_DISCON) {
        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] DISCONNECT\r\n");
    }
    if (inter & Sn_IR_TIMEOUT) {
        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] TIMEOUT\r\n");
    }
#if _CONF_CONSOLE_LEVEL_LOCAL_VV & WD_BIT0
    if (inter & Sn_IR_RECV) {
        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] RECV\r\n");
    }
    if (inter & Sn_IR_SENDOK) {
        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] SEND_OK\r\n");
    }
#endif

    switch (socket_info->Status) {
    case SOCK_ESTABLISHED:
        if (inter & Sn_IR_CON) {
            getsockopt(sock_num, SO_DESTIP, dst_ip);
            getsockopt(sock_num, SO_EXTSTATUS, &ext_status);
            if (ext_status & TCPSOCK_MODE) {
                //IPv6
                WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[TCP] %d:Peer IP : %04X:%04X",
                              sock_num, ((uint16_t)dst_ip[0] << 8) | ((uint16_t)dst_ip[1]),
                              ((uint16_t)dst_ip[2] << 8) | ((uint16_t)dst_ip[3]));
                WD_PRINT_INFO(WD_MSG_INFO, ":%04X:%04X",
                              ((uint16_t)dst_ip[4] << 8) | ((uint16_t)dst_ip[5]),
                              ((uint16_t)dst_ip[6] << 8) | ((uint16_t)dst_ip[7]));
                WD_PRINT_INFO(WD_MSG_INFO, ":%04X:%04X",
                              ((uint16_t)dst_ip[8] << 8) | ((uint16_t)dst_ip[9]),
                              ((uint16_t)dst_ip[10] << 8) | ((uint16_t)dst_ip[11]));
                WD_PRINT_INFO(WD_MSG_INFO, ":%04X:%04X, ",
                              ((uint16_t)dst_ip[12] << 8) | ((uint16_t)dst_ip[13]),
                              ((uint16_t)dst_ip[14] << 8) | ((uint16_t)dst_ip[15]));
            } else {
                //IPv4
                //getSn_DIPR(sock_num,dst_ip);
                WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO,
                              "[TCP] %d:Peer IP : %3d.%3d.%3d.%3d, ", sock_num, dst_ip[0],
                              dst_ip[1], dst_ip[2], dst_ip[3]);
            }
            getsockopt(sock_num, SO_DESTPORT, &dst_port);
            WD_PRINT_INFO(WD_MSG_INFO, "Peer Port : %d\r\n", dst_port);

            socket_info->Time_Stamp = wd_time_get_ms();

            {
                uint8_t keepalive_time;
                keepalive_time = 0;
                setsockopt(sock_num, SO_KEEPALIVEAUTO, &keepalive_time);
            }

            arg_tmp8 = Sn_IR_CON;
            ctlsocket(sock_num, CS_CLR_INTERRUPT, &arg_tmp8);
        }

        if (wd_time_calc_ms(&socket_info->Time_Stamp) >= 3000) {
            uint8_t keepalive_time;
            getsockopt(sock_num, SO_KEEPALIVEAUTO, &keepalive_time);

            if (keepalive_time != 0) {
                WD_LOG_ERROR("KeepAlive Config Error");
                keepalive_time = 0;
                setsockopt(sock_num, SO_KEEPALIVEAUTO, &keepalive_time);
            }

#pragma GCC diagnostic push /* DEV-WD-007 : int arithmetic inside the WIZnet register macro */
#pragma GCC diagnostic ignored "-Wsign-conversion"
            setSn_CR(sock_num, Sn_CR_SEND_KEEP);
#pragma GCC diagnostic pop

            if (eth_packet_dump_enabled((uint32_t)sock_num) == true) {
                WD_PRINT(WD_MSG_EVENT_LOG, "Send KeepAlive Packet [%u]", socket_info->Port);
            }

            socket_info->Time_Stamp = wd_time_get_ms();
        }
        break;

    case SOCK_CLOSE_WAIT:
        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "%d:CloseWait\r\n", sock_num);

        getsockopt(sock_num, SO_RECVBUF, &received_size);
        if (received_size > 0) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
        {
        }

        if ((ret = disconnect(sock_num)) != SOCK_OK) {
            WD_LOG_ERROR("Socket Error [%s]", wd_eth_socket_error_msg_box(ret));
            return ret;
        }

        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "%d:Socket Closed\r\n", sock_num);
        break;

    case SOCK_INIT:
        if ((ret = listen(sock_num)) != SOCK_OK) {
            WD_LOG_ERROR("Socket Error [%s]", wd_eth_socket_error_msg_box(ret));
            return ret;
        }
        break;

    case SOCK_CLOSED:
        tmp = socket(sock_num, Sn_MR_TCP4, socket_info->Port, SOCK_IO_NONBLOCK);
        if (tmp != sock_num) /* reinitialize the socket */
        {
            WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "%d : Fail to create socket.\r\n",
                          sock_num);
            WD_LOG_ERROR();
            Driver_Ethernet_Module_State = 0;
            wd_eth_driver_ethernet_module.Initialize(WDIF_ARG_NONE);
            return SOCKERR_SOCKNUM;
        }
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
        WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "%d:Socket opened[%d]\r\n", sock_num,
                      getSn_SR(sock_num));
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
        socket_info->State = 1;
        break;

    default:
        break;
    }

    return (WD_CODE_OK);
}

#if 0
static int32_t Driver_Ethernet_Module_UDP_Ops (uint8_t sock_num)
{
  wd_eth_socket_t *socket_info = &wd_eth_socket[sock_num];

  int8_t inter;
  int8_t tmp = 0;

  static uint8_t PHY_Status_Register = 0;
  static uint8_t PHY_Status_Register_Old = 0;

  PHY_Status_Register = getPHYSR();
  if(PHY_Status_Register_Old != PHY_Status_Register)
  {
    print_PHY_Status_Register(PHY_Status_Register);
    PHY_Status_Register_Old = PHY_Status_Register;
  }

  getsockopt(sock_num, SO_STATUS, &socket_info->Status);
  if(socket_info->Status_Old != socket_info->Status)
  {
    WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "%s -> %s\r\n", Socket_Status_Msg_Box(socket_info->Status_Old), Socket_Status_Msg_Box(socket_info->Status));
    WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "%s -> %s\r\n", Socket_Status_VV_Msg_Box(socket_info->Status_Old), Socket_Status_VV_Msg_Box(socket_info->Status));
    socket_info->Status_Old = socket_info->Status;
  }

  ctlsocket(sock_num,CS_GET_INTERRUPT,&inter);


  if(inter == (int8_t)0xFF)
  {
    return (WD_CODE_ERROR);
  }

  if(inter & Sn_IR_CON)
  {
    WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] CONNECT\r\n");
  }
  if(inter & Sn_IR_DISCON)
  {
    WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] DISCONNECT\r\n");
  }
  if(inter & Sn_IR_TIMEOUT)
  {
    WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] TIMEOUT\r\n");
  }
#if _CONF_CONSOLE_LEVEL_LOCAL_VV > 0
  if(inter & Sn_IR_RECV)
  {
    WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] RECV\r\n");
  }
  if(inter & Sn_IR_SENDOK)
  {
    WD_PRINT_INFO(WD_MSG_TIME_STAMP | WD_MSG_INFO, "[Socket Event] SEND_OK\r\n");
  }
#endif

  switch(socket_info->Status)
  {
    case SOCK_UDP :
      {
        datasize_t received_size;
        datasize_t rx_cnt;
        uint8_t  dst_ip[sizeof(wiz_IPAddress)];
        uint8_t  dst_addr_len;
        uint16_t dst_port;

        getsockopt(sock_num, SO_RECVBUF, &received_size);
        if(received_size > 0)
        {
          rx_cnt = recvfrom(sock_num, wd_eth_socket[sock_num].Buf, received_size, dst_ip, &dst_port, &dst_addr_len);

          if(rx_cnt < 0)
          {
            WD_LOG_ERROR();
          }
          else
          {
          }
        }
      }
      break;

    case SOCK_CLOSED:
      tmp = socket(sock_num, Sn_MR_UDP4, socket_info->Port, SOCK_IO_NONBLOCK);
      if(tmp != sock_num)    /* reinitialize the socket */
      {
        WD_LOG_ERROR();
        return SOCKERR_SOCKNUM;
      }
      break;

    default:
      break;
  }

  return (WD_CODE_OK);
}
#endif

static int32_t Driver_Ethernet_Module_Ops(uint8_t sock_num, uint32_t ipproto) {
    switch (ipproto) {
    case IPPROTO_TCP:
        if (wd_eth_tcp_ops(sock_num) != WD_CODE_OK) {
            return (WD_CODE_ERROR);
        }
        break;

#if 0
    case IPPROTO_UDP :
      if(Driver_Ethernet_Module_UDP_Ops(sock_num) != WD_CODE_OK)
      {
        return(WD_CODE_ERROR);
      }
      break;
#endif

    default:
        WD_LOG_ERROR("IP Protocol Not Supported");
        return (WD_CODE_ERROR);
    }

    return (WD_CODE_OK);
}

wd_eth_t wd_eth_driver_ethernet_module = {.Initialize = Driver_Ethernet_Module_Initialize,
                                          .Send = Driver_Ethernet_Module_Send,
                                          .Receive = Driver_Ethernet_Module_Receive,
                                          .GetRxCount = wd_eth_get_rx_count,
                                          .Ops = Driver_Ethernet_Module_Ops,
                                          .State = &Driver_Ethernet_Module_State,
                                          .Net_Info = &Driver_Ethernet_Module_Net_Info};


/***************************************************************************************************************************************************************
 **********  Driver				// _Driver_Ethernet.c[L496]
 **************************************************************************************************************************************************************/
static int32_t LOCAL_PREFIX(Register)(wd_network_port_t *port) {
    LOCAL_CAPABILITIES_CASTING(caps, port);

    port->Info->Xfer.Tx_Buf = port->Tx_Buf;
    port->Info->Xfer.Tx_Buf_Size = port->Tx_Buf_Size;
    port->Info->Xfer.Rx_Buf = port->Rx_Buf;
    port->Info->Xfer.Rx_Buf_Size = port->Rx_Buf_Size;
    port->Info->Wait_Timer = 0;
    port->Info->Time_Stamp = 0;
    port->Info->Tx_Timeout = 0;
    port->Info->Rx_Timeout = 0;

    port->Info->Msg_Queue =
        osMessageQueueNew(port->Msg_Queue_Size, sizeof(wd_message_t), port->Msg_Queue_Attr);
    osMessageQueueReset(port->Info->Msg_Queue);

    if (!Driver_Ethernet_Module_State) {
        // SPI6은 CubeMX MX_SPI6_Init()에서 이미 Master/8bit/CPOL0-CPHA0/MSB/NSS-Soft로 초기화됨
        // (기존 CMSIS-Driver Initialize+PowerControl+Control과 동일한 설정) — 재확인 차 재적용
        // 기존 CMSIS-Driver PowerControl(ARM_POWER_FULL)이 하던 SPI6 레지스터 하드 리셋과 동일하게 재현
        __HAL_RCC_SPI6_FORCE_RESET();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __HAL_RCC_SPI6_RELEASE_RESET();
        // 기존 CMSIS-Driver Control()이 Master 모드에서 항상 MasterKeepIOState=ENABLE로 강제 설정했음
        // (CubeMX MX_SPI6_Init()은 DISABLE로 생성 — W6100 통신 안정성을 위해 기존 동작 그대로 유지)
        SPI_HANDLE->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;

        // CubeMX 정적 설정(PRESCALER_2, 즉 PCLK4/2)이 아니라, 기존 CMSIS-Driver Control()이
        // RTE_COMPONENT_INTERFACE_ETHERNET_BAUDRATE(10MHz)를 넘지 않는 가장 빠른 분주비를
        // 실제 SPI6 커널클럭 기준으로 동적 계산해서 썼던 것과 동일하게 재현 (그렇지 않으면 W6100 통신 불가)
        {
            uint32_t __eth_spi_pclk = HAL_RCCEx_GetD3PCLK1Freq();
            uint32_t __eth_spi_val;

            for (__eth_spi_val = 0U; __eth_spi_val < 7U; __eth_spi_val++) {
                if (RTE_COMPONENT_INTERFACE_ETHERNET_BAUDRATE >=
                    (__eth_spi_pclk >> (__eth_spi_val + 1U))) {
                    break;
                }
            }

            switch (__eth_spi_val) {
            case 0:
                SPI_HANDLE->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
                break;
            case 1:
                SPI_HANDLE->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
                break;
            case 2:
                SPI_HANDLE->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
                break;
            case 3:
                SPI_HANDLE->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
                break;
            case 4:
                SPI_HANDLE->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
                break;
            case 5:
                SPI_HANDLE->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
                break;
            case 6:
                SPI_HANDLE->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
                break;
            default:
                SPI_HANDLE->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
                break;
            }
        }
        HAL_SPI_Init(SPI_HANDLE);
        wd_gpio_config(RTE_PORT_W6100_RESET, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(RTE_PORT_W6100_CS, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);

        wd_eth_reset_w6100();
        wd_time_wait_ms(500);

        if (caps->Extend != NULL) {
        }

        {
            wiz_NetInfo *net_info = &Driver_Ethernet_Module_Net_Info;

            if (!wd_eth_user_net_info(
                    net_info,
                    &wd_eth_socket[RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP]
                         .Port,
                    &wd_eth_socket[RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG]
                         .Port)) {
                eth_parse_u8_list(RTE_NETWORK_MAC, net_info->mac, 6U, 16);
                eth_parse_u8_list(RTE_NETWORK_IP, net_info->ip, 4U, 10);
                eth_parse_u8_list(RTE_NETWORK_SUB_NET_MASK, net_info->sn, 4U, 10);
                eth_parse_u8_list(RTE_NETWORK_GATEWAY, net_info->gw, 4U, 10);
                eth_parse_u8_list(RTE_NETWORK_DNS, net_info->dns, 4U, 10);

                eth_parse_u16(RTE_NETWORK_PORT, &wd_eth_socket[WD_SOCKET_ETHERNET].Port);
                eth_parse_u16(RTE_NETWORK_DEBUG_PORT, &wd_eth_socket[WD_SOCKET_DEBUG].Port);
            }
            eth_parse_u16(RTE_TELNET_PORT, &wd_eth_socket[WD_SOCKET_CONSOLE].Port);
        }

#if (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_BOOTLOADER)
        {
#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
            /* SPI method callback registration */
            //      WD_LOG_INFO("SPI DMA setting \r\n");
            //      reg_wizchip_spi_cbfunc(wd_eth_w6100_spi_read_byte, wd_eth_w6100_spi_write_byte, wd_eth_w6100_spi_read_burst, wd_eth_w6100_spi_write_burst);
            reg_wizchip_spi_cbfunc(wd_eth_w6100_spi_read_byte, wd_eth_w6100_spi_write_byte, 0, 0);
            /* CS function register */
            reg_wizchip_cs_cbfunc(wd_eth_enable_w6100_cs, wd_eth_disable_w6100_cs);
#else
            /* Indirect bus method callback registration */
#if defined WD_BUS_DMA
            //      WD_LOG_INFO("BUS DMA setting \r\n");
            reg_wizchip_bus_cbfunc(W6100BusDmaReadByte, W6100BusDmaWriteByte, 0, 0);
#else
            WD_LOG_INFO("no BUS DMA setting \r\n");
            reg_wizchip_bus_cbfunc(W6100BusReadByte, W6100BusWriteByte, 0, 0);
#endif
#endif

            if ((WIZCHIP_READ(_CIDR_) != 0x61) || (WIZCHIP_READ(_VER_) != 0x46)) {
                WD_LOG_ERROR("W6100 SPI Error");
            }

            s_config_skip = true;
        }
#elif (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
#if ( \
    RTE_COMPONENT_INTERFACE_ETHERNET_KEEP_CONFIG) /* keep W6100 configuration across MCU soft reset */
        {
            wiz_NetInfo driver_ethernet_module_net_info;

#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
            /* SPI method callback registration */
            //      WD_LOG_INFO("SPI DMA setting \r\n");
            //      reg_wizchip_spi_cbfunc(wd_eth_w6100_spi_read_byte, wd_eth_w6100_spi_write_byte, wd_eth_w6100_spi_read_burst, wd_eth_w6100_spi_write_burst);
            reg_wizchip_spi_cbfunc(wd_eth_w6100_spi_read_byte, wd_eth_w6100_spi_write_byte, 0, 0);
            /* CS function register */
            reg_wizchip_cs_cbfunc(wd_eth_enable_w6100_cs, wd_eth_disable_w6100_cs);
#else
            /* Indirect bus method callback registration */
#if defined WD_BUS_DMA
            //      WD_LOG_INFO("BUS DMA setting \r\n");
            reg_wizchip_bus_cbfunc(W6100BusDmaReadByte, W6100BusDmaWriteByte, 0, 0);
#else
            WD_LOG_INFO("no BUS DMA setting \r\n");
            reg_wizchip_bus_cbfunc(W6100BusReadByte, W6100BusWriteByte, 0, 0);
#endif
#endif

            if ((WIZCHIP_READ(_CIDR_) != 0x61) || (WIZCHIP_READ(_VER_) != 0x46)) {
                WD_LOG_ERROR("W6100 SPI Error");
            }

            if (!s_config_skip) {
                if (print_network_information(&driver_ethernet_module_net_info) != WD_CODE_OK) {
                    wd_eth_driver_ethernet_module.Initialize(WDIF_ARG_RESET);
                } else {
                    s_config_skip = true;
                    WD_LOG_NOTICE("Ethernet Already Configured !!!");
                }
            }
        }

#else
        wd_eth_driver_ethernet_module.Initialize(WDIF_ARG_RESET);
#endif
#endif // (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
    }

    port->Info->Status.Initialized = true;

    return (WD_CODE_OK);
}

static int32_t LOCAL_PREFIX(Unregister)(wd_network_port_t *port) {
    UNUSED(port);

    return (WD_CODE_OK);
}

static void LOCAL_PREFIX(Tx_Ops)(wd_network_port_t *port) {
    LOCAL_DRV_CASTING(drv, port);
    LOCAL_CAPABILITIES_CASTING(caps, port);
    LOCAL_INFO_CASTING(info, port);

    UNUSED(info);

    if ((!port->Info->Status.Tx_Busy) && (!port->Info->Status.Rx_Busy) &&
        (port->Info->Wait_Timer >= port->Capabilities->Tx_Wait_Timer)) {
        wd_message_t msg;
        osStatus_t status = osMessageQueueGet(port->Info->Msg_Queue, &msg, NULL, 0);

        if (status == osOK) {
            if ((msg.Size == 0) || (msg.Size > 32768)) {
                WD_LOG_ERROR("Message Size Invalid [%d]", msg.Size);
                osMessageQueueReset(port->Info->Msg_Queue);
            } else if (wd_eth_socket[caps->Sock_Descriptor].Status == SOCK_ESTABLISHED) {
                port->Info->Status.Tx_Busy = true;

                if (msg.Ptr != port->Info->Xfer.Tx_Buf) {
                    memcpy(port->Info->Xfer.Tx_Buf, msg.Ptr, msg.Size);
                }

                port->Info->Xfer.Tx_Num = msg.Size;
#if _CONF_CONSOLE_LEVEL_LOCAL_VV & (WD_BIT1 | WD_BIT2)
#if ((_CONF_CONSOLE_LEVEL_LOCAL_VV & WD_BIT1) != WD_BIT1)
                if (caps->Sock_Descriptor ==
                    RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP) {
                } else
#endif
#if ((_CONF_CONSOLE_LEVEL_LOCAL_VV & WD_BIT2) != WD_BIT2)
                    if (caps->Sock_Descriptor ==
                        RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG) {
                } else
#endif
#endif

                    if (eth_packet_dump_enabled(caps->Sock_Descriptor) == true) {
                    WD_PRINT(WD_MSG_FRAME_DUMP_LOG, "%s", port->Capabilities->Info, WD_MSG_MAGIC,
                             WD_NET_TX, port->Info->Xfer.Tx_Buf, msg.Size);
                }

                if (caps->Extend != NULL) {
                }

                if (drv->Send(caps->Sock_Descriptor, port->Info->Xfer.Tx_Buf,
                              port->Info->Xfer.Tx_Num) < 0) {
                    WD_LOG_ERROR();
                }

                port->Info->Event |= WD_USART_EVENT_TX_COMPLETE;
                //        port->Info->Event |= ARM_SPI_EVENT_TRANSFER_COMPLETE;

                port->Info->Status.Indicator_Tx = true;

                port->Info->Tx_Timeout = 0;
                if (caps->Extend != NULL) {
                }
            }
        }
    }
}

static void LOCAL_PREFIX(Tx_Complete)(wd_network_port_t *port) {
    if (port->Info->Status.Tx_Busy) {
        if (port->Info->Xfer.Rx_Offset) {
            port->Info->Rx_Timeout = 0;
        }
        port->Info->Status.Tx_Busy = false;
    } else {
        WD_PRINT_DEBUG(WD_MSG_NORMAL, "Driver_Ethernet Status.Tx_Busy Exception !!!");
    }

    port->Info->Wait_Timer = 0;
}

static int32_t LOCAL_PREFIX(Rx_Ops)(wd_network_port_t *port) {
    LOCAL_DRV_CASTING(drv, port);
    LOCAL_CAPABILITIES_CASTING(caps, port);

    int32_t ret = WD_CODE_NOP;
    uint16_t rx_cnt;
    uint32_t rx_flush = false;

    UNUSED(caps);

    // ----- Receiving ----- //
    rx_cnt = (uint16_t)drv->GetRxCount(caps->Sock_Descriptor);

    if (rx_cnt) {
        if (port->Info->Xfer.Rx_Cnt + port->Info->Xfer.Rx_Offset + rx_cnt >
            port->Info->Xfer.Rx_Buf_Size) {
            rx_cnt = (uint16_t)(port->Info->Xfer.Rx_Buf_Size - port->Info->Xfer.Rx_Cnt -
                                port->Info->Xfer.Rx_Offset);
        }

        if (rx_cnt != 0) {
            if (recv(caps->Sock_Descriptor,
                     port->Info->Xfer.Rx_Buf + port->Info->Xfer.Rx_Cnt + port->Info->Xfer.Rx_Offset,
                     (datasize_t)rx_cnt) <= 0) {
                WD_LOG_ERROR("Socket recv Error !!!");
            }

            port->Info->Xfer.Rx_Cnt += rx_cnt;
            port->Info->Rx_Timeout = 0;
        }

        port->Info->Status.Indicator_Rx = true;
        port->Info->Status.Rx_Complete = true;
        ret = WD_CODE_PROCESSED;
        port->Info->Event |= WD_USART_EVENT_RECEIVE_COMPLETE;
        //    port->Info->Event |= ARM_SPI_EVENT_TRANSFER_COMPLETE;
    } else if (port->Info->Xfer.Rx_Offset) {
        port->Info->Status.Indicator_Rx = true;
        port->Info->Status.Rx_Complete = true;
        ret = WD_CODE_PROCESSED;
        port->Info->Event |= WD_USART_EVENT_RECEIVE_COMPLETE;
        //    port->Info->Event |= ARM_SPI_EVENT_TRANSFER_COMPLETE;
    }

    if (port->Info->Xfer.Rx_Offset && !port->Info->Status.Tx_Busy) {
        if (port->Info->Rx_Timeout > 50) {
            rx_flush = true;
            WD_LOG_ERROR("Ethernet Rx Continuous Error");
        }
    }

    if (port->Info->Xfer.Rx_Cnt) {
        if (port->Info->Rx_Timeout > port->Capabilities->Rx_Frame_Timeout) {
            rx_flush = true;
        }
    }

    if (rx_flush) {
        port->Info->Rx_Timeout = 0;
        port->Info->Status.Rx_Busy = false;
        port->Info->Status.Rx_Complete = false;
        port->Info->Xfer.Rx_Cnt = 0;
        port->Info->Xfer.Rx_Offset = 0;
        port->Info->Xfer.Rx_Num = port->Rx_Buf_Size;
        memset(port->Info->Xfer.Rx_Buf, 0, port->Info->Xfer.Rx_Num);
        drv->Receive(caps->Sock_Descriptor, port->Info->Xfer.Rx_Buf, port->Info->Xfer.Rx_Buf_Size);
        //    port->Info->Status.Rx_Timeout = true;
    }

    return (ret);
}

static int32_t LOCAL_PREFIX(Rx_Complete)(wd_network_port_t *port) {
    LOCAL_CAPABILITIES_CASTING(caps, port);
    int32_t idx = 0;

    {
        if (port->Info->Xfer.Rx_Cnt == 0) {
            __NOP();
        }

        {
#if _CONF_CONSOLE_LEVEL_LOCAL_VV & (WD_BIT1 | WD_BIT2)
#if ((_CONF_CONSOLE_LEVEL_LOCAL_VV & WD_BIT1) != WD_BIT1)
            if (caps->Sock_Descriptor ==
                RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP) {
            } else
#endif
#if ((_CONF_CONSOLE_LEVEL_LOCAL_VV & WD_BIT2) != WD_BIT2)
                if (caps->Sock_Descriptor ==
                    RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG) {
            } else
#endif
#endif
                if (eth_packet_dump_enabled(caps->Sock_Descriptor) == true) {
                WD_PRINT(WD_MSG_FRAME_DUMP_LOG, "%s", port->Capabilities->Info, WD_MSG_MAGIC,
                         WD_NET_RX, port->Info->Xfer.Rx_Buf,
                         port->Info->Xfer.Rx_Cnt + port->Info->Xfer.Rx_Offset);
            }
        }

        idx = (port->Parser != NULL)
                  ? port->Parser(port, port->Info->Xfer.Rx_Buf,
                                 port->Info->Xfer.Rx_Buf + port->Info->Xfer.Rx_Cnt +
                                     port->Info->Xfer.Rx_Offset,
                                 0)
                  : 0; /* parser of the owning service */
    }

    if (idx <= 0) {
        __NOP();
    }

    return (idx);
}

static void LOCAL_PREFIX(Rx_Flush)(wd_network_port_t *port, uint16_t parsed_data_idx) {
    LOCAL_DRV_CASTING(drv, port);
    LOCAL_CAPABILITIES_CASTING(caps, port);

    if (port->Info->Xfer.Rx_Cnt + port->Info->Xfer.Rx_Offset > parsed_data_idx) {
        port->Info->Rx_Timeout = 0;
        port->Info->Status.Indicator_Rx = true;
        port->Info->Status.Rx_Complete = true;
        memmove(port->Info->Xfer.Rx_Buf, port->Info->Xfer.Rx_Buf + parsed_data_idx,
                (size_t)(port->Info->Xfer.Rx_Cnt + port->Info->Xfer.Rx_Offset - parsed_data_idx));
        port->Info->Xfer.Rx_Offset =
            (uint16_t)(port->Info->Xfer.Rx_Cnt + port->Info->Xfer.Rx_Offset - parsed_data_idx);
        port->Info->Xfer.Rx_Cnt = 0;
        WD_LOG_INFO("Ethernet Rx Continuous Packet [%d]", port->Info->Xfer.Rx_Offset);
    } else {
        port->Info->Status.Rx_Busy = false;
        port->Info->Status.Rx_Complete = false;
        port->Info->Xfer.Rx_Num = port->Rx_Buf_Size;
        memset(port->Info->Xfer.Rx_Buf, 0, port->Info->Xfer.Rx_Num);
        port->Info->Xfer.Rx_Offset = 0;
        port->Info->Xfer.Rx_Cnt = 0;
        drv->Receive(caps->Sock_Descriptor, port->Info->Xfer.Rx_Buf, port->Info->Xfer.Rx_Buf_Size);
    }
}

static int32_t LOCAL_PREFIX(Timeout)(wd_network_port_t *port, uint32_t time) {
    LOCAL_CAPABILITIES_CASTING(caps, port);
    LOCAL_INFO_CASTING(info, port);

    UNUSED(info);

    port->Info->Wait_Timer += time;
    port->Info->Tx_Timeout += time;
    port->Info->Error_Timeout += time;

    if (port->Info->Status.Tx_Busy) {
        if (port->Info->Tx_Timeout > 2000) {
            __NOP();
            WD_LOG_ERROR("Serial Tx Busy Exception");
            port->Info->Status.Tx_Busy = false;
        }

        port->Info->Rx_Timeout = 0;
        port->Info->Status.Indicator_Tx = true;

    } else {
        port->Info->Rx_Timeout += time;
    }

    if (caps->Extend != NULL) {
    }
    return (WD_CODE_NOP);
}

static void LOCAL_PREFIX(Error)(wd_network_port_t *port) {
    LOCAL_DRV_CASTING(drv, port);
    LOCAL_CAPABILITIES_CASTING(caps, port);

    UNUSED(drv);
    UNUSED(caps);

    //  if(0)
    //  {
    //    osMessageQueueReset(port->Info->Msg_Queue);

    //    port->Info->Xfer.Rx_Cnt = 0;
    //    memset(port->Info->Xfer.Rx_Buf, 0, port->Info->Xfer.Rx_Buf_Size);
    //    drv->Receive(WD_SOCKET_ETHERNET, port->Info->Xfer.Rx_Buf, port->Info->Xfer.Rx_Buf_Size);

    //    WD_PRINT_ERROR(WD_MSG_ERROR_LOG, "Driver_Ethernet_Error [%s] : ", port->Capabilities->Info);
    //  }
}

static int32_t LOCAL_PREFIX(Check)(wd_network_port_t *port) {
    if ((port->Capabilities == NULL) || (port->Tx_Buf == NULL) || (port->Tx_Buf_Size == 0) ||
        (port->Rx_Buf == NULL) || (port->Rx_Buf_Size == 0) || (port->Info->Msg_Queue == NULL)) {
        WD_PRINT_ERROR(WD_MSG_ERROR_DEBUG_LOG, "Driver_Ethernet_Register_Port %s",
                       port->Capabilities->Info);
        return (WD_CODE_ERROR);
    }

    return (WD_CODE_OK);
}

static void LOCAL_PREFIX(Process)(wd_network_port_t *port, uint32_t arg) {
    LOCAL_CAPABILITIES_CASTING(caps, port);
    LOCAL_INFO_CASTING(info, port);

    uint32_t time = 0;

    UNUSED(arg);
    UNUSED(info);

    if (caps->IP_Protocol == IPPROTO_TCP) {
        wd_eth_tcp_ops(caps->Sock_Descriptor);
    }
#if 0
  else if(caps->IP_Protocol == IPPROTO_UDP)
  {
    Driver_Ethernet_Module_UDP_Ops(caps->Sock_Descriptor);
  }
#endif
    else {
        WD_LOG_ERROR();
    } //  if(caps->IP_Protocol == IPPROTO_TCP)

    {
        time = wd_time_calc_ms(&port->Info->Time_Stamp);
        port->Info->Time_Stamp = wd_time_calc_ms(NULL);

        wd_eth_data.Timeout(port, time);

        if (port->Info->Event & WD_USART_EVENT_TX_COMPLETE) {
            if ((port->Info->Tx_Timeout * 1000 >= caps->Extend->Octet_Timeout) ||
                (caps->Extend == NULL)) {
                wd_eth_data.Tx_Complete(port);
                port->Info->Event &= ~WD_USART_EVENT_TX_COMPLETE;
            }
        }
        wd_eth_data.Tx_Ops(port);

        if (wd_eth_data.Rx_Ops(port) == WD_CODE_PROCESSING) {
        }

        if (((port->Info->Event & WD_USART_EVENT_RECEIVE_COMPLETE) &&
             (!port->Info->Status.Tx_Busy)) ||
            (port->Info->Event & WD_USART_EVENT_RX_OVERFLOW))
        //	if(((port->Info->Event & ARM_SPI_EVENT_TRANSFER_COMPLETE) && (!port->Info->Status.Tx_Busy)) || (port->Info->Event & ARM_SPI_EVENT_MODE_FAULT))
        {
            uint16_t parsed_data_idx = 0;
            char last_rx_data;

            UNUSED(last_rx_data);

            parsed_data_idx = (uint16_t)wd_eth_data.Rx_Complete(port);

            if (parsed_data_idx) {
                last_rx_data = port->Info->Xfer.Rx_Buf[parsed_data_idx - 1];
                wd_eth_data.Rx_Flush(port, parsed_data_idx);
            } else if (port->Info->Xfer.Rx_Cnt) {
                last_rx_data = port->Info->Xfer.Rx_Buf[port->Info->Xfer.Rx_Cnt - 1];
            }

            port->Info->Event &= ~WD_USART_EVENT_RECEIVE_COMPLETE;
            //      port->Info->Event &= ~ARM_SPI_EVENT_DATA_LOST;
        }
    }

    wd_eth_data.Error(port);

    if (port->Info->Status.Indicator_Tx) {
        port->Info->Status.Indicator_Tx = false;
    }
    if (port->Info->Status.Indicator_Rx) {
        port->Info->Status.Indicator_Rx = false;
    }
    if (port->Info->Status.Indicator_Error) {
        port->Info->Status.Indicator_Error = false;
    }
}

static void *LOCAL_PREFIX(Get_Priv)(wd_network_port_t *port) {
    return (port->Private);
}

wd_network_driver_t LOCAL_PREFIX_NAME = {
    .Register = LOCAL_PREFIX(Register),
    .Unregister = LOCAL_PREFIX(Unregister),
    .Tx_Ops = LOCAL_PREFIX(Tx_Ops),
    .Tx_Complete = LOCAL_PREFIX(Tx_Complete),
    .Rx_Ops = LOCAL_PREFIX(Rx_Ops),
    .Rx_Complete = LOCAL_PREFIX(Rx_Complete),
    .Rx_Flush = LOCAL_PREFIX(Rx_Flush),
    .Timeout = LOCAL_PREFIX(Timeout),
    .Error = LOCAL_PREFIX(Error),
    .Check = LOCAL_PREFIX(Check),
    .Process = LOCAL_PREFIX(Process),
    .Get_Priv = LOCAL_PREFIX(Get_Priv),
};


/***************************************************************************************************************************************************************
 **********  Terminal
 **************************************************************************************************************************************************************/
#if (RTE_PLATFORM_PRINT_TELNET) && (RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG == 2)
void Driver_Ethernet_Terminal_Write(const void *data, uint32_t num, uint32_t arg) {
    int32_t ret;
    datasize_t sentsize = 0;
    uint8_t *buf = (uint8_t *)((uint32_t)data);
    uint32_t __time_start = wd_time_get_us();

    UNUSED(arg);

    if (wd_eth_socket[RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG].Status !=
        SOCK_ESTABLISHED) {
        WD_LOG_ERROR("Socket is not ESTABLISHED");
        return;
    }

    while ((datasize_t)num != sentsize) {
        if (wd_time_calc_us(&__time_start) > 1000000) {
            WD_LOG_ERROR("Socket Send TimeOut");
            return;
        }

        ret = send(RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG, buf + sentsize,
                   (datasize_t)num - sentsize);
        if (ret < 0) {
            close(RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG);
            WD_LOG_ERROR("Socket Error [%s]", wd_eth_socket_error_msg_box(ret));
            return;
        }
        sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
    }
}

int32_t Driver_Ethernet_Terminal_Ops(uint8_t sock_num) {
    if (wd_eth_tcp_ops(sock_num) != WD_CODE_OK) {
        return (WD_CODE_ERROR);
    }

    if (wd_eth_socket[sock_num].Status == SOCK_ESTABLISHED) {
        uint16_t rx_cnt;

        rx_cnt = (uint16_t)wd_eth_get_rx_count(sock_num);

        if (rx_cnt > 0) {
            if (recv(sock_num, Driver_Ethernet_Socket_Buf, (datasize_t)rx_cnt) <= 0) {
                WD_LOG_ERROR();
            } else {
                wd_cbuf_push(&Terminal_Rx_C_Buf, Driver_Ethernet_Socket_Buf, rx_cnt);
            }
        }

        {
            datasize_t tx_free_size;

            getsockopt(sock_num, SO_SENDBUF, &tx_free_size);

            if ((tx_free_size >= 2048) && Terminal_Tx_C_Buf.Size) {
                if ((wd_time_calc_us(&Terminal_Tx_Time_Stamp) > 50000) ||
                    (Terminal_State & WD_TERMINAL_STATE_FLUSH) || (Terminal_Tx_C_Buf.Size > 4096)) {
                    WDIF_Flush_Terminal(NULL);
                }
            }
        }
    }

    return (WD_CODE_OK);
}
#endif


#if (RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG == 2) || (RTE_COMPONENT_INTERFACE_ETHERNET_TELNET)
static uint8_t Driver_Ethernet_Socket_Buf[2048];
#endif
/***************************************************************************************************************************************************************
 **********  Telnet
 **************************************************************************************************************************************************************/
#if (RTE_COMPONENT_INTERFACE_ETHERNET_TELNET)
void wd_eth_write_telnet(const void *data, uint32_t num, uint32_t arg) {
    int32_t ret;
    datasize_t sentsize = 0;
    uint8_t *buf = (uint8_t *)((uint32_t)data);
    uint32_t __time_start = wd_time_get_us();

    UNUSED(arg);

    if (wd_eth_socket[RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE].Status !=
        SOCK_ESTABLISHED) {
        WD_LOG_ERROR("Socket is not ESTABLISHED");
        return;
    }

    while ((datasize_t)num != sentsize) {
        if (wd_time_calc_us(&__time_start) > 1000000) {
            WD_LOG_ERROR("Socket Send TimeOut");
            return;
        }

        ret = send(RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE, buf + sentsize,
                   (datasize_t)num - sentsize);
        if (ret < 0) {
            close(RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE);
            WD_LOG_ERROR("Socket Error [%s]", wd_eth_socket_error_msg_box(ret));
            return;
        }
        sentsize = (datasize_t)(sentsize + ret); // Don't care SOCKERR_BUSY, because it is zero.
    }
}

int32_t wd_eth_telnet_ops(uint8_t sock_num) {
    if (wd_eth_tcp_ops(sock_num) != WD_CODE_OK) {
        return (WD_CODE_ERROR);
    }

    if (wd_eth_socket[sock_num].Status == SOCK_ESTABLISHED) {
        uint16_t rx_cnt;

        rx_cnt = (uint16_t)wd_eth_get_rx_count(sock_num);

        if (rx_cnt > 0) {
            if (recv(sock_num, Driver_Ethernet_Socket_Buf, (datasize_t)rx_cnt) <= 0) {
                WD_LOG_ERROR();
            } else {
#if (RTE_PLATFORM_PRINT_TELNET)
                wd_cbuf_push(&wd_eth_telnet_rx_c_buf, Driver_Ethernet_Socket_Buf, rx_cnt);
#endif
            }
        }

        {
            datasize_t tx_free_size;

            getsockopt(sock_num, SO_SENDBUF, &tx_free_size);

#if (RTE_PLATFORM_PRINT_TELNET)
            if ((tx_free_size >= 2048) && wd_eth_telnet_tx_c_buf.Size) {
                if ((wd_time_calc_us(&wd_eth_telnet_tx_time_stamp) > 50000) ||
                    (wd_eth_telnet_tx_c_buf.Size > 1900)) {
                    wd_eth_telnet_flush_telnet(0);
                }
            }
#endif
        }
    }

    return (WD_CODE_OK);
}
#endif

#endif

#endif /*--------------- RTE_COMPONENT_INTERFACE_ETHERNET ---------------*/
#endif // #if 1	// 토글


/** @} end of file */
