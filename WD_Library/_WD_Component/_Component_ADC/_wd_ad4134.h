/* SPDX-License-Identifier: MIT AND BSD-3-Clause
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright 2020(c) Analog Devices, Inc. (no-OS ad713x driver, BSD-3-Clause)
 */
/*
 * _wd_ad4134.h
 *
 *  Created on: Jan 15, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_AD4134_H
#define WD_COMPONENT_WD_AD4134_H

#include "_wd_adc.h"
#include "_wd_softspi.h"

#if 1 // 토글
#if (RTE_COMPONENT_ADC_AD4134)

/***************************************************************************************************************************************************************
 **********  ADC Driver
 **************************************************************************************************************************************************************/
// SPI1~5 배선이 바뀌면 이 정의부만 수정하면 됨 (내부 로직은 변경 불필요)
#ifndef RTE_COMPONENT_ADC_AD4134_V_DRIVER_HANDLE
#define RTE_COMPONENT_ADC_AD4134_V_DRIVER_HANDLE (&hspi1)
#endif
#ifndef RTE_COMPONENT_ADC_AD4134_V_DRIVER_REGISTER
#define RTE_COMPONENT_ADC_AD4134_V_DRIVER_REGISTER SPI1
#endif
#ifndef RTE_COMPONENT_ADC_AD4134_I_DRIVER_HANDLE
#define RTE_COMPONENT_ADC_AD4134_I_DRIVER_HANDLE (&hspi2)
#endif
#ifndef RTE_COMPONENT_ADC_AD4134_I_DRIVER_REGISTER
#define RTE_COMPONENT_ADC_AD4134_I_DRIVER_REGISTER SPI2
#endif
#ifndef RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_HANDLE
#define RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_HANDLE (&hspi3)
#endif
#ifndef RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_REGISTER
#define RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_REGISTER SPI3
#endif
#ifndef RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_HANDLE
#define RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_HANDLE (&hspi5)
#endif
#ifndef RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_REGISTER
#define RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_REGISTER SPI5
#endif


#define WD_ADC_DATA_SAMPLE_UNDER_NUMBER 8192

#define WD_ADC_DATA_SKIP_DATA_CALLBACK 128
#define WD_ADC_DATA_SKIP_CYCLE         1

#define WD_ADC_FILTER_FIR_BANDWITDH_1 (fpu_t)0.10825
#define WD_ADC_FILTER_FIR_BANDWITDH_4 (fpu_t)0.433

#define WD_ADC_CH4_ENABLE             0 // 1: CH4(SPI5) 활성화, 0: 비활성화
#define WD_ADC_DATA_CHANNEL_NUMBER    3 //3
#define WD_ADC_DATA_AVERAGE_NUMBER    1
#define WD_ADC_VOLTAGE_RMS_FACTOR     1.0
#define WD_ADC_DATA_SAMPLE_MIN_NUMBER 16

typedef enum {
#if 0
  WDIF_FLAG_INITIALIZED             = (1UL <<  0),
  WDIF_FLAG_POWERED                 = (1UL <<  1),
  WDIF_FLAG_CONFIGURED              = (1UL <<  2),
#endif
    // Specific
    WD_ADC_AD4134_FLAG_SPECIFIC = (1UL << 3),
} wd_ad4134_flag_t;

typedef enum {
    WD_ADC_AD4134_EVENT_V_COMPLETE = (1UL << 0),
    WD_ADC_AD4134_EVENT_V_TIMEOUT = (1UL << 1),
    WD_ADC_AD4134_EVENT_V_ERROR = (1UL << 2),
    WD_ADC_AD4134_EVENT_I_COMPLETE = (1UL << 3),
    WD_ADC_AD4134_EVENT_I_TIMEOUT = (1UL << 4),
    WD_ADC_AD4134_EVENT_I_ERROR = (1UL << 5),
    WD_ADC_AD4134_EVENT_CH3_COMPLETE = (1UL << 6),
    WD_ADC_AD4134_EVENT_CH3_TIMEOUT = (1UL << 7),
    WD_ADC_AD4134_EVENT_CH3_ERROR = (1UL << 8),
    WD_ADC_AD4134_EVENT_CH4_COMPLETE = (1UL << 9),
    WD_ADC_AD4134_EVENT_CH4_TIMEOUT = (1UL << 10),
    WD_ADC_AD4134_EVENT_CH4_ERROR = (1UL << 11),
} wd_ad4134_event_t;

typedef struct {
    uint32_t Complete;
    uint32_t Position;
    int32_t Peak;
} wd_ad4134_adc_zcd_t;

typedef struct {
    uint32_t ZCD_Start;
    uint32_t ZCD_End;
    fpu_t Frequency;
    int32_t DC_Offset;
    uint32_t RMS;
    int32_t Maximum;
    int32_t Peak_Positive;
    int32_t Peak_Negative;
    int32_t Sample_SUM;
} wd_ad4134_adc_data_analysis_t;

typedef struct {
    uint32_t Time_Stamp;
    int32_t Count;
    fpu_t Frequency;
    int32_t DC_Offset;
    uint32_t RMS;
    int32_t Maximum;
    int32_t Peak_Positive;
    int32_t Peak_Negative;
    int32_t Sample_SUM;
} wd_ad4134_adc_data_average_t;

typedef struct {
    wd_adc_status_t Status;
    uint32_t Option;
    wd_ad4134_adc_data_analysis_t Data;
    wd_ad4134_adc_data_average_t Average;
    wd_ad4134_adc_data_average_t Average_Raw;
} wd_ad4134_adc_data_t;

typedef struct {
    fpu_t Factor_Frequency;
    fpu_t Factor_Phase;
} wd_ad4134_adc_calibration_t;

typedef struct wd_adc_ad4134_capabilities_s {
    uint32_t BaudRate;
} wd_ad4134_capabilities_t;

typedef struct wd_adc_ad4134_xfer_s {
    int32_t *V_Buf;
    int32_t *I_Buf;
    int32_t *CH3_Buf;
    int32_t *CH4_Buf;
    uint32_t Num;
    uint32_t Under_Num;
    uint32_t Mode;
    uint32_t Cnt[WD_ADC_DATA_CHANNEL_NUMBER];
    uint32_t Method;
    uint32_t Ignore;
    uint32_t Offset;
    uint32_t Skip_Data_Callback_Num;
    uint32_t Skip_Data_Callback_Cnt[WD_ADC_DATA_CHANNEL_NUMBER];
    uint32_t Skip_Cycle_Num;
    uint32_t Skip_Cycle_Cnt[WD_ADC_DATA_CHANNEL_NUMBER];
} wd_ad4134_xfer_t;

typedef struct wd_adc_ad4134_info_s {
    void *Owner;
    uint32_t Time_Stamp;
    uint32_t Time_Wait;
    uint32_t Timeout;
    uint32_t Timer;
    wd_adc_signal_event_t CB_Event;
    wd_adc_status_t volatile Status_V;
    wd_adc_status_t volatile Status_I;
    wd_adc_status_t volatile Status_CH3;
    wd_adc_status_t volatile Status_CH4;
    int32_t volatile Flags;
    uint32_t volatile Event;
    uint32_t volatile Event_V;
    uint32_t volatile Event_I;
    uint32_t volatile Event_CH3;
    uint32_t volatile Event_CH4;
    wd_ad4134_xfer_t volatile Xfer;
    uint32_t Skip_Data_Callback;
    uint32_t Skip_Cycle;
    uint32_t Average_Number;
    uint32_t Mode;
    fpu_t Frequency;
    uint32_t Sampling_Number; // FFT Size
    uint32_t Sampling_Count;  // FFT Size / Cycle
    uint32_t Sampling_Cycle_Number;
    uint32_t Sampling_Under;
    fpu_t Sampling_Frequency;
    fpu_t Sampling_Rate;          // Sampling_Frequency / Sampling_Number
    uint32_t Cycle_Number;        // 설정된 Cycle 횟수
    uint32_t Sync_Settle_Time_ms; // V/I 동기화용 settling time (0이면 기존 방식)

    uint8_t V_Buf[WD_ADC_DATA_SAMPLE_UNDER_NUMBER * 2][WD_ADC_DATA_SIZE] __ALIGNED(
        WD_CONF_DMA_ALIGNMENT_SIZE);
    uint8_t I_Buf[WD_ADC_DATA_SAMPLE_UNDER_NUMBER * 2][WD_ADC_DATA_SIZE] __ALIGNED(
        WD_CONF_DMA_ALIGNMENT_SIZE);
    uint8_t CH3_Buf[WD_ADC_DATA_SAMPLE_UNDER_NUMBER * 2][WD_ADC_DATA_SIZE] __ALIGNED(
        WD_CONF_DMA_ALIGNMENT_SIZE);
    uint8_t CH4_Buf[WD_ADC_DATA_SAMPLE_UNDER_NUMBER * 2][WD_ADC_DATA_SIZE] __ALIGNED(
        WD_CONF_DMA_ALIGNMENT_SIZE);

    uint32_t Debug_Time_Component_ADC;
    uint32_t Debug_Time_Component_ADC_Read;
    uint32_t Debug_Time_Component_ADC_V;
    uint32_t Debug_Time_Component_ADC_I;
    uint32_t Debug_Time_Component_ADC_CH3;
    uint32_t Debug_Time_Component_ADC_CH4;

    uint32_t Debug_Time_Component_ADC_Stamp;
    uint32_t Debug_Time_Component_ADC_Read_Stamp;
    uint32_t Debug_Time_Component_ADC_V_Stamp;
    uint32_t Debug_Time_Component_ADC_I_Stamp;
    uint32_t Debug_Time_Component_ADC_CH3_Stamp;
    uint32_t Debug_Time_Component_ADC_CH4_Stamp;

    wd_ad4134_adc_data_t ADC_Data[WD_ADC_DATA_CHANNEL_NUMBER];
    wd_ad4134_adc_calibration_t Calibration;
} wd_ad4134_info_t;

typedef struct wd_adc_ad4134_io_s {
    uint32_t RESET;
} wd_ad4134_io_t;

typedef const struct wd_adc_ad4134_resources_s {
    wd_ad4134_capabilities_t Capabilities;
    wd_ad4134_info_t *Info;
    wd_ad4134_io_t IO;
    wd_softspi_resources_t *Drv;
    SPI_HandleTypeDef *Handle_V;
    SPI_TypeDef *Reg_V;
    SPI_HandleTypeDef *Handle_I;
    SPI_TypeDef *Reg_I;
    SPI_HandleTypeDef *Handle_CH3;
    SPI_TypeDef *Reg_CH3;
    SPI_HandleTypeDef *Handle_CH4;
    SPI_TypeDef *Reg_CH4;
    int32_t (*Start)(void);
    int32_t (*Lock)(void);
    int32_t (*Unlock)(void);
    void *Private;
} wd_ad4134_resources_t;

extern wd_adc_driver_adc_ac_t wd_drv_adc_ac;
extern wd_ad4134_resources_t *const wd_ad4134_res_adc_ac;
/* driver event callback and SPI completion / error dispatch (SPI driver shim or HAL callbacks) */
void wd_adc_ad4134_callback(uint32_t event);
void wd_ad4134_spi_complete_handler(SPI_TypeDef *reg);
void wd_ad4134_spi_error_handler(SPI_TypeDef *reg, uint32_t event);

#ifndef NDEBUG
typedef struct wd_debug_adc_ad4134_s {
    wd_ad4134_resources_t *Resource;
    wd_ad4134_info_t *Info;
    wd_adc_driver_adc_ac_t *Driver;
} const wd_ad4134_debug_adc_ad4134_t;

extern wd_ad4134_debug_adc_ad4134_t wd_ad4134_dbg_adc_ad4134;
#endif

#endif //#if (RTE_COMPONENT_ADC_AD4134)
#endif //	#if 1 // 토글


#endif /* WD_COMPONENT_WD_AD4134_H */
