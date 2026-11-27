/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_ads127l.h
 *
 *  Created on: Jan 15, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_ADS127L_H
#define WD_COMPONENT_WD_ADS127L_H

#include "_wd_adc.h"

/* board HAL mapping : default CubeMX names, override in wd_board.h */
#ifndef RTE_COMPONENT_ADC_ADS127L_V_DRIVER_HANDLE
#define RTE_COMPONENT_ADC_ADS127L_V_DRIVER_HANDLE (&hspi1)
#endif
#ifndef RTE_COMPONENT_ADC_ADS127L_I_DRIVER_HANDLE
#define RTE_COMPONENT_ADC_ADS127L_I_DRIVER_HANDLE (&hspi2)
#endif
#ifndef RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE
#define RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE (&htim5)
#endif
#ifndef RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_REGISTER
#define RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_REGISTER TIM5
#endif

#if 1 // 토글
#if RTE_COMPONENT_ADC_ADS127

/***************************************************************************************************************************************************************
 **********  ADC Driver
 **************************************************************************************************************************************************************/
#define WD_ADC_ADS127L01_MCLK_SOURCE_AD9837 1
#define WD_ADC_ADS127L01_MCLK_SOURCE_IO     0
#define WD_ADC_ADS127L01_MCLK_SOURCE        WD_ADC_ADS127L01_MCLK_SOURCE_AD9837

//#define WD_ADC_ADS127L01_DATA_SAMPLE_NUMBER                                     1024  //4096
//#define WD_ADC_ADS127L01_DATA_SAMPLE_CYCLE                                      1
//#define WD_ADC_ADS127L01_DATA_SIZE                                              4
//#define WD_ADC_ADS127L01_DATA_STABLE_NUMBER                                     0

#define WD_ADC_ADS127L01_SAMPLING_NUMBER       1024 //4096
#define WD_ADC_ADS127L01_SAMPLING_CYCLE_NUMBER 2

typedef enum wd_adc_ads127l01_control_e {
/*--------------- Control  : [ 0: 7] ---------------*/
#define WD_ADC_ADS127L01_CONTROL_POS 0
#define WD_ADC_ADS127L01_CONTROL_MSK (0xFFUL << WD_ADC_ADS127L01_CONTROL_POS)
    WD_ADC_ADS127L01_RESET = (1UL << WD_ADC_ADS127L01_CONTROL_POS),
    WD_ADC_ADS127L01_SET_SAMPLING_RATE =
        (2UL
         << WD_ADC_ADS127L01_CONTROL_POS), // arg = number of sampling point, value = frequency [hz]
    WD_ADC_ADS127L01_SET_FILTER =
        (3UL << WD_ADC_ADS127L01_CONTROL_POS), // arg = filter type, value = filter option
    WD_ADC_ADS127L01_SET_PHASE_DELAY =
        (4UL << WD_ADC_ADS127L01_CONTROL_POS), // arg = phase delay type, value = delay clock
    WD_ADC_ADS127L01_CONVERSION_START =
        (5UL << WD_ADC_ADS127L01_CONTROL_POS), // arg = number of read data (NULL = buffer size)
    WD_ADC_ADS127L01_CONVERSION_STOP = (6UL << WD_ADC_ADS127L01_CONTROL_POS),
    WD_ADC_ADS127L01_CONVERSION_RESULT = (7UL << WD_ADC_ADS127L01_CONTROL_POS),
    WD_ADC_ADS127L01_CONVERSION_PRINT =
        (8UL << WD_ADC_ADS127L01_CONTROL_POS), // arg = Option (0:Title 1:Skip)


    /*--------------- Reserved : [ 8:31] ---------------*/
} wd_ads127l_control_t;

typedef enum {
#if 0
  WDIF_FLAG_INITIALIZED             = (1UL <<  0),
  WDIF_FLAG_POWERED                 = (1UL <<  1),
  WDIF_FLAG_CONFIGURED              = (1UL <<  2),
#endif
    // Specific
    WD_ADC_ADS127L01_FLAG_SPECIFIC = (1UL << 3),
} wd_ads127l_flag_t;

typedef enum {
    WD_ADC_ADS127L01_EVENT_V_COMPLETE = (1UL << 0),
    WD_ADC_ADS127L01_EVENT_V_TIMEOUT = (1UL << 1),
    WD_ADC_ADS127L01_EVENT_V_ERROR = (1UL << 2),
    WD_ADC_ADS127L01_EVENT_I_COMPLETE = (1UL << 3),
    WD_ADC_ADS127L01_EVENT_I_TIMEOUT = (1UL << 4),
    WD_ADC_ADS127L01_EVENT_I_ERROR = (1UL << 5),
} wd_ads127l_event_t;

typedef struct wd_adc_ads127l01_capabilities_s {
    uint32_t BaudRate;
    uint32_t MCLK;
} wd_ads127l_capabilities_t;

typedef void (*wd_adc_signal_event_t)(uint32_t event);

typedef struct wd_adc_ads127l01_status_s {
    uint32_t Busy : 1;
    uint32_t Error : 1;
    uint32_t Read : 1;
    uint32_t Conversion_Start : 1;
    uint32_t Conversion_Finish : 1;
    uint32_t Conversion_Result : 1;
    uint32_t Reserved : 26;
} wd_ads127l_status_t;

typedef struct wd_adc_ads127l01_xfer_s {
    uint8_t *V_Buf;
    uint8_t *I_Buf;
    uint32_t Num;
    uint32_t Mode;
    uint32_t Cnt;
    uint32_t Idx;
    uint32_t Method;
    uint32_t Ignore;
} wd_ads127l_xfer_t;

typedef struct wd_adc_ads127l01_info_s {
    void *Owner;
    uint32_t Time_Stamp;
    uint32_t Time_Wait;
    uint32_t Timeout;
    uint32_t Timer;
    wd_adc_signal_event_t CB_Event;
    wd_adc_status_t volatile Status;
    int32_t volatile Flags;
    uint32_t volatile Event;
    wd_ads127l_xfer_t volatile Xfer;
    fpu_t MCLK;
    uint32_t Sampling_Number; // FFT Size
    uint32_t Sampling_Count;  // FFT Size / Cycle
    uint32_t Sampling_Cycle_Number;
    fpu_t Sampling_Frequency;
    fpu_t Sampling_Rate; // Sampling_Frequency / Sampling_Number

    uint8_t V_Buf[((WD_ADC_DATA_SAMPLE_NUMBER * WD_ADC_DATA_SAMPLE_CYCLE) +
                   WD_ADC_DATA_STABLE_NUMBER)][WD_ADC_DATA_SIZE];
    int32_t V_Sample[WD_ADC_DATA_SAMPLE_NUMBER * WD_ADC_DATA_SAMPLE_CYCLE];
    uint8_t I_Buf[((WD_ADC_DATA_SAMPLE_NUMBER * WD_ADC_DATA_SAMPLE_CYCLE) +
                   WD_ADC_DATA_STABLE_NUMBER)][WD_ADC_DATA_SIZE];
    int32_t I_Sample[WD_ADC_DATA_SAMPLE_NUMBER * WD_ADC_DATA_SAMPLE_CYCLE];
} wd_ads127l_info_t;

typedef struct wd_adc_ads127l01_io_s {
    uint32_t DRDY;
    uint32_t SPI_CS;
} wd_ads127l_io_t;

typedef const struct wd_adc_ads127l01_resources_s {
    wd_ads127l_capabilities_t Capabilities;
    wd_ads127l_info_t *Info;
    wd_ads127l_io_t IO;
    SPI_HandleTypeDef *Handle_V;
    SPI_TypeDef *Reg_V;
    SPI_HandleTypeDef *Handle_I;
    SPI_TypeDef *Reg_I;
    int32_t (*Start)(void);
    int32_t (*Lock)(void);
    int32_t (*Unlock)(void);
    void *Private;
} wd_ads127l_resources_t;

extern wd_adc_driver_adc_ac_t wd_drv_adc_ac;
extern wd_ads127l_resources_t *const wd_ad4134_res_adc_ac;

#ifndef NDEBUG
typedef struct wd_debug_adc_ads127l01_s {
    wd_ads127l_resources_t *Resource;
    wd_ads127l_info_t *Info;
    wd_adc_driver_adc_ac_t *Driver;
} const wd_ads127l_debug_adc_ads127l01_t;

extern wd_ads127l_debug_adc_ads127l01_t DBG_ADC_ADS127L01;
#endif


#endif //	#if RTE_COMPONENT_ADC_ADS127

#endif //	#if 1 // 토글

#endif /* WD_COMPONENT_WD_ADS127L_H */
