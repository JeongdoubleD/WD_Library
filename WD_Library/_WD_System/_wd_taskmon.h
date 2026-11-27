/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _Task_monitor.h
 *
 *  Created on: 2026-03-30
 *      Author: doubled
 *
 *  RTOS 스레드 스택 / 힙 모니터링 모듈
 *
 *  요구 FreeRTOSConfig.h 설정:
 *    configUSE_TRACE_FACILITY              1   (uxTaskGetSystemState 사용)
 *    INCLUDE_uxTaskGetStackHighWaterMark   1   (Stack HWM 사용)
 *    USE_FreeRTOS_HEAP_4                       (xPortGetMinimumEverFreeHeapSize 사용)
 */

#ifndef WD_SYSTEM_WD_TASKMON_H
#define WD_SYSTEM_WD_TASKMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "wd.h"

#if (RTE_RTOS)

/***************************************************************************************************************************************************************
 **********  Configuration
 **************************************************************************************************************************************************************/
#define WD_TASK_MONITOR_MAX_THREADS      16u
#define WD_TASK_MONITOR_STACK_WARN_BYTES 256u /* 잔여 스택 이 값 이하 → WARNING  */
#define WD_TASK_MONITOR_STACK_CRIT_BYTES 64u  /* 잔여 스택 이 값 이하 → CRITICAL */
#define WD_TASK_MONITOR_HEAP_WARN_PCT    80u  /* 힙 사용률 이 값 이상 → WARNING  */

/***************************************************************************************************************************************************************
 **********  Structures
 **************************************************************************************************************************************************************/

/*--------------- Thread Info ---------------*/
typedef struct {
    char Name[configMAX_TASK_NAME_LEN + 1u];
    TaskHandle_t Handle;
    eTaskState State;
    UBaseType_t Priority;
    uint32_t Stack_Min_Free_B; /* 최소 잔여 스택 [bytes] = High Water Mark × sizeof(StackType_t) */
} wd_taskmon_thread_info_t;

/*--------------- Heap Info ---------------*/
typedef struct {
    uint32_t Total_B;         /* configTOTAL_HEAP_SIZE             */
    uint32_t Free_B;          /* xPortGetFreeHeapSize()            */
    uint32_t Used_B;          /* Total - Free                      */
    uint32_t Min_Ever_Free_B; /* xPortGetMinimumEverFreeHeapSize() */
    uint32_t Max_Ever_Used_B; /* Total - Min_Ever_Free             */
    uint32_t Used_Pct;        /* Used / Total × 100                */
} wd_taskmon_heap_info_t;

/*--------------- Top-level Data ---------------*/
typedef struct {
    uint32_t Thread_Count;
    wd_taskmon_thread_info_t Thread[WD_TASK_MONITOR_MAX_THREADS];
    wd_taskmon_heap_info_t Heap;
    uint32_t Update_Tick_ms; /* osKernelGetTickCount() 시점 */
} wd_taskmon_data_t;

extern wd_taskmon_data_t wd_taskmon_data;

/***************************************************************************************************************************************************************
 **********  API
 **************************************************************************************************************************************************************/
void wd_taskmon_update(void);         /* 데이터 갱신 (스레드 컨텍스트에서만 호출)   */
void wd_taskmon_report(void);         /* Update 후 Threads + Heap 전체 출력         */
void wd_taskmon_report_threads(void); /* 스레드 목록 출력 (Update 별도 호출 필요)    */
void wd_taskmon_report_heap(void);    /* 힙 정보 출력    (Update 별도 호출 필요)    */

#endif /* RTE_RTOS */

#if defined(__cplusplus)
}
#endif

#endif /* WD_SYSTEM_WD_TASKMON_H */
