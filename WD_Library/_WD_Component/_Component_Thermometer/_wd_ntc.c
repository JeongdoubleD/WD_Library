/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_ntc.c
 *
 *  Created on: Oct 27, 2026
 *      Author: doubled
 */

#include "_wd_ntc.h"

#if (RTE_COMPONENT_THERMOMETER_NTC)

/***************************************************************************************************************************************************************
 **********  NTC(NTCALUG02A103F161A) + 내부 ADC(PA1_C, ADC1_INP1) 회로 상수
 **********  REF3V(3.0V) -> R246(330옴) -> [NODE, PA1_C로 버퍼 후 입력] -> NTC -> GND
 **************************************************************************************************************************************************************/
#define NTC_R25_OHM          (10000.0f) // NTCALUG02A103F161A 데이터시트 R25
#define NTC_BETA_25_85       (3435.0f)  // NTCALUG02A103F161A 데이터시트 B25/85
#define NTC_T25_KELVIN       (298.15f)  // 25℃ = 298.15K
#define NTC_DIVIDER_R246_OHM (330.0f)   // REF3V - R246 - NODE 분압 저항
#define NTC_DIVIDER_REF3V_VOLTAGE \
    (3.0f) // LM4040DIM3-3.0 정밀 기준전압 (분압 여기 전압, ADC 기준전압과 다름)
#define NTC_ADC_VREF_VOLTAGE    (3.3f)     // ADC 기준전압 VDDA
#define NTC_ADC_RESOLUTION_MAX  (65535.0f) // 16bit
#define NTC_ADC_OVERSAMPLE_CNT  16         // 폴링 반복 평균 횟수
#define NTC_ADC_POLL_TIMEOUT_MS 10

static float NTC_Convert_Voltage_To_Celsius(float v_node) {
    float r_ntc, inv_t_kelvin;

    // 분압 양 끝단에 너무 가까우면 NTC 단선/단락으로 판단 (log 연산 특이점 방지)
    if ((v_node <= 0.02f) || (v_node >= (NTC_DIVIDER_REF3V_VOLTAGE - 0.02f))) {
        return (WD_NAN);
    }

    r_ntc = NTC_DIVIDER_R246_OHM * v_node / (NTC_DIVIDER_REF3V_VOLTAGE - v_node);
    inv_t_kelvin = (1.0f / NTC_T25_KELVIN) + ((1.0f / NTC_BETA_25_85) * logf(r_ntc / NTC_R25_OHM));

    return ((1.0f / inv_t_kelvin) - 273.15f);
}

static int32_t NTC_Read_ADC_Voltage(float *voltage) {
    uint32_t sum = 0;
    uint32_t i;

    for (i = 0; i < NTC_ADC_OVERSAMPLE_CNT; i++) {
        if (HAL_ADC_Start(RTE_COMPONENT_THERMOMETER_NTC_ADC_HANDLE) != HAL_OK) {
            return (WD_CODE_ERROR);
        }
        if (HAL_ADC_PollForConversion(RTE_COMPONENT_THERMOMETER_NTC_ADC_HANDLE,
                                      NTC_ADC_POLL_TIMEOUT_MS) != HAL_OK) {
            HAL_ADC_Stop(RTE_COMPONENT_THERMOMETER_NTC_ADC_HANDLE);
            return (WD_CODE_ERROR_TIMEOUT);
        }
        sum += HAL_ADC_GetValue(RTE_COMPONENT_THERMOMETER_NTC_ADC_HANDLE);
        HAL_ADC_Stop(RTE_COMPONENT_THERMOMETER_NTC_ADC_HANDLE);
    }

    *voltage = ((float)sum / (float)NTC_ADC_OVERSAMPLE_CNT) * NTC_ADC_VREF_VOLTAGE /
               NTC_ADC_RESOLUTION_MAX;

    return (WD_CODE_OK);
}

/***************************************************************************************************************************************************************
 **********  Driver
 **************************************************************************************************************************************************************/
static const wd_ds18b20_thermometer_capabilities_t Thermometer_DriverCapabilities = {
    .TimeOut = 100000,
};

static wd_ds18b20_thermometer_info_t Thermometer_Info = {
    0,
};

static wd_ds18b20_thermometer_resources_t Thermometer_Resources = {
    .Capabilities = &Thermometer_DriverCapabilities, .Info = &Thermometer_Info};

#undef LOCAL_RES
static wd_ds18b20_thermometer_resources_t *LOCAL_RES = &Thermometer_Resources;

static int32_t thermometer_initialize(wd_ds18b20_thermometer_signal_event_t cb_event) {
    if (LOCAL_RES->Info->State & WD_STATE_INITIALIZED) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Thermometer NTC(Internal ADC) Constructor");

    memset(&Thermometer_Info, 0, sizeof(Thermometer_Info));

    LOCAL_RES->Info->Temperature = WD_NAN;
    LOCAL_RES->Info->CB_Event = cb_event;
    LOCAL_RES->Info->State = WD_STATE_INITIALIZED;

    return (WD_CODE_OK);
}

static int32_t thermometer_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Thermometer_NTC PowerControl:WD_POWER_OFF");

        LOCAL_RES->Info->State &= ~(WD_STATE_POWERED | WD_STATE_CONFIGURED);
    } break;

    case WD_POWER_FULL: {
        float voltage;

        if ((LOCAL_RES->Info->State & WD_STATE_INITIALIZED) == 0U) {
            return (WD_CODE_OK);
        }

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Thermometer_NTC PowerControl:WD_POWER_FULL");

        if ((NTC_Read_ADC_Voltage(&voltage) == WD_CODE_OK) &&
            (!isnan(NTC_Convert_Voltage_To_Celsius(voltage)))) {
            wd_core_status_hook(WD_STATUS_SOURCE_TEMPERATURE_SENSOR, WD_STATUS_OK);
            WD_LOG_DEBUG("Temperature Sensor Connect");
        } else {
            wd_core_status_hook(WD_STATUS_SOURCE_TEMPERATURE_SENSOR, WD_STATUS_ERROR);
            WD_LOG_ERROR("Temperature Sensor Error !!!");
        }

        LOCAL_RES->Info->State |= (WD_STATE_POWERED | WD_STATE_CONFIGURED);
    } break;

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);

    default:
        return (WD_CODE_ERROR);
    }

    return (WD_CODE_OK);
}

static int32_t thermometer_get_temperature(float *data) {
    int32_t ret = WD_CODE_OK;
    float voltage;
    float temperature = WD_NAN;

    if ((NTC_Read_ADC_Voltage(&voltage) == WD_CODE_OK) &&
        (!isnan(temperature = NTC_Convert_Voltage_To_Celsius(voltage)))) {
        LOCAL_RES->Info->Temperature = temperature;
        //		WD_PRINT_DEBUG(WD_MSG_DEBUG_CRLF, "Sensor temperature is %f degrees", (double) temperature);
    } else {
        ret = WD_CODE_ERROR;
    }

    if (data) {
        *data = temperature;
    }

    return (ret);
}

static int32_t thermometer_control(uint32_t control, uint32_t arg) {
    UNUSED(arg);
    UNUSED(control);

    // 내부 ADC는 Get_Temperature() 호출 시 즉시 변환하므로 별도 변환 트리거가 필요 없음
    return (WD_CODE_OK);
}

static wd_ds18b20_thermometer_status_t thermometer_get_status(void) {
    return (LOCAL_RES->Info->Status);
}

static wd_ds18b20_thermometer_resources_t *thermometer_get_priv(void) {
    return (LOCAL_RES);
}

wd_thermometer_driver_t wd_drv_thermometer = {
    .Initialize = thermometer_initialize,
    .PowerControl = thermometer_power_control,
    .Get_Temperature = thermometer_get_temperature,
    .Control = thermometer_control,
    .Get_Status = thermometer_get_status,
    .Get_Priv = thermometer_get_priv,
};

#endif // RTE_COMPONENT_THERMOMETER_NTC
