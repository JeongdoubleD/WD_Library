/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_ad9837.c
 *
 *  Created on: Jan 5, 2025
 *      Author: doubled
 */

#include "_wd_ad9837.h"

#if 1 // 토글
#if RTE_COMPONENT_ADC_AD4134
#else
#if RTE_COMPONENT_DDS_AD9837_MAIN

#define SPI_BAUDRATE_DDS_MAIN 20000000
#define DDS_AD9837_MCLK       16000000
#define DDS_AD9837_WAVE_POINT 128 // FFT Size 4096

static uint32_t AD9837_FreqCalc(float desiredFrequency);
static void AD9837_Reset(void);
static void AD9837_SetMode(wd_ad9837_all_mode_t newMode);
static void AD9837_SelectFreqReg(wd_ad9837_all_freqreg_t reg);
static void AD9837_SelectPhaseReg(wd_ad9837_all_phasereg_t reg);
static void AD9837_SetFreqAdjustMode(wd_ad9837_all_freqadjustmode_t newMode);
static void AD9837_AdjustPhaseShift(wd_ad9837_all_phasereg_t reg, uint16_t newPhase);
static void AD9837_AdjustFreq_Mode_32(wd_ad9837_all_freqreg_t reg,
                                      wd_ad9837_all_freqadjustmode_t mode, uint32_t newFreq);
static void AD9837_AdjustFreq_32(wd_ad9837_all_freqreg_t reg, uint32_t newFreq);
static void AD9837_Control(wd_ad9837_all_ad9837_control_t control);

/***************************************************************************************************************************************************************
 **********  Soft SPI AD9837
 **************************************************************************************************************************************************************/
static wd_softspi_info_t Driver_SPI_Soft_AD9837_Info = {
    0,
};
static wd_softspi_resources_t Driver_SPI_Soft_AD9837_Resources = {
    .This = (void *)&Driver_SPI_Soft_AD9837_Resources,
    .Name = "AD9837",
    .Info = &Driver_SPI_Soft_AD9837_Info,
    .IO =
        {
            .CS = RTE_PORT_SW_SPI_DDS_CS,
            .CLK = RTE_PORT_SW_SPI_SCLK,
            .MOSI = RTE_PORT_SW_SPI_MOSI,
            .MISO = 0, //RTE_CONFIG_IDLE_PIN_NC,
        },
    .Baudrate = 40000000,
    .Mode = WD_DRIVER_SPI_SOFT_MODE_2,
};

/***************************************************************************************************************************************************************
 **********  Driver
 **************************************************************************************************************************************************************/
static wd_ad9837_all_info_t DDS_AD9837_Info = {
    0,
};
#if RTE_RTOS
// Mutex 관련 메모리 및 속성 정의
static osMutexId_t Mutex_DDS_AD9837;               // Mutex 핸들
static osStaticMutexDef_t Mutex_DDS_AD9837_cb_mem; // Mutex 제어 블록 메모리
static const osMutexAttr_t Mutex_DDS_AD9837_Attr = {.name = "Mutex DDS_AD9837",
                                                    .attr_bits =
                                                        osMutexRecursive | osMutexPrioInherit,
                                                    .cb_mem = &Mutex_DDS_AD9837_cb_mem,
                                                    .cb_size = sizeof(Mutex_DDS_AD9837_cb_mem)};

// Mutex 초기화
static int32_t res_start(void) {
#if defined(RTE_RTOS)
    Mutex_DDS_AD9837 = osMutexNew(&Mutex_DDS_AD9837_Attr); // Mutex 생성
    if (Mutex_DDS_AD9837 == NULL) {
#ifndef NDEBUG
        WD_LOG_ERROR("Error: Mutex DDS_AD9837 creation failed\n"); // 디버그 모드에서 오류 출력
#endif
        return WD_CODE_ERROR;
    }
#endif
    return WD_CODE_OK;
}

// Mutex 잠금
static int32_t res_lock(void) {
    if (!WD_CORE_IS_IN_ISR()) { // ISR 내부가 아닌 경우에만 Mutex 동작
        if (osMutexAcquire(Mutex_DDS_AD9837, RTE_RTOS_MUTEX_TIMEOUT) != osOK) { // Mutex 획득
            WD_BREAK(); // 디버그 모드에서 멈춤 (선택 사항)
            return WD_CODE_ERROR_TIMEOUT;
        }
    }
    return WD_CODE_OK;
}

// Mutex 해제
static int32_t res_unlock(void) {
    if (!WD_CORE_IS_IN_ISR()) {           // ISR 내부가 아닌 경우에만 Mutex 해제
        osMutexRelease(Mutex_DDS_AD9837); // Mutex 해제
    }
    return WD_CODE_OK;
}
#else
static void res_start(void) {
}
static void res_lock(void) {
}
static void res_unlock(void) {
}
#endif

static wd_ad9837_resources_t DDS_AD9837_Resources = {
    .Capabilities =
        {
            .BaudRate = SPI_BAUDRATE_DDS_MAIN,
            .MCLK = DDS_AD9837_MCLK,
            .Wave_Point_Num = DDS_AD9837_WAVE_POINT,
            .Flag_Wave_Square = true,
        },
    .Info = &DDS_AD9837_Info,
    .IO =
        {
            .CS = RTE_PORT_SW_SPI_DDS_CS,
            .CLK = RTE_PORT_SW_SPI_SCLK,
            .MOSI = RTE_PORT_SW_SPI_MOSI,
        },
    .Drv = &Driver_SPI_Soft_AD9837_Resources,
    .Handle = NULL, //NULL
    .Reg = NULL,    //NULL
    .Start = res_start,
    .Lock = res_lock,
    .Unlock = res_unlock,
};

#undef LOCAL_DRV
#define LOCAL_DRV (&DRV_DDS_AD9837_MAIN) // 수정 dj
__USED static wd_ad9837_resources_t *const LOCAL_RES = &DDS_AD9837_Resources;

uint32_t wd_ad9837_all_set_mclk(uint32_t frequency) {
    uint32_t timer_clock;
    uint32_t prescaler;
    uint32_t period;
    uint32_t divisor;
    uint32_t lower_divisor;
    uint32_t upper_divisor;

    timer_clock = HAL_RCC_GetPCLK1Freq();

    if ((RCC->D1CFGR & RCC_D1CFGR_D1PPRE) != RCC_D1CFGR_D1PPRE_DIV1) {
        timer_clock *= 2;
    }

#if 1
    prescaler = (timer_clock / frequency / 0xFFFF);
    timer_clock /= (prescaler + 1);

    divisor = timer_clock / frequency;

    if (divisor == 0) {
        divisor = timer_clock / (LOCAL_RES->Capabilities.MCLK / (prescaler + 1));
    }

    if ((timer_clock % frequency) == 0) {
        lower_divisor = divisor;
        upper_divisor = divisor;
    } else {
        uint32_t i;

        i = divisor;

        do {
            if ((timer_clock % (++i)) == 0) {
                upper_divisor = i;
                i = 0;
            }
        } while (i);

        i = divisor;
        do {
            if ((timer_clock % (--i)) == 0) {
                lower_divisor = i;
                i = 0;
            }
        } while (i);

        divisor = lower_divisor;
    }

    if (divisor == 0) {
        WD_LOG_ERROR();
        divisor = 1;
    }

    period = divisor - 1;
#else
    divisor = timer_clock / frequency;

    if (divisor == 0) {
        divisor = timer_clock / LOCAL_RES->Capabilities.MCLK;
    }

    if ((timer_clock % frequency) == 0) {
        lower_divisor = divisor;
        upper_divisor = divisor;
    } else {
        uint32_t i;

        i = divisor;

        do {
            if ((timer_clock % (++i)) == 0) {
                upper_divisor = i;
                i = 0;
            }
        } while (i);

        i = divisor;
        do {
            if ((timer_clock % (--i)) == 0) {
                lower_divisor = i;
                i = 0;
            }
        } while (i);

#if 1
        divisor = upper_divisor;
#else
        if ((upper_divisor - divisor) >= (divisor - lower_divisor)) {
            divisor = lower_divisor;
        } else {
            divisor = upper_divisor;
        }
#endif
    }

    if (divisor == 0) {
        WD_LOG_ERROR();
        divisor = 1;
    }

    prescaler = 0;
    period = divisor - 1;
#endif

    WD_PRINT_DEBUG(WD_MSG_DEBUG_CRLF,
                   "prescaler [%u]    period [%u]    frequency [%f]    lower_divisor [%u]     "
                   "divisor [%u]    upper_divisor [%u] ",
                   prescaler + 1, period + 1, (float)timer_clock / (float)(period + 1),
                   lower_divisor, timer_clock / frequency, upper_divisor);

    __HAL_TIM_SET_PRESCALER(RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE, prescaler);
    __HAL_TIM_SET_AUTORELOAD(RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE, period);
    __HAL_TIM_SET_COMPARE(RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE, TIM_CHANNEL_3,
                          (period + 1) / 2);

    return (timer_clock / divisor);
}

static void dds_ad9837_callback(uint32_t event) {
    LOCAL_RES->Info->Event = event;
    if (LOCAL_RES->Info->CB_Event) {
        LOCAL_RES->Info->CB_Event(event);
    }
}

static int32_t dds_ad9837_initialize(wd_dds_signal_event_t cb_event) {
    if (LOCAL_RES->Info->Flags & (1UL << 0)) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "DDS_AD9837 Constructor");

    memset(LOCAL_RES->Info, 0, sizeof(wd_ad9837_all_info_t));

    if (cb_event) {
        LOCAL_RES->Info->CB_Event = cb_event;
    }

    LOCAL_RES->Start();

    LOCAL_RES->Info->Flags = (1UL << 0);

    return (WD_CODE_OK);
}

static int32_t dds_ad9837_uninitialize(void) {
    return (WD_CODE_OK);
}

static int32_t dds_ad9837_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "DDS AD9837 PowerControl:WD_POWER_OFF");

        LOCAL_RES->Info->Flags &= ~((1UL << 1) | (1UL << 2));
    } break;

    case WD_POWER_FULL: {
        if ((LOCAL_RES->Info->Flags & (1UL << 0)) == 0U) {
            return (WD_CODE_ERROR);
        } else if (LOCAL_RES->Info->Flags & (1UL << 1)) {
            return (WD_CODE_OK);
        }

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "DDS AD9837 PowerControl:WD_POWER_FULL");

        //        wd_gpio_config(RTE_PORT_DDS_CLOCK, WD_GPIO_FUNCTION_AUTO);
        //        {
        //          __HAL_RCC_TIM1_CLK_ENABLE();
        //
        //          (RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE)->Instance = RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_REGISTER;
        //          (RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE)->Init.Prescaler = 0;
        //          (RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE)->Init.CounterMode = TIM_COUNTERMODE_UP;
        //          (RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE)->Init.Period = 119;
        //          (RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE)->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        //          (RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE)->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        //
        //          if (HAL_TIM_PWM_Init(RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE) != HAL_OK)
        //          {
        //            WD_LOG_ERROR();
        //          }
        //
        //          TIM_OC_InitTypeDef sConfigOC;
        //          sConfigOC.OCMode = TIM_OCMODE_PWM1;
        //          sConfigOC.Pulse = 60;
        //          sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
        //          sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
        //          sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
        //          sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
        //          sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
        //          if (HAL_TIM_PWM_ConfigChannel(RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
        //          {
        //            WD_BOARD_ERROR_HANDLER();
        //          }
        //          wd_ad9837_all_set_mclk(LOCAL_RES->Capabilities.MCLK);
        //          HAL_TIM_PWM_Start(RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE, TIM_CHANNEL_3);
        //        }

        wd_softspi_power_control(LOCAL_RES->Drv, WD_POWER_OFF);
        WD_CHECK(wd_softspi_initialize(LOCAL_RES->Drv, dds_ad9837_callback));
        WD_CHECK(wd_softspi_power_control(LOCAL_RES->Drv, WD_POWER_FULL));
        WD_CHECK(wd_softspi_control(LOCAL_RES->Drv,
                                    WD_SPI_SOFT_MODE_MASTER | WD_SPI_SOFT_DATA_BITS(8) |
                                        WD_SPI_SOFT_CPOL1_CPHA0 | WD_SPI_SOFT_MSB_LSB |
                                        WD_SPI_SOFT_SS_MASTER_HW_OUTPUT,
                                    LOCAL_RES->Capabilities.BaudRate));
        WD_CHECK(wd_softspi_control(LOCAL_RES->Drv, WD_SPI_SOFT_SET_DEFAULT_TX_VALUE, 0xCC));


        AD9837_Reset();
        AD9837_SetMode(AD9837_SQUARE); // 기본 동작 구형파
        AD9837_SelectFreqReg(AD9837_FREQ0);
        AD9837_SelectPhaseReg(AD9837_PHASE0);
        AD9837_SetFreqAdjustMode(AD9837_FULL);
        AD9837_Control(AD9837_DAC_OFF);

        DRV_DDS_AD9837_MAIN.Set_Config(AD9837_DAC_OFF, 0, 0.0f, 0.0f);

        LOCAL_RES->Info->Flags |= ((1UL << 1) | (1UL << 2));
    } break;

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);
    }

    return (WD_CODE_OK);
}

static int32_t dds_ad9837_set_config(uint32_t power, uint32_t mode, fpu_t freq, fpu_t phase) {
    int32_t ret = WD_CODE_OK;

    //  if(    __ASSERT(WD_RANGE(0.0, freq,  16000000.0) || isnan(freq))
    //      || __ASSERT(WD_RANGE(0.0, phase, 360.0) || isnan(phase))
    //    )
    //  {
    //    return (WD_CODE_ERROR);
    //  }
    if (!(WD_RANGE(0.0, freq, 16000000.0) || isnan(freq)) ||
        !(WD_RANGE(0.0, phase, 360.0) || isnan(phase))) {
        return (WD_CODE_ERROR);
    }

    if ((power == AD9837_DAC_ON) && (freq == 0.0) && (phase == 0.0)) {
        AD9837_Reset();
    }

    if (power) {
        AD9837_Control((wd_ad9837_all_ad9837_control_t)power);
    }

    if (mode) {
        if ((LOCAL_RES->Capabilities.Flag_Wave_Square == false) && (mode == AD9837_SQUARE)) {
            AD9837_SetMode(AD9837_TRIANGLE);
        } else {
            AD9837_SetMode((wd_ad9837_all_mode_t)mode);
        }
    }

    if (freq >= 0.0f) {
        uint32_t value;

        LOCAL_RES->Info->MCLK = freq * LOCAL_RES->Capabilities.Wave_Point_Num;

        if (LOCAL_RES->Info->MCLK > LOCAL_RES->Capabilities.MCLK) {
            LOCAL_RES->Info->MCLK = LOCAL_RES->Capabilities.MCLK;
        } else if (LOCAL_RES->Info->MCLK <= LOCAL_RES->Capabilities.Wave_Point_Num) {
            if (freq >= 0.01) {
                LOCAL_RES->Info->MCLK = LOCAL_RES->Capabilities.Wave_Point_Num;
            } else if (freq == 0.0) {
                LOCAL_RES->Info->MCLK = LOCAL_RES->Capabilities.MCLK;
            } else {
                LOCAL_RES->Info->MCLK = 4;
            }
        }

        // LOCAL_RES->Info->MCLK = wd_ad9837_all_set_mclk(LOCAL_RES->Info->MCLK);
        LOCAL_RES->Info->MCLK = DDS_AD9837_MCLK;
#if 0
    wd_softspi_control (LOCAL_RES->Drv, WD_SPI_SOFT_SET_BUS_SPEED, LOCAL_RES->Info->MCLK / 20);
#endif
        value = AD9837_FreqCalc(freq);
        AD9837_AdjustFreq_32(AD9837_FREQ0, value);

        LOCAL_RES->Info->Frequency = freq;
        WD_LOG_INFO("[DDS]    MCLK : %u    Frequency : %.12f", LOCAL_RES->Info->MCLK,
                    (double)(value * ((float)LOCAL_RES->Info->MCLK / 268435456.0f)));
    }

    if (phase >= 0.0f) {
        uint16_t temp =
            (uint16_t)(phase * 4096 /
                       360 /* DEV_WAVEFORM3_PHASE_RESOLUTION / DEV_WAVEFORM3_PHASE_SET_MAX*/);

        AD9837_AdjustPhaseShift(AD9837_PHASE0, temp);
    }

    return (ret);
}

static int32_t dds_ad9837_control(uint32_t control, uint32_t arg) {
    UNUSED(control);
    UNUSED(arg);

    switch (control & WD_DDS_AD9837_CONTROL_MSK) {
    case WD_DDS_AD9837_CALIBRATION: {
        // TBD
    } break;
    }

    return (WD_CODE_OK);
}

static wd_dds_status_t dds_ad9837_get_status(void) {
    wd_dds_status_t status = {0};

    return (status);
}


static void dds_ad9837_validation(void) {
#if (API_VALIDATION)

#endif
}

wd_ad9837_resources_t *const RES_DDS_AD9837_MAIN = &DDS_AD9837_Resources;

wd_dds_driver_dds_t DRV_DDS_AD9837_MAIN = {
    .Initialize = dds_ad9837_initialize,
    .Uninitialize = dds_ad9837_uninitialize,
    .PowerControl = dds_ad9837_power_control,
    .Set_Config = dds_ad9837_set_config,
    .Control = dds_ad9837_control,
    .Get_Status = dds_ad9837_get_status,
    .Validation = dds_ad9837_validation,
};

#ifndef NDEBUG
__USED wd_ad9837_debug_dds_ad9837_t DBG_DDS_AD9837_MAIN = {
    .Resource = &DDS_AD9837_Resources,
    .Info = DDS_AD9837_Resources.Info,
    .Driver = LOCAL_DRV,
    .Driver_Resource = &Driver_SPI_Soft_AD9837_Resources,
};
#endif


// Helper function, used to calculate the integer value to be written to a
//  freq register for a desired output frequency.
// The output frequency is fclk/2^28 * FREQREG. For us, fclk is 16MHz. We can
//  save processor time by specifying a constant for fclk/2^28- .0596. That is,
//  in Hz, the smallest step size for adjusting the output frequency.
static uint32_t AD9837_FreqCalc(float desiredFrequency) {
    uint32_t frequency;
    frequency = (uint32_t)(desiredFrequency / ((float)LOCAL_RES->Info->MCLK / 268435456.0f));
    WD_PRINT_DEBUG(WD_MSG_DEBUG_CRLF, "AD9837 Frequency - input [%.9f]    output [%.9f]",
                   (double)desiredFrequency,
                   (double)(frequency * ((float)LOCAL_RES->Info->MCLK / 268435456.0f)));
    return (frequency);
}

static void AD9837_Write(uint16_t data) {
    uint8_t tx_buf[2];

    tx_buf[0] = (uint8_t)((data & DEV_WAVEFORM3_WR_REG_MASK_MSB) >> 8);
    tx_buf[1] = (uint8_t)(data & DEV_WAVEFORM3_WR_REG_MASK_LSB);

    wd_softspi_send(LOCAL_RES->Drv, tx_buf, 2);
}

// reset the AD part. This will disable all function generation and set the
//  output to approximately mid-level, constant voltage. Since we're resetting,
//  we can also forego worrying about maintaining the state of the other bits
//  in the config register.
static void AD9837_Reset(void) {
    LOCAL_RES->Info->Register = 0;
    LOCAL_RES->Info->Register |= (WAVEFORM3_REG_RESET_BIT | WAVEFORM3_REG_SLEEP12_BIT);
    AD9837_Write(LOCAL_RES->Info->Register);
    wd_time_wait_ms(10);
    AD9837_AdjustFreq_Mode_32(AD9837_FREQ0, AD9837_FULL, 0);
    AD9837_AdjustFreq_Mode_32(AD9837_FREQ1, AD9837_FULL, 0);
    AD9837_AdjustPhaseShift(AD9837_PHASE0, 0);
    AD9837_AdjustPhaseShift(AD9837_PHASE1, 0);
    LOCAL_RES->Info->Register &= ~(WAVEFORM3_REG_RESET_BIT);
    AD9837_Write(LOCAL_RES->Info->Register);
    wd_time_wait_ms(1);
}

// Set the mode of the part. The mode (trinagle, sine, or square) is set by
//  three bits in the status register: D5 (OPBITEN), D3 (DIV2), and D1 (MODE).
//  Here's a nice truth table for those settings:
//  D5 D1 D3
//  0  0  x   Sine wave output
//  0  1  x   Triangle wave output
//  1  0  0   Square wave @ 1/2 frequency
//  1  0  1   Square wave @ frequency
//  1  1  x   Not allowed
static void AD9837_SetMode(wd_ad9837_all_mode_t newMode) {
    // We want to adjust the three bits in the config register that we're
    //  interested in without screwing up anything else. Unfortunately, this
    //  part is write-only, so we need to maintain a local shadow, adjust that,
    //  then write it.
    LOCAL_RES->Info->Register &= ~0x002A; // Clear D5, D3, and D1.
    // This switch statement sets the appropriate bit in the config register.
    switch (newMode) {
    case AD9837_TRIANGLE:
        LOCAL_RES->Info->Register |= 0x0002;
        break;
    case AD9837_SQUARE_2:
        LOCAL_RES->Info->Register |= 0x0020;
        break;
    case AD9837_SQUARE:
        LOCAL_RES->Info->Register |= 0x0028;
        break;
    case AD9837_SINE:
        LOCAL_RES->Info->Register |= 0x0000;
        break;
    }
    AD9837_Write(LOCAL_RES->Info->Register); // Now write our shadow copy to the part.
}

// The AD9837 has two frequency registers that can be independently adjusted.
//  This allows us to fiddle with the value in one without affecting the output
//  of the device. The register used for calculating the output is selected by
//  toggling bit 11 of the config register.
static void AD9837_SelectFreqReg(wd_ad9837_all_freqreg_t reg) {
    // For register FREQ0, we want to clear bit 11.
    if (reg == AD9837_FREQ0) {
        LOCAL_RES->Info->Register &= ~0x0800;
    }
    // Otherwise, set bit 11.
    else {
        LOCAL_RES->Info->Register |= 0x0800;
    }
    AD9837_Write(LOCAL_RES->Info->Register);
}

// Similarly, there are two phase registers, selected by bit 10 of the config
//  register.
static void AD9837_SelectPhaseReg(wd_ad9837_all_phasereg_t reg) {
    if (reg == AD9837_PHASE0) {
        LOCAL_RES->Info->Register &= ~0x0400;
    } else {
        LOCAL_RES->Info->Register |= 0x0400;
    }
    AD9837_Write(LOCAL_RES->Info->Register);
}

// The frequency registers are 28 bits in size (combining the lower 14 bits of
//  two 16 bit writes; the upper 2 bits are the register address to write).
//  Bits 13 and 12 of the config register select how these writes are handled:
//  13 12
//  0  0   Any write to a frequency register is treated as a write to the lower
//          14 bits; this allows for fast fine adjustment.
//  0  1   Writes are send to upper 14 bits, allowing for fast coarse adjust.
//  1  x   First write of a pair goes to LSBs, second to MSBs. Note that the
//          user must, in this case, be certain to write in pairs, to avoid
//          unexpected results!
static void AD9837_SetFreqAdjustMode(wd_ad9837_all_freqadjustmode_t newMode) {
    // Start by clearing the bits in question.
    LOCAL_RES->Info->Register &= ~0x3000;
    // Now, adjust the bits to match the truth table above.
    switch (newMode) {
    case AD9837_COARSE: // D13:12 = 01
        LOCAL_RES->Info->Register |= 0x1000;
        break;
    case AD9837_FINE: // D13:12 = 00
        break;
    case AD9837_FULL: // D13:12 = 1x (we use 10)
        LOCAL_RES->Info->Register |= 0x2000;
        break;
    }
    AD9837_Write(LOCAL_RES->Info->Register);
}

// The phase shift value is 12 bits long; it gets routed to the proper phase
//  register based on the value of the 3 MSBs (4th MSB is ignored).
static void AD9837_AdjustPhaseShift(wd_ad9837_all_phasereg_t reg, uint16_t newPhase) {
    // First, let's blank the top four bits. Just because it's the right thing
    //  to do, you know?
    newPhase &= ~0xF000;
    // Now, we need to set the top three bits to properly route the data.
    //  D15:D13 = 110 for PHASE0...
    if (reg == AD9837_PHASE0) {
        newPhase |= 0xC000;
    }
    // ... and D15:D13 = 111 for PHASE1.
    else {
        newPhase |= 0xE000;
    }
    AD9837_Write(newPhase);
}

// Okay, now we're going to handle frequency adjustments. This is a little
//  trickier than a phase adjust, because in addition to properly routing the
//  data, we need to know whether we're writing all 32 bits or just 16. I've
//  overloaded this function call for three cases: write with a mode change (if
//  one is needed), and write with the existing mode.

// Adjust the contents of the given register, and, if necessary, switch mode
//  to do so. This is probably the slowest method of updating a register.
static void AD9837_AdjustFreq_Mode_32(wd_ad9837_all_freqreg_t reg,
                                      wd_ad9837_all_freqadjustmode_t mode, uint32_t newFreq) {
    AD9837_SetFreqAdjustMode(mode);
    // Now, we can just call the normal 32-bit write.
    AD9837_AdjustFreq_32(reg, newFreq);
}

// Adjust the contents of the register, but assume that the write mode is
//  already set to full. Note that if it is NOT set to full, bad things will
//  happen- the coarse or fine register will be updated with the contents of
//  the upper 14 bits of the 28 bits you *meant* to send.
static void AD9837_AdjustFreq_32(wd_ad9837_all_freqreg_t reg, uint32_t newFreq) {
    // We need to split the 32-bit input into two 16-bit values, blank the top
    //  two bits of those values, and set the top two bits according to the
    //  value of reg.
    // Start by acquiring the low 16-bits...
    uint16_t temp = (uint16_t)newFreq;
    // ...and blanking the first two bits.
    temp &= ~0xC000;
    // Now, set the top two bits according to the reg parameter.
    if (reg == AD9837_FREQ0) {
        temp |= 0x4000;
    } else {
        temp |= 0x8000;
    }
    // Now, we can write temp out to the device.
    AD9837_Write(temp);
    // Okay, that's the lower 14 bits. Now let's grab the upper 14.
    temp = (uint16_t)(newFreq >> 14);
    // ...and now, we can just repeat the process.
    temp &= ~0xC000;
    // Now, set the top two bits according to the reg parameter.
    if (reg == AD9837_FREQ0) {
        temp |= 0x4000;
    } else {
        temp |= 0x8000;
    }
    // Now, we can write temp out to the device.
    AD9837_Write(temp);
}

static void AD9837_Control(wd_ad9837_all_ad9837_control_t control) {
    if (control == AD9837_DAC_ON) {
        LOCAL_RES->Info->Register &= ~(WAVEFORM3_REG_SLEEP12_BIT);
    } else {
        LOCAL_RES->Info->Register |= (WAVEFORM3_REG_SLEEP12_BIT);
    }
    AD9837_Write(LOCAL_RES->Info->Register);
}

#endif // 	#if RTE_COMPONENT_DDS_AD9837_MAIN
#endif // 	#if RTE_COMPONENT_ADC_AD4134
#endif //	#if 1	// 토글
