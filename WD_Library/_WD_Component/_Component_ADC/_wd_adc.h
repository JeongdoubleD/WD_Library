/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_adc.h
 *
 *  Created on: Jan 15, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_ADC_H
#define WD_COMPONENT_WD_ADC_H

#include "wd_core.h"

#if 1 // 토글

#define WD_ADC_VOLTAGE_RMS_FACTOR 1.0
#define WD_ADC_VOLTAGE_LSB (fpu_t)(0.00000048828125 * WD_ADC_VOLTAGE_RMS_FACTOR) //  4.096 / 2^23

/* sample buffer geometry shared by the ADC drivers (AD4134 / ADS127L) and the CLI test commands */
#ifndef WD_ADC_DATA_SAMPLE_NUMBER
#define WD_ADC_DATA_SAMPLE_NUMBER 256
#endif
#ifndef WD_ADC_DATA_SAMPLE_CYCLE
#define WD_ADC_DATA_SAMPLE_CYCLE 1
#endif
#ifndef WD_ADC_DATA_SIZE
#define WD_ADC_DATA_SIZE 3
#endif
#ifndef WD_ADC_DATA_STABLE_NUMBER
#define WD_ADC_DATA_STABLE_NUMBER 0
#endif
#ifndef WD_ADC_SAMPLING_NUMBER
#define WD_ADC_SAMPLING_NUMBER 1024
#endif
#ifndef WD_ADC_SAMPLING_CYCLE_NUMBER
#define WD_ADC_SAMPLING_CYCLE_NUMBER 2
#endif

typedef enum {
    WD_ADC_CH_1 = 0,
    WD_ADC_CH_2 = 1,
    WD_ADC_CH_3 = 2,
    WD_ADC_CH_4 = 3,

    WD_ADC_FILTER_FIR = 0,
    WD_ADC_FILTER_SINC3 = 1,
    WD_ADC_FILTER_SINC6 = 2,

    WD_ADC_CH_AC_VOLTAGE = 0,
    WD_ADC_CH_AC_CURRENT = 1,
    WD_ADC_CH_DC_VOLTAGE = 2,
    WD_ADC_CH_4_VOLTAGE = 3,

    WD_ADC_CONVERSION_MODE_SINGLE = 0,
    WD_ADC_CONVERSION_MODE_CONTINUOUS = 1,
} wd_adc_table_t;

typedef enum wd_adc_control_e {
/*--------------- Control  : [ 0: 12] ---------------*/
#define WD_ADC_CONTROL_POS 0
#define WD_ADC_CONTROL_MSK (0xFFUL << WD_ADC_CONTROL_POS)
    WD_ADC_RESET = (1UL << WD_ADC_CONTROL_POS),
    WD_ADC_SET_SAMPLING_RATE =
        (2UL << WD_ADC_CONTROL_POS), // arg = number of sampling point, value = frequency [hz]
    WD_ADC_SET_FILTER = (3UL << WD_ADC_CONTROL_POS), // arg = filter type, value = filter option
    WD_ADC_SET_PHASE_DELAY =
        (4UL << WD_ADC_CONTROL_POS), // arg = phase delay type, value = delay clock
    WD_ADC_CONVERSION_START =
        (5UL << WD_ADC_CONTROL_POS), // arg = number of read data (NULL = buffer size)
    WD_ADC_CONVERSION_STOP = (6UL << WD_ADC_CONTROL_POS),
    WD_ADC_CONVERSION_RESULT = (7UL << WD_ADC_CONTROL_POS),
    WD_ADC_CONVERSION_PRINT = (8UL << WD_ADC_CONTROL_POS), // arg = Option (0:Title 1:Skip)
    WD_ADC_SET_ODR = (
        9UL
        << WD_ADC_CONTROL_POS), // arg = output data rate [hz] pointer, value = input data rate [hz]
    WD_ADC_GET_ODR = (10UL << WD_ADC_CONTROL_POS),         // arg = output data rate [hz] pointer
    WD_ADC_CONVERSION_CALC = (11UL << WD_ADC_CONTROL_POS), // arg = Analysis
    WD_ADC_SET_SYNC_SETTLE = (
        12UL
        << WD_ADC_CONTROL_POS), // arg = settling time [ms], V/I 동기화 Start_Conversion 시 ODR 정지/재개 후 settling 대기
    WD_ADC_SET_REINIT = (13UL << WD_ADC_CONTROL_POS), // arg = Reinitialization,
    /*--------------- Reserved : [ 14:31] ---------------*/
} wd_adc_control_t;

typedef void (*wd_adc_signal_event_t)(uint32_t event);

typedef struct wd_adc_status_s {
    uint32_t Busy : 1;
    uint32_t Error : 1;
    uint32_t Read : 1;
    uint32_t Conversion_Start : 1;
    uint32_t Conversion_Finish : 1;
    uint32_t Conversion_Result : 1;
    uint32_t Conversion_Calc : 1;
    uint32_t Reserved : 25;
} wd_adc_status_t;

typedef struct wd_driver_adc_ac_s {
    int32_t (*Initialize)(wd_adc_signal_event_t cb_event);
    int32_t (*Uninitialize)(void);
    int32_t (*PowerControl)(wd_power_state_t state);
    //  int32_t        (*Start_Conversion) (void *v_buf, void *i_buf, uint32_t num, uint32_t arg);  // 0 : Continuous Conversion Mode    1 : Single Conversion Mode
    int32_t (*Start_Conversion)(
        void *v_buf, void *i_buf, void *ch3_buf, void *ch4_buf, uint32_t num,
        uint32_t arg); // 0 : Continuous Conversion Mode    1 : Single Conversion Mode
    int32_t (*Poll_Conversion)(uint32_t timeout, uint32_t arg);
    int32_t (*Stop_Conversion)(uint32_t arg);
    int32_t (*Control)(uint32_t control, uint32_t arg, fpu_t value);
    wd_adc_status_t (*Get_Status)(void);
    void (*Validation)(void);
} const wd_adc_driver_adc_ac_t;


#endif //	#if 1 // 토글

#endif /* WD_COMPONENT_WD_ADC_H */
