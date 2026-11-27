/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_ntc.h
 *
 *  Created on: Oct 27, 2026
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_NTC_H
#define WD_COMPONENT_WD_NTC_H

#include "wd_core.h"
#include "_wd_ds18b20.h" // wd_thermometer_driver_t 공용 인터페이스 정의

/* board HAL mapping : default CubeMX names, override in wd_board.h */
#ifndef RTE_COMPONENT_THERMOMETER_NTC_ADC_HANDLE
#define RTE_COMPONENT_THERMOMETER_NTC_ADC_HANDLE (&hadc1)
#endif

#if (RTE_COMPONENT_THERMOMETER_NTC)

extern wd_thermometer_driver_t wd_drv_thermometer;

#endif // RTE_COMPONENT_THERMOMETER_NTC

#endif /* WD_COMPONENT_WD_NTC_H */
