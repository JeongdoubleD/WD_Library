/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _Thread.h   (TEMPLATE - project owned)
 * @brief   RTOS threads of the project : Main / Console / App / Ethernet / CAN
 ******************************************************************************/

#ifndef WD_TEMPLATE_THREAD_H
#define WD_TEMPLATE_THREAD_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "wd.h"

#define WD_CONF_THREAD_STACK_SIZE (1024 * 4)
#define WD_THREAD_STACK_SIZE      (1024 * 4)

#if 1 // 토글

extern uint32_t Debug_Time_Thread_App_Call;
extern uint32_t Debug_Time_Thread_App_Call_Stamp;

extern osThreadId_t Thread_API_Main_ID;
extern const osThreadAttr_t Thread_API_Main_Attr;
void Thread_API_Main(void *arg);

extern osThreadId_t Thread_Console_ID;
extern const osThreadAttr_t Thread_Console_Attr;
void Thread_Console(void *arg);

extern osThreadId_t Thread_CAN_ID;
extern const osThreadAttr_t Thread_CAN_Attr;
void Thread_CAN(void *arg);

extern osThreadId_t Thread_Ethernet_ID;
extern const osThreadAttr_t Thread_Ethernet_Attr;
void Thread_Ethernet(void *arg);

extern osThreadId_t Thread_App_ID;
extern const osThreadAttr_t Thread_App_Attr;
void Thread_App(void *arg);

/* Application hooks (weak in _Thread.c) */
void App_Initialize(void);
void App_Process(wd_reg_timer_t *timer);

#endif // #if 0

#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/

#endif /* WD_TEMPLATE_THREAD_H */
