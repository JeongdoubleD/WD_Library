/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _fault_log.h
 *
 *  Created on: Jan 14, 2026
 *      Author: doubled
 */

#ifndef WD_SYSTEM_WD_FAULTLOG_H
#define WD_SYSTEM_WD_FAULTLOG_H

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t magic;   // 0xDEADBEEF
    uint32_t version; // 1

    // Breadcrumb: 마지막 실행 지점 추적
    uint32_t checkpoint;

    // Exception context
    uint32_t exc_return; // LR 값(EXC_RETURN)
    uint32_t msp;
    uint32_t psp;

    // Stacked registers (자동 push된 프레임)
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;

    // SCB fault status registers
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t dfsr;
    uint32_t afsr;
    uint32_t mmfar;
    uint32_t bfar;

} fault_log_t;

// .noinit에 실제 로그 인스턴스
extern volatile fault_log_t wd_faultlog_data;

void wd_faultlog_set_checkpoint(uint32_t id);
void wd_faultlog_dump_if_present(void); // 부팅 초기에 호출 (UART printf 가능할 때)
void wd_faultlog_clear(void);

#ifdef __cplusplus
}
#endif


#endif /* WD_SYSTEM_WD_FAULTLOG_H */
