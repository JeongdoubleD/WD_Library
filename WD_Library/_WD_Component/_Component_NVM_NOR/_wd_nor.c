/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_nor.c
 *
 *  Created on: Jan 13, 2025
 *      Author: doubled
 */

#include "_wd_nor.h"

#if 1 // 토글

#if (RTE_COMPONENT_BLOCK_DEVICE_NOR_FLASH_MX66L2G45G)
#define API_VALIDATION 0

static wd_blockdev_instance_info_t Block_Device_NOR_Flash_Info = {
    0,
};

#if (RTE_RTOS)
static osStaticMutexDef_t Mutex_Block_Device_NOR_Flash_cb_mem;
static osMutexId_t Mutex_Block_Device_NOR_Flash;
static const osMutexAttr_t Mutex_Block_Device_NOR_Flash_Attr = {
    .name = "Mutex Block_Device_NOR_Flash",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = &Mutex_Block_Device_NOR_Flash_cb_mem,
    .cb_size = sizeof(Mutex_Block_Device_NOR_Flash_cb_mem)};

static int32_t res_start(void) {
#if (RTE_RTOS)
    Mutex_Block_Device_NOR_Flash = osMutexNew(&Mutex_Block_Device_NOR_Flash_Attr);
    if (Mutex_Block_Device_NOR_Flash == NULL) {
#ifndef NDEBUG
        WD_LOG_ERROR("Mutex Block_Device_NOR_Flash New Error");
#endif
        return (WD_CODE_ERROR);
    }
#endif

    return (WD_CODE_OK);
}

static int32_t res_lock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        if (osMutexAcquire(Mutex_Block_Device_NOR_Flash, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
            return (WD_CODE_ERROR_TIMEOUT);
        }
    }

    return (WD_CODE_OK);
}

static int32_t res_unlock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(Mutex_Block_Device_NOR_Flash);
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

static uint8_t Block_Device_NOR_Flash_Buf[RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_SECTOR_SIZE] __ALIGNED(
    WD_CONF_DMA_ALIGNMENT_SIZE);
static uint8_t
    Block_Device_NOR_Flash_Confirm_Buf[RTE_BLOCK_DEVICE_NOR_FLASH_PROGRAM_SIZE] __ALIGNED(
        WD_CONF_DMA_ALIGNMENT_SIZE);

static wd_blockdev_instance_resources_t Block_Device_NOR_Flash_Resources = {
    .Capabilities =
        {
            .Name = "[NVM Flash]",
            .Addr = RTE_BLOCK_DEVICE_NOR_FLASH_ADDR,
            .Size = RTE_BLOCK_DEVICE_NOR_FLASH_SIZE,
            .Program_Size = RTE_BLOCK_DEVICE_NOR_FLASH_PROGRAM_SIZE,
            .Erase_Sector_Size = RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_SECTOR_SIZE,
            .Erase_Block_Size = RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_BLOCK_SIZE,
            .Erase_Block_Addr = RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_BLOCK_ADDR,
            .Program_TimeOut = RTE_BLOCK_DEVICE_NOR_FLASH_PROGRAM_TIMEOUT,
            .Erase_Sector_TimeOut = RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_SECTOR_TIMEOUT,
            .Erase_Block_TimeOut = RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_BLOCK_TIMEOUT,
            .Erase_Chip_TimeOut = RTE_BLOCK_DEVICE_NOR_FLASH_ERASE_CHIP_TIMEOUT,
            .Buf = Block_Device_NOR_Flash_Buf,
            .Buf_Size = sizeof(Block_Device_NOR_Flash_Buf),
            .Confirm_Buf = Block_Device_NOR_Flash_Confirm_Buf,
            .Confirm_Buf_Size = sizeof(Block_Device_NOR_Flash_Confirm_Buf),
        },
    .Info = &Block_Device_NOR_Flash_Info,
    .IO =
        {
            .WP = RTE_CONFIG_IDLE_PIN_NC,
            .RESET = RTE_CONFIG_IDLE_PIN_NC,
        },
    .Drv = NULL,
    .Handle = NULL,
    .Reg = FLASH,
    .Start = res_start,
    .Lock = res_lock,
    .Unlock = res_unlock,
};

#undef LOCAL_DRV
#define LOCAL_DRV (&wd_drv_block_device_nor_flash)
__USED static wd_blockdev_instance_resources_t *const LOCAL_RES = &Block_Device_NOR_Flash_Resources;

/**************************************************************************************************************************************************************/
#if 1 /// Low Level Driver - MX66L2G45G, LLD, v0.2.zip *******************************************************************************************************{
/**************************************************************************************************************************************************************/
/*** MX66 series command hex code definition ***/
//ID comands
#define FLASH_CMD_RDID  0x9F //RDID (Read Identification)
#define FLASH_CMD_RES   0xAB //RES (Read Electronic ID)
#define FLASH_CMD_REMS  0x90 //REMS (Read Electronic & Device ID)
#define FLASH_CMD_QPIID 0xAF //QPIID (QPI ID Read)

//Register comands
#define FLASH_CMD_WRSR    0x01 //WRSR (Write Status Register)
#define FLASH_CMD_RDSR    0x05 //RDSR (Read Status Register)
#define FLASH_CMD_WRSCUR  0x2F //WRSCUR (Write Security Register)
#define FLASH_CMD_RDSCUR  0x2B //RDSCUR (Read Security Register)
#define FLASH_CMD_RDCR    0x15 //RDCR (Read Configuration Register)
#define FLASH_CMD_RDEAR   0xC8 //RDEAR (Read Extended Address Register)
#define FLASH_CMD_WREAR   0xC5 //WREAR (Write Extended Address Register)
#define FLASH_CMD_RDFBR   0x16 //RDFBR (read fast boot Register)
#define FLASH_CMD_WRFBR   0x17 //WRFBR (write fast boot Register)
#define FLASH_CMD_ESFBR   0x18 //ESFBR (erase fast boot Register)
#define FLASH_CMD_WRLR    0x2C //WRLR (write lock Register)
#define FLASH_CMD_RDLR    0x2D //RDLR (read lock Register)
#define FLASH_CMD_RDSPB   0xE2 //RDSPB (read SPB status)
#define FLASH_CMD_WRSPB   0xE3 //WRSPB (write SPB bit)
#define FLASH_CMD_ESSPB   0xE4 //ESSPB (erase all SPB status)
#define FLASH_CMD_WRPASS  0x28 //WRPASS (write password Register)
#define FLASH_CMD_RDPASS  0x27 //RDPASS (read password Register)
#define FLASH_CMD_PASSULK 0x29 //RDPASS (password unlock)
#define FLASH_CMD_RDDPB   0xE0 //RDDPB (read DPB register)
#define FLASH_CMD_WRDPB   0xE1 //WRDPB (write DPB register)

//READ comands
#define FLASH_CMD_READ       0x03 //READ (1 x I/O)
#define FLASH_CMD_2READ      0xBB //2READ (2 x I/O)
#define FLASH_CMD_4READ      0xEB //4READ (4 x I/O)
#define FLASH_CMD_FASTREAD   0x0B //FAST READ (Fast read data).
#define FLASH_CMD_DREAD      0x3B //DREAD (1In/2 Out fast read)
#define FLASH_CMD_QREAD      0x6B //QREAD (1In/4 Out fast read)
#define FLASH_CMD_4DTRD      0xED //4DTRD (Quad DT read)
#define FLASH_CMD_RDSFDP     0x5A //RDSFDP (Read SFDP)
#define FLASH_CMD_READ4B     0x13 //READ4B (1 x I/O with 4 byte address)
#define FLASH_CMD_FASTREAD4B 0x0C //FASTREAD4B (1 x I/O with 4 byte address)
#define FLASH_CMD_2READ4B    0xBC //2READ4B (2 x I/O with 4 byte address)
#define FLASH_CMD_4READ4B    0xEC //4READ4B (4 x I/O with 4 byte address)
#define FLASH_CMD_DREAD4B    0x3C //DREAD4B (1In/2 Out fast read with 4 byte addr)
#define FLASH_CMD_QREAD4B    0x6C //QREAD4B (1In/4 Out fast read with 4 byte addr)
#define FLASH_CMD_4DTRD4B    0xEE //4DTRD4B (Quad DT read with 4 byte address)

//Program comands
#define FLASH_CMD_WREN  0x06 //WREN (Write Enable)
#define FLASH_CMD_WRDI  0x04 //WRDI (Write Disable)
#define FLASH_CMD_PP    0x02 //PP (page program)
#define FLASH_CMD_4PP   0x38 //4PP (Quad page program)
#define FLASH_CMD_PP4B  0x12 //PP4B (page program with 4 byte address)
#define FLASH_CMD_4PP4B 0x3E //4PP4B (Quad page program with 4 byte address)

//Erase comands
#define FLASH_CMD_SE      0x20 //SE (Sector Erase)
#define FLASH_CMD_BE32K   0x52 //BE32K (Block Erase 32kb)
#define FLASH_CMD_BE      0xD8 //BE (Block Erase)
#define FLASH_CMD_BE4B    0xDC //BE4B (Block Erase with 4 byte address)
#define FLASH_CMD_CE      0x60 //CE (Chip Erase) hex code: 60 or C7
#define FLASH_CMD_SE4B    0x21 //SE (Sector Erase with 4 byte addr)
#define FLASH_CMD_BE32K4B 0x5C //BE32K4B (Block Erase 32kb with 4 byte addr)

//Mode setting comands
#define FLASH_CMD_DP    0xB9 //DP (Deep Power Down)
#define FLASH_CMD_ENSO  0xB1 //ENSO (Enter Secured OTP)
#define FLASH_CMD_EXSO  0xC1 //EXSO  (Exit Secured OTP)
#define FLASH_CMD_EQIO  0x35 //EQIO (Enable Quad I/O)
#define FLASH_CMD_WPSEL 0x68 //WPSEL (Enable block protect mode)
#ifdef SBL_CMD_0x77
#define FLASH_CMD_SBL 0x77 //SBL (Set Burst Length), new: 0x77
#else
#define FLASH_CMD_SBL 0xC0 //SBL (Set Burst Length), old: 0xC0
#endif
#define FLASH_CMD_EN4B 0xB7 //EN4B( Enter 4-byte Mode )
#define FLASH_CMD_EX4B 0xE9 //EX4B( Exit 4-byte Mode )

//Reset comands
#define FLASH_CMD_RSTEN  0x66 //RSTEN (Reset Enable)
#define FLASH_CMD_RST    0x99 //RST (Reset Memory)
#define FLASH_CMD_RSTQIO 0xF5 //RSTQIO (Reset Quad I/O)

//Security comands
#define FLASH_CMD_GBLK  0x7E //GBLK (Gang Block Lock)
#define FLASH_CMD_GBULK 0x98 //GBULK (Gang Block Unlock)

//Suspend/Resume comands
#ifdef PGM_ERS_0xB0
#define FLASH_CMD_PGM_ERS_S 0xB0 //PGM/ERS Suspend (Suspends Program/Erase) old: 0xB0
#define FLASH_CMD_PGM_ERS_R 0x30 //PGM/ERS Erase (Resumes Program/Erase) old: 0x30
#else
#define FLASH_CMD_PGM_ERS_S 0x75 //PGM/ERS Suspend (Suspends Program/Erase) old: 0xB0
#define FLASH_CMD_PGM_ERS_R 0x7A //PGM/ERS Erase (Resumes Program/Erase) old: 0x30
#endif

#define FLASH_CMD_NOP 0x00 //NOP (No Operation)

typedef struct {
    uint16_t WIP : 1;  // Bit 0: Write In Progress (1=Write/Erase in progress, 0=Idle)
    uint16_t WEL : 1;  // Bit 1: Write Enable Latch (1=Write Enabled, 0=Write Disabled)
    uint16_t BP0 : 1;  // Bit 2: Block Protect Bit 0
    uint16_t BP1 : 1;  // Bit 3: Block Protect Bit 1
    uint16_t BP2 : 1;  // Bit 4: Block Protect Bit 2
    uint16_t BP3 : 1;  // Bit 5: Block Protect Bit 3
    uint16_t QE : 1;   // Bit 6: Quad Enable (1=Quad mode enabled, 0=Disabled)
    uint16_t SRWD : 1; // Bit 7: Status Register Write Protect (1=Disabled, 0=Enabled)

    uint16_t ODS0 : 1; // Bit 0: Output Driver Strength Bit 0
    uint16_t ODS1 : 1; // Bit 1: Output Driver Strength Bit 1
    uint16_t ODS2 : 1; // Bit 2: Output Driver Strength Bit 2
    uint16_t TB : 1;   // Bit 3: Top/Bottom Protect (0=Top area, 1=Bottom area)
    uint16_t PBE : 1;  // Bit 4: Preamble Bit Enable
    uint16_t A4B : 1;  // Bit 5: 4-Byte Address Mode Enable (1=4-byte, 0=3-byte)
    uint16_t DC0 : 1;  // Bit 6: Dummy Cycle Bit 0
    uint16_t DC1 : 1;  // Bit 7: Dummy Cycle Bit 1
} MX66L2G45G_STATUS_REGISTER;

static MX66L2G45G_STATUS_REGISTER QSPI_Status_Register;

static int32_t QSPI_Read_Status_Register(QSPI_HandleTypeDef *qspi, void *data);

static uint32_t QSPI_StatusMatch = 0;
void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *qspi) {
    UNUSED(qspi);
    QSPI_StatusMatch++;
}

static void block_device_nor_flash_callback(uint32_t event);
void HAL_QSPI_ErrorCallback(QSPI_HandleTypeDef *qspi) {
    UNUSED(qspi);
    WD_LOG_ERROR();
    block_device_nor_flash_callback(WDIF_EVENT_TRANSFER_ERROR);
}

static int32_t QSPI_Write_Enable(QSPI_HandleTypeDef *qspi) {
    QSPI_CommandTypeDef sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    /* Enable write operations ------------------------------------------ */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = FLASH_CMD_WREN;

    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address = 0;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.AlternateBytes = 0x00;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = 0;

    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(qspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    /* Configure automatic polling mode to wait for write enabling ---- */
    sConfig.Match = 0x02;
    sConfig.Mask = 0x02;
    sConfig.MatchMode = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval = 0x10;
    sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    sCommand.Instruction = FLASH_CMD_RDSR;
    sCommand.DataMode = QSPI_DATA_1_LINE;

    if (HAL_QSPI_AutoPolling(qspi, &sCommand, &sConfig, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    {
        MX66L2G45G_STATUS_REGISTER status;

        if (QSPI_Read_Status_Register(qspi, &status) != WD_CODE_OK) {
            WD_LOG_ERROR();
            return WD_CODE_ERROR;
        }

        if (status.WEL != 1) {
            WD_LOG_ERROR();
            return WD_CODE_ERROR;
        }
    }

    return WD_CODE_OK;
}

static int32_t QSPI_AutoPolling_NB(QSPI_HandleTypeDef *qspi) {
    QSPI_CommandTypeDef sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    /* Configure automatic polling mode to wait for memory ready ------ */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = FLASH_CMD_RDSR;

    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address = 0;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.AlternateBytes = 0x00;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = 0;

    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    sConfig.Match = 0x00;
    sConfig.Mask = 0x01;
    sConfig.MatchMode = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval = 0x10;
    sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_QSPI_AutoPolling_IT(qspi, &sCommand, &sConfig) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    return WD_CODE_OK;
}

static int32_t QSPI_Reset(QSPI_HandleTypeDef *qspi) {
    QSPI_CommandTypeDef sCommand;

    /* Initialize the reset enable command */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = FLASH_CMD_RSTEN;

    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address = 0;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.AlternateBytes = 0x00;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = 0;

    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    /* Send the command */
    if (HAL_QSPI_Command(qspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    /* Send the reset memory command */
    sCommand.Instruction = FLASH_CMD_RST;
    if (HAL_QSPI_Command(qspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    /* Configure automatic polling mode to wait the memory is ready */
    QSPI_StatusMatch = 0;
    QSPI_AutoPolling_NB(qspi);
    //  WD_WAIT_WHILE((QSPI_StatusMatch == 0), 100000);

    return WD_CODE_OK;
}

static void QSPI_Print_Status_Register(MX66L2G45G_STATUS_REGISTER *status) {
    WD_PRINT_DEBUG(WD_MSG_DEBUG_CRLF,
                   "[MX66L2G45G Status Reg] WIP : %u    WEL : %u    BP : %u%u%u%u    QE : %u    "
                   "SRWD : %u    ODS : %u%u%u    TB : %u    PBE : %u    A4B : %u    DC0 : %u%u",
                   status->WIP, status->WEL, status->BP0, status->BP1, status->BP2, status->BP3,
                   status->QE, status->SRWD, status->ODS0, status->ODS1, status->ODS2, status->TB,
                   status->PBE, status->A4B, status->DC0, status->DC1);
}

static int32_t QSPI_Read_Status_Register(QSPI_HandleTypeDef *qspi, void *data) {
    QSPI_CommandTypeDef sCommand;

    uint8_t status[2];

    /* Read status register */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = FLASH_CMD_RDSR;

    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address = 0;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.AlternateBytes = 0x00;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = 1;

    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(qspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    if (HAL_QSPI_Receive(qspi, &status[0], HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    sCommand.Instruction = FLASH_CMD_RDCR;

    if (HAL_QSPI_Command(qspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    if (HAL_QSPI_Receive(qspi, &status[1], HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    if (data != NULL) {
        memcpy(data, status, sizeof(status));
    }

    return WD_CODE_OK;
}

static int32_t QSPI_Write_Status_Register(QSPI_HandleTypeDef *qspi, void *data) {
    QSPI_CommandTypeDef sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    MX66L2G45G_STATUS_REGISTER *status = (MX66L2G45G_STATUS_REGISTER *)data;

    QSPI_Write_Enable(qspi);

    /* Write status register */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = FLASH_CMD_WRSR;

    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address = 0;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.AlternateBytes = 0x00;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = sizeof(*status);

    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(qspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
    }

    if (HAL_QSPI_Transmit(qspi, (uint8_t *)status, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
    }

    /* Wait that memory is ready */

    /* Configure automatic polling mode to wait for memory ready ------ */
    sCommand.Instruction = FLASH_CMD_RDSR;

    sConfig.Match = 0x00;
    sConfig.Mask = 0x01;
    sConfig.MatchMode = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval = 0x10;
    sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_QSPI_AutoPolling(qspi, &sCommand, &sConfig, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    return WD_CODE_OK;
}

static int32_t QSPI_Read_ID(QSPI_HandleTypeDef *qspi, void *data) {
    QSPI_CommandTypeDef sCommand;

    uint8_t id[4];

    /* Read status register */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = FLASH_CMD_RDID;

    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address = 0;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.AlternateBytes = 0x00;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = 3;

    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(qspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    if (HAL_QSPI_Receive(qspi, id, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    memcpy(data, id, 3);

    return WD_CODE_OK;
}

static int32_t QSPI_Send_Command(QSPI_HandleTypeDef *qspi, uint8_t cmd) {
    QSPI_CommandTypeDef sCommand;

    /* Write status register */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = cmd;

    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address = 0;

    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.AlternateBytes = 0x00;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = 0;

    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(qspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        WD_LOG_ERROR();
    }

    return WD_CODE_OK;
}

#if (API_VALIDATION)
static int32_t QSPI_Chip_Erase(QSPI_HandleTypeDef *qspi) // 215332 ms
{
    QSPI_CommandTypeDef sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    int32_t status;

    status = QSPI_Write_Enable(qspi);

    if (status == WD_CODE_OK) {
        /* Enable write operations ------------------------------------------ */
        sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        sCommand.Instruction = FLASH_CMD_CE;

        sCommand.AddressMode = QSPI_ADDRESS_NONE;
        sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
        sCommand.Address = 0;

        sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        sCommand.AlternateBytes = 0x00;
        sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

        sCommand.DataMode = QSPI_DATA_NONE;
        sCommand.DummyCycles = 0;
        sCommand.NbData = 0;

        sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
        sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
        sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

        if (HAL_QSPI_Command(qspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            WD_LOG_ERROR();
            return WD_CODE_ERROR;
        }

        /* Configure automatic polling mode to wait for write enabling ---- */
        sConfig.Match = 0x00;
        sConfig.Mask = 0x01;
        sConfig.MatchMode = QSPI_MATCH_MODE_AND;
        sConfig.StatusBytesSize = 1;
        sConfig.Interval = 0x10;
        sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

        sCommand.Instruction = FLASH_CMD_RDSR;
        sCommand.DataMode = QSPI_DATA_1_LINE;

        if (HAL_QSPI_AutoPolling(qspi, &sCommand, &sConfig, 400000) != HAL_OK) {
            WD_LOG_ERROR();
            block_device_nor_flash_callback(WDIF_EVENT_TRANSFER_ERROR);
            return (WD_CODE_ERROR);
        }
    } else {
        return (WD_CODE_ERROR);
    }

    return (WD_CODE_OK);
}
#endif
#endif /// Low Level Driver - MX66L2G45G, LLD, v0.2.zip *******************************************************************************************************}

__USED static void block_device_nor_flash_callback(uint32_t event) {
    LOCAL_RES->Info->Event = event;
    if (LOCAL_RES->Info->CB_Event) {
        LOCAL_RES->Info->CB_Event(event);
    }
}

static int32_t block_device_nor_flash_initialize(wd_event_callback_t cb_event) {
    if (LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Block_Device_NOR_Flash Constructor");

    memset(LOCAL_RES->Info, 0, sizeof(wd_blockdev_instance_info_t));

    if (cb_event) {
        LOCAL_RES->Info->CB_Event = cb_event;
    }

    LOCAL_RES->Start();

    LOCAL_RES->Info->Flags = WDIF_FLAG_INITIALIZED;

    return (WD_CODE_OK);
}

static int32_t block_device_nor_flash_uninitialize(void) {
    return (WD_CODE_OK);
}

static int32_t block_device_nor_flash_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Block_Device_NOR_Flash PowerControl:WD_POWER_OFF");

        LOCAL_RES->Info->Flags &= ~(WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
    } break;

    case WD_POWER_FULL: {
        if ((LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) == 0U) {
            return (WD_CODE_ERROR);
        } else if (LOCAL_RES->Info->Flags & WDIF_FLAG_POWERED) {
            return (WD_CODE_OK);
        }

        if (LOCAL_RES->Capabilities.Buf) {
            LOCAL_RES->Info->Xfer.Buf = LOCAL_RES->Capabilities.Buf;
        } else {
            LOCAL_RES->Info->Xfer.Buf = wd_core_scratch_buffer();
        }

        if (LOCAL_RES->Capabilities.Buf_Size) {
            LOCAL_RES->Info->Xfer.Buf_Size = LOCAL_RES->Capabilities.Buf_Size;
        } else {
            LOCAL_RES->Info->Xfer.Buf_Size = wd_core_scratch_size();
        }

        {
            uint32_t hclkFreq = HAL_RCC_GetHCLKFreq();
            uint32_t prescaler = (RCC->D1CFGR & RCC_D1CFGR_HPRE) >> RCC_D1CFGR_HPRE_Pos;
            uint32_t apbclockfreq = (prescaler >= 8) ? (hclkFreq >> (prescaler - 7)) : hclkFreq;
            uint32_t desiredqspifreq = 4000000; //60000000; //4000000
            uint32_t calculatedprescaler = ((apbclockfreq * 2) / desiredqspifreq) - 1;
            uint32_t actualqspifreq = apbclockfreq / (calculatedprescaler + 1);

            UNUSED(actualqspifreq);

            (RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE)->Instance = QUADSPI;
            HAL_QSPI_DeInit(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE);

            (RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE)->Init.ClockPrescaler = calculatedprescaler;
            (RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE)->Init.FifoThreshold = 4;
            (RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE)->Init.SampleShifting =
                QSPI_SAMPLE_SHIFTING_HALFCYCLE; //QSPI_SAMPLE_SHIFTING_NONE;
            (RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE)->Init.FlashSize =
                27; // 2^(27 + 1) = 256 Mbytes
            (RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE)->Init.ChipSelectHighTime =
                QSPI_CS_HIGH_TIME_1_CYCLE;
            (RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE)->Init.ClockMode = QSPI_CLOCK_MODE_0;
            (RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE)->Init.FlashID = QSPI_FLASH_ID_1;
            (RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE)->Init.DualFlash = QSPI_DUALFLASH_DISABLE;

            if (HAL_QSPI_Init(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE) != HAL_OK) {
                WD_LOG_ERROR();
            }

            if (QSPI_Reset(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE) != WD_CODE_OK) {
                WD_LOG_ERROR();
                HAL_QSPI_Init(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE);
            }
            wd_time_wait_us(100);
            LOCAL_RES->Info->Status = 0;

            {
                uint8_t version[3] = {0};

                if (QSPI_Read_ID(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE, version) !=
                    WD_CODE_OK) {
                    WD_LOG_ERROR();
                    LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
                }

                if ((version[0] != 0xC2) || (version[1] != 0x20) ||
                    (version[2] != 0x1C)) // Manufacturer ID // Memory Type // Capacity
                {
                    WD_LOG_ERROR("NVM_NOR_Flash JEDEC ID Error");
                    LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
                }

                if (version[2] < 0x1C) {
                    WD_LOG_ERROR("NVM_NOR_Flash Size too Small");
                    LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
                }
                WD_PRINT_DEBUG(WD_MSG_DEBUG_CRLF,
                               "[MX66L2G45G JEDEC Code] Manufacturer ID : %02X    Memory Type : "
                               "%02X    Capacity : %02X",
                               version[0], version[1], version[2]);
            }
        }

        if (QSPI_Read_Status_Register(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE,
                                      &QSPI_Status_Register) != WD_CODE_OK) {
            WD_LOG_ERROR();
            LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
        }

        if ((QSPI_Status_Register.QE != 1) || (QSPI_Status_Register.SRWD != 0)) {
            GPIO_InitTypeDef GPIO_InitStruct = {0};

            GPIO_InitStruct.Pin = QSPI_FLASH_DQ2_Pin;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            HAL_GPIO_Init(QSPI_FLASH_DQ2_GPIO_Port, &GPIO_InitStruct);

            HAL_GPIO_WritePin(QSPI_FLASH_DQ2_GPIO_Port, QSPI_FLASH_DQ2_Pin, GPIO_PIN_SET);

            if (QSPI_Status_Register.QE != 1) {
                WD_LOG_ERROR("NVM_NOR_Flash Error Quad Enable");
            }

            if (QSPI_Status_Register.SRWD != 0) {
                WD_LOG_ERROR("NVM_NOR_Flash Error SRP (Status Register Protect)");
            }

            QSPI_Status_Register.QE = 1;
            QSPI_Status_Register.SRWD = 0;

            QSPI_Status_Register.ODS0 = 1;
            QSPI_Status_Register.ODS1 = 1;
            QSPI_Status_Register.ODS2 = 1;
            QSPI_Status_Register.TB = 1;
            QSPI_Status_Register.PBE = 0;
            QSPI_Status_Register.A4B = 1;

            if (QSPI_Write_Status_Register(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE,
                                           &QSPI_Status_Register) != WD_CODE_OK) {
                WD_LOG_ERROR();
                LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
            }

            GPIO_InitStruct.Pin = QSPI_FLASH_DQ2_Pin;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
            HAL_GPIO_Init(QSPI_FLASH_DQ2_GPIO_Port, &GPIO_InitStruct);
        }

        if (QSPI_Status_Register.A4B != 1) {
            QSPI_Send_Command(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE, FLASH_CMD_EN4B);
        }

        memset(&QSPI_Status_Register, 0, sizeof(QSPI_Status_Register));

        if (QSPI_Read_Status_Register(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE,
                                      &QSPI_Status_Register) != WD_CODE_OK) {
            WD_LOG_ERROR("Status Write");
            QSPI_Print_Status_Register(&QSPI_Status_Register);
            LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
        }

        QSPI_Print_Status_Register(&QSPI_Status_Register);

        if ((QSPI_Status_Register.QE != 1) || (QSPI_Status_Register.SRWD != 0) ||
            (QSPI_Status_Register.A4B != 1)) {
            WD_LOG_ERROR("NVM_NOR_Flash Error Protect");
            LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
        }

        if (LOCAL_RES->Info->Status & WDIF_BLOCK_DEVICE_STATUS_ERROR) {
            wd_core_status_hook(WD_STATUS_SOURCE_NVM_FLASH, WD_STATUS_ERROR);
            return WD_CODE_ERROR;
        }

        //        LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
        wd_core_status_hook(WD_STATUS_SOURCE_NVM_FLASH, WD_STATUS_OK);

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Block_Device_NOR_Flash PowerControl:WD_POWER_FULL");

        LOCAL_RES->Info->Flags |= (WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
    } break;

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);
    default:
        break;
    }

    return (WD_CODE_OK);
}

static int32_t block_device_nor_flash_write(uint32_t addr, const void *data, uint32_t num) {
    int32_t ret = WD_CODE_OK;
    int32_t status;
    const uint32_t base = LOCAL_RES->Capabilities.Addr;
    uint32_t phys = addr - base;

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    if ((!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size)) ||
        (!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr + num,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size))) {
        return (WD_CODE_ERROR);
    }

    if (!wd_core_is_valid_address((uint32_t)LOCAL_RES->Info->Xfer.Buf)) {
        LOCAL_RES->Info->Xfer.Buf = wd_core_scratch_buffer();
        LOCAL_RES->Info->Xfer.Buf_Size = wd_core_scratch_size();
    }

    LOCAL_RES->Lock();

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == wd_core_scratch_buffer()) {
            if (osMutexAcquire(wd_os_mutex_global, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
                WD_BREAK();
            }
        }
    }
#endif

    {
        uint8_t *buf;
        QSPI_CommandTypeDef sCommand;
        uint32_t offset = 0;
        uint32_t n;

        uint32_t __time_start = wd_time_get_us();
        uint32_t __time_start2;
        uint32_t __time_out = LOCAL_RES->Capabilities.Program_TimeOut *
                              (num / LOCAL_RES->Capabilities.Program_Size + 2);

        buf = (uint8_t *)data;

        {
            uint32_t __addr = (uint32_t)buf & ~(uint32_t)0x1F;
            uint32_t __len = ((uint32_t)num + ((uint32_t)buf - __addr) + 31u) & ~31u;
            SCB_CleanDCache_by_Addr((uint32_t *)__addr, (int32_t)__len);
        }

        do {
            /* Enable data write */
            status = QSPI_Write_Enable(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE);

            if (status == WD_CODE_OK) {
                n = LOCAL_RES->Capabilities.Program_Size -
                    (addr % LOCAL_RES->Capabilities.Program_Size);

                if (n > num) {
                    n = num;
                }

                /* Write status register */
                sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
                sCommand.Instruction = FLASH_CMD_4PP4B;

                sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
                sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
                sCommand.Address = phys; //addr;

                sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
                sCommand.AlternateBytes = 0x00;
                sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

                sCommand.DataMode = QSPI_DATA_4_LINES;
                sCommand.DummyCycles = 0;
                sCommand.NbData = n;

                sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
                sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
                sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

                if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE, &sCommand,
                                     HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
                    WD_LOG_ERROR();
                }

                if (HAL_QSPI_Transmit(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE, &buf[offset],
                                      HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
                    WD_LOG_ERROR();
                }

#if 0
        /* Wait that memory is ready */

        /* Configure automatic polling mode to wait for memory ready ------ */
        sCommand.Instruction        = FLASH_CMD_RDSR;

        sConfig.Match               = 0x00;
        sConfig.Mask                = 0x01;
        sConfig.MatchMode           = QSPI_MATCH_MODE_AND;
        sConfig.StatusBytesSize     = 1;
        sConfig.Interval            = 0x10;
        sConfig.AutomaticStop       = QSPI_AUTOMATIC_STOP_ENABLE;

        if (HAL_QSPI_AutoPolling(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE, &sCommand, &sConfig, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
          WD_LOG_ERROR();
          block_device_nor_flash_callback(WDIF_EVENT_TRANSFER_ERROR);
          ret = WD_CODE_ERROR;
          goto __end__;
        }
#endif

                addr += n;
                offset += n;
                num -= n;

                __time_start2 = wd_time_get_us();

                /* Read status until device ready */
                do {
                    MX66L2G45G_STATUS_REGISTER status_register;

                    if (QSPI_Read_Status_Register(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE,
                                                  &status_register) != WD_CODE_OK) {
                        WD_LOG_ERROR();
                        block_device_nor_flash_callback(WDIF_EVENT_TRANSFER_ERROR);
                        ret = WD_CODE_ERROR;
                        goto __end__;
                    }

                    if (status_register.SRWD != 0U) {
                        LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
                    }

                    if (status_register.WIP == 0U) {
                        LOCAL_RES->Info->Status &= ~(uint32_t)WDIF_BLOCK_DEVICE_STATUS_BUSY;

                    } else {
                        LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_BUSY;
                    }

                    if (wd_time_calc_us(&__time_start2) > LOCAL_RES->Capabilities.Program_TimeOut) {
                        LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
                        block_device_nor_flash_callback(WDIF_EVENT_TRANSFER_ERROR);
                        ret = WD_CODE_ERROR;
                        break;
                    }
                } while (LOCAL_RES->Info->Status & WDIF_BLOCK_DEVICE_STATUS_BUSY);
            } else {
                break;
            }

            /* Number of data items programmed */
            status = (int32_t)offset;

            if (wd_time_calc_us(&__time_start) > __time_out) {
                break;
            }
        } while (num);
    }

__end__:
#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == wd_core_scratch_buffer()) {
            osMutexRelease(wd_os_mutex_global);
        }
    }
#endif

    LOCAL_RES->Unlock();

    WD_CHECK(ret);

    return (ret);
}

static int32_t block_device_nor_flash_read(uint32_t addr, void *data, uint32_t num) {
    int32_t ret = WD_CODE_OK;
    uint8_t *buf = (uint8_t *)data;
    const uint32_t base = LOCAL_RES->Capabilities.Addr;
    uint32_t phys = addr - base;


    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    if ((!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size)) ||
        (!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr + num,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size))) {
        return (WD_CODE_ERROR);
    }

    if (!wd_core_is_valid_address((uint32_t)LOCAL_RES->Info->Xfer.Buf)) {
        LOCAL_RES->Info->Xfer.Buf = wd_core_scratch_buffer();
        LOCAL_RES->Info->Xfer.Buf_Size = wd_core_scratch_size();
    }

    LOCAL_RES->Lock();

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == wd_core_scratch_buffer()) {
            if (osMutexAcquire(wd_os_mutex_global, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
                WD_BREAK();
            }
        }
    }
#endif

    {
        uint32_t __addr = (uint32_t)buf & ~(uint32_t)0x1F;
        uint32_t __len = ((uint32_t)num + ((uint32_t)buf - __addr) + 31u) & ~31u;
        SCB_CleanDCache_by_Addr((uint32_t *)__addr, (int32_t)__len);
    }

    {
        QSPI_CommandTypeDef sCommand;

        sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        sCommand.Instruction = FLASH_CMD_4READ4B;

        sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
        sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
        sCommand.Address = phys; //addr;

        sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        sCommand.AlternateBytes = 0x00;
        sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

        sCommand.DataMode = QSPI_DATA_4_LINES;
        sCommand.DummyCycles = 6;
        sCommand.NbData = num;

        sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
        sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
        sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

        if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE, &sCommand,
                             HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            WD_LOG_ERROR();
            block_device_nor_flash_callback(WDIF_EVENT_TRANSFER_ERROR);
            ret = WD_CODE_ERROR;
            goto __end__;
        }

        if (HAL_QSPI_Receive(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE, (uint8_t *)data,
                             HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            WD_LOG_ERROR();
            block_device_nor_flash_callback(WDIF_EVENT_TRANSFER_ERROR);
            ret = WD_CODE_ERROR;
            goto __end__;
        }
    }

__end__:
#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == wd_core_scratch_buffer()) {
            osMutexRelease(wd_os_mutex_global);
        }
    }
#endif

    LOCAL_RES->Unlock();

    WD_CHECK(ret);

    return (ret);
}

static int32_t block_device_nor_flash_erase(uint32_t addr, const uint8_t data, uint32_t num) {
    int32_t ret = WD_CODE_OK;
    int32_t status;

    const uint32_t base = LOCAL_RES->Capabilities.Addr;
    uint32_t phys = addr - base;

    UNUSED(data);

    /*--------------- data : Unsupported ---------------*/
    /*--------------- num : Ignore       ---------------*/

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    if ((!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size)) ||
        (!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr + num,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size))) {
        return (WD_CODE_ERROR);
    }

    if (!wd_core_is_valid_address((uint32_t)LOCAL_RES->Info->Xfer.Buf)) {
        LOCAL_RES->Info->Xfer.Buf = wd_core_scratch_buffer();
        LOCAL_RES->Info->Xfer.Buf_Size = wd_core_scratch_size();
    }

    LOCAL_RES->Lock();

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == wd_core_scratch_buffer()) {
            if (osMutexAcquire(wd_os_mutex_global, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
                WD_BREAK();
            }
        }
    }
#endif

    {
        QSPI_CommandTypeDef sCommand;
        QSPI_AutoPollingTypeDef sConfig;

        status = QSPI_Write_Enable(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE);

        if (status == WD_CODE_OK) {
            LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_BUSY;
            LOCAL_RES->Info->Status &= ~(uint32_t)WDIF_BLOCK_DEVICE_STATUS_ERROR;

            /* Enable write operations ------------------------------------------ */
            sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
            if ((LOCAL_RES->Capabilities.Erase_Block_Addr > 0) &&
                (addr >= LOCAL_RES->Capabilities.Erase_Block_Addr) &&
                (addr % LOCAL_RES->Capabilities.Erase_Block_Size == 0)) {
                sCommand.Instruction = FLASH_CMD_BE4B;
            } else {
                sCommand.Instruction = FLASH_CMD_SE4B;
            }

            sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
            sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
            sCommand.Address = phys; //addr;

            sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
            sCommand.AlternateBytes = 0x00;
            sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;

            sCommand.DataMode = QSPI_DATA_NONE;
            sCommand.DummyCycles = 0;
            sCommand.NbData = 0;

            sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
            sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
            sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

            if (HAL_QSPI_Command(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE, &sCommand,
                                 HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
                WD_LOG_ERROR();
                block_device_nor_flash_callback(WDIF_EVENT_TRANSFER_ERROR);
                ret = WD_CODE_ERROR;
                goto __end__;
            }

            /* Configure automatic polling mode to wait for write enabling ---- */
            sConfig.Match = 0x00;
            sConfig.Mask = 0x01;
            sConfig.MatchMode = QSPI_MATCH_MODE_AND;
            sConfig.StatusBytesSize = 1;
            sConfig.Interval = 0x10;
            sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

            sCommand.Instruction = FLASH_CMD_RDSR;
            sCommand.DataMode = QSPI_DATA_1_LINE;

            if (HAL_QSPI_AutoPolling(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE, &sCommand, &sConfig,
                                     HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
                WD_LOG_ERROR();
                block_device_nor_flash_callback(WDIF_EVENT_TRANSFER_ERROR);
                ret = WD_CODE_ERROR;
                goto __end__;
            }
        }
    }

__end__:
#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == wd_core_scratch_buffer()) {
            osMutexRelease(wd_os_mutex_global);
        }
    }
#endif

    LOCAL_RES->Unlock();

    WD_CHECK(ret);

    return (ret);
}

static int32_t block_device_nor_flash_control(uint32_t control, uint32_t arg) {
    int32_t ret = WD_CODE_OK;

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    switch (control & WDIF_BLOCK_DEVICE_COMMAND_MSK) {
    case WDIF_BLOCK_DEVICE_COMMAND_ERASE: {
        if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
            return (WD_CODE_ERROR);
        }

        if (!WD_RANGE(LOCAL_RES->Capabilities.Addr, arg,
                      (LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size))) {
            return (WD_CODE_ERROR);
        }

        if (!wd_core_is_valid_address((uint32_t)LOCAL_RES->Info->Xfer.Buf)) {
            LOCAL_RES->Info->Xfer.Buf = wd_core_scratch_buffer();
            LOCAL_RES->Info->Xfer.Buf_Size = wd_core_scratch_size();
        }

        ret = WD_CODE_ERROR_UNSUPPORTED;
    } break;

    default:
        ret = WD_CODE_ERROR_UNSUPPORTED;
        break;
    }

    return (ret);
}

static int32_t block_device_nor_flash_get_status(void) {
    int32_t ret = WD_CODE_OK;

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    LOCAL_RES->Lock();

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == wd_core_scratch_buffer()) {
            if (osMutexAcquire(wd_os_mutex_global, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
                WD_BREAK();
            }
        }
    }
#endif

    if (QSPI_Read_Status_Register(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE,
                                  &QSPI_Status_Register) != WD_CODE_OK) {
        WD_LOG_ERROR();
        ret = WD_CODE_ERROR_TIMEOUT;
    } else {
        if (QSPI_Status_Register.SRWD) {
            LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_ERROR;
            ret = WD_CODE_ERROR;
        }

        if (QSPI_Status_Register.WIP) {
            LOCAL_RES->Info->Status |= WDIF_BLOCK_DEVICE_STATUS_BUSY;
            ret = WD_CODE_ERROR_BUSY;
        } else {
            LOCAL_RES->Info->Status &= ~(uint32_t)WDIF_BLOCK_DEVICE_STATUS_BUSY;
        }
    }

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == wd_core_scratch_buffer()) {
            osMutexRelease(wd_os_mutex_global);
        }
    }
#endif

    LOCAL_RES->Unlock();

    WD_CHECK(ret);

    return (ret);
}

static void block_device_nor_flash_validation(void) {
#if (API_VALIDATION)
#define API_VALIDATION_TEST_ADDR 0x12143
#define API_VALIDATION_TEST_CHAR 0xA5U

    uint32_t i, j;
    uint32_t debug_time;
    uint32_t data;
    //  api_validation_s
    /* validation work area on the shared scratch buffer (module local layout) */
    struct nor_api_validation_s {
        uint8_t W_buf[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE];
        uint8_t R_buf[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE * 3];
        uint8_t A_buf[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE * 3];
    } *api_validation = (struct nor_api_validation_s *)wd_core_scratch_buffer();

    if (wd_core_scratch_size() < sizeof(*api_validation)) {
        WD_LOG_ERROR("Scratch buffer too small for NOR validation");
        return;
    }

    wd_drv_block_device->Erase(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                               0xFF, LOCAL_RES->Capabilities.Erase_Sector_Size, NULL);
    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));

    wd_memset_rand(api_validation->W_buf, sizeof(api_validation->W_buf));

    wd_drv_block_device->Write(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                               api_validation->W_buf, 5, WDIF_BLOCK_DEVICE_ARGUMENT_CRC);
    wd_drv_block_device->Read(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                              api_validation->R_buf, 5, WDIF_BLOCK_DEVICE_ARGUMENT_CRC);

    wd_memcmp_print(api_validation->W_buf, api_validation->R_buf, 5);

    memset(api_validation->W_buf, 0xCC, sizeof(api_validation->R_buf));
    wd_drv_block_device->Erase(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                               0xCC, 30, WDIF_BLOCK_DEVICE_ARGUMENT_CRC);
    wd_drv_block_device->Read(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                              api_validation->R_buf, 30, WDIF_BLOCK_DEVICE_ARGUMENT_CRC);

    wd_memcmp_print(api_validation->W_buf, api_validation->R_buf, 30);


    /*--------------- Validation Initialize ---------------*/
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
    debug_time = wd_time_get_ms();
    QSPI_Chip_Erase(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE);
    debug_time = wd_time_calc_ms(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u" WD_COLOR_RESET "\r\n",
                  "[MX66L2G45G Validation Initialize] - Chip Erase [ms]", debug_time);

    for (i = 0; i < LOCAL_RES->Capabilities.Size; i += LOCAL_RES->Capabilities.Erase_Sector_Size) {
        memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));

        wd_drv_block_device_nor_flash.Read(i, api_validation->R_buf,
                                           LOCAL_RES->Capabilities.Erase_Sector_Size);

        if (wd_memchrn(api_validation->R_buf, 0xFF, LOCAL_RES->Capabilities.Erase_Sector_Size)) {
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
    }

    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
    WD_PRINT_INFO(WD_MSG_TITLE_CRLF, "[MX66L2G45G Validation]");
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);

    /*--------------- Step 1 - Byte Write ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_drv_block_device_nor_flash.Write(API_VALIDATION_TEST_ADDR, api_validation->W_buf, 1);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 1 - Byte Write", debug_time);

    /*--------------- Step 2 - Byte Read ---------------*/
    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    debug_time = wd_time_get_us();
    wd_drv_block_device_nor_flash.Read(API_VALIDATION_TEST_ADDR, api_validation->R_buf, 1);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 2 - Byte Read", debug_time);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    wd_drv_block_device_nor_flash.Erase(API_VALIDATION_TEST_ADDR, 0xFF,
                                        LOCAL_RES->Capabilities.Erase_Sector_Size);
    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(
        API_VALIDATION_TEST_ADDR -
            (API_VALIDATION_TEST_ADDR % LOCAL_RES->Capabilities.Erase_Sector_Size),
        api_validation->R_buf, LOCAL_RES->Capabilities.Erase_Sector_Size);
    if (wd_memchrn(api_validation->R_buf, 0xFF, LOCAL_RES->Capabilities.Erase_Sector_Size)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 3 - Page Write ---------------*/
    wd_drv_block_device_nor_flash.Erase(0, 0xFF, LOCAL_RES->Capabilities.Erase_Sector_Size);
    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(0, api_validation->R_buf,
                                       LOCAL_RES->Capabilities.Erase_Sector_Size);
    if (wd_memchrn(api_validation->R_buf, 0xFF, LOCAL_RES->Capabilities.Erase_Sector_Size)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    debug_time = wd_time_get_us();
    wd_drv_block_device_nor_flash.Write(0, api_validation->W_buf,
                                        LOCAL_RES->Capabilities.Program_Size);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 3 - Page Write", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(0, api_validation->R_buf,
                                       LOCAL_RES->Capabilities.Program_Size);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR,
                   LOCAL_RES->Capabilities.Program_Size)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    for (i = LOCAL_RES->Capabilities.Program_Size; i < LOCAL_RES->Capabilities.Erase_Sector_Size;
         i += LOCAL_RES->Capabilities.Program_Size) {
        wd_drv_block_device_nor_flash.Write(i, api_validation->W_buf,
                                            LOCAL_RES->Capabilities.Program_Size);
    }
    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(0, api_validation->R_buf,
                                       LOCAL_RES->Capabilities.Erase_Sector_Size);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR,
                   LOCAL_RES->Capabilities.Erase_Sector_Size)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 4 - Sector Erase ---------------*/
    debug_time = wd_time_get_us();
    wd_drv_block_device_nor_flash.Erase(API_VALIDATION_TEST_ADDR, 0xFF,
                                        LOCAL_RES->Capabilities.Erase_Sector_Size);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 4 - Sector Erase", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(
        API_VALIDATION_TEST_ADDR -
            (API_VALIDATION_TEST_ADDR % LOCAL_RES->Capabilities.Erase_Sector_Size),
        api_validation->R_buf, LOCAL_RES->Capabilities.Erase_Sector_Size);
    if (wd_memchrn(api_validation->R_buf, 0xFF, LOCAL_RES->Capabilities.Erase_Sector_Size)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    wd_drv_block_device_nor_flash.Erase(0, 0xFF, LOCAL_RES->Capabilities.Erase_Sector_Size);
    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(0, api_validation->R_buf,
                                       LOCAL_RES->Capabilities.Erase_Sector_Size);
    if (wd_memchrn(api_validation->R_buf, 0xFF, LOCAL_RES->Capabilities.Erase_Sector_Size)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 5 - Chip Write ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_ms();
    for (i = 0; i < LOCAL_RES->Capabilities.Size; i += LOCAL_RES->Capabilities.Program_Size) {
        for (j = 0; j < LOCAL_RES->Capabilities.Program_Size; j += 4) {
            data = (uint32_t)(api_validation->W_buf + j);
            *(uint32_t *)data = i + ((i / LOCAL_RES->Capabilities.Program_Size) + j);
        }
        wd_drv_block_device_nor_flash.Write(i, api_validation->W_buf,
                                            LOCAL_RES->Capabilities.Program_Size);
    }
    debug_time = wd_time_calc_ms(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [ms]" WD_COLOR_RESET "\r\n",
                  "Step 5 - Chip Write", debug_time);


    for (i = 0; i < LOCAL_RES->Capabilities.Size; i += LOCAL_RES->Capabilities.Program_Size) {
        for (j = 0; j < LOCAL_RES->Capabilities.Program_Size; j += 4) {
            data = (uint32_t)(api_validation->W_buf + j);
            *(uint32_t *)data = i + ((i / LOCAL_RES->Capabilities.Program_Size) + j);
        }
        memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
        wd_drv_block_device_nor_flash.Read(i, api_validation->R_buf,
                                           LOCAL_RES->Capabilities.Program_Size);
        if (memcmp(api_validation->R_buf, api_validation->W_buf,
                   LOCAL_RES->Capabilities.Program_Size)) {
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
    }

    /*--------------- Step 6 - Chip Erase ---------------*/
    debug_time = wd_time_get_ms();
    QSPI_Chip_Erase(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE);
    debug_time = wd_time_calc_ms(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [ms]" WD_COLOR_RESET "\r\n",
                  "Step 6 - Chip Erase", debug_time);

    for (i = 0; i < LOCAL_RES->Capabilities.Size; i += LOCAL_RES->Capabilities.Erase_Sector_Size) {
        memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
        wd_drv_block_device_nor_flash.Read(i, api_validation->R_buf,
                                           LOCAL_RES->Capabilities.Erase_Sector_Size);
        if (wd_memchrn(api_validation->R_buf, 0xFF, LOCAL_RES->Capabilities.Erase_Sector_Size)) {
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
    }

    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
    WD_PRINT_INFO(WD_MSG_TITLE_CRLF, "[MX66L2G45G API Validation]");
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);

    /*--------------- Step 1 - Byte Write (erased) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_drv_block_device->Write(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                               api_validation->W_buf, 1, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 1 - Byte Write (erased)", debug_time);

    /*--------------- Step 2 - Byte Read ---------------*/
    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    debug_time = wd_time_get_us();
    wd_drv_block_device->Read(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                              api_validation->R_buf, 1, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 2 - Byte Read", debug_time);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 3 - Byte Write (same) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_drv_block_device->Write(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                               api_validation->W_buf, 1, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 3 - Byte Write (same)", debug_time);

    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(API_VALIDATION_TEST_ADDR, api_validation->R_buf, 1);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 4 - Byte Write (different) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR + 1, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_drv_block_device->Write(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                               api_validation->W_buf, 1, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 3 - Byte Write (same)", debug_time);

    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(API_VALIDATION_TEST_ADDR, api_validation->R_buf, 1);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR + 1, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    memset(api_validation->W_buf, 0, sizeof(api_validation->W_buf));
    wd_drv_block_device_nor_flash.Write(LOCAL_RES->Capabilities.Erase_Sector_Size - 1,
                                        api_validation->W_buf, 1);
    wd_drv_block_device_nor_flash.Write(LOCAL_RES->Capabilities.Erase_Sector_Size,
                                        api_validation->W_buf, 1);

    /*--------------- Step 5 - 2 Byte Write [1 Page End][2 Page Start] (2 byte all different) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR + 2, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_drv_block_device->Write(&wd_drv_block_device_nor_flash,
                               (void *)(LOCAL_RES->Capabilities.Erase_Sector_Size - 1),
                               api_validation->W_buf, 2, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 5 - 2 Byte Write [1 Page End][2 Page Start] (2 byte all different)",
                  debug_time);

    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(LOCAL_RES->Capabilities.Erase_Sector_Size - 1,
                                       api_validation->R_buf, 2);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR + 2, 2)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    wd_drv_block_device_nor_flash.Erase(API_VALIDATION_TEST_ADDR, 0xFF,
                                        LOCAL_RES->Capabilities.Erase_Sector_Size);

    /*--------------- Step 6 - Page Write (erased) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_drv_block_device->Write(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                               api_validation->W_buf, LOCAL_RES->Capabilities.Program_Size, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 6 - Page Write (erased)", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(API_VALIDATION_TEST_ADDR, api_validation->R_buf,
                                       LOCAL_RES->Capabilities.Program_Size);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR,
                   LOCAL_RES->Capabilities.Program_Size)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 7 - Page Write (different) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR & 0x7C, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_drv_block_device->Write(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                               api_validation->W_buf, LOCAL_RES->Capabilities.Program_Size, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 7 - Page Write (different)", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(API_VALIDATION_TEST_ADDR, api_validation->R_buf,
                                       LOCAL_RES->Capabilities.Program_Size);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR & 0x7C,
                   LOCAL_RES->Capabilities.Program_Size)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    wd_drv_block_device_nor_flash.Erase(API_VALIDATION_TEST_ADDR, 0xFF,
                                        LOCAL_RES->Capabilities.Erase_Sector_Size);

    /*--------------- Step 8 - Byte Erase 0x00 [1 Page End] ---------------*/
    debug_time = wd_time_get_us();
    wd_drv_block_device->Erase(&wd_drv_block_device_nor_flash,
                               (void *)(LOCAL_RES->Capabilities.Program_Size - 1), 0x00, 1, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 8 - Byte Erase 0x00 [1 Page End]", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(LOCAL_RES->Capabilities.Program_Size - 1,
                                       api_validation->R_buf, 1);
    if (wd_memchrn(api_validation->R_buf, 0x00, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 9 - Byte Erase 0xA5 [1 Page End] ---------------*/
    debug_time = wd_time_get_us();
    wd_drv_block_device->Erase(&wd_drv_block_device_nor_flash,
                               (void *)(LOCAL_RES->Capabilities.Program_Size - 1), 0xA5, 1, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 9 - Byte Erase 0xA5 [1 Page End]", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(LOCAL_RES->Capabilities.Program_Size - 1,
                                       api_validation->R_buf, 1);
    if (wd_memchrn(api_validation->R_buf, 0xA5, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 10 - 2 Byte Erase 0x5A [1 Page End][2 Page Start] ---------------*/
    debug_time = wd_time_get_us();
    wd_drv_block_device->Erase(&wd_drv_block_device_nor_flash,
                               (void *)(LOCAL_RES->Capabilities.Program_Size - 1), 0x5A, 2, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 10 - 2 Byte Erase 0x5A [1 Page End][2 Page Start]", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(LOCAL_RES->Capabilities.Program_Size - 1,
                                       api_validation->R_buf, 2);
    if (wd_memchrn(api_validation->R_buf, 0x5A, 2)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 11 - Sector Erase 0xC3 ---------------*/
    debug_time = wd_time_get_us();
    wd_drv_block_device->Erase(
        &wd_drv_block_device_nor_flash,
        (void *)(API_VALIDATION_TEST_ADDR -
                 (API_VALIDATION_TEST_ADDR % LOCAL_RES->Capabilities.Erase_Sector_Size)),
        0xC3, LOCAL_RES->Capabilities.Erase_Sector_Size, NULL);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 11 - Sector Erase 0xC3", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_drv_block_device_nor_flash.Read(
        API_VALIDATION_TEST_ADDR -
            (API_VALIDATION_TEST_ADDR % LOCAL_RES->Capabilities.Erase_Sector_Size),
        api_validation->R_buf, LOCAL_RES->Capabilities.Erase_Sector_Size);
    if (wd_memchrn(api_validation->R_buf, 0xC3, LOCAL_RES->Capabilities.Erase_Sector_Size)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 12 - Write Test (shift byte) ---------------*/
    wd_drv_block_device->Erase(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                               0x81, sizeof(api_validation->A_buf), NULL);
    memset(api_validation->A_buf, 0, sizeof(api_validation->A_buf));
    wd_drv_block_device_nor_flash.Read(API_VALIDATION_TEST_ADDR, api_validation->A_buf,
                                       sizeof(api_validation->A_buf));
    if (wd_memchrn(api_validation->A_buf, 0x81, sizeof(api_validation->A_buf))) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    for (i = API_VALIDATION_TEST_ADDR + LOCAL_RES->Capabilities.Erase_Sector_Size - 250;
         i < API_VALIDATION_TEST_ADDR + LOCAL_RES->Capabilities.Erase_Sector_Size + 250; i += 25) {
        WD_PRINT_INFO(
            WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "%3u / %3u" WD_COLOR_RESET "\r\n",
            "Step 12 - Write Test (shift byte)",
            ((i - (API_VALIDATION_TEST_ADDR + LOCAL_RES->Capabilities.Erase_Sector_Size - 250)) /
             25) +
                1,
            500 / 25);
        for (j = 0; j < LOCAL_RES->Capabilities.Erase_Sector_Size; j += 4) {
            data = (uint32_t)(api_validation->W_buf + j);
            *(uint32_t *)data = ((i * 1000) + j) + wd_time_get_us();
        }
        wd_drv_block_device->Write(&wd_drv_block_device_nor_flash, (void *)i, api_validation->W_buf,
                                   LOCAL_RES->Capabilities.Erase_Sector_Size, NULL);
        memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
        wd_drv_block_device->Read(&wd_drv_block_device_nor_flash, (void *)i, api_validation->R_buf,
                                  LOCAL_RES->Capabilities.Erase_Sector_Size, NULL);
        if (memcmp(api_validation->R_buf, api_validation->W_buf,
                   LOCAL_RES->Capabilities.Erase_Sector_Size)) {
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
        memcpy(api_validation->A_buf + i - API_VALIDATION_TEST_ADDR, api_validation->W_buf,
               LOCAL_RES->Capabilities.Erase_Sector_Size);
        wd_drv_block_device->Read(&wd_drv_block_device_nor_flash, (void *)API_VALIDATION_TEST_ADDR,
                                  api_validation->R_buf, sizeof(api_validation->R_buf), NULL);
        if (memcmp(api_validation->R_buf, api_validation->A_buf, sizeof(api_validation->R_buf))) {
            {
                int t;
                for (t = 0; t < sizeof(api_validation->R_buf); t++) {
                    if (api_validation->R_buf[t] != api_validation->A_buf[t]) {
                        __NOP();
                    }
                }
            }
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
    }

    /*--------------- Validation Finalize ---------------*/
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
    debug_time = wd_time_get_ms();
    QSPI_Chip_Erase(RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE);
    debug_time = wd_time_calc_ms(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [ms]" WD_COLOR_RESET "\r\n",
                  "[MX66L2G45G Validation Finalize] - Chip Erase", debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
#endif
}

wd_blockdev_instance_info_t *const wd_nor_res_block_device_nor_flash = &Block_Device_NOR_Flash_Info;

wd_blockdev_instance_t wd_drv_block_device_nor_flash = {
    .Initialize = block_device_nor_flash_initialize,
    .Uninitialize = block_device_nor_flash_uninitialize,
    .PowerControl = block_device_nor_flash_power_control,
    .Write = block_device_nor_flash_write,
    .Read = block_device_nor_flash_read,
    .Erase = block_device_nor_flash_erase,
    .Control = block_device_nor_flash_control,
    .Get_Status = block_device_nor_flash_get_status,
    .Validation = block_device_nor_flash_validation,
    .Res = LOCAL_RES,
};

#ifndef NDEBUG
__USED wd_nor_debug_block_device_nor_flash_t wd_nor_dbg_block_device_nor_flash = {
    .Resource = LOCAL_RES,
    .Info = wd_nor_res_block_device_nor_flash,
    .Driver = LOCAL_DRV,
};
#endif

#endif //#if (RTE_COMPONENT_BLOCK_DEVICE_NOR_FLASH_MX66L2G45G)

#endif // #if 1 // 토글
