/* SPDX-License-Identifier: MIT AND BSD-3-Clause
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright 2020(c) Analog Devices, Inc. (no-OS ad713x driver, BSD-3-Clause)
 */
/*
 * _wd_ad4134.c
 *
 *  Created on: Jan 15, 2025
 *      Author: doubled
 */

#include "_wd_ad4134.h"

/* DMA remaining counters captured for debugging (watch in debugger) */
static volatile uint32_t s_debug_dma_remaining[4];

#if 1 // 토글

#if (RTE_COMPONENT_ADC_AD4134)

#if (RTE_DRV_VALIDATION)
#define API_VALIDATION 1
#endif

#define LOCAL_METUOD_NORMAL 1
#define LOCAL_METUOD_DMA    2
#define LOCAL_METUOD        LOCAL_METUOD_DMA

#define SPI_BAUDRATE_AD4134 10000000
#define WD_ADC_CH_V         WD_ADC_CH_1
#define WD_ADC_CH_I         WD_ADC_CH_2
#define WD_ADC_CH_VDC       WD_ADC_CH_3
#if WD_ADC_CH4_ENABLE
#define WD_ADC_CH_SP WD_ADC_CH_4
#endif

#define WD_ADC_AD4134_XFER_IGNORE_NUMBER 1

static wd_ad4134_resources_t *const LOCAL_RES;
static uint32_t adc_ad4134_get_max_cycle_by_frequency(fpu_t freq);

/***************************************************************************************************************************************************************
 **********  Soft SPI AD4134
 **************************************************************************************************************************************************************/
static wd_softspi_info_t Driver_SPI_Soft_AD4134_Info = {
    0,
};
static wd_softspi_resources_t Driver_SPI_Soft_AD4134_Resources = {
    .This = (void *)&Driver_SPI_Soft_AD4134_Resources,
    .Name = "AD4134",
    .Info = &Driver_SPI_Soft_AD4134_Info,
    .IO =
        {
            .CS = RTE_PORT_SW_SPI_ADC_CS,
            .CLK = RTE_PORT_SW_SPI_SCLK,
            .MOSI = RTE_PORT_SW_SPI_MOSI,
            .MISO = RTE_PORT_SW_SPI_MISO,
        },
    .Baudrate = 40000000,
    .Mode = WD_DRIVER_SPI_SOFT_MODE_2,
};


/***************************************************************************************************************************************************************
 **********  AD7134 (https://github.com/analogdevicesinc/no-OS/blob/master/drivers/adc/ad713x/ad713x.c)
 **************************************************************************************************************************************************************/
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wgnu-statement-expression-from-macro-expansion"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#define AD7134_1_SPI_CS       0
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

/*--------------- generic_spi.h ---------------*/
/**
 * @brief specific SPI platform ops structure
 */
//extern const struct no_os_spi_platform_ops generic_spi_ops;

/*--------------- no_os_spi.h ---------------*/
/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define NO_OS_SPI_CPHA 0x01
#define NO_OS_SPI_CPOL 0x02

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


/*--------------- ad713x.h (Header file for the ad713x Driver.) ---------------*/
/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
/*
 * AD713X registers definition
 */
#define AD713X_REG_INTERFACE_CONFIG_A       0x00
#define AD713X_REG_INTERFACE_CONFIG_B       0x01
#define AD713X_REG_DEVICE_CONFIG            0x02
#define AD713X_REG_CHIP_TYPE                0x03
#define AD713X_REG_PRODUCT_ID_LSB           0x04
#define AD713X_REG_PRODUCT_ID_MSB           0x05
#define AD713X_REG_CHIP_GRADE               0x06
#define AD713X_REG_CHIP_INDEX               0x07
#define AD713X_REG_SCTATCH_PAD              0x0A
#define AD713X_REG_SPI_REVISION             0x0B
#define AD713X_REG_VENDOR_ID_LSB            0x0C
#define AD713X_REG_VENDOR_ID_MSB            0x0D
#define AD713X_REG_STREAM_MODE              0x0E
#define AD713X_REG_TRANSFER_REGISTER        0x0F
#define AD713X_REG_DEVICE_CONFIG1           0x10
#define AD713X_REG_DATA_PACKET_CONFIG       0x11
#define AD713X_REG_DIGITAL_INTERFACE_CONFIG 0x12
#define AD713X_REG_POWER_DOWN_CONTROL       0x13
#define AD713X_REG_AIN_RANGE_SELECT         0x14
#define AD713X_REG_DEVICE_STATUS            0x15
#define AD713X_REG_ODR_VAL_INT_LSB          0x16
#define AD713X_REG_ODR_VAL_INT_MID          0x17
#define AD713X_REG_ODR_VAL_INT_MSB          0x18
#define AD713X_REG_ODR_VAL_FLT_LSB          0x19
#define AD713X_REG_ODR_VAL_FLT_MID0         0x1A
#define AD713X_REG_ODR_VAL_FLT_MID1         0x1B
#define AD713X_REG_ODR_VAL_FLT_MSB          0x1C
#define AD713X_REG_CHANNEL_ODR_SELECT       0x1D
#define AD713X_REG_CHAN_DIG_FILTER_SEL      0x1E
#define AD713X_REG_FIR_BW_SEL               0x1F
#define AD713X_REG_GPIO_DIR_CTRL            0x20
#define AD713X_REG_GPIO_DATA                0x21
#define AD713X_REG_ERROR_PIN_SRC_CONTROL    0x22
#define AD713X_REG_ERROR_PIN_CONTROL        0x23
#define AD713X_REG_VCMBUF_CTRL              0x24
#define AD713X_REG_DIAGNOSTIC_CONTROL       0x25
#define AD713X_REG_MPC_CONFIG               0x26
#define AD713X_REG_CH0_GAIN_LSB             0x27
#define AD713X_REG_CH0_GAIN_MID             0x28
#define AD713X_REG_CH0_GAIN_MSB             0x29
#define AD713X_REG_CH0_OFFSET_LSB           0x2A
#define AD713X_REG_CH0_OFFSET_MID           0x2B
#define AD713X_REG_CH0_OFFSET_MSB           0x2C
#define AD713X_REG_CH1_GAIN_LSB             0x2D
#define AD713X_REG_CH1_GAIN_MID             0x2E
#define AD713X_REG_CH1_GAIN_MSB             0x2F
#define AD713X_REG_CH1_OFFSET_LSB           0x30
#define AD713X_REG_CH1_OFFSET_MID           0x31
#define AD713X_REG_CH1_OFFSET_MSB           0x32
#define AD713X_REG_CH2_GAIN_LSB             0x33
#define AD713X_REG_CH2_GAIN_MID             0x34
#define AD713X_REG_CH2_GAIN_MSB             0x35
#define AD713X_REG_CH2_OFFSET_LSB           0x36
#define AD713X_REG_CH2_OFFSET_MID           0x37
#define AD713X_REG_CH2_OFFSET_MSB           0x38
#define AD713X_REG_CH3_GAIN_LSB             0x39
#define AD713X_REG_CH3_GAIN_MID             0x3A
#define AD713X_REG_CH3_GAIN_MSB             0x3B
#define AD713X_REG_CH3_OFFSET_LSB           0x3C
#define AD713X_REG_CH3_OFFSET_MID           0x3D
#define AD713X_REG_CH3_OFFSET_MSB           0x3E
#define AD713X_REG_MCLK_COUNTER             0x3F
#define AD713X_REG_DIG_FILTER_OFUF          0x40
#define AD713X_REG_DIG_FILTER_SETTLED       0x41
#define AD713X_REG_INTERNAL_ERROR           0x42
#define AD713X_REG_POWER_OV_ERROR_1         0x43
#define AD713X_REG_POWER_UV_ERROR_1         0x44
#define AD713X_REG_POWER_OV_ERROR_2         0x45
#define AD713X_REG_POWER_UV_ERROR_2         0x46
#define AD713X_REG_SPI_ERROR                0x47
#define AD713X_REG_AIN_OR_ERROR             0x48
#define AD713X_REG_AVDD5_VALUE              0x49
#define AD713X_REG_DVDD5_VALUE              0x4A
#define AD713X_REG_VREF_VALUE               0x4B
#define AD713X_REG_LDOIN_VALUE              0x4C
#define AD713X_REG_AVDD1V8_VALUE            0x4D
#define AD713X_REG_DVDD1V8_VALUE            0x4E
#define AD713X_REG_CLKVDD_VALUE             0x4F
#define AD713X_REG_IOVDD_VALUE              0x50
#define AD713X_REG_TEMPERATURE_DATA         0x51

/*
 * AD713X_REG_INTERFACE_CONFIG_A
 */
#define AD713X_INT_CONFIG_A_SOFT_RESET_MSK      NO_OS_BIT(7)
#define AD713X_INT_CONFIG_A_ADDR_ASC_BIT_MSK    NO_OS_BIT(5)
#define AD713X_INT_CONFIG_A_SDO_ACTIVE_BIT_MSK  NO_OS_BIT(4)
#define AD713X_INT_CONFIG_A_SOFT_RESET_MIRR_MSK NO_OS_BIT(0)
#define AD713X_INT_CONFIG_A_ADDR_ASC_MIRR_MSK   NO_OS_BIT(2)
#define AD713X_INT_CONFIG_A_SDO_ACTIVE_MIRR_MSK NO_OS_BIT(3)

/*
 * AD713X_REG_INTERFACE_CONFIG_B
 */
#define AD713X_INT_CONFIG_B_SINGLE_INSTR_MSK NO_OS_BIT(7)
#define AD713X_INT_CONFIG_B_M_S_RD_CTRL_MSK  NO_OS_BIT(5)
#define AD713X_INT_CONFIG_B_DIG_IF_RST_MSK   NO_OS_BIT(1)

/*
 * AD713X_REG_DEVICE_CONFIG
 */
#define AD713X_DEV_CONFIG_OP_IN_PROGRESS_MSK NO_OS_BIT(5)
#define AD713X_DEV_CONFIG_NO_CHIP_ERR_MSK    NO_OS_BIT(4)
#define AD713X_DEV_CONFIG_PWR_MODE_MSK       NO_OS_BIT(0)

/*
 * AD713X_REG_CHIP_TYPE
 */
#define AD713X_CHIP_TYPE_BITS_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_CHIP_TYPE_BITS_MODE(x) (((x) & 0xFF) << 0)
#define AD713X_CHIP_TYPE              0x07

/*
 * AD713X_REG_PRODUCT_ID_LSB
 */
#define AD713X_PRODUCT_ID_LSB_BITS_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_PRODUCT_ID_LSB_BITS_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_PRODUCT_ID_MSB
 */
#define AD713X_PRODUCT_ID_MSB_BITS_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_PRODUCT_ID_MSB_BITS_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_CHIP_GRADE
 */
#define AD713X_CHIP_GRADE_PROD_GRADE_BITS_MSK      NO_OS_GENMASK(7, 4)
#define AD713X_CHIP_GRADE_PROD_GRADE_BITS_MODE(x)  (((x) & 0x0F) << 4)
#define AD713X_CHIP_GRADE_DEV_VERSION_BITS_MSK     NO_OS_GENMASK(3, 0)
#define AD713X_CHIP_GRADE_DEV_VERSION_BITS_MODE(x) (((x) & 0x0F) << 0)

/*
 * AD713X_REG_CHIP_INDEX
 */
#define AD713X_SILICON_REV_ID_BITS_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_SILICON_REV_ID_BITS_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_SCRATCH_PAD
 */
#define AD713X_SCRATCH_PAD_BITS_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_SCRATCH_PAD_BITS_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_SPI_REVISION
 */
#define AD713X_SPI_REVISION_BITS_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_SPI_REVISION_BITS_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_VENDOR_ID_LSB
 */
#define AD713X_VENDOR_ID_LSB_BITS_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_VENDOR_ID_LSB_BITS_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_VENDOR_ID_MSB
 */
#define AD713X_VENDOR_ID_MSB_BITS_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_VENDOR_ID_MSB_BITS_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_STREAM_MODE
 */
#define AD713X_STREAM_MODE_BITS_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_STREAM_MODE_BITS_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_TRANSFER_REGISTER
 */
#define AD713X_TRANSFER_MASTER_SLAVE_TX_BIT_MSK NO_OS_BIT(0)

/*
 * AD713X_REG_DEVICE_CONFIG1
 */
#define AD713X_DEV_CONFIG1_MPC_MAGPHA_EN_MSK    NO_OS_BIT(6)
#define AD713X_DEV_CONFIG1_MPC_MAG_EN_MSK       NO_OS_BIT(5)
#define AD713X_DEV_CONFIG1_AA_MODE_MSK          NO_OS_BIT(4)
#define AD713X_DEV_CONFIG1_SDO_PIN_SRC_SEL_MSK  NO_OS_BIT(2)
#define AD713X_DEV_CONFIG1_REF_GAIN_CORR_EN_MSK NO_OS_BIT(1)
#define AD713X_DEV_CONFIG1_CLKOUT_EN_MSK        NO_OS_BIT(0)

/*
 * AD713X_REG_DATA_PACKET_CONFIG
 */
#define AD713X_DATA_PACKET_CONFIG_CRC_POLY_RST_MSK  NO_OS_BIT(7)
#define AD713X_DATA_PACKET_CONFIG_FRAME_MSK         NO_OS_GENMASK(6, 4)
#define AD713X_DATA_PACKET_CONFIG_FRAME_MODE(x)     (((x) & 0x7) << 4)
#define AD713X_DATA_PACKET_CONFIG_DCLK_FREQ_MSK     NO_OS_GENMASK(3, 0)
#define AD713X_DATA_PACKET_CONFIG_DCLK_FREQ_MODE(x) (((x) & 0xF) << 0)

/*
 * AD713X_REG_DIGITAL_INTERFACE_CONFIG
 */
#define AD713X_DIG_INT_CONFIG_DAISY_CHAIN_NUM_MSK     NO_OS_GENMASK(7, 4)
#define AD713X_DIG_INT_CONFIG_DAISY_CHAIN_NUM_MODE(x) (((x) & 0xF) << 4)
#define AD713X_DIG_INT_CONFIG_AVG_SEL_MSK             NO_OS_GENMASK(3, 2)
#define AD713X_DIG_INT_CONFIG_AVG_SEL_MODE(x)         (((x) & 0x3) << 2)
#define AD713X_DIG_INT_CONFIG_FORMAT_MSK              NO_OS_GENMASK(1, 0)
#define AD713X_DIG_INT_CONFIG_FORMAT_MODE(x)          (((x) & 0x3) << 0)

/*
 * AD713X_REG_POWER_DOWN_CONTROL
 */
#define AD713X_PWRDN_CTRL_PWRDN_CH_MSK(ch)        NO_OS_BIT(ch)
#define AD713X_PWRDN_CTRL_PWRDN_AUXADC_MSK        NO_OS_BIT(2)
#define AD713X_PWRDN_CTRL_PWRDN_LDO_MSK           NO_OS_BIT(1)
#define AD713X_PWRDN_CTRL_PWRDN_SLEEP_MODE_EN_MSK NO_OS_BIT(0)

/*
 * AD713X_REG_AIN_RANGE_SELECT
 */
#define AD713X_AIN_RANGE_SEL_CH_MSK(ch) NO_OS_BIT(ch)

/*
 * AD713X_REG_DEVICE_STATUS
 */
#define AD713X_DEV_STAT_DCLKMODE_MSK NO_OS_BIT(5)
#define AD713X_DEV_STAT_DCLKIO_MSK   NO_OS_BIT(4)
#define AD713X_DEV_STAT_MODE_MSK     NO_OS_BIT(3)
#define AD713X_DEV_STAT_CLKSEL_MSK   NO_OS_BIT(2)
#define AD713X_DEV_STAT_FUSE_ECC_MSK NO_OS_BIT(1)
#define AD713X_DEV_STAT_PLL_LOCK_MSK NO_OS_BIT(0)

/*
 * AD713X_REG_ODR_VAL_INT_LSB
 */
#define AD713X_ODR_VAL_INT_LSB_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_ODR_VAL_INT_LSB_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_ODR_VAL_INT_MID
 */
#define AD713X_ODR_VAL_INT_MID_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_ODR_VAL_INT_MID_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_ODR_VAL_INT_MSB
 */
#define AD713X_ODR_VAL_INT_MSB_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_ODR_VAL_INT_MSB_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_ODR_VAL_FLT_LSB
 */
#define AD713X_ODR_VAL_FLT_LSB_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_ODR_VAL_FLT_LSB_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_ODR_VAL_FLT_MID0
 */
#define AD713X_ODR_VAL_FLT_MID0_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_ODR_VAL_FLT_MID0_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_ODR_VAL_FLT_MID1
 */
#define AD713X_ODR_VAL_FLT_MID1_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_ODR_VAL_FLT_MID1_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_ODR_VAL_FLT_MSB
 */
#define AD713X_ODR_VAL_FLT_MSB_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_ODR_VAL_FLT_MSB_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_CHANNEL_ODR_SELECT
 */
#define AD713X_ODR_RATE_SEL_CH_MSK(ch)     (NO_OS_GENMASK(1, 0) << (2 * ch))
#define AD713X_ODR_RATE_SEL_CH_MODE(x, ch) (((x) & 0x3) << (2 * ch))

/*
 * AD713X_REG_CHAN_DIG_FILTER_SEL
 */
#define AD713X_DIGFILTER_SEL_CH_MSK(ch)     (NO_OS_GENMASK(1, 0) << (2 * ch))
#define AD713X_DIGFILTER_SEL_CH_MODE(x, ch) (((x) & 0x3) << (2 * ch))

/*
 * AD713X_REG_FIR_BW_SEL
 */
#define AD713X_FIR_BW_SEL_CH_MSK(ch) NO_OS_BIT(ch)

/*
 * AD713X_REG_GPIO_DIR_CTRL
 */
#define AD713X_GPIO_IO_CTRL_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_GPIO_IO_CTRL_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_GPIO_DATA
 */
#define AD713X_GPIO_DATA_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_GPIO_DATA_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_ERROR_PIN_SRC_CONTROL
 */
#define AD713X_ERR_PIN_EN_OR_AIN_MSK   NO_OS_BIT(5)
#define AD713X_ERR_PIN_EN_INTERNAL_MSK NO_OS_BIT(4)
#define AD713X_ERR_PIN_EN_SPI_MSK      NO_OS_BIT(3)
#define AD713X_ERR_PIN_EN_LDO_XOSC_MSK NO_OS_BIT(2)
#define AD713X_ERR_PIN_EN_TEMP_MSK     NO_OS_BIT(1)
#define AD713X_ERR_PIN_EN_PWR_MSK      NO_OS_BIT(0)

/*
 * AD713X_REG_ERROR_PIN_CONTROL
 */
#define AD713X_ERR_PIN_IN_STATUS_MSK NO_OS_BIT(2)
#define AD713X_ERR_PIN_IN_EN_MSK     NO_OS_BIT(1)
#define AD713X_ERR_PIN_OUT_EN_MSK    NO_OS_BIT(0)

/*
 * AD713X_REG_VCMBUF_CTRL
 */
#define AD713X_VCMBUF_CTRL_PWRDN_MSK           NO_OS_BIT(6)
#define AD713X_VCMBUF_CTRL_REF_DIV_SEL_MSK     NO_OS_GENMASK(5, 1)
#define AD713X_VCMBUF_CTRL_REF_DIV_SEL_MODE(x) (((x) & 0x1F) << 1)
#define AD713X_VCMBUF_CTRL_REF_SEL_MSK         NO_OS_BIT(0)

/*
 * AD713X_REG_DIAGNOSTIC_CONTROL
 */
#define AD713X_DIAGCTRL_ERR_OR_AIN_EN_MSK  NO_OS_BIT(5)
#define AD713X_DIAGCTRL_ERR_PWR_MON_EN_MSK NO_OS_BIT(4)
#define AD713X_DIAGCTRL_MCLK_CNT_EN_MSK    NO_OS_BIT(3)
#define AD713X_DIAGCTRL_ERR_SPI_CRC_EN_MSK NO_OS_BIT(2)
#define AD713X_DIAGCTRL_ERR_MM_CRC_EN_MSK  NO_OS_BIT(1)
#define AD713X_DIAGCTRL_FUSE_CRC_CHECK_MSK NO_OS_BIT(0)

/*
 * AD713X_REG_MPC_CONFIG
 */
#define AD713X_MPC_CLKDEL_EN_CH_MSK(ch)     (NO_OS_GENMASK(1, 0) << (2 * ch))
#define AD713X_MPC_CLKDEL_EN_CH_MODE(x, ch) (((x) & 0x3) << (2 * ch))

/*
 * AD713X_REG_CHx_GAIN_LSB
 */
#define AD713X_CH_GAIN_LSB_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_CH_GAIN_LSB_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_CHx_GAIN_MID
 */
#define AD713X_CH_GAIN_MID_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_CH_GAIN_MID_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_CHx_GAIN_MSB
 */
#define AD713X_CH_GAIN_CAL_SEL_MSK NO_OS_BIT(4)
#define AD713X_CH_GAIN_MSB_MSK     NO_OS_GENMASK(3, 0)
#define AD713X_CH_GAIN_MSB_MODE(x) (((x) & 0xF) << 0)

/*
 * AD713X_REG_CHx_OFFSET_LSB
 */
#define AD713X_CH_OFFSET_LSB_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_CH_OFFSET_LSB_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_CHx_OFFSET_MID
 */
#define AD713X_CH_OFFSET_MID_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_CH_OFFSET_MID_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_CHx_OFFSET_MSB
 */
#define AD713X_CH_OFFSET_CAL_EN_MSK  NO_OS_BIT(7)
#define AD713X_CH_OFFSET_MSB_MSK     NO_OS_GENMASK(6, 0)
#define AD713X_CH_OFFSET_MSB_MODE(x) (((x) & 0x7F) << 0)

/*
 * AD713X_REG_MCLK_COUNTER
 */
#define AD713X_MCLK_COUNT_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_MCLK_COUNT_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_DIG_FILTER_OFUF
 */
#define AD713X_DIGFILTER_ERR_OFUF_CH_MSK(ch) NO_OS_BIT(ch)

/*
 * AD713X_REG_DIG_FILTER_SETTLED
 */
#define AD713X_DIGFILTER_CH_SETTLED_MSK(ch) NO_OS_BIT(ch)

/*
 * AD713X_REG_INTERNAL_ERROR
 */
#define AD713X_INT_ERR_NO_CLOCK_MSK NO_OS_BIT(5)
#define AD713X_INT_ERR_TEMP_MSK     NO_OS_BIT(4)
#define AD713X_INT_ERR_DCLK_MSK     NO_OS_BIT(3)
#define AD713X_INT_ERR_FUSE_CRC_MSK NO_OS_BIT(2)
#define AD713X_INT_ERR_ASRC_MSK     NO_OS_BIT(1)
#define AD713X_INT_ERR_MM_CRC_MSK   NO_OS_BIT(0)

/*
 * AD713X_REG_POWER_OV_ERROR_1
 */
#define AD713X_POWER_ERR_OV_IOVDD_MSK   NO_OS_BIT(3)
#define AD713X_POWER_ERR_OV_CLKVDD_MSK  NO_OS_BIT(2)
#define AD713X_POWER_ERR_OV_DVDD1V8_MSK NO_OS_BIT(1)
#define AD713X_POWER_ERR_OV_AVDD1V8_MSK NO_OS_BIT(0)

/*
 * AD713X_REG_POWER_UV_ERROR_1
 */
#define AD713X_POWER_ERR_UV_IOVDD_MSK   NO_OS_BIT(3)
#define AD713X_POWER_ERR_UV_CLKVDD_MSK  NO_OS_BIT(2)
#define AD713X_POWER_ERR_UV_DVDD1V8_MSK NO_OS_BIT(1)
#define AD713X_POWER_ERR_UV_AVDD1V8_MSK NO_OS_BIT(0)

/*
 * AD713X_REG_POWER_OV_ERROR_2
 */
#define AD713X_POWER_ERR_OV_VREF_MSK  NO_OS_BIT(3)
#define AD713X_POWER_ERR_OV_LDOIN_MSK NO_OS_BIT(2)
#define AD713X_POWER_ERR_OV_DVDD5_MSK NO_OS_BIT(1)
#define AD713X_POWER_ERR_OV_AVDD5_MSK NO_OS_BIT(0)

/*
 * AD713X_REG_POWER_UV_ERROR_2
 */
#define AD713X_POWER_ERR_UV_VREF_MSK  NO_OS_BIT(3)
#define AD713X_POWER_ERR_UV_LDOIN_MSK NO_OS_BIT(2)
#define AD713X_POWER_ERR_UV_DVDD5_MSK NO_OS_BIT(1)
#define AD713X_POWER_ERR_UV_AVDD5_MSK NO_OS_BIT(0)

/*
 * AD713X_REG_SPI_ERROR
 */
#define AD713X_SPI_ERROR_CRC_MSK      NO_OS_BIT(3)
#define AD713X_SPI_ERROR_SCLK_CNT_MSK NO_OS_BIT(2)
#define AD713X_SPI_ERROR_WRITE_MSK    NO_OS_BIT(1)
#define AD713X_SPI_ERROR_READ_MSK     NO_OS_BIT(0)

/*
 * AD713X_REG_AIN_OR_ERROR
 */
#define AD713X_ERR_OR_AIN_MSK(ch) NO_OS_BIT(ch)

/*
 * AD713X_REG_AVDD5_VALUE
 */
#define AD713X_AVDD5_VALUE_PIN_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_AVDD5_VALUE_PIN_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_DVDD5_VALUE
 */
#define AD713X_DVDD5_VALUE_PIN_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_DVDD5_VALUE_PIN_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_VREF_VALUE
 */
#define AD713X_VREF_VALUE_PIN_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_VREF_VALUE_PIN_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_LDOIN_VALUE
 */
#define AD713X_LDOIN_VALUE_PIN_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_LDOIN_VALUE_PIN_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_AVDD1V8_VALUE
 */
#define AD713X_AVDD1V8_VALUE_PIN_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_AVDD1V8_VALUE_PIN_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_DVDD1V8_VALUE
 */
#define AD713X_DVDD1V8_VALUE_PIN_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_DVDD1V8_VALUE_PIN_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_CLKVDD_VALUE
 */
#define AD713X_CLKVDD_VALUE_PIN_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_CLKVDD_VALUE_PIN_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_IOVDD_VALUE
 */
#define AD713X_IOVDD_VALUE_PIN_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_IOVDD_VALUE_PIN_MODE(x) (((x) & 0xFF) << 0)

/*
 * AD713X_REG_TEMPERATURE_DATA
 */
#define AD713X_TEMP_DATA_MSK     NO_OS_GENMASK(7, 0)
#define AD713X_TEMP_DATA_MODE(x) (((x) & 0xFF) << 0)

#define AD713X_REG_READ(x) ((1 << 7) | (x & 0x7F))

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/**
 * @enum ad713x_supported_dev_ids
 * @brief ID of devices supported by the driver
 */
enum ad713x_supported_dev_ids { ID_AD7132, ID_AD7134, ID_AD7136, ID_AD4134 };

/**
 * @enum ad713x_power_mode
 * @brief AD713x power modes
 */
enum ad713x_power_mode {
    /** Low power mode option */
    LOW_POWER,
    /** Full power mode option */
    HIGH_POWER
};

/**
 * @enum ad713x_adc_data_len
 * @brief AD713x possible number of bits per data sample
 */
enum ad713x_adc_data_len {
    /** 16 bit data sample */
    ADC_16_BIT_DATA,
    /** 24 bit data sample */
    ADC_24_BIT_DATA,
    /** 32 bit data sample */
    ADC_32_BIT_DATA,
    /** To know when to stop when cycling between them */
    INVALID
};

/**
 * @enum ad713x_crc_header
 * @brief AD713x possible data CRC header choices
 */
enum ad713x_crc_header {
    /** Data sample comes with no CRC attached */
    NO_CRC,
    /** Data sample comes with 6-bit CRC attached */
    CRC_6,
    /** Data sample comes with 8-bit CRC attached */
    CRC_8
};

/**
 * @enum ad713x_doutx_format
 * @brief AD713x list for possible output modes
 */
enum ad713x_doutx_format {
    /** Single channel Daisy-chain mode */
    SINGLE_CH_DC,
    /** Dual channel Daisy-chain mode */
    DUAL_CH_DC,
    /** Quad-channel parallel output mode */
    QUAD_CH_PO,
    /** Channel average mode */
    CH_AVG_MODE
};

/**
 * @enum ad713x_dig_filter_sel
 * @brief AD713x list of input filters.
 */
enum ad713x_dig_filter_sel {
    /** Wideband filter (Finite impulse response) */
    FIR,
    /** Sinc6 filter */
    SINC6,
    /** Sinc3 filter */
    SINC3,
    /** Sinc3 filter with 50Hz and 60Hz rejection */
    SINC3_50_60_REJ
};

/**
 * @enum ad713x_channels
 * @brief AD713x list of channels
 */
enum ad713x_channels {
    /** Channel 0 */
    CH0,
    /** Channel 1 */
    CH1,
    /** Channel 2 */
    CH2,
    /** Channel 3 */
    CH3,
    /** Max number of channels */
    AD713X_CH_MAX
};

/**
 * @enum ad717x_mpc_clkdel
 * @brief AD713x list of clock delays
 */
enum ad717x_mpc_clkdel {
    /** No delay */
    DELAY_NONE,
    /** One clock cycle delay */
    DELAY_1_CLOCKS,
    /** Two clock cycles delay */
    DELAY_2_CLOCKS
};

/**
 * @struct ad713x_dev
 * @brief AD713x driver handler structure
 */
struct ad713x_dev {
    /** SPI layer handler. */
    struct no_os_spi_desc *spi_desc;
    /** MODE GPIO handler. */
    struct no_os_gpio_desc *gpio_mode;
    /** DCLKMODE GPIO handler. */
    struct no_os_gpio_desc *gpio_dclkmode;
    /** DCLKIO GPIO handler. */
    struct no_os_gpio_desc *gpio_dclkio;
    /** RESET GPIO handler. */
    struct no_os_gpio_desc *gpio_resetn;
    /** PDN GPIO handler. */
    struct no_os_gpio_desc *gpio_pnd;
    /** ID of supported device. */
    enum ad713x_supported_dev_ids dev_id;
    /** Length of data in bits. */
    enum ad713x_adc_data_len adc_data_len;
    /** CRC option. */
    enum ad713x_crc_header crc_header;
    /** MODE GPIO starting value */
    bool mode_master_nslave;
};

/**
 * @struct ad713x_init_param
 * @brief AD713x driver initialization structure
 */
struct ad713x_init_param {
    /** SPI layer initialization structure. */
    struct no_os_spi_init_param spi_init_prm;
    /** MODE GPIO initialization structure. */
    struct no_os_gpio_init_param *gpio_mode;
    /** DCLKMODE GPIO initialization structure. */
    struct no_os_gpio_init_param *gpio_dclkmode;
    /** DCLKIO GPIO initialization structure. */
    struct no_os_gpio_init_param *gpio_dclkio;
    /** RESET GPIO initialization structure. */
    struct no_os_gpio_init_param *gpio_resetn;
    /** PDN GPIO initialization structure. */
    struct no_os_gpio_init_param *gpio_pnd;
    /** MODE GPIO starting value */
    bool mode_master_nslave;
    /** DCLKMODE GPIO starting value */
    bool dclkmode_free_ngated;
    /** DCLKIO GPIO starting value */
    bool dclkio_out_nin;
    /** PDN GPIO starting value */
    bool pnd;
    /** ID of supported device. */
    enum ad713x_supported_dev_ids dev_id;
    /** Length of data in bits. */
    enum ad713x_adc_data_len adc_data_len;
    /** CRC option. */
    enum ad713x_crc_header crc_header;
    enum ad713x_doutx_format format;
    /** Clock delay state. */
    bool clk_delay_en;
    /** SPI layer handler if the SPI bus is shared with another device. In this
   *  case the SPI should not be initialized again. */
    struct no_os_spi_desc *spi_common_dev;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/** Read from device. */
static int32_t ad713x_spi_reg_read(struct ad713x_dev *dev, uint8_t reg_addr, uint8_t *reg_data);

/** Write to device. */
static int32_t ad713x_spi_reg_write(struct ad713x_dev *dev, uint8_t reg_addr, uint8_t reg_data);

/** SPI write to device using a mask. */
static int32_t ad713x_spi_write_mask(struct ad713x_dev *dev, uint8_t reg_addr, uint32_t mask,
                                     uint8_t data);

/** Device power mode control. */
static int32_t ad713x_set_power_mode(struct ad713x_dev *dev, enum ad713x_power_mode mode);

/** ADC conversion data output frame control. */
static int32_t ad713x_set_out_data_frame(struct ad713x_dev *dev,
                                         enum ad713x_adc_data_len adc_data_len,
                                         enum ad713x_crc_header crc_header);

/** DOUTx output format configuration. */
static int32_t ad713x_dout_format_config(struct ad713x_dev *dev, enum ad713x_doutx_format format);

/** Magnitude and phase matching calibration clock delay enable for all channels
 *  at 2 clock delay. */
static int32_t ad713x_mag_phase_clk_delay(struct ad713x_dev *dev, bool clk_delay_en);

static int32_t ad713x_mag_phase_clk_delay_chan(struct ad713x_dev *dev, enum ad713x_channels chan,
                                               enum ad717x_mpc_clkdel mode);

/** Digital filter type selection for each channel. */
static int32_t ad713x_dig_filter_sel_ch(struct ad713x_dev *dev, enum ad713x_dig_filter_sel filter,
                                        enum ad713x_channels ch);

/** Enable/Disable CLKOUT output. */
static int32_t ad713x_clkout_output_en(struct ad713x_dev *dev, bool enable);

/** Enable/Disable reference gain correction. */
static int32_t ad713x_ref_gain_correction_en(struct ad713x_dev *dev, bool enable);

/** Select the wideband filter bandwidth for a channel. */
static int32_t ad713x_wideband_bw_sel(struct ad713x_dev *dev, enum ad713x_channels ch,
                                      uint8_t wb_opt);

/** Initialize the device. */
static int32_t ad713x_init(struct ad713x_dev **device, struct ad713x_init_param *init_param);

/** Free the resources allocated by ad713x_init(). */
static int32_t ad713x_remove(struct ad713x_dev *dev);


static int32_t ad713x_set_odr(struct ad713x_dev *dev, fpu_t sps);
static int32_t ad713x_get_odr(struct ad713x_dev *dev, fpu_t *sps);


static struct ad713x_dev ad713x_dev_instance = {
    0,
};
static struct no_os_spi_desc spi_desc_instance = {
    0,
};

/***************************************************************************************************************************************************************
 **********  AD7134 (https://github.com/analogdevicesinc/no-OS/blob/master/drivers/adc/ad713x/ad713x.c)
 **************************************************************************************************************************************************************/

/*--------------- no_os_spi.c (Implementation of the SPI Interface) ---------------*/
/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/**
 * @brief Initialize the SPI communication peripheral.
 * @param desc - The SPI descriptor.
 * @param param - The structure that contains the SPI parameters.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t generic_spi_init(struct no_os_spi_desc **desc,
                                const struct no_os_spi_init_param *param) {
    no_os_spi_desc *descriptor = &spi_desc_instance;
    int32_t ret = WD_CODE_OK;

    uint32_t control = WD_SPI_SOFT_MODE_MASTER | WD_SPI_SOFT_DATA_BITS(8) | WD_SPI_SOFT_MSB_LSB |
                       WD_SPI_SOFT_SS_MASTER_HW_OUTPUT;

    if (!desc || !param->max_speed_hz) {
        return -1;
    }

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) == 0U) {
        return (WD_CODE_ERROR);
    }

    switch (param->mode) {
    case NO_OS_SPI_MODE_0:
        control |= WD_SPI_SOFT_CPOL0_CPHA0;
        break;

    case NO_OS_SPI_MODE_1:
        control |= WD_SPI_SOFT_CPOL0_CPHA1;
        break;

    case NO_OS_SPI_MODE_2:
        control |= WD_SPI_SOFT_CPOL1_CPHA0;
        break;

    case NO_OS_SPI_MODE_3:
        control |= WD_SPI_SOFT_CPOL1_CPHA1;
        break;
    default:
        break;
    }

    ret = wd_softspi_control(LOCAL_RES->Drv, control, param->max_speed_hz);
    if (ret != WD_CODE_OK) {
        return (WD_CODE_ERROR);
    }

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

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_OK);
    }
#if _CONF_CONSOLE_LEVEL_LOCAL_VV >= 4
    WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "ADC Config SPI", WD_MSG_MAGIC, WD_NET_TX,
                   generic_spi_write_and_read_tx_buf, bytes_number);
    //  wd_time_wait_us(1);	// DUMP_LOG 시 깨짐 발생_이건 USB 콘솔문제인거 같음.
#endif
    LOCAL_RES->Info->Event = 0;
    if (wd_softspi_transfer(LOCAL_RES->Drv, generic_spi_write_and_read_tx_buf, data,
                            bytes_number) != WD_CODE_OK) {
        WD_LOG_ERROR();
        ret = WD_CODE_ERROR;
    }
//  WD_WAIT_WHILE((LOCAL_RES->Info->Event & WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE) != WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE, 100000);
#if _CONF_CONSOLE_LEVEL_LOCAL_VV >= 4
    WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "ADC Config SPI", WD_MSG_MAGIC, WD_NET_RX, data,
                   bytes_number);
//  wd_time_wait_us(1);	// DUMP_LOG 시 깨짐 발생_이건 USB 콘솔문제인거 같음.
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
static int32_t no_os_spi_remove(struct no_os_spi_desc *desc) {
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


/*--------------- ad713x.c (Implementation of ad713x Driver) ---------------*/
/******************************************************************************/
/***************************** Variable definition ****************************/
/******************************************************************************/

static const int ad713x_output_data_frame[3][9][2] = {
    {{ADC_16_BIT_DATA, CRC_6},
     {ADC_24_BIT_DATA, CRC_6},
     {ADC_32_BIT_DATA, NO_CRC},
     {ADC_32_BIT_DATA, CRC_6},
     {ADC_16_BIT_DATA, NO_CRC},
     {ADC_24_BIT_DATA, NO_CRC},
     {ADC_24_BIT_DATA, CRC_8},
     {ADC_32_BIT_DATA, CRC_8},
     {INVALID}},
    {{ADC_16_BIT_DATA, NO_CRC},
     {ADC_16_BIT_DATA, CRC_6},
     {ADC_24_BIT_DATA, NO_CRC},
     {ADC_24_BIT_DATA, CRC_6},
     {ADC_16_BIT_DATA, CRC_8},
     {ADC_24_BIT_DATA, CRC_8},
     {INVALID}},
    {{ADC_16_BIT_DATA, NO_CRC}, {ADC_16_BIT_DATA, CRC_6}, {ADC_16_BIT_DATA, CRC_8}, {INVALID}},
};

/******************************************************************************/
/************************** Functions Implementation **************************/
/******************************************************************************/

/**
 * @brief Read from device.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The register data.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_spi_reg_read(struct ad713x_dev *dev, uint8_t reg_addr, uint8_t *reg_data) {
    int32_t ret;
    uint8_t buf[2];

    buf[0] = AD713X_REG_READ(reg_addr);
    buf[1] = 0x00;

    ret = no_os_spi_write_and_read(dev->spi_desc, buf, 2);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        return -1;
    }
    *reg_data = buf[1];

    return 0;
}

/**
 * @brief Write to device.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The register data.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_spi_reg_write(struct ad713x_dev *dev, uint8_t reg_addr, uint8_t reg_data) {
    uint8_t buf[2];

    buf[0] = reg_addr;
    buf[1] = reg_data;

    return no_os_spi_write_and_read(dev->spi_desc, buf, 2);
}

/**
 * @brief SPI write to device using a mask.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param mask - The mask.
 * @param data - The register data.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_spi_write_mask(struct ad713x_dev *dev, uint8_t reg_addr, uint32_t mask,
                                     uint8_t data) {
    uint8_t reg_data;
    int32_t ret;

    ret = ad713x_spi_reg_read(dev, reg_addr, &reg_data);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        return -1;
    }
    reg_data = (uint8_t)(reg_data & ~mask);
    reg_data |= data;

    return ad713x_spi_reg_write(dev, reg_addr, reg_data);
}

/**
 * @brief Device power mode control.
 * @param dev - The device structure.
 * @param mode - Type of power mode
 *      Accepted values: LOW_POWER
 *           HIGH_POWER
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_set_power_mode(struct ad713x_dev *dev, enum ad713x_power_mode mode) {
    if (mode == LOW_POWER) {
        return ad713x_spi_write_mask(dev, AD713X_REG_DEVICE_CONFIG, AD713X_DEV_CONFIG_PWR_MODE_MSK,
                                     0);
    } else if (mode == HIGH_POWER) {
        return ad713x_spi_write_mask(dev, AD713X_REG_DEVICE_CONFIG, AD713X_DEV_CONFIG_PWR_MODE_MSK,
                                     1);
    }

    return -1;
}

/**
 * @brief ADC conversion data output frame control.
 * @param dev - The device structure.
 * @param adc_data_len - Data conversion length
 *        Accepted values: ADC_16_BIT_DATA
 *             ADC_24_BIT_DATA
 *             ADC_32_BIT_DATA
 * @param crc_header - CRC header
 *        Accepted values: NO_CRC
 *             CRC_6
 *             CRC_8
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_set_out_data_frame(struct ad713x_dev *dev,
                                         enum ad713x_adc_data_len adc_data_len,
                                         enum ad713x_crc_header crc_header) {
    uint8_t id;
    uint8_t i = 0;

    id = (uint8_t)dev->dev_id;

    while (ad713x_output_data_frame[id][i][0] != INVALID) {
        if ((adc_data_len == ad713x_output_data_frame[id][i][0]) &&
            (crc_header == ad713x_output_data_frame[id][i][1])) {
            return ad713x_spi_write_mask(dev, AD713X_REG_DATA_PACKET_CONFIG,
                                         AD713X_DATA_PACKET_CONFIG_FRAME_MSK,
                                         (uint8_t)AD713X_DATA_PACKET_CONFIG_FRAME_MODE(i));
        }
        i++;
    }

    return -1;
}

/**
 * @brief DOUTx output format configuration.
 * @param dev - The device structure.
 * @param format - Single channel daisy chain mode. Dual channel daisy chain mode.
 *       Quad channel parallel output mode. Channel data averaging mode.
 *      Accepted values: SINGLE_CH_DC
 *           DUAL_CH_DC
 *           QUAD_CH_PO
 *           CH_AVG_MODE
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_dout_format_config(struct ad713x_dev *dev, enum ad713x_doutx_format format) {
    return ad713x_spi_write_mask(dev, AD713X_REG_DIGITAL_INTERFACE_CONFIG,
                                 AD713X_DIG_INT_CONFIG_FORMAT_MSK,
                                 (uint8_t)AD713X_DIG_INT_CONFIG_FORMAT_MODE(format));
}

/**
 * @brief Magnitude and phase matching calibration clock delay enable for all
 *        channels at 2 clock delay.
 *        This function is kept for backwards compatibility with the current
 *        application source, but it is deprecated. Use
 *        ad713x_mag_phase_clk_delay_chan().
 * @param dev - The device structure.
 * @param clk_delay_en - Enable or disable Mag/Phase clock delay.
 *        Accepted values: true
 *                     false
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_mag_phase_clk_delay(struct ad713x_dev *dev, bool clk_delay_en) {
    int32_t ret;
    int8_t i;
    int8_t temp_clk_delay;

    if (clk_delay_en) {
        temp_clk_delay = DELAY_2_CLOCKS;
    } else {
        temp_clk_delay = DELAY_NONE;
    }

    for (i = CH3; i >= 0; i--) {
        ret = ad713x_spi_write_mask(dev, AD713X_REG_MPC_CONFIG, AD713X_MPC_CLKDEL_EN_CH_MSK(i),
                                    (uint8_t)AD713X_MPC_CLKDEL_EN_CH_MODE(temp_clk_delay, i));
        if (NO_OS_IS_ERR_VALUE(ret)) {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief Change magnitude and phase calibration clock delay mode for a specific
 *        channel.
 * @param dev - The device structure.
 * @param chan - ID of the channel to be changed.
 *        Accepted values: CH0, CH1, CH2, CH3
 * @param mode - Delay in clock periods.
 *        Accepted values: DELAY_NONE,
 *             DELAY_1_CLOCKS,
 *             DELAY_2_CLOCKS
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_mag_phase_clk_delay_chan(struct ad713x_dev *dev, enum ad713x_channels chan,
                                               enum ad717x_mpc_clkdel mode) {
    return ad713x_spi_write_mask(dev, AD713X_REG_MPC_CONFIG, AD713X_MPC_CLKDEL_EN_CH_MSK(chan),
                                 (uint8_t)AD713X_MPC_CLKDEL_EN_CH_MODE(mode, chan));
}

/**
 * @brief Digital filter type selection for each channel
 * @param dev - The device structure.
 * @param filter - Type of filter: Wideband, Sinc6, Sinc3,
 *           Sinc3 filter with simultaneous 50Hz and 60Hz rejection.
 *            Accepted values: FIR
 *               SINC6
 *               SINC3
 *               SINC3_50_60_REJ
 * @param ch - Channel to apply the filter to
 *          Accepted values: CH0
 *               CH1
 *               CH2
 *               CH3
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_dig_filter_sel_ch(struct ad713x_dev *dev, enum ad713x_dig_filter_sel filter,
                                        enum ad713x_channels ch) {
    return ad713x_spi_write_mask(dev, AD713X_REG_CHAN_DIG_FILTER_SEL,
                                 AD713X_DIGFILTER_SEL_CH_MSK(ch),
                                 (uint8_t)AD713X_DIGFILTER_SEL_CH_MODE(filter, ch));
}

/**
 * @brief Enable/Disable CLKOUT output.
 * @param [in] dev - The device structure.
 * @param [in] enable - true to enable the clkout output;
 *                      false to disable the clkout output.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_clkout_output_en(struct ad713x_dev *dev, bool enable) {
    return ad713x_spi_write_mask(dev, AD713X_REG_DEVICE_CONFIG1, AD713X_DEV_CONFIG1_CLKOUT_EN_MSK,
                                 enable ? AD713X_DEV_CONFIG1_CLKOUT_EN_MSK : 0);
}

/**
 * @brief Enable/Disable reference gain correction.
 * @param [in] dev - The device structure.
 * @param [in] enable - true to enable the reference gain correction;
 *                      false to disable the reference gain correction.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_ref_gain_correction_en(struct ad713x_dev *dev, bool enable) {
    return ad713x_spi_write_mask(dev, AD713X_REG_DEVICE_CONFIG1,
                                 AD713X_DEV_CONFIG1_REF_GAIN_CORR_EN_MSK,
                                 enable ? AD713X_DEV_CONFIG1_REF_GAIN_CORR_EN_MSK : 0);
}

/**
 * @brief Select the wideband filter bandwidth for a channel.
 *        The option is relative to ODR, so it's a fraction of it.
 * @param [in] dev - The device structure.
 * @param [in] ch - Number of the channel to which to set the wideband filter
 *                  option.
 * @param [in] wb_opt - Option to set the wideband filter:
 *                      Values are:
 *                          0 - bandwidth of 0.443 * ODR;
 *                          1 - bandwidth of 0.10825 * ODR.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_wideband_bw_sel(struct ad713x_dev *dev, enum ad713x_channels ch,
                                      uint8_t wb_opt) {
    return ad713x_spi_write_mask(dev, AD713X_REG_FIR_BW_SEL, AD713X_FIR_BW_SEL_CH_MSK(ch),
                                 wb_opt ? (uint8_t)AD713X_FIR_BW_SEL_CH_MSK(ch) : 0);
}

/**
 * @brief Initialize GPIO driver handlers for the GPIOs in the system.
 *        ad713x_init() helper function.
 * @param [out] dev - AD713X device handler.
 * @param [in] init_param - Pointer to the initialization structure.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_init_gpio(struct ad713x_dev *dev, struct ad713x_init_param *init_param) {
    UNUSED(dev);
    UNUSED(init_param);
    return 0;
}

/**
 * @brief Free the resources allocated by ad713x_init_gpio().
 * @param dev - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad713x_remove_gpio(struct ad713x_dev *dev) {
    UNUSED(dev);
    return 0;
}

/**
 * @brief Initialize the wideband filter bandwidth for every channel.
 *        ad713x_init() helper function.
 * @param [in] dev - AD713X device handler.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_init_chan_bw(struct ad713x_dev *dev) {
    int8_t i;
    int32_t ret;

    for (i = CH3; i >= 0; i--) {
        ret = ad713x_wideband_bw_sel(dev, (enum ad713x_channels)i, 0);
        if (NO_OS_IS_ERR_VALUE(ret)) {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief Initialize the device.
 * @param device - The device structure.
 * @param init_param - The structure that contains the device initial
 *                     parameters.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t ad713x_init(struct ad713x_dev **device, struct ad713x_init_param *init_param) {
    struct ad713x_dev *dev;
    int32_t ret;
    uint8_t data = 0U;

    dev = &ad713x_dev_instance;

    if (!init_param->spi_common_dev) {
        ret = no_os_spi_init(&dev->spi_desc, &init_param->spi_init_prm);
        if (NO_OS_IS_ERR_VALUE(ret)) {
            goto error_dev;
        }
    } else {
        dev->spi_desc = &spi_desc_instance;
        dev->spi_desc->chip_select = init_param->spi_init_prm.chip_select;
        dev->spi_desc->extra = init_param->spi_common_dev->extra;
        dev->spi_desc->max_speed_hz = init_param->spi_init_prm.max_speed_hz;
        dev->spi_desc->mode = init_param->spi_init_prm.mode;
        dev->spi_desc->platform_ops = init_param->spi_init_prm.platform_ops;
    }

    ret = ad713x_init_gpio(dev, init_param);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }

    dev->dev_id = init_param->dev_id;
    dev->mode_master_nslave = init_param->mode_master_nslave;

    //  data = AD713X_INT_CONFIG_A_SOFT_RESET_MSK;
    //  ret = ad713x_spi_reg_write(dev, AD713X_REG_INTERFACE_CONFIG_A, data);

    ret = ad713x_spi_reg_read(dev, AD713X_REG_CHIP_TYPE, &data);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }
    if (AD713X_CHIP_TYPE_BITS_MODE(data) != AD713X_CHIP_TYPE) {
        goto error_gpio;
    }

    ret = ad713x_spi_reg_read(dev, AD713X_REG_DEVICE_CONFIG, &data);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }
    data |= AD713X_DEV_CONFIG_PWR_MODE_MSK;
    ret = ad713x_spi_reg_write(dev, AD713X_REG_DEVICE_CONFIG, data);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }

    ret = ad713x_set_power_mode(dev, HIGH_POWER);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }

    ret = ad713x_clkout_output_en(dev, true);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }

    ret = ad713x_ref_gain_correction_en(dev, true);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }

    ret = ad713x_set_out_data_frame(dev, init_param->adc_data_len, init_param->crc_header);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }

    ret = ad713x_dout_format_config(dev, init_param->format);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }

    ret = ad713x_mag_phase_clk_delay(dev, init_param->clk_delay_en);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }

    ret = ad713x_init_chan_bw(dev);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        goto error_gpio;
    }

    *device = dev;

    return 0;

error_gpio:
    WD_LOG_ERROR("error_gpio");
    ad713x_remove_gpio(dev);
error_dev:
    WD_LOG_ERROR("error_dev");
    ad713x_remove(dev);

    return -1;
}

/**
 * @brief Free the resources allocated by ad713x_init().
 * @param dev - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad713x_remove(struct ad713x_dev *dev) {
    int32_t ret;

    if (!dev) {
        return -1;
    }

    ret = no_os_spi_remove(dev->spi_desc);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        return -1;
    }

    ret = ad713x_remove_gpio(dev);
    if (NO_OS_IS_ERR_VALUE(ret)) {
        return -1;
    }

    return 0;
}


static int32_t ad713x_set_odr(struct ad713x_dev *dev, fpu_t sps) {
    uint32_t odr_val_int, odr_val_flt;
    fpu_t decimation_rate = (fpu_t)24000000.0 / sps;
    fpu_t integer, fraction;
    uint8_t data = 0U;

    UNUSED(dev);

    fraction = wd_modf(decimation_rate, &integer);

    odr_val_int = (uint32_t)integer;
    odr_val_flt = (uint32_t)((fpu_t)0xFFFFFFFFL * fraction);
    if (odr_val_flt == 0) {
        odr_val_flt = 1;
    }
#if _CONF_CONSOLE_LEVEL_LOCAL_VV >= 4
    WD_LOG_DEBUG("odr_val_int : %d", odr_val_int);
    WD_LOG_DEBUG("odr_val_flt : %u", odr_val_flt);
#endif
    data = odr_val_int & 0xff;
    ad713x_spi_reg_write(&ad713x_dev_instance, AD713X_REG_ODR_VAL_INT_LSB, data);
    data = (odr_val_int >> 8) & 0xff;
    ad713x_spi_reg_write(&ad713x_dev_instance, AD713X_REG_ODR_VAL_INT_MID, data);
    data = (odr_val_int >> 16) & 0xff;
    ad713x_spi_reg_write(&ad713x_dev_instance, AD713X_REG_ODR_VAL_INT_MSB, data);

    data = odr_val_flt & 0xff;
    ad713x_spi_reg_write(&ad713x_dev_instance, AD713X_REG_ODR_VAL_FLT_LSB, data);
    data = (odr_val_flt >> 8) & 0xff;
    ad713x_spi_reg_write(&ad713x_dev_instance, AD713X_REG_ODR_VAL_FLT_MID0, data);
    data = (odr_val_flt >> 16) & 0xff;
    ad713x_spi_reg_write(&ad713x_dev_instance, AD713X_REG_ODR_VAL_FLT_MID1, data);
    data = (uint8_t)((odr_val_flt >> 24) & 0xff);
    ad713x_spi_reg_write(&ad713x_dev_instance, AD713X_REG_ODR_VAL_FLT_MSB, data);

    ad713x_spi_reg_write(&ad713x_dev_instance, AD713X_REG_TRANSFER_REGISTER,
                         AD713X_TRANSFER_MASTER_SLAVE_TX_BIT_MSK);

    return 0;
}

static int32_t ad713x_get_odr(struct ad713x_dev *dev, fpu_t *sps) {
    uint32_t odr_val_int, odr_val_flt;
    fpu_t decimation_rate;
    fpu_t freq;
    uint8_t data = 0U;

    UNUSED(dev);

    ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_ODR_VAL_INT_LSB, &data);
    odr_val_int = data;
    ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_ODR_VAL_INT_MID, &data);
    odr_val_int |= (uint32_t)(data << 8);
    ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_ODR_VAL_INT_MSB, &data);
    odr_val_int |= (uint32_t)(data << 16);

    ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_ODR_VAL_FLT_LSB, &data);
    odr_val_flt = data;
    ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_ODR_VAL_FLT_MID0, &data);
    odr_val_flt |= (uint32_t)(data << 8);
    ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_ODR_VAL_FLT_MID1, &data);
    odr_val_flt |= (uint32_t)(data << 16);
    ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_ODR_VAL_FLT_MSB, &data);
    odr_val_flt |= (uint32_t)(data << 24);

    if (odr_val_int == 0xFFFFFF) {
        decimation_rate = 0.0;
        freq = 0.0;
    } else {
        decimation_rate = (fpu_t)((fpu_t)odr_val_int + ((fpu_t)odr_val_flt / (fpu_t)0xFFFFFFFFL));
        freq = (fpu_t)24000000.0 / decimation_rate;
    }

    //  WD_LOG_DEBUG("decimation_rate : %f        odr sps : %f", (double)decimation_rate, (double)freq);

    if (sps) {
        *sps = freq;
    }

    return 0;
}
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

static void wd_ad4134_initialize(void) {
    int32_t ret = WD_CODE_OK;

    struct ad713x_dev *ad713x_dev;
    struct ad713x_init_param ad713x_init_param;

    ad713x_init_param.adc_data_len = ADC_24_BIT_DATA;
    ad713x_init_param.clk_delay_en = false;
    ad713x_init_param.crc_header = NO_CRC;
    ad713x_init_param.dev_id = ID_AD7134;
    ad713x_init_param.format = QUAD_CH_PO;
    ad713x_init_param.gpio_dclkio = NULL;
    ad713x_init_param.gpio_dclkmode = NULL;
    ad713x_init_param.gpio_mode = NULL;
    ad713x_init_param.gpio_pnd = NULL;
    ad713x_init_param.gpio_resetn = NULL;
    ad713x_init_param.mode_master_nslave = false;
    ad713x_init_param.dclkmode_free_ngated = false;
    ad713x_init_param.dclkio_out_nin = false;
    ad713x_init_param.pnd = true;
    ad713x_init_param.spi_init_prm.chip_select = 0;
    ad713x_init_param.spi_init_prm.device_id = 0;
    ad713x_init_param.spi_init_prm.max_speed_hz = LOCAL_RES->Capabilities.BaudRate;
    ad713x_init_param.spi_init_prm.mode = NO_OS_SPI_MODE_0;
    ad713x_init_param.spi_init_prm.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST;
    ad713x_init_param.spi_init_prm.platform_ops = &generic_spi_ops;
    ad713x_init_param.spi_init_prm.extra = NULL;
    ad713x_init_param.spi_common_dev = 0;

    WD_PRINT_INFO(WD_MSG_NOTI_LOG, "ADC Config Initialize");

    ret = ad713x_init(&ad713x_dev, &ad713x_init_param);
    if (ret != 0) {
        WD_LOG_ERROR();
        wd_core_status_hook(WD_STATUS_SOURCE_ADC, WD_STATUS_ERROR);
    }

    wd_core_status_hook(WD_STATUS_SOURCE_ADC, WD_STATUS_OK);

    {
        uint8_t data = 0U;

        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_DEVICE_CONFIG, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_DEVICE_CONFIG  %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_CHIP_TYPE, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_CHIP_TYPE      %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_PRODUCT_ID_LSB, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_PRODUCT_ID_LSB %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_PRODUCT_ID_MSB, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_PRODUCT_ID_MSB %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_CHIP_GRADE, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_CHIP_GRADE     %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_CHIP_INDEX, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_CHIP_INDEX     %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_SPI_REVISION, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_SPI_REVISION   %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_VENDOR_ID_LSB, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_VENDOR_ID_LSB  %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_VENDOR_ID_MSB, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_VENDOR_ID_MSB  %02X", data);

        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_DEVICE_STATUS, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_DEVICE_STATUS  %02X", data);

        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_INTERNAL_ERROR, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_INTERNAL_ERROR %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_SPI_ERROR, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_SPI_ERROR      %02X", data);
        ad713x_spi_reg_read(&ad713x_dev_instance, AD713X_REG_AIN_OR_ERROR, &data);
        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "AD713X_REG_AIN_OR_ERROR   %02X", data);
    }

    ad713x_dig_filter_sel_ch(&ad713x_dev_instance, FIR, CH0);
    ad713x_dig_filter_sel_ch(&ad713x_dev_instance, FIR, CH1);
    ad713x_dig_filter_sel_ch(&ad713x_dev_instance, FIR, CH2);
    ad713x_dig_filter_sel_ch(&ad713x_dev_instance, FIR, CH3);

    ad713x_mag_phase_clk_delay_chan(&ad713x_dev_instance, CH0, DELAY_NONE);
    ad713x_mag_phase_clk_delay_chan(&ad713x_dev_instance, CH1, DELAY_NONE);
    ad713x_mag_phase_clk_delay_chan(&ad713x_dev_instance, CH2, DELAY_NONE);
    ad713x_mag_phase_clk_delay_chan(&ad713x_dev_instance, CH3, DELAY_NONE);

#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-statement-expression-from-macro-expansion"
#endif
    ad713x_spi_write_mask(&ad713x_dev_instance, AD713X_REG_DATA_PACKET_CONFIG,
                          AD713X_DATA_PACKET_CONFIG_DCLK_FREQ_MSK,
                          AD713X_DATA_PACKET_CONFIG_DCLK_FREQ_MODE(0)); // 0 fDCLK = 48 MHz.
#if defined(__clang__)
#pragma GCC diagnostic pop
#endif
    ad713x_set_odr(&ad713x_dev_instance, 0);
}


/***************************************************************************************************************************************************************
 **********  AD4134
 **************************************************************************************************************************************************************/
void wd_ad4134_initialize(void);

static wd_ad4134_info_t ADC_AD4134_Info = {
    0,
};

#if (RTE_RTOS)
static osStaticMutexDef_t Mutex_ADC_AD4134_cb_mem;
static osMutexId_t Mutex_ADC_AD4134;
static const osMutexAttr_t Mutex_ADC_AD4134_Attr = {.name = "Mutex AC ADC",
                                                    .attr_bits =
                                                        osMutexRecursive | osMutexPrioInherit,
                                                    .cb_mem = &Mutex_ADC_AD4134_cb_mem,
                                                    .cb_size = sizeof(Mutex_ADC_AD4134_cb_mem)};

static int32_t res_start(void) {
#if (RTE_RTOS)
    Mutex_ADC_AD4134 = osMutexNew(&Mutex_ADC_AD4134_Attr);
    if (Mutex_ADC_AD4134 == NULL) {
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
        if (osMutexAcquire(Mutex_ADC_AD4134, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
            return (WD_CODE_ERROR_TIMEOUT);
        }
    }

    return (WD_CODE_OK);
}

static int32_t res_unlock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(Mutex_ADC_AD4134);
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

wd_ad4134_resources_t wd_ad4134_resources = {
    .Capabilities =
        {
            .BaudRate = SPI_BAUDRATE_AD4134,
        },
    .Info = &ADC_AD4134_Info,
    .IO =
        {
            .RESET = RTE_CONFIG_IDLE_PIN_NC,
        },
    .Drv = &Driver_SPI_Soft_AD4134_Resources,
    .Handle_V = RTE_COMPONENT_ADC_AD4134_V_DRIVER_HANDLE,
    .Reg_V = RTE_COMPONENT_ADC_AD4134_V_DRIVER_REGISTER,
    .Handle_I = RTE_COMPONENT_ADC_AD4134_I_DRIVER_HANDLE,
    .Reg_I = RTE_COMPONENT_ADC_AD4134_I_DRIVER_REGISTER,
    .Handle_CH3 = RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_HANDLE,
    .Reg_CH3 = RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_REGISTER,
    .Handle_CH4 = RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_HANDLE,
    .Reg_CH4 = RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_REGISTER,
    .Start = res_start,
    .Lock = res_lock,
    .Unlock = res_unlock,
};


#undef LOCAL_DRV
#define LOCAL_DRV (&wd_drv_adc_ac)
static wd_ad4134_resources_t *const LOCAL_RES = &wd_ad4134_resources;


static uint32_t adc_ad4134_get_max_cycle_by_frequency(fpu_t freq) {
#if 1
    if (freq >= (fpu_t)10.0) {
        return 16U;
    } else if (freq >= (fpu_t)5.0) {
        return 8U;
    } else if (freq >= (fpu_t)1.0) {
        return 4U;
    }
#else
    if (freq >= (fpu_t)10.0) {
        return 32U;
    } else if (freq >= (fpu_t)5.0) {
        return 16U;
    } else if (freq >= (fpu_t)1.0) {
        return 8U;
    } else if (freq >= (fpu_t)0.5) {
        return 4U;
    }
#endif
    else {
        return 2U;
    }
}

__STATIC_INLINE uint32_t DCache_IsAligned(const void *addr, uint32_t size) {
    uint32_t result = 0U;

    if (((((uint32_t)addr) & 0x1FU) == 0U) && ((size & 0x1FU) == 0U)) {
        // If address is 32-byte aligned and if size is multiple of 32 bytes
        result = 1U;
    }

    return result;
}

void wd_adc_ad4134_callback(uint32_t event) {
    LOCAL_RES->Info->Event |= event;
    if (LOCAL_RES->Info->CB_Event) {
        LOCAL_RES->Info->CB_Event(event);
    }
}

static void adc_data_callback(uint32_t ch, uint32_t idx) {
    wd_adc_status_t volatile *status;

    switch (ch) {
    case WD_ADC_CH_V:
        status = &LOCAL_RES->Info->Status_V;
        break;

    case WD_ADC_CH_I:
        status = &LOCAL_RES->Info->Status_I;
        break;

    case WD_ADC_CH_VDC:
        status = &LOCAL_RES->Info->Status_CH3;
        break;
#if WD_ADC_CH4_ENABLE
    case WD_ADC_CH_SP:
        status = &LOCAL_RES->Info->Status_CH4;
        break;
#endif
    default:
        WD_LOG_ERROR();
        return;
    }

    if (status->Read) {
        {
            SPI_HandleTypeDef *hspi;
            uint8_t *buf;
            int32_t *dst;
            uint8_t *src;

            switch (ch) {
            case WD_ADC_CH_V:
                status = &LOCAL_RES->Info->Status_V;
                hspi = LOCAL_RES->Handle_V;
                buf = (uint8_t *)LOCAL_RES->Info->V_Buf;
                dst = LOCAL_RES->Info->Xfer.V_Buf;
                break;

            case WD_ADC_CH_I:
                status = &LOCAL_RES->Info->Status_I;
                hspi = LOCAL_RES->Handle_I;
                buf = (uint8_t *)LOCAL_RES->Info->I_Buf;
                dst = LOCAL_RES->Info->Xfer.I_Buf;
                break;

            case WD_ADC_CH_VDC:
                status = &LOCAL_RES->Info->Status_CH3;
                hspi = LOCAL_RES->Handle_CH3;
                buf = (uint8_t *)LOCAL_RES->Info->CH3_Buf;
                dst = LOCAL_RES->Info->Xfer.CH3_Buf;
                break;
#if WD_ADC_CH4_ENABLE
            case WD_ADC_CH_SP:
                status = &LOCAL_RES->Info->Status_CH4;
                hspi = LOCAL_RES->Handle_CH4;
                buf = (uint8_t *)LOCAL_RES->Info->CH4_Buf;
                dst = LOCAL_RES->Info->Xfer.CH4_Buf;
                break;
#endif

            default:
                WD_LOG_ERROR();
                return;
            }

            if (idx == WD_DIR_HALF) {
                if (__HAL_DMA_GET_COUNTER(hspi->hdmarx) >
                    LOCAL_RES->Info->Xfer.Under_Num + LOCAL_RES->Info->Xfer.Offset) {
                    WD_LOG_ERROR("SPI DMA Overrun Half [%d]", __HAL_DMA_GET_COUNTER(hspi->hdmarx));
                    return;
                }
                src =
                    buf + ((LOCAL_RES->Info->Xfer.Cnt[ch] == 0) ? LOCAL_RES->Info->Xfer.Offset : 0);
            } else {
                if (__HAL_DMA_GET_COUNTER(hspi->hdmarx) <
                    LOCAL_RES->Info->Xfer.Under_Num + LOCAL_RES->Info->Xfer.Offset) {
                    WD_LOG_ERROR("SPI DMA Overrun Full [%d]", __HAL_DMA_GET_COUNTER(hspi->hdmarx));
                    return;
                }
                src = buf + LOCAL_RES->Info->Xfer.Under_Num + LOCAL_RES->Info->Xfer.Offset +
                      ((LOCAL_RES->Info->Xfer.Cnt[ch] == 0) ? LOCAL_RES->Info->Xfer.Offset : 0);
            }

            if (LOCAL_RES->Info->Xfer.Skip_Data_Callback_Cnt[ch] <
                LOCAL_RES->Info->Xfer.Skip_Data_Callback_Num) {
                LOCAL_RES->Info->Xfer.Skip_Data_Callback_Cnt[ch]++;
                return;
            }

            {
                uint32_t i;
                uint32_t size =
                    LOCAL_RES->Info->Xfer.Under_Num +
                    ((LOCAL_RES->Info->Xfer.Cnt[ch] == 0) ? 0 : LOCAL_RES->Info->Xfer.Offset);

                SCB_InvalidateDCache_by_Addr((uint32_t *)src, (int32_t)size);

                for (i = 0; i < size; i += (WD_ADC_DATA_SIZE * LOCAL_RES->Info->Xfer.Method)) {
                    if (LOCAL_RES->Info->Xfer.Cnt[ch] <
                        LOCAL_RES->Info->Xfer.Num) // 범위 수정 <= ->  < 변경
                    {
                        dst[LOCAL_RES->Info->Xfer.Cnt[ch]++] =
                            (int32_t)(((src[i] & 0x80) ? 0xFF000000 : 0x00000000) |
                                      ((uint32_t)src[i] << 16) | ((uint32_t)src[i + 1] << 8) |
                                      ((uint32_t)src[i + 2] << 0));
                    }
                }

                if (LOCAL_RES->Info->Xfer.Cnt[ch] >=
                    LOCAL_RES->Info->Xfer.Num) // Xfer.Cnt[ch]가 설정한 개수(ex. 1024)보다 크면
                {
                    LOCAL_RES->Info->Xfer.Cnt[ch] = 0;
                    LOCAL_RES->Info->Xfer.Skip_Cycle_Cnt[ch]++; //Skip_Cycle 카운트 증가

                    if (LOCAL_RES->Info->Xfer.Skip_Cycle_Cnt[ch] >
                        LOCAL_RES->Info->Xfer
                            .Skip_Cycle_Num) //Skip_Cycle 설정되었으면, 스킵 주기가 끝나면 변환 완료 플래그
                    {
                        LOCAL_RES->Info->Xfer.Skip_Cycle_Cnt[ch] = 0;

                        if (ch == WD_ADC_CH_V) {
                            __HAL_DMA_DISABLE((RTE_COMPONENT_ADC_AD4134_V_DRIVER_HANDLE)->hdmarx);
                            __HAL_SPI_DISABLE(RTE_COMPONENT_ADC_AD4134_V_DRIVER_HANDLE);
                            s_debug_dma_remaining[0] =
                                __HAL_DMA_GET_COUNTER(LOCAL_RES->Handle_V->hdmarx);
                        } else if (ch == WD_ADC_CH_I) {
                            __HAL_DMA_DISABLE((RTE_COMPONENT_ADC_AD4134_I_DRIVER_HANDLE)->hdmarx);
                            __HAL_SPI_DISABLE(RTE_COMPONENT_ADC_AD4134_I_DRIVER_HANDLE);
                            s_debug_dma_remaining[1] =
                                __HAL_DMA_GET_COUNTER(LOCAL_RES->Handle_I->hdmarx);
                        } else if (ch == WD_ADC_CH_VDC) {
                            __HAL_DMA_DISABLE((RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_HANDLE)->hdmarx);
                            __HAL_SPI_DISABLE(RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_HANDLE);

                            s_debug_dma_remaining[2] =
                                __HAL_DMA_GET_COUNTER(LOCAL_RES->Handle_CH3->hdmarx);
                        }
#if WD_ADC_CH4_ENABLE
                        else if (ch == WD_ADC_CH_SP) {
                            __HAL_DMA_DISABLE((RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_HANDLE)->hdmarx);
                            __HAL_SPI_DISABLE(RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_HANDLE);

                            s_debug_dma_remaining[3] =
                                __HAL_DMA_GET_COUNTER(LOCAL_RES->Handle_CH4->hdmarx);
                        }
#endif
                        status->Conversion_Finish = true;
                        status->Read = false;
                    }
                }
            }
        }
    }
}

void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi) {
    {
        if (hspi == LOCAL_RES->Handle_V) {
            adc_data_callback(WD_ADC_CH_V, WD_DIR_HALF);
        } else if (hspi == LOCAL_RES->Handle_I) {
            adc_data_callback(WD_ADC_CH_I, WD_DIR_HALF);
        } else if (hspi == LOCAL_RES->Handle_CH3) {
            adc_data_callback(WD_ADC_CH_VDC, WD_DIR_HALF);
        }
#if WD_ADC_CH4_ENABLE
        else if (hspi == LOCAL_RES->Handle_CH4) {
            adc_data_callback(WD_ADC_CH_SP, WD_DIR_HALF);
        }
#endif
    }
}

static void adc_data_error_handler(uint32_t ch) {
    wd_adc_status_t volatile *status;

    switch (ch) {
    case WD_ADC_CH_V:
        status = &LOCAL_RES->Info->Status_V;
        break;

    case WD_ADC_CH_I:
        status = &LOCAL_RES->Info->Status_I;
        break;

    case WD_ADC_CH_VDC:
        status = &LOCAL_RES->Info->Status_CH3;
        break;
#if WD_ADC_CH4_ENABLE

    case WD_ADC_CH_SP:
        status = &LOCAL_RES->Info->Status_CH4;
        break;
#endif

    default:
        WD_LOG_ERROR();
        return;
    }

    status->Error = true;
}

static void adc_v_callback(uint32_t event) {
    if (event & (WD_SPI_SOFT_EVENT_DATA_LOST | WD_SPI_SOFT_EVENT_MODE_FAULT)) {
        adc_data_error_handler(WD_ADC_CH_V);
    }

    if (event & WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE) {
        adc_data_callback(WD_ADC_CH_V, WD_DIR_FULL);
        LOCAL_RES->Info->Debug_Time_Component_ADC_V =
            wd_time_calc_us(&LOCAL_RES->Info->Debug_Time_Component_ADC_V_Stamp);
        LOCAL_RES->Info->Debug_Time_Component_ADC_V_Stamp = wd_time_get_us();
    }

    LOCAL_RES->Info->Event_V = event;
}

static void adc_i_callback(uint32_t event) {
    if (event & (WD_SPI_SOFT_EVENT_DATA_LOST | WD_SPI_SOFT_EVENT_MODE_FAULT)) {
        adc_data_error_handler(WD_ADC_CH_I);
    }

    if (event & WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE) {
        adc_data_callback(WD_ADC_CH_I, WD_DIR_FULL);
        LOCAL_RES->Info->Debug_Time_Component_ADC_I =
            wd_time_calc_us(&LOCAL_RES->Info->Debug_Time_Component_ADC_I_Stamp);
        LOCAL_RES->Info->Debug_Time_Component_ADC_I_Stamp = wd_time_get_us();
    }

    LOCAL_RES->Info->Event_I = event;
}

static void adc_vdc_callback(uint32_t event) {
    if (event & (WD_SPI_SOFT_EVENT_DATA_LOST | WD_SPI_SOFT_EVENT_MODE_FAULT)) {
        adc_data_error_handler(WD_ADC_CH_VDC);
    }

    if (event & WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE) {
        adc_data_callback(WD_ADC_CH_VDC, WD_DIR_FULL);
        LOCAL_RES->Info->Debug_Time_Component_ADC_CH3 =
            wd_time_calc_us(&LOCAL_RES->Info->Debug_Time_Component_ADC_CH3_Stamp);
        LOCAL_RES->Info->Debug_Time_Component_ADC_CH3_Stamp = wd_time_get_us();
    }

    LOCAL_RES->Info->Event_CH3 = event;
}

#if WD_ADC_CH4_ENABLE
static void adc_sp_callback(uint32_t event) {
    if (event & (WD_SPI_SOFT_EVENT_DATA_LOST | WD_SPI_SOFT_EVENT_MODE_FAULT)) {
        adc_data_error_handler(WD_ADC_CH_SP);
    }

    if (event & WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE) {
        adc_data_callback(WD_ADC_CH_SP, WD_DIR_FULL);
        LOCAL_RES->Info->Debug_Time_Component_ADC_CH4 =
            wd_time_calc_us(&LOCAL_RES->Info->Debug_Time_Component_ADC_CH4_Stamp);
        LOCAL_RES->Info->Debug_Time_Component_ADC_CH4_Stamp = wd_time_get_us();
    }

    LOCAL_RES->Info->Event_CH4 = event;
}
#endif

// SPI1/2/3/5 하드웨어 리셋(RCC) — 기존 CMSIS-Driver SPI_PeripheralReset()과 동일 효과
static void adc_spi_peripheral_reset(SPI_TypeDef *reg) {
    if (reg == SPI1) {
        __HAL_RCC_SPI1_FORCE_RESET();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __HAL_RCC_SPI1_RELEASE_RESET();
    } else if (reg == SPI2) {
        __HAL_RCC_SPI2_FORCE_RESET();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __HAL_RCC_SPI2_RELEASE_RESET();
    } else if (reg == SPI3) {
        __HAL_RCC_SPI3_FORCE_RESET();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __HAL_RCC_SPI3_RELEASE_RESET();
    } else if (reg == SPI5) {
        __HAL_RCC_SPI5_FORCE_RESET();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __HAL_RCC_SPI5_RELEASE_RESET();
    }
}

// V/I/CH3/CH4 채널 공통 재초기화: 항상 Slave/8bit/CPOL0-CPHA0/MSB/HW-NSS로 고정
// (기존 CMSIS-Driver Initialize+PowerControl(OFF/FULL)+Control 시퀀스와 동일한 최종 상태를 만듦)
static void adc_spi_slave_reinit(SPI_HandleTypeDef *hspi, SPI_TypeDef *reg) {
    HAL_SPI_Abort(hspi);
    adc_spi_peripheral_reset(reg);
    HAL_SPI_DeInit(hspi);

    hspi->Init.Mode = SPI_MODE_SLAVE;
    hspi->Init.Direction = SPI_DIRECTION_2LINES;
    hspi->Init.DataSize = SPI_DATASIZE_8BIT;
    hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi->Init.NSS = SPI_NSS_HARD_INPUT;
    hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi->Init.TIMode = SPI_TIMODE_DISABLE;
    hspi->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

    adc_spi_peripheral_reset(reg);
    HAL_SPI_Init(hspi);
    HAL_SPI_Abort(hspi);
}

// SPI_STM32H7xx.c(CMSIS-Driver shim)의 콜백 디스패처가 SPI1/2/3/5 완료/에러를 직접 이 함수로 전달함
void wd_ad4134_spi_complete_handler(SPI_TypeDef *reg) {
    if (reg == LOCAL_RES->Reg_V) {
        adc_v_callback(WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE);
    } else if (reg == LOCAL_RES->Reg_I) {
        adc_i_callback(WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE);
    } else if (reg == LOCAL_RES->Reg_CH3) {
        adc_vdc_callback(WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE);
    }
#if WD_ADC_CH4_ENABLE
    else if (reg == LOCAL_RES->Reg_CH4) {
        adc_sp_callback(WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE);
    }
#endif
}

void wd_ad4134_spi_error_handler(SPI_TypeDef *reg, uint32_t event) {
    if (reg == LOCAL_RES->Reg_V) {
        adc_v_callback(event);
    } else if (reg == LOCAL_RES->Reg_I) {
        adc_i_callback(event);
    } else if (reg == LOCAL_RES->Reg_CH3) {
        adc_vdc_callback(event);
    }
#if WD_ADC_CH4_ENABLE
    else if (reg == LOCAL_RES->Reg_CH4) {
        adc_sp_callback(event);
    }
#endif
}

// SPI1/2/3/5는 이 컴포넌트만 사용하므로 HAL의 SPI 완료/에러 콜백(링크 전역 weak 심볼)을 여기서 바로 구현한다.
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    wd_ad4134_spi_complete_handler(hspi->Instance);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    wd_ad4134_spi_complete_handler(hspi->Instance);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    uint32_t error, event;

    error = HAL_SPI_GetError(hspi);

    event = 0;
    if (error & HAL_SPI_ERROR_MODF) {
        event |= WD_SPI_SOFT_EVENT_MODE_FAULT;
    }
    if (error & HAL_SPI_ERROR_OVR) {
        event |= WD_SPI_SOFT_EVENT_DATA_LOST;
    }

    if (event != 0) {
        wd_ad4134_spi_error_handler(hspi->Instance, event);
    }
}

static void SPI_Reinitialize(void) {
    HAL_SPI_Abort(LOCAL_RES->Handle_V);
    HAL_SPI_Abort(LOCAL_RES->Handle_I);
    HAL_SPI_Abort(LOCAL_RES->Handle_CH3);
#if WD_ADC_CH4_ENABLE
    HAL_SPI_Abort(LOCAL_RES->Handle_CH4);
#endif

    adc_spi_slave_reinit(LOCAL_RES->Handle_V, LOCAL_RES->Reg_V);
    adc_spi_slave_reinit(LOCAL_RES->Handle_I, LOCAL_RES->Reg_I);
    adc_spi_slave_reinit(LOCAL_RES->Handle_CH3, LOCAL_RES->Reg_CH3);
#if WD_ADC_CH4_ENABLE
    adc_spi_slave_reinit(LOCAL_RES->Handle_CH4, LOCAL_RES->Reg_CH4);
#endif
}

static void SPI_SetDMAMode(SPI_HandleTypeDef *hspi, uint32_t mode) {
    if (hspi->State == HAL_SPI_STATE_BUSY_TX || hspi->State == HAL_SPI_STATE_BUSY_RX) {
        HAL_SPI_Abort(hspi);
    }

    if (hspi->hdmatx && hspi->hdmatx->Init.Mode != mode) {
        HAL_DMA_Abort(hspi->hdmatx);
        HAL_DMA_DeInit(hspi->hdmatx);
        hspi->hdmatx->Init.Mode = mode;
        if (HAL_DMA_Init(hspi->hdmatx) != HAL_OK) {
            WD_BOARD_ERROR_HANDLER();
        }
    }

    if (hspi->hdmarx && hspi->hdmarx->Init.Mode != mode) {
        HAL_DMA_Abort(hspi->hdmarx);
        HAL_DMA_DeInit(hspi->hdmarx);
        hspi->hdmarx->Init.Mode = mode;
        if (HAL_DMA_Init(hspi->hdmarx) != HAL_OK) {
            WD_LOG_ERROR();
        }
    }
}


static int32_t adc_ad4134_initialize(wd_adc_signal_event_t cb_event) {
    if (LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "ADC_AD4134 Constructor");

    memset(LOCAL_RES->Info, 0, sizeof(wd_ad4134_info_t));

    LOCAL_RES->Info->Mode = WD_ADC_FILTER_FIR;
    LOCAL_RES->Info->Sampling_Number = WD_ADC_SAMPLING_NUMBER;
    LOCAL_RES->Info->Sampling_Cycle_Number = WD_ADC_SAMPLING_CYCLE_NUMBER;
    LOCAL_RES->Info->Sampling_Count = WD_ADC_SAMPLING_NUMBER / WD_ADC_SAMPLING_CYCLE_NUMBER;
    LOCAL_RES->Info->Sampling_Frequency = 1.0;
    LOCAL_RES->Info->Skip_Data_Callback = WD_ADC_DATA_SKIP_DATA_CALLBACK;
    LOCAL_RES->Info->Skip_Cycle = WD_ADC_DATA_SKIP_CYCLE;
    LOCAL_RES->Info->Cycle_Number = WD_ADC_SAMPLING_CYCLE_NUMBER;

    if (cb_event) {
        LOCAL_RES->Info->CB_Event = cb_event;
    }

    LOCAL_RES->Start();

    LOCAL_RES->Info->Flags = WDIF_FLAG_INITIALIZED;

    return (WD_CODE_OK);
}

static int32_t adc_ad4134_uninitialize(void) {
    return (WD_CODE_OK);
}

static int32_t adc_ad4134_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "ADC_AD4134 PowerControl:WD_POWER_OFF");

        LOCAL_RES->Info->Flags &= ~(WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
    } break;

    case WD_POWER_FULL: {
        if ((LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) == 0U) {
            return (WD_CODE_ERROR);
        } else if (LOCAL_RES->Info->Flags & WDIF_FLAG_POWERED) {
            return (WD_CODE_OK);
        }

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "ADC_AD4134 PowerControl:WD_POWER_FULL");

        {
            wd_softspi_power_control(LOCAL_RES->Drv, WD_POWER_OFF);
            WD_CHECK(wd_softspi_initialize(LOCAL_RES->Drv, wd_adc_ad4134_callback));
            WD_CHECK(wd_softspi_power_control(LOCAL_RES->Drv, WD_POWER_FULL));
            WD_CHECK(wd_softspi_control(LOCAL_RES->Drv,
                                        WD_SPI_SOFT_MODE_MASTER | WD_SPI_SOFT_DATA_BITS(8) |
                                            WD_SPI_SOFT_CPOL1_CPHA0 | WD_SPI_SOFT_MSB_LSB |
                                            WD_SPI_SOFT_SS_MASTER_HW_OUTPUT,
                                        LOCAL_RES->Capabilities.BaudRate));
            WD_CHECK(wd_softspi_control(LOCAL_RES->Drv, WD_SPI_SOFT_SET_DEFAULT_TX_VALUE, 0xCC));

            __HAL_RCC_SPI1_CLK_ENABLE();
            __HAL_RCC_DMA1_CLK_ENABLE();

            adc_spi_slave_reinit(LOCAL_RES->Handle_V, LOCAL_RES->Reg_V);

            __HAL_RCC_SPI2_CLK_ENABLE();
            __HAL_RCC_DMA2_CLK_ENABLE();

            adc_spi_slave_reinit(LOCAL_RES->Handle_I, LOCAL_RES->Reg_I);


            __HAL_RCC_SPI3_CLK_ENABLE();
            __HAL_RCC_DMA2_CLK_ENABLE();

            adc_spi_slave_reinit(LOCAL_RES->Handle_CH3, LOCAL_RES->Reg_CH3);

#if WD_ADC_CH4_ENABLE
            __HAL_RCC_SPI5_CLK_ENABLE();
            __HAL_RCC_DMA2_CLK_ENABLE();

            adc_spi_slave_reinit(LOCAL_RES->Handle_CH4, LOCAL_RES->Reg_CH4);
#endif
        }

        LOCAL_RES->Info->Flags |= (WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);

        WD_PRINT_INFO(WD_MSG_INFO_LOG, "ADC Data Initialize");

        wd_ad4134_initialize();
    } break;

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);
    default:
        break;
    }

    return (WD_CODE_OK);
}

static int32_t adc_ad4134_start_conversion(void *v_buf, void *i_buf, void *ch3_buf, void *ch4_buf,
                                           uint32_t num, uint32_t arg)
//static int32_t adc_ad4134_start_conversion (void *v_buf, void *i_buf, uint32_t num, uint32_t arg)
{
    int32_t ret = WD_CODE_OK;
    if (!((v_buf != 0) || (i_buf != 0) || (ch3_buf != 0) || (ch4_buf != 0) || (num != 0))) {
        WD_LOG_ERROR();
        return (WD_CODE_ERROR);
    }

    LOCAL_RES->Lock();

    LOCAL_RES->Info->Xfer.V_Buf = v_buf;
    LOCAL_RES->Info->Xfer.I_Buf = i_buf;
    LOCAL_RES->Info->Xfer.CH3_Buf = ch3_buf;
#if WD_ADC_CH4_ENABLE
    LOCAL_RES->Info->Xfer.CH4_Buf = ch4_buf;
#endif

    LOCAL_RES->Info->Xfer.Mode = arg;
    LOCAL_RES->Info->Xfer.Ignore = WD_ADC_AD4134_XFER_IGNORE_NUMBER;

    LOCAL_RES->Info->Xfer.Num = num;                                // FFT Size
    LOCAL_RES->Info->Xfer.Method = LOCAL_RES->Info->Sampling_Under; // UnderSampling
    LOCAL_RES->Info->Xfer.Offset = WD_ADC_DATA_STABLE_NUMBER * WD_ADC_DATA_SIZE;
    memset((void *)&LOCAL_RES->Info->Xfer.Cnt, 0, sizeof(LOCAL_RES->Info->Xfer.Cnt));

    /* 주파수 기반 Skip 콜백 수 결정:
   *   ≥ 1kHz  → 기준값(CLI 설정, 기본 128) — 고주파 불량 샘플 최대 제거
   *   100Hz~  → 기준값 / 2                 — 중주파 절충
   *   < 100Hz → 0                          — 저주파/VDC 측정 영향 없음        */
    {
        uint32_t _base = LOCAL_RES->Info->Skip_Data_Callback;
        fpu_t _freq = LOCAL_RES->Info->Frequency;
        if (_freq >= (fpu_t)1000.0) {
            LOCAL_RES->Info->Xfer.Skip_Data_Callback_Num = _base;
        } else if (_freq >= (fpu_t)100.0) {
            LOCAL_RES->Info->Xfer.Skip_Data_Callback_Num = _base >> 1U;
        } else {
            LOCAL_RES->Info->Xfer.Skip_Data_Callback_Num = 0U;
        }
    }
    LOCAL_RES->Info->Xfer.Skip_Cycle_Num = LOCAL_RES->Info->Skip_Cycle;
    memset((void *)&LOCAL_RES->Info->Xfer.Skip_Data_Callback_Cnt, 0,
           sizeof(LOCAL_RES->Info->Xfer.Skip_Data_Callback_Cnt));
    memset((void *)&LOCAL_RES->Info->Xfer.Skip_Cycle_Cnt, 0,
           sizeof(LOCAL_RES->Info->Xfer.Skip_Cycle_Cnt));

    LOCAL_DRV->Control(WD_ADC_CONVERSION_START, arg, 0);

    LOCAL_RES->Unlock();

    return (ret);
}

static int32_t adc_ad4134_poll_conversion(uint32_t timeout, uint32_t arg) {
    uint32_t __time_start = wd_time_get_ms();

    UNUSED(arg);

    LOCAL_RES->Lock();

    do {
//    if(LOCAL_RES->Info->Status_V.Conversion_Finish && LOCAL_RES->Info->Status_I.Conversion_Finish)
#if WD_ADC_CH4_ENABLE
        if (LOCAL_RES->Info->Status_V.Conversion_Finish &&
            LOCAL_RES->Info->Status_I.Conversion_Finish &&
            LOCAL_RES->Info->Status_CH3.Conversion_Finish &&
            LOCAL_RES->Info->Status_CH4.Conversion_Finish)
#else
        if (LOCAL_RES->Info->Status_V.Conversion_Finish &&
            LOCAL_RES->Info->Status_I.Conversion_Finish &&
            LOCAL_RES->Info->Status_CH3
                .Conversion_Finish) // && LOCAL_RES->Info->Status_CH4.Conversion_Finish)
#endif
        {
            LOCAL_RES->Info->Status_V.Conversion_Finish = false;
            LOCAL_RES->Info->Status_I.Conversion_Finish = false;
            LOCAL_RES->Info->Status_CH3.Conversion_Finish = false;
#if WD_ADC_CH4_ENABLE
            LOCAL_RES->Info->Status_CH4.Conversion_Finish = false; // ch4 아직 사용 안함
#endif
            break;
        }

        if (wd_time_calc_ms(&__time_start) >= timeout) {
            WD_LOG_ERROR("_ADC_AD4134_Poll_Conversion Timeout");

            LOCAL_DRV->Control(WD_ADC_RESET, 0, 0);
            break;
        }
    } while (1);

    LOCAL_RES->Unlock();

    LOCAL_RES->Info->Debug_Time_Component_ADC_Read =
        wd_time_calc_us(&LOCAL_RES->Info->Debug_Time_Component_ADC_Read_Stamp);

    return (WD_CODE_OK);
}

static int32_t adc_ad4134_stop_conversion(uint32_t arg) {
    UNUSED(arg);

    LOCAL_RES->Lock();

    if (arg & WDIF_ARG_RESET) {
        ad713x_set_odr(&ad713x_dev_instance, 0);
    }

    LOCAL_DRV->Control(WD_ADC_CONVERSION_STOP, 0, 0);
    LOCAL_DRV->Control(WD_ADC_CONVERSION_RESULT, 0, 0);
    LOCAL_DRV->Control(WD_ADC_CONVERSION_CALC, 0, 0); // offset, rms, peak 값 등 계산

    LOCAL_RES->Unlock();

    return (WD_CODE_OK);
}

static int32_t adc_ad4134_control(uint32_t control, uint32_t arg, fpu_t value) {
    //arg = size ,  value = freq
    switch (control & WD_ADC_CONTROL_MSK) {
    case WD_ADC_RESET: {
        ad713x_set_odr(&ad713x_dev_instance, 0);

        HAL_SPI_Abort(LOCAL_RES->Handle_V);
        HAL_SPI_Abort(LOCAL_RES->Handle_I);
        HAL_SPI_Abort(LOCAL_RES->Handle_CH3);
#if WD_ADC_CH4_ENABLE
        HAL_SPI_Abort(LOCAL_RES->Handle_CH4); // ch4 아직 사용 안함
#endif
    } break;

    case WD_ADC_SET_SAMPLING_RATE: // arg = sample number, value = frequency
    {
        uint32_t output_samples;
        uint32_t min_cycle;
        uint32_t max_cycle_candidate;
        uint32_t max_cycle_by_freq;
        uint32_t cycle;
        uint32_t best_cycle = 0;

        uint32_t samples_per_cycle;
        uint32_t best_samples_per_cycle = 0;

        uint32_t under;
        uint32_t best_under = 1;

        uint32_t under_max_by_buffer;

        fpu_t input_freq;
        fpu_t logical_fs;
        fpu_t best_logical_fs = (fpu_t)0.0;

        fpu_t adc_odr;
        fpu_t best_adc_odr = (fpu_t)0.0;

        fpu_t min_odr;
        fpu_t max_odr;

        if (arg == 0) {
            arg = LOCAL_RES->Info->Sampling_Number;
        }

        output_samples = arg;
        input_freq = value;

        if ((output_samples == 0) || (output_samples > WD_ADC_SAMPLING_NUMBER)) {
            WD_LOG_ERROR("WD_ADC_SET_SAMPLING Out of Range !!!");
            return (WD_CODE_ERROR);
        }

        if (input_freq <= (fpu_t)0.0) {
            WD_LOG_ERROR("WD_ADC_SET_SAMPLING Frequency Error !!!");
            return (WD_CODE_ERROR);
        }

        if (!((output_samples == 256) || (output_samples == 512) || (output_samples == 1024))) {
            WD_LOG_ERROR("WD_ADC_SET_SAMPLING Sample Number Error !!! [%u]", output_samples);
            return (WD_CODE_ERROR);
        }

        min_cycle = LOCAL_RES->Info->Cycle_Number;
        if (min_cycle < 1) {
            min_cycle = 1;
        }

        if (LOCAL_RES->Info->Mode == WD_ADC_FILTER_FIR) {
            max_odr = (fpu_t)375001.0;
        } else {
            max_odr = (fpu_t)1500001.0;
        }

        //       기본 최소 ODR 선정. - 일반 구간 : 512 SPS,  -저주파 구간 : 256 SPS
        if (input_freq <= (fpu_t)0.125) {
            min_odr = (fpu_t)256.0;
        } else {
            min_odr = (fpu_t)512.0;
        }

        //      DMA buffer 제한
        //      valid samples per half-buffer = 8192 / under
        under_max_by_buffer =
            (uint32_t)(sizeof(LOCAL_RES->Info->V_Buf) / WD_ADC_DATA_SIZE) / output_samples;

        if (under_max_by_buffer < 1U) {
            under_max_by_buffer = 1U;
        }
        //      UnderSampling 최대 8까지만 허용
        if (under_max_by_buffer > 8U) {
            under_max_by_buffer = 8U;
        }

        max_cycle_candidate = output_samples / WD_ADC_DATA_SAMPLE_MIN_NUMBER;

        if (max_cycle_candidate < 1U) {
            max_cycle_candidate = 1U;
        }

        //      주파수별 최대 Cycle 제한
        max_cycle_by_freq = adc_ad4134_get_max_cycle_by_frequency(input_freq);

        if (max_cycle_candidate > max_cycle_by_freq) {
            max_cycle_candidate = max_cycle_by_freq;
        }

        //       최소 cycle보다 작은 경우는 최소 cycle까지 올림
        if (max_cycle_candidate < min_cycle) {
            max_cycle_candidate = min_cycle;
        }

        //       2의 거듭제곱 cycle만 허용
        cycle = 1U;
        while ((cycle << 1U) <= max_cycle_candidate) {
            cycle <<= 1U;
        }

        max_cycle_candidate = cycle;

        //      최대 cycle → 최소 cycle 순으로 탐색
        for (cycle = max_cycle_candidate; cycle >= min_cycle; cycle >>= 1) {
            if (output_samples % cycle) {
                if (cycle == 1) {
                    break;
                }
                continue;
            }

            samples_per_cycle = output_samples / cycle;

            if (samples_per_cycle < WD_ADC_DATA_SAMPLE_MIN_NUMBER) {
                if (cycle == 1) {
                    break;
                }
                continue;
            }

            logical_fs = (fpu_t)samples_per_cycle * input_freq;

            if (((fpu_t)1.0 / input_freq) < (fpu_t)0.00005) {
                if (cycle == 1) {
                    break;
                }
                continue;
            }

            under = 1;
            adc_odr = logical_fs;

            while (adc_odr < min_odr) {
                under <<= 1;

                if (under > under_max_by_buffer) {
                    break;
                }

                adc_odr = logical_fs * (fpu_t)under;
            }

            if (under > under_max_by_buffer) {
                if (cycle == 1) {
                    break;
                }
                continue;
            }

            if (adc_odr > max_odr) {
                if (cycle == 1) {
                    break;
                }
                continue;
            }

            best_cycle = cycle;
            best_samples_per_cycle = samples_per_cycle;
            best_under = under;
            best_logical_fs = logical_fs;
            best_adc_odr = adc_odr;

            break;
        }

        if (best_cycle == 0) {
            WD_LOG_ERROR("WD_ADC_SET_SAMPLING No Valid Plan !!! "
                         "[Freq:%f][Sample:%u][MinCycle:%u][UnderMax:%u]",
                         (double)input_freq, output_samples, min_cycle, under_max_by_buffer);

            return (WD_CODE_ERROR);
        }

        HAL_SPI_Abort(LOCAL_RES->Handle_V);
        HAL_SPI_Abort(LOCAL_RES->Handle_I);
        HAL_SPI_Abort(LOCAL_RES->Handle_CH3);
#if WD_ADC_CH4_ENABLE
        HAL_SPI_Abort(LOCAL_RES->Handle_CH4);
#endif

        ad713x_set_odr(&ad713x_dev_instance, best_adc_odr);
        ad713x_get_odr(&ad713x_dev_instance, NULL);

        LOCAL_RES->Info->Frequency = input_freq;
        LOCAL_RES->Info->Sampling_Number = output_samples;
        LOCAL_RES->Info->Sampling_Cycle_Number = best_cycle;
        LOCAL_RES->Info->Sampling_Count = best_samples_per_cycle;
        LOCAL_RES->Info->Sampling_Under = best_under;
        LOCAL_RES->Info->Sampling_Frequency = best_logical_fs;
        LOCAL_RES->Info->Sampling_Rate = best_logical_fs / (fpu_t)output_samples;

        WD_LOG_DEBUG("[ADC Info]   Test Frequency : %f    ODR : %f    Sample/Cycle : %d    "
                     "UnderSampling : %d    Cycle : %d",
                     (double)LOCAL_RES->Info->Frequency, (double)best_adc_odr,
                     LOCAL_RES->Info->Sampling_Count, LOCAL_RES->Info->Sampling_Under,
                     LOCAL_RES->Info->Sampling_Cycle_Number);

        WD_PRINT_DEBUG(
            WD_MSG_DEBUG_CRLF,
            "[ADC AC Set Sample Rate]    Test Frequency [%f]    UnderSampling [%d]    ODR [%f]    "
            "Frequency [%f]    Sample Number [%u]    Sample Count [%u]    Sample Cycle [%u]",
            (double)LOCAL_RES->Info->Frequency, LOCAL_RES->Info->Sampling_Under,
            (double)best_adc_odr, (double)LOCAL_RES->Info->Sampling_Frequency,
            LOCAL_RES->Info->Sampling_Number, LOCAL_RES->Info->Sampling_Count,
            LOCAL_RES->Info->Sampling_Cycle_Number);
    } break;

    case WD_ADC_SET_FILTER: // arg = filter type, value = filter option
    {
        enum ad713x_dig_filter_sel mode;
        uint8_t fir_bw_sel = 0;

        int8_t i;

        switch (arg) {
        case WD_ADC_FILTER_FIR: {
            mode = FIR;
            fir_bw_sel = (value <= (fpu_t)WD_ADC_FILTER_FIR_BANDWITDH_1) ? 1 : 0;
        } break;

        case WD_ADC_FILTER_SINC6:
            mode = SINC6;
            break;
        case WD_ADC_FILTER_SINC3:
            mode = SINC3;
            break;
        default:
            return (WD_CODE_ERROR);
        }

        for (i = CH3; i >= 0; i--) {
            ad713x_dig_filter_sel_ch(&ad713x_dev_instance, mode, (enum ad713x_channels)i);
            ad713x_wideband_bw_sel(&ad713x_dev_instance, (enum ad713x_channels)i, fir_bw_sel);
        }

        LOCAL_RES->Info->Mode = arg;
    } break;

    case WD_ADC_SET_PHASE_DELAY: // arg = phase delay type, value = delay clock
    {
        enum ad717x_mpc_clkdel delay;

        switch (arg) {
        case CH0:
        case CH1:
            break;

        default:
            WD_LOG_ERROR("WD_ADC_SET_SAMPLING Out of Range !!!");
            return (WD_CODE_ERROR);
        }

        if (value >= (fpu_t)2.0) {
            delay = DELAY_2_CLOCKS;
        } else if (value >= (fpu_t)1.0) {
            delay = DELAY_1_CLOCKS;
        } else {
            delay = DELAY_NONE;
        }

        ad713x_mag_phase_clk_delay_chan(&ad713x_dev_instance, (enum ad713x_channels)arg, delay);
    } break;

    case WD_ADC_CONVERSION_START: // arg = WD_ADC_CONVERSION_MODE_SINGLE, WD_ADC_CONVERSION_MODE_CONTINUOUS
    {
        uint8_t *v_buf;
        uint8_t *i_buf;
        uint8_t *ch3_buf;
#if WD_ADC_CH4_ENABLE
        uint8_t *ch4_buf;
#endif
        uint32_t num;
        //        uint32_t transfer_item;

        memset((void *)&LOCAL_RES->Info->Status_V, 0, sizeof(LOCAL_RES->Info->Status_V));
        memset((void *)&LOCAL_RES->Info->Status_I, 0, sizeof(LOCAL_RES->Info->Status_I));
        memset((void *)&LOCAL_RES->Info->Status_CH3, 0, sizeof(LOCAL_RES->Info->Status_CH3));
#if WD_ADC_CH4_ENABLE
        memset((void *)&LOCAL_RES->Info->Status_CH4, 0,
               sizeof(LOCAL_RES->Info->Status_CH4)); // ch4 아직 사용 안함
#endif

        {
            uint32_t raw_sample_count;
            uint32_t raw_total_bytes;
            uint32_t raw_half_bytes;

            v_buf = (uint8_t *)LOCAL_RES->Info->V_Buf;
            i_buf = (uint8_t *)LOCAL_RES->Info->I_Buf;
            ch3_buf = (uint8_t *)LOCAL_RES->Info->CH3_Buf;
#if WD_ADC_CH4_ENABLE
            ch4_buf = (uint8_t *)LOCAL_RES->Info->CH4_Buf; // ch4 아직 사용 안함
#endif
            /*
           * 필요한 실제 raw sample 수만큼만 DMA 수신
           * raw_sample_count = output_sample_count * under_sampling
           *
           * 예)
           *   1024 sample, under=1  -> 1024 raw sample
           *   1024 sample, under=8  -> 8192 raw sample
           */
            raw_sample_count = LOCAL_RES->Info->Xfer.Num * LOCAL_RES->Info->Xfer.Method;
            raw_total_bytes = raw_sample_count * WD_ADC_DATA_SIZE;
            raw_half_bytes = raw_total_bytes / 2U;

            /*
           * half/full callback 기반이므로 전체 길이는 짝수 half로 나뉘어야 함
           * 현재 Sampling_Number가 256/512/1024, Method가 2의 거듭제곱이라면
           * raw_total_bytes는 항상 짝수이며 32-byte alignment도 만족하는 편이지만
           * 방어적으로 체크
           */
            if ((raw_total_bytes == 0U) || (raw_half_bytes == 0U)) {
                WD_LOG_ERROR("ADC DMA Size Error !!!");
                return (WD_CODE_ERROR);
            }

            if ((raw_total_bytes & 0x1U) != 0U) {
                WD_LOG_ERROR("ADC DMA Size Not Even !!! [%u]", raw_total_bytes);
                return (WD_CODE_ERROR);
            }

            if (raw_total_bytes > sizeof(LOCAL_RES->Info->V_Buf)) {
                WD_LOG_ERROR("ADC DMA Size Overflow !!! [%u > %u]", raw_total_bytes,
                             (uint32_t)sizeof(LOCAL_RES->Info->V_Buf));
                return (WD_CODE_ERROR);
            }

            if ((DCache_IsAligned((void *)WD_CONF_DMA_ALIGNMENT_SIZE, raw_total_bytes) == 0) ||
                (DCache_IsAligned((void *)WD_CONF_DMA_ALIGNMENT_SIZE, raw_half_bytes) == 0)) {
                WD_LOG_ERROR("ADC DMA Aligned Error #1 !!!");
                WD_LOG_INFO("ADC DMA [Half] %u    [Full] %u", raw_half_bytes, raw_total_bytes);
                WD_LOG_INFO(
                    "ADC DMA Aligned Adjust [Half] %u -> %u    [Full] %u -> %u", raw_half_bytes,
                    WD_ALIGNED_UP(raw_half_bytes, WD_CONF_DMA_ALIGNMENT_SIZE), raw_total_bytes,
                    WD_ALIGNED_UP(raw_total_bytes, WD_CONF_DMA_ALIGNMENT_SIZE));
                return (WD_CODE_ERROR);
            }

            LOCAL_RES->Info->Xfer.Under_Num = raw_half_bytes;
            num = raw_total_bytes;
        }
        if (num == 0) {
            WD_LOG_ERROR("ADC Xfer Number 0 !!!");
            return (WD_CODE_ERROR);
        }

        memset(v_buf, 0, num);
        memset(i_buf, 0, num);
        memset(ch3_buf, 0, num);
#if WD_ADC_CH4_ENABLE
        memset(ch4_buf, 0, num); // ch4 아직 사용 안함
#endif

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        if ((DCache_IsAligned(v_buf, WD_CONF_DMA_ALIGNMENT_SIZE) == 0) ||
            (DCache_IsAligned(i_buf, WD_CONF_DMA_ALIGNMENT_SIZE) == 0) ||
            (DCache_IsAligned(ch3_buf, WD_CONF_DMA_ALIGNMENT_SIZE) == 0))
#if WD_ADC_CH4_ENABLE
        || (DCache_IsAligned(ch4_buf, WD_CONF_DMA_ALIGNMENT_SIZE) == 0))	// ch4 아직 사용 안함, 활성화 시 괄호 수정해야함.
#endif
        {
            WD_LOG_ERROR("ADC DMA Aligned Error #2 !!!");
            return (WD_CODE_ERROR);
        }

        if ((DCache_IsAligned((void *)WD_CONF_DMA_ALIGNMENT_SIZE, num) == 0) ||
            (DCache_IsAligned((void *)WD_CONF_DMA_ALIGNMENT_SIZE, num) == 0)) {
            WD_LOG_INFO("ADC DMA Aligned Adjust %d → %d", num,
                        WD_ALIGNED_UP(num, WD_CONF_DMA_ALIGNMENT_SIZE));
        }

        SCB_CleanDCache_by_Addr((uint32_t *)v_buf,
                                (int32_t)WD_ALIGNED_UP(num, WD_CONF_DMA_ALIGNMENT_SIZE));
        SCB_CleanDCache_by_Addr((uint32_t *)i_buf,
                                (int32_t)WD_ALIGNED_UP(num, WD_CONF_DMA_ALIGNMENT_SIZE));
        SCB_CleanDCache_by_Addr((uint32_t *)ch3_buf,
                                (int32_t)WD_ALIGNED_UP(num, WD_CONF_DMA_ALIGNMENT_SIZE));
#if WD_ADC_CH4_ENABLE
        SCB_CleanDCache_by_Addr(
            (uint32_t *)ch4_buf,
            (int32_t)WD_ALIGNED_UP(num, WD_CONF_DMA_ALIGNMENT_SIZE)); // ch4 아직 사용 안함
#endif
#endif

        SPI_Reinitialize();

#if 1
        SPI_SetDMAMode(RTE_COMPONENT_ADC_AD4134_V_DRIVER_HANDLE, DMA_CIRCULAR);
        SPI_SetDMAMode(RTE_COMPONENT_ADC_AD4134_I_DRIVER_HANDLE, DMA_CIRCULAR);
        SPI_SetDMAMode(RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_HANDLE, DMA_CIRCULAR);
#if WD_ADC_CH4_ENABLE
        SPI_SetDMAMode(RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_HANDLE,
                       DMA_CIRCULAR); //ch4 아직 사용 안함
#endif
#else
        SPI_SetDMAMode(RTE_COMPONENT_ADC_AD4134_V_DRIVER_HANDLE,
                       (LOCAL_RES->Info->Xfer.Method > 1) ? DMA_CIRCULAR : DMA_NORMAL);
        SPI_SetDMAMode(RTE_COMPONENT_ADC_AD4134_I_DRIVER_HANDLE,
                       (LOCAL_RES->Info->Xfer.Method > 1) ? DMA_CIRCULAR : DMA_NORMAL);
#endif

        {
            // V/I/CH3 채널 동기화 (Phase Halving 버그 수정)
            // HAL_SPI_Receive_DMA가 내부에서 SPE를 켜면 SPI가 DCLK에 즉시 반응
            // 순차 호출 시 V/I 간 DCLK edge 차이로 1샘플 어긋남 발생
            //
            // 해결: SCK 핀을 GPIO Input으로 전환하여 DCLK를 SPI에서 차단
            // → HAL이 SPE를 켜도 SPI는 클럭을 못 받음 → shift register 깨끗
            // → 3채널 DMA 설정 완료 후 SCK를 동시에 AF 복원
            // → 모든 채널이 동일한 DCLK edge부터 수집 시작

            // SPI1_SCK=PG11, SPI2_SCK=PD3, SPI3_SCK=PC10
            // MODER: 00=Input, 10=AF — 각 핀은 2비트씩 차지
            uint32_t moder_g_save = GPIOG->MODER;
            uint32_t moder_d_save = GPIOD->MODER;
            uint32_t moder_c_save = GPIOC->MODER;

            // 현재 ODR 읽기 — SCK restore 직전 decimation counter reset에 사용
            fpu_t current_odr = 0.0f;
            ad713x_get_odr(&ad713x_dev_instance, &current_odr);

            // SCK 핀을 GPIO Input으로 전환 (DCLK이 SPI에 도달 못함)
            GPIOG->MODER &= ~(3UL << (11 * 2)); // PG11 → Input
            GPIOD->MODER &= ~(3UL << (3 * 2));  // PD3  → Input
            GPIOC->MODER &= ~(3UL << (10 * 2)); // PC10 → Input

            // DMA + SPI 설정 (SPE 활성화되지만 DCLK 차단 상태)
            HAL_SPI_Receive_DMA(LOCAL_RES->Handle_V, v_buf,
                                (uint16_t)(WD_ALIGNED_UP(num, WD_CONF_DMA_ALIGNMENT_SIZE)));
            HAL_SPI_Receive_DMA(LOCAL_RES->Handle_I, i_buf,
                                (uint16_t)(WD_ALIGNED_UP(num, WD_CONF_DMA_ALIGNMENT_SIZE)));
            HAL_SPI_Receive_DMA(LOCAL_RES->Handle_CH3, ch3_buf,
                                (uint16_t)(WD_ALIGNED_UP(num, WD_CONF_DMA_ALIGNMENT_SIZE)));
#if WD_ADC_CH4_ENABLE
            HAL_SPI_Receive_DMA(
                LOCAL_RES->Handle_CH4, ch4_buf,
                (uint16_t)(WD_ALIGNED_UP(num, WD_CONF_DMA_ALIGNMENT_SIZE))); //ch4 아직 사용 안함
#endif

            // AD4134 decimation counter reset: TRANSFER_REGISTER write → 다음 DCLK burst가 bit 0부터 시작
            // SCK 차단 중이므로 SPI는 이 기간의 DCLK를 받지 않음 → byte alignment 보장
            ad713x_set_odr(&ad713x_dev_instance, current_odr);

            // 3채널 SCK를 동시에 AF 복원 — 새 사이클 bit 0부터 DCLK 수신
            __disable_irq();
            GPIOG->MODER = moder_g_save;
            GPIOD->MODER = moder_d_save;
            GPIOC->MODER = moder_c_save;
            __enable_irq();
        }

        LOCAL_RES->Info->Debug_Time_Component_ADC_Read_Stamp = wd_time_get_us();
        LOCAL_RES->Info->Debug_Time_Component_ADC_V_Stamp = wd_time_get_us();
        LOCAL_RES->Info->Debug_Time_Component_ADC_I_Stamp = wd_time_get_us();
        LOCAL_RES->Info->Debug_Time_Component_ADC_CH3_Stamp = wd_time_get_us();
#if WD_ADC_CH4_ENABLE
        LOCAL_RES->Info->Debug_Time_Component_ADC_CH4_Stamp = wd_time_get_us(); //ch4 아직 사용 안함
#endif

        LOCAL_RES->Info->Status_V.Read = true;
        LOCAL_RES->Info->Status_I.Read = true;
        LOCAL_RES->Info->Status_CH3.Read = true;
#if WD_ADC_CH4_ENABLE
        LOCAL_RES->Info->Status_CH4.Read = true; //ch4 아직 사용 안함
#endif
    } break;

    case WD_ADC_CONVERSION_STOP: {
        memset((void *)&LOCAL_RES->Info->Status_V, 0, sizeof(LOCAL_RES->Info->Status_V));
        memset((void *)&LOCAL_RES->Info->Status_I, 0, sizeof(LOCAL_RES->Info->Status_I));
        memset((void *)&LOCAL_RES->Info->Status_CH3, 0, sizeof(LOCAL_RES->Info->Status_CH3));
#if WD_ADC_CH4_ENABLE
        memset((void *)&LOCAL_RES->Info->Status_CH4, 0,
               sizeof(LOCAL_RES->Info->Status_CH4)); //ch4 아직 사용 안함
#endif
        SPI_Reinitialize();
        //        LOCAL_RES->Info->Sampling_Cycle_Number = WD_ADC_SAMPLING_CYCLE_NUMBER;  	//여기에 설정된 사이클 주기로 변경
        //        LOCAL_RES->Info->Sampling_Cycle_Number = LOCAL_RES->Info->Cycle_Number;  		//여기에 설정된 사이클 주기로 변경
    } break;

    case WD_ADC_CONVERSION_RESULT: {
#if 1
        uint8_t *v_buf;
        uint8_t *i_buf;
        uint8_t *ch3_buf;
        uint32_t i;
        //        uint32_t num = (LOCAL_RES->Info->Xfer.Num >= WD_ADC_DATA_SAMPLE_NUMBER) ? WD_ADC_DATA_SAMPLE_NUMBER : LOCAL_RES->Info->Xfer.Num;
        uint32_t num = (LOCAL_RES->Info->Xfer.Num >= WD_ADC_SAMPLING_NUMBER)
                           ? WD_ADC_SAMPLING_NUMBER
                           : LOCAL_RES->Info->Xfer.Num;

        v_buf = (uint8_t *)LOCAL_RES->Info->V_Buf;
        i_buf = (uint8_t *)LOCAL_RES->Info->I_Buf;
        ch3_buf = (uint8_t *)LOCAL_RES->Info->CH3_Buf;
#if WD_ADC_CH4_ENABLE
        uint8_t *ch4_buf;
        ch4_buf = (uint8_t *)LOCAL_RES->Info->CH4_Buf; //ch4 아직 사용 안함
#endif

        if (LOCAL_RES->Info->Xfer.Method >
            1) // under sampling 일 경우 데이터 사이즈 * Method만큼 건너뛰어서 읽어옴
        {
            for (i = 0; i < num; i++) {
                LOCAL_RES->Info->Xfer.V_Buf[i] =
                    (int32_t)(((v_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                              ((uint32_t)v_buf[0] << 16) | ((uint32_t)v_buf[1] << 8) |
                              ((uint32_t)v_buf[2] << 0));
                LOCAL_RES->Info->Xfer.I_Buf[i] =
                    (int32_t)(((i_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                              ((uint32_t)i_buf[0] << 16) | ((uint32_t)i_buf[1] << 8) |
                              ((uint32_t)i_buf[2] << 0));
                LOCAL_RES->Info->Xfer.I_Buf[i] *= -1;
                v_buf += WD_ADC_DATA_SIZE * LOCAL_RES->Info->Xfer.Method;
                i_buf += WD_ADC_DATA_SIZE * LOCAL_RES->Info->Xfer.Method;
                LOCAL_RES->Info->Xfer.CH3_Buf[i] =
                    (int32_t)(((ch3_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                              ((uint32_t)ch3_buf[0] << 16) | ((uint32_t)ch3_buf[1] << 8) |
                              ((uint32_t)ch3_buf[2] << 0));
                ch3_buf += WD_ADC_DATA_SIZE * LOCAL_RES->Info->Xfer.Method;
#if WD_ADC_CH4_ENABLE
                LOCAL_RES->Info->Xfer.CH4_Buf[i] =
                    (int32_t)(((ch4_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                              ((uint32_t)ch4_buf[0] << 16) | ((uint32_t)ch4_buf[1] << 8) |
                              ((uint32_t)ch4_buf[2] << 0));
                ch4_buf += WD_ADC_DATA_SIZE * LOCAL_RES->Info->Xfer.Method;
#endif
            }
        } else {
            for (i = 0; i < num; i++) // under sampling 아닐 경우 데이터 사이즈만큼 offset
            {
                LOCAL_RES->Info->Xfer.V_Buf[i] =
                    (int32_t)(((v_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                              ((uint32_t)v_buf[0] << 16) | ((uint32_t)v_buf[1] << 8) |
                              ((uint32_t)v_buf[2] << 0));
                LOCAL_RES->Info->Xfer.I_Buf[i] =
                    (int32_t)(((i_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                              ((uint32_t)i_buf[0] << 16) | ((uint32_t)i_buf[1] << 8) |
                              ((uint32_t)i_buf[2] << 0));
                LOCAL_RES->Info->Xfer.I_Buf[i] *= -1;
                v_buf += WD_ADC_DATA_SIZE;
                i_buf += WD_ADC_DATA_SIZE;
                LOCAL_RES->Info->Xfer.CH3_Buf[i] =
                    (int32_t)(((ch3_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                              ((uint32_t)ch3_buf[0] << 16) | ((uint32_t)ch3_buf[1] << 8) |
                              ((uint32_t)ch3_buf[2] << 0));
                ch3_buf += WD_ADC_DATA_SIZE * LOCAL_RES->Info->Xfer.Method;
#if WD_ADC_CH4_ENABLE
                LOCAL_RES->Info->Xfer.CH4_Buf[i] =
                    (int32_t)(((ch4_buf[0] & 0x80) ? 0xFF000000 : 0x00000000) |
                              ((uint32_t)ch4_buf[0] << 16) | ((uint32_t)ch4_buf[1] << 8) |
                              ((uint32_t)ch4_buf[2] << 0));
                ch4_buf += WD_ADC_DATA_SIZE * LOCAL_RES->Info->Xfer.Method;
#endif
            }
        }
#endif
    } break;

    case WD_ADC_CONVERSION_PRINT: {
        uint32_t i;
        //      uint32_t num = (LOCAL_RES->Info->Xfer.Num >= WD_ADC_DATA_SAMPLE_NUMBER) ? WD_ADC_DATA_SAMPLE_NUMBER : LOCAL_RES->Info->Xfer.Num;
        uint32_t num = (LOCAL_RES->Info->Xfer.Num >= WD_ADC_SAMPLING_NUMBER)
                           ? WD_ADC_SAMPLING_NUMBER
                           : LOCAL_RES->Info->Xfer.Num;

        if (!arg) {
            WD_PRINT(
                WD_MSG_TITLE_CRLF,
                "[ADC AC Set Sample Rate]    Test Frequency [%f]    UnderSampling [%d]    ODR [%f] "
                "   Frequency [%f]    Sample Number [%u]    Sample Count [%u]    Sample Cycle [%u]",
                (double)LOCAL_RES->Info->Frequency, LOCAL_RES->Info->Sampling_Under,
                (double)(LOCAL_RES->Info->Sampling_Frequency *
                         (float)LOCAL_RES->Info->Sampling_Under),
                LOCAL_RES->Info->Sampling_Frequency, LOCAL_RES->Info->Sampling_Number,
                LOCAL_RES->Info->Sampling_Count, LOCAL_RES->Info->Sampling_Cycle_Number);
        }

#if 1
        for (i = 0; i < num; i++) {
#if WD_ADC_CH4_ENABLE
            WD_PRINT_PLAIN("%-4u,        %10d,        %10d,        %10d,        %10d", i + 1,
                           LOCAL_RES->Info->Xfer.V_Buf[i], LOCAL_RES->Info->Xfer.I_Buf[i],
                           LOCAL_RES->Info->Xfer.CH3_Buf[i], LOCAL_RES->Info->Xfer.CH4_Buf[i]);
#else
            WD_PRINT_PLAIN("%-4u,        %10d,        %10d,        %10d", i + 1,
                           LOCAL_RES->Info->Xfer.V_Buf[i], LOCAL_RES->Info->Xfer.I_Buf[i],
                           LOCAL_RES->Info->Xfer.CH3_Buf[i]);
#endif
            wd_time_wait_ms(1); // 1ms 정도 대기 // console 깨짐
        }
#else
        {
            uint32_t v_data, i_data;

            for (i = 0; i < num; i++) {
                v_data = ((uint32_t)LOCAL_RES->Info->V_Buf[i][0] << 16) |
                         ((uint32_t)LOCAL_RES->Info->V_Buf[i][1] << 8) |
                         (LOCAL_RES->Info->V_Buf[i][2] << 0);
                i_data = ((uint32_t)LOCAL_RES->Info->I_Buf[i][0] << 16) |
                         ((uint32_t)LOCAL_RES->Info->I_Buf[i][1] << 8) |
                         (LOCAL_RES->Info->I_Buf[i][2] << 0);
                WD_PRINT_PLAIN("%-4u\t\t%21.11f\t\t%21.11f\t\t0x%.8X\t\t0x%.8X\t\t0x%.8X\t\t0x%.8X",
                               i + 1, (double)(LOCAL_RES->Info->V_Sample[i]),
                               (double)(LOCAL_RES->Info->I_Sample[i]), LOCAL_RES->Info->V_Sample[i],
                               LOCAL_RES->Info->I_Sample[i], v_data, i_data);
                wd_time_wait_ms(1); // 1ms 정도 대기 // console 깨짐
            }
        }
#endif
    } break;

    case WD_ADC_SET_ODR: {
        fpu_t output_data_rate, output_data_rate2;

        ad713x_set_odr(&ad713x_dev_instance, value);

        do {
            ad713x_get_odr(&ad713x_dev_instance, &output_data_rate);
            ad713x_get_odr(&ad713x_dev_instance, &output_data_rate2);
        } while (!(output_data_rate == output_data_rate2));

        if (arg) {
            *((fpu_t *)arg) = output_data_rate;
        }

        WD_PRINT(WD_MSG_TITLE_CRLF, "[ADC AC] Set ODR Config [%f] Setting [%f]", (double)value,
                 (double)output_data_rate);
    } break;

    case WD_ADC_GET_ODR: {
        fpu_t output_data_rate;

        ad713x_get_odr(&ad713x_dev_instance, &output_data_rate);

        if (arg) {
            *((fpu_t *)arg) = output_data_rate;
        }

        WD_PRINT(WD_MSG_TITLE_CRLF, "[ADC AC] Get ODR [%f]", (double)output_data_rate);
    } break;

    case WD_ADC_CONVERSION_CALC: {
        uint32_t i, k;

        for (i = 0; i < WD_ADC_DATA_CHANNEL_NUMBER; i++) // 46328 us * 2
        {
            /*--------------- Analysis ---------------*/ // 5284 us * 2
            bool start = true;                           // false : 샘플이 많을 경우 안정적임
            bool side = false;
            bool zcd_fail = false;

            int32_t sample;
            int32_t last_sample = 0;
            int32_t peak_positive = 0;
            int32_t peak_negative = 0;
            int32_t max_sample = 0;

            wd_ad4134_adc_zcd_t *zc;
            wd_ad4134_adc_zcd_t zc_np_1 = {
                0,
            };
            wd_ad4134_adc_zcd_t zc_pn_1 = {
                0,
            };
            wd_ad4134_adc_zcd_t zc_np_2 = {
                0,
            };
            wd_ad4134_adc_zcd_t zc_pn_2 = {
                0,
            };

            int32_t dc_offset_positive[5];
            int32_t dc_offset_negative[5];
            int32_t dc_offset;

            wd_ad4134_adc_data_analysis_t *data;
            wd_ad4134_adc_data_average_t *average;
            wd_ad4134_adc_data_average_t *average_raw;

            int32_t *ptr;

            UNUSED(side);
            UNUSED(zcd_fail);

            data = &LOCAL_RES->Info->ADC_Data[i].Data;
            average = &LOCAL_RES->Info->ADC_Data[i].Average;
            average_raw = &LOCAL_RES->Info->ADC_Data[i].Average_Raw;

            if (i == 0) {
                ptr = LOCAL_RES->Info->Xfer.V_Buf;
            } else if (i == 1) {
                ptr = LOCAL_RES->Info->Xfer.I_Buf;
            } else if (i == 2) {
                ptr = LOCAL_RES->Info->Xfer.CH3_Buf; // VDC로 변경
            } else {
#if WD_ADC_CH4_ENABLE
                ptr = LOCAL_RES->Info->Xfer.CH4_Buf; // CH4로 변경
#endif
            }
#if 1
            dc_offset_positive[0] = INT32_MIN;
            dc_offset_positive[1] = INT32_MIN;
            dc_offset_positive[2] = INT32_MIN;
            dc_offset_positive[3] = INT32_MIN;
            dc_offset_positive[4] = INT32_MIN;
            dc_offset_negative[0] = INT32_MAX;
            dc_offset_negative[1] = INT32_MAX;
            dc_offset_negative[2] = INT32_MAX;
            dc_offset_negative[3] = INT32_MAX;
            dc_offset_negative[4] = INT32_MAX;

            for (k = 0; k < LOCAL_RES->Info->Sampling_Number; k++) {
                sample = ptr[k];

                if (dc_offset_positive[0] < sample) {
                    dc_offset_positive[4] = dc_offset_positive[3];
                    dc_offset_positive[3] = dc_offset_positive[2];
                    dc_offset_positive[2] = dc_offset_positive[1];
                    dc_offset_positive[1] = dc_offset_positive[0];
                    dc_offset_positive[0] = sample;
                } else if (dc_offset_positive[1] < sample) {
                    dc_offset_positive[4] = dc_offset_positive[3];
                    dc_offset_positive[3] = dc_offset_positive[2];
                    dc_offset_positive[2] = dc_offset_positive[1];
                    dc_offset_positive[1] = sample;
                } else if (dc_offset_positive[2] < sample) {
                    dc_offset_positive[4] = dc_offset_positive[3];
                    dc_offset_positive[3] = dc_offset_positive[2];
                    dc_offset_positive[2] = sample;
                } else if (dc_offset_positive[3] < sample) {
                    dc_offset_positive[4] = dc_offset_positive[3];
                    dc_offset_positive[3] = sample;
                } else if (dc_offset_positive[4] < sample) {
                    dc_offset_positive[4] = sample;
                }

                if (dc_offset_negative[0] > sample) {
                    dc_offset_negative[4] = dc_offset_negative[3];
                    dc_offset_negative[3] = dc_offset_negative[2];
                    dc_offset_negative[2] = dc_offset_negative[1];
                    dc_offset_negative[1] = dc_offset_negative[0];
                    dc_offset_negative[0] = sample;
                } else if (dc_offset_negative[1] > sample) {
                    dc_offset_negative[4] = dc_offset_negative[3];
                    dc_offset_negative[3] = dc_offset_negative[2];
                    dc_offset_negative[2] = dc_offset_negative[1];
                    dc_offset_negative[1] = sample;
                } else if (dc_offset_negative[2] > sample) {
                    dc_offset_negative[4] = dc_offset_negative[3];
                    dc_offset_negative[3] = dc_offset_negative[2];
                    dc_offset_negative[2] = sample;
                } else if (dc_offset_negative[3] > sample) {
                    dc_offset_negative[4] = dc_offset_negative[3];
                    dc_offset_negative[3] = sample;
                } else if (dc_offset_negative[4] > sample) {
                    dc_offset_negative[4] = sample;
                }
            }

            dc_offset = (((dc_offset_positive[0] + dc_offset_positive[1] + dc_offset_positive[2] +
                           dc_offset_positive[3] + dc_offset_positive[4]) /
                          5) +
                         ((dc_offset_negative[0] + dc_offset_negative[1] + dc_offset_negative[2] +
                           dc_offset_negative[3] + dc_offset_negative[4]) /
                          5)) /
                        2;
#else
            dc_offset_positive[0] = INT32_MIN;
            dc_offset_positive[1] = INT32_MIN;
            dc_offset_positive[2] = INT32_MIN;
            dc_offset_negative[0] = INT32_MAX;
            dc_offset_negative[1] = INT32_MAX;
            dc_offset_negative[2] = INT32_MAX;

            for (k = 0; k < size; k++) {
                sample = ptr[k];

                if (dc_offset_positive[0] < sample) {
                    dc_offset_positive[2] = dc_offset_positive[1];
                    dc_offset_positive[1] = dc_offset_positive[0];
                    dc_offset_positive[0] = sample;
                } else if (dc_offset_positive[1] < sample) {
                    dc_offset_positive[2] = dc_offset_positive[1];
                    dc_offset_positive[1] = sample;
                } else if (dc_offset_positive[2] < sample) {
                    dc_offset_positive[2] = sample;
                }

                if (dc_offset_negative[0] > sample) {
                    dc_offset_negative[2] = dc_offset_negative[1];
                    dc_offset_negative[1] = dc_offset_negative[0];
                    dc_offset_negative[0] = sample;
                } else if (dc_offset_negative[1] > sample) {
                    dc_offset_negative[2] = dc_offset_negative[1];
                    dc_offset_negative[1] = sample;
                } else if (dc_offset_negative[2] > sample) {
                    dc_offset_negative[2] = sample;
                }
            }

            dc_offset =
                (((dc_offset_positive[0] + dc_offset_positive[1] + dc_offset_positive[2]) / 3) +
                 ((dc_offset_negative[0] + dc_offset_negative[1] + dc_offset_negative[2]) / 3)) /
                2;
#endif

            for (k = 0; k < LOCAL_RES->Info->Sampling_Number; k++) {
                sample = ptr[k] - dc_offset;

                if (max_sample < labs(sample)) {
                    max_sample = labs(sample);
                }

                if (sample > 0) {
                    if (peak_positive < sample) {
                        peak_positive = sample;
                    }
                } else {
                    if (peak_negative > sample) {
                        peak_negative = sample;
                    }
                }

                if (sample >= 0) {
                    if (last_sample < 0) {
                        if (!start) {
                            // start = true;
                        } else {
                            if (!zc_pn_1.Complete) {
                                zc_pn_1.Complete = true;
                                zc_pn_1.Position = k;
                            } else if (!zc_pn_2.Complete) {
                                zc_pn_2.Complete = true;
                                zc_pn_2.Position = k;
                            }
                        }
                    }

                    if (!zc_np_1.Complete) {
                        zc = &zc_np_1;
                    } else if (!zc_np_2.Complete) {
                        zc = &zc_np_2;
                    } else {
                        continue;
                    }

                    if (zc->Peak < sample) {
                        zc->Peak = sample;
                    }
                } else {
                    if (last_sample > 0) {
                        if (!start) {
                            start = true;
                        } else {
                            if (!zc_np_1.Complete) {
                                zc_np_1.Complete = true;
                                zc_np_1.Position = k;
                            } else if (!zc_np_2.Complete) {
                                zc_np_2.Complete = true;
                                zc_np_2.Position = k;
                            }
                        }
                    }

                    if (!zc_pn_1.Complete) {
                        zc = &zc_pn_1;
                    } else if (!zc_pn_2.Complete) {
                        zc = &zc_pn_2;
                    } else {
                        continue;
                    }

                    if (zc->Peak > sample) {
                        zc->Peak = sample;
                    }
                }

                last_sample = sample;
            }

            if (max_sample < 25) {
                zcd_fail = true;
            } else {
                uint32_t gap_pn, gap_np;

                if ((zc_pn_1.Position != 0) && (zc_pn_1.Position != 1) && (zc_pn_2.Position != 0)) {
                    side = true;

                    if ((zc_np_2.Position == 0) || (zc_np_2.Position > zc_pn_2.Position)) {
                        gap_pn = zc_np_1.Position - zc_pn_1.Position;
                        gap_np = zc_pn_2.Position - zc_np_1.Position;
                    } else {
                        gap_pn = zc_np_2.Position - zc_pn_1.Position;
                        gap_np = zc_pn_2.Position - zc_np_2.Position;
                    }

                    if (WD_RANGE((uint32_t)(gap_np * 0.5), gap_pn, (uint32_t)(gap_np * 1.5)) &&
                        (zc_pn_2.Position - zc_pn_1.Position >= WD_ADC_DATA_SAMPLE_MIN_NUMBER)) {
                        data->ZCD_Start = zc_pn_1.Position;
                        data->ZCD_End = zc_pn_2.Position - 1;
                    } else {
                        zcd_fail = true;
                    }
                } else if ((zc_np_1.Position < zc_np_2.Position) && (zc_np_1.Position != 0) &&
                           (zc_np_1.Position != 1) && (zc_np_2.Position != 0)) {
                    side = false;

                    gap_pn = zc_np_2.Position - zc_pn_1.Position;
                    gap_np = zc_pn_1.Position - zc_np_1.Position;

                    if (WD_RANGE((uint32_t)(gap_np * 0.8), gap_pn, (uint32_t)(gap_np * 1.2)) &&
                        (zc_np_2.Position - zc_np_1.Position >= WD_ADC_DATA_SAMPLE_MIN_NUMBER)) {
                        data->ZCD_Start = zc_np_1.Position;
                        data->ZCD_End = zc_np_2.Position - 1;
                    } else {
                        zcd_fail = true;
                    }
                } else {
                    zcd_fail = true;
                }
            }

            if (zcd_fail) {
                if (i == 0) {
                    __NOP();
                }
                data->ZCD_Start = 0;
                data->ZCD_End = LOCAL_RES->Info->Sampling_Count - 1;
            }

            {
                uint64_t rms_raw = 0;
                int64_t sample_sum_raw = 0;
                for (k = 0; k < LOCAL_RES->Info->Sampling_Number; k++) {
                    rms_raw += ((uint64_t)(ptr[k]) * (uint64_t)((ptr[k])));
                    sample_sum_raw += (int64_t)ptr[k];
                }

                data->Maximum = max_sample;
                data->Peak_Positive = peak_positive;
                data->Peak_Negative = peak_negative;
                if ((data->Peak_Positive == 0) || (data->Peak_Negative == 0)) {
                    data->Frequency = (fpu_t)0.0;
                } else {
                    data->Frequency = ((fpu_t)LOCAL_RES->Info->Sampling_Count *
                                       LOCAL_RES->Info->Sampling_Frequency) /
                                      (fpu_t)(LOCAL_RES->Info->Sampling_Count);
                }
                data->DC_Offset = dc_offset;
                data->RMS = wd_toolkit_isqrt64(rms_raw / LOCAL_RES->Info->Sampling_Number);
                data->Sample_SUM = (int32_t)(sample_sum_raw / LOCAL_RES->Info->Sampling_Number);

                average_raw->Frequency += data->Frequency;
                average_raw->DC_Offset += data->DC_Offset;
                average_raw->RMS += data->RMS;
                average_raw->Maximum += data->Maximum;
                average_raw->Peak_Positive += data->Peak_Positive;
                average_raw->Peak_Negative += data->Peak_Negative;
                average_raw->Sample_SUM += data->Sample_SUM;
                average_raw->Count++;
#if WD_ADC_DATA_AVERAGE_NUMBER == 1
                if ((average_raw->Count >= WD_ADC_DATA_AVERAGE_NUMBER))
#elif WD_ADC_DATA_AVERAGE_NUMBER > 1
                if ((average_raw->Count >= WD_ADC_DATA_AVERAGE_NUMBER) ||
                    (wd_time_calc_us(&average->Time_Stamp) > 3000000))
#else
                if (wd_time_calc_us(&average->Time_Stamp) > 3000000)
#endif
                {
                    average->Count = average_raw->Count;
                    average->Frequency = average_raw->Frequency / (fpu_t)average_raw->Count;
                    average->DC_Offset = average_raw->DC_Offset / average_raw->Count;
                    average->RMS = average_raw->RMS / (uint32_t)average_raw->Count;
                    average->Maximum = average_raw->Maximum / average_raw->Count;
                    average->Peak_Positive = average_raw->Peak_Positive / average_raw->Count;
                    average->Peak_Negative = average_raw->Peak_Negative / average_raw->Count;
                    average->Sample_SUM = average_raw->Sample_SUM / average_raw->Count;
                    memset(average_raw, 0, sizeof(wd_ad4134_adc_data_average_t));
                    average->Time_Stamp = wd_time_get_us();
                }
            }
        } //	for(i = 0; i < WD_ADC_DATA_CHANNEL_NUMBER; i ++)                           // 46328 us
    } break;

    case WD_ADC_SET_SYNC_SETTLE: // arg = settling time [ms]
        LOCAL_RES->Info->Sync_Settle_Time_ms = arg;
        break;
    case WD_ADC_SET_REINIT: // arg = Reinitialization
    {
        SPI_Reinitialize();
        wd_ad4134_initialize();
    } break;
    default:
        break;
    }

    return (WD_CODE_OK);
}

static wd_adc_status_t adc_ad4134_get_status(void) {
    wd_adc_status_t status = {0};
    LOCAL_RES->Lock();
//  if(LOCAL_RES->Info->Status_V.Conversion_Finish && LOCAL_RES->Info->Status_I.Conversion_Finish)
#if WD_ADC_CH4_ENABLE
    if (LOCAL_RES->Info->Status_V.Conversion_Finish &&
        LOCAL_RES->Info->Status_I.Conversion_Finish &&
        LOCAL_RES->Info->Status_CH3.Conversion_Finish &&
        LOCAL_RES->Info->Status_CH4.Conversion_Finish)
#else
    if (LOCAL_RES->Info->Status_V.Conversion_Finish &&
        LOCAL_RES->Info->Status_I.Conversion_Finish &&
        LOCAL_RES->Info->Status_CH3
            .Conversion_Finish) // && LOCAL_RES->Info->Status_CH4.Conversion_Finish)
#endif
    {
        status.Conversion_Finish = true;
        LOCAL_RES->Info->Status_V.Conversion_Finish = false;
        LOCAL_RES->Info->Status_I.Conversion_Finish = false;
        LOCAL_RES->Info->Status_CH3.Conversion_Finish = false;
#if WD_ADC_CH4_ENABLE
        LOCAL_RES->Info->Status_CH4.Conversion_Finish = false;
#endif
    }
    LOCAL_RES->Unlock();

    return (status);
}

static void adc_ad4134_validation(void) {
#if (API_VALIDATION)

#endif
}

wd_ad4134_resources_t *const wd_ad4134_res_adc_ac = &wd_ad4134_resources;


wd_adc_driver_adc_ac_t wd_drv_adc_ac = {
    .Initialize = adc_ad4134_initialize,
    .Uninitialize = adc_ad4134_uninitialize,
    .PowerControl = adc_ad4134_power_control,
    .Start_Conversion = adc_ad4134_start_conversion,
    .Poll_Conversion = adc_ad4134_poll_conversion,
    .Stop_Conversion = adc_ad4134_stop_conversion,
    .Control = adc_ad4134_control,
    .Get_Status = adc_ad4134_get_status,
    .Validation = adc_ad4134_validation,
};

#ifndef NDEBUG
__USED wd_ad4134_debug_adc_ad4134_t wd_ad4134_dbg_adc_ad4134 = {
    .Resource = &wd_ad4134_resources,
    .Info = wd_ad4134_resources.Info,
    .Driver = LOCAL_DRV,
};
#endif


#endif //#if (RTE_COMPONENT_ADC_AD4134)

#endif //	#if 1 // 토글
