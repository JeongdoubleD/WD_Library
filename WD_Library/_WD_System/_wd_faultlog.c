/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _fault_log.c
 *
 *  Created on: Jan 14, 2026
 *      Author: doubled
 */


#include "wd.h"
#include "stm32h7xx.h"
#include <stdio.h>

volatile fault_log_t wd_faultlog_data __attribute__((section(".noinit")));

void wd_faultlog_set_checkpoint(uint32_t id) {
    wd_faultlog_data.checkpoint = id;
    __DSB();
    __ISB();
}

void wd_faultlog_clear(void) {
    wd_faultlog_data.magic = 0;
    wd_faultlog_data.version = 0;
    wd_faultlog_data.checkpoint = 0;
}

void wd_faultlog_dump_if_present(void) {
    if (wd_faultlog_data.magic != 0xDEADBEEFu || wd_faultlog_data.version != 1u) {
        return;
    }

    // 여기서부터는 UART가 초기화된 뒤에 호출해야 함
    printf("\r\n=== FAULT LOG DETECTED ===\r\n");
    printf("checkpoint : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.checkpoint);
    printf("exc_return : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.exc_return);
    printf("msp/psp    : 0x%08lX / 0x%08lX\r\n", (unsigned long)wd_faultlog_data.msp,
           (unsigned long)wd_faultlog_data.psp);

    printf("stacked r0 : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.r0);
    printf("stacked r1 : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.r1);
    printf("stacked r2 : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.r2);
    printf("stacked r3 : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.r3);
    printf("stacked r12: 0x%08lX\r\n", (unsigned long)wd_faultlog_data.r12);
    printf("stacked lr : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.lr);
    printf("stacked pc : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.pc);
    printf("stacked xpsr:0x%08lX\r\n", (unsigned long)wd_faultlog_data.xpsr);

    printf("CFSR       : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.cfsr);
    printf("HFSR       : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.hfsr);
    printf("DFSR       : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.dfsr);
    printf("AFSR       : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.afsr);
    printf("MMFAR      : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.mmfar);
    printf("BFAR       : 0x%08lX\r\n", (unsigned long)wd_faultlog_data.bfar);
    printf("=== END FAULT LOG ===\r\n");

    // 한번 출력했으면 지워서 다음 부팅에 반복 출력 방지(원하면 유지해도 됨)
    wd_faultlog_clear();
}
