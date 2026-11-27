/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 *//**
 *  \version         V1.0.0 [cubeide]
 *  \author          doubled
 ** @{
 ******************************************************************************/

#ifndef WD_CORE_WD_PRINT_H
#define WD_CORE_WD_PRINT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "wd_core.h"


/* output back-ends (_PF_Print_hal UART, Telnet, ITM, RTT ...) are optional modules outside the core :
   they register themselves with wd_print_register_output() / wd_print_register_input() */

#define WD_TERM_SOH       "\001" // ASCII Start Of Header   [Level]
#define WD_TERM_SOH_ASCII '\001'

#define WD_TERM_DC1       "\021" // ASCII Device Control 1  [Color]
#define WD_TERM_DC1_ASCII '\021'

#define WD_TERM_DC2       "\022" // ASCII Device Control 2  [BG Color]
#define WD_TERM_DC2_ASCII '\022'

#define WD_TERM_DC3       "\023" // ASCII Device Control 3  [Pre Command]
#define WD_TERM_DC3_ASCII '\023'

#define WD_TERM_DC4       "\024" // ASCII Device Control 4  [Post Command]
#define WD_TERM_DC4_ASCII '\024'

#define WD_TERM_EMERG   WD_TERM_SOH "0" // [pr_emerg ]  system is unusable
#define WD_TERM_ALERT   WD_TERM_SOH "1" // [pr_alert ]  action must be taken immediately
#define WD_TERM_CRIT    WD_TERM_SOH "2" // [pr_crit  ]  critical conditions
#define WD_TERM_ERR     WD_TERM_SOH "3" // [pr_err   ]  error conditions
#define WD_TERM_WARNING WD_TERM_SOH "4" // [pr_warn  ]  warning conditions
#define WD_TERM_NOTICE  WD_TERM_SOH "5" // [pr_notice]  normal but significant condition
#define WD_TERM_INFO    WD_TERM_SOH "6" // [pr_info  ]  informational
#define WD_TERM_DEBUG   WD_TERM_SOH "7" // [pr_debug ]  debug-level messages
#define WD_TERM_CONT    WD_TERM_SOH "c" // [pr_cont  ]  without pr_fmt(fmt)

#define WD_TERM_DEFAULT "" /* the default kernel loglevel */
#define WD_TERM_NULL    ""

#define WD_CONSOLE_LEVEL_SILENT     0  /* Mum's the word */
#define WD_CONSOLE_LEVEL_MIN        1  /* Minimum loglevel we let people use */
#define WD_CONSOLE_LEVEL_MOTORMOUTH 15 /* You can't shut this one up */
#define WD_CONSOLE_LEVEL_EMERG      0
#define WD_CONSOLE_LEVEL_ALERT      1
#define WD_CONSOLE_LEVEL_CRIT       2
#define WD_CONSOLE_LEVEL_ERR        3
#define WD_CONSOLE_LEVEL_WARNING    4
#define WD_CONSOLE_LEVEL_NOTICE     5
#define WD_CONSOLE_LEVEL_INFO       6
#define WD_CONSOLE_LEVEL_DEBUG      7
#define WD_CONSOLE_LEVEL_FULL       15

/** \brief General Message Control */
typedef enum {
/*--------------- Color    : [ 0: 7] ---------------*/
#define WD_MSG_COLOR_POS 0
#define WD_MSG_COLOR_MSK (0xFFUL << WD_MSG_COLOR_POS)

#define WD_MSG_FOREGROUND_COLOR_POS 0
#define WD_MSG_FOREGROUND_COLOR_MSK (0x7UL << WD_MSG_FOREGROUND_COLOR_POS)
    WD_MSG_RED = ((1UL << 0) << WD_MSG_COLOR_POS),    ///< ANSI escape code Foreground Red Color
    WD_MSG_GREEN = ((2UL << 0) << WD_MSG_COLOR_POS),  ///< ANSI escape code Foreground Green Color
    WD_MSG_YELLOW = ((3UL << 0) << WD_MSG_COLOR_POS), ///< ANSI escape code Foreground Yellow Color
    WD_MSG_BLUE = ((4UL << 0) << WD_MSG_COLOR_POS),   ///< ANSI escape code Foreground Blue Color
    WD_MSG_MAGENTA =
        ((5UL << 0) << WD_MSG_COLOR_POS),            ///< ANSI escape code Foreground Magenta Color
    WD_MSG_CYAN = ((6UL << 0) << WD_MSG_COLOR_POS),  ///< ANSI escape code Foreground Cyan Color
    WD_MSG_WHITE = ((7UL << 0) << WD_MSG_COLOR_POS), ///< ANSI escape code Foreground White Color
    WD_MSG_BOLD = ((1UL << 3) << WD_MSG_COLOR_POS),  ///< ANSI escape code Set Bold font weight

#define WD_MSG_BACKGROUND_COLOR_POS 4
#define WD_MSG_BACKGROUND_COLOR_MSK (0x7UL << WD_MSG_BACKGROUND_COLOR_POS)
    WD_MSG_BG_RED = ((1UL << 4) << WD_MSG_COLOR_POS),   ///< ANSI escape code Background Red Color
    WD_MSG_BG_GREEN = ((2UL << 4) << WD_MSG_COLOR_POS), ///< ANSI escape code Background Green Color
    WD_MSG_BG_YELLOW =
        ((3UL << 4) << WD_MSG_COLOR_POS),              ///< ANSI escape code Background Yellow Color
    WD_MSG_BG_BLUE = ((4UL << 4) << WD_MSG_COLOR_POS), ///< ANSI escape code Background Blue Color
    WD_MSG_BG_MAGENTA =
        ((5UL << 4) << WD_MSG_COLOR_POS), ///< ANSI escape code Background Magenta Color
    WD_MSG_BG_CYAN = ((6UL << 4) << WD_MSG_COLOR_POS),  ///< ANSI escape code Background Cyan Color
    WD_MSG_BG_WHITE = ((7UL << 4) << WD_MSG_COLOR_POS), ///< ANSI escape code Background White Color
    WD_MSG_BG_BOLD = ((1UL << 7) << WD_MSG_COLOR_POS),  ///< ANSI escape code Set Bold font weight

/*--------------- Command  : [ 8:11] ---------------*/
#define WD_MSG_COMMAND_POS 8
#define WD_MSG_COMMAND_MSK (0x0FUL << WD_MSG_COMMAND_POS)
    WD_MSG_CRLF = ((1UL << 0) << WD_MSG_COMMAND_POS),       ///< Clear screen and home cursor ""
    WD_MSG_CLEAR = ((1UL << 1) << WD_MSG_COMMAND_POS),      ///< Clear screen and home cursor ""
    WD_MSG_DELIMETER = ((1UL << 2) << WD_MSG_COMMAND_POS),  ///< Delimeter "========...========"
    WD_MSG_TIME_STAMP = ((1UL << 3) << WD_MSG_COMMAND_POS), ///< Current Time Stamp
/*--------------- Function : [12:15] ---------------*/
#define WD_MSG_FUNCTION_POS 12
#define WD_MSG_FUNCTION_MSK (0xFUL << WD_MSG_FUNCTION_POS)
    WD_MSG_DUMP = (
        1UL
        << WD_MSG_FUNCTION_POS), ///< Dump ((char *) msg, uint32_t ptr, uint32_t num, uint32_t option)
    WD_MSG_MEM_DUMP = (
        2UL
        << WD_MSG_FUNCTION_POS), ///< Memory dump ((char *) msg, uint32_t addr, uint32_t ptr, uint32_t num, uint32_t column)
    WD_MSG_FRAME_DUMP = (
        3UL
        << WD_MSG_FUNCTION_POS), ///< Frame dump ((char *) msg, uint32_t type, uint32_t ptr, uint32_t num)
    WD_MSG_HEX_TO_ASCII = (
        4UL
        << WD_MSG_FUNCTION_POS), ///< Change hex to ascii ((char *) msg, uint32_t ptr, uint32_t num)

/*--------------- Argument : [16:31] ---------------*/
#define WD_MSG_ARGUMENT_POS 16
#define WD_MSG_ARGUMENT_MSK (0xFFFFUL << WD_MSG_ARGUMENT_POS)
#define WD_MSG_ARGUMENT(n)  ((n) << WD_MSG_ARGUMENT_POS)
/*--------------- Pre-defined ---------------*/
#if 0
  WD_MSG_MAGIC              = 1234567890,                                             // 0x499602D2
  WD_MSG_NORMAL             = WD_MSG_TIME_STAMP | WD_MSG_CRLF,

  WD_MSG_FLAG_DELIMETER          = WD_MSG_DELIMETER,
  WD_MSG_FLAG_CRLF               = WD_MSG_CRLF,

  WD_MSG_ERROR              = WD_MSG_WHITE | WD_MSG_BOLD | WD_MSG_BG_RED | WD_MSG_BG_BOLD,
  WD_MSG_ERROR_DEBUG        = WD_MSG_RED | WD_MSG_BOLD,
  WD_MSG_ASSERT             = WD_MSG_BOLD | WD_MSG_BG_WHITE | WD_MSG_BG_BOLD,
  WD_MSG_DEBUG              = WD_MSG_CYAN | WD_MSG_BOLD,

  WD_MSG_SYSTEM             = WD_MSG_YELLOW | WD_MSG_BOLD | WD_MSG_BG_BLUE | WD_MSG_BG_BOLD,
  WD_MSG_NOTI               = WD_MSG_WHITE | WD_MSG_BOLD | WD_MSG_BG_CYAN,
  WD_MSG_TITLE              = WD_MSG_YELLOW | WD_MSG_BOLD,
  WD_MSG_TX                 = WD_MSG_WHITE | WD_MSG_BOLD | WD_MSG_BG_BLUE | WD_MSG_BG_BOLD,
  WD_MSG_RX                 = WD_MSG_WHITE | WD_MSG_BOLD | WD_MSG_BG_GREEN | WD_MSG_BG_BOLD,
  WD_MSG_INFO               = WD_MSG_MAGENTA | WD_MSG_BOLD,
  WD_MSG_EVENT              = WD_MSG_GREEN | WD_MSG_BOLD,

  WD_MSG_PASS               = WD_MSG_GREEN,
  WD_MSG_FAIL               = WD_MSG_RED,
#else
    WD_MSG_MAGIC = 1234567890, // 0x499602D2
    WD_MSG_NORMAL = WD_MSG_TIME_STAMP | WD_MSG_CRLF,

    WD_MSG_FLAG_DELIMETER = WD_MSG_DELIMETER,
    WD_MSG_FLAG_CRLF = WD_MSG_CRLF,

    WD_MSG_ERROR = WD_MSG_WHITE | WD_MSG_BOLD | WD_MSG_BG_RED | WD_MSG_BG_BOLD,
    WD_MSG_ERROR_DEBUG = WD_MSG_RED | WD_MSG_BOLD,
    WD_MSG_ASSERT = WD_MSG_BOLD | WD_MSG_BG_WHITE | WD_MSG_BG_BOLD,
    WD_MSG_DEBUG = WD_MSG_CYAN | WD_MSG_BOLD,

    WD_MSG_SYSTEM = WD_MSG_YELLOW | WD_MSG_BOLD,
    WD_MSG_NOTI = WD_MSG_WHITE | WD_MSG_BOLD,
    WD_MSG_TITLE = WD_MSG_WHITE | WD_MSG_BOLD,
    WD_MSG_TX = WD_MSG_WHITE | WD_MSG_BOLD,
    WD_MSG_RX = WD_MSG_WHITE | WD_MSG_BOLD,
    WD_MSG_INFO = WD_MSG_WHITE | WD_MSG_BOLD,
    WD_MSG_EVENT = WD_MSG_WHITE | WD_MSG_BOLD,

    WD_MSG_PASS = WD_MSG_GREEN,
    WD_MSG_FAIL = WD_MSG_RED,
#endif
    /*--------------- Msg Format ---------------*/
    WD_MSG_ERROR_CRLF = WD_MSG_ERROR | WD_MSG_CRLF,
    WD_MSG_ERROR_DEBUG_CRLF = WD_MSG_ERROR_DEBUG | WD_MSG_CRLF,
    WD_MSG_ASSERT_CRLF = WD_MSG_ASSERT | WD_MSG_CRLF,
    WD_MSG_DEBUG_CRLF = WD_MSG_DEBUG | WD_MSG_CRLF,

    WD_MSG_SYSTEM_CRLF = WD_MSG_SYSTEM | WD_MSG_CRLF,
    WD_MSG_NOTI_CRLF = WD_MSG_NOTI | WD_MSG_CRLF,
    WD_MSG_TITLE_CRLF = WD_MSG_TITLE | WD_MSG_CRLF,
    WD_MSG_TX_CRLF = WD_MSG_TX | WD_MSG_CRLF,
    WD_MSG_RX_CRLF = WD_MSG_RX | WD_MSG_CRLF,
    WD_MSG_INFO_CRLF = WD_MSG_INFO | WD_MSG_CRLF,
    WD_MSG_EVENT_CRLF = WD_MSG_EVENT | WD_MSG_CRLF,

    WD_MSG_DUMP_CRLF = WD_MSG_DUMP | WD_MSG_CRLF,
    WD_MSG_MEM_DUMP_CRLF = WD_MSG_MEM_DUMP | WD_MSG_CRLF,
    WD_MSG_FRAME_DUMP_CRLF = WD_MSG_FRAME_DUMP | WD_MSG_CRLF,
    WD_MSG_HEX_TO_ASCII_CRLF = WD_MSG_HEX_TO_ASCII | WD_MSG_CRLF,

    WD_MSG_PASS_CRLF = WD_MSG_GREEN | WD_MSG_CRLF,
    WD_MSG_FAIL_CRLF = WD_MSG_RED | WD_MSG_CRLF,

    WD_MSG_ERROR_LOG = WD_MSG_TIME_STAMP | WD_MSG_ERROR | WD_MSG_CRLF,
    WD_MSG_ERROR_DEBUG_LOG = WD_MSG_TIME_STAMP | WD_MSG_ERROR_DEBUG | WD_MSG_CRLF,
    WD_MSG_ASSERT_LOG = WD_MSG_TIME_STAMP | WD_MSG_ASSERT | WD_MSG_CRLF,
    WD_MSG_DEBUG_LOG = WD_MSG_TIME_STAMP | WD_MSG_DEBUG | WD_MSG_CRLF,

    WD_MSG_SYSTEM_LOG = WD_MSG_TIME_STAMP | WD_MSG_SYSTEM | WD_MSG_CRLF,
    WD_MSG_NOTI_LOG = WD_MSG_TIME_STAMP | WD_MSG_NOTI | WD_MSG_CRLF,
    WD_MSG_TITLE_LOG = WD_MSG_TIME_STAMP | WD_MSG_TITLE | WD_MSG_CRLF,
    WD_MSG_TX_LOG = WD_MSG_TIME_STAMP | WD_MSG_TX | WD_MSG_CRLF,
    WD_MSG_RX_LOG = WD_MSG_TIME_STAMP | WD_MSG_RX | WD_MSG_CRLF,
    WD_MSG_INFO_LOG = WD_MSG_TIME_STAMP | WD_MSG_INFO | WD_MSG_CRLF,
    WD_MSG_EVENT_LOG = WD_MSG_TIME_STAMP | WD_MSG_EVENT | WD_MSG_CRLF,

    WD_MSG_DUMP_LOG = WD_MSG_TIME_STAMP | WD_MSG_DUMP | WD_MSG_CRLF,
    WD_MSG_MEM_DUMP_LOG = WD_MSG_TIME_STAMP | WD_MSG_MEM_DUMP | WD_MSG_CRLF,
    WD_MSG_FRAME_DUMP_LOG = WD_MSG_TIME_STAMP | WD_MSG_FRAME_DUMP | WD_MSG_CRLF,
    WD_MSG_HEX_TO_ASCII_LOG = WD_MSG_TIME_STAMP | WD_MSG_HEX_TO_ASCII | WD_MSG_CRLF,

    WD_MSG_PASS_LOG = WD_MSG_TIME_STAMP | WD_MSG_GREEN | WD_MSG_CRLF,
    WD_MSG_FAIL_LOG = WD_MSG_TIME_STAMP | WD_MSG_RED | WD_MSG_CRLF,

    /*--------------- Option ---------------*/
    WD_MSG_OPTION_HEX = 0,
    WD_MSG_OPTION_ASCII = 65535,

} wd_print_msg_control_t;

#if 0
#define WD_MSG_RED     A
#define WD_MSG_GREEN   B
#define WD_MSG_YELLOW  C
#define WD_MSG_BLUE    D
#define WD_MSG_MAGENTA E
#define WD_MSG_CYAN    F
#define WD_MSG_WHITE   G

#define WD_MSG_RED_B     a
#define WD_MSG_GREEN_B   b
#define WD_MSG_YELLOW_B  c
#define WD_MSG_BLUE_B    d
#define WD_MSG_MAGENTA_B e
#define WD_MSG_CYAN_B    f
#define WD_MSG_WHITE_B   g

#define WD_MSG_CLEAR      0
#define WD_MSG_DELIMETER  1
#define WD_MSG_TIME_STAMP 2

#define WD_MSG_MEM_DUMP     A
#define WD_MSG_HEX_TO_ASCII B

#define WD_COLOR(color) WD_TERM_DC1 WD_STRINGIFY(color)
#endif

/* ----------------------------------------------------------------------------- console output / input registry */
/** @brief output sink : writes one byte (e.g. UART, Telnet, RTT). Must not block indefinitely. */
typedef void (*wd_print_putc_fn_t)(uint8_t ch);
/** @brief input source : returns 0..255, or a negative value when no data (non-blocking). */
typedef int32_t (*wd_print_getc_fn_t)(void);

/**
 * @brief  Register a console output sink (all sinks receive every byte).
 * @param  putc_fn non-NULL
 * @return WD_CODE_OK, WD_CODE_ERROR_PARAMETER (NULL), WD_CODE_ERROR_BUSY (table full)
 * @note   Call at initialization (not thread safe, not from ISR). Re-registering the same sink is allowed.
 */
wd_code_t wd_print_register_output(wd_print_putc_fn_t putc_fn);

/**
 * @brief  Register a console input source (polled in registration order).
 * @param  getc_fn non-NULL
 * @return WD_CODE_OK, WD_CODE_ERROR_PARAMETER (NULL), WD_CODE_ERROR_BUSY (table full)
 */
wd_code_t wd_print_register_input(wd_print_getc_fn_t getc_fn);

/** @brief Write one byte to all registered outputs. No output is registered : byte is dropped. */
void wd_print_putc(uint8_t ch);
/** @brief Write size_bytes bytes to all registered outputs. data may be NULL (no operation). */
void wd_print_write(const uint8_t *data, size_t size_bytes);
/** @brief Non-blocking read : returns 0..255, or -1 when no input has data. */
int32_t wd_print_getc(void);
/** @brief Blocking read (yields to the RTOS while waiting). Do NOT call from ISR. */
int32_t wd_print_waitc(void);

int wd_print_pre_control(uint32_t control, wd_printx_t printx_func);
int wd_print_post_control(uint32_t control, wd_printx_t printx_func);
int wd_print_function(uint32_t control, wd_printx_t printx_func, const char *fmt, va_list *ap);

//extern int Console_Level;
extern int wd_print_level;

#if (RTE_RTOS)
extern uint32_t wd_print_owner;
#endif
int wd_print_printd(uint32_t control, const char *fmt, ...);

#ifndef pr_err_fmt
#define pr_err_fmt(fmt) \
    WD_COLOR_ERROR "[%s::%d  %s]" WD_COLOR_RESET " " fmt, __FILE__, __LINE__, __FUNCTION__
#endif

#ifndef pr_fmt
#define pr_fmt(fmt) "[%s:%d] " fmt, __func__, __LINE__
//#define pr_fmt(fmt)                                                             fmt
#endif

#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif
#define pr_print(fmt, ...)                                                         \
    do {                                                                           \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {             \
            (void)wd_print_printd(NULL, WD_TERM_EMERG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                          \
    } while (0)
#define pr_emerg(fmt, ...)                                                                 \
    do {                                                                                   \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                     \
            (void)wd_print_printd(WD_MSG_ERROR, WD_TERM_EMERG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                  \
    } while (0)
#define pr_alert(fmt, ...)                                                                 \
    do {                                                                                   \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {                     \
            (void)wd_print_printd(WD_MSG_ERROR, WD_TERM_ALERT pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                  \
    } while (0)
#define pr_crit(fmt, ...)                                                                 \
    do {                                                                                  \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {                     \
            (void)wd_print_printd(WD_MSG_ERROR, WD_TERM_CRIT pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                 \
    } while (0)
#define pr_err(fmt, ...)                                                                 \
    do {                                                                                 \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {                     \
            (void)wd_print_printd(WD_MSG_ERROR, WD_TERM_ERR pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                \
    } while (0)
#define pr_warn(fmt, ...)                                                            \
    do {                                                                             \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {             \
            (void)wd_print_printd(NULL, WD_TERM_WARNING pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                            \
    } while (0)
#define pr_notice(fmt, ...)                                                         \
    do {                                                                            \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {             \
            (void)wd_print_printd(NULL, WD_TERM_NOTICE pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                           \
    } while (0)
#define pr_info(fmt, ...)                                                         \
    do {                                                                          \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {             \
            (void)wd_print_printd(NULL, WD_TERM_INFO pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                         \
    } while (0)
#define pr_debug(fmt, ...)                                                         \
    do {                                                                           \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {             \
            (void)wd_print_printd(NULL, WD_TERM_DEBUG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                          \
    } while (0)

#define pf_print(control, fmt, ...)                                                   \
    do {                                                                              \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                \
            (void)wd_print_printd(control, WD_TERM_EMERG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                             \
    } while (0)
#define pf_emerg(control, fmt, ...)                                                   \
    do {                                                                              \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                \
            (void)wd_print_printd(control, WD_TERM_EMERG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                             \
    } while (0)
#define pf_alert(control, fmt, ...)                                                   \
    do {                                                                              \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {                \
            (void)wd_print_printd(control, WD_TERM_ALERT pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                             \
    } while (0)
#define pf_crit(control, fmt, ...)                                                   \
    do {                                                                             \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {                \
            (void)wd_print_printd(control, WD_TERM_CRIT pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                            \
    } while (0)
#define pf_err(control, fmt, ...)                                                   \
    do {                                                                            \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {                \
            (void)wd_print_printd(control, WD_TERM_ERR pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                           \
    } while (0)
#define pf_warn(control, fmt, ...)                                                      \
    do {                                                                                \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {                \
            (void)wd_print_printd(control, WD_TERM_WARNING pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                               \
    } while (0)
#define pf_notice(control, fmt, ...)                                                   \
    do {                                                                               \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {                \
            (void)wd_print_printd(control, WD_TERM_NOTICE pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                              \
    } while (0)
#define pf_info(control, fmt, ...)                                                   \
    do {                                                                             \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {                \
            (void)wd_print_printd(control, WD_TERM_INFO pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                            \
    } while (0)
#define pf_debug(control, fmt, ...)                                                   \
    do {                                                                              \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {                \
            (void)wd_print_printd(control, WD_TERM_DEBUG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                             \
    } while (0)

#define WD_PRINT_PLAIN(fmt, ...)                                                       \
    do {                                                                               \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                 \
            (void)wd_print_printd(WD_MSG_FLAG_CRLF, WD_TERM_EMERG fmt, ##__VA_ARGS__); \
        }                                                                              \
    } while (0)
#define WD_LOG_EMERG(fmt, ...)                                                                     \
    do {                                                                                           \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                             \
            (void)wd_print_printd(WD_MSG_ERROR_LOG, WD_TERM_EMERG pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                          \
    } while (0)
#define WD_LOG_ALERT(fmt, ...)                                                                     \
    do {                                                                                           \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {                             \
            (void)wd_print_printd(WD_MSG_ERROR_LOG, WD_TERM_ALERT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                          \
    } while (0)
#define WD_LOG_CRITICAL(fmt, ...)                                                                 \
    do {                                                                                          \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {                             \
            (void)wd_print_printd(WD_MSG_ERROR_LOG, WD_TERM_CRIT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                         \
    } while (0)
#define WD_LOG_ERROR(fmt, ...)                                                                   \
    do {                                                                                         \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {                             \
            (void)wd_print_printd(WD_MSG_ERROR_LOG, WD_TERM_ERR pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                        \
    } while (0)
#define WD_LOG_WARN(fmt, ...)                                                            \
    do {                                                                                 \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {                 \
            (void)wd_print_printd(WD_MSG_ERROR_LOG, WD_TERM_WARNING fmt, ##__VA_ARGS__); \
        }                                                                                \
    } while (0)
#define WD_LOG_NOTICE(fmt, ...)                                                        \
    do {                                                                               \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {                \
            (void)wd_print_printd(WD_MSG_NOTI_LOG, WD_TERM_NOTICE fmt, ##__VA_ARGS__); \
        }                                                                              \
    } while (0)
#define WD_LOG_INFO(fmt, ...)                                                         \
    do {                                                                              \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {                 \
            (void)wd_print_printd(WD_MSG_INFO_CRLF, WD_TERM_INFO fmt, ##__VA_ARGS__); \
        }                                                                             \
    } while (0)
#define WD_LOG_DEBUG(fmt, ...)                                                         \
    do {                                                                               \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {                 \
            (void)wd_print_printd(WD_MSG_DEBUG_LOG, WD_TERM_DEBUG fmt, ##__VA_ARGS__); \
        }                                                                              \
    } while (0)

#if 0
#define WD_PRINT_CTRL(control, fmt, ...)                                      \
    do {                                                                      \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {        \
            (void)wd_print_printd(control, WD_TERM_EMERG fmt, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#define WD_EMERG(control, fmt, ...)                                                       \
    do {                                                                                  \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                    \
            (void)wd_print_printd(control, WD_TERM_EMERG pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                 \
    } while (0)
#define WD_ALERT(control, fmt, ...)                                                       \
    do {                                                                                  \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {                    \
            (void)wd_print_printd(control, WD_TERM_ALERT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                 \
    } while (0)
#define WD_CRITICAL(control, fmt, ...)                                                   \
    do {                                                                                 \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {                    \
            (void)wd_print_printd(control, WD_TERM_CRIT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                \
    } while (0)
#define WD_ERROR(control, fmt, ...)                                                     \
    do {                                                                                \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {                    \
            (void)wd_print_printd(control, WD_TERM_ERR pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                               \
    } while (0)
#define WD_WARNING(control, fmt, ...)                                           \
    do {                                                                        \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {        \
            (void)wd_print_printd(control, WD_TERM_WARNING fmt, ##__VA_ARGS__); \
        }                                                                       \
    } while (0)
#define WD_NOTICE(control, fmt, ...)                                           \
    do {                                                                       \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {        \
            (void)wd_print_printd(control, WD_TERM_NOTICE fmt, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#define WD_INFO(control, fmt, ...)                                           \
    do {                                                                     \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {        \
            (void)wd_print_printd(control, WD_TERM_INFO fmt, ##__VA_ARGS__); \
        }                                                                    \
    } while (0)
#define WD_DEBUG(control, fmt, ...)                                           \
    do {                                                                      \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {        \
            (void)wd_print_printd(control, WD_TERM_DEBUG fmt, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#endif

#define pr_print_x(term, fmt, ...) \
    wd_print_printx(term, NULL, WD_TERM_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pr_emerg_x(term, fmt, ...)                                                               \
    do {                                                                                         \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                           \
            (void)wd_print_printx(term, WD_MSG_ERROR, WD_TERM_EMERG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                        \
    } while (0)
#define pr_alert_x(term, fmt, ...)                                                               \
    do {                                                                                         \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {                           \
            (void)wd_print_printx(term, WD_MSG_ERROR, WD_TERM_ALERT pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                        \
    } while (0)
#define pr_crit_x(term, fmt, ...)                                                               \
    do {                                                                                        \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {                           \
            (void)wd_print_printx(term, WD_MSG_ERROR, WD_TERM_CRIT pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                       \
    } while (0)
#define pr_err_x(term, fmt, ...)                                                               \
    do {                                                                                       \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {                           \
            (void)wd_print_printx(term, WD_MSG_ERROR, WD_TERM_ERR pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                      \
    } while (0)
#define pr_warn_x(term, fmt, ...)                                                          \
    do {                                                                                   \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {                   \
            (void)wd_print_printx(term, NULL, WD_TERM_WARNING pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                  \
    } while (0)
#define pr_notice_x(term, fmt, ...)                                                       \
    do {                                                                                  \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {                   \
            (void)wd_print_printx(term, NULL, WD_TERM_NOTICE pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                 \
    } while (0)
#define pr_info_x(term, fmt, ...)                                                       \
    do {                                                                                \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {                   \
            (void)wd_print_printx(term, NULL, WD_TERM_INFO pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                               \
    } while (0)
#define pr_debug_x(term, fmt, ...)                                                       \
    do {                                                                                 \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {                   \
            (void)wd_print_printx(term, NULL, WD_TERM_DEBUG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                \
    } while (0)

#define pf_print_x(term, control, fmt, ...) \
    wd_print_printx(term, control, WD_TERM_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pf_emerg_x(term, control, fmt, ...)                                                 \
    do {                                                                                    \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                      \
            (void)wd_print_printx(term, control, WD_TERM_EMERG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                   \
    } while (0)
#define pf_alert_x(term, control, fmt, ...)                                                 \
    do {                                                                                    \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {                      \
            (void)wd_print_printx(term, control, WD_TERM_ALERT pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                   \
    } while (0)
#define pf_crit_x(term, control, fmt, ...)                                                 \
    do {                                                                                   \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {                      \
            (void)wd_print_printx(term, control, WD_TERM_CRIT pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                  \
    } while (0)
#define pf_err_x(term, control, fmt, ...)                                                 \
    do {                                                                                  \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {                      \
            (void)wd_print_printx(term, control, WD_TERM_ERR pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                 \
    } while (0)
#define pf_warn_x(term, control, fmt, ...)                                                    \
    do {                                                                                      \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {                      \
            (void)wd_print_printx(term, control, WD_TERM_WARNING pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                     \
    } while (0)
#define pf_notice_x(term, control, fmt, ...)                                                 \
    do {                                                                                     \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {                      \
            (void)wd_print_printx(term, control, WD_TERM_NOTICE pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                    \
    } while (0)
#define pf_info_x(term, control, fmt, ...)                                                 \
    do {                                                                                   \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {                      \
            (void)wd_print_printx(term, control, WD_TERM_INFO pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                  \
    } while (0)
#define pf_debug_x(term, control, fmt, ...)                                                 \
    do {                                                                                    \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {                      \
            (void)wd_print_printx(term, control, WD_TERM_DEBUG pr_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                   \
    } while (0)

#define WD_PRINT_PLAIN_X(term, fmt, ...) \
    wd_print_printx(term, WD_MSG_FLAG_CRLF, WD_TERM_EMERG fmt, ##__VA_ARGS__)
#define WD_LOG_EMERG_X(term, fmt, ...)                                           \
    do {                                                                         \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {           \
            (void)wd_print_printx(term, WD_MSG_ERROR_LOG,                        \
                                  WD_TERM_EMERG pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                        \
    } while (0)
#define WD_LOG_ALERT_X(term, fmt, ...)                                           \
    do {                                                                         \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {           \
            (void)wd_print_printx(term, WD_MSG_ERROR_LOG,                        \
                                  WD_TERM_ALERT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                        \
    } while (0)
#define WD_LOG_CRITICAL_X(term, fmt, ...)                                       \
    do {                                                                        \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {           \
            (void)wd_print_printx(term, WD_MSG_ERROR_LOG,                       \
                                  WD_TERM_CRIT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                       \
    } while (0)
#define WD_LOG_ERROR_X(term, fmt, ...)                                         \
    do {                                                                       \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {           \
            (void)wd_print_printx(term, WD_MSG_ERROR_LOG,                      \
                                  WD_TERM_ERR pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#define WD_LOG_WARN_X(term, fmt, ...)                                                          \
    do {                                                                                       \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {                       \
            (void)wd_print_printx(term, WD_MSG_ERROR_LOG, WD_TERM_WARNING fmt, ##__VA_ARGS__); \
        }                                                                                      \
    } while (0)
#define WD_LOG_NOTICE_X(term, fmt, ...)                                                      \
    do {                                                                                     \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {                      \
            (void)wd_print_printx(term, WD_MSG_NOTI_LOG, WD_TERM_NOTICE fmt, ##__VA_ARGS__); \
        }                                                                                    \
    } while (0)
#define WD_LOG_INFO_X(term, fmt, ...)                                                       \
    do {                                                                                    \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {                       \
            (void)wd_print_printx(term, WD_MSG_INFO_CRLF, WD_TERM_INFO fmt, ##__VA_ARGS__); \
        }                                                                                   \
    } while (0)
#define WD_LOG_DEBUG_X(term, fmt, ...)                                                       \
    do {                                                                                     \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {                       \
            (void)wd_print_printx(term, WD_MSG_DEBUG_LOG, WD_TERM_DEBUG fmt, ##__VA_ARGS__); \
        }                                                                                    \
    } while (0)

#define WD_PRINT_CTRL_X(term, control, fmt, ...) \
    wd_print_printx(term, control, WD_TERM_EMERG fmt, ##__VA_ARGS__)
#define WD_EMERG_X(term, control, fmt, ...)                                                     \
    do {                                                                                        \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                          \
            (void)wd_print_printx(term, control, WD_TERM_EMERG pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                       \
    } while (0)
#define WD_ALERT_X(term, control, fmt, ...)                                                     \
    do {                                                                                        \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {                          \
            (void)wd_print_printx(term, control, WD_TERM_ALERT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                       \
    } while (0)
#define WD_CRITICAL_X(term, control, fmt, ...)                                                 \
    do {                                                                                       \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {                          \
            (void)wd_print_printx(term, control, WD_TERM_CRIT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                      \
    } while (0)
#define WD_ERROR_X(term, control, fmt, ...)                                                   \
    do {                                                                                      \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {                          \
            (void)wd_print_printx(term, control, WD_TERM_ERR pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                     \
    } while (0)
#define WD_WARNING_X(term, control, fmt, ...)                                         \
    do {                                                                              \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {              \
            (void)wd_print_printx(term, control, WD_TERM_WARNING fmt, ##__VA_ARGS__); \
        }                                                                             \
    } while (0)
#define WD_NOTICE_X(term, control, fmt, ...)                                         \
    do {                                                                             \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {              \
            (void)wd_print_printx(term, control, WD_TERM_NOTICE fmt, ##__VA_ARGS__); \
        }                                                                            \
    } while (0)
#define WD_INFO_X(term, control, fmt, ...)                                         \
    do {                                                                           \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {              \
            (void)wd_print_printx(term, control, WD_TERM_INFO fmt, ##__VA_ARGS__); \
        }                                                                          \
    } while (0)
#define WD_DEBUG_X(term, control, fmt, ...)                                         \
    do {                                                                            \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {              \
            (void)wd_print_printx(term, control, WD_TERM_DEBUG fmt, ##__VA_ARGS__); \
        }                                                                           \
    } while (0)


#ifndef WD_PRINT
#define WD_PRINT(control, fmt, ...)                                           \
    do {                                                                      \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {        \
            (void)wd_print_printd(control, WD_TERM_EMERG fmt, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#endif
#ifndef WD_PRINT_ALERT
#define WD_PRINT_ALERT(control, fmt, ...)                                                 \
    do {                                                                                  \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {                    \
            (void)wd_print_printd(control, WD_TERM_ALERT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                 \
    } while (0)
#endif
#ifndef WD_PRINT_CRITICAL
#define WD_PRINT_CRITICAL(control, fmt, ...)                                             \
    do {                                                                                 \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {                    \
            (void)wd_print_printd(control, WD_TERM_CRIT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                \
    } while (0)
#endif
#ifndef WD_PRINT_ERROR
#define WD_PRINT_ERROR(control, fmt, ...)                                               \
    do {                                                                                \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {                    \
            (void)wd_print_printd(control, WD_TERM_ERR pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                               \
    } while (0)
#endif
#ifndef WD_PRINT_WARN
#define WD_PRINT_WARN(control, fmt, ...)                                        \
    do {                                                                        \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {        \
            (void)wd_print_printd(control, WD_TERM_WARNING fmt, ##__VA_ARGS__); \
        }                                                                       \
    } while (0)
#endif
#ifndef WD_PRINT_NOTICE
#define WD_PRINT_NOTICE(control, fmt, ...)                                     \
    do {                                                                       \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {        \
            (void)wd_print_printd(control, WD_TERM_NOTICE fmt, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#endif
#ifndef WD_PRINT_INFO
#define WD_PRINT_INFO(control, fmt, ...)                                     \
    do {                                                                     \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {        \
            (void)wd_print_printd(control, WD_TERM_INFO fmt, ##__VA_ARGS__); \
        }                                                                    \
    } while (0)
#endif
#ifndef WD_PRINT_DEBUG
#define WD_PRINT_DEBUG(control, fmt, ...)                                     \
    do {                                                                      \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {        \
            (void)wd_print_printd(control, WD_TERM_DEBUG fmt, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#endif

#ifndef WD_PRINT_X
#define WD_PRINT_X(term, control, fmt, ...) \
    wd_print_printx(term, control, WD_TERM_EMERG fmt, ##__VA_ARGS__)
#endif
#ifndef WD_PRINT_EMERG_X
#define WD_PRINT_EMERG_X(term, control, fmt, ...)                                               \
    do {                                                                                        \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_EMERG) {                          \
            (void)wd_print_printx(term, control, WD_TERM_EMERG pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                       \
    } while (0)
#endif
#ifndef WD_PRINT_ALERT_X
#define WD_PRINT_ALERT_X(term, control, fmt, ...)                                               \
    do {                                                                                        \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ALERT) {                          \
            (void)wd_print_printx(term, control, WD_TERM_ALERT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                       \
    } while (0)
#endif
#ifndef WD_PRINT_CRITICAL_X
#define WD_PRINT_CRITICAL_X(term, control, fmt, ...)                                           \
    do {                                                                                       \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_CRIT) {                          \
            (void)wd_print_printx(term, control, WD_TERM_CRIT pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                      \
    } while (0)
#endif
#ifndef WD_PRINT_ERROR_X
#define WD_PRINT_ERROR_X(term, control, fmt, ...)                                             \
    do {                                                                                      \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_ERR) {                          \
            (void)wd_print_printx(term, control, WD_TERM_ERR pr_err_fmt(fmt), ##__VA_ARGS__); \
        }                                                                                     \
    } while (0)
#endif
#ifndef WD_PRINT_WARN_X
#define WD_PRINT_WARN_X(term, control, fmt, ...)                                      \
    do {                                                                              \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_WARNING) {              \
            (void)wd_print_printx(term, control, WD_TERM_WARNING fmt, ##__VA_ARGS__); \
        }                                                                             \
    } while (0)
#endif
#ifndef WD_PRINT_NOTICE_X
#define WD_PRINT_NOTICE_X(term, control, fmt, ...)                                   \
    do {                                                                             \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_NOTICE) {              \
            (void)wd_print_printx(term, control, WD_TERM_NOTICE fmt, ##__VA_ARGS__); \
        }                                                                            \
    } while (0)
#endif
#ifndef WD_PRINT_INFO_X
#define WD_PRINT_INFO_X(term, control, fmt, ...)                                   \
    do {                                                                           \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_INFO) {              \
            (void)wd_print_printx(term, control, WD_TERM_INFO fmt, ##__VA_ARGS__); \
        }                                                                          \
    } while (0)
#endif
#ifndef WD_PRINT_DEBUG_X
#define WD_PRINT_DEBUG_X(term, control, fmt, ...)                                   \
    do {                                                                            \
        if (WD_CONF_CONSOLE_LEVEL_COMPILE >= WD_CONSOLE_LEVEL_DEBUG) {              \
            (void)wd_print_printx(term, control, WD_TERM_DEBUG fmt, ##__VA_ARGS__); \
        }                                                                           \
    } while (0)
#endif


#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
int stdout_putchar(int ch);
int stdin_getchar(void);
int stderr_putchar(int ch);
#endif

int wd_print_mbed_minimal_formatted_string(char *buffer, size_t length, const char *format,
                                           va_list arguments, wd_write_func_t write_func);
int wd_print_vprintx(wd_terminal_t *term, const char *format, va_list arguments);
int wd_print_printx(wd_terminal_t *term, uint32_t control, const char *fmt, ...);

#if 0
int wd_print_printx(const char *format, ...);
int wd_print_vprintx(const char *format, va_list arguments);
int sprintx(char *buffer, const char *format, ...);
int snprintx(char *buffer, size_t length, const char *format, ...);
int vsprintx(char *buffer, const char *format, va_list arguments);
int vsnprintx(char *buffer, size_t length, const char *format, va_list arguments);
#endif

#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/
#endif /*--------------- WD_CORE_WD_PRINT_H ---------------*/
/** @} end of file */
