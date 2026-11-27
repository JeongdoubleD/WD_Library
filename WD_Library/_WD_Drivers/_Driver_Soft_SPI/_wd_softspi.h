/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_softspi.h
 *
 *  Created on: Dec 30, 2024
 *      Author: doubled
 */

#ifndef WD_DRIVERS_WD_SOFTSPI_H
#define WD_DRIVERS_WD_SOFTSPI_H

#include "wd_core.h"

#if 1
#if RTE_DRIVERS_SPI_SOFT

#if defined(__cplusplus)
extern "C" {
#endif

// Control() 상위 8bit: 동작 선택 (기존 ARM_SPI_CONTROL_* 과 동일한 값)
#define WD_SPI_SOFT_CONTROL_Pos          0
#define WD_SPI_SOFT_CONTROL_Msk          (0xFFUL << WD_SPI_SOFT_CONTROL_Pos)
#define WD_SPI_SOFT_MODE_MASTER          (0x01UL << WD_SPI_SOFT_CONTROL_Pos)
#define WD_SPI_SOFT_SET_BUS_SPEED        (0x10UL << WD_SPI_SOFT_CONTROL_Pos)
#define WD_SPI_SOFT_GET_BUS_SPEED        (0x11UL << WD_SPI_SOFT_CONTROL_Pos)
#define WD_SPI_SOFT_SET_DEFAULT_TX_VALUE (0x12UL << WD_SPI_SOFT_CONTROL_Pos)
#define WD_SPI_SOFT_CONTROL_SS           (0x13UL << WD_SPI_SOFT_CONTROL_Pos)
#define WD_SPI_SOFT_ABORT_TRANSFER       (0x14UL << WD_SPI_SOFT_CONTROL_Pos)

// Frame format (CPOL/CPHA)
#define WD_SPI_SOFT_FRAME_FORMAT_Pos 8
#define WD_SPI_SOFT_FRAME_FORMAT_Msk (7UL << WD_SPI_SOFT_FRAME_FORMAT_Pos)
#define WD_SPI_SOFT_CPOL0_CPHA0      (0UL << WD_SPI_SOFT_FRAME_FORMAT_Pos)
#define WD_SPI_SOFT_CPOL0_CPHA1      (1UL << WD_SPI_SOFT_FRAME_FORMAT_Pos)
#define WD_SPI_SOFT_CPOL1_CPHA0      (2UL << WD_SPI_SOFT_FRAME_FORMAT_Pos)
#define WD_SPI_SOFT_CPOL1_CPHA1      (3UL << WD_SPI_SOFT_FRAME_FORMAT_Pos)

// Data bits
#define WD_SPI_SOFT_DATA_BITS_Pos 12
#define WD_SPI_SOFT_DATA_BITS_Msk (0x3FUL << WD_SPI_SOFT_DATA_BITS_Pos)
#define WD_SPI_SOFT_DATA_BITS(n)  (((n) & 0x3FUL) << WD_SPI_SOFT_DATA_BITS_Pos)

// Bit order
#define WD_SPI_SOFT_BIT_ORDER_Pos 18
#define WD_SPI_SOFT_BIT_ORDER_Msk (1UL << WD_SPI_SOFT_BIT_ORDER_Pos)
#define WD_SPI_SOFT_MSB_LSB       (0UL << WD_SPI_SOFT_BIT_ORDER_Pos)
#define WD_SPI_SOFT_LSB_MSB       (1UL << WD_SPI_SOFT_BIT_ORDER_Pos)

// Slave Select (Master 기준)
#define WD_SPI_SOFT_SS_MASTER_MODE_Pos  19
#define WD_SPI_SOFT_SS_MASTER_MODE_Msk  (3UL << WD_SPI_SOFT_SS_MASTER_MODE_Pos)
#define WD_SPI_SOFT_SS_MASTER_UNUSED    (0UL << WD_SPI_SOFT_SS_MASTER_MODE_Pos)
#define WD_SPI_SOFT_SS_MASTER_SW        (1UL << WD_SPI_SOFT_SS_MASTER_MODE_Pos)
#define WD_SPI_SOFT_SS_MASTER_HW_OUTPUT (2UL << WD_SPI_SOFT_SS_MASTER_MODE_Pos)

#define WD_SPI_SOFT_SS_INACTIVE 0UL
#define WD_SPI_SOFT_SS_ACTIVE   1UL

// 완료/에러 이벤트 (기존 ARM_SPI_EVENT_* 과 동일한 값)
#define WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE (1UL << 0)
#define WD_SPI_SOFT_EVENT_DATA_LOST         (1UL << 1)
#define WD_SPI_SOFT_EVENT_MODE_FAULT        (1UL << 2)

typedef void (*wd_softspi_event_t)(uint32_t event);

typedef enum wd_driver_spi_soft_control_e {
    WD_DRIVER_SPI_SOFT_CPHA = WD_BIT0,
    WD_DRIVER_SPI_SOFT_CPOL = WD_BIT1,
    WD_DRIVER_SPI_SOFT_MODE_0 = (0 | 0),
    WD_DRIVER_SPI_SOFT_MODE_1 = (0 | WD_DRIVER_SPI_SOFT_CPHA),
    WD_DRIVER_SPI_SOFT_MODE_2 = (WD_DRIVER_SPI_SOFT_CPOL | 0),
    WD_DRIVER_SPI_SOFT_MODE_3 = (WD_DRIVER_SPI_SOFT_CPOL | WD_DRIVER_SPI_SOFT_CPHA),

    WD_DRIVER_SPI_SOFT_XFER_BEGIN = WD_BIT2,
    WD_DRIVER_SPI_SOFT_XFER_END = WD_BIT3,
    WD_DRIVER_SPI_SOFT_XFER_ONCE = (WD_DRIVER_SPI_SOFT_XFER_BEGIN | WD_DRIVER_SPI_SOFT_XFER_END),

    WD_DRIVER_SPI_SOFT_XFER_MASTER_NO_RX = WD_BIT4,
    WD_DRIVER_SPI_SOFT_XFER_MASTER_NO_TX = WD_BIT5,
} wd_softspi_control_t;

typedef struct wd_driver_spi_soft_xfer_s {
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
} wd_softspi_xfer_t;

typedef struct wd_driver_spi_soft_info_s {
    void *Owner;
    uint32_t Time_Stamp;
    uint32_t Time_Wait;
    uint32_t Timeout;
    uint32_t Timer;
    wd_softspi_event_t CB_Event;
    int32_t volatile Flags;
    uint32_t volatile Event;
    wd_softspi_xfer_t volatile Xfer;
    uint32_t Baudrate;
    uint32_t Control;
    uint32_t Mode;
    uint8_t Def_Val;
} wd_softspi_info_t;

typedef struct wd_driver_spi_soft_io_s {
    uint32_t CS;
    uint32_t CLK;
    uint32_t MOSI;
    uint32_t MISO;
} wd_softspi_io_t;

typedef const struct wd_driver_spi_soft_resources_s {
    void *This;
    char *Name;
    wd_softspi_info_t *Info;
    wd_softspi_io_t IO;
    uint32_t Baudrate;
    uint32_t Mode;
} wd_softspi_resources_t;

// 소프트 SPI는 단일 구현체만 존재하고 다른 구현으로 교체될 일이 없으므로
// vtable(함수 포인터 테이블) 없이 인스턴스를 직접 넘겨 호출한다.
int32_t wd_softspi_initialize(wd_softspi_resources_t *instance, wd_softspi_event_t cb_event);
int32_t wd_softspi_power_control(wd_softspi_resources_t *instance, wd_power_state_t state);
int32_t wd_softspi_send(wd_softspi_resources_t *instance, const void *data, uint32_t num);
int32_t wd_softspi_receive(wd_softspi_resources_t *instance, void *data, uint32_t num);
int32_t wd_softspi_transfer(wd_softspi_resources_t *instance, const void *data_out, void *data_in,
                            uint32_t num);
int32_t wd_softspi_control(wd_softspi_resources_t *instance, uint32_t control, uint32_t arg);

#if defined(__cplusplus)
}
#endif

#endif // #if RTE_DRIVERS_SPI_SOFT
#endif // #if 1

#endif /* WD_DRIVERS_WD_SOFTSPI_H */
