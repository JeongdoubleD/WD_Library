/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_ads127l.c
 *
 *  Created on: Jan 15, 2025
 *      Author: user
 */

#include "_wd_ads127l.h"

#if 1 // 토글
#if RTE_COMPONENT_ADC_ADS127

#define SPI_BAUDRATE_ADS127 24000000

#define ADC_ADS127L01_MCLK               16000000
#define ADC_ADS127L01_XFER_IGNORE_NUMBER 1


static wd_ads127l_info_t ADC_ADS127L01_Info = {
    0,
};

#if (RTE_RTOS)
static osMutexId_t Mutex_ADC_ADS127L01;
static osStaticMutexDef_t
    Mutex_ADC_ADS127L01_cb_mem; //[sizeof(osMutexId_t)] __attribute__((section(".bss.os.mutex")));  // Mutex 제어 블록 메모리
static const osMutexAttr_t Mutex_ADC_ADS127L01_Attr = {
    .name = "Mutex AC ADC",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = &Mutex_ADC_ADS127L01_cb_mem,
    .cb_size = sizeof(Mutex_ADC_ADS127L01_cb_mem)};

static int32_t res_start(void) {
#if (RTE_RTOS)
    Mutex_ADC_ADS127L01 = osMutexNew(&Mutex_ADC_ADS127L01_Attr);
    if (Mutex_ADC_ADS127L01 == NULL) {
#ifndef NDEBUG
        WD_LOG_ERROR("Mutex AC ADC New Error");
#endif
        return (WD_CODE_ERROR);
    }
#endif

    return (WD_CODE_OK);
}

static int32_t res_lock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        if (osMutexAcquire(Mutex_ADC_ADS127L01, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
            return (WD_CODE_ERROR_TIMEOUT);
        }
    }

    return (WD_CODE_OK);
}

static int32_t res_unlock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(Mutex_ADC_ADS127L01);
    }

    return (WD_CODE_OK);
}
#else
static static void res_start(void) {
}
static static void res_lock(void) {
}
static static void res_unlock(void) {
}
#endif

wd_ads127l_resources_t ADC_ADS127L01_Resources = {
    .Capabilities =
        {
            .BaudRate = SPI_BAUDRATE_ADS127,
            .MCLK = ADC_ADS127L01_MCLK,
        },
    .Info = &ADC_ADS127L01_Info,
    .IO =
        {
            .DRDY = RTE_CONFIG_IDLE_PIN_NC,
            .SPI_CS = RTE_CONFIG_IDLE_PIN_NC,
        },
    .Handle_V = RTE_COMPONENT_ADC_ADS127L_V_DRIVER_HANDLE,
    .Reg_V = SPI1,
    .Handle_I = RTE_COMPONENT_ADC_ADS127L_I_DRIVER_HANDLE,
    .Reg_I = SPI2,
    .Start = res_start,
    .Lock = res_lock,
    .Unlock = res_unlock,
};


#undef LOCAL_DRV
#define LOCAL_DRV (&wd_drv_adc_ac)
__USED static wd_ads127l_resources_t *const LOCAL_RES = &ADC_ADS127L01_Resources;
void EXTI0_IRQHandler(void) {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);

    if (wd_core_system_mode < WD_SYSTEM_MODE_NORMAL) // 추후 삭제 (리소스 많이 먹음)
    {
        WD_LOG_ERROR();
    } else if (LOCAL_RES->Info->Xfer.Ignore) {
        LOCAL_RES->Info->Xfer.Ignore--;
    } else {
        if (LL_DMA_IsActiveFlag_TC6(DMA1)) {
            LL_DMA_ClearFlag_TC6(DMA1);
            LOCAL_RES->Info->Status.Conversion_Finish = true;
            if (LOCAL_RES->Info->Xfer.Mode == WD_ADC_CONVERSION_MODE_SINGLE) {
                NVIC_DisableIRQ(EXTI0_IRQn);
                return;
            }
        }
#if 0
    else if(LL_DMA_IsActiveFlag_HT6(DMA1))
    {
      LL_DMA_ClearFlag_HT6(DMA1);
    }
    else if(LL_DMA_IsActiveFlag_TE6(DMA1))
    {
      LL_DMA_ClearFlag_TE6(DMA1);
    }
#endif

        /*  if(!LL_SPI_GetRemainingDataFrames(SPI2)) */
        {
            LL_SPI_ClearFlag_EOT(SPI2);
            LL_SPI_StartMasterTransfer(SPI2);
        }

        /*  if(!LL_SPI_GetRemainingDataFrames(SPI1)) */
        {
            LL_SPI_ClearFlag_EOT(SPI1);
            LL_SPI_StartMasterTransfer(SPI1);
        }
    }
}

#if (WD_ADC_DATA_SIZE == 4)
static uint8_t const ADC_ADS127L01_Table_Byte[256] = {
    0x00, 0x30, 0x60, 0x50, 0xc0, 0xf0, 0xa0, 0x90, 0xb0, 0x80, 0xd0, 0xe0, 0x70, 0x40, 0x10, 0x20,
    0x50, 0x60, 0x30, 0x00, 0x90, 0xa0, 0xf0, 0xc0, 0xe0, 0xd0, 0x80, 0xb0, 0x20, 0x10, 0x40, 0x70,
    0xa0, 0x90, 0xc0, 0xf0, 0x60, 0x50, 0x00, 0x30, 0x10, 0x20, 0x70, 0x40, 0xd0, 0xe0, 0xb0, 0x80,
    0xf0, 0xc0, 0x90, 0xa0, 0x30, 0x00, 0x50, 0x60, 0x40, 0x70, 0x20, 0x10, 0x80, 0xb0, 0xe0, 0xd0,
    0x70, 0x40, 0x10, 0x20, 0xb0, 0x80, 0xd0, 0xe0, 0xc0, 0xf0, 0xa0, 0x90, 0x00, 0x30, 0x60, 0x50,
    0x20, 0x10, 0x40, 0x70, 0xe0, 0xd0, 0x80, 0xb0, 0x90, 0xa0, 0xf0, 0xc0, 0x50, 0x60, 0x30, 0x00,
    0xd0, 0xe0, 0xb0, 0x80, 0x10, 0x20, 0x70, 0x40, 0x60, 0x50, 0x00, 0x30, 0xa0, 0x90, 0xc0, 0xf0,
    0x80, 0xb0, 0xe0, 0xd0, 0x40, 0x70, 0x20, 0x10, 0x30, 0x00, 0x50, 0x60, 0xf0, 0xc0, 0x90, 0xa0,
    0xe0, 0xd0, 0x80, 0xb0, 0x20, 0x10, 0x40, 0x70, 0x50, 0x60, 0x30, 0x00, 0x90, 0xa0, 0xf0, 0xc0,
    0xb0, 0x80, 0xd0, 0xe0, 0x70, 0x40, 0x10, 0x20, 0x00, 0x30, 0x60, 0x50, 0xc0, 0xf0, 0xa0, 0x90,
    0x40, 0x70, 0x20, 0x10, 0x80, 0xb0, 0xe0, 0xd0, 0xf0, 0xc0, 0x90, 0xa0, 0x30, 0x00, 0x50, 0x60,
    0x10, 0x20, 0x70, 0x40, 0xd0, 0xe0, 0xb0, 0x80, 0xa0, 0x90, 0xc0, 0xf0, 0x60, 0x50, 0x00, 0x30,
    0x90, 0xa0, 0xf0, 0xc0, 0x50, 0x60, 0x30, 0x00, 0x20, 0x10, 0x40, 0x70, 0xe0, 0xd0, 0x80, 0xb0,
    0xc0, 0xf0, 0xa0, 0x90, 0x00, 0x30, 0x60, 0x50, 0x70, 0x40, 0x10, 0x20, 0xb0, 0x80, 0xd0, 0xe0,
    0x30, 0x00, 0x50, 0x60, 0xf0, 0xc0, 0x90, 0xa0, 0x80, 0xb0, 0xe0, 0xd0, 0x40, 0x70, 0x20, 0x10,
    0x60, 0x50, 0x00, 0x30, 0xa0, 0x90, 0xc0, 0xf0, 0xd0, 0xe0, 0xb0, 0x80, 0x10, 0x20, 0x70, 0x40};

static int32_t ADC_ADS127L01_CRC4x(void const *mem) {
    unsigned char const *data = mem;
    uint8_t crc = 0;

    if (data == NULL) {
        return 0;
    }
    crc <<= 4;
    for (size_t i = 3; i > 0; i--) {
        crc = ADC_ADS127L01_Table_Byte[crc ^ data[i]];
    }

    if ((data[0] & 0xF0) != crc) {
        return WD_CODE_ERROR_CRC;
    }
    return WD_CODE_OK;
}
#endif

fpu_t ADC_ADS127L01_Set_MCLK(fpu_t frequency) {
#if (WD_ADC_ADS127L01_MCLK_SOURCE == WD_ADC_ADS127L01_MCLK_SOURCE_IO)
    uint32_t timer_clock;
    uint32_t prescaler;
    uint32_t period;
    fpu_t value;

    timer_clock = HAL_RCC_GetPCLK1Freq();

    if ((RCC->D1CFGR & RCC_D1CFGR_D1PPRE) != RCC_D1CFGR_D1PPRE_DIV1) {
        timer_clock *= 2;
    }

    if (0xFFFFFFFF / timer_clock > frequency) {
        WD_LOG_ERROR();
    }

    prescaler = 0;
    value = timer_clock / frequency;
    period = (uint32_t)value - 1;
    if (fmod(value, 1.0) >= 0.5) {
        period++;
    }
    value = (fpu_t)timer_clock / (fpu_t)(period + 1);

    //   WD_PRINT_DEBUG(WD_MSG_DEBUG_CRLF, "prescaler [%u]    period [%u]    input [%f]    output [%f]", prescaler + 1, period + 1, frequency, value);
    WD_PRINT_DEBUG("prescaler [%u]    period [%u]    input [%f]    output [%f]", prescaler + 1,
                   period + 1, frequency, value);

    __HAL_TIM_SET_PRESCALER(RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE, prescaler);
    __HAL_TIM_SET_AUTORELOAD(RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE, period);
    __HAL_TIM_SET_COMPARE(RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE, TIM_CHANNEL_3,
                          (period + 1) / 2);

    return (value);
#elif (WD_ADC_ADS127L01_MCLK_SOURCE == WD_ADC_ADS127L01_MCLK_SOURCE_AD9837)
    DRV_DDS_AD9837_MAIN.Set_Config(AD9837_DAC_ON, AD9837_SQUARE, frequency, 0.0f);
    return (RES_DDS_AD9837_MAIN->Info->Frequency);

#endif
}

void wd_adc_ads127l01_callback(uint32_t event) {
    LOCAL_RES->Info->Event |= event;
    if (LOCAL_RES->Info->CB_Event) {
        LOCAL_RES->Info->CB_Event(event);
    }
}

static int32_t adc_ads127l01_initialize(wd_adc_signal_event_t cb_event) {
    if (LOCAL_RES->Info->Flags & (1UL << 0)) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "ADC_ADS127L01 Constructor");

    memset(LOCAL_RES->Info, 0, sizeof(wd_ads127l_info_t));

    LOCAL_RES->Info->Sampling_Number = WD_ADC_ADS127L01_SAMPLING_NUMBER;
    LOCAL_RES->Info->Sampling_Cycle_Number = WD_ADC_ADS127L01_SAMPLING_CYCLE_NUMBER;
    LOCAL_RES->Info->Sampling_Count = 256;
    LOCAL_RES->Info->Sampling_Frequency = 1.0;

    if (cb_event) {
        LOCAL_RES->Info->CB_Event = cb_event;
    }

    LOCAL_RES->Start();

    LOCAL_RES->Info->Flags = (1UL << 0);

    return (WD_CODE_OK);
}

static int32_t adc_ads127l01_uninitialize(void) {
    return (WD_CODE_OK);
}

static int32_t adc_ads127l01_power_control(ARM_POWER_STATE state) {
    switch ((int32_t)state) {
    case ARM_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "ADC_ADS127L01 PowerControl:ARM_POWER_OFF");

        LOCAL_RES->Info->Flags &= ~((1UL << 1) | (1UL << 2));
    } break;

    case ARM_POWER_FULL: {
        if ((LOCAL_RES->Info->Flags & (1UL << 0)) == 0U) {
            return (WD_CODE_OK);
        }

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "ADC_ADS127L01 PowerControl:ARM_POWER_FULL");

        wd_gpio_config(RTE_PORT_ADC_V_INT, WD_GPIO_FUNCTION_AUTO);
        wd_gpio_config(RTE_PORT_ADC_V_INT2, WD_GPIO_FUNCTION_AUTO);
        wd_gpio_config(RTE_PORT_ADC_I_INT, WD_GPIO_FUNCTION_AUTO);
        wd_gpio_config(RTE_PORT_ADC_START, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);

        NVIC_DisableIRQ(EXTI0_IRQn);

#if (WD_ADC_ADS127L01_MCLK_SOURCE == WD_ADC_ADS127L01_MCLK_SOURCE_IO)
        wd_gpio_config(RTE_PORT_ADC_V_CLOCK, WD_GPIO_FUNCTION_AUTO);
        wd_gpio_config(RTE_PORT_ADC_I_CLOCK, WD_GPIO_FUNCTION_AUTO);
        {
            __HAL_RCC_TIM5_CLK_ENABLE();

            (RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE)->Instance =
                RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_REGISTER;
            (RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE)->Init.Prescaler = 0;
            (RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE)->Init.CounterMode = TIM_COUNTERMODE_UP;
            (RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE)->Init.Period = 59;
            (RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE)->Init.ClockDivision =
                TIM_CLOCKDIVISION_DIV1;
            (RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE)->Init.AutoReloadPreload =
                TIM_AUTORELOAD_PRELOAD_ENABLE;

            if (HAL_TIM_PWM_Init(RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE) != HAL_OK) {
                WD_LOG_ERROR();
            }

            TIM_OC_InitTypeDef sConfigOC;
            sConfigOC.OCMode = TIM_OCMODE_PWM1;
            sConfigOC.Pulse = 30;
            sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
            sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
            sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
            sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
            sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
            if (HAL_TIM_PWM_ConfigChannel(RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE, &sConfigOC,
                                          TIM_CHANNEL_3) != HAL_OK) {
                WD_BOARD_ERROR_HANDLER();
            }
            HAL_TIM_PWM_Stop(RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE, TIM_CHANNEL_3);
            ADC_ADS127L01_Set_MCLK(LOCAL_RES->Capabilities.MCLK);
        }
#elif (WD_ADC_ADS127L01_MCLK_SOURCE == WD_ADC_ADS127L01_MCLK_SOURCE_AD9837)
//        ADC_ADS127L01_Set_MCLK(1000000);
//        ADC_ADS127L01_Set_MCLK(LOCAL_RES->Capabilities.MCLK);
#endif
        {
            __HAL_RCC_SPI1_CLK_ENABLE();
            __HAL_RCC_DMA1_CLK_ENABLE();

            LOCAL_RES->Drv_V->PowerControl(ARM_POWER_OFF);
            WD_CHECK(LOCAL_RES->Drv_V->Initialize(wd_adc_ads127l01_callback));
            WD_CHECK(LOCAL_RES->Drv_V->PowerControl(ARM_POWER_FULL));
            WD_CHECK(LOCAL_RES->Drv_V->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(8) |
                                                   ARM_SPI_CPOL0_CPHA1 | ARM_SPI_MSB_LSB |
                                                   ARM_SPI_SS_MASTER_SW,
                                               LOCAL_RES->Capabilities.BaudRate));
            WD_CHECK(LOCAL_RES->Drv_V->Control(ARM_SPI_SET_DEFAULT_TX_VALUE, 0xCC));

            __HAL_RCC_SPI2_CLK_ENABLE();
            __HAL_RCC_DMA2_CLK_ENABLE();

            LOCAL_RES->Drv_I->PowerControl(ARM_POWER_OFF);
            WD_CHECK(LOCAL_RES->Drv_I->Initialize(wd_adc_ads127l01_callback));
            WD_CHECK(LOCAL_RES->Drv_I->PowerControl(ARM_POWER_FULL));
            WD_CHECK(LOCAL_RES->Drv_I->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(8) |
                                                   ARM_SPI_CPOL0_CPHA1 | ARM_SPI_MSB_LSB |
                                                   ARM_SPI_SS_MASTER_SW,
                                               LOCAL_RES->Capabilities.BaudRate));
            WD_CHECK(LOCAL_RES->Drv_I->Control(ARM_SPI_SET_DEFAULT_TX_VALUE, 0xCC));

            NVIC_SetPriority(SPI1_IRQn,
                             NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                 WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                 WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
            NVIC_SetPriority(SPI2_IRQn,
                             NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                 WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                 WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));

            NVIC_DisableIRQ(SPI1_IRQn);
            NVIC_DisableIRQ(DMA1_Stream6_IRQn);
            NVIC_DisableIRQ(DMA1_Stream7_IRQn);

            NVIC_DisableIRQ(SPI2_IRQn);
            NVIC_DisableIRQ(DMA2_Stream0_IRQn);
            NVIC_DisableIRQ(DMA2_Stream1_IRQn);

            LL_SPI_Disable(SPI1);
            LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);

            LL_SPI_SetTransferDirection(SPI1, LL_SPI_SIMPLEX_RX);
            LL_SPI_SetDataWidth(SPI1, (WD_ADC_DATA_SIZE == 4) ? LL_SPI_DATAWIDTH_32BIT
                                                              : LL_SPI_DATAWIDTH_8BIT);
            LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);
            LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW);

            LL_DMA_SetPeriphRequest(DMA1, LL_DMA_STREAM_6, LL_DMAMUX1_REQ_SPI1_RX);
            LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_6,
                                            LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
            LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_6, LL_DMA_PRIORITY_HIGH);
            LL_DMA_SetMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MODE_NORMAL);
            LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_PERIPH_NOINCREMENT);
            LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MEMORY_INCREMENT);
            LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_6,
                                 (WD_ADC_DATA_SIZE == 4) ? LL_DMA_PDATAALIGN_WORD
                                                         : LL_DMA_PDATAALIGN_BYTE);
            LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_6,
                                 (WD_ADC_DATA_SIZE == 4) ? LL_DMA_MDATAALIGN_WORD
                                                         : LL_DMA_MDATAALIGN_BYTE);

            LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)&SPI1->RXDR);
            LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)LOCAL_RES->Info->V_Buf);
            LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6,
                                 (WD_ADC_DATA_SIZE == 4) ? WD_ARRAY_SIZE(LOCAL_RES->Info->V_Buf)
                                                         : sizeof(LOCAL_RES->Info->V_Buf));

            LL_DMA_ClearFlag_TC6(DMA1);
            LL_DMA_ClearFlag_TE6(DMA1);
            LL_DMA_ClearFlag_DME6(DMA1);
            LL_SPI_ClearFlag_OVR(SPI1);
            LL_SPI_ClearFlag_EOT(SPI1);
            LL_SPI_SetTransferSize(SPI1, (WD_ADC_DATA_SIZE == 4) ? 1 : WD_ADC_DATA_SIZE);

            __HAL_RCC_SPI2_CLK_ENABLE();
            __HAL_RCC_DMA2_CLK_ENABLE();

            LL_SPI_Disable(SPI2);
            LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);

            LL_SPI_SetTransferDirection(SPI2, LL_SPI_SIMPLEX_RX);
            LL_SPI_SetDataWidth(SPI2, (WD_ADC_DATA_SIZE == 4) ? LL_SPI_DATAWIDTH_32BIT
                                                              : LL_SPI_DATAWIDTH_8BIT);
            LL_SPI_SetClockPhase(SPI2, LL_SPI_PHASE_2EDGE);
            LL_SPI_SetClockPolarity(SPI2, LL_SPI_POLARITY_LOW);

            LL_DMA_SetPeriphRequest(DMA2, LL_DMA_STREAM_0, LL_DMAMUX1_REQ_SPI2_RX);
            LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_0,
                                            LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
            LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_0, LL_DMA_PRIORITY_HIGH);
            LL_DMA_SetMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);
            LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
            LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
            LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_0,
                                 (WD_ADC_DATA_SIZE == 4) ? LL_DMA_PDATAALIGN_WORD
                                                         : LL_DMA_PDATAALIGN_BYTE);
            LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_0,
                                 (WD_ADC_DATA_SIZE == 4) ? LL_DMA_MDATAALIGN_WORD
                                                         : LL_DMA_MDATAALIGN_BYTE);

            LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)&SPI2->RXDR);
            LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)LOCAL_RES->Info->I_Buf);
            LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0,
                                 (WD_ADC_DATA_SIZE == 4) ? WD_ARRAY_SIZE(LOCAL_RES->Info->I_Buf)
                                                         : sizeof(LOCAL_RES->Info->I_Buf));

            LL_DMA_ClearFlag_TC0(DMA2);
            LL_DMA_ClearFlag_TE0(DMA2);
            LL_DMA_ClearFlag_DME0(DMA2);
            LL_SPI_ClearFlag_OVR(SPI2);
            LL_SPI_ClearFlag_EOT(SPI2);
            LL_SPI_SetTransferSize(SPI2, (WD_ADC_DATA_SIZE == 4) ? 1 : WD_ADC_DATA_SIZE);
        }

        NVIC_SetPriority(EXTI0_IRQn,
                         NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                             WD_INTERRUPT_PREEMPT_PRIORITY_REALTIME_HANDLER,
                                             WD_INTERRUPT_SUB_PRIORITY_REALTIME_HANDLER));
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);

        //        ADC_ADS127L01_Set_MCLK(LOCAL_RES->Capabilities.MCLK);

#if (WD_ADC_ADS127L01_MCLK_SOURCE == WD_ADC_ADS127L01_MCLK_SOURCE_IO)
//        HAL_TIM_PWM_Start(RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE, TIM_CHANNEL_3);
#endif

        wd_gpio_write(RTE_PORT_ADC_START, WD_GPIO_ON);

        wd_time_wait_ms(20);

        LOCAL_RES->Info->Flags |= ((1UL << 1) | (1UL << 2));
    } break;

    case ARM_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);
    }

    return (WD_CODE_OK);
}

static int32_t adc_ads127l01_start_conversion(void *v_buf, void *i_buf, uint32_t num,
                                              uint32_t arg) {
    int32_t ret = WD_CODE_OK;

    // if(    __ASSERT(v_buf != NULL)
    //     || __ASSERT(i_buf != NULL)
    //     || __ASSERT(num != NULL)
    //   )
    if (!(v_buf != NULL) || !(i_buf != NULL) || !(num != 0)) {
        WD_LOG_ERROR();
        return (WD_CODE_ERROR);
    }

    LOCAL_RES->Lock();

    LOCAL_RES->Info->Xfer.V_Buf = v_buf;
    LOCAL_RES->Info->Xfer.I_Buf = i_buf;
    LOCAL_RES->Info->Xfer.Num = num;
    LOCAL_RES->Info->Xfer.Mode = arg;
    LOCAL_RES->Info->Xfer.Ignore = ADC_ADS127L01_XFER_IGNORE_NUMBER;

    LOCAL_DRV->Control(WD_ADC_ADS127L01_CONVERSION_START, arg, 0);

    LOCAL_RES->Unlock();

    return (ret);
}

static int32_t adc_ads127l01_poll_conversion(uint32_t timeout, uint32_t arg) {
    uint32_t __time_start = wd_time_get_ms();

    UNUSED(arg);

    LOCAL_RES->Lock();

    do {
        if (LOCAL_RES->Info->Status.Conversion_Finish) {
            LOCAL_RES->Info->Status.Conversion_Finish = false;
            break;
        }
#if 0
    if(LL_DMA_IsActiveFlag_TC6(DMA1))
    {
      LL_DMA_ClearFlag_TC6(DMA1);
    }
    else if(LL_DMA_IsActiveFlag_HT6(DMA1))
    {
      LL_DMA_ClearFlag_HT6(DMA1);
    }
    else if(LL_DMA_IsActiveFlag_TE6(DMA1))
    {
      LL_DMA_ClearFlag_TE6(DMA1);
    }

    if(LL_DMA_IsActiveFlag_TC0(DMA2))
    {
      LL_DMA_ClearFlag_TC0(DMA2);
    }
    else if(LL_DMA_IsActiveFlag_HT0(DMA2))
    {
      LL_DMA_ClearFlag_HT0(DMA2);
    }
    else if(LL_DMA_IsActiveFlag_TE0(DMA2))
    {
      LL_DMA_ClearFlag_TE0(DMA2);
    }
#endif

        if (wd_time_calc_ms(&__time_start) >= timeout) {
            WD_LOG_ERROR("_ADC_ADS127L01_Poll_Conversion Timeout");
            LL_SPI_Disable(SPI1);
            LL_SPI_SetTransferSize(SPI1, (WD_ADC_DATA_SIZE == 4) ? 1 : WD_ADC_DATA_SIZE);
            LL_SPI_Enable(SPI1);

            LL_SPI_Disable(SPI2);
            LL_SPI_SetTransferSize(SPI2, (WD_ADC_DATA_SIZE == 4) ? 1 : WD_ADC_DATA_SIZE);
            LL_SPI_Enable(SPI2);

            LOCAL_DRV->Control(WD_ADC_ADS127L01_RESET, 0, 0);
            break;
        }
    } while (1);

    LOCAL_RES->Unlock();

    return (WD_CODE_OK);
}

static int32_t adc_ads127l01_stop_conversion(uint32_t arg) {
    UNUSED(arg);

    LOCAL_RES->Lock();

    LOCAL_DRV->Control(WD_ADC_ADS127L01_CONVERSION_STOP, 0, 0);
    LOCAL_DRV->Control(WD_ADC_ADS127L01_CONVERSION_RESULT, 0, 0);

    LOCAL_RES->Unlock();

    return (WD_CODE_OK);
}

static int32_t adc_ads127l01_control(uint32_t control, uint32_t arg, fpu_t value) {
    UNUSED(control);
    UNUSED(arg);
    UNUSED(value);

    switch (control & WD_ADC_ADS127L01_CONTROL_MSK) {
    case WD_ADC_ADS127L01_RESET: {
        wd_gpio_write(RTE_PORT_ADC_START, WD_GPIO_OFF);

        NVIC_DisableIRQ(EXTI0_IRQn);

#if (WD_ADC_ADS127L01_MCLK_SOURCE == WD_ADC_ADS127L01_MCLK_SOURCE_IO)
        HAL_TIM_PWM_Stop(RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE, TIM_CHANNEL_3);
#endif

        LL_SPI_Disable(SPI1);
        LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);

        LL_SPI_ClearFlag_EOT(SPI1);
        LL_SPI_ClearFlag_OVR(SPI1);
        LL_SPI_ClearFlag_TXTF(SPI1);
        LL_DMA_ClearFlag_TC6(DMA1);
        LL_DMA_ClearFlag_HT6(DMA1);
        LL_DMA_ClearFlag_TE6(DMA1);
        LL_DMA_ClearFlag_DME6(DMA1);
        LL_SPI_SetTransferSize(SPI1, (WD_ADC_DATA_SIZE == 4) ? 1 : WD_ADC_DATA_SIZE);

        LL_SPI_Disable(SPI2);
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);

        LL_SPI_ClearFlag_EOT(SPI2);
        LL_SPI_ClearFlag_OVR(SPI2);
        LL_SPI_ClearFlag_TXTF(SPI2);
        LL_DMA_ClearFlag_TC0(DMA2);
        LL_DMA_ClearFlag_HT0(DMA2);
        LL_DMA_ClearFlag_TE0(DMA2);
        LL_DMA_ClearFlag_DME0(DMA2);
        LL_SPI_SetTransferSize(SPI2, (WD_ADC_DATA_SIZE == 4) ? 1 : WD_ADC_DATA_SIZE);

        LL_SPI_ClearFlag_EOT(SPI1);
        LL_SPI_ClearFlag_EOT(SPI2);

        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);

        //        ADC_ADS127L01_Set_MCLK(LOCAL_RES->Capabilities.MCLK);

#if (WD_ADC_ADS127L01_MCLK_SOURCE == WD_ADC_ADS127L01_MCLK_SOURCE_IO)
//        HAL_TIM_PWM_Start(RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE, TIM_CHANNEL_3);
#endif

        wd_gpio_write(RTE_PORT_ADC_START, WD_GPIO_ON);

        wd_time_wait_ms(20);
    } break;

    case WD_ADC_ADS127L01_SET_SAMPLING_RATE: // arg = number of sampling point, value = frequency [hz]
    {
        uint32_t sampling_count;
        fpu_t frequency;
        fpu_t max_frequency = ADC_ADS127L01_MCLK;
        uint32_t osr = 32;

        if ((arg == 0) || (arg > WD_ADC_ADS127L01_SAMPLING_NUMBER)) {
            LOCAL_RES->Info->Sampling_Number = WD_ADC_ADS127L01_SAMPLING_NUMBER;
            arg = LOCAL_RES->Info->Sampling_Number;
        }

        LOCAL_RES->Info->Sampling_Cycle_Number = WD_ADC_ADS127L01_SAMPLING_CYCLE_NUMBER;

        frequency = (fpu_t)arg * value;

        // if(    __ASSERT(WD_RANGE(1, arg, LOCAL_RES->Info->Sampling_Number))
        //     || __ASSERT(((fpu_t)arg * value) >= (fpu_t)1.0)
        //   )
        if (!(WD_RANGE(1, arg, LOCAL_RES->Info->Sampling_Number)) ||
            !(((fpu_t)arg * value) >= (fpu_t)1.0)) {
            WD_LOG_ERROR("WD_ADC_SET_SAMPLING Out of Range !!!");
            return (WD_CODE_ERROR);
        }

        max_frequency /= (fpu_t)osr;

        sampling_count = 1;
        frequency = (fpu_t)max_frequency / value;

        do {
            sampling_count *= 2;
            if (sampling_count > arg) //4096
            {
                break;
            }
            if (sampling_count >
                ((LOCAL_RES->Info->Sampling_Cycle_Number > 1)
                     ? LOCAL_RES->Info->Sampling_Number / LOCAL_RES->Info->Sampling_Cycle_Number
                     : LOCAL_RES->Info->Sampling_Number)) // 2048
            {
                break;
            }
        } while ((fpu_t)sampling_count <= frequency); // 32
        sampling_count /= 2;

        if ((fpu_t)0.00005 >
            (fpu_t)sampling_count * ((fpu_t)1.0 / ((fpu_t)sampling_count * value))) {
            int i;
            for (i = 0; i < 10; i++) {
                WD_LOG_ERROR("WD_ADC_SET_SAMPLING Time too short !!!");
            }

            return (WD_CODE_ERROR);
        }

        frequency = (fpu_t)sampling_count * value;

        if (0) {
            if (frequency < (fpu_t)2500.0) {
                LOCAL_RES->Info->Xfer.Method = 1;
                do {
                    LOCAL_RES->Info->Xfer.Method *= 2;
                    if (LOCAL_RES->Info->Xfer.Method >= LOCAL_RES->Info->Sampling_Number) {
                        break;
                    }
                } while (frequency * (fpu_t)LOCAL_RES->Info->Xfer.Method < (fpu_t)2500.0);

                if (sampling_count != (uint32_t)(frequency / value)) {
                    WD_LOG_ERROR("Sampling Count Overflow !!! [%d] → [%d]", sampling_count,
                                 (uint32_t)(frequency / value));
                    sampling_count = (uint32_t)(frequency / value);
                }
                frequency *= (fpu_t)LOCAL_RES->Info->Xfer.Method;
            }
        }

        LOCAL_RES->Info->MCLK = ADC_ADS127L01_Set_MCLK(frequency * osr);
        LOCAL_RES->Info->Sampling_Number = arg;
        LOCAL_RES->Info->Sampling_Count = sampling_count;
        LOCAL_RES->Info->Sampling_Cycle_Number = arg / sampling_count;
        LOCAL_RES->Info->Sampling_Frequency = LOCAL_RES->Info->MCLK / osr / sampling_count;

        WD_PRINT_DEBUG(WD_MSG_DEBUG_CRLF,
                       "[ADC AC Set Sample Rate]    MCLK [%f]    ODR [%f]    Frequency [%f]    "
                       "Sample Number [%u]    Sample Count [%u]    Sample Cycle [%u]",
                       LOCAL_RES->Info->MCLK,
                       LOCAL_RES->Info->Sampling_Frequency * LOCAL_RES->Info->Sampling_Count,
                       LOCAL_RES->Info->Sampling_Frequency, LOCAL_RES->Info->Sampling_Number,
                       LOCAL_RES->Info->Sampling_Count, LOCAL_RES->Info->Sampling_Cycle_Number);
    } break;

    case WD_ADC_ADS127L01_SET_FILTER: // arg = filter type, value = filter option
        return (WD_CODE_ERROR_UNSUPPORTED);

    case WD_ADC_ADS127L01_SET_PHASE_DELAY: // arg = phase delay type, value = delay clock
        return (WD_CODE_ERROR_UNSUPPORTED);

    case WD_ADC_ADS127L01_CONVERSION_START: // arg = WD_ADC_CONVERSION_MODE_SINGLE, WD_ADC_CONVERSION_MODE_CONTINUOUS
    {
        {
            uint32_t timeout;
            uint8_t dummy;

            UNUSED(dummy);

            timeout = 1000;
            while (LL_SPI_IsActiveFlag_RXP(SPI1) && timeout > 0) {
                dummy = LL_SPI_ReceiveData8(SPI1);
                timeout--;
                if (timeout == 0) {
                    WD_LOG_ERROR();
                }
            }
            timeout = 1000;
            while (LL_SPI_IsActiveFlag_RXP(SPI2) && timeout > 0) {
                dummy = LL_SPI_ReceiveData8(SPI2);
                timeout--;
                if (timeout == 0) {
                    WD_LOG_ERROR();
                }
            }
        }

        LL_SPI_Disable(SPI1);
        LL_SPI_Disable(SPI2);
        LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);

        memset(LOCAL_RES->Info->Xfer.V_Buf, 0, LOCAL_RES->Info->Xfer.Num * WD_ADC_DATA_SIZE);
        memset(LOCAL_RES->Info->Xfer.I_Buf, 0, LOCAL_RES->Info->Xfer.Num * WD_ADC_DATA_SIZE);

        SCB_CleanDCache_by_Addr((uint32_t *)LOCAL_RES->Info->Xfer.V_Buf,
                                LOCAL_RES->Info->Xfer.Num * WD_ADC_DATA_SIZE);
        SCB_CleanDCache_by_Addr((uint32_t *)LOCAL_RES->Info->Xfer.I_Buf,
                                LOCAL_RES->Info->Xfer.Num * WD_ADC_DATA_SIZE);

        LL_DMA_SetMode(DMA1, LL_DMA_STREAM_6,
                       (LOCAL_RES->Info->Xfer.Mode == WD_ADC_CONVERSION_MODE_SINGLE)
                           ? LL_DMA_MODE_NORMAL
                           : LL_DMA_MODE_CIRCULAR);
        LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)LOCAL_RES->Info->Xfer.V_Buf);
        LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6,
                             (WD_ADC_DATA_SIZE == 4)
                                 ? LOCAL_RES->Info->Xfer.Num
                                 : LOCAL_RES->Info->Xfer.Num * WD_ADC_DATA_SIZE);
        LL_SPI_SetTransferSize(SPI1, (WD_ADC_DATA_SIZE == 4) ? 1 : WD_ADC_DATA_SIZE);
        LL_SPI_EnableDMAReq_RX(SPI1);

        LL_DMA_SetMode(DMA2, LL_DMA_STREAM_0,
                       (LOCAL_RES->Info->Xfer.Mode == WD_ADC_CONVERSION_MODE_SINGLE)
                           ? LL_DMA_MODE_NORMAL
                           : LL_DMA_MODE_CIRCULAR);
        LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)LOCAL_RES->Info->Xfer.I_Buf);
        LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0,
                             (WD_ADC_DATA_SIZE == 4)
                                 ? LOCAL_RES->Info->Xfer.Num
                                 : LOCAL_RES->Info->Xfer.Num * WD_ADC_DATA_SIZE);
        LL_SPI_SetTransferSize(SPI2, (WD_ADC_DATA_SIZE == 4) ? 1 : WD_ADC_DATA_SIZE);
        LL_SPI_EnableDMAReq_RX(SPI2);

        LL_SPI_ClearFlag_EOT(SPI1);
        LL_SPI_ClearFlag_TXTF(SPI1);
        LL_SPI_ClearFlag_UDR(SPI1);
        LL_SPI_ClearFlag_OVR(SPI1);
        LL_SPI_ClearFlag_SUSP(SPI1);
        LL_DMA_ClearFlag_TC6(DMA1);
        LL_DMA_ClearFlag_HT6(DMA1);
        LL_DMA_ClearFlag_TE6(DMA1);
        LL_DMA_ClearFlag_DME6(DMA1);

        LL_SPI_ClearFlag_EOT(SPI2);
        LL_SPI_ClearFlag_TXTF(SPI2);
        LL_SPI_ClearFlag_UDR(SPI2);
        LL_SPI_ClearFlag_OVR(SPI2);
        LL_SPI_ClearFlag_SUSP(SPI2);
        LL_DMA_ClearFlag_TC0(DMA2);
        LL_DMA_ClearFlag_HT0(DMA2);
        LL_DMA_ClearFlag_TE0(DMA2);
        LL_DMA_ClearFlag_DME0(DMA2);

        LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
        LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);

        LL_SPI_Enable(SPI1);
        LL_SPI_Enable(SPI2);

        LL_EXTI_DisableRisingTrig_0_31(LL_EXTI_LINE_0);
        LL_EXTI_DisableFallingTrig_0_31(LL_EXTI_LINE_0);
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);

        wd_gpio_write(RTE_PORT_ADC_START, WD_GPIO_ON);

        wd_time_wait_ms(10);

        LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_0);

        NVIC_EnableIRQ(EXTI0_IRQn);
    } break;

    case WD_ADC_ADS127L01_CONVERSION_STOP: {
        wd_gpio_write(RTE_PORT_ADC_START, WD_GPIO_OFF);

        {
            uint32_t timeout;
            uint8_t dummy;

            UNUSED(dummy);

            timeout = 1000;
            while (LL_SPI_IsActiveFlag_RXP(SPI1) && timeout > 0) {
                dummy = LL_SPI_ReceiveData8(SPI1);
                timeout--;
                if (timeout == 0) {
                    WD_LOG_ERROR();
                }
            }
            timeout = 1000;
            while (LL_SPI_IsActiveFlag_RXP(SPI2) && timeout > 0) {
                dummy = LL_SPI_ReceiveData8(SPI2);
                timeout--;
                if (timeout == 0) {
                    WD_LOG_ERROR();
                }
            }
        }

        LL_SPI_SuspendMasterTransfer(SPI1);
        LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);
        LL_SPI_Disable(SPI1);
        LL_SPI_ClearFlag_EOT(SPI1);
        LL_SPI_ClearFlag_TXTF(SPI1);
        LL_SPI_ClearFlag_UDR(SPI1);
        LL_SPI_ClearFlag_OVR(SPI1);
        LL_SPI_ClearFlag_SUSP(SPI1);
        LL_DMA_ClearFlag_TC6(DMA1);
        LL_DMA_ClearFlag_HT6(DMA1);
        LL_DMA_ClearFlag_TE6(DMA1);
        LL_DMA_ClearFlag_DME6(DMA1);

        LL_SPI_SuspendMasterTransfer(SPI2);
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
        LL_SPI_Disable(SPI2);
        LL_SPI_ClearFlag_EOT(SPI2);
        LL_SPI_ClearFlag_TXTF(SPI2);
        LL_SPI_ClearFlag_UDR(SPI2);
        LL_SPI_ClearFlag_OVR(SPI2);
        LL_SPI_ClearFlag_SUSP(SPI2);
        LL_DMA_ClearFlag_TC0(DMA2);
        LL_DMA_ClearFlag_HT0(DMA2);
        LL_DMA_ClearFlag_TE0(DMA2);
        LL_DMA_ClearFlag_DME0(DMA2);

        NVIC_DisableIRQ(EXTI0_IRQn);
    } break;

    case WD_ADC_ADS127L01_CONVERSION_RESULT: {
        uint8_t *v_buf;
        uint8_t *i_buf;
        uint32_t i;
        uint32_t num = (WD_ARRAY_SIZE(LOCAL_RES->Info->V_Sample) >= LOCAL_RES->Info->Xfer.Num)
                           ? LOCAL_RES->Info->Xfer.Num
                           : WD_ARRAY_SIZE(LOCAL_RES->Info->V_Sample);

        SCB_InvalidateDCache_by_Addr((uint32_t *)LOCAL_RES->Info->Xfer.V_Buf,
                                     LOCAL_RES->Info->Xfer.Num * WD_ADC_DATA_SIZE);
        SCB_InvalidateDCache_by_Addr((uint32_t *)LOCAL_RES->Info->Xfer.I_Buf,
                                     LOCAL_RES->Info->Xfer.Num * WD_ADC_DATA_SIZE);

        v_buf = LOCAL_RES->Info->Xfer.V_Buf;
        i_buf = LOCAL_RES->Info->Xfer.I_Buf;

        for (i = 0; i < num; i++) {
#if (WD_ADC_DATA_SIZE == 4)
            LOCAL_RES->Info->V_Sample[i] = ((v_buf[3] & 0x80) ? 0xFF000000 : 0x00000000) |
                                           ((uint32_t)v_buf[3] << 16) | ((uint32_t)v_buf[2] << 8) |
                                           ((uint32_t)v_buf[1] << 0);
            LOCAL_RES->Info->I_Sample[i] = ((i_buf[3] & 0x80) ? 0xFF000000 : 0x00000000) |
                                           ((uint32_t)i_buf[3] << 16) | ((uint32_t)i_buf[2] << 8) |
                                           ((uint32_t)i_buf[1] << 0);
#else
            LOCAL_RES->Info->V_Sample[i] = ((v_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                                           ((uint32_t)v_buf[0] << 16) | ((uint32_t)v_buf[1] << 8) |
                                           ((uint32_t)v_buf[2] << 0);
            LOCAL_RES->Info->I_Sample[i] = ((i_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                                           ((uint32_t)i_buf[0] << 16) | ((uint32_t)i_buf[1] << 8) |
                                           ((uint32_t)i_buf[2] << 0);
#endif
            v_buf += WD_ADC_DATA_SIZE;
            i_buf += WD_ADC_DATA_SIZE;
        }
    } break;

    case WD_ADC_ADS127L01_CONVERSION_PRINT: {
        uint32_t i;
        uint32_t v_data, i_data;
        uint8_t crc;
        uint32_t num = (WD_ARRAY_SIZE(LOCAL_RES->Info->V_Sample) >= LOCAL_RES->Info->Xfer.Num)
                           ? LOCAL_RES->Info->Xfer.Num
                           : WD_ARRAY_SIZE(LOCAL_RES->Info->V_Sample);

        UNUSED(crc);

        if (!arg) {
            WD_PRINT(WD_MSG_TITLE_CRLF,
                     "[ADC AC Set Sample Rate]    MCLK [%f]    Frequency [%f]    Sample Number "
                     "[%u]    Sample Count [%u]    Sample Cycle [%u]",
                     LOCAL_RES->Info->MCLK, LOCAL_RES->Info->Sampling_Frequency,
                     LOCAL_RES->Info->Sampling_Number, LOCAL_RES->Info->Sampling_Count,
                     LOCAL_RES->Info->Sampling_Cycle_Number);
        }
        for (i = 0; i < num; i++) {
#if (WD_ADC_DATA_SIZE == 4)
            v_data = ((uint32_t)LOCAL_RES->Info->V_Buf[i][3] << 24) |
                     ((uint32_t)LOCAL_RES->Info->V_Buf[i][2] << 16) |
                     ((uint32_t)LOCAL_RES->Info->V_Buf[i][1] << 8) |
                     (LOCAL_RES->Info->V_Buf[i][0] << 0);
            i_data = ((uint32_t)LOCAL_RES->Info->I_Buf[i][3] << 24) |
                     ((uint32_t)LOCAL_RES->Info->I_Buf[i][2] << 16) |
                     ((uint32_t)LOCAL_RES->Info->I_Buf[i][1] << 8) |
                     (LOCAL_RES->Info->I_Buf[i][0] << 0);
#if 1
            WD_PRINT_PLAIN("%21.11f,%21.11f", (double)(LOCAL_RES->Info->V_Sample[i]),
                           (double)(LOCAL_RES->Info->I_Sample[i]));
#else
            WD_PRINT_PLAIN(
                "%-4u\t\t%21.11f\t\t%21.11f\t\t0x%.8X\t\t0x%.8X\t%s0x%.8X\t%s0x%.8X%s", i + 1,
                (double)(LOCAL_RES->Info->V_Sample[i]), (double)(LOCAL_RES->Info->I_Sample[i]),
                LOCAL_RES->Info->V_Sample[i], LOCAL_RES->Info->I_Sample[i],
                (ADC_ADS127L01_CRC4x(LOCAL_RES->Info->V_Buf[i]) == WD_CODE_OK)
                    ? "        " WD_COLOR_GREEN
                    : WD_COLOR_RED "--------",
                v_data,
                (ADC_ADS127L01_CRC4x(LOCAL_RES->Info->I_Buf[i]) == WD_CODE_OK)
                    ? "        " WD_COLOR_GREEN
                    : WD_COLOR_RED "--------",
                i_data,
                ((LOCAL_RES->Info->V_Buf[i][0] & 0x08) || (LOCAL_RES->Info->I_Buf[i][0] & 0x08))
                    ? "\t\t" WD_COLOR_YELLOW_BOLD "Out-of-Range" WD_COLOR_RESET
                    : WD_COLOR_RESET);
#endif
#else
            v_data = ((uint32_t)LOCAL_RES->Info->V_Buf[i][0] << 16) |
                     ((uint32_t)LOCAL_RES->Info->V_Buf[i][1] << 8) |
                     (LOCAL_RES->Info->V_Buf[i][2] << 0);
            i_data = ((uint32_t)LOCAL_RES->Info->I_Buf[i][0] << 16) |
                     ((uint32_t)LOCAL_RES->Info->I_Buf[i][1] << 8) |
                     (LOCAL_RES->Info->I_Buf[i][2] << 0);
            //          WD_PRINT_PLAIN("%-4u\t\t%21.11f\t\t%21.11f\t\t0x%.8X\t\t0x%.8X\t\t0x%.8X\t\t0x%.8X", i + 1,(double)(LOCAL_RES->Info->V_Sample[i]), (double)(LOCAL_RES->Info->I_Sample[i]), LOCAL_RES->Info->V_Sample[i], LOCAL_RES->Info->I_Sample[i],v_data, i_data);
            WD_PRINT_PLAIN("%21.11f,%21.11f", (double)(LOCAL_RES->Info->V_Sample[i]),
                           (double)(LOCAL_RES->Info->I_Sample[i]));
#endif
            wd_time_wait_ms(1); // 1ms 정도 대기 // console 깨짐
        }
    } break;
    }

    return (WD_CODE_OK);
}

static wd_ads127l_status_t adc_ads127l01_get_status(void) {
    wd_ads127l_status_t status = {0};


    return (status);
}

static void adc_ads127l01_validation(void) {
#if (API_VALIDATION)

#endif
}

wd_ads127l_resources_t *const wd_ad4134_res_adc_ac = &ADC_ADS127L01_Resources;

wd_adc_driver_adc_ac_t wd_drv_adc_ac = {
    .Initialize = adc_ads127l01_initialize,
    .Uninitialize = adc_ads127l01_uninitialize,
    .PowerControl = adc_ads127l01_power_control,
    .Start_Conversion = adc_ads127l01_start_conversion,
    .Poll_Conversion = adc_ads127l01_poll_conversion,
    .Stop_Conversion = adc_ads127l01_stop_conversion,
    .Control = adc_ads127l01_control,
    .Get_Status = adc_ads127l01_get_status,
    .Validation = adc_ads127l01_validation,
};

#ifndef NDEBUG
__USED wd_ads127l_debug_adc_ads127l01_t DBG_ADC_ADS127L01 = {
    .Resource = &ADC_ADS127L01_Resources,
    .Info = ADC_ADS127L01_Resources.Info,
    .Driver = LOCAL_DRV,
};
#endif

#endif // #if RTE_COMPONENT_ADC_ADS127
#endif //	#if 1 // 토글
