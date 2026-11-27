/* SPDX-License-Identifier: MIT AND Apache-2.0
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright (c) 2017-2020 ARM Limited (mbed OS minimal-printf, Apache-2.0)
 * The mbed portions were modified by DoubleD (function names, configuration macros, explicit casts).
 */
///*************************** (C) COPYRIGHT DoubleD *****************************
// *//**
// *  \version         V1.0.0 [cubeide]
// *  \author          doubled
// ** @{
// ******************************************************************************/
//
/*************************** (C) COPYRIGHT DoubleD *****************************
 *//**
 *  \version         V1.0.0 [cubeide]
 *  \author          doubled
 ** @{
 ******************************************************************************/

#define WD_CONF_CONSOLE_MUTEX         1
#define WD_CONF_CONSOLE_MUTEX_TIMEOUT 10000 // 10 s

#include "wd_core.h"

#ifndef WD_CONF_DMA_ALIGNMENT_SIZE
#define WD_CONF_DMA_ALIGNMENT_SIZE 32
#endif

/***************************************************************************************************************************************************************
 **********  Console output / input registry  (core : no dependency on a physical port)
 **************************************************************************************************************************************************************/
#ifndef WD_CONF_PRINT_OUTPUT_MAX
#define WD_CONF_PRINT_OUTPUT_MAX (3U)
#endif
#ifndef WD_CONF_PRINT_INPUT_MAX
#define WD_CONF_PRINT_INPUT_MAX (3U)
#endif

static wd_print_putc_fn_t s_print_output[WD_CONF_PRINT_OUTPUT_MAX];
static wd_print_getc_fn_t s_print_input[WD_CONF_PRINT_INPUT_MAX];

wd_code_t wd_print_register_output(wd_print_putc_fn_t putc_fn) {
    wd_code_t st = WD_CODE_ERROR_PARAMETER;

    if (putc_fn != NULL) {
        st = WD_CODE_ERROR_BUSY;
        for (uint32_t i = 0U; i < WD_CONF_PRINT_OUTPUT_MAX; i++) {
            if ((s_print_output[i] == NULL) || (s_print_output[i] == putc_fn)) {
                s_print_output[i] = putc_fn;
                st = WD_CODE_OK;
                break;
            }
        }
    }
    return st;
}

wd_code_t wd_print_register_input(wd_print_getc_fn_t getc_fn) {
    wd_code_t st = WD_CODE_ERROR_PARAMETER;

    if (getc_fn != NULL) {
        st = WD_CODE_ERROR_BUSY;
        for (uint32_t i = 0U; i < WD_CONF_PRINT_INPUT_MAX; i++) {
            if ((s_print_input[i] == NULL) || (s_print_input[i] == getc_fn)) {
                s_print_input[i] = getc_fn;
                st = WD_CODE_OK;
                break;
            }
        }
    }
    return st;
}

void wd_print_putc(uint8_t ch) {
    for (uint32_t i = 0U; i < WD_CONF_PRINT_OUTPUT_MAX; i++) {
        if (s_print_output[i] != NULL) {
            s_print_output[i](ch);
        }
    }
}

void wd_print_write(const uint8_t *data, size_t size_bytes) {
    if (data != NULL) {
        for (size_t i = 0U; i < size_bytes; i++) {
            wd_print_putc(data[i]);
        }
    }
}

int32_t wd_print_getc(void) {
    int32_t ch = -1;

    for (uint32_t i = 0U; i < WD_CONF_PRINT_INPUT_MAX; i++) {
        if (s_print_input[i] != NULL) {
            ch = s_print_input[i]();
            if (ch >= 0) {
                break;
            }
        }
    }
    return ch;
}

int32_t wd_print_waitc(void) {
    int32_t ch = wd_print_getc();

    while (ch < 0) {
#if (RTE_RTOS)
        osThreadYield();
#endif
        ch = wd_print_getc();
    }
    return ch;
}


#if 1 /// I/O Retargeting ****************************************************{

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
int stdout_putchar(int ch) {
    wd_print_putc((uint8_t)ch);

    return (ch);
}

int stdin_getchar(void) {
    int c;

    c = (int)wd_print_waitc();

    return (c);
}

int stderr_putchar(int ch) {
    wd_print_putc((uint8_t)ch);

    return (ch);
}

#if 0
/* IO device file handles. */
#define FH_STDIN  0x8001
#define FH_STDOUT 0x8002
#define FH_STDERR 0x8003
int _sys_write (FILEHANDLE fh, const uint8_t *buf, uint32_t num, int mode)
{
  int ch;
  UNUSED(mode);
  UNUSED(ch);

  switch (fh)
  {
    case FH_STDIN :
      return ( -1);

#if 0
    case FH_STDOUT :
      for (; len; len--)
      {
        ch = *buf++;
        stdout_putchar(ch);
      }
      return (0);

    case FH_STDERR :
      for (; len; len--)
      {
        ch = *buf++;
        stderr_putchar(ch);
      }
      return (0);
#else
    case FH_STDOUT :
    case FH_STDERR :
      wd_print_write(buf, num);
      return (0);
#endif
  }

  return ( -1);
}
#endif

#elif defined(__ICCARM__)
#pragma module_name = "?__write"
size_t __write(int handle, const unsigned char *buffer, size_t size) {
    size_t nChars = 0;

    if (buffer == 0) {
        /*
     * This means that we should flush internal buffers.  Since we
     * don't we just return.  (Remember, "handle" == -1 means that all
     * handles should be flushed.)
     */
        return 0;
    }

    /* This template only writes to "standard out" and "standard err",
   * for all other file handles it returns failure. */
    if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR) {
        return _LLIO_ERROR;
    }

    for (/* Empty */; size != 0; --size) {
        wd_print_putc((uint8_t)*buffer);

        ++buffer;
        ++nChars;
    }

    return nChars;
}

#if 0
static size_t s_write_store_len = 0;
static unsigned char write_store_t[256];                                        // 100kB 296,568 us (256, 10Mbps)    100kB 308,063 us (80, 10Mbps)    100kB 1,844,618 us (1, 10Mbps)
/* Returns 0 or _LLIO_ERROR. */
static size_t write_flush(void)
{
  if (s_write_store_len > 0)
  {
    size_t size = s_write_store_len;
    s_write_store_len = 0;

    wd_print_write(write_store_t, size);

  }

  return 0;
}

#pragma required = __cstart_closeall
size_t wd_write_buffered(int handle, const unsigned char * buf, size_t size)
{
  /* Flush our buffer. */
  if (buf == 0)
  {
    if (   handle == _LLIO_STDOUT
        || handle == -1)
    {
      return write_flush();
    }
    return 0;
  }

  /* Only standard out is buffered. */
  if (handle != _LLIO_STDOUT)
  {
    return _LLIO_ERROR;
  }

  /*
   * In case the buf is larger than our store then the most efficient
   * we can do is to pass the entire buffer to __dwrite.  (We need to
   * flush our store first.)
   */

  if (size > sizeof(write_store_t))
  {
    if (write_flush() == _LLIO_ERROR)
    {
      return _LLIO_ERROR;
    }

    return size;
  }
  else
  {
    /*
     * Fill up a buffer, if there are more characters to be written,
     * or if the buffer contains newline characters, then flush the
     * buffer.
     */

    size_t charsLeft = size;

    while (charsLeft)
    {
      int foundNewline = 0;
      /* Fill buffer. */
      while (charsLeft && s_write_store_len < sizeof(write_store_t))
      {
        char ch = *buf++;
        if (ch == '\n')
        {
          foundNewline = 1;
        }
        write_store_t[s_write_store_len++] = ch;
        --charsLeft;
      }

      if (charsLeft || foundNewline)
      {
        if (write_flush() == _LLIO_ERROR)
        {
          return _LLIO_ERROR;
        }
      }
    }
    return size;
  }
}
#endif

size_t __read(int handle, unsigned char *buffer, size_t size) {
    int nChars = 0;

    /* This template only reads from "standard in", for all other file
   * handles it returns failure. */
    if (handle != _LLIO_STDIN) {
        return _LLIO_ERROR;
    }

    for (/* Empty */; size > 0; --size) {
        int c;

        c = (int)wd_print_waitc();

        if (c < 0) {
            break;
        }

        *buffer++ = c; // __iar_Stdin
        ++nChars;
    }

    return nChars;
}
#endif
#endif /// I/O Retargeting ****************************************************}

#if 1 /// print **************************************************************{

//__STATIC_INLINE int print_get_level(const char *buffer)
static inline int print_get_level(const char *buffer) {
    if (buffer[0] == WD_TERM_SOH_ASCII && buffer[1]) {
        if (WD_RANGE('0', buffer[1], '0' + WD_CONSOLE_LEVEL_FULL)) {
            return buffer[1];
        }
    }
    return 0;
}

//__STATIC_INLINE const char *print_skip_level(const char *buffer)
static inline const char *print_skip_level(const char *buffer) {
    if (print_get_level(buffer)) {
        return buffer + 2;
    }

    return buffer;
}

#if 0
//__STATIC_INLINE int print_get_control(const char *buffer)
static inline int print_get_control(const char *buffer)
{
  if ((buffer[0] == WD_TERM_DC1_ASCII) && (buffer[1] || buffer[2] || buffer[3]))
  {
      return (buffer[1] | (buffer[2] << 8) | (buffer[3] << 16));
  }
  return 0;
}

//__STATIC_INLINE const char *print_skip_control(const char *buffer)
static inline const char *print_skip_control(const char *buffer)
{
  if (printd_get_control(buffer))
    return buffer + 4;

  return buffer;
}
#endif

int wd_print_pre_control(uint32_t control, wd_printx_t printx_func) {
    if (printx_func == 0) {
        return (WD_CODE_ERROR);
    }

    if (WD_CORE_IS_IN_ISR()) {
        if (WD_CORE_IS_IRQ_MASKED()) {
            if (wd_core_system_mode >= WD_SYSTEM_MODE_NORMAL) {
                printx_func("\r\n" WD_COLOR_BG_RED_BOLD "[CRI]" WD_COLOR_RESET " ");
            }
        } else {
            printx_func("\r\n" WD_COLOR_BG_MAGENTA_BOLD "[ISR]" WD_COLOR_RESET " ");
        }
    }

    if (control & WD_MSG_CLEAR) {
        printx_func(WD_ANSI_CLEAR);
    }

    if (control & WD_MSG_FOREGROUND_COLOR_MSK) {
        printx_func(WD_ANSI "%d"
                            ";3"
                            "%d"
                            "m",
                    (control & WD_MSG_BOLD) ? 1 : 0,
                    (control & WD_MSG_FOREGROUND_COLOR_MSK) >> WD_MSG_FOREGROUND_COLOR_POS);
    }

    if (control & WD_MSG_BACKGROUND_COLOR_MSK) {
        printx_func(WD_ANSI "%d"
                            ";4"
                            "%d"
                            "m",
                    (control & WD_MSG_BG_BOLD) ? 1 : 0,
                    (control & WD_MSG_BACKGROUND_COLOR_MSK) >> WD_MSG_BACKGROUND_COLOR_POS);
    }

    if (control & WD_MSG_DELIMETER) {
        printx_func("=============================================================================="
                    "==========================================\r\n");
    }

    if (control & WD_MSG_TIME_STAMP) {
        struct DEBUG_TIME_WD_MSG_TIME {
            uint32_t Time;
            uint32_t Time_Stamp;
            uint32_t Time_ms;
            uint32_t Time_Stamp_ms;
            time_t Time_RTC_Time_Old;
            uint32_t Time_Sub_Second_Old;
        };
        static struct DEBUG_TIME_WD_MSG_TIME Console_MSG_Time = {
            0,
        };
        static struct DEBUG_TIME_WD_MSG_TIME Terminal_MSG_Time = {
            0,
        };

        time_t rtc_time;
        struct tm date_time;
        uint32_t sub_second = 0;
        struct DEBUG_TIME_WD_MSG_TIME *msg_time;

        if (printx_func == &printf) {
            msg_time = &Console_MSG_Time;
        } else {
            msg_time = &Terminal_MSG_Time;
        }

        msg_time->Time = wd_time_calc_us(&msg_time->Time_Stamp);
        msg_time->Time_Stamp = wd_time_get_us();
        msg_time->Time_ms = wd_time_calc_ms(&msg_time->Time_Stamp_ms);
        msg_time->Time_Stamp_ms = wd_time_get_ms();

        wd_time_sub_second_busy = true;

        if (!WD_CORE_IS_IN_ISR()) {
            wd_time_read(&rtc_time);
        } else {
            rtc_time = wd_time_time;
        }

        wd_time_sub_second_busy = false;

        wd_time_read_sub_handler();
        sub_second = (uint32_t)wd_time_read_sub_second();

        if (sub_second > 999999) {
            __NOP();
            sub_second = 999999;
            wd_time_sub_second_clk = DWT->CYCCNT;
        }

        if (msg_time->Time_RTC_Time_Old > rtc_time) {
            rtc_time++;
        }
        if (msg_time->Time_Sub_Second_Old > sub_second) {
            if (msg_time->Time_RTC_Time_Old == rtc_time) {
                rtc_time++;
            }
        }
        wd_time_time_to_tm(rtc_time, &date_time);

#if 0
    if(WD_MSG_TIME_STAMP_RTC_Time_Old * 1000000 + WD_MSG_TIME_STAMP_Sub_Second_Old >= rtc_time * 1000000 + sub_second)
    {
      if(!WD_CORE_IS_IN_ISR())
      {
        __NOP();
      }
    }
#endif

        msg_time->Time_RTC_Time_Old = rtc_time;
        msg_time->Time_Sub_Second_Old = sub_second;

        printx_func("[%04d/%02d/%02d ", date_time.tm_year + 1900, date_time.tm_mon + 1,
                    date_time.tm_mday);
        printx_func("%02d:%02d:%02d.%06d] ", date_time.tm_hour, date_time.tm_min, date_time.tm_sec,
                    sub_second);
        if (msg_time->Time_ms > 50000) {
            printx_func("[+%d.%06d] ", msg_time->Time_ms / 1000,
                        (msg_time->Time_ms % 1000) * 1000 + msg_time->Time % 1000);
        } else {
            printx_func("[+%d.%06d] ", msg_time->Time / 1000000, msg_time->Time % 1000000);
        }
    }

    return (WD_CODE_OK);
}

int wd_print_post_control(uint32_t control, wd_printx_t printx_func) {
    if (printx_func == 0) {
        return (WD_CODE_ERROR);
    }

    if (control & WD_MSG_COLOR_MSK) {
        printx_func(WD_COLOR_RESET);
    }

    if (control & WD_MSG_CRLF) {
        printx_func("\r\n");
    }

    return (WD_CODE_OK);
}

int wd_print_function(uint32_t control, wd_printx_t printx_func, const char *fmt, va_list *ap) {
    uint32_t count = 0;
    uint32_t magic;


    do {
        count++;

        if (count > 16) {
            return (0);
        }

        magic = va_arg(*ap, uint32_t);

        if (magic == WD_MSG_MAGIC) {
            count = 0;
            break;
        }
    } while (count);

    switch (control & WD_MSG_FUNCTION_MSK) {
    case WD_MSG_DUMP: {
        uint32_t frame_ptr;
        uint32_t frame_num;
        uint32_t frame_option = 0xFFFFFFFF;

        uint32_t i;
        char *s;

        frame_ptr = va_arg(*ap, uint32_t);
        frame_num = va_arg(*ap, uint32_t);
        frame_option = va_arg(*ap, uint32_t);

        s = (char *)frame_ptr;

        if (frame_ptr == 0) {
            printx_func("\r\n");
        } else {
            if (frame_option == WD_MSG_OPTION_HEX) {
                for (i = 0; i < frame_num; i++) {
                    printx_func("%02X", *s++);
                }
                //            printx_func("\r\n");
            } else if (frame_option == WD_MSG_OPTION_ASCII) {
                for (i = 0; i < frame_num; i++) {
                    if (*s == '\r') {
                        printx_func("\\r", *s++);
                    } else {
                        printx_func("%c", *s++);
                    }
                }
            } else {
                printx_func((char *)frame_ptr, frame_num);
            }
        }
    } break;

    case WD_MSG_MEM_DUMP: {
        uint32_t frame_addr;
        uint32_t frame_ptr;
        uint32_t frame_num;
        uint32_t frame_column;

        uint32_t i;
        char *s;

        frame_addr = va_arg(*ap, uint32_t);
        frame_ptr = va_arg(*ap, uint32_t);
        frame_num = va_arg(*ap, uint32_t);
        frame_column = va_arg(*ap, uint32_t);

        s = (char *)frame_ptr;

        if (frame_column == WD_MSG_OPTION_HEX) {
            for (i = 0; i < frame_num; i++) {
                printx_func("%02X", *s++);
            }
        } else if (frame_column == WD_MSG_OPTION_ASCII) {
            for (i = 0; i < frame_num; i++) {
                if (*s == '\r') {
                    printx_func("\\r", *s++);
                } else {
                    printx_func("%c", *s++);
                }
            }
        } else {
            printx_func(" [%d bytes]\r\n", frame_num);

            {
                if (frame_column == 0) {
                    frame_column = 128;
                }

                printx_func("              ");

                for (i = 0; i < frame_column; i++) {
                    printx_func("%02X ", i);
                }

                for (i = 0; i < frame_num; i++) {
                    if (i % frame_column == 0) {
                        printx_func("\r\n 0x%08X : ", frame_addr);
                    }
                    printx_func("%02X ", *s++);
                    frame_addr++;
                }
                printx_func("\r\n");
            }
        }
    } break;

    case WD_MSG_FRAME_DUMP: {
        uint32_t frame_type;
        uint32_t frame_ptr;
        uint32_t frame_num;

        frame_type = va_arg(*ap, uint32_t);
        frame_ptr = va_arg(*ap, uint32_t);
        frame_num = va_arg(*ap, uint32_t);

        switch (frame_type & WD_NET_TYPE_MSK) {
        case WD_NET_RX:
            printx_func(WD_COLOR_GREEN_BOLD " RX[");
            break;

        case WD_NET_RX_LOOP_BACK:
            printx_func(WD_COLOR_CYAN_BOLD " LB[");
            break;

        case WD_NET_ERROR:
            printx_func(WD_COLOR_RED_BOLD " [");
            break;

        case WD_NET_TX:
        default:
            printx_func(WD_COLOR_BLUE_BOLD " TX[");
            break;
        }

        printx_func("%d] : ", frame_num);

        if (frame_ptr != 0) {
            int i;
            for (i = 0; i < (int)frame_num; i++) {
                if ((frame_type & WD_NET_ARGUMENT_MSK) == WD_NET_OPTION_ASCII) {
                    if (isprint((int)(((char *)frame_ptr)[i])) != 0) {
                        printx_func("%c", ((char *)frame_ptr)[i]);
                    }
                } else {
                    printx_func("%02X", ((uint8_t *)frame_ptr)[i]);
                }
            }
        }

        printx_func(WD_COLOR_RESET);
    } break;

    case WD_MSG_HEX_TO_ASCII: {
        uint32_t frame_ptr;
        uint32_t frame_num;

        frame_ptr = va_arg(*ap, uint32_t);
        frame_num = va_arg(*ap, uint32_t);

        if (fmt == 0) {
            printx_func("\r\n");
        } else {
            printx_func(fmt);
        }

        printx_func(" [%u] : ", frame_num);

        if (frame_ptr != 0) {
            int i;
            for (i = 0; i < (int)frame_num; i++) {
                printx_func("%02X", ((uint8_t *)frame_ptr)[i]);
            }
        }
    } break;
    default:
        break;
    }

    return (0);
}

#endif /// print **************************************************************}

#if 1 /// printd *************************************************************{
//int Console_Level = WD_CONF_CONSOLE_LEVEL_DEFAULT;
int wd_print_level; // = WD_CONF_CONSOLE_LEVEL_DEFAULT;
#if (RTE_RTOS)
uint32_t wd_print_owner = 0;
#endif

int wd_print_printd(uint32_t control, const char *fmt, ...) {
    va_list args;
    int printed = 0;
    int loglevel;
    UNUSED(control);

    loglevel = print_get_level(fmt);

    if (WD_RANGE('0', loglevel, '0' + WD_CONSOLE_LEVEL_FULL)) {
        loglevel -= '0';
    } else {
        loglevel = -1;
    }

    if (loglevel >= wd_print_level) {
        return 0;
    }

#if (RTE_RTOS)
    if (wd_print_owner != 0) {
        if (wd_print_owner != (uint32_t)(osThreadGetId())) {
            return 0;
        }
    }
#endif


    fmt = print_skip_level(fmt);


#if (RTE_RTOS) && (WD_CONF_CONSOLE_MUTEX)
    if (!WD_CORE_IS_IN_ISR()) {
        if (osMutexAcquire(wd_os_mutex_printd, WD_CONF_CONSOLE_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
        }
    }
#endif

    if (control) {
        if (wd_print_pre_control(control, &printf) < 0) {
            WD_BREAK();
            goto __end__;
        }
    }

    /*--------------- printf ---------------*/
    va_start(args, fmt);
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    printed = vprintf(fmt, args);
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    if (control & WD_MSG_FUNCTION_MSK) {
        fmt += printed;
        printed += wd_print_function(control, &printf, fmt, &args);
    }

    va_end(args);

    if (control) {
        if (wd_print_post_control(control, &printf) < 0) {
            WD_BREAK();
            goto __end__;
        }
    }

__end__:
#if (RTE_RTOS) && (WD_CONF_CONSOLE_MUTEX)
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(wd_os_mutex_printd);
    }
#endif

    return printed;
}
#endif /// printd *************************************************************}

#if 1 /// printx *************************************************************{
#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#elif defined(__ICCARM__)
#pragma diag_suppress = Pa093
#endif
#define MBED_CONF_PLATFORM_MINIMAL_PRINTF_ENABLE_FLOATING_POINT           1
#define MBED_CONF_PLATFORM_MINIMAL_PRINTF_SET_FLOATING_POINT_MAX_DECIMALS 6
#define MBED_CONF_PLATFORM_MINIMAL_PRINTF_ENABLE_64_BIT                   1

/**
 * Check architecture and choose storage data type.
 * On 32 bit machines, the default storage type is 32 bit wide
 * unless 64 bit integers are enabled in the configuration.
 */
#if INTPTR_MAX == INT32_MAX
#define MBED_SIGNED_NATIVE_TYPE   int32_t
#define MBED_UNSIGNED_NATIVE_TYPE uint32_t
#if MBED_CONF_PLATFORM_MINIMAL_PRINTF_ENABLE_64_BIT
#define MBED_SIGNED_STORAGE   int64_t
#define MBED_UNSIGNED_STORAGE uint64_t
#else
#define MBED_SIGNED_STORAGE   int32_t
#define MBED_UNSIGNED_STORAGE uint32_t
#endif

#elif INTPTR_MAX == INT64_MAX
#define MBED_SIGNED_NATIVE_TYPE   int64_t
#define MBED_UNSIGNED_NATIVE_TYPE uint64_t
#define MBED_SIGNED_STORAGE       int64_t
#define MBED_UNSIGNED_STORAGE     uint64_t
#else
#error unsupported architecture
#endif

/**
 * Precision defines
 */
#define PRECISION_DEFAULT (INT_MAX)

/**
 * Enum for storing width modifier.
 */
typedef enum {
    LENGTH_NONE = 0x00,
    LENGTH_H = 0x11,
    LENGTH_L = 0x21,
    LENGTH_J = 0x31,
    LENGTH_Z = 0x41,
    LENGTH_T = 0x51,
    LENGTH_CAPITAL_L = 0x61,
    LENGTH_HH = 0x72,
    LENGTH_LL = 0x82
} length_t;

/**
 * Enum for integer printing type
 */
typedef enum {
    INT_UNSIGNED,
    INT_SIGNED,
    HEX_LOWER,
    HEX_UPPER,
    ZERO_NEGATIVE /* special case when printing integer part of double values where it is 0 and the value is negative */
} integer_type_t;

/**
 * Prototypes
 */
static void mbed_minimal_formatted_string_integer(char *buffer, size_t length, int *result,
                                                  MBED_UNSIGNED_STORAGE value, integer_type_t type,
                                                  int width_size, bool prepend_zeros,
                                                  wd_write_func_t write_func);
static void mbed_minimal_formatted_string_void_pointer(char *buffer, size_t length, int *result,
                                                       const void *value,
                                                       wd_write_func_t write_func);
static void mbed_minimal_formatted_string_string(char *buffer, size_t length, int *result,
                                                 const char *string, size_t precision,
                                                 wd_write_func_t write_func);

/**
 * @brief      Print a single character, checking for buffer and size overflows.
 *
 * @param      buffer  The buffer to store output (NULL for stdout).
 * @param[in]  length  The length of the buffer.
 * @param      result  The current output location.
 * @param[in]  data    The char to be printed.
 */
static void mbed_minimal_putchar(char *buffer, size_t length, int *result, char data,
                                 wd_write_func_t write_func) {
    /* only continue if 'result' doesn't overflow */
    if ((*result >= 0) && (*result <= INT_MAX - 1)) {
        if (write_func) {
            write_func(&data, 1, 0);
            *result += 1;
        } else {
            if (buffer) {
                /* write data only if there's enough space */
                if ((size_t)*result < length) {
                    buffer[*result] = data;
                }
            }

            /* increment 'result' even if data was not written. This ensures that
         'wd_print_mbed_minimal_formatted_string' returns the correct value. */
            *result += 1;
        }
    }
}

/**
 * @brief      Print integer in signed, unsigned or hexadecimal format.
 *
 * @param      buffer           The buffer to store output (NULL for stdout).
 * @param[in]  length           The length of the buffer.
 * @param      result           The current output location.
 * @param[in]  value            The value to be printed.
 * @param      type             The type of integer format that shall be printed (signed, unsigend or hexadecimal)
 * @param      width_size       The width modifier.
 * @param      prepend_zeros    Flag to prepends zeros when the width_size is greater than 0
 */
static void mbed_minimal_formatted_string_integer(char *buffer, size_t length, int *result,
                                                  MBED_UNSIGNED_STORAGE value, integer_type_t type,
                                                  int width_size, bool prepend_zeros,
                                                  wd_write_func_t write_func) {
    /* allocate 3 digits per byte */
    char scratch[sizeof(MBED_UNSIGNED_STORAGE) * 3] = {0};

    int index = 0;

    bool negative_value = false;

    const char filler = prepend_zeros ? '0' : ' ';

    if (type == INT_SIGNED) {
        if ((MBED_SIGNED_STORAGE)value < 0) {
            /* get absolute value using two's complement */
            value = ~value + 1;
            negative_value = true;
        }
    } else if (type == ZERO_NEGATIVE) {
        negative_value = true;
    }

    if (value == 0) {
        scratch[index] = '0';
        index++;
    } else {
        /* write numbers in reverse order to scratch pad */
        for (; value > 0; index++) {
            if (type == HEX_LOWER || type == HEX_UPPER) {
                /* get least significant byte */
                const uint8_t output = value & 0x0F;

                static const char int2hex_lower[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                                       '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
                static const char int2hex_upper[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                                       '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

                if (type == HEX_LOWER) {
                    scratch[index] = int2hex_lower[output];
                } else {
                    scratch[index] = int2hex_upper[output];
                }

                /* shift value one byte position */
                value = value >> 4;
            } else {
                /* use '0' as base and add digit */
                scratch[index] = (char)('0' + (value % 10));

                /* shift value one decimal position */
                value = value / 10;
            }
        }
    }

    if (negative_value) {
        if (prepend_zeros) {
            mbed_minimal_putchar(buffer, length, result, '-', write_func);
        }
        index++; // add one to index to count '-'
    }

    // print filler characters
    if (width_size > index) {
        for (int i = width_size; i > index; i--) {
            mbed_minimal_putchar(buffer, length, result, filler, write_func);
        }
    }

    if (negative_value) {
        if (!prepend_zeros) {
            mbed_minimal_putchar(buffer, length, result, '-', write_func);
        }
        index--; // Restore index to correct position
    }

    /* print absolute value of integer */
    for (; index > 0; index--) {
        mbed_minimal_putchar(buffer, length, result, scratch[index - 1], write_func);
    }
}

/**
 * @brief      Print pointer.
 *
 * @param      buffer  The buffer to store output (NULL for stdout).
 * @param[in]  length  The length of the buffer.
 * @param      result  The current output location.
 * @param[in]  value   The pointer to be printed.
 */
static void mbed_minimal_formatted_string_void_pointer(char *buffer, size_t length, int *result,
                                                       const void *value,
                                                       wd_write_func_t write_func) {
    /* write leading 0x */
    mbed_minimal_putchar(buffer, length, result, '0', write_func);
    mbed_minimal_putchar(buffer, length, result, 'x', write_func);

    /* write rest as a regular hexadecimal number */
    mbed_minimal_formatted_string_integer(buffer, length, result,
                                          (MBED_UNSIGNED_STORAGE)(ptrdiff_t)value, HEX_UPPER, 0,
                                          false, write_func);
}

#if MBED_CONF_PLATFORM_MINIMAL_PRINTF_ENABLE_FLOATING_POINT
/**
 * @brief      Write double.
 *
 * @param      buffer          The buffer to store output (NULL for stdout).
 * @param[in]  length          The length of the buffer.
 * @param      result          The current output location.
 * @param[in]  value           The value to be printed.
 * @param[in]  dec_precision   The decimal precision. If PRECISION_DEFAULT MBED_CONF_PLATFORM_MINIMAL_PRINTF_SET_FLOATING_POINT_MAX_DECIMALS is used.
 * @param      width_size       The width modifier.
 * @param      prepend_zeros    Flag to prepends zeros when the width_size is greater than 0
 */
static void mbed_minimal_formatted_string_double(char *buffer, size_t length, int *result,
                                                 double value, int dec_precision, int width_size,
                                                 bool prepend_zeros, wd_write_func_t write_func) {
    /* get integer part */
    MBED_SIGNED_STORAGE integer = (MBED_SIGNED_STORAGE)value;
    /* fractional part represented as the int that will be formatted after the dot, e.g. 95 for 1.95 */
    MBED_SIGNED_STORAGE decimal = 0;

    if (dec_precision == PRECISION_DEFAULT) {
        dec_precision = MBED_CONF_PLATFORM_MINIMAL_PRINTF_SET_FLOATING_POINT_MAX_DECIMALS;
    }

    if (dec_precision != 0) {
        /* get decimal part */
        MBED_SIGNED_STORAGE precision = 1;
        for (int index = 0; index < dec_precision; index++) {
            precision *= 10;
        }

        /* Multiply the frac part so we get an int value with the required accuracy.
       E.g. For 0.1234 and dec_precision=3 you'd get 123.4 */
        double decimal_double = (value - (double)integer) * (double)precision;
        if (value < 0) {
            /* The part after the dot does not have a sign, so negate the value before rounding */
            decimal = (MBED_SIGNED_STORAGE)(-decimal_double + 0.5);
            if (decimal >= precision) {
                /* Rounding carries over to value's integer part (e.g. -1.95 with dec_precision=1 -> -2.0) */
                integer--;
                decimal = 0;
            }
        } else {
            /* Round the value */
            decimal = (MBED_SIGNED_STORAGE)(decimal_double + 0.5);
            if (decimal >= precision) {
                /* Rounding carries over to value's integer part (e.g. 1.95 with dec_precision=1 -> 2.0) */
                integer++;
                decimal = 0;
            }
        }

        width_size -= dec_precision + 1; // decimal precision plus '.'
        if (width_size < 0) {
            width_size = 0;
        }
    } else {
        value = (value - (double)integer) * 1.0;
        if (value > 0.5) {
            integer++;
        } else if (value < -0.5) {
            integer--;
        }
    }

    /* write integer part */
    if (integer == 0 && value < 0) {
        mbed_minimal_formatted_string_integer(buffer, length, result,
                                              (MBED_UNSIGNED_STORAGE)integer, ZERO_NEGATIVE,
                                              width_size, prepend_zeros, write_func);
    } else {
        mbed_minimal_formatted_string_integer(buffer, length, result,
                                              (MBED_UNSIGNED_STORAGE)integer, INT_SIGNED,
                                              width_size, prepend_zeros, write_func);
    }

    if (dec_precision != 0) {
        /* write decimal point */
        mbed_minimal_putchar(buffer, length, result, '.', write_func);
        /* write decimal part */
        mbed_minimal_formatted_string_integer(buffer, length, result,
                                              (MBED_UNSIGNED_STORAGE)decimal, INT_UNSIGNED,
                                              dec_precision, true, write_func);
    }
}
#endif

/**
 * @brief      Print string with precision.
 *
 * @param      buffer     The buffer to store output (NULL for stdout).
 * @param[in]  length     The length of the buffer.
 * @param      result     The current output location.
 * @param[in]  value      The string to be printed.
 * @param[in]  precision  The maximum number of characters to be printed.
 */
static void mbed_minimal_formatted_string_string(char *buffer, size_t length, int *result,
                                                 const char *string, size_t precision,
                                                 wd_write_func_t write_func) {
    while ((*string != '\0') && (precision)) {
        mbed_minimal_putchar(buffer, length, result, *string, write_func);
        string++;
        precision--;
    }
}

/**
 * @brief Parse a string to an integer value as long as there are numerical characters in the string
 *
 * @param[in]   string      The input string. Has to begin with a numerical character to parse
 * @param[out]  value       The output value.
 * @return      size_t      The number of numerical characters parsed
 */
static size_t parse_string_to_integer(const char *string, int *value) {
    size_t inner_index = 0;

    while ((string[inner_index] >= '0') && (string[inner_index] <= '9')) {
        *value = *value * 10 + (string[inner_index] - '0');

        inner_index++;
    }

    return inner_index;
}

/**
 * @brief      Parse formatted string and invoke write handlers based on type.
 *
 * @param      buffer     The buffer to write to, write to stdout if NULL.
 * @param[in]  length     The length of the buffer.
 * @param[in]  format     The formatted string.
 * @param[in]  arguments  The va_list arguments.
 *
 * @return     Number of characters written.
 */
int wd_print_mbed_minimal_formatted_string(char *buffer, size_t length, const char *format,
                                           va_list arguments, wd_write_func_t write_func) {
    int result = 0;
    bool empty_buffer = false;

    /* ensure that function wasn't called with an empty buffer, or with or with
     a buffer size that is larger than the maximum 'int' value, or with
     a NULL format specifier */
    if (format && length <= INT_MAX) {
        /* Make sure that there's always space for the NULL terminator */
        if (length > 0) {
            length--;
        } else {
            /* the buffer is empty, there's no place to write the terminator */
            empty_buffer = true;
        }
        /* parse string */
        for (size_t index = 0; format[index] != '\0'; index++) {
            /* format specifier begin */
            if (format[index] == '%') {
                size_t next_index = index + 1;

                /**************************************************************
         * skip and ignore flags [-+(space)#]
         *************************************************************/
                if ((format[next_index] == '-') || (format[next_index] == '+') ||
                    (format[next_index] == ' ') || (format[next_index] == '#')) {
                    /* skip to next character */
                    next_index++;
                }

                /**************************************************************
         * look for width and prepending zeros [(number)], skip [*]
         *************************************************************/
                bool prepend_zeros = false;
                int width_size = 0;

                if (format[next_index] == '*') {
                    /* skip to next character */
                    next_index++;

                    /* discard argument */
                    va_arg(arguments, MBED_SIGNED_NATIVE_TYPE);
                } else {
                    if (format[next_index] == '0') {
                        prepend_zeros = true;
                        do {
                            next_index++;
                        } while (format[next_index] == '0');
                    }

                    /* parse width modifier until not a decimal */
                    next_index += parse_string_to_integer(&format[next_index], &width_size);
                }

                /**************************************************************
         * look for precision modifier
         *************************************************************/
                int precision = PRECISION_DEFAULT;

                if ((format[next_index] == '.') && (format[next_index + 1] == '*')) {
                    next_index += 2;

                    /* read precision from argument list */
                    precision = va_arg(arguments, MBED_SIGNED_NATIVE_TYPE);
                } else if (format[next_index] == '.') {
                    /* precision modifier found, reset default to 0 and increment index */
                    next_index++;
                    precision = 0;

                    /* parse precision until not a decimal */
                    next_index += parse_string_to_integer(&format[next_index], &precision);
                }

                /**************************************************************
         * look for length modifier, default to NONE
         *************************************************************/
                length_t length_modifier = LENGTH_NONE;

                /* look for two character length modifier */
                if ((format[next_index] == 'h') && (format[next_index + 1] == 'h')) {
                    length_modifier = LENGTH_HH;
                } else if ((format[next_index] == 'l') && (format[next_index + 1] == 'l')) {
                    length_modifier = LENGTH_LL;
                }
                /* look for one character length modifier if two character search failed */
                else if (format[next_index] == 'h') {
                    length_modifier = LENGTH_H;
                } else if (format[next_index] == 'l') {
                    length_modifier = LENGTH_L;
                } else if (format[next_index] == 'j') {
                    length_modifier = LENGTH_J;
                } else if (format[next_index] == 'z') {
                    length_modifier = LENGTH_Z;
                } else if (format[next_index] == 't') {
                    length_modifier = LENGTH_T;
                } else if (format[next_index] == 'L') {
                    length_modifier = LENGTH_CAPITAL_L;
                }

                /* increment index, length is encoded in modifier enum */
                next_index += (length_modifier & 0x0F);

                /**************************************************************
         * read out character - this is a supported format character,
         * '\0', or a not suported character
         *************************************************************/
                char next = format[next_index];

                /* signed integer */
                if ((next == 'd') || (next == 'i')) {
                    MBED_SIGNED_STORAGE value = 0;

#if MBED_CONF_PLATFORM_MINIMAL_PRINTF_ENABLE_64_BIT
                    /* if 64 bit is enabled and the integer types are larger than the native type */
                    if (((length_modifier == LENGTH_LL) &&
                         (sizeof(long long int) > sizeof(MBED_SIGNED_NATIVE_TYPE))) ||
                        ((length_modifier == LENGTH_L) &&
                         (sizeof(long int) > sizeof(MBED_SIGNED_NATIVE_TYPE))) ||
                        ((length_modifier == LENGTH_NONE) &&
                         (sizeof(int) > sizeof(MBED_SIGNED_NATIVE_TYPE)))) {
                        /* use 64 bit storage type for readout */
                        value = va_arg(arguments, MBED_SIGNED_STORAGE);
                    } else
#else
                    /* If 64 bit is not enabled, print %ll[di] rather than truncated value */
                    if (length_modifier == LENGTH_LL) {
                        mbed_minimal_putchar(buffer, length, &result, '%', write_func);
                        if (next == '%') {
                            // Continue printing loop after `%`
                            index = next_index;
                        }
                        continue;
                    }
#endif
                    {
                        /* use native storage type (which can be 32 or 64 bit) */
                        value = va_arg(arguments, MBED_SIGNED_NATIVE_TYPE);
                    }

                    /* constrict value based on length modifier */
                    switch (length_modifier) {
                    case LENGTH_NONE:
                        value = (int)value;
                        break;
                    case LENGTH_HH:
                        value = (signed char)value;
                        break;
                    case LENGTH_H:
                        value = (short int)value;
                        break;
                    case LENGTH_L:
                        value = (long int)value;
                        break;
                    case LENGTH_LL:
                        value = (long long int)value;
                        break;
                    case LENGTH_J:
                        value = (intmax_t)value;
                        break;
                    case LENGTH_T:
                        value = (ptrdiff_t)value;
                        break;
                    default:
                        break;
                    }

                    index = next_index;

                    mbed_minimal_formatted_string_integer(buffer, length, &result,
                                                          (MBED_UNSIGNED_STORAGE)value, INT_SIGNED,
                                                          width_size, prepend_zeros, write_func);
                }
                /* unsigned integer */
                else if ((next == 'u') || (next == 'x') || (next == 'X')) {
                    MBED_UNSIGNED_STORAGE value = 0;

#if MBED_CONF_PLATFORM_MINIMAL_PRINTF_ENABLE_64_BIT
                    /* if 64 bit is enabled and the integer types are larger than the native type */
                    if (((length_modifier == LENGTH_LL) &&
                         (sizeof(unsigned long long int) > sizeof(MBED_UNSIGNED_NATIVE_TYPE))) ||
                        ((length_modifier == LENGTH_L) &&
                         (sizeof(unsigned long int) > sizeof(MBED_UNSIGNED_NATIVE_TYPE))) ||
                        ((length_modifier == LENGTH_NONE) &&
                         (sizeof(unsigned int) > sizeof(MBED_UNSIGNED_NATIVE_TYPE)))) {
                        /* use 64 bit storage type for readout */
                        value = va_arg(arguments, MBED_UNSIGNED_STORAGE);
                    } else
#else
                    /* If 64 bit is not enabled, print %ll[uxX] rather than truncated value */
                    if (length_modifier == LENGTH_LL) {
                        mbed_minimal_putchar(buffer, length, &result, '%', write_func);
                        if (next == '%') {
                            // Continue printing loop after `%`
                            index = next_index;
                        }
                        continue;
                    }
#endif
                    {
                        /* use native storage type (which can be 32 or 64 bit) */
                        value = va_arg(arguments, MBED_UNSIGNED_NATIVE_TYPE);
                    }

                    /* constrict value based on length modifier */
                    switch (length_modifier) {
                    case LENGTH_NONE:
                        value = (unsigned int)value;
                        break;
                    case LENGTH_HH:
                        value = (unsigned char)value;
                        break;
                    case LENGTH_H:
                        value = (unsigned short int)value;
                        break;
                    case LENGTH_L:
                        value = (unsigned long int)value;
                        break;
                    case LENGTH_LL:
                        value = (unsigned long long int)value;
                        break;
                    case LENGTH_J:
                        value = (uintmax_t)value;
                        break;
                    case LENGTH_Z:
                        value = (size_t)value;
                        break;
                    case LENGTH_T:
                        value = (MBED_UNSIGNED_STORAGE)(ptrdiff_t)value;
                        break;
                    default:
                        break;
                    }

                    index = next_index;

                    /* write unsigned or hexadecimal */
                    if (next == 'u') {
                        mbed_minimal_formatted_string_integer(buffer, length, &result, value,
                                                              INT_UNSIGNED, width_size,
                                                              prepend_zeros, write_func);
                    } else if (next == 'X') {
                        mbed_minimal_formatted_string_integer(buffer, length, &result, value,
                                                              HEX_UPPER, width_size, prepend_zeros,
                                                              write_func);
                    } else {
                        mbed_minimal_formatted_string_integer(buffer, length, &result, value,
                                                              HEX_LOWER, width_size, prepend_zeros,
                                                              write_func);
                    }
                }
#if MBED_CONF_PLATFORM_MINIMAL_PRINTF_ENABLE_FLOATING_POINT
                /* treat all floating points the same */
                else if ((next == 'f') || (next == 'F') || (next == 'g') || (next == 'G')) {
                    double value = va_arg(arguments, double);
                    index = next_index;

                    mbed_minimal_formatted_string_double(buffer, length, &result, value, precision,
                                                         width_size, prepend_zeros, write_func);
                }
#endif
                /* character */
                else if (next == 'c') {
                    char value = (char)va_arg(arguments, MBED_SIGNED_NATIVE_TYPE);
                    index = next_index;

                    mbed_minimal_putchar(buffer, length, &result, value, write_func);
                }
                /* string */
                else if (next == 's') {
                    char *value = va_arg(arguments, char *);
                    index = next_index;

                    mbed_minimal_formatted_string_string(buffer, length, &result, value,
                                                         (size_t)precision, write_func);
                }
                /* pointer */
                else if (next == 'p') {
                    void *value = va_arg(arguments, void *);
                    index = next_index;

                    mbed_minimal_formatted_string_void_pointer(buffer, length, &result, value,
                                                               write_func);
                } else {
                    // Unrecognised, or `%%`. Print the `%` that led us in.
                    mbed_minimal_putchar(buffer, length, &result, '%', write_func);
                    if (next == '%') {
                        // Continue printing loop after `%%`
                        index = next_index;
                    }
                    // Otherwise we continue the printing loop after the leading `%`, so an
                    // unrecognised thing like "Blah = %a" will just come out as "Blah = %a"
                }
            } else
            /* not a format specifier */
            {
                /* write normal character */
                mbed_minimal_putchar(buffer, length, &result, format[index], write_func);
            }
        }

        if (buffer && !empty_buffer) {
            /* NULL-terminate the buffer no matter what. We use '<=' to compare instead of '<'
         because we know that we initially reserved space for '\0' by decrementing length */
            if ((size_t)result <= length) {
                buffer[result] = '\0';
            } else {
                buffer[length] = '\0';
            }
        }
    }

    return result;
}

int wd_print_vprintx(wd_terminal_t *term, const char *format, va_list arguments) {
    return wd_print_mbed_minimal_formatted_string(0, LONG_MAX, format, arguments, term->Write);
}

int wd_print_printx(wd_terminal_t *term, uint32_t control, const char *fmt, ...) {
    va_list args;
    int printed = 0;
    int loglevel;
    UNUSED(term);
    UNUSED(control);

    loglevel = print_get_level(fmt);

    if (WD_RANGE('0', loglevel, '0' + WD_CONSOLE_LEVEL_FULL)) {
        loglevel -= '0';
    } else {
        loglevel = -1;
    }

    if (loglevel >= wd_print_level) {
        return 0;
    }


    fmt = print_skip_level(fmt);


#if (RTE_RTOS) && (RTE_PLATFORM_PRINT_USE_MUTEX_PRINTD)
    if (!WD_CORE_IS_IN_ISR()) {
        if (osMutexAcquire(Mutex_printx, WD_CONF_CONSOLE_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
        }
    }
#endif

    if (control) {
        if (wd_print_pre_control(control, term->Print) < 0) {
            WD_BREAK();
            goto __end__;
        }
    }

    /*--------------- printf ---------------*/
    va_start(args, fmt);
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    printed = wd_print_vprintx(term, fmt, args);
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    if (control & WD_MSG_FUNCTION_MSK) {
        fmt += printed;
        printed += wd_print_function(control, term->Print, fmt, &args);
    }

    va_end(args);

    if (control) {
        if (wd_print_post_control(control, term->Print) < 0) {
            WD_BREAK();
            goto __end__;
        }
    }

__end__:
#if (RTE_RTOS) && (RTE_PLATFORM_PRINT_USE_MUTEX_PRINTD)
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(Mutex_printx);
    }
#endif

    return printed;
}

#if 0
int wd_print_printx(const char *format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  int result = wd_print_mbed_minimal_formatted_string(0, LONG_MAX, format, arguments, wd_print_write);
  va_end(arguments);

  return result;
}

int wd_print_vprintx(const char *format, va_list arguments)
{
  return wd_print_mbed_minimal_formatted_string(0, LONG_MAX, format, arguments, wd_print_write);
}

int sprintx(char *buffer, const char *format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  int result = wd_print_mbed_minimal_formatted_string(buffer, LONG_MAX, format, arguments, 0);
  va_end(arguments);

  return result;
}

int snprintx(char *buffer, size_t length, const char *format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  int result = wd_print_mbed_minimal_formatted_string(buffer, length, format, arguments, 0);
  va_end(arguments);

  return result;
}

int vsprintx(char *buffer, const char *format, va_list arguments)
{
  return wd_print_mbed_minimal_formatted_string(buffer, LONG_MAX, format, arguments, 0);
}

int vsnprintx(char *buffer, size_t length, const char *format, va_list arguments)
{
  return wd_print_mbed_minimal_formatted_string(buffer, length, format, arguments, 0);
}
#endif


#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(__ICCARM__)
#pragma diag_default = Pa093
#endif
#endif /// printx *************************************************************}

/** @} end of file */
