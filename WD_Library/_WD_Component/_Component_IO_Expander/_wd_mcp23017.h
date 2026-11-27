/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_mcp23017.h
 *
 *  Created on: Jan 17, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_MCP23017_H
#define WD_COMPONENT_WD_MCP23017_H

#include "wd_core.h"
#include "_wd_softi2c.h"

#if 1 // 토글
#if RTE_COMPONENT_IO_EXPANDER_MCP23017

typedef enum wd_expander_mcp23017_control_e {
/*--------------- Control  : [ 0:15] ---------------*/
/*--------------- Control  : [ 0:15] ---------------*/
#define WD_EXPANDER_MCP23017_CONTROL_POS 0
#define WD_EXPANDER_MCP23017_CONTROL_MSK (0xFFFFUL << WD_EXPANDER_MCP23017_CONTROL_POS)
    WD_EXPANDER_MCP23017_PIN_LED8 = (1UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_LED7 = (2UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_LED6 = (3UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_LED5 = (4UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_LED4 = (5UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_LED3 = (6UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_LED2 = (7UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_LED1 = (8UL << WD_EXPANDER_MCP23017_CONTROL_POS),

    WD_EXPANDER_MCP23017_PIN_SW_LOGIC = (9UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_V_CTL = (10UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_FET_DDS_CS = (11UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_ADC_OSR2 = (12UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_MUX_A = (13UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_PIN_MUX_B = (14UL << WD_EXPANDER_MCP23017_CONTROL_POS),
    WD_EXPANDER_MCP23017_CTRL_DC_PWR = (15UL << WD_EXPANDER_MCP23017_CONTROL_POS),

/*--------------- Option   : [16:31] ---------------*/
#define WD_EXPANDER_MCP23017_OPTION_POS 16
#define WD_EXPANDER_MCP23017_OPTION_MSK (0xFFFFUL << WD_EXPANDER_MCP23017_OPTION_POS)
#define WD_EXPANDER_MCP23017_OPTION(n)  ((n) << WD_EXPANDER_MCP23017_OPTION_POS)
} wd_mcp23017_control_t;


typedef struct wd_io_expander_mcp23017_capabilities_s {
    uint32_t BaudRate;
} wd_mcp23017_capabilities_t;

typedef void (*wd_mcp23017_signal_event_t)(uint32_t event);

typedef struct wd_io_expander_mcp23017_status_s {
    // Fix
    uint32_t Busy : 1;
    uint32_t Error : 1;
    // Specific
    uint32_t Reserved : 30;
} wd_mcp23017_status_t;

typedef struct wd_io_expander_mcp23017_xfer_s {
    // Specific
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
} wd_mcp23017_xfer_t;

typedef struct wd_io_expander_mcp23017_info_s {
    // Fix
    void *Owner;
    uint32_t Time_Stamp;
    uint32_t Time_Wait;
    uint32_t Timeout;
    uint32_t Timer;
    wd_mcp23017_signal_event_t CB_Event;
    wd_mcp23017_status_t volatile Status;
    int32_t volatile Flags;
    uint32_t volatile Event;
    wd_mcp23017_xfer_t volatile Xfer;
    //
    uint8_t McpPortA_Cmd;
    uint8_t McpPortB_Cmd;
    //
} wd_mcp23017_info_t;

typedef struct wd_io_expander_mcp23017_io_s {
    uint32_t SCL;
    uint32_t SDA;
} wd_mcp23017_io_t;

typedef const struct wd_io_expander_mcp23017_resources_s {
    wd_mcp23017_capabilities_t Capabilities;
    wd_mcp23017_info_t *Info;
    wd_mcp23017_io_t IO;
    wd_softi2c_resources_t *Drv;
    void *Handle;
    void *Reg;
    int32_t (*Start)(void);
    int32_t (*Lock)(void);
    int32_t (*Unlock)(void);
} wd_mcp23017_resources_t;


typedef struct wd_driver_io_expander_mcp23017_s {
    int32_t (*Initialize)(wd_mcp23017_signal_event_t cb_event);
    int32_t (*Uninitialize)(void);
    int32_t (*PowerControl)(wd_power_state_t state);
    int32_t (*Control)(uint32_t control, uint32_t arg);
    wd_mcp23017_status_t (*Get_Status)(void);
    void (*Validation)(void);
} const wd_mcp23017_driver_io_expander_mcp23017_t;

extern wd_mcp23017_driver_io_expander_mcp23017_t wd_drv_io_expander_mcp23017;
extern wd_mcp23017_resources_t *const wd_mcp23017_res_io_expander_mcp23017;

#ifndef NDEBUG
typedef struct wd_debug_io_expander_mcp23017_s {
    wd_mcp23017_resources_t *Resource;
    wd_mcp23017_info_t *Info;
    wd_mcp23017_driver_io_expander_mcp23017_t *Driver;
} const wd_mcp23017_debug_io_expander_mcp23017_t;

extern wd_mcp23017_debug_io_expander_mcp23017_t wd_mcp23017_dbg_io_expander_mcp23017;
#endif

// /***************************************************************************************************************************************************************
//  **********  MCP23017 IC
//  **************************************************************************************************************************************************************/
#define MCP23017_ADDRESS 0x20

#define MCP23017_IODIRA 0x00
#define MCP23017_IODIRB 0x01 //0x10

#define MCP23017_GPIOA 0x12
#define MCP23017_GPIOB 0x13

#define MCP23017_OLATA 0x14
#define MCP23017_OLATB 0x15
//#define MCP23017_GPIOA    0x09
//#define MCP23017_GPIOB    0x19

#define WD_EXPANDER_LED1 7 //0x80
#define WD_EXPANDER_LED2 6 //0x40
#define WD_EXPANDER_LED3 5 //0x20
#define WD_EXPANDER_LED4 4 //0x10
#define WD_EXPANDER_LED5 3 //0x08
#define WD_EXPANDER_LED6 2 //0x04


typedef struct {
    uint8_t mask;      // GPIOB 마스크 값
    uint8_t pin_value; // 설정할 핀 값
} wd_mcp23017_pin_config_t;

void wd_mcp23017_init(void);
void wd_mcp23017_write_iodir(uint8_t reg, uint8_t data);
void wd_mcp23017_write_gpio(uint8_t reg, uint8_t data);
uint8_t wd_mcp23017_read_gpio(uint8_t reg);
void wd_mcp23017_pin_control(uint8_t _reg, uint8_t _pin, bool _flag);
void wd_mcp23017_set_pin(uint8_t _reg, uint8_t s_value, uint8_t c_value);

#endif //	#if RTE_COMPONENT_IO_EXPANDER_MCP23017
#endif //	#if 1 // 토글

#endif /* WD_COMPONENT_WD_MCP23017_H */
