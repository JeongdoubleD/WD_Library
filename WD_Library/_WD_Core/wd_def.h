/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    wd_def.h
 * @brief   WD Library - common definitions (types, codes, macros, compiler helpers)
 *          Hardware/project independent. Do NOT put project configuration here.
 *          (project configuration -> wd_config.h, board mapping -> wd_board.h)
 ******************************************************************************/
/* umbrella header first (before the guard) : any include order expands in the umbrella order */
#include "wd_core.h"

#ifndef WD_CORE_WD_DEF_H
#define WD_CORE_WD_DEF_H

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************************************************************************************************************************************
 **********  Define
 **************************************************************************************************************************************************************/
typedef enum {
    WD_SYSTEM_MODE_BOOT = 0,
    WD_SYSTEM_MODE_NORMAL,
    WD_SYSTEM_MODE_P_FAIL,
    WD_SYSTEM_MODE_WAIT_FOR_EVENT,
    WD_SYSTEM_MODE_WAIT_FOR_INTERRUPT,
    WD_SYSTEM_MODE_WAIT_FOR_SLEEP,
    WD_SYSTEM_MODE_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_system_mode_t;

typedef enum {
    WD_INTERRUPT_PREEMPT_PRIORITY_REALTIME_HANDLER = 5,
    WD_INTERRUPT_PREEMPT_PRIORITY_HIGH_HANDLER = 6,
    WD_INTERRUPT_PREEMPT_PRIORITY_ABOVE_NORMAL_HANDLER = 7,
    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER = 8,
    WD_INTERRUPT_PREEMPT_PRIORITY_BELOW_NORMAL_HANDLER = 9,
    WD_INTERRUPT_PREEMPT_PRIORITY_LOW_HANDLER = 10,
    WD_INTERRUPT_PREEMPT_PRIORITY_UNUSED_HANDLER = 15,
    WD_INTERRUPT_PREEMPT_PRIORITY_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_interrupt_preempt_priority_t;

typedef enum {
    WD_INTERRUPT_SUB_PRIORITY_REALTIME_HANDLER = 0,
    WD_INTERRUPT_SUB_PRIORITY_HIGH_HANDLER = 0,
    WD_INTERRUPT_SUB_PRIORITY_ABOVE_NORMAL_HANDLER = 0,
    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER = 0,
    WD_INTERRUPT_SUB_PRIORITY_BELOW_NORMAL_HANDLER = 0,
    WD_INTERRUPT_SUB_PRIORITY_LOW_HANDLER = 0,
    WD_INTERRUPT_SUB_PRIORITY_UNUSED_HANDLER = 0,
    WD_INTERRUPT_SUB_PRIORITY_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_interrupt_sub_priority_t;

typedef enum {
    WD_CODE_OK = 0,                 ///< Operation succeededbl
    WD_CODE_ERROR = -1,             ///< Unspecified error
    WD_CODE_ERROR_BUSY = -2,        ///< Driver is busy
    WD_CODE_ERROR_TIMEOUT = -3,     ///< Timeout occurred
    WD_CODE_ERROR_UNSUPPORTED = -4, ///< Operation not supported
    WD_CODE_ERROR_PARAMETER = -5,   ///< Parameter error
    WD_CODE_ERROR_SPECIFIC = -6,    ///< Start of driver specific errors

    WD_CODE_ERROR_CRC = -99,

    WD_CODE_EOF = -1, ///< End-of-file
    WD_CODE_ALL = -1,

    WD_CODE_NOP = 0,          ///< No Operation
    WD_CODE_PROCESSED = 101,  ///< Processed
    WD_CODE_PROCESSING = 102, ///< Processing
    WD_CODE_START = 103,      ///< Start
    WD_CODE_STOP = 104,       ///< Stop
    WD_CODE_INACTIVE = 105,   ///< Inactive
    WD_CODE_RUNNING = 106,    ///< Running
    WD_CODE_READY = 107,      ///< Ready
    WD_CODE_WAITING = 108,    ///< Waiting

    WD_CODE_ARGUMENT_MATCH = 0,
    WD_CODE_ARGUMENT_UNMATCH = -100,

    WD_CODE_IDLE = 0,
    WD_CODE_ACK = 1,
    WD_CODE_REQUEST = 2,
    WD_CODE_RESPONSE = 3,
    WD_CODE_NACK = -1,

    WD_CODE_TBD = 0,
    WD_CODE_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_code_t;

/* wd_code_t : single error code type of the WD Library (GEN-S-008) */

// 자체 드라이버 인터페이스(PowerControl 등)용 전원 상태 — 값은 CMSIS ARM_POWER_STATE와 동일하게 유지
typedef enum {
    WD_POWER_OFF = 0, ///< Power off: no operation possible
    WD_POWER_LOW = 1, ///< Low Power mode: retain state, detect and signal wake-up events
    WD_POWER_FULL = 2 ///< Power on: full operation at maximum performance
} wd_power_state_t;

typedef enum {
    WD_STATUS_OK = 0,
    WD_STATUS_ERROR = 1,

    WD_STATUS_OFF = 0,
    WD_STATUS_ON = 1,
    WD_STATUS_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_status_t;

typedef enum {
    WD_STATE_INITIALIZED = (1UL << 0),
    WD_STATE_POWERED = (1UL << 1),
    WD_STATE_CONFIGURED = (1UL << 2),
    WD_STATE_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의

} wd_state_t; // 1 Byte [ 0: 7]

typedef enum {
    WD_METHOD_POLLING = (1UL << 0),
    WD_METHOD_INTERRUPT = (1UL << 1),
    WD_METHOD_DMA = (1UL << 2),
    WD_METHOD_BLOCKING = (1UL << 7),
    WD_METHOD_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_method_t;

#define WD_METHOD_POLLING   (1UL << 0)
#define WD_METHOD_INTERRUPT (1UL << 1)
#define WD_METHOD_DMA       (1UL << 2)
#define WD_METHOD_BLOCKING  (1UL << 7)

typedef enum {
    WD_PERIPHERAL_NONE = 0,
    /*--------------- CMSIS ---------------*/
    WD_PERIPHERAL_CAN = 1,
    WD_PERIPHERAL_ETHERNET = 2,
    WD_PERIPHERAL_I2C = 3,
    WD_PERIPHERAL_MCI = 4,
    WD_PERIPHERAL_NAND = 5,
    WD_PERIPHERAL_FLASH = 6,
    WD_PERIPHERAL_SAI = 7,
    WD_PERIPHERAL_SPI = 8,
    WD_PERIPHERAL_STORAGE = 9,
    WD_PERIPHERAL_USART = 10,
    WD_PERIPHERAL_USB = 11,
    WD_PERIPHERAL_VIO = 12,
    WD_PERIPHERAL_WIFI = 13,

    /*--------------- Specific ---------------*/
    WD_PERIPHERAL_ADC = 20,
    WD_PERIPHERAL_DAC = 21,
    WD_PERIPHERAL_OPAMP = 22,
    WD_PERIPHERAL_COMP = 23,
    WD_PERIPHERAL_QSPI = 24,
    WD_PERIPHERAL_I2C3 = 25,
    WD_PERIPHERAL_SDMMC = 26,
    WD_PERIPHERAL_RNG = 27,
    WD_PERIPHERAL_AES = 28,
    WD_PERIPHERAL_HASH = 29,
    WD_PERIPHERAL_CRC = 30,
    WD_PERIPHERAL_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_peripheral_t;

#define WD_PERIPHERAL_NONE 0
/*--------------- CMSIS ---------------*/
#define WD_PERIPHERAL_CAN      1
#define WD_PERIPHERAL_ETHERNET 2
#define WD_PERIPHERAL_I2C      3
#define WD_PERIPHERAL_MCI      4
#define WD_PERIPHERAL_NAND     5
#define WD_PERIPHERAL_FLASH    6
#define WD_PERIPHERAL_SAI      7
#define WD_PERIPHERAL_SPI      8
#define WD_PERIPHERAL_STORAGE  9
#define WD_PERIPHERAL_USART    10
#define WD_PERIPHERAL_USB      11
#define WD_PERIPHERAL_VIO      12
#define WD_PERIPHERAL_WIFI     13

/*--------------- Specific ---------------*/
#define WD_PERIPHERAL_ADC   20
#define WD_PERIPHERAL_DAC   21
#define WD_PERIPHERAL_OPAMP 22
#define WD_PERIPHERAL_COMP  23
#define WD_PERIPHERAL_QSPI  24
#define WD_PERIPHERAL_I2C3  25
#define WD_PERIPHERAL_SDMMC 26
#define WD_PERIPHERAL_RNG   27
#define WD_PERIPHERAL_AES   28
#define WD_PERIPHERAL_HASH  29
#define WD_PERIPHERAL_CRC   30

typedef enum {
    /*
	 ------------------------------------------------------------------------------------------
	 Event Recorder Component Number
	 ------------------------------------------------------------------------------------------
	 0x00 .. 0x3F | (0   .. 63)  | Software components of the user application
	 0x40 .. 0x7F | (64  .. 127) | Third-party middleware components
	 0x80 .. 0xED | (128 .. 237) | MDK-Middleware components
	 0xEE         | (238)        | Fault component
	 0xEF         | (239)        | Start/Stop events for Event Statistic information
	 0xF0 .. 0xFC | (240 .. 253) | RTOS kernel
	 0xFD         | (253)        | Inter-process communication layer (multiprocessor systems)
	 0xFE         | (254)        | Printf-style debug outputs
	 0xFF         | (255)        | Event Recorder messages
	 ------------------------------------------------------------------------------------------
	 */
    // User components -------------------------------------------------------------0x0 .. 0x3F (0 .. 63) software components of the user application
    /*---------------  Reserved         0      ---------------*/
    WD_SYMBOL_NONE = 0,

    /*---------------  System           1 ~  4 ---------------*/
    WD_SYMBOL_SYSTEM = 1,
    WD_SYMBOL_ERROR,
    WD_SYMBOL_DEBUG,

    /*---------------  06_Hardware      5 ~  8 ---------------*/
    WD_SYMBOL_06_HARDWARE = 5,
    WD_SYMBOL_06_POWER,
    /*---------------  05_Secure        9      ---------------*/
    /*---------------  04_Software     10 ~ 14 ---------------*/
    WD_SYMBOL_04_PLATFORM = 10,
    WD_SYMBOL_04_RTOS,
    WD_SYMBOL_04_SEGGER,
    /*---------------  03_Component    15 ~ 49 ---------------*/
    WD_SYMBOL_03_ADC = 15,
    WD_SYMBOL_03_CLI,
    WD_SYMBOL_03_DDS,
    WD_SYMBOL_03_DIGIPOT,
    WD_SYMBOL_03_DISPLAY,
    WD_SYMBOL_03_INTERFACE_BLUETOOTH,
    WD_SYMBOL_03_INTERFACE_CAN,
    WD_SYMBOL_03_INTERFACE_ETHERNET,
    WD_SYMBOL_03_INTERFACE_RS422,
    WD_SYMBOL_03_INTERFACE_WIFI,
    WD_SYMBOL_03_IO_EXPANDER,
    WD_SYMBOL_03_NETWORK,
    WD_SYMBOL_03_NVM_NAND,
    WD_SYMBOL_03_NVM_NOR,
    WD_SYMBOL_03_SDRAM,
    WD_SYMBOL_03_SENSOR_TEMPERATURE,

    WD_SYMBOL_03_PROTOCOL = 35,
    WD_SYMBOL_03_PROTOCOL_USER,
    WD_SYMBOL_03_PROTOCOL_RESERVED_1,
    WD_SYMBOL_03_PROTOCOL_MZ_MANAGER,
    WD_SYMBOL_03_PROTOCOL_TELNET,
    WD_SYMBOL_03_PROTOCOL_MODBUS,
    WD_SYMBOL_03_PROTOCOL_Z_MODEM,
    WD_SYMBOL_03_PROTOCOL_SCPI,
    WD_SYMBOL_03_PROTOCOL_VXI_11,
    WD_SYMBOL_03_PROTOCOL_HISLIP,
    WD_SYMBOL_03_PROTOCOL_USBTMC,

    /*---------------  02_Service      50 ~ 59 ---------------*/
    WD_SYMBOL_02_NET_BLUETOOTH = 50,
    WD_SYMBOL_02_NET_CAN,
    WD_SYMBOL_02_NET_ETHERNET,
    WD_SYMBOL_02_NET_RS422,
    WD_SYMBOL_02_NET_WIFI,

/*---------------  01_Application  60 ~ 62 ---------------*/
/*---------------  00_Bootloader   63      ---------------*/

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
    WD_SYMBOL_ALL = (int)0x7FFFFFFF,
#elif defined(__ICCARM__)
    WD_SYMBOL_ALL = 0x7FFFFFFF,
#endif
} wd_system_symbol_t;

typedef struct {
    uint8_t b0 : 1, // WD_BIT0
        b1 : 1,     // WD_BIT1
        b2 : 1,     // WD_BIT2
        b3 : 1,     // WD_BIT3
        b4 : 1,     // WD_BIT4
        b5 : 1,     // WD_BIT5
        b6 : 1,     // WD_BIT6
        b7 : 1;     // WD_BIT7
} uint1_t;

typedef struct {
    uint8_t addr[4]; ///< IPv4 address value used in \ref vioSetIPv4 / \ref vioGetIPv4
} wd_i_pv4_t;

typedef struct {
    uint8_t addr[16]; ///< IPv6 address value used in \ref vioSetIPv6 / \ref vioGetIPv6
} wd_i_pv6_t;

#if 0
typedef struct
{
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t  Data4[8];
} wd_guid_t;

typedef struct
{
  uint32_t Time_Low;
  uint16_t Time_Mid;
  uint16_t Time_Hi_And_Version;
  uint8_t  Clock_Seq_Hi_And_Reserved;
  uint8_t  Clock_Seq_Low;
  uint8_t  Node[6];
} wd_uuid_t;


typedef struct
{
  uint64_t Signature;
  uint32_t Revision;
  uint32_t HeaderSize;
  uint32_t wd_toolkit_crc32;
} wd_table_header_t;
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef ENABLE
#define ENABLE  1U
#define DISABLE 0U
#endif

#ifndef HIGH
#define HIGH 1U
#define LOW  0U
#endif

#ifndef SET
#define SET      1U
#define WD_CLEAR 0U
#endif

#define OFF 0U
#define ON  1U

#define WD_UNLOCK 0U
#define LOCK      1U

#define WD_DIR_LEFT  0U
#define WD_DIR_RIGHT 1U

#define WD_DIR_FIFO 0U
#define WD_DIR_LIFO 1U

#define WD_DIR_HALF 0U
#define WD_DIR_FULL 1U

#define WD_FULL_DUPLEX 1U
#define WD_HALF_DUPLEX 0U

typedef enum { /*--------------- Type     : [ 0: 7] ---------------*/
#define WD_NET_TYPE_POS 0
#define WD_NET_TYPE_MSK (0xFFUL << WD_NET_TYPE_POS)
               WD_NET_TX = ((1UL) << WD_NET_TYPE_POS),
               WD_NET_RX = ((2UL) << WD_NET_TYPE_POS),
               WD_NET_RX_LOOP_BACK = ((3UL) << WD_NET_TYPE_POS),
               WD_NET_ERROR = ((4UL) << WD_NET_TYPE_POS),

/*--------------- Argument : [16:31] ---------------*/
#define WD_NET_ARGUMENT_POS 8
#define WD_NET_ARGUMENT_MSK (0xFFFFFFUL << WD_NET_ARGUMENT_POS)
#define WD_NET_ARGUMENT(n)  ((n) << WD_NET_ARGUMENT_POS)
               WD_NET_OPTION_HEX = ((0UL) << WD_NET_ARGUMENT_POS),
               WD_NET_OPTION_ASCII = ((1UL) << WD_NET_ARGUMENT_POS),

               /*--------------- Pre-defined ---------------*/
               WD_NET_TX_ASCII = WD_NET_TX | WD_NET_OPTION_ASCII,
               WD_NET_RX_ASCII = WD_NET_RX | WD_NET_OPTION_ASCII,

} wd_net_control_t;

#ifndef NULL
#define NULL 0U
#endif

#ifndef WD_BIT0
#define WD_BIT0 0x01U
#define WD_BIT1 0x02U
#define WD_BIT2 0x04U
#define WD_BIT3 0x08U
#define WD_BIT4 0x10U
#define WD_BIT5 0x20U
#define WD_BIT6 0x40U
#define WD_BIT7 0x80U
#endif

#ifndef WD_BIT8
#define WD_BIT8  0x0100U
#define WD_BIT9  0x0200U
#define WD_BIT10 0x0400U
#define WD_BIT11 0x0800U
#define WD_BIT12 0x1000U
#define WD_BIT13 0x2000U
#define WD_BIT14 0x4000U
#define WD_BIT15 0x8000U
#endif

#ifndef WD_BIT16
#define WD_BIT16   0x010000U
#define WD_BIT17   0x020000U
#define WD_BIT18   0x040000U
#define WD_BIT19   0x080000U
#define WD_BIT20   0x100000U
#define WD_BIT21   0x200000U
#define WD_BIT22   0x400000U
#define WD_BIT23   0x800000U
#define WD_BIT24   0x01000000U
#define WD_BIT25   0x02000000U
#define WD_BIT26   0x04000000U
#define WD_BIT27   0x08000000U
#define WD_BIT28   0x10000000U
#define WD_BIT29   0x20000000U
#define WD_BIT30   0x40000000U
#define WD_BIT31   0x80000000U
#define WD_BIT_ALL 0xFFFFFFFFU
#endif

#define WD_MATH_PI                ((fpu_t)3.14159265358979)
#define WD_MATH_ROOT_2            ((fpu_t)1.4142135623730950488016887242097)
#define WD_MATH_ROOT_3            ((fpu_t)1.7320508075688772935274463415059)
#define WD_MATH_RADIAN_180_DIV_PI ((fpu_t)57.2957795130823)
#define WD_MATH_RADIAN_PI_DIV_180 ((fpu_t)0.017453292519943)
#define WD_MATH_MUL_RAD0                                                        ((fpu_t)(0.52359877559829887307710723054658)
#define WD_MATH_MUL_RAD1 ((fpu_t)(-0.52359877559829887307710723054658))

#define WD_ANSI                  "\x1b["
#define WD_ANSI_CLEAR            "\r\n\x1b[2J\x1b[H"
#define WD_COLOR_RED             "\x1b[0;31m"
#define WD_COLOR_GREEN           "\x1b[0;32m"
#define WD_COLOR_YELLOW          "\x1b[0;33m"
#define WD_COLOR_BLUE            "\x1b[0;34m"
#define WD_COLOR_MAGENTA         "\x1b[0;35m"
#define WD_COLOR_CYAN            "\x1b[0;36m"
#define WD_COLOR_WHITE           "\x1b[0;37m"
#define WD_COLOR_RED_BOLD        "\x1b[1;31m"
#define WD_COLOR_GREEN_BOLD      "\x1b[1;32m"
#define WD_COLOR_YELLOW_BOLD     "\x1b[1;33m"
#define WD_COLOR_BLUE_BOLD       "\x1b[1;34m"
#define WD_COLOR_MAGENTA_BOLD    "\x1b[1;35m"
#define WD_COLOR_CYAN_BOLD       "\x1b[1;36m"
#define WD_COLOR_WHITE_BOLD      "\x1b[1;37m"
#define WD_COLOR_RED             "\x1b[0;31m"
#define WD_COLOR_BG_RED          "\x1b[0;41m"
#define WD_COLOR_BG_GREEN        "\x1b[0;42m"
#define WD_COLOR_BG_YELLOW       "\x1b[0;43m"
#define WD_COLOR_BG_BLUE         "\x1b[0;44m"
#define WD_COLOR_BG_MAGENTA      "\x1b[0;45m"
#define WD_COLOR_BG_CYAN         "\x1b[0;46m"
#define WD_COLOR_BG_WHITE        "\x1b[0;47m"
#define WD_COLOR_BG_RED_BOLD     "\x1b[1;41m"
#define WD_COLOR_BG_GREEN_BOLD   "\x1b[1;42m"
#define WD_COLOR_BG_YELLOW_BOLD  "\x1b[1;43m"
#define WD_COLOR_BG_BLUE_BOLD    "\x1b[1;44m"
#define WD_COLOR_BG_MAGENTA_BOLD "\x1b[1;45m"
#define WD_COLOR_BG_CYAN_BOLD    "\x1b[1;46m"
#define WD_COLOR_BG_WHITE_BOLD   "\x1b[1;47m"
//#define WD_COLOR_RESET                                                           "\x1b[0;30m\x1b[0;40m"
#define WD_COLOR_RESET "\x1b[0m"
#define WD_COLOR_ERROR "\x1b[1;37m\x1b[1;41m"
#define WD_COLOR_TIME  "\x1b[1;33m\x1b[1;44m"

#define WD_ANSI_MSG_NETWORK_TX           WD_COLOR_BLUE_BOLD
#define WD_ANSI_MSG_NETWORK_RX           WD_COLOR_GREEN_BOLD
#define WD_ANSI_MSG_NETWORK_ERROR        WD_COLOR_RED_BOLD
#define WD_ANSI_MSG_NETWORK_RX_LOOP_BACK WD_COLOR_CYAN_BOLD

#define WD_COLOR_RGB_R WD_BIT0
#define WD_COLOR_RGB_G WD_BIT1
#define WD_COLOR_RGB_B WD_BIT2
// (R,G,B)
#define WD_COLOR_RGB_BLACK   0                                                  // (0,0,0)
#define WD_COLOR_RGB_RED     WD_COLOR_RGB_R                                     // (255,0,0)
#define WD_COLOR_RGB_GREEN   WD_COLOR_RGB_G                                     // (0,255,0)
#define WD_COLOR_RGB_BLUE    WD_COLOR_RGB_B                                     // (0,0,255)
#define WD_COLOR_RGB_YELLOW  (WD_COLOR_RGB_R | WD_COLOR_RGB_G)                  // (255,255,0)
#define WD_COLOR_RGB_CYAN    (WD_COLOR_RGB_G | WD_COLOR_RGB_B)                  // (0,255,255)
#define WD_COLOR_RGB_MAGENTA (WD_COLOR_RGB_R | WD_COLOR_RGB_B)                  // (255,0,255)
#define WD_COLOR_RGB_WHITE   (WD_COLOR_RGB_R | WD_COLOR_RGB_G | WD_COLOR_RGB_B) // (255,0,255)
#define WD_COLOR_RGB_MASK    (WD_COLOR_RGB_R | WD_COLOR_RGB_G | WD_COLOR_RGB_B)

/*
 Black                 #000000    (0,0,0)
 White                 #FFFFFF    (255,255,255)
 Red                   #FF0000    (255,0,0)
 Lime                  #00FF00    (0,255,0)
 Blue                  #0000FF    (0,0,255)
 Yellow                #FFFF00    (255,255,0)
 Cyan / Aqua           #00FFFF    (0,255,255)
 Magenta / Fuchsia     #FF00FF    (255,0,255)
 Silver                #C0C0C0    (192,192,192)
 Gray                  #808080    (128,128,128)
 Maroon                #800000    (128,0,0)
 Olive                 #808000    (128,128,0)
 Green                 #008000    (0,128,0)
 Purple                #800080    (128,0,128)
 Teal                  #008080    (0,128,128)
 Navy                  #000080    (0,0,128)
 */

#define WD_ANSI_MSG_OK    "\x1b[1;32mOK\x1b[0m"
#define WD_ANSI_MSG_ERROR "\x1b[1;31mERROR\x1b[0m"

#if 1
#if defined(__clang__)
#define WD_NAN                                                          \
    _Pragma("clang diagnostic push")                                    \
        _Pragma("clang diagnostic ignored \"-Wnan-infinity-disabled\"") \
            NAN _Pragma("clang diagnostic pop")
#else
#define WD_NAN NAN
#endif
#else
#define WD_NAN 0xFFFFFFFFu
#endif
#define WD_NAN_MAX   0x7FFFFFFFu // maximum normal number
#define WD_NAN_INF_P 0x7F800000u // Plus infinity
#define WD_NAN_INF_N 0xFF800000u // Minus infinity
#define WD_NAN_NAN_S 0x7F800001u // Signaling NaN
#define WD_NAN_NAN_Q 0x7FC00000u // Quiet NaN

#define WD_IS_ALIGNED(x, a) (((x) & ((typeof(x))(a) - 1)) == 0)

#define WD_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define WD_FILENAME        (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define WD_PASTE_IMPL(a, b) a##b
#ifndef WD_PASTE
#define WD_PASTE(a, b) WD_PASTE_IMPL(a, b)
#endif

#define WD_PASTE3_IMPL(a, b, c) a##b##c
#define WD_PASTE3(a, b, c)      WD_PASTE3_IMPL(a, b, c)

#define WD_PASTE4_IMPL(a, b, c, d) a##b##c##d
#define WD_PASTE4(a, b, c, d)      WD_PASTE4_IMPL(a, b, c, d)

#define WD_MIN(a, b)              (((a) < (b)) ? (a) : (b))
#define WD_MAX(a, b)              (((a) > (b)) ? (a) : (b))
#define WD_RANGE(min, value, max) ((min <= value) && (value <= max))

#define WD_RANDOM(__min__, __max__)                                             \
    ((int)(((double)((rand() << 15) | (rand() & 0) | rand())) /                 \
           ((RAND_MAX << 15 | RAND_MAX) + 1) * (((__max__) + 1) - (__min__))) + \
     (__min__))

#define WD_STRINGIFY_IMPL(a) #a
#define WD_STRINGIFY(a)      WD_STRINGIFY_IMPL(a)

#define WD_CONVERT_BIN2BCD(__VALUE__) (uint8_t)((((__VALUE__) / 10U) << 4U) | ((__VALUE__) % 10U))
#define WD_CONVERT_BCD2BIN(__VALUE__)                                            \
    (uint8_t)(((uint8_t)((__VALUE__) & (uint8_t)0xF0U) >> (uint8_t)0x4U) * 10U + \
              ((__VALUE__) & (uint8_t)0x0FU))

#define WD_COUNT_VA_ARGS(...) \
    WD_GET_NTH_ARG(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define WD_GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, N, ...) N

#define WD_FIELD_SIZEOF(t, f)    (sizeof(((t *)0)->f))
#define WD_FIELD_OFFSET(t, e, s) ((uint32_t)(&((t *)0)->e) - (uint32_t)(&((t *)0)->s))

#define WD_MEM_SIZE_MEMORYPOOL \
    (block_count, block_size)(4 * (block_count) * (((block_size) + 3) / 4))
#define WD_MEM_SIZE_MESSAGEQUEUE(msg_count, msg_size) \
    (4 * (msg_count) * (3 + (((msg_size) + 3) / 4)))

#define WD_ROOT_2            (fpu_t)(1.4142135623730950488016887242097)
#define WD_ROOT_3            (fpu_t)(1.7320508075688772935274463415059)
#define WD_RADIAN_180_DIV_PI (fpu_t)(57.2957795130823)
#define WD_RADIAN_PI_DIV_180 (fpu_t)(0.017453292519943)
#define WD_MUL_RAD0          (fpu_t)(0.52359877559829887307710723054658)
#define WD_MUL_RAD1          (fpu_t)(-0.52359877559829887307710723054658)

#ifndef NDEBUG
#define WD_BREAK()
//#define WD_BREAK()                                                               if(WD_CORE_IS_IN_DEBUG()) __ASM("BKPT #0\n")
#else
#if NDEBUG_SUPPORT_BREAK
#define WD_BREAK()             \
    if (WD_CORE_IS_IN_DEBUG()) \
    __ASM("BKPT #0\n")
#else
#define WD_BREAK()
#endif
#endif

#ifndef NDEBUG
#define WD_CHECK(...)                                         \
    {                                                         \
        int32_t ret_check;                                    \
        ret_check = __VA_ARGS__;                              \
        if (ret_check < 0) {                                  \
            WD_LOG_ERROR("Return is Not OK [%d]", ret_check); \
        }                                                     \
    }                                                         \
    (void)0

#define WD_TEST(test_procedure) \
    WD_LOG_DEBUG(#test_procedure "   \x1b[1;35m[%X]\x1b[0m", test_procedure)

#else
#define WD_CHECK(...)           (void)__VA_ARGS__
#define WD_TEST(test_procedure) test_procedure
#endif

#define WD_CORE_IS_PRIVILEGED() ((__get_CONTROL() & 1U) == 0U)
#define WD_CORE_IS_IRQ_MODE()   (__get_IPSR() != 0U)
#define WD_CORE_IS_IRQ_MASKED() \
    ((__get_PRIMASK() != 0U) || (__get_BASEPRI() != 0U)) // (__get_PRIMASK() != 0U)
#define WD_CORE_IS_IN_ISR()   (WD_CORE_IS_IRQ_MODE() || WD_CORE_IS_IRQ_MASKED())
#define WD_CORE_IS_IN_DEBUG() (DCB->DHCSR & DCB_DHCSR_C_DEBUGEN_Msk)

/***************************************************************************************************************************************************************
 **********  Preprocessor
 **************************************************************************************************************************************************************/
#ifndef NDEBUG
#define WD_WHILE_REPORT() \
    {                     \
        WD_LOG_ERROR();   \
    }                     \
    (void)0

#define WD_WAIT_WHILE(condition, time_us)                   \
    {                                                       \
        uint32_t __time_start = wd_time_get_us();           \
        if (time_us > 8000000) {                            \
            WD_LOG_ERROR();                                 \
        }                                                   \
        while (condition) {                                 \
            if (wd_time_calc_us(&__time_start) > time_us) { \
                WD_WHILE_REPORT();                          \
                break;                                      \
            }                                               \
        }                                                   \
    }                                                       \
    (void)0

#define WD_WHILE_RET(condition, time_us, result)            \
    {                                                       \
        uint32_t __time_start = wd_time_get_us();           \
        if (time_us > 8000000) {                            \
            WD_LOG_ERROR();                                 \
        }                                                   \
        while (condition) {                                 \
            if (wd_time_calc_us(&__time_start) > time_us) { \
                result = WD_CODE_ERROR_TIMEOUT;             \
                WD_WHILE_REPORT();                          \
                break;                                      \
            }                                               \
        }                                                   \
    }                                                       \
    (void)0

#else
#define WD_WHILE_REPORT()
#define WD_WAIT_WHILE(condition, time_us)                   \
    {                                                       \
        uint32_t __time_start = wd_time_get_us();           \
        while (condition) {                                 \
            if (wd_time_calc_us(&__time_start) > time_us) { \
                break;                                      \
            }                                               \
        }                                                   \
    }                                                       \
    (void)0

#define WD_WHILE_RET(condition, time_us, result)            \
    {                                                       \
        uint32_t __time_start = wd_time_get_us();           \
        while (condition) {                                 \
            if (wd_time_calc_us(&__time_start) > time_us) { \
                result = WD_CODE_ERROR_TIMEOUT;             \
                break;                                      \
            }                                               \
        }                                                   \
    }                                                       \
    (void)0
#endif

#define WD_DO_WHILE(condition, time_us, job, job_time)      \
    {                                                       \
        uint32_t __time_start = wd_time_get_us();           \
        if (time_us > 8000000) {                            \
            WD_LOG_ERROR();                                 \
        }                                                   \
        while ((condition)) {                               \
            job;                                            \
            wd_time_wait_us(job_time);                      \
            if (wd_time_calc_us(&__time_start) > time_us) { \
                WD_WHILE_REPORT();                          \
                break;                                      \
            }                                               \
        }                                                   \
    }                                                       \
    (void)0

#define WD_WHILE_MS(condition, time_ms)                     \
    {                                                       \
        uint32_t __time_start = wd_time_get_ms();           \
        if (time_ms > 8000000) {                            \
            WD_LOG_ERROR();                                 \
        }                                                   \
        while (condition) {                                 \
            if (wd_time_calc_ms(&__time_start) > time_ms) { \
                WD_WHILE_REPORT();                          \
                break;                                      \
            }                                               \
        }                                                   \
    }                                                       \
    (void)0

#define WD_WHILE_RET_MS(condition, time_ms, result)         \
    {                                                       \
        uint32_t __time_start = wd_time_get_ms();           \
        if (time_ms > 8000000) {                            \
            WD_LOG_ERROR();                                 \
        }                                                   \
        while (condition) {                                 \
            if (wd_time_calc_ms(&__time_start) > time_ms) { \
                result = WD_CODE_ERROR_TIMEOUT;             \
                WD_WHILE_REPORT();                          \
                break;                                      \
            }                                               \
        }                                                   \
    }                                                       \
    (void)0

/***************************************************************************************************************************************************************
 **********  errno.h
 **************************************************************************************************************************************************************/
#define EPERM   1  /* Operation not permitted */
#define ENOENT  2  /* No such file or directory */
#define ESRCH   3  /* No such process */
#define EINTR   4  /* Interrupted system call */
#define EIO     5  /* I/O error */
#define ENXIO   6  /* No such device or address */
#define E2BIG   7  /* Argument list too long */
#define ENOEXEC 8  /* Exec format error */
#define EBADF   9  /* Bad file number */
#define ECHILD  10 /* No child processes */
#define EAGAIN  11 /* Try again */
#define ENOMEM  12 /* Out of memory */
#define EACCES  13 /* Permission denied */
#define EFAULT  14 /* Bad address */
#define ENOTBLK 15 /* Block device required */
#define EBUSY   16 /* Device or resource busy */
#define EEXIST  17 /* File exists */
#define EXDEV   18 /* Cross-device link */
#define ENODEV  19 /* No such device */
#define ENOTDIR 20 /* Not a directory */
#define EISDIR  21 /* Is a directory */
#define EINVAL  22 /* Invalid argument */
#define ENFILE  23 /* File table overflow */
#define EMFILE  24 /* Too many open files */
#define ENOTTY  25 /* Not a typewriter */
#define ETXTBSY 26 /* Text file busy */
#define EFBIG   27 /* File too large */
#define ENOSPC  28 /* No space left on device */
#define ESPIPE  29 /* Illegal seek */
#define EROFS   30 /* Read-only file system */
#define EMLINK  31 /* Too many links */
#define EPIPE   32 /* Broken pipe */
#define EDOM    33 /* Math argument out of domain of func */
#define ERANGE  34 /* Math result not representable */

#define EDEADLK      35 /* Resource deadlock would occur */
#define ENAMETOOLONG 36 /* File name too long */
#define ENOLCK       37 /* No record locks available */

#define ENOSYS 38 /* Invalid system call number */

#define ENOTEMPTY   39     /* Directory not empty */
#define ELOOP       40     /* Too many symbolic links encountered */
#define EWOULDBLOCK EAGAIN /* Operation would block */
#define ENOMSG      42     /* No message of desired type */
#define EIDRM       43     /* Identifier removed */
#define ECHRNG      44     /* Channel number out of range */
#define EL2NSYNC    45     /* Level 2 not synchronized */
#define EL3HLT      46     /* Level 3 halted */
#define EL3RST      47     /* Level 3 reset */
#define ELNRNG      48     /* Link number out of range */
#define EUNATCH     49     /* Protocol driver not attached */
#define ENOCSI      50     /* No CSI structure available */
#define EL2HLT      51     /* Level 2 halted */
#define EBADE       52     /* Invalid exchange */
#define EBADR       53     /* Invalid request descriptor */
#define EXFULL      54     /* Exchange full */
#define ENOANO      55     /* No anode */
#define EBADRQC     56     /* Invalid request code */
#define EBADSLT     57     /* Invalid slot */

#define EDEADLOCK EDEADLK

#define EBFONT          59  /* Bad font file format */
#define ENOSTR          60  /* Device not a stream */
#define ENODATA         61  /* No data available */
#define ETIME           62  /* Timer expired */
#define ENOSR           63  /* Out of streams resources */
#define ENONET          64  /* Machine is not on the network */
#define ENOPKG          65  /* Package not installed */
#define EREMOTE         66  /* Object is remote */
#define ENOLINK         67  /* Link has been severed */
#define EADV            68  /* Advertise error */
#define ESRMNT          69  /* Srmount error */
#define ECOMM           70  /* Communication error on send */
#define EPROTO          71  /* Protocol error */
#define EMULTIHOP       72  /* Multihop attempted */
#define EDOTDOT         73  /* RFS specific error */
#define EBADMSG         74  /* Not a data message */
#define EOVERFLOW       75  /* Value too large for defined data type */
#define ENOTUNIQ        76  /* Name not unique on network */
#define EBADFD          77  /* File descriptor in bad state */
#define EREMCHG         78  /* Remote address changed */
#define ELIBACC         79  /* Can not access a needed shared library */
#define ELIBBAD         80  /* Accessing a corrupted shared library */
#define ELIBSCN         81  /* .lib section in a.out corrupted */
#define ELIBMAX         82  /* Attempting to link in too many shared libraries */
#define ELIBEXEC        83  /* Cannot exec a shared library directly */
#define EILSEQ          84  /* Illegal byte sequence */
#define ERESTART        85  /* Interrupted system call should be restarted */
#define ESTRPIPE        86  /* Streams pipe error */
#define EUSERS          87  /* Too many users */
#define ENOTSOCK        88  /* Socket operation on non-socket */
#define EDESTADDRREQ    89  /* Destination address required */
#define EMSGSIZE        90  /* Message too long */
#define EPROTOTYPE      91  /* Protocol wrong type for socket */
#define ENOPROTOOPT     92  /* Protocol not available */
#define EPROTONOSUPPORT 93  /* Protocol not supported */
#define ESOCKTNOSUPPORT 94  /* Socket type not supported */
#define EOPNOTSUPP      95  /* Operation not supported on transport endpoint */
#define EPFNOSUPPORT    96  /* Protocol family not supported */
#define EAFNOSUPPORT    97  /* Address family not supported by protocol */
#define EADDRINUSE      98  /* Address already in use */
#define EADDRNOTAVAIL   99  /* Cannot assign requested address */
#define ENETDOWN        100 /* Network is down */
#define ENETUNREACH     101 /* Network is unreachable */
#define ENETRESET       102 /* Network dropped connection because of reset */
#define ECONNABORTED    103 /* Software caused connection abort */
#define ECONNRESET      104 /* Connection reset by peer */
#define ENOBUFS         105 /* No buffer space available */
#define EISCONN         106 /* Transport endpoint is already connected */
#define ENOTCONN        107 /* Transport endpoint is not connected */
#define ESHUTDOWN       108 /* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS    109 /* Too many references: cannot splice */
#define ETIMEDOUT       110 /* Connection timed out */
#define ECONNREFUSED    111 /* Connection refused */
#define EHOSTDOWN       112 /* Host is down */
#define EHOSTUNREACH    113 /* No route to host */
#define EALREADY        114 /* Operation already in progress */
#define EINPROGRESS     115 /* Operation now in progress */
#define ESTALE          116 /* Stale file handle */
#define EUCLEAN         117 /* Structure needs cleaning */
#define ENOTNAM         118 /* Not a XENIX named type file */
#define ENAVAIL         119 /* No XENIX semaphores available */
#define EISNAM          120 /* Is a named type file */
#define EREMOTEIO       121 /* Remote I/O error */
#define EDQUOT          122 /* Quota exceeded */

#define ENOMEDIUM    123 /* No medium found */
#define EMEDIUMTYPE  124 /* Wrong medium type */
#define ECANCELED    125 /* Operation Canceled */
#define ENOKEY       126 /* Required key not available */
#define EKEYEXPIRED  127 /* Key has expired */
#define EKEYREVOKED  128 /* Key has been revoked */
#define EKEYREJECTED 129 /* Key was rejected by service */

/* for robust mutexes */
#define EOWNERDEAD      130 /* Owner died */
#define ENOTRECOVERABLE 131 /* State not recoverable */

#define ERFKILL 132 /* Operation not possible due to RF-kill */

#define EHWPOISON 133 /* Memory page has hardware error */

#define ERESTARTSYS              512
#define ERESTARTNOINTR           513
#define ERESTARTNOHAND           514 /* restart if no handler.. */
#define ENOIOCTLCMD              515 /* No ioctl command */
#define WD_ERESTART_RESTARTBLOCK 516 /* restart by calling sys_restart_syscall */
#define WD_EPROBE_DEFER          517 /* Driver requests probe retry */
#define EOPENSTALE               518 /* open found a stale dentry */

/* Defined for the NFSv3 protocol */
#define EBADHANDLE      521 /* Illegal NFS file handle */
#define ENOTSYNC        522 /* Update synchronization mismatch */
#define EBADCOOKIE      523 /* Cookie is stale */
#define ENOTSUPP        524 /* Operation is not supported */
#define ETOOSMALL       525 /* Buffer or request is too small */
#define ESERVERFAULT    526 /* An untranslatable error occurred */
#define EBADTYPE        527 /* Type not supported by server */
#define EJUKEBOX        528 /* Request initiated, but will not complete before timeout */
#define EIOCBQUEUED     529 /* iocb queued, will get completion event */
#define ERECALLCONFLICT 530 /* conflict with recalled state */

typedef enum {
    WD_ESUCCESS = 0,            //  0   For those rare times one needs to say all is OK
    WD_EMINERRORVAL = 1,        // The lowest valid error value
    WD_EPERM = WD_EMINERRORVAL, //  1   Operation not permitted
    WD_ENOENT,                  //  2   No such file or directory
    WD_ESRCH,                   //  3   No such process
    WD_EINTR,                   //  4   Interrupted system call
    WD_EIO,                     //  5   Input/output error
    WD_ENXIO,                   //  6   Device not configured
    WD_E2BIG,                   //  7   Argument list too long
    WD_ENOEXEC,                 //  8   Exec format error
    WD_EBADF,                   //  9   Bad file descriptor
    WD_ECHILD,                  // 10   No child processes
    WD_EDEADLK,                 // 11   Resource deadlock avoided
    WD_ENOMEM,                  // 12   Cannot allocate memory
    WD_EACCES,                  // 13   Permission denied
    WD_EFAULT,                  // 14   Bad address
    WD_ENOTBLK,                 // 15   Block device required
    WD_EBUSY,                   // 16   Device busy
    WD_EEXIST,                  // 17   File exists
    WD_EXDEV,                   // 18   Cross-device link
    WD_ENODEV,                  // 19   Operation not supported by device
    WD_ENOTDIR,                 // 20   Not a directory
    WD_EISDIR,                  // 21   Is a directory
    WD_EINVAL,                  // 22   Invalid argument
    WD_ENFILE,                  // 23   Too many open files in systems
    WD_EMFILE,                  // 24   Too many open file descriptors
    WD_ENOTTY,                  // 25   Inappropriate ioctl for device
    WD_ETXTBSY,                 // 26   Text file busy
    WD_EFBIG,                   // 27   File too large
    WD_ENOSPC,                  // 28   No space left on device
    WD_ESPIPE,                  // 29   Illegal seek
    WD_EROFS,                   // 30   Read-only filesystem
    WD_EMLINK,                  // 31   Too many links
    WD_EPIPE,                   // 32   Broken pipe

    /*--------------- math software -- these are the only two values required by the C Standard ---------------*/
    WD_EDOM,   // 33   Numerical argument out of domain
    WD_ERANGE, // 34   Result too large

    /*--------------- non-blocking and interrupt i/o ---------------*/
    WD_EAGAIN,                  // 35   Resource temporarily unavailable
    WD_EWOULDBLOCK = WD_EAGAIN, // 35   Operation would block
    WD_EINPROGRESS,             // 36   Operation now in progress
    WD_EALREADY,                // 37   Operation already in progress

    /*--------------- ipc/network software -- argument errors ---------------*/
    WD_ENOTSOCK,                // 38   Socket operation on non-socket
    WD_EDESTADDRREQ,            // 39   Destination address required
    WD_EMSGSIZE,                // 40   Message too long
    WD_EPROTOTYPE,              // 41   Protocol wrong type for socket
    WD_ENOPROTOOPT,             // 42   Protocol not available
    WD_EPROTONOSUPPORT,         // 43   Protocol not supported
    WD_ESOCKTNOSUPPORT,         // 44   Socket type not supported
    WD_EOPNOTSUPP,              // 45   Operation not supported
    WD_ENOTSUP = WD_EOPNOTSUPP, // 45   Operation not supported
    WD_EPFNOSUPPORT,            // 46   Protocol family not supported
    WD_EAFNOSUPPORT,            // 47   Address family not supported by protocol family
    WD_EADDRINUSE,              // 48   Address already in use
    WD_EADDRNOTAVAIL,           // 49   Can't assign requested address

    /*--------------- ipc/network software -- operational errors ---------------*/
    WD_ENETDOWN,     // 50   Network is down
    WD_ENETUNREACH,  // 51   Network is unreachable
    WD_ENETRESET,    // 52   Network dropped connection on reset
    WD_ECONNABORTED, // 53   Software caused connection abort
    WD_ECONNRESET,   // 54   Connection reset by peer
    WD_ENOBUFS,      // 55   No buffer space available
    WD_EISCONN,      // 56   Socket is already connected
    WD_ENOTCONN,     // 57   Socket is not connected
    WD_ESHUTDOWN,    // 58   Can't send after socket shutdown
    WD_ETOOMANYREFS, // 59   Too many references: can't splice
    WD_ETIMEDOUT,    // 60   Operation timed out
    WD_ECONNREFUSED, // 61   Connection refused
    WD_ELOOP,        // 62   Too many levels of symbolic links
    WD_ENAMETOOLONG, // 63   File name too long
    WD_EHOSTDOWN,    // 64   Host is down
    WD_EHOSTUNREACH, // 65   No route to host

    WD_ENOTEMPTY, // 66   Directory not empty

    /*--------------- quotas, etc. ---------------*/
    WD_EPROCLIM, // 67   Too many processes
    WD_EUSERS,   // 68   Too many users
    WD_EDQUOT,   // 69   Disc quota exceeded

    /*--------------- Network File System ---------------*/
    WD_ESTALE,        // 70   Stale NFS file handle
    WD_EREMOTE,       // 71   Too many levels of remote in path
    WD_EBADRPC,       // 72   RPC struct is bad
    WD_ERPCMISMATCH,  // 73   RPC version wrong
    WD_EPROGUNAVAIL,  // 74   RPC prog. not avail
    WD_EPROGMISMATCH, // 75   Program version wrong
    WD_EPROCUNAVAIL,  // 76   Bad procedure for program
    WD_ENOLCK,        // 77   No locks available
    WD_ENOSYS,        // 78   Function not implemented
    WD_EFTYPE,        // 79   Inappropriate file type or format
    WD_EAUTH,         // 80   Authentication error
    WD_ENEEDAUTH,     // 81   Need authenticator
    WD_EIDRM,         // 82   Identifier removed
    WD_ENOMSG,        // 83   No message of desired type
    WD_EOVERFLOW,     // 84   Value too large to be stored in data type
    WD_EILSEQ,        // 85   Illegal byte sequence
    WD_ENOTHING_1,    // 86   Place Holder
    WD_ECANCELED,     // 87   Operation canceled

    WD_EBADMSG, // 88   Bad message
    WD_ENODATA, // 89   No message available
    WD_ENOSR,   // 90   No STREAM resources
    WD_ENOSTR,  // 91   Not a STREAM
    WD_ETIME,   // 92   STREAM ioctl timeout

    WD_ENOATTR, // 93   Attribute not found

    WD_EDOOFUS, // 94   Programming error

    WD_EMULTIHOP, // 95   Multihop attempted
    WD_ENOLINK,   // 96   Link has been severed
    WD_EPROTO,    // 97   Protocol error

    WD_EBUFSIZE, // 98   Buffer too small to hold result

    WD_EMAXERRORVAL,                      // One more than the highest defined error value.
    WD_ERRNO_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_errno_t;

/***************************************************************************************************************************************************************
 **********  Compiler
 **************************************************************************************************************************************************************/
#if 0                 // _Appconfig.h에 GCC 컴파일러용으로 따로 선언함.
#if defined(__CC_ARM) // Arm Compiler 4/5
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) && \
    (__ARMCC_VERSION < 6100100) // Arm Compiler 6.6 LTM (armclang)
#elif defined(__ARMCC_VERSION) && \
    (__ARMCC_VERSION >= 6100100) // Arm Compiler above 6.10.1 (armclang)
#ifndef WD_UNALIGNED_FLOAT_WRITE
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpacked"
/*lint -esym(9058, T_FLOAT_WRITE)*/ /* disable MISRA 2012 Rule 2.4 for T_FLOAT_WRITE */
  __PACKED_STRUCT T_FLOAT_WRITE { float v; };
#pragma clang diagnostic pop
#define WD_UNALIGNED_FLOAT_WRITE(addr, val) \
    (void)((((struct T_FLOAT_WRITE *)(void *)(addr))->v) = (val))
#endif
#ifndef WD_UNALIGNED_FLOAT_READ
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpacked"
/*lint -esym(9058, T_FLOAT_READ)*/ /* disable MISRA 2012 Rule 2.4 for T_FLOAT_READ */
  __PACKED_STRUCT T_FLOAT_READ { float v; };
#pragma clang diagnostic pop
#define WD_UNALIGNED_FLOAT_READ(addr) (((const struct T_FLOAT_READ *)(const void *)(addr))->v)
#endif
#elif defined(__GNUC__)   // GNU Compiler
#elif defined(__ICCARM__) // IAR Compiler
#ifndef WD_UNALIGNED_FLOAT_READ
#pragma language = save
#pragma language = extended
  _Pragma("inline=forced") __intrinsic float __iar_float_read(void const *ptr)
{
  return *(__packed float*)(ptr);
}
#pragma language = restore
#define WD_UNALIGNED_FLOAT_READ(PTR) __iar_float_read(PTR)
#endif

#ifndef WD_UNALIGNED_FLOAT_WRITE
#pragma language = save
#pragma language = extended
  _Pragma("inline=forced") __intrinsic void __iar_float_write(void const *ptr, float val)
{
  *(__packed float*)(ptr) = val;;
}
#pragma language = restore
#define WD_UNALIGNED_FLOAT_WRITE(PTR, VAL) __iar_float_write(PTR, VAL)
#endif
#endif
#endif

/***************************************************************************************************************************************************************
 **********  UNIX_TIMESTAMP
 **************************************************************************************************************************************************************/
// Some definitions for calculation
#define WD_SEC_PER_MIN  60UL
#define WD_SEC_PER_HOUR 3600UL
#define WD_SEC_PER_DAY  86400UL
#define WD_SEC_PER_YEAR (WD_SEC_PER_DAY * 365)

// extracts 1..4 characters from a string and interprets it as a decimal value
#define WD_CONV_STR2DEC_1(str, i) (str[i] > '0' ? str[i] - '0' : 0)
#define WD_CONV_STR2DEC_2(str, i) (WD_CONV_STR2DEC_1(str, i) * 10 + str[i + 1] - '0')
#define WD_CONV_STR2DEC_3(str, i) (WD_CONV_STR2DEC_2(str, i) * 10 + str[i + 2] - '0')
#define WD_CONV_STR2DEC_4(str, i) (WD_CONV_STR2DEC_3(str, i) * 10 + str[i + 3] - '0')

// Custom "glue logic" to convert the month name to a usable number
#define WD_GET_MONTH(str, i)                                        \
    (str[i] == 'J' && str[i + 1] == 'a' && str[i + 2] == 'n'   ? 1  \
     : str[i] == 'F' && str[i + 1] == 'e' && str[i + 2] == 'b' ? 2  \
     : str[i] == 'M' && str[i + 1] == 'a' && str[i + 2] == 'r' ? 3  \
     : str[i] == 'A' && str[i + 1] == 'p' && str[i + 2] == 'r' ? 4  \
     : str[i] == 'M' && str[i + 1] == 'a' && str[i + 2] == 'y' ? 5  \
     : str[i] == 'J' && str[i + 1] == 'u' && str[i + 2] == 'n' ? 6  \
     : str[i] == 'J' && str[i + 1] == 'u' && str[i + 2] == 'l' ? 7  \
     : str[i] == 'A' && str[i + 1] == 'u' && str[i + 2] == 'g' ? 8  \
     : str[i] == 'S' && str[i + 1] == 'e' && str[i + 2] == 'p' ? 9  \
     : str[i] == 'O' && str[i + 1] == 'c' && str[i + 2] == 't' ? 10 \
     : str[i] == 'N' && str[i + 1] == 'o' && str[i + 2] == 'v' ? 11 \
     : str[i] == 'D' && str[i + 1] == 'e' && str[i + 2] == 'c' ? 12 \
                                                               : 0)

// extract the information from the time string given by __TIME__ and __DATE__
#define WD_TIME_SECONDS WD_CONV_STR2DEC_2(__TIME__, 6)
#define WD_TIME_MINUTES WD_CONV_STR2DEC_2(__TIME__, 3)
#define WD_TIME_HOURS   WD_CONV_STR2DEC_2(__TIME__, 0)
#define WD_TIME_DAYS    WD_CONV_STR2DEC_2(__DATE__, 4)
#define WD_TIME_MONTH   WD_GET_MONTH(__DATE__, 0)
#define WD_TIME_YEARS   WD_CONV_STR2DEC_4(__DATE__, 7)

// Days in February
#define WD_UNIX_TIMESTAMP_FDAY(year) \
    (((year) % 400) == 0UL ? 29UL    \
                           : (((year) % 100) == 0UL ? 28UL : (((year) % 4) == 0UL ? 29UL : 28UL)))

// Days in the year
#define WD_UNIX_TIMESTAMP_YDAY(year, month, day)                                             \
    (/* January */ day /* February */ + (month >= 2 ? 31UL : 0UL) /* March */ +              \
     (month >= 3 ? WD_UNIX_TIMESTAMP_FDAY(year) : 0UL) /* April */ +                         \
     (month >= 4 ? 31UL : 0UL) /* May */ + (month >= 5 ? 30UL : 0UL) /* June */ +            \
     (month >= 6 ? 31UL : 0UL) /* July */ + (month >= 7 ? 30UL : 0UL) /* August */ +         \
     (month >= 8 ? 31UL : 0UL) /* September */ + (month >= 9 ? 31UL : 0UL) /* October */ +   \
     (month >= 10 ? 30UL : 0UL) /* November */ + (month >= 11 ? 31UL : 0UL) /* December */ + \
     (month >= 12 ? 30UL : 0UL))

// get the UNIX timestamp from a digits representation
#define WD_UNIX_TIMESTAMP(year, month, day, hour, minute, second)                              \
    (/* time */ second + minute * WD_SEC_PER_MIN + hour * WD_SEC_PER_HOUR +                    \
     /* year day (month + day) */ (WD_UNIX_TIMESTAMP_YDAY(year, month, day) - 1) *             \
         WD_SEC_PER_DAY +                                                                      \
     /* year */ (year - 1970UL) * WD_SEC_PER_YEAR + ((year - 1969UL) / 4UL) * WD_SEC_PER_DAY - \
     ((year - 1901UL) / 100UL) * WD_SEC_PER_DAY + ((year - 1601UL) / 400UL) * WD_SEC_PER_DAY)

// the UNIX timestamp
#define WD_BUILD_UNIX_TIMESTAMP                                                                    \
    (WD_UNIX_TIMESTAMP(WD_TIME_YEARS, WD_TIME_MONTH, WD_TIME_DAYS, WD_TIME_HOURS, WD_TIME_MINUTES, \
                       WD_TIME_SECONDS))

/***************************************************************************************************************************************************************
 **********  math
 **************************************************************************************************************************************************************/
#ifndef RTE_FPU_TYPE
#define RTE_FPU_TYPE 1
#endif

#if RTE_FPU_TYPE != 2
#define fpu_t                  double
#define wd_arm_cfft_init       arm_cfft_init_f64
#define wd_arm_cfft_instance   arm_cfft_instance_f64
#define wd_arm_cfft            arm_cfft_f64
#define wd_arm_cfft_sr_len1024 arm_cfft_sR_f64_len1024
#define wd_arm_cmplx_mag       arm_cmplx_mag_f64
#define wd_arm_max             arm_max_f64
#define wd_creal               creal
#define wd_cimag               cimag
#define wd_fmod                fmod
#define wd_modf                modf
#define wd_atan2               atan2
#define wd_acos                acos
#define wd_asin                asin
#define wd_sin                 sin
#define wd_cos                 cos
#else
#define fpu_t                  float
#define wd_arm_cfft_init       arm_cfft_init_f32
#define wd_arm_cfft_instance   arm_cfft_instance_f32
#define wd_arm_cfft            arm_cfft_f32
#define wd_arm_cfft_sr_len1024 arm_cfft_sR_f32_len1024
#define wd_arm_cmplx_mag       arm_cmplx_mag_f32
#define wd_arm_max             arm_max_f32
#define wd_creal               crealf
#define wd_cimag               cimagf
#define wd_fmod                fmodf
#define wd_modf                modff
#define wd_atan2               atan2f
#define wd_acos                acosf
#define wd_asin                asinf
#define wd_sin                 sinf
#define wd_cos                 cosf
#endif

/***************************************************************************************************************************************************************
 **********  Include
 **************************************************************************************************************************************************************/
//#include "_HW_Board.h"
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticTask_t os_thread_t;

/****************************************************************25600.000000***********************************************************************************************
 **********  System
 **************************************************************************************************************************************************************/
#define RTE_SOFTWARE_TYPE_MASK       0xFF00u
#define RTE_SOFTWARE_TYPE_BOOTLOADER 0x0000u
#define RTE_SOFTWARE_TYPE_APPLICATON 0x0100u

#define RTE_SOFTWARE_BUILD_MASK    0xF0u
#define RTE_SOFTWARE_BUILD_RELEASE 0x00u
#define RTE_SOFTWARE_BUILD_DEBUG   0x10u

#define RTE_SOFTWARE_LEVEL_MASK   0x0Fu
#define RTE_SOFTWARE_LEVEL_NORMAL 0x01u
#define RTE_SOFTWARE_LEVEL_V      0x02u // Verbose level 1
#define RTE_SOFTWARE_LEVEL_VV     0x03u // Verbose level 2
#define RTE_SOFTWARE_LEVEL_VVV    0x04u // Verbose level 3
#define RTE_SOFTWARE_LEVEL_TEST   0x05u // Run Unit / CI Test

#define RTE_SOFTWARE_VERSION(type, build, level) (type | build | level)
#define RTE_SOFTWARE_TYPE(data)                  (data & RTE_SOFTWARE_TYPE_MASK)
#define RTE_SOFTWARE_BUILD(data)                 (data & RTE_SOFTWARE_BUILD_MASK)
#define RTE_SOFTWARE_LEVEL(data)                 (data & RTE_SOFTWARE_LEVEL_MASK)


/***************************************************************************************************************************************************************
 **********  Generic helper macros (moved from _AppConfig.h)
 **************************************************************************************************************************************************************/
#ifndef WD_SIZEOF_CSTR
#define WD_SIZEOF_CSTR(s) (sizeof(s) - 1)
#endif
#ifndef WD_PACKED_PTR
#define WD_PACKED_PTR(arg) arg *__attribute__((packed))
#endif
#ifndef WD_LOCATION
#define WD_LOCATION(location) __attribute__((section(".bss.ARM.__at_" #location)))
#endif
#ifndef WD_ALIGNED_UP
#define WD_ALIGNED_UP(x, aligned_num) (((x) + (aligned_num) - 1) & ~((uint32_t)(aligned_num) - 1U))
#endif
#ifndef WD_ALIGNED_DOWN
#define WD_ALIGNED_DOWN(x, aligned_num) ((x) & ~((aligned_num) - 1))
#endif

#if defined(__GNUC__)
#ifndef WD_UNALIGNED_FLOAT_WRITE
#define WD_UNALIGNED_FLOAT_WRITE(addr, val)                  \
    do {                                                     \
        struct __attribute__((__packed__)) T_FLOAT_WRITE {   \
            float v;                                         \
        };                                                   \
        ((struct T_FLOAT_WRITE *)(void *)(addr))->v = (val); \
    } while (0)
#endif
#ifndef WD_UNALIGNED_FLOAT_READ
#define WD_UNALIGNED_FLOAT_READ(addr)                           \
    ({                                                          \
        struct __attribute__((__packed__)) T_FLOAT_READ {       \
            float v;                                            \
        };                                                      \
        ((const struct T_FLOAT_READ *)(const void *)(addr))->v; \
    })
#endif
#endif

#if defined(__cplusplus)
}
#endif

#endif /* WD_CORE_WD_DEF_H */
