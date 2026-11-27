/* SPDX-License-Identifier: MIT AND BSD-3-Clause
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright (c) 2019 STMicroelectronics (IS42S32800G component, BSD-3-Clause)
 */
/*
 * _wd_sdram_is42s.c
 *
 *  Created on: Jan 23, 2025
 *      Author: doubled
 */


#include "_wd_sdram_is42s.h"

#if 1 // 토글

#define REFRESH_COUNT       ((uint32_t)0x0603) /* SDRAM refresh counter (100Mhz SD clock) */
#define IS42S32800G_TIMEOUT ((uint32_t)0xFFFF)

int32_t wd_sdram_is42s_init_sdram(void);
void wd_sdram_is42s_check_sdram(uint32_t address, uint8_t *dst, uint8_t *src, uint32_t num);

/***************************************************************************************************************************************************************
 **********  SDRAM Driver
 **************************************************************************************************************************************************************/
#if (RTE_COMPONENT_SDRAM_IS42S32800G)
static wd_sdram_is42s_info_t SDRAM_IS42S32800G_Info = {
    0,
};

static wd_sdram_is42s_resources_t SDRAM_IS42S32800G_Resources = {
    .Capabilities =
        {
            .Address = WD_SDRAM_ADDRESS_BASE,
            .Size = WD_SDRAM_ADDRESS_SIZE,
        },
    .Info = &SDRAM_IS42S32800G_Info,
    .Handle = RTE_COMPONENT_SDRAM_DRIVER_HANDLE,
    .Reg = FMC_SDRAM_DEVICE,
};


#undef LOCAL_DRV
#define LOCAL_DRV (&wd_drv_sdram)
__USED static wd_sdram_is42s_resources_t *const LOCAL_RES = &SDRAM_IS42S32800G_Resources;

static int32_t sdram_is42s32800g_initialize(void) {
    if (LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "SDRAM_IS42S32800G Constructor");

    memset(LOCAL_RES->Info, 0, sizeof(wd_sdram_is42s_info_t));

    /*activate CSI clock mondatory for I/O Compensation Cell*/
    __HAL_RCC_CSI_ENABLE();

    /* Enable SYSCFG clock mondatory for I/O Compensation Cell */
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* Enables the I/O Compensation Cell */
    HAL_EnableCompensationCell();

    wd_sdram_is42s_init_sdram();

    wd_core_status_hook(WD_STATUS_SOURCE_SDRAM, WD_STATUS_OK);

    wd_sdram_is42s_check_sdram(LOCAL_RES->Capabilities.Address, wd_core_scratch_buffer(),
                               wd_core_scratch_buffer() + wd_core_scratch_size() / 2,
                               wd_core_scratch_size() / 2);
    wd_sdram_is42s_check_sdram(LOCAL_RES->Capabilities.Address + LOCAL_RES->Capabilities.Size -
                                   wd_core_scratch_size() / 2 - 1,
                               wd_core_scratch_buffer(),
                               wd_core_scratch_buffer() + wd_core_scratch_size() / 2,
                               wd_core_scratch_size() / 2);

    LOCAL_RES->Info->Flags = WDIF_FLAG_INITIALIZED;

    return (WD_CODE_OK);
}

static int32_t sdram_is42s32800g_uninitialize(void) {
    return (WD_CODE_OK);
}

static void sdram_is42s32800g_validation(void) {
#if (API_VALIDATION)

#endif
}

wd_sdram_is42s_info_t *const wd_sdram_is42s_res_sdram = &SDRAM_IS42S32800G_Info;

wd_sdram_driver_sdram_t wd_drv_sdram = {
    .Initialize = sdram_is42s32800g_initialize,
    .Uninitialize = sdram_is42s32800g_uninitialize,
    .Validation = sdram_is42s32800g_validation,
};

#ifndef NDEBUG
__USED wd_sdram_is42s_debug_sdram_is42s32800g_t wd_sdram_is42s_dbg_sdram = {
    .Resource = LOCAL_RES,
    .Info = wd_sdram_is42s_res_sdram,
    .Driver = LOCAL_DRV,
};
#endif


/***************************************************************************************************************************************************************
 **********  IS42S32800G
 **************************************************************************************************************************************************************/
/* Common Error codes */
#define BSP_ERROR_NONE                  0
#define BSP_ERROR_NO_INIT               -1
#define BSP_ERROR_WRONG_PARAM           -2
#define BSP_ERROR_BUSY                  -3
#define BSP_ERROR_PERIPH_FAILURE        -4
#define BSP_ERROR_COMPONENT_FAILURE     -5
#define BSP_ERROR_UNKNOWN_FAILURE       -6
#define BSP_ERROR_UNKNOWN_COMPONENT     -7
#define BSP_ERROR_BUS_FAILURE           -8
#define BSP_ERROR_CLOCK_FAILURE         -9
#define BSP_ERROR_MSP_FAILURE           -10
#define BSP_ERROR_FEATURE_NOT_SUPPORTED -11

/* BSP OSPI error codes */
#define BSP_ERROR_QSPI_ASSIGN_FAILURE     -24
#define BSP_ERROR_QSPI_SETUP_FAILURE      -25
#define BSP_ERROR_QSPI_MMP_LOCK_FAILURE   -26
#define BSP_ERROR_QSPI_MMP_UNLOCK_FAILURE -27

/* BSP TS error code */
#define BSP_ERROR_TS_TOUCH_NOT_DETECTED -30

/* BSP BUS error codes */
#define BSP_ERROR_BUS_TRANSACTION_FAILURE -100
#define BSP_ERROR_BUS_ARBITRATION_LOSS    -101
#define BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE -102
#define BSP_ERROR_BUS_PROTOCOL_FAILURE    -103

#define BSP_ERROR_BUS_MODE_FAULT  -104
#define BSP_ERROR_BUS_FRAME_ERROR -105
#define BSP_ERROR_BUS_CRC_ERROR   -106
#define BSP_ERROR_BUS_DMA_FAILURE -107

/** @defgroup IS42S32800G_Exported_Types Exported Types
  * @{
  */
typedef struct {
    uint32_t TargetBank;     /*!< Target Bank                             */
    uint32_t RefreshMode;    /*!< Refresh Mode                            */
    uint32_t RefreshRate;    /*!< Refresh Rate                            */
    uint32_t BurstLength;    /*!< Burst Length                            */
    uint32_t BurstType;      /*!< Burst Type                              */
    uint32_t CASLatency;     /*!< CAS Latency                             */
    uint32_t OperationMode;  /*!< Operation Mode                          */
    uint32_t WriteBurstMode; /*!< Write Burst Mode                        */
} IS42S32800G_Context_t;

/**
  * @}
  */

/** @defgroup IS42S32800G_Exported_Constants
  * @{
  */
#define IS42S32800G_OK    (0)
#define IS42S32800G_ERROR (-1)

/* Register Mode */
#define IS42S32800G_BURST_LENGTH_1             0x00000000U
#define IS42S32800G_BURST_LENGTH_2             0x00000001U
#define IS42S32800G_BURST_LENGTH_4             0x00000002U
#define IS42S32800G_BURST_LENGTH_8             0x00000004U
#define IS42S32800G_BURST_TYPE_SEQUENTIAL      0x00000000U
#define IS42S32800G_BURST_TYPE_INTERLEAVED     0x00000008U
#define IS42S32800G_CAS_LATENCY_2              0x00000020U
#define IS42S32800G_CAS_LATENCY_3              0x00000030U
#define IS42S32800G_OPERATING_MODE_STANDARD    0x00000000U
#define IS42S32800G_WRITEBURST_MODE_PROGRAMMED 0x00000000U
#define IS42S32800G_WRITEBURST_MODE_SINGLE     0x00000200U

/* Command Mode */
#define IS42S32800G_NORMAL_MODE_CMD      0x00000000U
#define IS42S32800G_CLK_ENABLE_CMD       0x00000001U
#define IS42S32800G_PALL_CMD             0x00000002U
#define IS42S32800G_AUTOREFRESH_MODE_CMD 0x00000003U
#define IS42S32800G_LOAD_MODE_CMD        0x00000004U
#define IS42S32800G_SELFREFRESH_MODE_CMD 0x00000005U
#define IS42S32800G_POWERDOWN_MODE_CMD   0x00000006U

/**
  * @}
  */

/** @addtogroup IS42S32800G_Private_Functions
  * @{
  */
int32_t wd_sdram_is42s_init(SDRAM_HandleTypeDef *Ctx, IS42S32800G_Context_t *pRegMode);
int32_t wd_sdram_is42s_enable_clock(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t wd_sdram_is42s_precharge(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t wd_sdram_is42s_mode_reg_config(SDRAM_HandleTypeDef *Ctx, IS42S32800G_Context_t *pRegMode);
int32_t wd_sdram_is42s_timing_config(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_TimingTypeDef *pTiming);
int32_t wd_sdram_is42s_refresh_mode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface,
                                    uint32_t RefreshMode);
int32_t wd_sdram_is42s_refresh_rate(SDRAM_HandleTypeDef *Ctx, uint32_t RefreshCount);
int32_t wd_sdram_is42s_enter_power_mode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t wd_sdram_is42s_exit_power_mode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t wd_sdram_is42s_sendcmd(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_CommandTypeDef *SdramCmd);

/**
  * @}
  */

/** @defgroup IS42S32800G IS42S32800G
  * @brief     This file provides a set of functions needed to drive the
  *            IS42S32800G SDRAM memory.
  * @{
  */

/** @defgroup IS42S32800G_Private_Variables IS42S32800G Private Variables
  * @{
  */
static FMC_SDRAM_CommandTypeDef Command;
/**
  * @}
  */

/** @defgroup IS42S32800G_Function_Prototypes IS42S32800G Function Prototypes
  * @{
  */

/**
  * @}
  */

/** @defgroup IS42S32800G_Exported_Functions IS42S32800G Exported Functions
  * @{
  */
/**
  * @brief  Initializes the IS42S32800G SDRAm memory
  * @param  Ctx Component object pointer
  * @param  pRegMode : Pointer to Register Mode structure
  * @retval error status
  */
int32_t wd_sdram_is42s_init(SDRAM_HandleTypeDef *Ctx, IS42S32800G_Context_t *pRegMode) {
    int32_t ret = IS42S32800G_ERROR;

    /* Step 1: Configure a clock configuration enable command */
    if (wd_sdram_is42s_enable_clock(Ctx, pRegMode->TargetBank) == IS42S32800G_OK) {
        /* Step 2: Insert 100 us minimum delay */
        /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
        //    (void)IS42S32800G_Delay(1);
        wd_time_wait_ms(1);

        /* Step 3: Configure a PALL (precharge all) command */
        if (wd_sdram_is42s_precharge(Ctx, pRegMode->TargetBank) == IS42S32800G_OK) {
            /* Step 4: Configure a Refresh command */
            if (wd_sdram_is42s_refresh_mode(Ctx, pRegMode->TargetBank, pRegMode->RefreshMode) ==
                IS42S32800G_OK) {
                /* Step 5: Program the external memory mode register */
                if (wd_sdram_is42s_mode_reg_config(Ctx, pRegMode) == IS42S32800G_OK) {
                    /* Step 6: Set the refresh rate counter */
                    if (wd_sdram_is42s_refresh_rate(Ctx, pRegMode->RefreshRate) == IS42S32800G_OK) {
                        ret = IS42S32800G_OK;
                    }
                }
            }
        }
    }
    return ret;
}

/**
  * @brief  Enable SDRAM clock
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t wd_sdram_is42s_enable_clock(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) {
    Command.CommandMode = IS42S32800G_CLK_ENABLE_CMD;
    Command.CommandTarget = Interface;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800G_TIMEOUT) != HAL_OK) {
        return IS42S32800G_ERROR;
    } else {
        return IS42S32800G_OK;
    }
}

/**
  * @brief  Precharge all sdram banks
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t wd_sdram_is42s_precharge(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) {
    Command.CommandMode = IS42S32800G_PALL_CMD;
    Command.CommandTarget = Interface;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800G_TIMEOUT) != HAL_OK) {
        return IS42S32800G_ERROR;
    } else {
        return IS42S32800G_OK;
    }
}

/**
  * @brief  Program the external memory mode register
  * @param  Ctx Component object pointer
  * @param  pRegMode : Pointer to Register Mode structure
  * @retval error status
  */
int32_t wd_sdram_is42s_mode_reg_config(SDRAM_HandleTypeDef *Ctx, IS42S32800G_Context_t *pRegMode) {
    uint32_t tmpmrd;

    /* Program the external memory mode register */
    tmpmrd = (uint32_t)pRegMode->BurstLength | pRegMode->BurstType | pRegMode->CASLatency |
             pRegMode->OperationMode | pRegMode->WriteBurstMode;

    Command.CommandMode = IS42S32800G_LOAD_MODE_CMD;
    Command.CommandTarget = pRegMode->TargetBank;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800G_TIMEOUT) != HAL_OK) {
        return IS42S32800G_ERROR;
    } else {
        return IS42S32800G_OK;
    }
}

/**
  * @brief  Program the SDRAM timing
  * @param  Ctx Component object pointer
  * @param  pTiming Pointer to SDRAM timing configuration structure
  * @retval error status
  */
int32_t wd_sdram_is42s_timing_config(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_TimingTypeDef *pTiming) {
    /* Program the SDRAM timing */
    if (HAL_SDRAM_Init(Ctx, pTiming) != HAL_OK) {
        return IS42S32800G_ERROR;
    } else {
        return IS42S32800G_OK;
    }
}

/**
  * @brief  Configure Refresh mode
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @param  RefreshMode Could be IS42S32800G_CMD_AUTOREFRESH_MODE or
  *                      IS42S32800G_CMD_SELFREFRESH_MODE
  * @retval error status
  */
int32_t wd_sdram_is42s_refresh_mode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface,
                                    uint32_t RefreshMode) {
    Command.CommandMode = RefreshMode;
    Command.CommandTarget = Interface;
    Command.AutoRefreshNumber = 8;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800G_TIMEOUT) != HAL_OK) {
        return IS42S32800G_ERROR;
    } else {
        return IS42S32800G_OK;
    }
}

/**
  * @brief  Set the device refresh rate
  * @param  Ctx Component object pointer
  * @param  RefreshCount The refresh rate to be programmed
  * @retval error status
  */
int32_t wd_sdram_is42s_refresh_rate(SDRAM_HandleTypeDef *Ctx, uint32_t RefreshCount) {
    /* Set the device refresh rate */
    if (HAL_SDRAM_ProgramRefreshRate(Ctx, RefreshCount) != HAL_OK) {
        return IS42S32800G_ERROR;
    } else {
        return IS42S32800G_OK;
    }
}

/**
  * @brief  Enter Power mode
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t wd_sdram_is42s_enter_power_mode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) {
    Command.CommandMode = IS42S32800G_POWERDOWN_MODE_CMD;
    Command.CommandTarget = Interface;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800G_TIMEOUT) != HAL_OK) {
        return IS42S32800G_ERROR;
    } else {
        return IS42S32800G_OK;
    }
}

/**
  * @brief  Exit Power mode
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
int32_t wd_sdram_is42s_exit_power_mode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) {
    Command.CommandMode = IS42S32800G_NORMAL_MODE_CMD;
    Command.CommandTarget = Interface;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800G_TIMEOUT) != HAL_OK) {
        return IS42S32800G_ERROR;
    } else {
        return IS42S32800G_OK;
    }
}

/**
  * @brief  Sends command to the SDRAM bank.
  * @param  Ctx Component object pointer
  * @param  SdramCmd : Pointer to SDRAM command structure
  * @retval SDRAM status
  */
int32_t wd_sdram_is42s_sendcmd(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_CommandTypeDef *SdramCmd) {
    if (HAL_SDRAM_SendCommand(Ctx, SdramCmd, IS42S32800G_TIMEOUT) != HAL_OK) {
        return IS42S32800G_ERROR;
    } else {
        return IS42S32800G_OK;
    }
}

/**
  * @}
  */

/** @defgroup IS42S32800G_Private_Functions IS42S32800G Private Functions
  * @{
  */

/**
  * @brief This function provides accurate delay (in milliseconds)
  * @param Delay : specifies the delay time length, in milliseconds
  * @retval IS42S32800G_OK
  */

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup STM32H743_Private_Functions_Prototypes Private Functions Prototypes
  * @{
  */
static void SDRAM_MspInit(SDRAM_HandleTypeDef *hSdram);
HAL_StatusTypeDef MX_SDRAM_BANK2_Init(SDRAM_HandleTypeDef *hSdram);

/**
  * @}
  */

/** @defgroup STM32H743_Exported_Functions Exported Functions
  * @{
  */
/**
  * @brief  Initializes the SDRAM device.
  * @retval BSP status
  */
int32_t wd_sdram_is42s_init_sdram(void) {
    int32_t ret;
    static IS42S32800G_Context_t pRegMode;

    ret = BSP_ERROR_NONE;
    SDRAM_MspInit(RTE_COMPONENT_SDRAM_DRIVER_HANDLE);

    if (ret == BSP_ERROR_NONE) {
        if (MX_SDRAM_BANK2_Init(RTE_COMPONENT_SDRAM_DRIVER_HANDLE) != HAL_OK) {
            ret = BSP_ERROR_NO_INIT;
        } else {
            /* External memory mode register configuration */
            pRegMode.TargetBank = FMC_SDRAM_CMD_TARGET_BANK2;
            pRegMode.RefreshMode = IS42S32800G_AUTOREFRESH_MODE_CMD;
            pRegMode.RefreshRate = REFRESH_COUNT;
            pRegMode.BurstLength = IS42S32800G_BURST_LENGTH_1;
            pRegMode.BurstType = IS42S32800G_BURST_TYPE_SEQUENTIAL;
            pRegMode.CASLatency = IS42S32800G_CAS_LATENCY_3;
            pRegMode.OperationMode = IS42S32800G_OPERATING_MODE_STANDARD;
            pRegMode.WriteBurstMode = IS42S32800G_WRITEBURST_MODE_SINGLE;

            /* SDRAM initialization sequence */
            if (wd_sdram_is42s_init(RTE_COMPONENT_SDRAM_DRIVER_HANDLE, &pRegMode) !=
                IS42S32800G_OK) {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
        }
    }

    return ret;
}

/**
  * @brief  DeInitializes the SDRAM device.
  * @retval BSP status
  */

void wd_sdram_is42s_check_sdram(uint32_t address, uint8_t *dst, uint8_t *src, uint32_t num) {
    uint32_t i;

    uint32_t *write_buf = (uint32_t *)src;
    uint32_t *read_buf = (uint32_t *)dst;

    for (i = 0; i < num / 4; i++) {
        write_buf[i] = 0xA244250F + DWT->CYCCNT + i;
        read_buf[i] = 0xBBBBBBBB + i;
    }

    for (i = 0; i < num / 4; i++) {
        *(__IO uint32_t *)(address + (4 * i)) = write_buf[i];
    }

    for (i = 0; i < num / 4; i++) {
        read_buf[i] = *(__IO uint32_t *)(address + (4 * i));
    }

    if (memcmp(dst, src, num)) {
        wd_core_status_hook(WD_STATUS_SOURCE_SDRAM, WD_STATUS_ERROR);
        WD_LOG_ERROR("SDRAM Error [IS42S32800G] !!!");
    }
}

/**
  * @brief  Initializes the SDRAM periperal.
  * @param  hSdram SDRAM handle
  * @retval HAL status
  */
HAL_StatusTypeDef MX_SDRAM_BANK2_Init(SDRAM_HandleTypeDef *hSdram) {
    FMC_SDRAM_TimingTypeDef sdram_timing;

    /* SDRAM device configuration */
    hSdram->Instance = FMC_SDRAM_DEVICE;

    /* SDRAM handle configuration */
    hSdram->Init.SDBank = FMC_SDRAM_BANK2;
    hSdram->Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
    hSdram->Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
    hSdram->Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
    hSdram->Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hSdram->Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
    hSdram->Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hSdram->Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    hSdram->Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    hSdram->Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;

    /* Timing configuration for as SDRAM */
    sdram_timing.LoadToActiveDelay = 2;
    sdram_timing.ExitSelfRefreshDelay = 7;
    sdram_timing.SelfRefreshTime = 4;
    sdram_timing.RowCycleDelay = 7;
    sdram_timing.WriteRecoveryTime = 2;
    sdram_timing.RPDelay = 2;
    sdram_timing.RCDDelay = 2;

    /* SDRAM controller initialization */
    if (HAL_SDRAM_Init(hSdram, &sdram_timing) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
  * @brief  Sends command to the SDRAM bank.
  * @param  SdramCmd  Pointer to SDRAM command structure
  * @retval BSP status
  */

/**
  * @brief  This function handles SDRAM MDMA interrupt request.
  * @retval None
  */
void SDRAM_IRQHandler(void) {
    HAL_MDMA_IRQHandler((RTE_COMPONENT_SDRAM_DRIVER_HANDLE)->hmdma);
}

/**
  * @}
  */

/** @defgroup STM32H743_Private_Functions Private Functions
  * @{
  */

/**
  * @brief  Initializes SDRAM MSP.
  * @param  hSdram SDRAM handle
  * @retval None
  */
static void SDRAM_MspInit(SDRAM_HandleTypeDef *hSdram) {
    static MDMA_HandleTypeDef mdma_handle;
    GPIO_InitTypeDef gpio_init_structure;

    /* Enable FMC clock */
    __HAL_RCC_FMC_CLK_ENABLE();

    /* Enable chosen MDMAx clock */
    __HAL_RCC_MDMA_CLK_ENABLE();

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    /* Common GPIO configuration */
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Alternate = GPIO_AF12_FMC;

    /* GPIOD configuration */
    gpio_init_structure.Pin =
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;

    HAL_GPIO_Init(GPIOD, &gpio_init_structure);

    /* GPIOE configuration */
    gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                              GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio_init_structure);

    /* GPIOF configuration */
    gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
                              GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                              GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &gpio_init_structure);

    /* GPIOG configuration */
    gpio_init_structure.Pin =
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &gpio_init_structure);

    /* GPIOH configuration */
    gpio_init_structure.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                              GPIO_PIN_15;
    HAL_GPIO_Init(GPIOH, &gpio_init_structure);

    /* GPIOI configuration */
    gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
                              GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOI, &gpio_init_structure);

    /* Configure common MDMA parameters */
    mdma_handle.Init.Request = MDMA_REQUEST_SW;
    mdma_handle.Init.TransferTriggerMode = MDMA_BLOCK_TRANSFER;
    mdma_handle.Init.Priority = MDMA_PRIORITY_HIGH;
    mdma_handle.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    mdma_handle.Init.SourceInc = MDMA_SRC_INC_WORD;
    mdma_handle.Init.DestinationInc = MDMA_DEST_INC_WORD;
    mdma_handle.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
    mdma_handle.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
    mdma_handle.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    mdma_handle.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    mdma_handle.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    mdma_handle.Init.BufferTransferLength = 128;
    mdma_handle.Init.SourceBlockAddressOffset = 0;
    mdma_handle.Init.DestBlockAddressOffset = 0;
    mdma_handle.Instance = MDMA_Channel0;

    /* Associate the MDMA handle */
    __HAL_LINKDMA(hSdram, hmdma, mdma_handle);

    /* De-initialize the Stream for new transfer */
    HAL_MDMA_DeInit(&mdma_handle);

    /* Configure the MDMA stream */
    (void)HAL_MDMA_Init(&mdma_handle);

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(MDMA_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
}

/**
  * @brief  DeInitializes SDRAM MSP.
  * @param  hSdram SDRAM handle
  * @retval None
  */

#endif //	#if (RTE_COMPONENT_SDRAM_IS42S32800G)

#endif //	#if 1 // 토글
