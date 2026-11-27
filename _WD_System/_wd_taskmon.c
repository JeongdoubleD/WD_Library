/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _Task_monitor.c
 *
 *  Created on: 2026-03-30
 *      Author: doubled
 *
 *  RTOS 스레드 스택 / 힙 모니터링 모듈
 *
 *  ■ 스레드 모니터링
 *    - uxTaskGetSystemState() 로 전체 태스크 일괄 조회
 *    - Stack_Min_Free_B = usStackHighWaterMark × sizeof(StackType_t)
 *      (스케줄러 시작 이후 최소 잔여 스택 바이트 — 작을수록 위험)
 *
 *  ■ 힙 모니터링  (heap_4 기준)
 *    - xPortGetFreeHeapSize()           : 현재 여유 힙
 *    - xPortGetMinimumEverFreeHeapSize() : 역대 최소 여유 힙
 *    - configTOTAL_HEAP_SIZE            : 전체 힙 크기
 *
 *  ■ 사용 예시 (Thread_API_Main 등 1000 ms 틱에서 호출)
 *    if (wd_reg_timer->Tick_1000ms) {
 *        static uint8_t cnt = 0;
 *        if (++cnt >= 10) {          // 10 초마다 리포트
 *            cnt = 0;
 *            wd_taskmon_report();
 *        }
 *    }
 */

#include "wd.h"

#if (RTE_RTOS)

/***************************************************************************************************************************************************************
 **********  Module Data
 **************************************************************************************************************************************************************/
wd_taskmon_data_t wd_taskmon_data;

/***************************************************************************************************************************************************************
 **********  Private Helpers
 **************************************************************************************************************************************************************/
static const char *thread_state_str(eTaskState s) {
    switch (s) {
    case eRunning:
        return "RUNNING ";
    case eReady:
        return "READY   ";
    case eBlocked:
        return "BLOCKED ";
    case eSuspended:
        return "SUSPEND ";
    case eDeleted:
        return "DELETED ";
    default:
        return "INVALID ";
    }
}

static const char *thread_prio_str(UBaseType_t p) {
    /* CMSIS-RTOS2 우선순위 기준 (FreeRTOS 내부 값은 오름차순) */
    if (p == tskIDLE_PRIORITY) {
        return "IDLE  ";
    } else if (p == 1u) {
        return "LOW   ";
    } else if (p <= 16u) {
        return "NORMAL";
    } else if (p <= 32u) {
        return "HIGH  ";
    } else {
        return "RTIME ";
    }
}

/***************************************************************************************************************************************************************
 **********  wd_taskmon_update
 *  스레드 컨텍스트에서만 호출할 것 (ISR 불가)
 **************************************************************************************************************************************************************/
void wd_taskmon_update(void) {
    static TaskStatus_t task_status_buf[WD_TASK_MONITOR_MAX_THREADS];
    uint32_t count;
    uint32_t i;

    /*--------------- Threads ---------------*/
    /* pulTotalRunTime = NULL : configGENERATE_RUN_TIME_STATS 비활성 환경 호환 */
    count = uxTaskGetSystemState(task_status_buf, WD_TASK_MONITOR_MAX_THREADS, NULL);

    if (count > WD_TASK_MONITOR_MAX_THREADS) {
        count = WD_TASK_MONITOR_MAX_THREADS;
    }

    wd_taskmon_data.Thread_Count = count;

    for (i = 0u; i < count; i++) {
        wd_taskmon_thread_info_t *t = &wd_taskmon_data.Thread[i];
        TaskStatus_t *s = &task_status_buf[i];

        t->Handle = s->xHandle;
        t->State = s->eCurrentState;
        t->Priority = s->uxCurrentPriority;

        /* HWM : 단위 StackType_t(word) → bytes 변환 */
        t->Stack_Min_Free_B = (uint32_t)s->usStackHighWaterMark * (uint32_t)sizeof(StackType_t);

        if (s->pcTaskName != NULL) {
            strncpy(t->Name, s->pcTaskName, configMAX_TASK_NAME_LEN);
        } else {
            strncpy(t->Name, "???", configMAX_TASK_NAME_LEN);
        }
        t->Name[configMAX_TASK_NAME_LEN] = '\0';
    }

    /*--------------- Heap ---------------*/
    {
        wd_taskmon_heap_info_t *h = &wd_taskmon_data.Heap;

        h->Total_B = (uint32_t)configTOTAL_HEAP_SIZE;
        h->Free_B = (uint32_t)xPortGetFreeHeapSize();
        h->Min_Ever_Free_B = (uint32_t)xPortGetMinimumEverFreeHeapSize();

        h->Used_B = (h->Total_B > h->Free_B) ? (h->Total_B - h->Free_B) : 0u;
        h->Max_Ever_Used_B =
            (h->Total_B > h->Min_Ever_Free_B) ? (h->Total_B - h->Min_Ever_Free_B) : 0u;
        h->Used_Pct = (h->Total_B > 0u) ? ((h->Used_B * 100u) / h->Total_B) : 0u;
    }

    wd_taskmon_data.Update_Tick_ms = (uint32_t)osKernelGetTickCount();
}

/***************************************************************************************************************************************************************
 **********  wd_taskmon_report_threads
 **************************************************************************************************************************************************************/
void wd_taskmon_report_threads(void) {
    uint32_t i;

    WD_LOG_NOTICE("[Task Monitor] Threads  Count: %u   Tick: %u ms", wd_taskmon_data.Thread_Count,
                  wd_taskmon_data.Update_Tick_ms);

    WD_LOG_INFO("%-16s  %-8s  %-6s  %s", "Name", "State", "Prio", "StackMinFree");
    WD_LOG_INFO("-----------------------------------------------------");

    for (i = 0u; i < wd_taskmon_data.Thread_Count; i++) {
        wd_taskmon_thread_info_t *t = &wd_taskmon_data.Thread[i];

        if (t->Stack_Min_Free_B <= WD_TASK_MONITOR_STACK_CRIT_BYTES) {
            WD_LOG_ERROR("%-16s  %-8s  %-6s  %5u B  <<< STACK CRITICAL", t->Name,
                         thread_state_str(t->State), thread_prio_str(t->Priority),
                         t->Stack_Min_Free_B);
        } else if (t->Stack_Min_Free_B <= WD_TASK_MONITOR_STACK_WARN_BYTES) {
            WD_LOG_WARN("%-16s  %-8s  %-6s  %5u B  <<< STACK WARNING", t->Name,
                        thread_state_str(t->State), thread_prio_str(t->Priority),
                        t->Stack_Min_Free_B);
        } else {
            WD_LOG_INFO("%-16s  %-8s  %-6s  %5u B", t->Name, thread_state_str(t->State),
                        thread_prio_str(t->Priority), t->Stack_Min_Free_B);
        }
    }
}

/***************************************************************************************************************************************************************
 **********  wd_taskmon_report_heap
 **************************************************************************************************************************************************************/
void wd_taskmon_report_heap(void) {
    wd_taskmon_heap_info_t *h = &wd_taskmon_data.Heap;

    WD_LOG_NOTICE("[Task Monitor] Heap");
    WD_LOG_INFO("Total         : %6u bytes  (%u KB)", h->Total_B, h->Total_B / 1024u);
    WD_LOG_INFO("Current Free  : %6u bytes  (%u KB)", h->Free_B, h->Free_B / 1024u);
    WD_LOG_INFO("Current Used  : %6u bytes  (%u KB)  [%u%%]", h->Used_B, h->Used_B / 1024u,
                h->Used_Pct);
    WD_LOG_INFO("Min Ever Free : %6u bytes  (%u KB)  (역대 최소 여유)", h->Min_Ever_Free_B,
                h->Min_Ever_Free_B / 1024u);
    WD_LOG_INFO("Max Ever Used : %6u bytes  (%u KB)  (역대 최대 사용)", h->Max_Ever_Used_B,
                h->Max_Ever_Used_B / 1024u);

    if (h->Used_Pct >= WD_TASK_MONITOR_HEAP_WARN_PCT) {
        WD_LOG_WARN("Heap 사용률 %u%% >= 경고 임계값 %u%%  <<< HEAP WARNING", h->Used_Pct,
                    WD_TASK_MONITOR_HEAP_WARN_PCT);
    }
}

/***************************************************************************************************************************************************************
 **********  wd_taskmon_report  (Update + 전체 리포트)
 **************************************************************************************************************************************************************/
void wd_taskmon_report(void) {
    wd_taskmon_update();
    wd_taskmon_report_threads();
    wd_taskmon_report_heap();
}

#endif /* RTE_RTOS */
