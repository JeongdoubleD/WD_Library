/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _Scheduler.h
 *
 *  Created on: Jan 7, 2025
 *      Author: doubled
 */

#ifndef WD_SYSTEM_WD_SCHED_H
#define WD_SYSTEM_WD_SCHED_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "wd.h"

#if 1

void wd_sched_tick(void);
void wd_sched_tick_driver(wd_reg_timer_t *timer_src, wd_reg_timer_t *timer);
void wd_sched_finalize(wd_reg_timer_t *timer);

#endif // #if 1 	// 토글

#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/

#endif /* WD_SYSTEM_WD_SCHED_H */
