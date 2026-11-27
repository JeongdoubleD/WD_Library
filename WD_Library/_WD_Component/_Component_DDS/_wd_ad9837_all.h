/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_ad9837_all.h
 *
 *  Created on: Feb 12, 2025
 *      Author: doubleD
 */

#ifndef WD_COMPONENT_WD_AD9837_ALL_H
#define WD_COMPONENT_WD_AD9837_ALL_H

#include "_wd_dds.h"
#include "_wd_softspi.h"

#if 1 // 토글
#if (RTE_COMPONENT_ADC_AD4134)
typedef enum wd_dds_ad9837_control_e {
/*--------------- Control  : [ 0:15] ---------------*/
#define WD_DDS_AD9837_CONTROL_POS 0
#define WD_DDS_AD9837_CONTROL_MSK (0xFFFFUL << WD_DDS_AD9837_CONTROL_POS)
    WD_DDS_AD9837_CALIBRATION = (1UL << WD_DDS_AD9837_CONTROL_POS),
/*--------------- Option   : [16:31] ---------------*/
#define WD_DDS_AD9837_OPTION_POS 16
#define WD_DDS_AD9837_OPTION_MSK (0xFFFFUL << WD_DDS_AD9837_OPTION_POS)
#define WD_DDS_AD9837_OPTION(n)  ((n) << WD_DDS_AD9837_OPTION_POS)
} wd_ad9837_all_control_t;

typedef enum {
// Fix
#if 0
  WDIF_FLAG_INITIALIZED             = (1UL <<  0),
  WDIF_FLAG_POWERED                 = (1UL <<  1),
  WDIF_FLAG_CONFIGURED              = (1UL <<  2),
#endif
    // Specific
    WD_DDS_AD9837_FLAG_SPECIFIC = (1UL << 3),
} wd_ad9837_all_flag_t;

typedef enum {
    // Specific
    WD_DDS_AD9837_EVENT_CHANGE = (1UL << 0),
} wd_ad9837_all_event_t;

typedef struct wd_dds_ad9837_capabilities_s {
    uint32_t BaudRate;
    fpu_t MCLK;
    uint32_t Wave_Point_Num;
    uint32_t Flag_Wave_Square;
} wd_ad9837_all_capabilities_t;

typedef struct wd_dds_ad9837_xfer_s {
    uint8_t *Tx_Buf;
    uint8_t *Rx_Buf;
    uint32_t Tx_Buf_Size;
    uint32_t Rx_Buf_Size;
    uint32_t Tx_Num;
    uint32_t Rx_Num;
    uint32_t Tx_Cnt;
    uint32_t Rx_Cnt;
    uint32_t Tx_Idx;
    uint32_t Rx_Idx;
} wd_ad9837_all_xfer_t;

typedef struct wd_dds_ad9837_info_s {
    void *Owner;
    uint32_t Time_Stamp;
    uint32_t Time_Wait;
    uint32_t Timeout;
    uint32_t Timer;
    wd_dds_signal_event_t CB_Event;
    wd_dds_status_t volatile Status;
    int32_t volatile Flags;
    uint32_t volatile Event;
    wd_ad9837_all_xfer_t volatile Xfer;
    fpu_t MCLK;
    fpu_t Frequency;
    uint16_t Register;
    uint16_t Register_Sub;
} wd_ad9837_all_info_t;

typedef struct wd_dds_ad9837_io_s {
    uint32_t CS;
    uint32_t CLK;
    uint32_t MOSI;
} wd_ad9837_all_io_t;

typedef const struct wd_dds_ad9837_resources_s {
    wd_ad9837_all_capabilities_t Capabilities;
    wd_ad9837_all_info_t *Info;
    wd_ad9837_all_io_t IO;
    wd_softspi_resources_t *Drv;
    void *Handle;
    void *Reg;
    int32_t (*Start)(void);
    int32_t (*Lock)(void);
    int32_t (*Unlock)(void);
} wd_ad9837_resources_t;

extern wd_dds_driver_dds_t wd_drv_dds_ad9837;
extern wd_ad9837_resources_t *const wd_ad9837_all_res_dds_ad9837;

extern fpu_t wd_ad9837_all_set_mclk(fpu_t frequency);

#ifndef NDEBUG
typedef struct wd_debug_dds_ad9837_s {
    wd_ad9837_resources_t *Resource;
    wd_ad9837_all_info_t *Info;
    wd_dds_driver_dds_t *Driver;
    wd_softspi_resources_t *Driver_Resource;
} const wd_ad9837_debug_dds_ad9837_t;

extern wd_ad9837_debug_dds_ad9837_t wd_ad9837_all_dbg_dds_ad9837;
#endif

/***************************************************************************************************************************************************************
 **********  AD9837 (https://www.mikroe.com/waveform-3-click)
 **************************************************************************************************************************************************************/
typedef enum {
    AD9837_TRIANGLE = 1,
    AD9837_SINE,
    AD9837_SQUARE,
    AD9837_SQUARE_2
} wd_ad9837_all_mode_t;
typedef enum { AD9837_FREQ0 = 1, AD9837_FREQ1 } wd_ad9837_all_freqreg_t;
typedef enum { AD9837_PHASE0 = 1, AD9837_PHASE1 } wd_ad9837_all_phasereg_t;
typedef enum { AD9837_FULL = 1, AD9837_COARSE, AD9837_FINE } wd_ad9837_all_freqadjustmode_t;
typedef enum {
    AD9837_DAC_OFF = 1,
    AD9837_DAC_ON,
    AD9837_DAC_TOGGLE
} wd_ad9837_all_ad9837_control_t;

#else

#endif

#endif //#if 1 // 토글

#endif /* WD_COMPONENT_WD_AD9837_ALL_H */
