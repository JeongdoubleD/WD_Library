/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_os.h
 *
 *  Created on: Apr 4, 2025
 *      Author: doubled
 */

#ifndef WD_CORE_WD_OS_H
#define WD_CORE_WD_OS_H

#include "wd_core.h"

#if 1

extern osMutexId_t wd_os_mutex_global;
extern const osMutexAttr_t wd_os_mutex_global_attr;

extern osMutexId_t wd_os_mutex_printd;
extern const osMutexAttr_t wd_os_mutex_printd_attr;

#if (RTE_PLATFORM_PRINT_USE_MUTEX_PRINTD)
extern osMutexId_t Mutex_printx;
extern const osMutexAttr_t Mutex_printx_Attr;
#endif

#if defined(RTE_RTOS_FreeRTOS_CORE)

#define os_thread_t StaticTask_t
#define os_timer_t
#define os_timer_finfo_t
#define os_event_flags_t
#define os_mutex_t StaticSemaphore_t
#define os_semaphore_t
#define os_mp_info_t
#define os_memory_pool_t
#define os_message_t
#define os_message_queue_t
#define os_object_t
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
uint32_t HAL_GetTick(void);
void vApplicationTickHook(void);
#endif

#endif

#endif /* WD_CORE_WD_OS_H */
