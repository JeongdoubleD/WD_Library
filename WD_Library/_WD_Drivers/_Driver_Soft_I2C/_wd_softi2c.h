/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_softi2c.h
 *
 *  Created on: Jan 6, 2025
 *      Author: doubled
 */

#ifndef WD_DRIVERS_WD_SOFTI2C_H
#define WD_DRIVERS_WD_SOFTI2C_H

#include "wd_core.h"

#if 1
#if RTE_DRIVERS_I2C_SOFT

//#define CONFIG_SOFT_I2C_READ_REPEATED_START			// 추가함

/* Bus Clear clock period definition */
#define WD_I2C_BUS_CLEAR_CLOCK_PERIOD (2) // I2C bus clock period (ms)

/* Current driver status flag definition */
#define WD_I2C_INIT         ((uint8_t)0x01) // I2C initialized
#define WD_I2C_POWER        ((uint8_t)0x02) // I2C powered on
#define WD_I2C_SETUP        ((uint8_t)0x04) // I2C configured
#define WD_I2C_DMA_TX       ((uint8_t)0x08) // Tx DMA enabled
#define WD_I2C_DMA_RX       ((uint8_t)0x10) // Rx DMA enabled
#define WD_I2C_XFER_SET     ((uint8_t)0x20) // Transfer buffer registered
#define WD_I2C_XFER_NO_STOP ((uint8_t)0x40) // Transfer that did not generate STOP at the end

/* Transfer status flags definitions */
#define WD_XFER_CTRL_RESTART   ((uint8_t)0x01) // Transfer pending
#define WD_XFER_CTRL_STOP      ((uint8_t)0x02) // Generate repeated start and readdress
#define WD_XFER_CTRL_ADDR_NACK ((uint8_t)0x04) // Slave address not acknowledged
#define WD_XFER_CTRL_ADDR_DONE ((uint8_t)0x08) // Addressing done

/* TIMING register limit values */
#define I2C_TIMINGR_PRESC_MAX  (1UL << 4)
#define I2C_TIMINGR_SCLDEL_MAX (1UL << 4)
#define I2C_TIMINGR_SDADEL_MAX (1UL << 4)
#define I2C_TIMINGR_SCLH_MAX   (1UL << 8)
#define I2C_TIMINGR_SCLL_MAX   (1UL << 8)

/* Pulse width of spikes suppressed by the analog filter */
#define WD_I2C_ANALOG_FILTER_DELAY_MIN 50  // Min delay added by the analog filter (ns)
#define WD_I2C_ANALOG_FILTER_DELAY_MAX 260 // Max delay added by the analog filter (ns)

/* I2C communication */
#define WD_DRIVER_I2C_MODE_NONE   0x00U /*!< No I2C communication on going             */
#define WD_DRIVER_I2C_MODE_MASTER 0x10U /*!< I2C communication is in Master Mode       */
#define WD_DRIVER_I2C_MODE_SLAVE  0x20U /*!< I2C communication is in Slave Mode        */
#define WD_DRIVER_I2C_MODE_MEM    0x40U /*!< I2C communication is in Memory Mode       */

/* I2C Bus Speed */
#define WD_DRIVER_I2C_BUS_SPEED_STANDARD  0x01U /*!< I2C Bus Speed Standard Speed (100kHz)   */
#define WD_DRIVER_I2C_BUS_SPEED_FAST      0x02U /*!< I2C Bus Speed Fast Speed (400kHz)       */
#define WD_DRIVER_I2C_BUS_SPEED_FAST_PLUS 0x03U /*!< I2C Bus Speed Fast+ Speed ( 1MHz)       */
#define WD_DRIVER_I2C_BUS_SPEED_HIGH      0x04U /*!< I2C Bus Speed High Speed (3.4MHz)       */

// Control() 상위 8bit: 동작 선택 (기존 ARM_I2C_* 와 동일한 값)
#define WD_I2C_SOFT_CONTROL_Pos    0
#define WD_I2C_SOFT_CONTROL_Msk    (0xFFUL << WD_I2C_SOFT_CONTROL_Pos)
#define WD_I2C_SOFT_OWN_ADDRESS    (0x01UL) ///< Set Own Slave Address; arg = address
#define WD_I2C_SOFT_BUS_SPEED      (0x02UL) ///< Set Bus Speed; arg = speed
#define WD_I2C_SOFT_BUS_CLEAR      (0x03UL) ///< Execute Bus clear: send nine clock pulses
#define WD_I2C_SOFT_ABORT_TRANSFER (0x04UL) ///< Abort Master/Slave Transmit/Receive

// 완료 이벤트 (기존 ARM_I2C_EVENT_* 과 동일한 값)
#define WD_I2C_SOFT_EVENT_TRANSFER_DONE (1UL << 0)

typedef void (*wd_softi2c_event_t)(uint32_t event);

typedef enum wd_driver_i2c_soft_control_e {
/*--------------- Control  : [ 0:15] ---------------*/
#define WD_DRIVER_I2C_SOFT_CONTROL_POS 16
#define WD_DRIVER_I2C_SOFT_CONTROL_MSK (0xFFFFUL << WD_DRIVER_I2C_SOFT_CONTROL_POS)
    WD_DRIVER_I2C_SOFT_SCL_CTRL = (01UL << WD_DRIVER_I2C_SOFT_CONTROL_POS),
    WD_DRIVER_I2C_SOFT_SDA_CTRL = (02UL << WD_DRIVER_I2C_SOFT_CONTROL_POS),

    //  /*--------------- Option   : [16:31] ---------------*/
    //#define WD_DRIVER_I2C_SOFT_OPTION_POS                                                  16
    //#define WD_DRIVER_I2C_SOFT_OPTION_MSK                                                  (0xFFFFUL << WD_DRIVER_I2C_SOFT_OPTION_POS)
    //#define WD_DRIVER_I2C_SOFT_OPTION(n)                                                   ((n) << WD_DRIVER_I2C_SOFT_OPTION_POS)
} wd_softi2c_control_t;


typedef struct wd_driver_i2c_soft_xfer_s {
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
} wd_softi2c_xfer_t;

typedef struct wd_driver_i2c_soft_info_s {
    void *Owner;
    uint32_t Time_Stamp;
    uint32_t Time_Wait;
    uint32_t Timeout;
    uint32_t Timer;
    wd_softi2c_event_t CB_Event;
    int32_t volatile Flags;
    uint32_t volatile Event;
    wd_softi2c_xfer_t volatile Xfer;
    uint32_t Baudrate;
    uint32_t Control;
    uint32_t Mode;
    uint8_t Def_Val;
    uint8_t Address;
    int Acnt; // 장치 개수
} wd_softi2c_info_t;

typedef struct wd_driver_i2c_soft_io_s {
    uint32_t SCL; // SCL IO Pin
    uint32_t SDA; // SDA IO Pin
} wd_softi2c_io_t;

typedef const struct wd_driver_i2c_soft_resources_s {
    void *This;
    char *Name;
    wd_softi2c_info_t *Info;
    wd_softi2c_io_t IO;
    uint32_t Baudrate;
    uint32_t Mode;
} wd_softi2c_resources_t;

// 소프트 I2C는 단일 구현체만 존재하고 다른 구현으로 교체될 일이 없으므로
// vtable(함수 포인터 테이블) 없이 인스턴스를 직접 넘겨 호출한다.
int32_t wd_softi2c_initialize(wd_softi2c_resources_t *instance, wd_softi2c_event_t cb_event);
int32_t wd_softi2c_power_control(wd_softi2c_resources_t *instance, wd_power_state_t state);
int32_t wd_softi2c_master_transmit(wd_softi2c_resources_t *instance, uint32_t addr,
                                   const uint8_t *data, uint32_t num, bool xfer_pending);
int32_t wd_softi2c_master_receive(wd_softi2c_resources_t *instance, uint32_t addr, uint8_t *data,
                                  uint32_t num, bool xfer_pending);
int32_t wd_softi2c_control(wd_softi2c_resources_t *instance, uint32_t control, uint32_t arg);

#endif // #if RTE_DRIVERS_I2C_SOFT
#endif // #if 1

#endif /* WD_DRIVERS_WD_SOFTI2C_H */
