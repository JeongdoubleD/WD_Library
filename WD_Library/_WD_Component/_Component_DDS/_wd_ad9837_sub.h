/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_ad9837_sub.h
 *
 *  Created on: Jan 17, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_AD9837_SUB_H
#define WD_COMPONENT_WD_AD9837_SUB_H

#include "_wd_dds.h"
#include "_wd_softspi.h"

#if 1 // 토글
#if RTE_COMPONENT_ADC_AD4134
#else
#if RTE_COMPONENT_DDS_AD9837_SUB

typedef const struct wd_dds_ad9837_sub_resources_s {
    wd_ad9837_all_capabilities_t Capabilities;
    wd_ad9837_all_info_t *Info;
    wd_ad9837_all_io_t IO;
    wd_softspi_resources_t *Drv;
    void *Handle;
    void *Reg;
    int32_t (*Start)(void);
    int32_t (*Lock)(void);
    int32_t (*Unlock)(void);
} wd_ad9837_sub_resources_t;

extern wd_dds_driver_dds_t DRV_DDS_AD9837_SUB;
extern wd_ad9837_sub_resources_t *const RES_DDS_AD9837_SUB;

#ifndef NDEBUG
typedef struct wd_debug_dds_ad9837_sub_s {
    wd_ad9837_sub_resources_t *Resource;
    wd_ad9837_all_info_t *Info;
    wd_dds_driver_dds_t *Driver;
    wd_softspi_resources_t *Driver_Resource;
} const wd_ad9837_sub_debug_dds_ad9837_sub_t;

extern wd_ad9837_sub_debug_dds_ad9837_sub_t DBG_DDS_AD9837_SUB;
#endif


#endif //	#if RTE_COMPONENT_DDS_AD9837_SUB
#endif //	#if RTE_COMPONENT_ADC_AD4134
#endif //	#if 1 // 토글

#endif /* WD_COMPONENT_WD_AD9837_SUB_H */
