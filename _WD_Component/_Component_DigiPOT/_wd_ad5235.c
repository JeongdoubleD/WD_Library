/* SPDX-License-Identifier: MIT AND BSD-3-Clause
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright 2013(c) Analog Devices, Inc. (no-OS ad525x driver, BSD-3-Clause)
 */
/*
 * _wd_ad5235.c
 *
 *  Created on: Jan 17, 2025
 *      Author: doubled
 */

#include "_wd_ad5235.h"

#if 1 // 토글
#if RTE_COMPONENT_DIGIPOT_AD5235

#define SPI_BAUDRATE_DIGIPOT 200000

/***************************************************************************************************************************************************************
 **********  Soft SPI AD5235
 **************************************************************************************************************************************************************/
static wd_softspi_info_t Driver_SPI_Soft_AD5235_Info = {
    0,
};
static wd_softspi_resources_t Driver_SPI_Soft_AD5235_Resources = {
    .This = (void *)&Driver_SPI_Soft_AD5235_Resources,
    .Name = "AD5235",
    .Info = &Driver_SPI_Soft_AD5235_Info,
    .IO =
        {
            .CS = RTE_PORT_SW_SPI_DIGIPOT_CS,
            .CLK = RTE_PORT_SW_SPI_SCLK,
            .MOSI = RTE_PORT_SW_SPI_MOSI,
            .MISO = RTE_PORT_SW_SPI_MISO,
        },
    .Baudrate = 20000000,
    .Mode = WD_DRIVER_SPI_SOFT_MODE_3,
};

/***************************************************************************************************************************************************************
 **********  Driver
 **************************************************************************************************************************************************************/
static wd_ad5235_info_t DigiPOT_AD5235_Info = {
    0,
};

#if (RTE_RTOS)
static osStaticMutexDef_t Mutex_DigiPOT_AD5235_cb_mem;
static osMutexId_t Mutex_DigiPOT_AD5235;
static const osMutexAttr_t Mutex_DigiPOT_AD5235_Attr = {
    .name = "Mutex DigiPOT AD5235",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = &Mutex_DigiPOT_AD5235_cb_mem,
    .cb_size = sizeof(Mutex_DigiPOT_AD5235_cb_mem)};

static int32_t res_start(void) {
#if (RTE_RTOS)
    Mutex_DigiPOT_AD5235 = osMutexNew(&Mutex_DigiPOT_AD5235_Attr);
    if (Mutex_DigiPOT_AD5235 == NULL) {
#ifndef NDEBUG
        WD_LOG_ERROR("Mutex DigiPOT AD5235 New Error");
#endif
        return (WD_CODE_ERROR);
    }
#endif

    return (WD_CODE_OK);
}

static int32_t res_lock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        if (osMutexAcquire(Mutex_DigiPOT_AD5235, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
            return (WD_CODE_ERROR_TIMEOUT);
        }
    }

    return (WD_CODE_OK);
}

static int32_t res_unlock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(Mutex_DigiPOT_AD5235);
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

static wd_ad5235_resources_t DigiPOT_AD5235_Resources = {
    .Capabilities =
        {
            .BaudRate = SPI_BAUDRATE_DIGIPOT,
        },
    .Info = &DigiPOT_AD5235_Info,
    .IO =
        {
            .CS = RTE_PORT_SW_SPI_DIGIPOT_CS,
            .CLK = RTE_PORT_SW_SPI_SCLK,
            .MOSI = RTE_PORT_SW_SPI_MOSI,
            .MISO = RTE_PORT_SW_SPI_MISO,
        },
    .Drv = &Driver_SPI_Soft_AD5235_Resources,
    .Handle = NULL,
    .Reg = NULL,
    .Start = res_start,
    .Lock = res_lock,
    .Unlock = res_unlock,
};

#undef LOCAL_DRV
#define LOCAL_DRV (&wd_drv_digi_pot_ad5235)
__USED static wd_ad5235_resources_t *const LOCAL_RES = &DigiPOT_AD5235_Resources;

__USED static void wd_digi_pot_ad5235_callback(uint32_t event) {
    LOCAL_RES->Info->Event = event;
    if (LOCAL_RES->Info->CB_Event) {
        LOCAL_RES->Info->CB_Event(event);
    }
}

static int32_t digi_pot_ad5235_initialize(wd_ad5235_signal_event_t cb_event) {
    if (LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "DigiPOT_AD5235 Constructor");

    memset(LOCAL_RES->Info, 0, sizeof(wd_ad5235_info_t));

    if (cb_event) {
        LOCAL_RES->Info->CB_Event = cb_event;
    }

    LOCAL_RES->Start();

    LOCAL_RES->Info->Flags = WDIF_FLAG_INITIALIZED;

    return (WD_CODE_OK);
}

static int32_t digi_pot_ad5235_uninitialize(void) {
    return (WD_CODE_OK);
}

static int32_t digi_pot_ad5235_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "DigiPOT_AD5235 PowerControl:WD_POWER_OFF");

        LOCAL_RES->Info->Flags &= ~(WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
    } break;

    case WD_POWER_FULL: {
        if ((LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) == 0U) {
            return (WD_CODE_ERROR);
        } else if (LOCAL_RES->Info->Flags & WDIF_FLAG_POWERED) {
            return (WD_CODE_OK);
        }

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "DigiPOT_AD5235 PowerControl:WD_POWER_FULL");

        wd_ad5235_init();


        LOCAL_RES->Info->Flags |= (WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
    } break;

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);
    default:
        break;
    }

    return (WD_CODE_OK);
}

static int32_t digi_pot_ad5235_control(uint32_t control, uint32_t arg) {
    UNUSED(control);
    UNUSED(arg);

    switch (control & WD_DIGIPOT_AD5235_CONTROL_MSK) {
    case WD_DIGIPOT_AD5235_CONFIG_CH_1:
    case WD_DIGIPOT_AD5235_CONFIG_CH_2: {
        uint16_t rdac;
        uint32_t ch = (control == WD_DIGIPOT_AD5235_CONFIG_CH_1) ? 0 : 1;

        wd_ad5235_write(ch, arg);
        rdac = wd_ad5235_read(ch);

        if (rdac != arg) {
            WD_LOG_ERROR();
            return (WD_CODE_ERROR);
        }
    } break;

    case WD_DIGIPOT_AD5235_CALIBRATION: {
    } break;
    default:
        break;
    }

    return (WD_CODE_OK);
}

static wd_ad5235_status_t digi_pot_ad5235_get_status(void) {
    wd_ad5235_status_t status = {0};

    return (status);
}


static void digi_pot_ad5235_validation(void) {
#if (API_VALIDATION)

#endif
}
wd_ad5235_info_t *const wd_ad5235_res_digi_pot_ad5235_info = &DigiPOT_AD5235_Info;

wd_ad5235_resources_t *const wd_ad5235_res_digi_pot_ad5235 = &DigiPOT_AD5235_Resources;

wd_ad5235_driver_digipot_ad5235_t wd_drv_digi_pot_ad5235 = {
    .Initialize = digi_pot_ad5235_initialize,
    .Uninitialize = digi_pot_ad5235_uninitialize,
    .PowerControl = digi_pot_ad5235_power_control,
    .Control = digi_pot_ad5235_control,
    .Get_Status = digi_pot_ad5235_get_status,
    .Validation = digi_pot_ad5235_validation,
};

#ifndef NDEBUG
__USED wd_ad5235_debug_digipot_ad5235_t wd_ad5235_dbg_digi_pot_ad5235 = {
    .Resource = LOCAL_RES,
    .Info = wd_ad5235_res_digi_pot_ad5235_info,
    .Driver = LOCAL_DRV,
};
#endif

/***************************************************************************************************************************************************************
 **********  AD5235 (https://github.com/analogdevicesinc/no-OS/blob/main/drivers/potentiometer/ad525x/ad525x.h)
 **************************************************************************************************************************************************************/
/*****************************************************************************/
/*  Device specific MACROs                                                   */
/*****************************************************************************/
#define NO_OS_IS_ERR_VALUE(x) ((x) < 0)
#define NO_OS_BIT(x)          (1 << (x))
#define NO_OS_BITS_PER_LONG   32
#define NO_OS_GENMASK(h, l)                           \
    ({                                                \
        uint32_t t = (uint32_t)(~0UL);                \
        t = t << (NO_OS_BITS_PER_LONG - (h - l + 1)); \
        t = t >> (NO_OS_BITS_PER_LONG - (h + 1));     \
        t;                                            \
    })

/* Data word masks */
#define AD525X_MEM_ADDR_MASK       0xF
#define AD525X_RDAC_ADDR_MASK_1BIT 0x1
#define AD525X_RDAC_ADDR_MASK_3BIT 0x7
#define AD525X_DATA8_MASK          0xFF
#define AD525X_DATA10_MASK         0x3FF

/*********************** Command definitions *********************************/
#define AD525X_CMD_NOP            0x0
#define AD525X_CMD_MEM2RDAC       0x1
#define AD525X_CMD_RDAC2MEM       0x2
#define AD525X_CMD_DECRDAC_6DB    0x3
#define AD525X_CMD_DECALLRDAC_6DB 0x4
#define AD525X_CMD_DECRDAC_ONE    0x5
#define AD525X_CMD_DECALLRDAC_ONE 0x6
#define AD525X_CMD_RESET          0x7
#define AD525X_CMD_INCRDAC_6DB    0x8
#define AD525X_CMD_INCALLRDAC_6DB 0x9
#define AD525X_CMD_INCRDAC_ONE    0xA
#define AD525X_CMD_INCALLRDAC_ONE 0xB
/*****************************************************************************/

/**************************** SPI specific macros ****************************/
#define NO_OS_SPI_CPHA 0x01
#define NO_OS_SPI_CPOL 0x02

#define AD525X_CMD_SPI_OFFSET 0x4
#define AD525X_CMD_MASK       0xF
/* SPI Read/Write commands */
#define AD525X_CMD_SPI_SREG2MEM  0x3
#define AD525X_CMD_SPI_MEM2SREG  0x9 // Result in the next frame
#define AD525X_CMD_SPI_RDAC2SREG 0xA // Result in the next frame
#define AD525X_CMD_SPI_SREG2RDAC 0xB

/*************************** I2C specific macros *****************************/
/* I2C device address */
#define AD525X_I2C_HARD_ADDR 0x2C
/* Package pin-programmable address bits */
#define AD525X_I2C_PIN_ADDR_MASK 0x03
/* Mask for I2C Control Bits */
#define AD525X_I2C_CNTR_MASK 0xBF
/* Access Types */
#define AD525X_I2C_CMD_OR_REG 0x80
#define AD525X_I2C_EE_OR_RDAC 0x20
/* Address mask for EEMEM addresses, one bit wider than at devices with SPI */
#define AD525X_I2C_MEM_ADDR_MASK 0x1F
/* The offset of the Command in the I2C word */
#define AD525X_CMD_I2C_OFFSET 0x3

/*************************** Reading tolerance addresses *********************/
/* Note: The valid tolerance addresses varies on different devices. Check the data sheet for further information */
#define AD525x_RDAC_TOLERANCE    0x0F
#define AD525X_RDAC0_SIGN_TOL    0x18
#define AD525X_RDAC0_DECIMAL_TOL 0x19
#define AD525X_RDAC1_SIGN_TOL    0x1A
#define AD525X_RDAC1_DECIMAL_TOL 0x1B
#define AD525X_RDAC2_SIGN_TOL    0x1C
#define AD525X_RDAC2_DECIMAL_TOL 0x1D
#define AD525X_RDAC3_SIGN_TOL    0x1E
#define AD525X_RDAC4_DECIMAL_TOL 0x1F

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/**
 * @enum no_os_spi_mode
 * @brief SPI configuration for clock phase and polarity.
 */
typedef enum no_os_spi_mode {
    /** Data on rising, shift out on falling */
    NO_OS_SPI_MODE_0 = (0 | 0),
    /** Data on falling, shift out on rising */
    NO_OS_SPI_MODE_1 = (0 | NO_OS_SPI_CPHA),
    /** Data on falling, shift out on rising */
    NO_OS_SPI_MODE_2 = (NO_OS_SPI_CPOL | 0),
    /** Data on rising, shift out on falling */
    NO_OS_SPI_MODE_3 = (NO_OS_SPI_CPOL | NO_OS_SPI_CPHA)
} no_os_spi_mode;

/**
 * @enum no_os_spi_bit_order
 * @brief SPI configuration for bit order (MSB/LSB).
 */
typedef enum no_os_spi_bit_order {
    /** Most-significant bit (MSB) first */
    NO_OS_SPI_BIT_ORDER_MSB_FIRST = 0,
    /** Least-significant bit (LSB) first */
    NO_OS_SPI_BIT_ORDER_LSB_FIRST = 1,
} no_os_spi_bit_order;

/**
 * @struct no_os_spi_msg_list
 * @brief List item describing a SPI transfer
 */
struct no_os_spi_msg {
    /** Buffer with data to send. If NULL, 0x00 will be sent */
    uint8_t *tx_buff;
    /** Buffer where to store data. If NULL, incoming data won't be saved */
    uint8_t *rx_buff;
    /** Length of buffers. Must have equal size. */
    uint32_t bytes_number;
    /** If set, CS will be deasserted after the transfer */
    uint8_t cs_change;
};

/**
 * @struct no_os_spi_platform_ops
 * @brief Structure holding SPI function pointers that point to the platform
 * specific function
 */
struct no_os_spi_platform_ops;

/**
 * @struct no_os_spi_init_param
 * @brief Structure holding the parameters for SPI initialization
 */
typedef struct no_os_spi_init_param {
    /** Device ID */
    uint32_t device_id;
    /** maximum transfer speed */
    uint32_t max_speed_hz;
    /** SPI chip select */
    uint8_t chip_select;
    /** SPI mode */
    enum no_os_spi_mode mode;
    /** SPI bit order */
    enum no_os_spi_bit_order bit_order;
    const struct no_os_spi_platform_ops *platform_ops;
    /**  SPI extra parameters (device specific) */
    void *extra;
} no_os_spi_init_param;

/**
 * @struct no_os_spi_desc
 * @brief Structure holding SPI descriptor.
 */
typedef struct no_os_spi_desc {
    /** Device ID */
    uint32_t device_id;
    /** maximum transfer speed */
    uint32_t max_speed_hz;
    /** SPI chip select */
    uint8_t chip_select;
    /** SPI mode */
    enum no_os_spi_mode mode;
    /** SPI bit order */
    enum no_os_spi_bit_order bit_order;
    const struct no_os_spi_platform_ops *platform_ops;
    /**  SPI extra parameters (device specific) */
    void *extra;
} no_os_spi_desc;

/**
 * @struct no_os_spi_platform_ops
 * @brief Structure holding SPI function pointers that point to the platformfe
 * specific function
 */
struct no_os_spi_platform_ops {
    /** SPI initialization function pointer */
    int32_t (*init)(struct no_os_spi_desc **, const struct no_os_spi_init_param *);
    /** SPI write/read function pointer */
    int32_t (*write_and_read)(struct no_os_spi_desc *, uint8_t *, uint16_t);
    /** Iterate over the spi_msg array and send all messages at once */
    int32_t (*transfer)(struct no_os_spi_desc *, struct no_os_spi_msg *, uint32_t);
    /** SPI remove function pointer */
    int32_t (*remove)(struct no_os_spi_desc *);
};

/* Supported devices */
enum ad525_x_type_t {
    ID_AD5235,
};

/* Communication types */
enum comm_type_t { SPI, I2C };

struct ad525x_chip_info {
    uint8_t num_channels;
    enum comm_type_t comm_type;
    uint16_t num_position;
};

struct ad525x_dev {
    /* SPI */
    no_os_spi_desc *spi_desc;
    /* GPIO */
    struct no_os_gpio_desc *gpio_reset;
    struct no_os_gpio_desc *gpio_shutdown;
    struct no_os_gpio_desc *gpio_ready;
    struct no_os_gpio_desc *gpio_wpbf;
    /* Device Settings */
    enum ad525_x_type_t this_device;
};

struct ad525x_init_param {
    /* SPI */
    no_os_spi_init_param spi_init;
    /* GPIO */
    uint32_t gpio_reset;
    uint32_t gpio_shutdown;
    uint32_t gpio_ready;
    uint32_t gpio_wpbf;
    /* Device Settings */
    enum ad525_x_type_t this_device;
};


#define MSB_BYTE_MASK  0xFF00
#define LSB_BYTE_MASK  0x00FF
#define ONEBYTE_OFFSET 0x8

static struct ad525x_dev ad525x_dev_instance = {
    0,
};
static struct no_os_spi_desc spi_desc_instance = {
    0,
};

/*****************************************************************************/
/***************************** Constant definition ***************************/
/*****************************************************************************/
static const struct ad525x_chip_info chip_info = {
    .num_channels = 2,
    .comm_type = SPI,
    .num_position = 1024,
};

static int32_t generic_spi_init(struct no_os_spi_desc **desc,
                                const struct no_os_spi_init_param *param) {
    no_os_spi_desc *descriptor = &spi_desc_instance;

    wd_softspi_power_control(LOCAL_RES->Drv, WD_POWER_OFF);
    WD_CHECK(wd_softspi_initialize(LOCAL_RES->Drv, wd_digi_pot_ad5235_callback));
    WD_CHECK(wd_softspi_power_control(LOCAL_RES->Drv, WD_POWER_FULL));
    WD_CHECK(wd_softspi_control(LOCAL_RES->Drv,
                                WD_SPI_SOFT_MODE_MASTER | WD_SPI_SOFT_DATA_BITS(8) |
                                    WD_SPI_SOFT_CPOL1_CPHA1 | WD_SPI_SOFT_MSB_LSB |
                                    WD_SPI_SOFT_SS_MASTER_HW_OUTPUT,
                                param->max_speed_hz));
    WD_CHECK(wd_softspi_control(LOCAL_RES->Drv, WD_SPI_SOFT_SET_DEFAULT_TX_VALUE, 0xCC));

    descriptor->device_id = param->device_id;
    descriptor->max_speed_hz = param->max_speed_hz;
    descriptor->chip_select = param->chip_select;
    descriptor->mode = param->mode;
    descriptor->bit_order = param->bit_order;

    *desc = descriptor;

    return 0;
}

/**
 * @brief Free the resources allocated by no_os_spi_init().
 * @param desc - The SPI descriptor.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t generic_spi_remove(struct no_os_spi_desc *desc) {
    UNUSED(desc);
    return 0;
}

/**
 * @brief Write and read data to/from SPI.
 * @param desc - The SPI descriptor.
 * @param data - The buffer with the transmitted/received data.
 * @param bytes_number - Number of bytes to write/read.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t generic_spi_write_and_read(struct no_os_spi_desc *desc, uint8_t *data,
                                          uint16_t bytes_number) {
    int32_t ret = WD_CODE_OK;

    static uint8_t generic_spi_write_and_read_tx_buf[16];

    if (bytes_number >= sizeof(generic_spi_write_and_read_tx_buf)) {
        bytes_number = 2;
        WD_LOG_CRITICAL();
    }

    memcpy(generic_spi_write_and_read_tx_buf, data, bytes_number);

    UNUSED(desc);
#if _CONF_CONSOLE_LEVEL_LOCAL_VV >= 4
    WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "DigiPOT_AD5235 SPI", WD_MSG_MAGIC, WD_NET_TX,
                   generic_spi_write_and_read_tx_buf, bytes_number);
    wd_time_wait_us(1); // DUMP_LOG 시 깨짐 발생_이건 USB 콘솔문제인거 같음.
#endif
    LOCAL_RES->Info->Event = 0;
    if (wd_softspi_transfer(LOCAL_RES->Drv, generic_spi_write_and_read_tx_buf, data,
                            bytes_number) != WD_CODE_OK) {
        WD_LOG_ERROR();
        ret = WD_CODE_ERROR;
    }
//  WD_WAIT_WHILE((LOCAL_RES->Info->Event & ARM_SPI_EVENT_TRANSFER_COMPLETE) != ARM_SPI_EVENT_TRANSFER_COMPLETE, 100000);
#if _CONF_CONSOLE_LEVEL_LOCAL_VV >= 4
    WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "DigiPOT_AD5235 SPI", WD_MSG_MAGIC, WD_NET_RX, data,
                   bytes_number);
    wd_time_wait_us(1); // DUMP_LOG 시 깨짐 발생_이건 USB 콘솔문제인거 같음.
#endif

    return ret;
}

/**
 * @brief Generic platform SPI ops
 */
static const struct no_os_spi_platform_ops generic_spi_ops = {.init = &generic_spi_init,
                                                              .write_and_read =
                                                                  &generic_spi_write_and_read,
                                                              .remove = &generic_spi_remove};

/***************************************************************************************************************************************************************
 **********  no_os_spi.c (Implementation of the SPI Interface)
 **************************************************************************************************************************************************************/
/**
 * @brief Initialize the SPI communication peripheral.
 * @param desc - The SPI descriptor.
 * @param param - The structure that contains the SPI parameters.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t no_os_spi_init(struct no_os_spi_desc **desc,
                              const struct no_os_spi_init_param *param) {
    if (!param) {
        return -1;
    }

    if ((param->platform_ops->init(desc, param))) {
        return -1;
    }

    (*desc)->platform_ops = param->platform_ops;

    return 0;
}

/**
 * @brief Free the resources allocated by no_os_spi_init().
 * @param desc - The SPI descriptor.
 * @return 0 in case of success, -1 otherwise.
 */
__USED static int32_t no_os_spi_remove(struct no_os_spi_desc *desc) {
    return desc->platform_ops->remove(desc);
}

/**
 * @brief Write and read data to/from SPI.
 * @param desc - The SPI descriptor.
 * @param data - The buffer with the transmitted/received data.
 * @param bytes_number - Number of bytes to write/read.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t no_os_spi_write_and_read(struct no_os_spi_desc *desc, uint8_t *data,
                                        uint16_t bytes_number) {
    return desc->platform_ops->write_and_read(desc, data, bytes_number);
}

#if 0
/**
 * @brief  Iterate over head list and send all spi messages
 * @param desc - The SPI descriptor.
 * @param msgs - Array of messages.
 * @param len - Number of messages in the array.
 * @return 0 in case of success, negativ error code otherwise.
 */
static int32_t no_os_spi_transfer(struct no_os_spi_desc *desc,
         struct no_os_spi_msg *msgs,
         uint32_t len)
{
  int32_t  ret;
  uint32_t i;

  if (!desc || !desc->platform_ops)
    return -EINVAL;

  if (desc->platform_ops->transfer)
    return desc->platform_ops->transfer(desc, msgs, len);

  for (i = 0; i < len; i++) {
    if (msgs[i].rx_buff != msgs[i].tx_buff || !msgs[i].tx_buff)
      return -EINVAL;
    ret = no_os_spi_write_and_read(desc, msgs[i].rx_buff,
                 (uint16_t)msgs[i].bytes_number);
    if (NO_OS_IS_ERR_VALUE(ret))
      return ret;
  }

  return 0;
}
#endif

/*****************************************************************************/
/*************************** Functions definitions ***************************/
/*****************************************************************************/

/**************************************************************************/ /**
 * @brief Initialize SPI and Initial Values for AD5172 Board.
 *
 * @param device     - The device structure.
 * @param init_param - The structure that contains the device initial
 *             parameters.
 *
 * @return success
******************************************************************************/
static int8_t wd_ad5235_init_ad525x(struct ad525x_dev **device,
                                    struct ad525x_init_param init_param) {
    struct ad525x_dev *dev;
    int8_t status;

    dev = &ad525x_dev_instance;

    dev->this_device = init_param.this_device;

    if (chip_info.comm_type == SPI) {
        /* CPHA = 0; CPOL = 0; */
        status = (int8_t)no_os_spi_init(&dev->spi_desc, &init_param.spi_init);
    } else {
        //    status = no_os_i2c_init(&dev->i2c_desc, &init_param.i2c_init);
    }

    /* Deactivate Hardware Reset */
    /* Deactivate Shutdown Pin  */
    /* Deactivate Write Protect Pin */
    /* Setup the RDY input */

    *device = dev;

    return status;
}

/***************************************************************************/ /**
 * @brief Free the resources allocated by wd_ad5235_init_ad525x().
 *
 * @param dev - The device structure.
 *
 * @return ret - The result of the remove procedure.
*******************************************************************************/

/**************************************************************************/ /**
 * @brief Read data from the EEMEM
 *
 * @param dev     - The device structure.
 * @param address - desired address of the EEMEM memory
 *
 *
 * @return success
******************************************************************************/

/**************************************************************************/ /**
 * @brief Write data to EEMEM.
 *
 * @param dev     - The device structure.
 * @param address - desired address of the EEMEM memory
 * @param data    - the data which will be written to the memory
 *
 * @return success
******************************************************************************/

/**************************************************************************/ /**
 * @brief Read data from the RDAC register
 *
 * @param dev     - The device structure.
 * @param address - desired address of the RDAC registers
 *
 *
 * @return success
******************************************************************************/
static uint16_t wd_ad5235_ad525x_read_rdac(struct ad525x_dev *dev, uint8_t address) {
    uint8_t data_buffer[3] = {
        0,
    };
    uint16_t data = 0;

    if (chip_info.comm_type == SPI) {
        /* Sending the command, reading the result on the next frame */
        data_buffer[0] |= AD525X_CMD_SPI_RDAC2SREG << AD525X_CMD_SPI_OFFSET;
        data_buffer[0] |= address & AD525X_MEM_ADDR_MASK;
        /* 3 byte data word */
        if ((dev->this_device == ID_AD5235) /*|| (dev->this_device == ID_ADN2850)*/) {
            no_os_spi_write_and_read(dev->spi_desc, data_buffer, 3);
            wd_time_wait_ms(50);
            data_buffer[0] &= AD525X_CMD_NOP << AD525X_CMD_SPI_OFFSET;
            no_os_spi_write_and_read(dev->spi_desc, data_buffer, 3);
            data = (data_buffer[1] << ONEBYTE_OFFSET) | data_buffer[2];
            return (data & AD525X_DATA10_MASK);
        } else { /* 2 byte data word */
            no_os_spi_write_and_read(dev->spi_desc, data_buffer, 2);
            wd_time_wait_ms(50);
            /* Sending a dummy frame to read the result */
            data_buffer[0] &= AD525X_CMD_NOP << AD525X_CMD_SPI_OFFSET;
            no_os_spi_write_and_read(dev->spi_desc, data_buffer, 2);
            data = (uint16_t)data_buffer[1];
            return data;
        }
    } else { /* Communication interface is I2C */
#if 0
    data_buffer[0] &= ~AD525X_I2C_CMD_OR_REG;         // reset CMD/REG_n
    data_buffer[0] &= ~AD525X_I2C_EE_OR_RDAC;         // reset EE/RDAC_n
    data_buffer[0] |= address & AD525X_MEM_ADDR_MASK; // set address
    /* Dummy write to select the desired register */
    no_os_i2c_write(dev->i2c_desc,
        data_buffer,
        1,
        1);
    data_buffer[0] &= AD525X_CMD_NOP;
    no_os_i2c_read(dev->i2c_desc,
             data_buffer,
             1,
             1);
    data = (uint16_t)data_buffer[0];
    return data;
#endif
    }
    wd_time_wait_ms(5);
    return data; /* I2C interface not implemented : 0 */
}

/**************************************************************************/ /**
 * @brief Write data to RDAC register.
 *
 * @param dev     - The device structure.
 * @param address - desired address of the RDAC register
 * @param data    - the data which will be written to the RDAC register
 *
 * @return success
******************************************************************************/
static void wd_ad5235_ad525x_write_rdac(struct ad525x_dev *dev, uint8_t address, uint16_t data) {
    uint8_t data_buffer[3] = {
        0,
    };

    if (chip_info.comm_type == SPI) {
        /* Sending the command, reading the result on the next frame */
        data_buffer[0] |= AD525X_CMD_SPI_SREG2RDAC << AD525X_CMD_SPI_OFFSET;
        data_buffer[0] |= address & AD525X_MEM_ADDR_MASK;
        if ((dev->this_device == ID_AD5235)
            /*|| (dev->this_device == ID_ADN2850)*/) { /* 3 byte data word */
            data_buffer[1] = (uint8_t)((data & MSB_BYTE_MASK) >> ONEBYTE_OFFSET);
            data_buffer[2] = (uint8_t)(data & LSB_BYTE_MASK);
            no_os_spi_write_and_read(dev->spi_desc, data_buffer, 3);
        } else { /* 2 byte data word */
            data_buffer[1] = (uint8_t)(data & LSB_BYTE_MASK);
            no_os_spi_write_and_read(dev->spi_desc, data_buffer, 2);
        }
    } else { /* Communication interface is I2C */
#if 0
    data_buffer[0] &= ~AD525X_I2C_CMD_OR_REG;         // reset CMD/REG_n
    data_buffer[0] &= ~AD525X_I2C_EE_OR_RDAC;         // reset EE/RDAC_n
    data_buffer[0] |= address & AD525X_MEM_ADDR_MASK; // set address

    data_buffer[1] = (data & LSB_BYTE_MASK);
    no_os_i2c_write(dev->i2c_desc,
        data_buffer,
        2,
        1);
#endif
    }
    wd_time_wait_ms(50);
}

/**************************************************************************/ /**
 * @brief Write quick commands to the device.
 *
 * @param dev     - The device structure.
 * @param command - desired command, the functionality of a command it may
 *                    vary at diffrent devices
 * @param address - Address to be written
 * @return success
******************************************************************************/
static void wd_ad5235_ad525x_write_command(struct ad525x_dev *dev, uint8_t command,
                                           uint8_t address) {
    uint8_t data_buffer[3] = {
        0,
    };

    if (chip_info.comm_type == SPI) {
        /* Sending the command, reading the result on the next frame */
        command &= AD525X_CMD_MASK;
        /* Command adjustment (because of the diffrence between command
    representations) */
        if ((command >= AD525X_CMD_DECRDAC_6DB) && (command <= AD525X_CMD_RESET)) {
            command += 1;
        } else if (command >= AD525X_CMD_INCRDAC_6DB) {
            command += 4;
        }
        data_buffer[0] |= command << AD525X_CMD_SPI_OFFSET;
        if (command == AD525X_CMD_RESET) {
            data_buffer[0] &= 0xF0;
        } else {
            data_buffer[0] |= (address & AD525X_RDAC_ADDR_MASK_1BIT);
        }

        if ((dev->this_device == ID_AD5235)
            /*|| (dev->this_device == ID_ADN2850)*/) { /* 3 byte data word */
            no_os_spi_write_and_read(dev->spi_desc, data_buffer, 3);
            if (command == AD525X_CMD_MEM2RDAC) {
                data_buffer[0] &= AD525X_CMD_NOP;
                no_os_spi_write_and_read(dev->spi_desc, data_buffer, 3);
            }
        } else { /* 2 byte data word */
            no_os_spi_write_and_read(dev->spi_desc, data_buffer, 2);
            if (command == AD525X_CMD_MEM2RDAC) {
                no_os_spi_write_and_read(dev->spi_desc, data_buffer, 2);
            }
        }
    } else { /* Communication interface is I2C */
#if 0
    data_buffer[0] |= AD525X_I2C_CMD_OR_REG;                // set CMD/REG_n
    data_buffer[0] |= (command & AD525X_CMD_MASK) << AD525X_CMD_I2C_OFFSET;
    data_buffer[0] |= address & AD525X_RDAC_ADDR_MASK_3BIT; // set address

    no_os_i2c_write(dev->i2c_desc,
        data_buffer,
        2,
        1);
    if(command == AD525X_CMD_MEM2RDAC) {
      data_buffer[0] &= AD525X_CMD_NOP;
      no_os_i2c_write(dev->i2c_desc,
          data_buffer,
          1,
          1);
    }
#endif
    }
    wd_time_wait_ms(50);
}

void wd_ad5235_init(void) {
    int32_t ret;
    struct ad525x_dev *ad525x_dev;
    struct ad525x_init_param ad525x_init_param;

    ad525x_init_param.spi_init.device_id = 0;
    ad525x_init_param.spi_init.max_speed_hz = SPI_BAUDRATE_DIGIPOT;
    ad525x_init_param.spi_init.chip_select = 0;
    ad525x_init_param.spi_init.mode = NO_OS_SPI_MODE_3;
    ad525x_init_param.spi_init.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST;
    ad525x_init_param.spi_init.platform_ops = &generic_spi_ops;
    ad525x_init_param.spi_init.extra = NULL;

    ad525x_init_param.gpio_reset = RTE_CONFIG_IDLE_PIN_NC;
    ad525x_init_param.gpio_shutdown = RTE_CONFIG_IDLE_PIN_NC;
    ad525x_init_param.gpio_ready = RTE_CONFIG_IDLE_PIN_NC;
    ad525x_init_param.gpio_wpbf = RTE_CONFIG_IDLE_PIN_NC;

    ad525x_init_param.this_device = ID_AD5235;

    ret = wd_ad5235_init_ad525x(&ad525x_dev, ad525x_init_param);
    if (ret != 0) {
        WD_LOG_ERROR();
    }

    {
        uint16_t rdac1_old, rdac2_old, rdac1, rdac2;

        rdac1_old = wd_ad5235_ad525x_read_rdac(&ad525x_dev_instance, 0);
        rdac2_old = wd_ad5235_ad525x_read_rdac(&ad525x_dev_instance, 1);
        wd_ad5235_ad525x_write_command(&ad525x_dev_instance, AD525X_CMD_MEM2RDAC, 0);
        wd_ad5235_ad525x_write_command(&ad525x_dev_instance, AD525X_CMD_MEM2RDAC, 1);
        rdac1 = wd_ad5235_ad525x_read_rdac(&ad525x_dev_instance, 0);
        rdac2 = wd_ad5235_ad525x_read_rdac(&ad525x_dev_instance, 1);

        LOCAL_RES->Info->RDAC1 = rdac1;
        LOCAL_RES->Info->RDAC2 = rdac2;
        WD_PRINT_DEBUG(
            WD_MSG_DEBUG_LOG,
            "DigiPOT_AD5235 Read RDAC1 [%u]    RDAC2 [%u]    →    RDAC1 [%u]    RDAC2 [%u]",
            rdac1_old, rdac2_old, rdac1, rdac2);
    }
}

void wd_ad5235_write(uint32_t ch, uint32_t value) {
    uint16_t rdac_old, rdac;

    if (ch > 1) {
        WD_LOG_ERROR();
        return;
    }

    if (value > 1023) {
        WD_LOG_ERROR();
        return;
    }

    rdac_old = wd_ad5235_ad525x_read_rdac(&ad525x_dev_instance, (uint8_t)ch);
    wd_ad5235_ad525x_write_rdac(&ad525x_dev_instance, (uint8_t)ch, (uint16_t)value);
    wd_ad5235_ad525x_write_command(&ad525x_dev_instance, AD525X_CMD_RDAC2MEM, (uint8_t)ch);
    wd_ad5235_ad525x_write_command(&ad525x_dev_instance, AD525X_CMD_MEM2RDAC, (uint8_t)ch);
    rdac = wd_ad5235_ad525x_read_rdac(&ad525x_dev_instance, (uint8_t)ch);

    WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "DigiPOT_AD5235 Read Ch [%u] - RDAC [%u]    →    RDAC [%u]",
                   ch, rdac_old, rdac);
}

uint16_t wd_ad5235_read(uint32_t ch) {
    uint16_t rdac;

    if (ch > 1) {
        WD_LOG_ERROR();
        return 0;
    }

    rdac = wd_ad5235_ad525x_read_rdac(&ad525x_dev_instance, (uint8_t)ch);

    WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "DigiPOT_AD5235 Read Ch [%u] - RDAC [%u]", ch, rdac);

    return (rdac);
}

#include <stdio.h>
#include <stdint.h>

int wd_ad5235_resistance_int(uint32_t rwa) {
    uint32_t rab = WD_MAX_VR_VALUE;

    // rwa: 입력된 저항값 (예: 5050)
    // rab: 전체 저항값 (예: 250000)

    if (rwa < 50 || rwa > rab + 50) {
        return -1; // 범위 밖
    }

    // (rwa - 50) * 1024 / rab
    uint32_t numerator = (rwa - 50) * 1024;
    uint32_t quotient = (numerator + rab / 2) / rab; // 반올림 적용

    int32_t dx = 1024 - (int32_t)quotient;

    // 범위 보정
    if (dx < 0) {
        dx = 0;
    }
    if (dx > 1023) {
        dx = 1023;
    }

    return dx;
}


#endif //#if RTE_COMPONENT_DIGIPOT_AD5235
#endif //#if 1 // 토글
