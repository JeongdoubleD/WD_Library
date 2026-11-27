/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_mcp23017.c
 *
 *  Created on: Jan 17, 2025
 *      Author: doubled
 */

#include "_wd_mcp23017.h"

#if 1 // 토글
#if RTE_COMPONENT_IO_EXPANDER_MCP23017

#define I2C_BAUDRATE_IO_EXPANDER 100000

/***************************************************************************************************************************************************************
 **********  Soft I2C MCP23017
 **************************************************************************************************************************************************************/
static wd_softi2c_info_t Driver_I2C_Soft_MCP23017_Info = {
    0,
};
static wd_softi2c_resources_t Driver_I2C_Soft_MCP23017_Resources = {
    .This = (void *)&Driver_I2C_Soft_MCP23017_Resources,
    .Name = "MCP23017",
    .Info = &Driver_I2C_Soft_MCP23017_Info,
    .IO =
        {
            .SCL = RTE_PORT_SW_I2C_SCL,
            .SDA = RTE_PORT_SW_I2C_SDA,
        },
    .Baudrate = WD_DRIVER_I2C_BUS_SPEED_STANDARD, // convert speed
    .Mode = WD_DRIVER_I2C_MODE_MASTER,
};
/***************************************************************************************************************************************************************
 **********  Driver
 **************************************************************************************************************************************************************/
static wd_mcp23017_info_t IO_Expander_MCP23017_Info = {
    0,
};

#if (RTE_RTOS)
static osStaticMutexDef_t Mutex_IO_Expander_MCP23017_cb_mem;
static osMutexId_t Mutex_IO_Expander_MCP23017;
static const osMutexAttr_t Mutex_IO_Expander_MCP23017_Attr = {
    .name = "Mutex IO_Expander_MCP23017",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = &Mutex_IO_Expander_MCP23017_cb_mem,
    .cb_size = sizeof(Mutex_IO_Expander_MCP23017_cb_mem)};

static int32_t res_start(void) {
#if (RTE_RTOS)
    Mutex_IO_Expander_MCP23017 = osMutexNew(&Mutex_IO_Expander_MCP23017_Attr);
    if (Mutex_IO_Expander_MCP23017 == NULL) {
#ifndef NDEBUG
        WD_LOG_ERROR("Mutex IO_Expander_MCP23017 New Error");
#endif
        return (WD_CODE_ERROR);
    }
#endif

    return (WD_CODE_OK);
}

static int32_t res_lock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        if (osMutexAcquire(Mutex_IO_Expander_MCP23017, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
            return (WD_CODE_ERROR_TIMEOUT);
        }
    }

    return (WD_CODE_OK);
}

static int32_t res_unlock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(Mutex_IO_Expander_MCP23017);
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

static wd_mcp23017_resources_t IO_Expander_MCP23017_Resources = {
    .Capabilities =
        {
            .BaudRate = WD_DRIVER_I2C_BUS_SPEED_STANDARD,
        },
    .Info = &IO_Expander_MCP23017_Info,
    .IO =
        {
            .SCL = RTE_PORT_SW_I2C_SCL,
            .SDA = RTE_PORT_SW_I2C_SDA,
        },
    .Drv = &Driver_I2C_Soft_MCP23017_Resources,
    .Handle = NULL,
    .Reg = NULL,
    .Start = res_start,
    .Lock = res_lock,
    .Unlock = res_unlock,
};

#undef LOCAL_DRV
#define LOCAL_DRV (&wd_drv_io_expander_mcp23017)
__USED static wd_mcp23017_resources_t *const LOCAL_RES = &IO_Expander_MCP23017_Resources;

__USED static void wd_io_expander_mcp23017_callback(uint32_t event) {
    LOCAL_RES->Info->Event = event;
    if (LOCAL_RES->Info->CB_Event) {
        LOCAL_RES->Info->CB_Event(event);
    }
}
// 변수 등 자원 초기화
static int32_t io_expander_mcp23017_initialize(wd_mcp23017_signal_event_t cb_event) {
    if (LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "IO_Expander_MCP23017 Constructor");

    memset(LOCAL_RES->Info, 0, sizeof(wd_mcp23017_info_t));

    if (cb_event) {
        LOCAL_RES->Info->CB_Event = cb_event;
    }

    LOCAL_RES->Start();

    LOCAL_RES->Info->Flags = WDIF_FLAG_INITIALIZED;

    return (WD_CODE_OK);
}

static int32_t io_expander_mcp23017_uninitialize(void) {
    return (WD_CODE_OK);
}

// 핀 설정 등 peripheral 초기화
static int32_t io_expander_mcp23017_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "IO_Expander_MCP23017 PowerControl:WD_POWER_OFF");

        LOCAL_RES->Info->Flags &= ~(WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
    } break;

    case WD_POWER_FULL: {
        if ((LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) == 0U) {
            return (WD_CODE_OK);
        }
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "IO_Expander_MCP23017 PowerControl:WD_POWER_FULL");
        WD_CHECK(wd_softi2c_initialize(LOCAL_RES->Drv, wd_io_expander_mcp23017_callback));
        WD_CHECK(wd_softi2c_power_control(LOCAL_RES->Drv, WD_POWER_FULL));
        WD_CHECK(wd_softi2c_control(LOCAL_RES->Drv, WD_I2C_SOFT_OWN_ADDRESS, MCP23017_ADDRESS));
        WD_CHECK(wd_softi2c_control(LOCAL_RES->Drv, WD_I2C_SOFT_BUS_SPEED, 500000));
        WD_CHECK(wd_softi2c_control(LOCAL_RES->Drv, WD_I2C_SOFT_ABORT_TRANSFER, 1));

        wd_mcp23017_init();


        LOCAL_RES->Info->Flags |= (WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
    } break;

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);
    }

    return (WD_CODE_OK);
}

static int32_t io_expander_mcp23017_control(uint32_t control, uint32_t arg) {
    UNUSED(control);
    UNUSED(arg);

    bool _flag = (bool)arg;

    switch (control & WD_EXPANDER_MCP23017_CONTROL_MSK) {
    case WD_EXPANDER_MCP23017_PIN_LED1:
    case WD_EXPANDER_MCP23017_PIN_LED2:
    case WD_EXPANDER_MCP23017_PIN_LED3:
    case WD_EXPANDER_MCP23017_PIN_LED4:
    case WD_EXPANDER_MCP23017_PIN_LED5:
    case WD_EXPANDER_MCP23017_PIN_LED6:
    case WD_EXPANDER_MCP23017_PIN_LED7:
    case WD_EXPANDER_MCP23017_PIN_LED8:
        wd_mcp23017_pin_control(MCP23017_GPIOA,
                                (uint8_t)(control & WD_EXPANDER_MCP23017_CONTROL_MSK), _flag);
        break;
    case WD_EXPANDER_MCP23017_PIN_SW_LOGIC:
    case WD_EXPANDER_MCP23017_PIN_V_CTL:
    case WD_EXPANDER_MCP23017_PIN_FET_DDS_CS:
    case WD_EXPANDER_MCP23017_PIN_ADC_OSR2:
    case WD_EXPANDER_MCP23017_PIN_MUX_A:
    case WD_EXPANDER_MCP23017_PIN_MUX_B:
    case WD_EXPANDER_MCP23017_CTRL_DC_PWR:
        wd_mcp23017_pin_control(MCP23017_GPIOB,
                                (uint8_t)(control & WD_EXPANDER_MCP23017_CONTROL_MSK), _flag);
        break;
    default:
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Unhandled case: %d",
                      (control & WD_EXPANDER_MCP23017_CONTROL_MSK));
        break;
    }
    return (WD_CODE_OK);
}

static wd_mcp23017_status_t io_expander_mcp23017_get_status(void) {
    wd_mcp23017_status_t status = {0};

    return (status);
}

static void io_expander_mcp23017_validation(void) {
#if (API_VALIDATION)

#endif
}

wd_mcp23017_resources_t *const wd_mcp23017_res_io_expander_mcp23017 =
    &IO_Expander_MCP23017_Resources;

wd_mcp23017_driver_io_expander_mcp23017_t wd_drv_io_expander_mcp23017 = {
    .Initialize = io_expander_mcp23017_initialize,
    .Uninitialize = io_expander_mcp23017_uninitialize,
    .PowerControl = io_expander_mcp23017_power_control,
    .Control = io_expander_mcp23017_control,
    .Get_Status = io_expander_mcp23017_get_status,
    .Validation = io_expander_mcp23017_validation,
};

#ifndef NDEBUG
__USED wd_mcp23017_debug_io_expander_mcp23017_t wd_mcp23017_dbg_io_expander_mcp23017 = {
    .Resource = LOCAL_RES,
    .Info = &IO_Expander_MCP23017_Info,
    .Driver = LOCAL_DRV,
};
#endif


/***************************************************************************************************************************************************************
 **********  MCP23017
 **************************************************************************************************************************************************************/
// MCP23017 핀 구성 배열
const wd_mcp23017_pin_config_t wd_mcp23017_pin_configs[] = {
    [WD_EXPANDER_MCP23017_PIN_LED1] = {0x7F, 0x80},
    [WD_EXPANDER_MCP23017_PIN_LED2] = {0xBF, 0x40},
    [WD_EXPANDER_MCP23017_PIN_LED3] = {0xDF, 0x20},
    [WD_EXPANDER_MCP23017_PIN_LED4] = {0xEF, 0x10},
    [WD_EXPANDER_MCP23017_PIN_LED5] = {0xF7, 0x08},
    [WD_EXPANDER_MCP23017_PIN_LED6] = {0xFB, 0x04},
    [WD_EXPANDER_MCP23017_PIN_LED7] = {0xFD, 0x02},
    [WD_EXPANDER_MCP23017_PIN_LED8] = {0xFE, 0x01},

    [WD_EXPANDER_MCP23017_PIN_SW_LOGIC] = {0xFE, 0x01},
    [WD_EXPANDER_MCP23017_PIN_V_CTL] = {0xFD, 0x02},
    [WD_EXPANDER_MCP23017_PIN_FET_DDS_CS] = {0xFB, 0x04},
    [WD_EXPANDER_MCP23017_PIN_ADC_OSR2] = {0xF7, 0x08},
    [WD_EXPANDER_MCP23017_PIN_MUX_A] = {0xEF, 0x10},
    [WD_EXPANDER_MCP23017_PIN_MUX_B] = {0xDF, 0x20},
    [WD_EXPANDER_MCP23017_CTRL_DC_PWR] = {0xBF, 0x40},
};


void wd_mcp23017_init(void) {
    wd_mcp23017_write_iodir(MCP23017_IODIRB, 0x00); // 출력으로 설정
    wd_mcp23017_write_iodir(MCP23017_IODIRA, 0x00); // 출력으로 설정

    wd_mcp23017_write_gpio(MCP23017_GPIOA, 0xFF); // 초기값 LED 전체 off
    wd_mcp23017_write_gpio(MCP23017_GPIOB, 0x11); // 초기값 MUX 400mA
}


void wd_mcp23017_write_iodir(uint8_t reg, uint8_t data) {
    wd_softi2c_master_transmit(LOCAL_RES->Drv, reg, &data, sizeof(data), true);
}

void wd_mcp23017_write_gpio(uint8_t reg, uint8_t data) {
    wd_softi2c_master_transmit(LOCAL_RES->Drv, reg, &data, sizeof(data), true);
}

uint8_t wd_mcp23017_read_gpio(uint8_t reg) {
    uint8_t data;

    wd_softi2c_master_receive(LOCAL_RES->Drv, reg, &data, sizeof(data), true);

    return data;
}

void wd_mcp23017_pin_control(uint8_t _reg, uint8_t _pin, bool _flag) {
    if (_pin < sizeof(wd_mcp23017_pin_configs) / sizeof(wd_mcp23017_pin_configs[0])) {
        uint8_t mask = wd_mcp23017_pin_configs[_pin].mask;
        uint8_t pin_value = _flag ? wd_mcp23017_pin_configs[_pin].pin_value : 0x00;
        wd_mcp23017_set_pin(_reg, mask, pin_value);
    } else {
        // 예외 처리: 잘못된 핀 값
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Invalid pin: %d", _pin);
    }
}


// 특정 포트의 특정 핀 설정
void wd_mcp23017_set_pin(uint8_t _reg, uint8_t s_value, uint8_t c_value) {
    // uint8_t reg = (_port == 1) ? MCP23017_GPIOA : MCP23017_GPIOB;
    uint8_t current = wd_mcp23017_read_gpio(_reg); // 현재 레지스터 값 읽기

    current &= s_value; // 변경값 위치 초기화
    current |= c_value; // 변경할값 쓰기

    wd_mcp23017_write_gpio(_reg, current);
}

#endif //	#if RTE_COMPONENT_IO_EXPANDER_MCP23017
#endif //	#if 1 // 토글
