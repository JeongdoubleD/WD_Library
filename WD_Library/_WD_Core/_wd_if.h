/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 *//**
 *  \version         V1.0.0 [MDK 5.37]
 *  \author
 ** @{
 ******************************************************************************/

/* umbrella header first (before the guard) : any include order expands in the umbrella order */
#include "wd_core.h"

#ifndef WD_CORE_WD_IF_H
#define WD_CORE_WD_IF_H

#if defined(__cplusplus)
extern "C" {
#endif


/***************************************************************************************************************************************************************
 **********  Porting Layer
 **************************************************************************************************************************************************************/
typedef IRQn_Type wd_irq_type_t;
typedef DMA_TypeDef wd_dma_type_t;

typedef struct {
    char *Ptr;
    uint32_t Pos;
    uint32_t Size;
} wd_buffer_t;

typedef struct {
    uint8_t *Tx_Buf;
    uint32_t Tx_Size;
    uint32_t Tx_Head;
    uint32_t Tx_Tail;
    uint8_t *Rx_Buf;
    uint32_t Rx_Size;
    uint32_t Rx_Head;
    uint32_t Rx_Tail;
} wd_circ_buffer_t;


/***************************************************************************************************************************************************************
 **********  Function
 **************************************************************************************************************************************************************/
typedef void (*wd_write_func_t)(const void *data, uint32_t num, uint32_t arg);
typedef int32_t (*wd_tstc_func_t)(void);
typedef int32_t (*wd_read_func_t)(void *data, uint32_t num, uint32_t arg);
typedef int (*wd_printx_t)(const char *fmt, ...);
typedef int (*wd_vprintx_t)(const char *fmt, va_list *ap);
typedef int (*wd_snprintx_t)(char *s, size_t n, const char *fmt, ...);
typedef int (*wd_vsnprintx_t)(char *s, size_t n, const char *fmt, va_list *ap);
typedef int32_t (*wd_parser_t)(const void *protocol, uint32_t head, uint32_t tail, uint32_t arg);
typedef void (*wd_flush_t)(uint32_t arg);

typedef struct {
    uint32_t Call;
    uint32_t Call_Stamp;
    uint32_t Exec;
    uint32_t Exec_Stamp;
} wd_timeit_t;


/***************************************************************************************************************************************************************
 **********  Callback
 **************************************************************************************************************************************************************/
typedef void (*wd_callback_t)(void);
typedef int32_t (*wd_callback_function_t)(uint32_t arg);
typedef int (*wd_callback_api_t)(int argc, char *argv[]);

typedef void (*wd_event_callback_t)(uint32_t event);


/***************************************************************************************************************************************************************
 **********  Firmware
 **************************************************************************************************************************************************************/
#define WDIF_FIRMWARE_UPDATE_INTERFACE_VERSION      0
#define WDIF_FIRMWARE_UPDATE_INTERFACE_MAGIC_NUMBER 0xA55AA500
#define WDIF_FIRMWARE_ISP_MODE_MAGIC_NUMBER         0x12345678

typedef struct {
    uint32_t File_Type;         // 1 : Download Application
                                // 2 : Download Bootloader
                                // 3 : Download Library
                                // 4 : Factory Application
                                // 5 : Factory Bootloader
                                // 6 : Factory Library
    uint32_t File_Location;     // 파일 저장 위치 (0 : 사용하지 않음)
    char File_Name[128];        // 이미지 이름
    uint32_t File_Length;       // 이미지 크기
    uint32_t Modification_Date; // 최종 이미지 생성 or 전송 시간 (UNIX Time)
    uint32_t Update_Date;       // 이미지 적용 일자 (UNIX Time) (0 : 즉시 적용)
    uint32_t File_CheckSum;     // 이미지 체크섬 (CRC-32)

    /*--------------- User ---------------*/
    char Copyright[64];
    char Department[64];     //
    char Author[16];         //
    char Project[128];       //
    char Product_Name[128];  //
    char Specification[128]; //
    uint32_t Creation_Date;  // 메인 라인 이미지 생성 시간 (UNIX Time)
    uint32_t Release_Date;   // 이미지 배포 일자 (UNIX Time)
    uint8_t Commit[40];      // Commit ID
    char Memo[292];          // 기타 정보

    uint8_t Reserved[1024];

    /*--------------- Checksum ---------------*/
    uint32_t Header_CheckSum; // Header 체크섬 (CRC-32, File Type ~ Memo)
} wd_firmware_header_t;

typedef struct {
    char File_Name[128];
    uint32_t File_Length;
    uint32_t Modification_Date;
#if 0
  uint8_t File_Mode;
  uint8_t Serial_Number;
  uint8_t Number_Of_Files_Remaining;
  uint8_t Number_Of_Bytes_Remaining;
  uint8_t File_Type;
#endif
} wd_firmware_file_info_t;

typedef struct {
    uint8_t FW_Area_Idx : 1;                // 인덱스
    uint8_t EVT_Firmware_System_Update : 1; // 펌웨어 업데이트 실행 완료
    uint8_t : 6;
    uint8_t FW_Activate; // 펌웨어 업데이트 실행
    uint8_t FW_RollBack; // 펌웨어 업데이트 복원
    uint8_t FW_ISP_Mode; // 부트로더 펌웨어 업데이트 입력 모드
    uint8_t
        Boot_Fail_Count; // 앱 점프 전 부트로더가 증가, 앱이 안정 부팅하면 0으로 리셋. 5 누적 시 Factory 복구 트리거

    uint32_t FW_ISP_Mode_Magic; // 0x12345678
    uint32_t FW_ISP_Mode_Conrtol;
    uint32_t FW_ISP_Mode_Baudrate;

    uint32_t FW_Update_Interface_Version; // FW 업데이트 인터페이스 버전 (0xA55AA5XX, XX : 버전)
    uint32_t FW_Type;                     // 1 : Download Application
                                          // 2 : Download Bootloader
                                          // 3 : Download Library
                                          // 4 : Factory Application
                                          // 5 : Factory Bootloader
                                          // 6 : Factory Library
    uint32_t FW_Image_Addr;               // FW 이미지 주소
    uint32_t FW_Image_Size;               // FW 이미지 전체 크기
    uint32_t CRC_32;

    char FW_Version[128];

    uint16_t Padding;
    uint16_t Digest; // wd_toolkit_crc16
} wd_firmware_t;

typedef enum {
    WDIF_FIRMWARE_FOTA_IMAGE_TRANSFER_NOT_INITIATED = 0,
    WDIF_FIRMWARE_FOTA_IMAGE_TRANSFER_INITIATED = 1,
    WDIF_FIRMWARE_FOTA_IMAGE_VERIFICATION_INITIATED = 2,
    WDIF_FIRMWARE_FOTA_IMAGE_VERIFICATION_SUCCESSFUL = 3,
    WDIF_FIRMWARE_FOTA_IMAGE_VERIFICATION_FAILED = 4,
    WDIF_FIRMWARE_FOTA_IMAGE_ACTIVATION_INITIATED = 5,
    WDIF_FIRMWARE_FOTA_IMAGE_ACTIVATION_SUCCESSFUL = 6,
    WDIF_FIRMWARE_FOTA_IMAGE_ACTIVATION_FAILED = 7,
    WDIF_FIRMWARE_FOTA_IMAGE_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_firmware_fota_image_transfer_status_t;

typedef struct {
    wd_firmware_fota_image_transfer_status_t Image_Transfer_Status;
    wd_firmware_header_t Firmware_Header;
    wd_firmware_file_info_t File_Info; // Z-Modem

    uint32_t File_Type;
    uint32_t File_Location;
    uint32_t File_Length;
    uint32_t Update_Date;
    uint32_t File_CheckSum;

    uint32_t Set_Offset;
} wd_firmware_fota_info_t;


typedef enum {
    /*--------------- Request ---------------*/
    WDIF_FIRMWARE_FOTA_IMAGE_TRANSFER_INITIATE = 1, // Firmware Update 전송 초기화
    WDIF_FIRMWARE_FOTA_IMAGE_BLOCK_TRANSFER = 2,    // Firmware Update Image 전송
    WDIF_FIRMWARE_FOTA_IMAGE_VERIFY = 3,            // Firmware Update Image 검증
    WDIF_FIRMWARE_FOTA_IMAGE_ACTIVATE = 4,          // Firmware Update 실행
    /*--------------- Response ---------------*/
    WDIF_FIRMWARE_FOTA_ACK = 0,                         // ACK, OK
    WDIF_FIRMWARE_FOTA_NACK = -1,                       // Error
    WDIF_FIRMWARE_FOTA_IMAGE_BLOCK_TRANSFER_ERROR = -2, // Firmware Update Image 전송 실패
    WDIF_FIRMWARE_FOTA_IMAGE_VERIFY_ERROR = -3,         // Firmware Update 검증 실패
    WDIF_FIRMWARE_FOTA_ENUM_UINT32_MAX = 0x7FFFFFFF     // 직접 정의
} wd_firmware_fota_table_t;


/***************************************************************************************************************************************************************
 **********  Driver
 **************************************************************************************************************************************************************/
typedef enum {
    WDIF_ARG_NONE = 0,
    WDIF_ARG_EVENT = (1UL << 0),
    WDIF_ARG_RESET = (1UL << 1),
    WDIF_ARG_SAVE = (1UL << 2),

    WDIF_ARG_WARM_RESET = false,
    WDIF_ARG_COLD_RESET = true,

    WDIF_ARG_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의

} wd_arg_table_t;

typedef enum {
    WDIF_EVENT_NONE = 0,
    WDIF_EVENT_OK = (1UL << 0),
    WDIF_EVENT_ERROR = (1UL << 1),
    WDIF_EVENT_SUCCESS = (1UL << 2),
    WDIF_EVENT_FAIL = (1UL << 3),
    WDIF_EVENT_ALARM = (1UL << 4),
    WDIF_EVENT_DONE = (1UL << 5),
    WDIF_EVENT_READY = (1UL << 6),
    WDIF_EVENT_WAKEUP = (1UL << 7),
    WDIF_EVENT_SUSPEND = (1UL << 8),
    WDIF_EVENT_ABORTED = (1UL << 9),
    WDIF_EVENT_TIMEOUT = (1UL << 10),
    WDIF_EVENT_CONNECT = (1UL << 11),
    WDIF_EVENT_DISCONNECT = (1UL << 12),
    WDIF_EVENT_ESTABLISHED = (1UL << 13),
    WDIF_EVENT_ACK = (1UL << 14),
    WDIF_EVENT_NACK = (1UL << 15),
    WDIF_EVENT_TX_FRAME = (1UL << 16),
    WDIF_EVENT_TX_UNDERFLOW = (1UL << 17),
    WDIF_EVENT_RX_BREAK = (1UL << 18),
    WDIF_EVENT_RX_FRAME = (1UL << 19),
    WDIF_EVENT_RX_FRAMING_ERROR = (1UL << 20),
    WDIF_EVENT_RX_OVERFLOW = (1UL << 21),
    WDIF_EVENT_RX_PARITY_ERROR = (1UL << 22),
    WDIF_EVENT_RX_TIMEOUT = (1UL << 23),
    WDIF_EVENT_SEND_COMPLETE = (1UL << 24),
    WDIF_EVENT_RECEIVE_COMPLETE = (1UL << 25),
    WDIF_EVENT_TRANSFER_COMPLETE = (1UL << 26),
    WDIF_EVENT_TRANSFER_ERROR = (1UL << 27),
    WDIF_EVENT_TRANSFER_INCOMPLETE = (1UL << 28),
    WDIF_EVENT_TRANSFER_TIMEOUT = (1UL << 29),
    WDIF_EVENT_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_event_table_t;


typedef enum {
    WDIF_FLAG_INITIALIZED = (1UL << 0),
    WDIF_FLAG_POWERED = (1UL << 1),
    WDIF_FLAG_CONFIGURED = (1UL << 2),
    WDIF_FLAG_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_flag_table_t;

typedef struct {
    wd_dma_type_t *Tx_DMA;
    uint8_t Tx_DMA_Channel;
    uint8_t Tx_DMA_Request;
    wd_irq_type_t Tx_IRQn;

    wd_dma_type_t *Rx_DMA;
    uint8_t Rx_DMA_Channel;
    uint8_t Rx_DMA_Request;
    wd_irq_type_t Rx_IRQn;
} wd_dma_capabilities_t;


/***************************************************************************************************************************************************************
 **********  Connectivity
 **************************************************************************************************************************************************************/
#if 1
typedef struct {
    wd_system_symbol_t ID; ///< Object Identifier
    uint8_t Tyoe;          ///< Message Type
    uint8_t Priority;      ///< Message Flag & Priority
    uint16_t Size;         ///< Message Size
    void *Ptr;             ///< Message Pointer
} wd_message_t;

typedef struct {
    uint8_t *Tx_Buf;      ///< Pointer to out data buffer
    uint8_t *Rx_Buf;      ///< Pointer to in data buffer
    uint16_t Tx_Buf_Size; ///< Pointer to out data buffer Size
    uint16_t Rx_Buf_Size; ///< Pointer to in data buffer Size
    uint16_t Tx_Num;      ///< Total number of send
    uint16_t Rx_Num;      ///< Total number of receive
    uint16_t Tx_Cnt;      ///< Number of data sent
    uint16_t Rx_Cnt;      ///< Number of data received
    uint16_t Tx_Offset;
    uint16_t Rx_Offset;
} wd_xfer_t;

typedef struct {
    char *Info;
    uint32_t Tx_Wait_Timer; ///< [ms] 연속 패킷 전송시 사이의 대기 시간
    uint32_t
        Rx_Inter_Octet_Timeout; ///< [ms] Timeout 발생시 ARM_USART_EVENT_RECEIVE_COMPLETE 이벤트 및 parser 호출(parser 실패시 fflush 미실행)
    uint32_t
        Rx_Frame_Timeout; ///< [ms] Timeout 발생시 ARM_USART_EVENT_RECEIVE_COMPLETE 이벤트 및 parser 호출(parser 실패시 fflush 실행)
    void *Private;
} const wd_network_capabilities_t;

typedef volatile struct wdif_network_status_t_s {
    uint32_t Initialized : 1; ///< Initialized Powered Configured
    uint32_t Power : 1;       ///< Powered On
    uint32_t Configure : 1;   ///< Configured
    uint32_t Tx_Busy : 1;     ///< Transmitter busy flag
    uint32_t Rx_Busy : 1;     ///< Receiver busy flag
    uint32_t Rx_Complete : 1; ///< Receiver complete flag
    uint32_t Rx_Timeout : 1;  ///< Receiver time out flag
    uint32_t Error : 1;       ///< Error flag
    uint32_t Loop_Back : 1;   ///< Loop Back flag
    uint32_t Indicator_Tx : 1;
    uint32_t Indicator_Rx : 1;
    uint32_t Indicator_Error : 1;
    uint32_t reserved : 20;
} wd_network_status_t;

typedef struct {
    void *Owner;
    wd_network_status_t Status; ///< Status
    wd_xfer_t Xfer;             ///< Network representing a message flow
    uint32_t Event;             ///< Event
    uint32_t Time_Stamp;
    uint32_t Wait_Timer;    ///< Wait Process Timer
    uint32_t Tx_Timeout;    ///< Tx_Timeout
    uint32_t Rx_Timeout;    ///< Rx_Timeout
    uint32_t Error_Timeout; ///< Error_Timeout
    wd_message_t Tx_Msg;
    wd_message_t Rx_Msg;
    osMessageQueueId_t Msg_Queue; ///< Default Message Queue
} wd_network_info_t;

typedef struct {
    wd_system_symbol_t ID;                   ///< ID
    wd_network_capabilities_t *Capabilities; ///< Capabilities
    wd_event_callback_t Signal_Event; ///< Handle Driver Network Serial Interface Event Process.
    uint8_t *Tx_Buf;                  ///< Default Tx Buffer
    uint8_t *Rx_Buf;                  ///< Default Rx Buffer
    uint16_t Tx_Buf_Size;             ///< Default Tx Buffer Size
    uint16_t Rx_Buf_Size;             ///< Default Rx Buffer Size
    const osMessageQueueAttr_t *Msg_Queue_Attr; ///< Default Message Queue Attribute
    uint32_t Msg_Queue_Size;                    ///< Default Message Queue Size
    void const *Drv;                            ///< Driver
    void *Handle;                               ///< Handle
    void const *Protocol;                       ///< Protocol
    int32_t (*Parser)(
        const void *port, uint8_t *head, uint8_t *tail,
        uint32_t
            arg); ///< frame parser of the upper layer (set by the service that owns the port, NULL : none)
    wd_network_info_t *Info; ///< Resource
    void *Private;
} const wd_network_port_t;


typedef struct {
#if (RTE_CONFIG_DRIVER_EXTENSION)
    const char *(*GetVersion)(void);
#endif
    int32_t (*Register)(wd_network_port_t *port);
    int32_t (*Unregister)(wd_network_port_t *port);
    void (*Tx_Ops)(wd_network_port_t *port);
    void (*Tx_Complete)(wd_network_port_t *port);
    int32_t (*Rx_Ops)(wd_network_port_t *port);
    int32_t (*Rx_Complete)(wd_network_port_t *port);
    void (*Rx_Flush)(wd_network_port_t *port, uint16_t parsed_data_idx);
    int32_t (*Timeout)(wd_network_port_t *port, uint32_t time);
    void (*Error)(wd_network_port_t *port);
    int32_t (*Check)(wd_network_port_t *port);
    void (*Process)(wd_network_port_t *port, uint32_t arg);
    void *(*Get_Priv)(wd_network_port_t *port);
} const wd_network_driver_t;
#endif


#if 1
/***************************************************************************************************************************************************************
 **********  Socket
 **************************************************************************************************************************************************************/
typedef enum {
    DCCP_ROLE_UNDEFINED,
    DCCP_ROLE_LISTEN,
    DCCP_ROLE_CLIENT,
    DCCP_ROLE_SERVER,
} wd_dccp_role_t;

typedef enum {
    DCCP_PKT_REQUEST = 0,
    DCCP_PKT_RESPONSE,
    DCCP_PKT_DATA,
    DCCP_PKT_ACK,
    DCCP_PKT_DATAACK,
    DCCP_PKT_CLOSEREQ,
    DCCP_PKT_CLOSE,
    DCCP_PKT_RESET,
    DCCP_PKT_SYNC,
    DCCP_PKT_SYNCACK,
    DCCP_PKT_INVALID,
} wd_dccp_pkt_type_t;

typedef enum {
    DCCP_OPEN = 1,
    DCCP_REQUESTING,
    DCCP_LISTEN,
    DCCP_RESPOND,
    DCCP_ACTIVE_CLOSEREQ,
    DCCP_PASSIVE_CLOSE,
    DCCP_CLOSING,
    DCCP_TIME_WAIT,
    DCCP_CLOSED,
    DCCP_NEW_SYN_RECV,
    DCCP_PARTOPEN,
    DCCP_PASSIVE_CLOSEREQ,
    DCCP_MAX_STATES
} wd_dccp_state_t;

typedef enum {
    DCCPF_OPEN = (1 << DCCP_OPEN),
    DCCPF_REQUESTING = (1 << DCCP_REQUESTING),
    DCCPF_LISTEN = (1 << DCCP_LISTEN),
    DCCPF_RESPOND = (1 << DCCP_RESPOND),
    DCCPF_ACTIVE_CLOSEREQ = (1 << DCCP_ACTIVE_CLOSEREQ),
    DCCPF_PASSIVE_CLOSE = (1 << DCCP_PASSIVE_CLOSE),
    DCCPF_CLOSING = (1 << DCCP_CLOSING),
    DCCPF_TIME_WAIT = (1 << DCCP_TIME_WAIT),
    DCCPF_CLOSED = (1 << DCCP_CLOSED),
    DCCPF_NEW_SYN_RECV = (1 << DCCP_NEW_SYN_RECV),
    DCCPF_PARTOPEN = (1 << DCCP_PARTOPEN),
    DCCPF_PASSIVE_CLOSEREQ = (1 << DCCP_PASSIVE_CLOSEREQ),
    DCCPF_MAX_STATES = (1 << DCCP_MAX_STATES),
} wd_dccpf_state_t;

#ifndef htonl
#define htonl(x) (x)
#define ntohl(x) (x)
#define htons(x) (x)
#define ntohs(x) (x)
#endif

typedef enum {
    SOCK_DGRAM = 1,
    SOCK_STREAM = 2,
    SOCK_RAW = 3,
    SOCK_RDM = 4,
    SOCK_SEQPACKET = 5,
    SOCK_DCCP = 6,
    SOCK_PACKET = 10,
} wd_sock_type_t;

#ifndef AF_INET
// Supported address families.
#define AF_UNSPEC     0
#define AF_UNIX       1  // Unix domain sockets
#define AF_LOCAL      1  // POSIX name for AF_UNIX
#define AF_FILE       1  // POSIX name for AF_UNIX
#define AF_INET       2  // Internet IP Protocol
#define AF_AX25       3  // Amateur Radio AX.25
#define AF_IPX        4  // Novell IPX
#define AF_APPLETALK  5  // AppleTalk DDP
#define AF_NETROM     6  // Amateur Radio NET/ROM
#define AF_BRIDGE     7  // Multiprotocol bridge
#define AF_ATMPVC     8  // ATM PVCs
#define AF_X25        9  // Reserved for X.25 project
#define AF_INET6      10 // IP version 6
#define AF_ROSE       11 // Amateur Radio X.25 PLP
#define AF_DECnet     12 // Reserved for DECnet project
#define AF_NETBEUI    13 // Reserved for 802.2LLC project
#define AF_SECURITY   14 // Security callback pseudo AF
#define AF_KEY        15 // PF_KEY key management API
#define AF_NETLINK    16
#define AF_ROUTE      AF_NETLINK // Alias to emulate 4.4BSD
#define AF_PACKET     17         // Packet family
#define AF_ASH        18         // Ash
#define AF_ECONET     19         // Acorn Econet
#define AF_ATMSVC     20         // ATM SVCs
#define AF_RDS        21         // RDS sockets
#define AF_SNA        22         // Linux SNA Project (nutters!)
#define AF_IRDA       23         // IRDA sockets
#define AF_PPPOX      24         // PPPoX sockets
#define AF_WANPIPE    25         // Wanpipe API Sockets
#define AF_LLC        26         // Linux LLC
#define AF_IB         27         // Native InfiniBand address
#define AF_MPLS       28         // MPLS
#define AF_CAN        29         // Controller Area Network
#define AF_TIPC       30         // TIPC sockets
#define AF_BLUETOOTH  31         // Bluetooth sockets
#define AF_IUCV       32         // IUCV sockets
#define AF_RXRPC      33         // RxRPC sockets
#define AF_ISDN       34         // mISDN sockets
#define AF_PHONET     35         // Phonet sockets
#define AF_IEEE802154 36         // IEEE802154 sockets
#define AF_CAIF       37         // CAIF sockets
#define AF_ALG        38         // Algorithm sockets
#define AF_NFC        39         // NFC sockets
#define AF_VSOCK      40         // vSockets
#define AF_KCM        41         // Kernel Connection Multiplexor
#define AF_QIPCRTR    42         // Qualcomm IPC Router
#define AF_SMC \
    43 // smc sockets: reserve number for PF_SMC protocol family that reuses AF_INET address family
#define AF_XDP  44 // XDP sockets
#define AF_MCTP 45 // Management component transport protocol
#define AF_MAX  46 // For now..

// Protocol families, same as address families.
#define PF_UNSPEC     AF_UNSPEC
#define PF_UNIX       AF_UNIX
#define PF_LOCAL      AF_LOCAL
#define PF_FILE       AF_FILE
#define PF_INET       AF_INET
#define PF_AX25       AF_AX25
#define PF_IPX        AF_IPX
#define PF_APPLETALK  AF_APPLETALK
#define PF_NETROM     AF_NETROM
#define PF_BRIDGE     AF_BRIDGE
#define PF_ATMPVC     AF_ATMPVC
#define PF_X25        AF_X25
#define PF_INET6      AF_INET6
#define PF_ROSE       AF_ROSE
#define PF_DECnet     AF_DECnet
#define PF_NETBEUI    AF_NETBEUI
#define PF_SECURITY   AF_SECURITY
#define PF_KEY        AF_KEY
#define PF_NETLINK    AF_NETLINK
#define PF_ROUTE      AF_ROUTE
#define PF_PACKET     AF_PACKET
#define PF_ASH        AF_ASH
#define PF_ECONET     AF_ECONET
#define PF_ATMSVC     AF_ATMSVC
#define PF_RDS        AF_RDS
#define PF_SNA        AF_SNA
#define PF_IRDA       AF_IRDA
#define PF_PPPOX      AF_PPPOX
#define PF_WANPIPE    AF_WANPIPE
#define PF_LLC        AF_LLC
#define PF_IB         AF_IB
#define PF_MPLS       AF_MPLS
#define PF_CAN        AF_CAN
#define PF_TIPC       AF_TIPC
#define PF_BLUETOOTH  AF_BLUETOOTH
#define PF_IUCV       AF_IUCV
#define PF_RXRPC      AF_RXRPC
#define PF_ISDN       AF_ISDN
#define PF_PHONET     AF_PHONET
#define PF_IEEE802154 AF_IEEE802154
#define PF_CAIF       AF_CAIF
#define PF_ALG        AF_ALG
#define PF_NFC        AF_NFC
#define PF_VSOCK      AF_VSOCK
#define PF_KCM        AF_KCM
#define PF_QIPCRTR    AF_QIPCRTR
#define PF_SMC        AF_SMC
#define PF_XDP        AF_XDP
#define PF_MCTP       AF_MCTP
#define PF_MAX        AF_MAX
#endif

/* Standard well-defined IP protocols.  */
typedef enum {
    IPPROTO_IP = 0,         // Dummy protocol for TCP
    IPPROTO_ICMP = 1,       // Internet Control Message Protocol
    IPPROTO_IGMP = 2,       // Internet Group Management Protocol
    IPPROTO_IPIP = 4,       // IPIP tunnels (older KA9Q tunnels use 94)
    IPPROTO_TCP = 6,        // Transmission Control Protocol
    IPPROTO_EGP = 8,        // Exterior Gateway Protocol
    IPPROTO_PUP = 12,       // PUP protocol
    IPPROTO_UDP = 17,       // User Datagram Protocol
    IPPROTO_IDP = 22,       // XNS IDP protocol
    IPPROTO_TP = 29,        // SO Transport Protocol Class 4
    IPPROTO_DCCP = 33,      // Datagram Congestion Control Protocol
    IPPROTO_IPV6 = 41,      // IPv6-in-IPv4 tunnelling
    IPPROTO_RSVP = 46,      // RSVP Protocol
    IPPROTO_GRE = 47,       // Cisco GRE tunnels (rfc 1701, 1702)
    IPPROTO_ESP = 50,       // Encapsulation Security Payload protocol
    IPPROTO_AH = 51,        // Authentication Header protocol
    IPPROTO_MTP = 92,       // Multicast Transport Protocol
    IPPROTO_BEETPH = 94,    // IP option pseudo header for BEET
    IPPROTO_ENCAP = 98,     // Encapsulation Header
    IPPROTO_PIM = 103,      // Protocol Independent Multicast
    IPPROTO_COMP = 108,     // Compression Header Protocol
    IPPROTO_L2TP = 115,     // Layer 2 Tunnelling Protocol
    IPPROTO_SCTP = 132,     // Stream Control Transport Protocol
    IPPROTO_UDPLITE = 136,  // UDP-Lite (RFC 3828)
    IPPROTO_MPLS = 137,     // MPLS in IP (RFC 4023)
    IPPROTO_ETHERNET = 143, // Ethernet-within-IPv6 Encapsulation
    IPPROTO_RAW = 255,      // Raw IP packets
    IPPROTO_MPTCP = 262,    // Multipath TCP connection
    IPPROTO_MAX

#define IPPROTO_IP       IPPROTO_IP
#define IPPROTO_ICMP     IPPROTO_ICMP
#define IPPROTO_IGMP     IPPROTO_IGMP
#define IPPROTO_IPIP     IPPROTO_IPIP
#define IPPROTO_TCP      IPPROTO_TCP
#define IPPROTO_EGP      IPPROTO_EGP
#define IPPROTO_PUP      IPPROTO_PUP
#define IPPROTO_UDP      IPPROTO_UDP
#define IPPROTO_IDP      IPPROTO_IDP
#define IPPROTO_TP       IPPROTO_TP
#define IPPROTO_DCCP     IPPROTO_DCCP
#define IPPROTO_IPV6     IPPROTO_IPV6
#define IPPROTO_RSVP     IPPROTO_RSVP
#define IPPROTO_GRE      IPPROTO_GRE
#define IPPROTO_ESP      IPPROTO_ESP
#define IPPROTO_AH       IPPROTO_AH
#define IPPROTO_MTP      IPPROTO_MTP
#define IPPROTO_BEETPH   IPPROTO_BEETPH
#define IPPROTO_ENCAP    IPPROTO_ENCAP
#define IPPROTO_PIM      IPPROTO_PIM
#define IPPROTO_COMP     IPPROTO_COMP
#define IPPROTO_L2TP     IPPROTO_L2TP
#define IPPROTO_SCTP     IPPROTO_SCTP
#define IPPROTO_UDPLITE  IPPROTO_UDPLITE
#define IPPROTO_MPLS     IPPROTO_MPLS
#define IPPROTO_ETHERNET IPPROTO_ETHERNET
#define IPPROTO_RAW      IPPROTO_RAW
#define IPPROTO_MPTCP    IPPROTO_MPTCP
} wd_ip_proto_t;

typedef struct {
    uint8_t IP[4];
    uint8_t Subnet_Mask[4];
    uint8_t Gateway[4];
    uint8_t DNS[4];
} wd_ipv4_t;
#endif


/***************************************************************************************************************************************************************
 **********  Terminal
 **************************************************************************************************************************************************************/
typedef enum {
/*--------------- Status   : [ 0: 7] ---------------*/
//  wd_state_t
/*--------------- Config   : [ 8:15] ---------------*/
#define WDIF_TERMINAL_CONFIG_POS 8
#define WDIF_TERMINAL_CONFIG_MSK (0xFFUL << WDIF_TERMINAL_CONFIG_POS)
#define WDIF_TERMINAL_CONFIG(n)  (((n) & 0xFFUL) << WDIF_TERMINAL_CONFIG_POS)
    WDIF_TERMINAL_CONFIG_NONE = WDIF_TERMINAL_CONFIG(0),
    WDIF_TERMINAL_CONFIG_OWN_DRIVER = WDIF_TERMINAL_CONFIG(1UL << 0),
    WDIF_TERMINAL_CONFIG_EVENTRECORDER = WDIF_TERMINAL_CONFIG(1UL << 1),
    WDIF_TERMINAL_CONFIG_ITM = WDIF_TERMINAL_CONFIG(1UL << 2),
    WDIF_TERMINAL_CONFIG_RTT = WDIF_TERMINAL_CONFIG(1UL << 3),
    WDIF_TERMINAL_CONFIG_SERIAL = WDIF_TERMINAL_CONFIG(1UL << 4),
    WDIF_TERMINAL_CONFIG_TELNET = WDIF_TERMINAL_CONFIG(1UL << 5),
    WDIF_TERMINAL_CONFIG_TERMINAL = WDIF_TERMINAL_CONFIG(1UL << 6),
/*--------------- Argument : [16:23] ---------------*/
#define WDIF_TERMINAL_ARGUMENT_POS 16
#define WDIF_TERMINAL_ARGUMENT_MSK (0xFFUL << WDIF_TERMINAL_ARGUMENT_POS)
#define WDIF_TERMINAL_ARGUMENT(n)  (((n) & 0xFFUL) << WDIF_TERMINAL_ARGUMENT_POS)
#define WDIF_TERMINAL_ARGUMENT_GET_CHANNEL(ch) \
    (((ch) & WDIF_TERMINAL_ARGUMENT_MSK) >> WDIF_TERMINAL_ARGUMENT_POS)

/*--------------- Control  : [24:31] ---------------*/
#define WDIF_TERMINAL_CONTROL_POS 24
#define WDIF_TERMINAL_CONTROL_MSK (0xFFUL << WDIF_TERMINAL_CONTROL_POS)
#define WDIF_TERMINAL_CONTROL(n)  (((n) & 0xFFUL) << WDIF_TERMINAL_CONTROL_POS)
    WDIF_TERMINAL_CONTROL_NONE = WDIF_TERMINAL_CONTROL(0),
    WDIF_TERMINAL_CONTROL_TEST = WDIF_TERMINAL_CONTROL(1UL),
} wd_terminal_control_table_t;

typedef struct {
    /*--------------- General functions ---------------*/
    void (*Lock)(void);                     // To Lock
    void (*Unlock)(void);                   // To Unlock
    int32_t (*Start)(void);                 // To start the device
    int32_t (*Stop)(void);                  // To stop the device
    int32_t (*Set_Baudrate)(uint32_t baud); // Set up the baud rate generator
    /*--------------- Output functions ---------------*/
    void (*Putc)(const char c);                                  // To put a char
    void (*Puts)(const void *ptr);                               // To put a string (accelerator)
    void (*Write)(const void *data, uint32_t num, uint32_t arg); // To Write
    int (*Print)(const char *fmt, ...);                          // printx
    void (*Flush)(void);
    /*--------------- Input functions ---------------*/
    int32_t (*Getc)(void); // To get that char
    int32_t (*Tstc)(
        void); // To test if a char is ready... positive if character is available, zero otherwise.
    void (*Read)(void *ptr, uint32_t num); // To Write
    /*--------------- Config functions ---------------*/
    void (*Set_Control)(uint32_t control);
    uint32_t (*Get_Control)(void);
    /*--------------- Data ---------------*/
    void *(*Get_Priv)(void); // Private extensions
} const wd_terminal_t;       // u-boot stdio_dev.h struct stdio_dev  circ_buf.h

#if 0
/***************************************************************************************************************************************************************
 **********  Storage
 **************************************************************************************************************************************************************/
extern uint32_t wd_blockdev_retry_error_e_w;
extern uint32_t wd_blockdev_retry_error_r;

#define WD_BLOCK_DEVICE_ARGUMENT_WRITE 1
#define WD_BLOCK_DEVICE_ARGUMENT_ERASE 0
#define WD_BLOCK_DEVICE_RETRY          3

typedef enum
{
  /*--------------- Command ---------------*/
#define WDIF_BLOCK_DEVICE_COMMAND_POS  0
#define WDIF_BLOCK_DEVICE_COMMAND_MSK  (0xFFUL << WDIF_BLOCK_DEVICE_COMMAND_POS)
  WDIF_BLOCK_DEVICE_COMMAND_WRITE           = (1UL << WDIF_BLOCK_DEVICE_COMMAND_POS),
  WDIF_BLOCK_DEVICE_COMMAND_READ            = (2UL << WDIF_BLOCK_DEVICE_COMMAND_POS),
  WDIF_BLOCK_DEVICE_COMMAND_ERASE           = (3UL << WDIF_BLOCK_DEVICE_COMMAND_POS),
  WDIF_BLOCK_DEVICE_COMMAND_INSTANCE_RESET  = (100UL << WDIF_BLOCK_DEVICE_COMMAND_POS),
  WDIF_BLOCK_DEVICE_COMMAND_VALIDATION      = (200UL << WDIF_BLOCK_DEVICE_COMMAND_POS),
  /*--------------- Control ---------------*/
#define WDIF_BLOCK_DEVICE_CONTROL_POS  8
#define WDIF_BLOCK_DEVICE_CONTROL_MSK  (0xFFUL << WDIF_BLOCK_DEVICE_CONTROL_POS)

  /*--------------- Argument ---------------*/
#define WDIF_BLOCK_DEVICE_ARGUMENT_POS 24
#define WDIF_BLOCK_DEVICE_ARGUMENT_MSK (0xFFUL << WDIF_BLOCK_DEVICE_ARGUMENT_POS)

  WDIF_BLOCK_DEVICE_ARGUMENT_CONFIRM_WRITE  = ((1UL << 1) << WDIF_BLOCK_DEVICE_ARGUMENT_POS),
  WDIF_BLOCK_DEVICE_ARGUMENT_INIT           = ((1UL << 2) << WDIF_BLOCK_DEVICE_ARGUMENT_POS),
}wd_block_device_parameter_table_t;

typedef struct
{
  uint32_t                            Virture_Addr;
  uint32_t                            Size;
  uint16_t                            Program_Block_Size;
  uint16_t                            Program_TimeOut;
  uint16_t                            Erase_Block_Size;
  uint16_t                            Erase_TimeOut;
} const wd_block_device_capabilities_t;

typedef struct
{
  wd_event_callback_t                 CB_Event;
  uint32_t                            Status;
  int32_t                    volatile State;
  uint32_t                            Time_Stamp;
  uint32_t                            Event;
} wd_block_device_instance_info_t;

typedef struct
{
  void                              (*Lock)                (void);
  void                              (*Unlock)              (void);
  int32_t                           (*Initialize)          (wd_event_callback_t cb_event);
  int32_t                           (*PowerControl)        (wd_power_state_t state);
  int32_t                           (*Write)               (uint32_t addr, const void *data, uint32_t num);
  int32_t                           (*Read)                (uint32_t addr,       void *data, uint32_t num);
  int32_t                           (*Erase)               (uint32_t addr, const uint8_t data, uint32_t num);
  int32_t                           (*GetStatus)           (void);
  void                              (*Validation)          (void);
  wd_block_device_capabilities_t*    Capabilities;
  uint32_t                           IO_WP;
  uint8_t*                           Buf;
  uint8_t*                           Confirm_Buf;
  uint16_t                           Buf_Size;
  uint16_t                           Confirm_Buf_Size;
  wd_block_device_instance_info_t*   Info;
} const wd_block_device_instance_t;

typedef struct
{
  int32_t                           (*Initialize)           (wd_block_device_instance_t *instance, wd_event_callback_t cb_event);
  int32_t                           (*Write)                (wd_block_device_instance_t *instance, uint32_t addr, const void *data,   uint32_t num);
  int32_t                           (*Read)                 (wd_block_device_instance_t *instance, uint32_t addr,       void *data,   uint32_t num);
  int32_t                           (*Erase)                (wd_block_device_instance_t *instance, uint32_t addr, const uint8_t data, uint32_t num);
  int32_t                           (*Sync)                 (wd_block_device_instance_t *instance, uint32_t arg);
  int32_t                           (*Control)              (wd_block_device_instance_t *instance, uint32_t control);
  wd_block_device_instance_t*       (*Get_Instance)         (uint32_t addr);
}const wd_block_device_t;

extern wd_block_device_t wd_blockdev_driver;

extern wd_block_device_t* const wd_drv_block_device;

#endif

/***************************************************************************************************************************************************************
 **********  Event Recorder
 **************************************************************************************************************************************************************/
typedef struct evr_addr {
    int16_t type;
    uint16_t port;
    uint8_t addr[16];
} wd_evr_addr_t;

// Socket event identifiers ----------------------------------------------------
#define EvtSocket_InitSockets(component)               EventID(EventLevelOp, component, 0)
#define EvtSocket_SocketCreate(component)              EventID(EventLevelAPI, component, 1)
#define EvtSocket_SocketInvalidParameter(component)    EventID(EventLevelError, component, 2)
#define EvtSocket_NoSocketAvailable(component)         EventID(EventLevelError, component, 3)
#define EvtSocket_BindSocket(component)                EventID(EventLevelAPI, component, 4)
#define EvtSocket_BindSocketNotValid(component)        EventID(EventLevelError, component, 5)
#define EvtSocket_BindInvalidParameter(component)      EventID(EventLevelError, component, 6)
#define EvtSocket_BindSocketNotCreated(component)      EventID(EventLevelError, component, 7)
#define EvtSocket_BindSocketAlreadyBound(component)    EventID(EventLevelError, component, 8)
#define EvtSocket_BindSocketConnected(component)       EventID(EventLevelError, component, 9)
#define EvtSocket_BindPortInUse(component)             EventID(EventLevelError, component, 10)
#define EvtSocket_ViewNetAddress(component)            EventID(EventLevelOp, component, 11)
#define EvtSocket_ListenSocket(component)              EventID(EventLevelAPI, component, 13)
#define EvtSocket_ListenSocketNotValid(component)      EventID(EventLevelError, component, 14)
#define EvtSocket_ListenInvalidParameter(component)    EventID(EventLevelError, component, 15)
#define EvtSocket_ListenSocketNotCreated(component)    EventID(EventLevelError, component, 145)
#define EvtSocket_ListenSocketNotBound(component)      EventID(EventLevelError, component, 16)
#define EvtSocket_ListenSocketNotStreamType(component) EventID(EventLevelError, component, 17)
#define EvtSocket_ListenSockAlreadyListens(component)  EventID(EventLevelError, component, 18)
#define EvtSocket_ListenBacklogCreateFailed(component) EventID(EventLevelError, component, 19)
#define EvtSocket_AcceptSocket(component)              EventID(EventLevelAPI, component, 20)
#define EvtSocket_AcceptSocketNotValid(component)      EventID(EventLevelError, component, 21)
#define EvtSocket_AcceptSocketNotCreated(component)    EventID(EventLevelError, component, 146)
#define EvtSocket_AcceptSocketNotListening(component)  EventID(EventLevelError, component, 22)
#define EvtSocket_AcceptSocketNotStreamType(component) EventID(EventLevelError, component, 23)
#define EvtSocket_AcceptSocketLocked(component)        EventID(EventLevelError, component, 24)
#define EvtSocket_AcceptChildSockConnected(component)  EventID(EventLevelOp, component, 25)
#define EvtSocket_AcceptSocketClosed(component)        EventID(EventLevelError, component, 26)
#define EvtSocket_AcceptSocketKilled(component)        EventID(EventLevelError, component, 27)
#define EvtSocket_ConnectSocket(component)             EventID(EventLevelAPI, component, 28)
#define EvtSocket_ConnectSocketNotValid(component)     EventID(EventLevelError, component, 29)
#define EvtSocket_ConnectInvalidParameter(component)   EventID(EventLevelError, component, 30)
#define EvtSocket_ConnectSocketLocked(component)       EventID(EventLevelError, component, 31)
#define EvtSocket_ConnectSocketNotCreated(component)   EventID(EventLevelError, component, 32)
#define EvtSocket_ConnectSocketWrongState(component)   EventID(EventLevelError, component, 33)
#define EvtSocket_ConnectDatagramSuccess(component)    EventID(EventLevelOp, component, 34)
#define EvtSocket_ConnectStreamSuccess(component)      EventID(EventLevelOp, component, 35)
#define EvtSocket_ConnectStreamTimeout(component)      EventID(EventLevelError, component, 36)
#define EvtSocket_ConnectStreamRefused(component)      EventID(EventLevelError, component, 37)
#define EvtSocket_ConnectSocketKilled(component)       EventID(EventLevelError, component, 38)
#define EvtSocket_SendSocket(component)                EventID(EventLevelAPI, component, 39)
#define EvtSocket_SendSocketNotValid(component)        EventID(EventLevelError, component, 40)
#define EvtSocket_SendInvalidParameter(component)      EventID(EventLevelError, component, 41)
#define EvtSocket_SendSocketNotCreated(component)      EventID(EventLevelError, component, 42)
#define EvtSocket_SendSocketLocked(component)          EventID(EventLevelError, component, 43)
#define EvtSocket_SendSocketNotConnected(component)    EventID(EventLevelError, component, 44)
#define EvtSocket_SendSocketWrongState(component)      EventID(EventLevelError, component, 45)
#define EvtSocket_SendSocketClosed(component)          EventID(EventLevelError, component, 46)
#define EvtSocket_SendCompleteNonBlocking(component)   EventID(EventLevelOp, component, 47)
#define EvtSocket_SendSocketKilled(component)          EventID(EventLevelError, component, 48)
#define EvtSocket_SendStreamError(component)           EventID(EventLevelError, component, 49)
#define EvtSocket_SendtoInvalidParameter(component)    EventID(EventLevelError, component, 50)
#define EvtSocket_SendDestAddressUndefined(component)  EventID(EventLevelError, component, 51)
#define EvtSocket_SendtoMsgTooLarge(component)         EventID(EventLevelError, component, 152)
#define EvtSocket_SendtoNoMemory(component)            EventID(EventLevelError, component, 153)
#define EvtSocket_SendDatagramError(component)         EventID(EventLevelError, component, 52)
#define EvtSocket_SendCompleteBlocking(component)      EventID(EventLevelOp, component, 53)
#define EvtSocket_SendTimeoutBlocking(component)       EventID(EventLevelOp, component, 54)
#define EvtSocket_SendNoMemory(component)              EventID(EventLevelOp, component, 55)
#define EvtSocket_SendmsgSocket(component)             EventID(EventLevelAPI, component, 56)
#define EvtSocket_SendmsgSocketNotValid(component)     EventID(EventLevelError, component, 57)
#define EvtSocket_SendmsgInvalidParameter(component)   EventID(EventLevelError, component, 58)
#define EvtSocket_SendmsgSocketNotCreated(component)   EventID(EventLevelError, component, 59)
#define EvtSocket_SendmsgSocketLocked(component)       EventID(EventLevelError, component, 60)
#define EvtSocket_SendmsgDestAddrUndefined(component)  EventID(EventLevelError, component, 61)
#define EvtSocket_SendmsgMsgTooLarge(component)      EventID(EventLevelError, component, 154) // End
#define EvtSocket_SendmsgNoMemory(component)         EventID(EventLevelError, component, 62)
#define EvtSocket_SendmsgDatagramError(component)    EventID(EventLevelError, component, 63)
#define EvtSocket_SendmsgSocketKilled(component)     EventID(EventLevelError, component, 64)
#define EvtSocket_SendmsgComplete(component)         EventID(EventLevelOp, component, 65)
#define EvtSocket_RecvSocket(component)              EventID(EventLevelAPI, component, 66)
#define EvtSocket_RecvSocketNotValid(component)      EventID(EventLevelError, component, 67)
#define EvtSocket_RecvInvalidParameter(component)    EventID(EventLevelError, component, 68)
#define EvtSocket_RecvSocketNotCreated(component)    EventID(EventLevelError, component, 69)
#define EvtSocket_RecvSocketNotConnected(component)  EventID(EventLevelError, component, 70)
#define EvtSocket_RecvSocketWrongState(component)    EventID(EventLevelError, component, 71)
#define EvtSocket_RecvSocketLocked(component)        EventID(EventLevelError, component, 72)
#define EvtSocket_RecvSocketClosed(component)        EventID(EventLevelError, component, 73)
#define EvtSocket_RecvTimeout(component)             EventID(EventLevelOp, component, 74)
#define EvtSocket_RecvSocketKilled(component)        EventID(EventLevelError, component, 75)
#define EvtSocket_RecvQueueFree(component)           EventID(EventLevelOp, component, 76)
#define EvtSocket_RecvComplete(component)            EventID(EventLevelOp, component, 77)
#define EvtSocket_RecvmsgSocket(component)           EventID(EventLevelAPI, component, 78)
#define EvtSocket_RecvmsgSocketNotValid(component)   EventID(EventLevelError, component, 79)
#define EvtSocket_RecvmsgInvalidParameter(component) EventID(EventLevelError, component, 80)
#define EvtSocket_RecvmsgSocketNotCreated(component) EventID(EventLevelError, component, 81)
#define EvtSocket_RecvmsgSocketLocked(component)     EventID(EventLevelError, component, 82)
#define EvtSocket_RecvmsgSocketWrongState(component) EventID(EventLevelError, component, 83)
#define EvtSocket_RecvmsgTimeout(component)          EventID(EventLevelOp, component, 84)
#define EvtSocket_RecvmsgSocketKilled(component)     EventID(EventLevelError, component, 85)
#define EvtSocket_RecvmsgComplete(component)         EventID(EventLevelOp, component, 86)
#define EvtSocket_Closesocket(component)             EventID(EventLevelAPI, component, 87)
#define EvtSocket_CloseSocketNotValid(component)     EventID(EventLevelError, component, 88)
#define EvtSocket_CloseSocketNotCreated(component)   EventID(EventLevelError, component, 147)
#define EvtSocket_Select(component)                  EventID(EventLevelAPI, component, 89)
#define EvtSocket_SelectInvalidParameter(component)  EventID(EventLevelError, component, 90)
#define EvtSocket_SelectSuspendFailed(component)     EventID(EventLevelError, component, 91)
#define EvtSocket_SelectComplete(component)          EventID(EventLevelOp, component, 92)
#define EvtSocket_SelectCompleteBlocking(component)  EventID(EventLevelOp, component, 93)
#define EvtSocket_Getpeername(component)             EventID(EventLevelAPI, component, 94)
#define EvtSocket_GetpeerSocketNotValid(component)   EventID(EventLevelError, component, 95)
#define EvtSocket_GetpeerSocketNotCreated(component) EventID(EventLevelError, component, 148)
#define EvtSocket_GetpeerInvalidParameter(component) EventID(EventLevelError, component, 96)
#define EvtSocket_GetpeerSocketNotConnected(component) EventID(EventLevelError, component, 97)
#define EvtSocket_Getsockname(component)               EventID(EventLevelAPI, component, 98)
#define EvtSocket_GetsockSocketNotValid(component)     EventID(EventLevelError, component, 99)
#define EvtSocket_GetsockSocketNotCreated(component)   EventID(EventLevelError, component, 149)
#define EvtSocket_GetsockInvalidParameter(component)   EventID(EventLevelError, component, 100)
#define EvtSocket_GetsockSocketNotBound(component)     EventID(EventLevelError, component, 101)
#define EvtSocket_Setsockopt(component)                EventID(EventLevelAPI, component, 102)
#define EvtSocket_SetoptSocketNotValid(component)      EventID(EventLevelError, component, 103)
#define EvtSocket_SetoptInvalidParameter(component)    EventID(EventLevelError, component, 104)
#define EvtSocket_SetoptSocketNotCreated(component)    EventID(EventLevelError, component, 105)
#define EvtSocket_SetoptOptionNotSupported(component)  EventID(EventLevelError, component, 106)
#define EvtSocket_SetoptKeepAlive(component)           EventID(EventLevelOp, component, 107)
#define EvtSocket_SetoptRecvTimeout(component)         EventID(EventLevelOp, component, 108)
#define EvtSocket_SetoptSendTimeout(component)         EventID(EventLevelOp, component, 109)
#define EvtSocket_SetoptBindToDevice(component)        EventID(EventLevelOp, component, 12)
#define EvtSocket_SetoptIp4Tos(component)              EventID(EventLevelOp, component, 110)
#define EvtSocket_SetoptIp4Ttl(component)              EventID(EventLevelOp, component, 111)
#define EvtSocket_SetoptIp4RecvDstAddr(component)      EventID(EventLevelOp, component, 112)
#define EvtSocket_SetoptIp6Tclass(component)           EventID(EventLevelOp, component, 113)
#define EvtSocket_SetoptIp6HopLimit(component)         EventID(EventLevelOp, component, 114)
#define EvtSocket_SetoptIp6RecvDstAddr(component)      EventID(EventLevelOp, component, 115)
#define EvtSocket_SetoptIp6Only(component)             EventID(EventLevelOp, component, 150)
#define EvtSocket_SetoptSocketBound(component)         EventID(EventLevelError, component, 151)
#define EvtSocket_Getsockopt(component)                EventID(EventLevelAPI, component, 116)
#define EvtSocket_GetoptSocketNotValid(component)      EventID(EventLevelError, component, 117)
#define EvtSocket_GetoptInvalidParameter(component)    EventID(EventLevelError, component, 118)
#define EvtSocket_GetoptSocketNotCreated(component)    EventID(EventLevelError, component, 119)
#define EvtSocket_GetoptOptionNotSupported(component)  EventID(EventLevelError, component, 120)
#define EvtSocket_Ioctlsocket(component)               EventID(EventLevelAPI, component, 121)
#define EvtSocket_IoctlSocketNotValid(component)       EventID(EventLevelError, component, 122)
#define EvtSocket_IoctlInvalidParameter(component)     EventID(EventLevelError, component, 123)
#define EvtSocket_IoctlSocketNotCreated(component)     EventID(EventLevelError, component, 124)
#define EvtSocket_IoctlSocketNotStreamType(component)  EventID(EventLevelError, component, 125)
#define EvtSocket_IoctlNonBlocking(component)          EventID(EventLevelOp, component, 126)
#define EvtSocket_IoctlDelayAck(component)             EventID(EventLevelOp, component, 127)
#define EvtSocket_IoctlKeepAlive(component)            EventID(EventLevelOp, component, 128)
#define EvtSocket_IoctlFlowControl(component)          EventID(EventLevelOp, component, 129)
#define EvtSocket_CbfuncTcpEvent(component)            EventID(EventLevelOp, component, 130)
#define EvtSocket_CbfuncTcpQueueAdd(component)         EventID(EventLevelOp, component, 131)
#define EvtSocket_CbfuncTcpNoMemory(component)         EventID(EventLevelError, component, 132)
#define EvtSocket_CbfuncUdpQueueAdd(component)         EventID(EventLevelOp, component, 133)
#define EvtSocket_CbfuncUdpDumpData(component)         EventID(EventLevelError, component, 134)
#define EvtSocket_UninitSockets(component)             EventID(EventLevelOp, component, 135)
/* Host resolver events */
#define EvtSocket_GetHostInit(component)             EventID(EventLevelOp, component, 136)
#define EvtSocket_GetHostByName(component)           EventID(EventLevelAPI, component, 137)
#define EvtSocket_GetHostNameResolved(component)     EventID(EventLevelOp, component, 138)
#define EvtSocket_GetHostNameNotExisting(component)  EventID(EventLevelOp, component, 139)
#define EvtSocket_GetHostResolverTimeout(component)  EventID(EventLevelOp, component, 140)
#define EvtSocket_GetHostResolverError(component)    EventID(EventLevelError, component, 141)
#define EvtSocket_GetHostResolverBusy(component)     EventID(EventLevelError, component, 142)
#define EvtSocket_GetHostInvalidParameter(component) EventID(EventLevelError, component, 143)
#define EvtSocket_GetHostUninit(component)           EventID(EventLevelOp, component, 144)


#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/
#endif /*--------------- WD_CORE_WD_IF_H ---------------*/
/** @} end of file */
