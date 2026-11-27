/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 *//**
 *  \version         V1.0.0 [MDK 5.37]
 *  \author
 ** @{
 ******************************************************************************/

#include "wd.h"

#if (RTE_PLATFORM_PRINT_SERIAL) /* module switch */

void wd_print_serial_write_serial(const void *data, uint32_t num, uint32_t arg) {
    const char *buffer = data;
    size_t size = num;

    UNUSED(arg);

    for (/* Empty */; size != 0; --size) {
        while (!LL_USART_IsActiveFlag_TXE(RTE_PLATFORM_PRINT_SERIAL_DRIVER_REGISTER))
            ;
        LL_USART_TransmitData8(RTE_PLATFORM_PRINT_SERIAL_DRIVER_REGISTER, *buffer++);
    }
}

// extern int stdout_putchar (int ch)   //24.12.24
// {
//   uint8_t buf[1];

//   buf[0] = (uint8_t) ch;
//   //LPUART
//   while ( !LL_LPUART_IsActiveFlag_TXE(LPUART1));
//   LL_LPUART_TransmitData8(LPUART1, buf[0]);
//   //USART
// //  while ( !LL_USART_IsActiveFlag_TXE(LPUART1));
// //  LL_USART_TransmitData8(LPUART1, buf[0]);
//   return (ch);
// }

extern int stdout_puts(const char *str) {
    if ((str[0] != 0x1b) || (str[1] != 0x5b) || (str[2] != 0x44)) {
        __NOP();
    }
    while (*str) {
        stdout_putchar(*str++);
    }

    return (0);
}

extern char wd_itoa(uint8_t number) {
    if (number <= 9) {
        return ('0' + number);
    } else if (number <= 16) {
        return ('A' + number - 10);
    } else {
        return ('?');
    }
}

extern void stdout_put_hex(char *msg, const uint8_t *hex, uint32_t len) {
    int i;
    uint8_t buf[2];

    stdout_puts(msg);
    stdout_puts(" : ");

    for (i = 0; (uint32_t)i < len; i++) {
        buf[0] = hex[i] >> 4;
        buf[1] = hex[i] & 0x0F;
        stdout_putchar(wd_itoa(buf[0]));
        stdout_putchar(wd_itoa(buf[1]));
    }

    stdout_puts("\r\n");
}


/** @} end of file */

#endif /* (RTE_PLATFORM_PRINT_SERIAL) */
