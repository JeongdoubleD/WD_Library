/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_vio.h
 *
 *  Created on: May 13, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_VIO_H
#define WD_COMPONENT_WD_VIO_H


#include "wd_core.h"

#if 1 // 토글
typedef enum {
/*--------------- Command  : [ 0: 7] ---------------*/
#define WD_VIO_COMMAND_POS 0
#define WD_VIO_COMMAND_MSK (0xFFUL << WD_VIO_COMMAND_POS)
    WD_VIO_LED_SET_OFF_TIMER = ((1UL) << WD_VIO_COMMAND_POS), // ms (div 50ms)
    WD_VIO_LED_CASE_ERROR = ((2UL) << WD_VIO_COMMAND_POS),    // ms (div 50ms)
/*--------------- Control  : [ 8:31] ---------------*/
#define WD_VIO_CONTROL_POS 8
#define WD_VIO_CONTROL_MSK (0xFFFFFFUL << WD_VIO_CONTROL_POS)
    WD_VIO_CONTROL_1 = ((1UL << 0) << WD_VIO_CONTROL_POS),
} wd_vio_control_t;

typedef struct {
    uint32_t Busy : 1;
    uint32_t Error : 1;
    uint32_t Reserved : 30;
} wd_vio_status_t;

typedef enum {
    /*
  WD_STATE_INITIALIZED = (1UL <<  0),
  WD_STATE_POWERED     = (1UL <<  1),
  WD_STATE_CONFIGURED  = (1UL <<  2),
*/
    WD_VIO_STATE_1 = (1UL << 3),
} wd_vio_state_t;

typedef enum {
    WD_VIO_EVENT_FALLING = (1UL << 0),
    WD_VIO_EVENT_RISING = (1UL << 1),
} wd_vio_event_t;

typedef wd_event_callback_t wd_vio_signal_event_t;

typedef struct {
    uint32_t TimeOut; // us
} wd_vio_capabilities_t;

typedef struct {
    uint32_t SHDN;
    uint32_t Mux_A;
    uint32_t Mux_B;
    uint32_t LED_Red;
    uint32_t LED_Green;
    uint32_t LED_Blue;
    uint32_t LED_RUN;
    uint32_t LED_ADC;
    uint32_t LED_DDS;
    uint32_t LED_COM;
    uint32_t FAN;
    uint32_t Mux_sel_1;
    uint32_t Mux_sel_2;
    uint32_t Mux_sel_3;
    uint32_t Mux_sel_4;
    uint32_t VAC_AMP;
} const wd_vio_io_t;

typedef enum {
    WD_VIO_SHDN = 1,
    WD_VIO_MUX = 2,
    WD_VIO_LED_RGB = 3,
    WD_VIO_LED_RUN = 4,
    WD_VIO_LED_ADC = 5,
    WD_VIO_LED_DDS = 6,
    WD_VIO_LED_COM = 7,
    WD_VIO_FAN = 8,
    WD_VIO_MUX_SEL = 9,

    WD_VIO_LED_PORT_1_TX = 5,
    WD_VIO_LED_PORT_1_RX = 6,
    WD_VIO_LED_PORT_2_TX = 7,
    WD_VIO_LED_PORT_2_RX = 4,


    WD_VIO_SHDN_OFF = 0,
    WD_VIO_SHDN_ON = 1,

    WD_VIO_LED_OFF = WD_GPIO_OFF,
    WD_VIO_LED_ON = WD_GPIO_ON,

    WD_VIO_FAN_OFF = WD_GPIO_OFF,
    WD_VIO_FAN_ON = WD_GPIO_ON,

    WD_VIO_MUX_1 = 1, //
    WD_VIO_MUX_2 = 2, //
    WD_VIO_MUX_3 = 3, //
    WD_VIO_MUX_4 = 4, //

    WD_VIO_MUX_SEL_1 = 1, //
    WD_VIO_MUX_SEL_2 = 2, //
    WD_VIO_MUX_SEL_3 = 3, //
    WD_VIO_MUX_SEL_4 = 4, //

    WD_VIO_VAC_AMP = 10,
    WD_VIO_VAC_AMP_OFF = WD_GPIO_OFF,
    WD_VIO_VAC_AMP_ON = WD_GPIO_ON,
} wd_vio_table_t;

typedef struct {
    void *Owner;
    wd_vio_signal_event_t CB_Event;
    wd_vio_status_t volatile Status;
    int32_t volatile State;
    uint32_t volatile Event;
    uint32_t Time_Stamp;
    uint32_t Wait_Time;
    uint32_t Timer;
    uint32_t Tx_Timeout;
    uint32_t SHDN;
    uint32_t Mux;
    uint32_t LED_RGB;
    uint32_t LED_RUN;
    uint32_t LED_ADC;
    uint32_t LED_DDS;
    uint32_t LED_COM;
    uint32_t FAN;
    uint32_t LED_RGB_Off_Timer; // us
    uint32_t LED_RUN_Off_Timer; // us
    uint32_t LED_ADC_Off_Timer; // us
    uint32_t LED_DDS_Off_Timer; // us
    uint32_t LED_COM_Off_Timer; // us
    uint32_t Mux_sel;
    uint32_t VAC_AMP;
} wd_vio_info_t;

typedef struct {
    const wd_vio_capabilities_t *Capabilities;
    wd_vio_io_t IO;
    wd_vio_info_t *Info;
} const wd_vio_resources_t;

typedef struct {
#if (RTE_CONFIG_DRIVER_EXTENSION)
    const char *(*GetVersion)(void);
    void *(*GetCapabilities)(void);
#endif
    int32_t (*Initialize)(wd_vio_signal_event_t cb_event);
#if (RTE_CONFIG_DRIVER_EXTENSION)
    int32_t (*Uninitialize)(void);
#endif
    int32_t (*PowerControl)(wd_power_state_t state);
    void (*Set)(uint32_t io, uint32_t arg);
    uint32_t (*Get)(uint32_t io);
    int32_t (*Control)(uint32_t control, uint32_t io, uint32_t arg);
    wd_vio_status_t (*Get_Status)(void);
    wd_vio_resources_t *(*Get_Priv)(void);
} const wd_vio_driver_vio_t;

extern wd_vio_driver_vio_t wd_drv_vio;

#endif

#endif /* WD_COMPONENT_WD_VIO_H */
