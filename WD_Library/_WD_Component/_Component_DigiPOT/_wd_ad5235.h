/* SPDX-License-Identifier: MIT AND BSD-3-Clause
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright 2013(c) Analog Devices, Inc. (no-OS ad525x driver, BSD-3-Clause)
 */
/*
 * _wd_ad5235.h
 *
 *  Created on: Jan 17, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_AD5235_H
#define WD_COMPONENT_WD_AD5235_H

#include "wd_core.h"
#include "_wd_softspi.h"


#if 1 // 토글
#if RTE_COMPONENT_DIGIPOT_AD5235

#define WD_MAX_VR_VALUE 250000 // 전체 저항값 (예: 250000)

typedef enum wd_digipot_ad5235_control_e {
/*--------------- Control  : [ 0:15] ---------------*/
#define WD_DIGIPOT_AD5235_CONTROL_POS 0
#define WD_DIGIPOT_AD5235_CONTROL_MSK (0xFFFFUL << WD_DIGIPOT_AD5235_CONTROL_POS)
    WD_DIGIPOT_AD5235_CONFIG_CH_1 = (1UL << WD_DIGIPOT_AD5235_CONTROL_POS),
    WD_DIGIPOT_AD5235_CONFIG_CH_2 = (2UL << WD_DIGIPOT_AD5235_CONTROL_POS),
    WD_DIGIPOT_AD5235_CALIBRATION = (3UL << WD_DIGIPOT_AD5235_CONTROL_POS),

/*--------------- Option   : [16:31] ---------------*/
#define WD_DIGIPOT_AD5235_OPTION_POS 16
#define WD_DIGIPOT_AD5235_OPTION_MSK (0xFFFFUL << WD_DIGIPOT_AD5235_OPTION_POS)
#define WD_DIGIPOT_AD5235_OPTION(n)  ((n) << WD_DIGIPOT_AD5235_OPTION_POS)
} wd_ad5235_control_t;

typedef enum {
// Fix
#if 0
  WDIF_FLAG_INITIALIZED             = (1UL <<  0),
  WDIF_FLAG_POWERED                 = (1UL <<  1),
  WDIF_FLAG_CONFIGURED              = (1UL <<  2),
#endif
    // Specific
    WD_DIGIPOT_AD5235_FLAG_SPECIFIC = (1UL << 3),
} wd_ad5235_flag_t;

typedef enum {
    // Specific
    WD_DIGIPOT_AD5235_EVENT_CHANGE = (1UL << 0),
} wd_ad5235_event_t;

typedef struct wd_digipot_ad5235_capabilities_s {
    uint32_t BaudRate;
} wd_ad5235_capabilities_t;

typedef void (*wd_ad5235_signal_event_t)(uint32_t event);

typedef struct wd_digipot_ad5235_status_s {
    // Fix
    uint32_t Busy : 1;
    uint32_t Error : 1;
    // Specific
    uint32_t Reserved : 30;
} wd_ad5235_status_t;

typedef struct wd_digipot_ad5235_xfer_s {
    // Specific
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
} wd_ad5235_xfer_t;

typedef struct wd_digipot_ad5235_info_s {
    // Fix
    void *Owner;
    uint32_t Time_Stamp;
    uint32_t Time_Wait;
    uint32_t Timeout;
    uint32_t Timer;
    wd_ad5235_signal_event_t CB_Event;
    wd_ad5235_status_t volatile Status;
    int32_t volatile Flags;
    uint32_t volatile Event;
    wd_ad5235_xfer_t volatile Xfer;
    uint16_t RDAC1;
    uint16_t RDAC2;
} wd_ad5235_info_t;

typedef struct wd_digipot_ad5235_io_s {
    uint32_t CS;
    uint32_t CLK;
    uint32_t MOSI;
    uint32_t MISO;
} wd_ad5235_io_t;

typedef const struct wd_digipot_ad5235_resources_s {
    wd_ad5235_capabilities_t Capabilities;
    wd_ad5235_info_t *Info;
    wd_ad5235_io_t IO;
    wd_softspi_resources_t *Drv;
    void *Handle;
    void *Reg;
    int32_t (*Start)(void);
    int32_t (*Lock)(void);
    int32_t (*Unlock)(void);
} wd_ad5235_resources_t;

typedef struct wd_driver_digipot_ad5235_s {
    int32_t (*Initialize)(wd_ad5235_signal_event_t cb_event);
    int32_t (*Uninitialize)(void);
    int32_t (*PowerControl)(wd_power_state_t state);
    int32_t (*Control)(uint32_t control, uint32_t arg);
    wd_ad5235_status_t (*Get_Status)(void);
    void (*Validation)(void);
} const wd_ad5235_driver_digipot_ad5235_t;

extern wd_ad5235_driver_digipot_ad5235_t wd_drv_digi_pot_ad5235;
extern wd_ad5235_resources_t *const wd_ad5235_res_digi_pot_ad5235;

#ifndef NDEBUG
typedef struct wd_debug_digipot_ad5235_s {
    wd_ad5235_resources_t *Resource;
    wd_ad5235_info_t *Info;
    wd_ad5235_driver_digipot_ad5235_t *Driver;
} const wd_ad5235_debug_digipot_ad5235_t;

extern wd_ad5235_debug_digipot_ad5235_t wd_ad5235_dbg_digi_pot_ad5235;
#endif

void wd_ad5235_init(void);
uint16_t wd_ad5235_read(uint32_t ch);
void wd_ad5235_write(uint32_t ch, uint32_t value);
extern int wd_ad5235_resistance_int(uint32_t rwa);


#endif //#if RTE_COMPONENT_DIGIPOT_AD5235
#endif //#if 1 // 토글

#endif /* WD_COMPONENT_WD_AD5235_H */
