/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_os.c
 *
 *  Created on: Apr 4, 2025
 *      Author: doubled
 */

#include "wd_core.h"

#if 1 // 토글

#if 1 /// Mutex **************************************************************{
static osStaticMutexDef_t Mutex_Global_cb_mem;
osMutexId_t wd_os_mutex_global;
const osMutexAttr_t wd_os_mutex_global_attr = {.name = "Mutex Global",
                                               .attr_bits = osMutexRecursive | osMutexPrioInherit,
                                               .cb_mem = &Mutex_Global_cb_mem,
                                               .cb_size = sizeof(Mutex_Global_cb_mem)};

static osStaticMutexDef_t Mutex_printd_cb_mem;
osMutexId_t wd_os_mutex_printd;
const osMutexAttr_t wd_os_mutex_printd_attr = {.name = "Mutex printd",
                                               .attr_bits = osMutexRecursive | osMutexPrioInherit,
                                               .cb_mem = &Mutex_printd_cb_mem,
                                               .cb_size = sizeof(Mutex_printd_cb_mem)};

#if (RTE_PLATFORM_PRINT_USE_MUTEX_PRINTD)
static os_mutex_t Mutex_printx_cb_mem;
osMutexId_t Mutex_printx;
const osMutexAttr_t Mutex_printx_Attr = {.name = "Mutex printx",
                                         .attr_bits = osMutexRecursive | osMutexPrioInherit,
                                         .cb_mem = &Mutex_printx_cb_mem,
                                         .cb_size = sizeof(Mutex_printx_cb_mem)};
#endif
#endif

#if defined(RTE_RTOS_FreeRTOS_CORE)

__WEAK void configureTimerForRunTimeStats(void) {
}

__WEAK unsigned long getRunTimeCounterValue(void) {
    return 0;
}

uint32_t HAL_GetTick(void) {
    static uint32_t ticks = 0U;
    uint32_t i;

    if (osKernelGetState() == osKernelRunning) {
        return ((uint32_t)osKernelGetTickCount());
    }

    for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }

    return ++ticks;
}

void vApplicationTickHook(void) {
    wd_time_clock_ms++;
    wd_time_clock_cycle += (SysTick->LOAD + 1);

    if ((wd_time_clock_ms % 50) ==
        0) // 4,294,967,295 % 50 = 45, 오버플로우 될 때 45로 연산되니 참조
    {
        wd_os_tick_50ms_hook();
    }

    wd_time_read_sub_handler();
}
#elif defined(RTE_RTOS_FreeRTOS_CORE)
#endif

#endif
