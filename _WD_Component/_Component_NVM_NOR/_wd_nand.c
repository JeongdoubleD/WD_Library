/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_nand.c
 *
 *  Winbond W25N04KVZEIR 4Gb SPI NAND Flash driver
 *  Interface: wd_blockdev_instance_t (same as NOR Flash driver)
 *  Address mapping: 4 KB logical sector → 1 NAND block (128 KB physical)
 *
 *  Created on: 2026-06-17
 *      Author: doubled
 */

#include "_wd_nand.h"

#if (RTE_COMPONENT_BLOCK_DEVICE_NAND_FLASH_W25N04KBZEIR)

/***************************************************************************************************************************************************************
 **********  W25N04KVZEIR Commands
 **************************************************************************************************************************************************************/
#define W25N_CMD_RESET         0xFF
#define W25N_CMD_JEDEC_ID      0x9F
#define W25N_CMD_READ_SR       0x0F
#define W25N_CMD_WRITE_SR      0x1F
#define W25N_CMD_WRITE_ENABLE  0x06
#define W25N_CMD_WRITE_DISABLE 0x04
#define W25N_CMD_PAGE_READ     0x13
#define W25N_CMD_READ_CACHE    0x03
#define W25N_CMD_LOAD_PROG     0x02
#define W25N_CMD_PROG_EXEC     0x10
#define W25N_CMD_BLOCK_ERASE   0xD8

/* Status Register addresses */
#define W25N_SR1_PROT 0xA0 /* Protection Register */
#define W25N_SR2_CFG  0xB0 /* Configuration Register */
#define W25N_SR3_STAT 0xC0 /* Status Register */

/* SR3 bits */
#define W25N_SR3_BUSY  (1u << 0)
#define W25N_SR3_WEL   (1u << 1)
#define W25N_SR3_EFAIL (1u << 2)
#define W25N_SR3_PFAIL (1u << 3)
#define W25N_SR3_ECCS0 (1u << 4)
#define W25N_SR3_ECCS1 (1u << 5)

/* SR1 default (all blocks protected) → unlock = 0x00 */
#define W25N_SR1_UNLOCK_ALL 0x00

/* SR2: ECC-E=1 (bit4), BUF=1 (bit3) → 0x18 (default, usually already set) */
#define W25N_SR2_DEFAULT 0x18

/***************************************************************************************************************************************************************
 **********  Driver Resources
 **************************************************************************************************************************************************************/

static wd_blockdev_instance_info_t Block_Device_NAND_Flash_Info = {0};

#if (RTE_RTOS)
static osStaticMutexDef_t Mutex_Block_Device_NAND_Flash_cb_mem;
static osMutexId_t Mutex_Block_Device_NAND_Flash;
static const osMutexAttr_t Mutex_Block_Device_NAND_Flash_Attr = {
    .name = "Mutex Block_Device_NAND_Flash",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = &Mutex_Block_Device_NAND_Flash_cb_mem,
    .cb_size = sizeof(Mutex_Block_Device_NAND_Flash_cb_mem)};

static int32_t res_start(void) {
    Mutex_Block_Device_NAND_Flash = osMutexNew(&Mutex_Block_Device_NAND_Flash_Attr);
    if (Mutex_Block_Device_NAND_Flash == NULL) {
        WD_LOG_ERROR("Mutex Block_Device_NAND_Flash New Error");
        return (WD_CODE_ERROR);
    }
    return (WD_CODE_OK);
}
static int32_t res_lock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        if (osMutexAcquire(Mutex_Block_Device_NAND_Flash, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
            return (WD_CODE_ERROR_TIMEOUT);
        }
    }
    return (WD_CODE_OK);
}
static int32_t res_unlock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(Mutex_Block_Device_NAND_Flash);
    }
    return (WD_CODE_OK);
}
#else
static int32_t res_start(void) {
    return WD_CODE_OK;
}
static int32_t res_lock(void) {
    return WD_CODE_OK;
}
static int32_t res_unlock(void) {
    return WD_CODE_OK;
}
#endif

static uint8_t Block_Device_NAND_Flash_Buf[W25N_PAGE_SIZE] __ALIGNED(WD_CONF_DMA_ALIGNMENT_SIZE);
static uint8_t
    Block_Device_NAND_Flash_Confirm_Buf[W25N_PAGE_SIZE] __ALIGNED(WD_CONF_DMA_ALIGNMENT_SIZE);

static wd_blockdev_instance_resources_t Block_Device_NAND_Flash_Resources = {
    .Capabilities =
        {
            .Name = "[NVM NAND]",
            .Addr = RTE_BLOCK_DEVICE_NAND_FLASH_ADDR,
            .Size = RTE_BLOCK_DEVICE_NAND_FLASH_SIZE,
            .Program_Size = RTE_BLOCK_DEVICE_NAND_FLASH_PROGRAM_SIZE,
            .Erase_Sector_Size = RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_SECTOR_SIZE,
            .Erase_Block_Size = RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_BLOCK_SIZE,
            .Erase_Block_Addr = RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_BLOCK_ADDR,
            .Program_TimeOut = RTE_BLOCK_DEVICE_NAND_FLASH_PROGRAM_TIMEOUT,
            .Erase_Sector_TimeOut = RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_SECTOR_TIMEOUT,
            .Erase_Block_TimeOut = RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_BLOCK_TIMEOUT,
            .Erase_Chip_TimeOut = RTE_BLOCK_DEVICE_NAND_FLASH_ERASE_CHIP_TIMEOUT,
            .Buf = Block_Device_NAND_Flash_Buf,
            .Buf_Size = sizeof(Block_Device_NAND_Flash_Buf),
            .Confirm_Buf = Block_Device_NAND_Flash_Confirm_Buf,
            .Confirm_Buf_Size = sizeof(Block_Device_NAND_Flash_Confirm_Buf),
        },
    .Info = &Block_Device_NAND_Flash_Info,
    .IO = {.WP = RTE_CONFIG_IDLE_PIN_NC, .RESET = RTE_CONFIG_IDLE_PIN_NC},
    .Drv = NULL,
    .Handle = NULL,
    .Reg = NULL,
    .Start = res_start,
    .Lock = res_lock,
    .Unlock = res_unlock,
};

#undef LOCAL_DRV
#define LOCAL_DRV (&wd_drv_block_device_nand_flash)
__USED static wd_blockdev_instance_resources_t *const LOCAL_RES =
    &Block_Device_NAND_Flash_Resources;

/***************************************************************************************************************************************************************
 **********  Low-Level QSPI Primitives
 **************************************************************************************************************************************************************/

static void NAND_CMD_Fill(QSPI_CommandTypeDef *s) {
    s->AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s->AlternateBytes = 0;
    s->AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    s->DdrMode = QSPI_DDR_MODE_DISABLE;
    s->DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s->SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
}

/* Send command-only (no address, no data) */
static int32_t NAND_Send_Cmd(uint8_t cmd) {
    QSPI_CommandTypeDef s = {0};
    NAND_CMD_Fill(&s);
    s.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s.Instruction = cmd;
    s.AddressMode = QSPI_ADDRESS_NONE;
    s.DataMode = QSPI_DATA_NONE;
    s.DummyCycles = 0;
    return (HAL_QSPI_Command(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &s,
                             HAL_QSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK)
               ? WD_CODE_OK
               : WD_CODE_ERROR;
}

/* Read 1-byte status register (reg: W25N_SR1_PROT / W25N_SR2_CFG / W25N_SR3_STAT) */
static uint8_t NAND_Read_SR(uint8_t reg) {
    QSPI_CommandTypeDef s = {0};
    uint8_t val = 0xFF;
    NAND_CMD_Fill(&s);
    s.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s.Instruction = W25N_CMD_READ_SR;
    s.AddressMode = QSPI_ADDRESS_1_LINE;
    s.AddressSize = QSPI_ADDRESS_8_BITS;
    s.Address = reg;
    s.DataMode = QSPI_DATA_1_LINE;
    s.NbData = 1;
    s.DummyCycles = 0;
    if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &s,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return 0xFF;
    }
    if (HAL_QSPI_Receive(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &val,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return 0xFF;
    }
    return val;
}

/* Write 1-byte status register */
static int32_t NAND_Write_SR(uint8_t reg, uint8_t val) {
    QSPI_CommandTypeDef s = {0};
    NAND_CMD_Fill(&s);
    s.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s.Instruction = W25N_CMD_WRITE_SR;
    s.AddressMode = QSPI_ADDRESS_1_LINE;
    s.AddressSize = QSPI_ADDRESS_8_BITS;
    s.Address = reg;
    s.DataMode = QSPI_DATA_1_LINE;
    s.NbData = 1;
    s.DummyCycles = 0;
    if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &s,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    if (HAL_QSPI_Transmit(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &val,
                          HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    return WD_CODE_OK;
}

/* Poll SR3 until BUSY=0 or timeout */
static int32_t NAND_Wait_Not_Busy(uint32_t timeout_us) {
    uint32_t elapsed = 0;
    while (1) {
        uint8_t sr3 = NAND_Read_SR(W25N_SR3_STAT);
        if (!(sr3 & W25N_SR3_BUSY)) {
            return WD_CODE_OK;
        }
        if (elapsed >= timeout_us) {
            return WD_CODE_ERROR;
        }
        wd_time_wait_us(10);
        elapsed += 10;
    }
}

/* Send Write Enable and verify WEL=1 */
static int32_t NAND_Write_Enable(void) {
    if (NAND_Send_Cmd(W25N_CMD_WRITE_ENABLE) != WD_CODE_OK) {
        return WD_CODE_ERROR;
    }
    return (NAND_Read_SR(W25N_SR3_STAT) & W25N_SR3_WEL) ? WD_CODE_OK : WD_CODE_ERROR;
}

/* Read JEDEC ID: 0x9F + 8 dummy cycles → 3 bytes */
static int32_t NAND_Read_ID(uint8_t id[3]) {
    QSPI_CommandTypeDef s = {0};
    NAND_CMD_Fill(&s);
    s.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s.Instruction = W25N_CMD_JEDEC_ID;
    s.AddressMode = QSPI_ADDRESS_NONE;
    s.DataMode = QSPI_DATA_1_LINE;
    s.NbData = 3;
    s.DummyCycles = 8; /* 1 dummy byte before ID bytes */
    if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &s,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    if (HAL_QSPI_Receive(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, id,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    return WD_CODE_OK;
}

/* 0x13: Load NAND page into internal cache. page_addr = block*64 + page_in_block */
static int32_t NAND_Page_Read(uint32_t page_addr) {
    QSPI_CommandTypeDef s = {0};
    NAND_CMD_Fill(&s);
    s.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s.Instruction = W25N_CMD_PAGE_READ;
    s.AddressMode = QSPI_ADDRESS_1_LINE;
    s.AddressSize = QSPI_ADDRESS_24_BITS;
    s.Address = page_addr & 0x0000FFFFu; /* bits[23:16]=0 dummy, bits[15:0]=page addr */
    s.DataMode = QSPI_DATA_NONE;
    s.DummyCycles = 0;
    if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &s,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    return NAND_Wait_Not_Busy(LOCAL_RES->Capabilities.Erase_Sector_TimeOut);
}

/* 0x03: Read num bytes from cache starting at column */
static int32_t NAND_Read_Cache(uint16_t col, void *buf, uint32_t num) {
    QSPI_CommandTypeDef s = {0};
    NAND_CMD_Fill(&s);
    s.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s.Instruction = W25N_CMD_READ_CACHE;
    s.AddressMode = QSPI_ADDRESS_1_LINE;
    s.AddressSize = QSPI_ADDRESS_16_BITS;
    s.Address = col;
    s.DataMode = QSPI_DATA_1_LINE;
    s.NbData = num;
    s.DummyCycles = 8; /* 1 dummy byte */
    if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &s,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    if (HAL_QSPI_Receive(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, (uint8_t *)buf,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    return WD_CODE_OK;
}

/* 0x02: Load num bytes into cache starting at column */
static int32_t NAND_Load_Cache(uint16_t col, const void *data, uint32_t num) {
    QSPI_CommandTypeDef s = {0};
    NAND_CMD_Fill(&s);
    s.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s.Instruction = W25N_CMD_LOAD_PROG;
    s.AddressMode = QSPI_ADDRESS_1_LINE;
    s.AddressSize = QSPI_ADDRESS_16_BITS;
    s.Address = col;
    s.DataMode = QSPI_DATA_1_LINE;
    s.NbData = num;
    s.DummyCycles = 0;
    if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &s,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    if (HAL_QSPI_Transmit(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, (uint8_t *)data,
                          HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    return WD_CODE_OK;
}

/* 0x10: Program cache to NAND array at page_addr */
static int32_t NAND_Program_Execute(uint32_t page_addr) {
    QSPI_CommandTypeDef s = {0};
    NAND_CMD_Fill(&s);
    s.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s.Instruction = W25N_CMD_PROG_EXEC;
    s.AddressMode = QSPI_ADDRESS_1_LINE;
    s.AddressSize = QSPI_ADDRESS_24_BITS;
    s.Address = page_addr & 0x0000FFFFu;
    s.DataMode = QSPI_DATA_NONE;
    s.DummyCycles = 0;
    if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &s,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    if (NAND_Wait_Not_Busy(LOCAL_RES->Capabilities.Program_TimeOut) != WD_CODE_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }
    if (NAND_Read_SR(W25N_SR3_STAT) & W25N_SR3_PFAIL) {
        WD_LOG_ERROR("NAND P-FAIL");
        return WD_CODE_ERROR;
    }
    return WD_CODE_OK;
}

/* 0xD8: Block erase — erases the entire 128 KB block containing page_addr */
static int32_t NAND_Block_Erase(uint32_t block_num) {
    QSPI_CommandTypeDef s = {0};
    uint32_t page_addr = block_num * W25N_PAGES_PER_BLOCK;

    if (NAND_Write_Enable() != WD_CODE_OK) {
        WD_LOG_ERROR("NAND Erase WE");
        return WD_CODE_ERROR;
    }

    NAND_CMD_Fill(&s);
    s.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s.Instruction = W25N_CMD_BLOCK_ERASE;
    s.AddressMode = QSPI_ADDRESS_1_LINE;
    s.AddressSize = QSPI_ADDRESS_24_BITS;
    s.Address = page_addr & 0x0000FFFFu;
    s.DataMode = QSPI_DATA_NONE;
    s.DummyCycles = 0;
    if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE, &s,
                         HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return WD_CODE_ERROR;
    }
    if (NAND_Wait_Not_Busy(LOCAL_RES->Capabilities.Erase_Block_TimeOut) != WD_CODE_OK) {
        WD_LOG_ERROR("NAND Erase Timeout");
        return WD_CODE_ERROR;
    }
    if (NAND_Read_SR(W25N_SR3_STAT) & W25N_SR3_EFAIL) {
        WD_LOG_ERROR("NAND E-FAIL");
        return WD_CODE_ERROR;
    }
    return WD_CODE_OK;
}

/***************************************************************************************************************************************************************
 **********  wd_blockdev_instance_t Interface
 **************************************************************************************************************************************************************/

static int32_t block_device_nand_flash_initialize(wd_event_callback_t cb_event) {
    if (LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Block_Device_NAND_Flash Constructor");

    memset(LOCAL_RES->Info, 0, sizeof(wd_blockdev_instance_info_t));

    if (cb_event) {
        LOCAL_RES->Info->CB_Event = cb_event;
    }

    LOCAL_RES->Start();

    LOCAL_RES->Info->Flags = WDIF_FLAG_INITIALIZED;

    return (WD_CODE_OK);
}

static int32_t block_device_nand_flash_uninitialize(void) {
    LOCAL_RES->Info->Flags = 0;
    return (WD_CODE_OK);
}

static int32_t block_device_nand_flash_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF:
        LOCAL_RES->Info->Flags &= ~(WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
        break;

    case WD_POWER_FULL: {
        if ((LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) == 0U) {
            return (WD_CODE_ERROR);
        }
        if (LOCAL_RES->Info->Flags & WDIF_FLAG_POWERED) {
            return (WD_CODE_OK);
        }

        LOCAL_RES->Info->Xfer.Buf = LOCAL_RES->Capabilities.Buf;
        LOCAL_RES->Info->Xfer.Buf_Size = LOCAL_RES->Capabilities.Buf_Size;

        /* Re-initialize QSPI with calculated prescaler */
        {
            uint32_t hclkFreq = HAL_RCC_GetHCLKFreq();
            uint32_t prescaler = (RCC->D1CFGR & RCC_D1CFGR_HPRE) >> RCC_D1CFGR_HPRE_Pos;
            uint32_t apbclk = (prescaler >= 8) ? (hclkFreq >> (prescaler - 7)) : hclkFreq;
            uint32_t desiredFreq = 20000000u;
            uint32_t calcPrescaler = ((apbclk * 2) / desiredFreq) - 1;

            (RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE)->Instance = QUADSPI;
            HAL_QSPI_DeInit(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE);

            (RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE)->Init.ClockPrescaler = calcPrescaler;
            (RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE)->Init.FifoThreshold = 4;
            (RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE)->Init.SampleShifting =
                QSPI_SAMPLE_SHIFTING_NONE;
            (RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE)->Init.FlashSize = 27;
            (RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE)->Init.ChipSelectHighTime =
                QSPI_CS_HIGH_TIME_2_CYCLE;
            (RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE)->Init.ClockMode = QSPI_CLOCK_MODE_0;
            (RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE)->Init.FlashID = QSPI_FLASH_ID_1;
            (RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE)->Init.DualFlash = QSPI_DUALFLASH_DISABLE;

            if (HAL_QSPI_Init(RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE) != HAL_OK) {
                WD_LOG_ERROR();
                return WD_CODE_ERROR;
            }
        }

        /* Hardware reset */
        if (NAND_Send_Cmd(W25N_CMD_RESET) != WD_CODE_OK) {
            WD_LOG_ERROR("NAND Reset");
        }
        wd_time_wait_us(1000u); /* tRST max ~500 us */

        /* Verify JEDEC ID */
        {
            uint8_t id[3] = {0};
            if (NAND_Read_ID(id) != WD_CODE_OK) {
                WD_LOG_ERROR("NAND JEDEC Read Error");
                LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
            }

            WD_PRINT_INFO(WD_MSG_DEBUG_LOG,
                          "[W25N04KVZEIR JEDEC] Mfr:0x%02X  Type:0x%02X  Cap:0x%02X", id[0], id[1],
                          id[2]);

            if (id[0] != W25N_JEDEC_MFR || id[1] != W25N_JEDEC_TYPE || id[2] != W25N_JEDEC_CAP) {
                WD_LOG_ERROR("NAND JEDEC ID Mismatch");
                LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
            }
        }

        if (LOCAL_RES->Info->Status & WDIF_BLOCK_DEVICE_STATUS_ERROR) {
            wd_core_status_hook(WD_STATUS_SOURCE_NVM_FLASH, WD_STATUS_ERROR);
            return WD_CODE_ERROR;
        }

        /* Unlock all blocks (SR1 default = 0x7C = all protected) */
        NAND_Write_SR(W25N_SR1_PROT, W25N_SR1_UNLOCK_ALL);

        /* Verify SR2: ECC-E=1 (bit4), BUF=1 (bit3) → default 0x18, write to ensure */
        {
            uint8_t sr2 = NAND_Read_SR(W25N_SR2_CFG);
            if (sr2 != W25N_SR2_DEFAULT) {
                NAND_Write_SR(W25N_SR2_CFG, W25N_SR2_DEFAULT);
            }
        }

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Block_Device_NAND_Flash PowerControl:WD_POWER_FULL");

        LOCAL_RES->Info->Status = 0;
        LOCAL_RES->Info->Flags |= (WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);

        wd_core_status_hook(WD_STATUS_SOURCE_NVM_FLASH, WD_STATUS_OK);
        break;
    }

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);
    }

    return (WD_CODE_OK);
}

static int32_t block_device_nand_flash_read(uint32_t addr, void *data, uint32_t num) {
    int32_t ret = WD_CODE_OK;

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    LOCAL_RES->Lock();

    {
        uint32_t remaining = num;
        uint32_t cur_addr = addr;
        uint8_t *cur_data = (uint8_t *)data;

        while (remaining > 0) {
            uint32_t phys = cur_addr - LOCAL_RES->Capabilities.Addr;
            uint32_t page_addr = W25N_ADDR_TO_PAGE(phys);
            uint16_t col = (uint16_t)W25N_ADDR_TO_COL(phys);
            uint32_t to_read = remaining;

            if ((col + to_read) > W25N_PAGE_SIZE) {
                to_read = W25N_PAGE_SIZE - col;
            }

            if (NAND_Page_Read(page_addr) != WD_CODE_OK) {
                WD_LOG_ERROR();
                ret = WD_CODE_ERROR;
                break;
            }
            if (NAND_Read_Cache(col, cur_data, to_read) != WD_CODE_OK) {
                WD_LOG_ERROR();
                ret = WD_CODE_ERROR;
                break;
            }

            cur_addr += to_read;
            cur_data += to_read;
            remaining -= to_read;
        }
    }

    LOCAL_RES->Unlock();
    return ret;
}

static int32_t block_device_nand_flash_write(uint32_t addr, const void *data, uint32_t num) {
    int32_t ret = WD_CODE_OK;

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    LOCAL_RES->Lock();

    {
        uint32_t remaining = num;
        uint32_t cur_addr = addr;
        const uint8_t *cur_data = (const uint8_t *)data;

        while (remaining > 0) {
            uint32_t phys = cur_addr - LOCAL_RES->Capabilities.Addr;
            uint32_t page_addr = W25N_ADDR_TO_PAGE(phys);
            uint16_t col = (uint16_t)W25N_ADDR_TO_COL(phys);
            uint32_t to_write = remaining;

            if ((col + to_write) > W25N_PAGE_SIZE) {
                to_write = W25N_PAGE_SIZE - col;
            }

            if (NAND_Write_Enable() != WD_CODE_OK) {
                WD_LOG_ERROR();
                ret = WD_CODE_ERROR;
                break;
            }
            if (NAND_Load_Cache(col, cur_data, to_write) != WD_CODE_OK) {
                WD_LOG_ERROR();
                ret = WD_CODE_ERROR;
                break;
            }
            if (NAND_Program_Execute(page_addr) != WD_CODE_OK) {
                WD_LOG_ERROR();
                ret = WD_CODE_ERROR;
                break;
            }

            cur_addr += to_write;
            cur_data += to_write;
            remaining -= to_write;
        }
    }

    LOCAL_RES->Unlock();
    return ret;
}

static int32_t block_device_nand_flash_erase(uint32_t addr, const uint8_t fill, uint32_t num) {
    int32_t ret = WD_CODE_OK;

    UNUSED(fill); /* NAND always erases to 0xFF */

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    LOCAL_RES->Lock();

    {
        uint32_t cur_addr = addr;
        uint32_t remaining = num;

        while (remaining > 0) {
            uint32_t phys = cur_addr - LOCAL_RES->Capabilities.Addr;
            uint32_t block_num = W25N_ADDR_TO_BLOCK(phys);
            uint32_t erase_size = LOCAL_RES->Capabilities.Erase_Sector_Size;

            if (NAND_Block_Erase(block_num) != WD_CODE_OK) {
                WD_LOG_ERROR();
                ret = WD_CODE_ERROR;
                break;
            }

            cur_addr += erase_size;
            remaining = (remaining > erase_size) ? (remaining - erase_size) : 0;
        }
    }

    LOCAL_RES->Unlock();
    return ret;
}

static int32_t block_device_nand_flash_control(uint32_t control, uint32_t arg) {
    UNUSED(control);
    UNUSED(arg);
    return (WD_CODE_OK);
}

static int32_t block_device_nand_flash_get_status(void) {
    return (int32_t)(LOCAL_RES->Info->Status);
}

static void block_device_nand_flash_validation(void) {
    /* Not implemented */
}

/***************************************************************************************************************************************************************
 **********  Exported Instance
 **************************************************************************************************************************************************************/

wd_blockdev_instance_info_t *const wd_nand_res_block_device_nand_flash =
    &Block_Device_NAND_Flash_Info;

wd_blockdev_instance_t wd_drv_block_device_nand_flash = {
    .Initialize = block_device_nand_flash_initialize,
    .Uninitialize = block_device_nand_flash_uninitialize,
    .PowerControl = block_device_nand_flash_power_control,
    .Write = block_device_nand_flash_write,
    .Read = block_device_nand_flash_read,
    .Erase = block_device_nand_flash_erase,
    .Control = block_device_nand_flash_control,
    .Get_Status = block_device_nand_flash_get_status,
    .Validation = block_device_nand_flash_validation,
    .Res = LOCAL_RES,
};

#ifndef NDEBUG
__USED wd_nand_debug_block_device_nand_flash_t wd_nand_dbg_block_device_nand_flash = {
    .Resource = LOCAL_RES,
    .Info = &LOCAL_RES->Info,
    .Driver = LOCAL_DRV,
};
#endif

#endif /* RTE_COMPONENT_BLOCK_DEVICE_NAND_FLASH_W25N04KBZEIR */
