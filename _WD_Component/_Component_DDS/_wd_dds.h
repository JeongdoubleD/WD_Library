/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_dds.h
 *
 *  Created on: Jan 5, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_DDS_H
#define WD_COMPONENT_WD_DDS_H

#include "wd_core.h"

/* board HAL mapping : default CubeMX names, override in wd_board.h */
#ifndef RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE
#define RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE (&htim1)
#endif
#ifndef RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_REGISTER
#define RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_REGISTER TIM1
#endif

typedef enum {
    WD_DDS_WAVE_SINE = 0,
    WD_DDS_WAVE_TRIANGLE = 1,
    WD_DDS_WAVE_SQUARE = 2,
} wd_dds_table_t;

typedef struct wd_dds_status_s {
    uint32_t Busy : 1;
    uint32_t Error : 1;
    uint32_t Reserved : 30;
} wd_dds_status_t;

typedef void (*wd_dds_signal_event_t)(uint32_t event);

typedef struct wd_driver_dds_s {
    int32_t (*Initialize)(wd_dds_signal_event_t cb_event);
    int32_t (*Uninitialize)(void);
    int32_t (*PowerControl)(wd_power_state_t state);
    int32_t (*Set_Config)(uint32_t power, uint32_t mode, fpu_t freq, fpu_t phase);
    int32_t (*Control)(uint32_t control, uint32_t arg);
    wd_dds_status_t (*Get_Status)(void);
    void (*Validation)(void);
} const wd_dds_driver_dds_t;

#if RTE_COMPONENT_ADC_AD4134

#elif (RTE_COMPONENT_DDS) /* DDS without AD4134 : legacy board (COMPLIANCE.md known defects) */
typedef enum wd_dds_ad9837_control_e {
/*--------------- Control  : [ 0:15] ---------------*/
#define WD_DDS_AD9837_CONTROL_POS 0
#define WD_DDS_AD9837_CONTROL_MSK (0xFFFFUL << WD_DDS_AD9837_CONTROL_POS)
    WD_DDS_AD9837_CALIBRATION = (1UL << WD_DDS_AD9837_CONTROL_POS),
/*--------------- Option   : [16:31] ---------------*/
#define WD_DDS_AD9837_OPTION_POS  16
#define WD_DDS_AD9837_OPTION_MSK  (0xFFFFUL << WD_DDS_AD9837_OPTION_POS)
#define WD_DDS_AD9837_OPTION(n)   ((n) << WD_DDS_AD9837_OPTION_POS)
} wd_ad9837_all_control_t;

typedef enum {
// Fix
#if 0
  WDIF_FLAG_INITIALIZED             = (1UL <<  0),
  WDIF_FLAG_POWERED                 = (1UL <<  1),
  WDIF_FLAG_CONFIGURED              = (1UL <<  2),
#endif
    // Specific
    WD_DDS_AD9837_FLAG_SPECIFIC = (1UL << 3),
} wd_ad9837_all_flag_t;

typedef enum {
    // Specific
    WD_DDS_AD9837_EVENT_CHANGE = (1UL << 0),
} wd_ad9837_all_event_t;

typedef struct wd_dds_ad9837_capabilities_s {
    uint32_t BaudRate;
    uint32_t MCLK;
    uint32_t Wave_Point_Num;
    uint32_t Flag_Wave_Square;
} wd_ad9837_all_capabilities_t;

typedef struct wd_dds_ad9837_xfer_s {
    uint8_t *Tx_Buf;
    uint8_t *Rx_Buf;
    uint32_t Tx_Buf_Size;
    uint32_t Rx_Buf_Size;
    uint32_t Tx_Num;
    uint32_t Rx_Num;
    uint32_t Tx_Cnt;
    uint32_t Rx_Cnt;
    uint32_t Tx_Idx;
    uint32_t Rx_Idx;
} wd_ad9837_all_xfer_t;

typedef struct wd_dds_ad9837_info_s {
    void *Owner;
    uint32_t Time_Stamp;
    uint32_t Time_Wait;
    uint32_t Timeout;
    uint32_t Timer;
    wd_dds_signal_event_t CB_Event;
    wd_dds_status_t volatile Status;
    int32_t volatile Flags;
    uint32_t volatile Event;
    wd_ad9837_all_xfer_t volatile Xfer;
    uint32_t MCLK;
    fpu_t Frequency;
    uint16_t Register;
} wd_ad9837_all_info_t;

typedef struct wd_dds_ad9837_io_s {
    uint32_t CS;
    uint32_t CLK;
    uint32_t MOSI;
} wd_ad9837_all_io_t;


typedef enum {
    AD9837_TRIANGLE = 1,
    AD9837_SINE,
    AD9837_SQUARE,
    AD9837_SQUARE_2
} wd_ad9837_all_mode_t;
typedef enum { AD9837_FREQ0 = 1, AD9837_FREQ1 } wd_ad9837_all_freqreg_t;
typedef enum { AD9837_PHASE0 = 1, AD9837_PHASE1 } wd_ad9837_all_phasereg_t;
typedef enum { AD9837_FULL = 1, AD9837_COARSE, AD9837_FINE } wd_ad9837_all_freqadjustmode_t;
typedef enum {
    AD9837_DAC_OFF = 1,
    AD9837_DAC_ON,
    AD9837_DAC_TOGGLE
} wd_ad9837_all_ad9837_control_t;

/***************************************************************************************************************************************************************
 **********  AD9837 (https://www.mikroe.com/waveform-3-click)
 **************************************************************************************************************************************************************/
/**
 * @brief Clock input frequency.
 * @details This is the frequency of MCLK in Hz.
 */
#define DEV_WAVEFORM3_FMCLK                16000000

/**
 * @brief Frequency resolution.
 * @details This is the resolution of frequency ( 2^28 ).
 */
#define DEV_WAVEFORM3_RESOLUTION           268435456

/**
 * @brief Correction coefficient constant.
 * @details Used to set more accurate frequency on output.
 */
#define DEV_WAVEFORM3_CORR_COEFF           1.0488

/**
 * @brief Write register lsb mask.
 * @details Extracts the least significant byte from a word.
 */
#define DEV_WAVEFORM3_WR_REG_MASK_LSB      0x00FF

/**
 * @brief Write register msb mask.
 * @details Extracts the most significant byte from a word.
 */
#define DEV_WAVEFORM3_WR_REG_MASK_MSB      0xFF00

/**
 * @brief Maximal output frequency.
 * @details Limiting output frequency in Hz.
 */
#define DEV_WAVEFORM3_FREQ_SET_MAX         15000000

/**
 * @brief Frequency step size.
 * @details Used for return value of frequency that has been set.
 */
#define DEV_WAVEFORM3_FREQ_STEP_SIZE       0.0596

/**
 * @brief Frequency lsb mask.
 * @details Used to extract the least significant 14-bit for frequency.
 */
#define DEV_WAVEFORM3_FREQ_MASK_LSB        0x3FFF

/**
 * @brief Frequency msb mask.
 * @details Used to extract the most significant 14-bit for frequency.
 */
#define DEV_WAVEFORM3_FREQ_MASK_MSB        0xFFFC000

/**
 * @brief Freq0 channel mask.
 * @details Used to set frequency for freq0.
 */
#define DEV_WAVEFORM3_FREQ0_BITMASK        0x4000

/**
 * @brief Freq1 channel mask.
 * @details Used to set frequency for freq1.
 */
#define DEV_WAVEFORM3_FREQ1_BITMASK        0x8000

/**
 * @brief Freq0 channel bitmask retval.
 * @details Bitmask used for return value of freq0.
 */
#define DEV_WAVEFORM3_FREQ0_BITMASK_RETVAL 0xBFFF

/**
 * @brief Freq1 channel bitmask retval.
 * @details Bitmask used for return value of freq1.
 */
#define DEV_WAVEFORM3_FREQ1_BITMASK_RETVAL 0x7FFF

/**
 * @brief Maximal output phase shift.
 * @details Limiting output phase shift in angle degrees.
 */
#define DEV_WAVEFORM3_PHASE_SET_MAX        360

/**
 * @brief Phase resolution.
 * @details This is the resolution of phase shift ( 2^12 ).
 */
#define DEV_WAVEFORM3_PHASE_RESOLUTION     4096

/**
 * @brief Phase bitmask.
 * @details This bitmask is used for safe storage of phase shift.
 */
#define DEV_WAVEFORM3_PHASE_BITMASK        0xF000

/**
 * @brief Phase bitmask retval.
 * @details Bitmask used for return value of phase shift.
 */
#define DEV_WAVEFORM3_PHASE_BITMASK_RETVAL 0x0FFF

/**
 * @brief Phase0 channel mask.
 * @details Used to set frequency for phase0.
 */
#define DEV_WAVEFORM3_PHASE0_MASK          0xE000

/**
 * @brief Phase1 channel mask.
 * @details Used to set frequency for phase1.
 */
#define DEV_WAVEFORM3_PHASE1_MASK          0xC000

/**
 * @addtogroup waveform3_reg
 * @{
 */

/**
 * @brief Waveform 3 B28 register bit.
 * @details Allows 28-bit frequency register to operate as two 14-bit
 * registers of Waveform 3 Click driver.
 */
#define WAVEFORM3_REG_B28_BIT              1 << 13

/**
 * @brief Waveform 3 HLB register bit.
 * @details This control bit allows the user to continuously load the MSBs
 * or LSBs of a frequency register while ignoring the remaining 14 bits.
 */
#define WAVEFORM3_REG_HLB_BIT              1 << 12

/**
 * @brief Waveform 3 FSEL register bit.
 * @details This bit defines whether the FREQ0 register or the FREQ1
 * register is used in the phase accumulator.
 */
#define WAVEFORM3_REG_FSEL_BIT             1 << 11

/**
 * @brief Waveform 3 PSEL register bit.
 * @details This bit defines whether the PHASE0 register data or the
 * PHASE1 register data is added to the output of the phase accumulator.
 */
#define WAVEFORM3_REG_PSEL_BIT             1 << 10

/**
 * @brief Waveform 3 RESET register bit.
 * @details This bit resets internal registers to 0.
 */
#define WAVEFORM3_REG_RESET_BIT            1 << 8

/**
 * @brief Waveform 3 SLEEP1 register bit.
 * @details This bit disables the internal MCLK. The DAC output remains
 * at its present value because the NCO is no longer accumulating.
 */
#define WAVEFORM3_REG_SLEEP1_BIT           1 << 7

/**
 * @brief Waveform 3 SLEEP12 register bit.
 * @details This bit powers down the on-chip DAC. This is useful when
 * the AD9837 is used to output the MSB of the DAC data.
 */
#define WAVEFORM3_REG_SLEEP12_BIT          1 << 6

/**
 * @brief Waveform 3 OPBITEN register bit.
 * @details This bit, in association with the MODE bit, controls
 * the output at the VOUT pin.
 */
#define WAVEFORM3_REG_OPBITEN_BIT          1 << 5

/**
 * @brief Waveform 3 DIV2 register bit.
 * @details This bit causes the MSB of the DAC data to be output
 * at the VOUT pin.
 */
#define WAVEFORM3_REG_DIV2_BIT             1 << 3

/**
 * @brief Waveform 3 MODE register bit.
 * @details This bit bypasses the SIN ROM, resulting in a triangle
 * output from the DAC.
 */
#define WAVEFORM3_REG_MODE_BIT             1 << 1

/**
 * @brief Waveform 3 RESET register bit.
 * @details This bit disables the reset function.
 */
#define WAVEFORM3_REG_RESET_CLEAR          0x0000

/*! @} */ // waveform3_reg

/**
 * @defgroup mode Waveform 3 Mode Settings
 * @brief Settings for modes of Waveform 3 Click driver.
 */

/**
 * @addtogroup mode
 * @{
 */

/**
 * @brief Waveform 3 sinusoidal output mode.
 * @details Results sinusoidal output from AD9837 waveform generator.
 */
#define WAVEFORM3_CFG_MODE_SINUSOIDAL      0x00

/**
 * @brief Waveform 3 triangle output mode.
 * @details Results triangle output from AD9837 waveform generator.
 */
#define WAVEFORM3_CFG_MODE_TRIANGLE        0x01

/**
 * @brief Waveform 3 DAC divided by 2 output mode.
 * @details Results square wave divided by 2 on output from
 * AD9837 waveform generator.
 */
#define WAVEFORM3_CFG_MODE_DAC_2           0x02

/**
 * @brief Waveform 3 DAC output mode.
 * @details Results square wave on output from AD9837 waveform generator.
 */
#define WAVEFORM3_CFG_MODE_DAC             0x03

/*! @} */ // mode

/**
 * @defgroup freq_reg Waveform 3 FREQ Settings
 * @brief Selection for frequency channel of Waveform 3 Click driver.
 */

/**
 * @addtogroup freq_reg
 * @{
 */

/**
 * @brief Waveform 3 FREQ0 channel select.
 * @details Selects FREQ0 frequency channel of AD9837 waveform generator.
 */
#define WAVEFORM3_CFG_FREQ_REG0            0x00

/**
 * @brief Waveform 3 FREQ1 channel select.
 * @details Selects FREQ1 frequency channel of AD9837 waveform generator.
 */
#define WAVEFORM3_CFG_FREQ_REG1            0x01

/*! @} */ // freq_reg

/**
 * @defgroup phase_reg Waveform 3 PHASE Settings
 * @brief Selection for phase shift channel of Waveform 3 Click driver.
 */

/**
 * @addtogroup phase_reg
 * @{
 */

/**
 * @brief Waveform 3 PHASE0 channel select.
 * @details Selects PHASE0 phase shift channel of AD9837 waveform generator.
 */
#define WAVEFORM3_CFG_PHASE_REG0           0x00

/**
 * @brief Waveform 3 PHASE1 channel select.
 * @details Selects PHASE1 phase shift channel of AD9837 waveform generator.
 */
#define WAVEFORM3_CFG_PHASE_REG1           0x01

/*! @} */ // phase_reg

/**
 * @defgroup waveform3_default_settings Waveform 3 default Settings
 * @brief Default settings on reset, located in default_cfg function.
 */

/**
 * @addtogroup waveform3_default_settings
 * @{
 */

/**
 * @brief Waveform 3 default FREQ0 value.
 * @details This is the value of frequency on freq0 channel after reset.
 */
#define WAVEFORM3_DEFAULT_FREQ0            0

/**
 * @brief Waveform 3 default FREQ1 value.
 * @details This is the value of frequency on freq1 channel after reset.
 */
#define WAVEFORM3_DEFAULT_FREQ1            0

/**
 * @brief Waveform 3 default PHASE0 value.
 * @details This is the value of phase shift on phase0 channel after reset.
 */
#define WAVEFORM3_DEFAULT_PHASE0           0

/**
 * @brief Waveform 3 default PHASE1 value.
 * @details This is the value of phase shift on phase1 channel after reset.
 */
#define WAVEFORM3_DEFAULT_PHASE1           0

#endif

#endif /* WD_COMPONENT_WD_DDS_H */
