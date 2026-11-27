/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_softi2c.c
 *
 *  Created on: Jan 6, 2025
 *      Author: doubled
 */

#include "_wd_softi2c.h"

#if 1
#if RTE_DRIVERS_I2C_SOFT

#if (RTE_DRV_VALIDATION)
#define API_VALIDATION 1
#endif

static int32_t res_lock(wd_softi2c_resources_t *instance) {
#if (RTE_RTOS)
    uint32_t owner = (uint32_t)(osThreadGetId());
#else
    uint32_t owner = instance->Name;
#endif

    if (instance->Info->Owner != NULL) {
        if ((uint32_t)instance->Info->Owner != owner) {
#if (RTE_RTOS)
            WD_WAIT_WHILE((instance->Info->Owner != NULL), 1000000);
#else
            WD_LOG_ERROR("Soft I2C __Acquire");
#endif
            instance->Info->Owner = (void *)owner;
        }
    }

    return (WD_CODE_OK);
}

static int32_t res_unlock(wd_softi2c_resources_t *instance) {
    instance->Info->Owner = NULL;

    return (WD_CODE_OK);
}

/* ---------------------------------------------------------------------------------------------------------------
 * Bit-banged I2C master engine.
 * - SDA is driven push-pull while the master sends, switched to input (pull-up) while it receives
 * - the slave ACK after each sent byte is clocked but not checked (devices used so far do not need it)
 * - register read : address phase, then a stop-like release sequence, then a new START with the read address
 * Timing unit : a quarter of the SCL period, WD_CONF_SOFTI2C_QUARTER_CYCLES CPU cycles.
 * ------------------------------------------------------------------------------------------------------------- */
#ifndef WD_CONF_SOFTI2C_QUARTER_CYCLES
#define WD_CONF_SOFTI2C_QUARTER_CYCLES 2000U
#endif

static void i2c_wait(void) {
    const uint32_t start = DWT->CYCCNT;

    while ((DWT->CYCCNT - start) < WD_CONF_SOFTI2C_QUARTER_CYCLES) {
    }
}

static void i2c_sda(wd_softi2c_resources_t *instance, uint32_t level) {
    wd_gpio_write(instance->IO.SDA, (level != 0U) ? WD_GPIO_SET : WD_GPIO_CLEAR);
}

static void i2c_scl(wd_softi2c_resources_t *instance, uint32_t level) {
    wd_gpio_write(instance->IO.SCL, (level != 0U) ? WD_GPIO_SET : WD_GPIO_CLEAR);
}

/* START condition : SDA falls while SCL is high, then SCL low */
static void i2c_start(wd_softi2c_resources_t *instance) {
    i2c_scl(instance, 1U);
    i2c_wait();
    i2c_sda(instance, 1U);
    i2c_wait();
    i2c_sda(instance, 0U);
    i2c_wait();
    i2c_scl(instance, 0U);
    i2c_wait();
}

/* STOP condition : SDA rises while SCL is high */
static void i2c_stop(wd_softi2c_resources_t *instance) {
    wd_gpio_config(instance->IO.SDA, WD_GPIO_DIR_OUTPUT);
    i2c_sda(instance, 0U);
    i2c_wait();
    i2c_scl(instance, 1U);
    i2c_wait();
    i2c_sda(instance, 1U);
    i2c_wait();
}

/* bus release between the address phase and the read phase of a register read */
static void i2c_release(wd_softi2c_resources_t *instance) {
    wd_gpio_config(instance->IO.SDA, WD_GPIO_DIR_OUTPUT);
    i2c_sda(instance, 0U);
    i2c_wait();
    i2c_sda(instance, 1U);
    i2c_wait();
    i2c_scl(instance, 1U);
    i2c_wait();
}

/* 8 data bits MSB first, then one clock for the slave acknowledge */
static void i2c_send_byte(wd_softi2c_resources_t *instance, uint8_t value) {
    wd_gpio_config(instance->IO.SDA, WD_GPIO_DIR_OUTPUT);
    for (uint32_t bit = 0U; bit < 8U; bit++) {
        i2c_sda(instance, ((value << bit) & 0x80U) != 0U ? 1U : 0U);
        i2c_wait();
        i2c_scl(instance, 1U);
        i2c_wait();
        i2c_scl(instance, 0U);
    }
    i2c_sda(instance, 0U); /* acknowledge clock */
    i2c_wait();
    i2c_scl(instance, 1U);
    i2c_wait();
    i2c_scl(instance, 0U);
    i2c_wait();
}

/* 8 data bits MSB first, then ACK (more bytes follow) or NACK (last byte) */
static uint8_t i2c_receive_byte(wd_softi2c_resources_t *instance, bool last) {
    uint32_t value = 0U;

    wd_gpio_config(instance->IO.SDA, WD_GPIO_INPUT_PULL_UP);
    for (uint32_t bit = 0U; bit < 8U; bit++) {
        i2c_scl(instance, 1U);
        i2c_wait();
        value = (value << 1) | (((uint32_t)wd_gpio_read(instance->IO.SDA) != 0U) ? 1U : 0U);
        i2c_scl(instance, 0U);
        i2c_wait();
    }
    wd_gpio_config(instance->IO.SDA, WD_GPIO_OUTPUT_PP_PULL_NONE);
    i2c_sda(instance, last ? 1U : 0U); /* NACK = high, ACK = low */
    i2c_wait();
    i2c_scl(instance, 1U);
    i2c_sda(instance, 1U);
    i2c_wait();
    i2c_scl(instance, 0U);
    i2c_wait();
    return (uint8_t)value;
}

/* register address, big endian, 'reg_len' bytes (0 .. 4) */
static void i2c_send_register(wd_softi2c_resources_t *instance, uint32_t reg, uint32_t reg_len) {
    for (uint32_t i = reg_len; i > 0U; i--) {
        i2c_send_byte(instance, (uint8_t)(reg >> ((i - 1U) * 8U)));
    }
}

/* write 'len' bytes to register 'reg' of the 7-bit device 'chip' */
static void i2c_write_register(wd_softi2c_resources_t *instance, uint8_t chip, uint32_t reg,
                               uint32_t reg_len, const uint8_t *data, uint32_t len) {
    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "i2c write : chip %02X reg %02lX (%lu) len %lu", chip,
                  (unsigned long)reg, (unsigned long)reg_len, (unsigned long)len);
    i2c_start(instance);
    i2c_send_byte(instance, (uint8_t)(chip << 1));
    i2c_send_register(instance, reg, reg_len);
    for (uint32_t i = 0U; i < len; i++) {
        i2c_send_byte(instance, data[i]);
    }
    i2c_stop(instance);
}

/* read 'len' bytes from register 'reg' (reg_len == 0 : plain read without register phase) */
static void i2c_read_register(wd_softi2c_resources_t *instance, uint8_t chip, uint32_t reg,
                              uint32_t reg_len, uint8_t *data, uint32_t len) {
    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "i2c read : chip %02X reg %02lX (%lu) len %lu", chip,
                  (unsigned long)reg, (unsigned long)reg_len, (unsigned long)len);
    i2c_start(instance);
    if (reg_len > 0U) {
        i2c_send_byte(instance, (uint8_t)(chip << 1));
        i2c_send_register(instance, reg, reg_len);
        i2c_release(instance);
        i2c_start(instance);
    }
    i2c_send_byte(instance, (uint8_t)((chip << 1) | 1U));
    i2c_wait();
    for (uint32_t i = 0U; i < len; i++) {
        data[i] = i2c_receive_byte(instance, (i + 1U) == len);
    }
    i2c_stop(instance);
}

static void I2C_Soft_SignalEvent(wd_softi2c_resources_t *instance, uint32_t event) {
    instance->Info->Event |= event;
    if (instance->Info->CB_Event) {
        instance->Info->CB_Event(event);
    }
}

int32_t wd_softi2c_initialize(wd_softi2c_resources_t *instance, wd_softi2c_event_t cb_event) {
    if (instance->Info->Flags & WDIF_FLAG_INITIALIZED) {
        return WD_CODE_OK;
    }

    WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "Soft I2C Driver [%s] Constructor",
                   (instance->Name != NULL) ? instance->Name : "?");

    memset(instance->Info, 0, sizeof(wd_softi2c_info_t));

    instance->Info->Baudrate = instance->Baudrate;
    instance->Info->Mode = instance->Mode;

    if (cb_event) {
        instance->Info->CB_Event = cb_event;
    }

    instance->Info->Flags = WDIF_FLAG_INITIALIZED;

    return (WD_CODE_OK);
}

int32_t wd_softi2c_power_control(wd_softi2c_resources_t *instance, wd_power_state_t state) {
    switch (state) {
    case WD_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Soft I2C [%s] PowerControl:WD_POWER_OFF",
                      (instance->Name != NULL) ? instance->Name : "?");

        instance->Info->Flags &= ~(WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
    } break;

    case WD_POWER_FULL: {
        if ((instance->Info->Flags & WDIF_FLAG_INITIALIZED) == 0U) {
            return (WD_CODE_OK);
        }

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Soft I2C [%s] PowerControl:WD_POWER_FULL",
                      (instance->Name != NULL) ? instance->Name : "?");


        //		wd_gpio_config(instance->IO.SCL,   WD_GPIO_INPUT_PULL_UP | WD_GPIO_SET);
        //		wd_gpio_config(instance->IO.SDA,   WD_GPIO_INPUT_PULL_UP | WD_GPIO_SET);

        res_unlock(instance);

        instance->Info->Flags |= WDIF_FLAG_POWERED;
    } break;

    case WD_POWER_LOW:
        return WD_CODE_ERROR_UNSUPPORTED;
    default:
        break;
    }

    return WD_CODE_OK;
}

int32_t wd_softi2c_master_transmit(wd_softi2c_resources_t *instance, uint32_t addr,
                                   const uint8_t *data, uint32_t num, bool xfer_pending) {
    (void)xfer_pending;
    WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "Soft I2C [%s] Send 0x%.8X",
                   (instance->Name != NULL) ? instance->Name : "?", data, WD_MSG_MAGIC, WD_NET_TX,
                   data, num);
    wd_time_wait_us(1); // DUMP_LOG 시 깨짐 발생_이건 USB 콘솔문제인거 같음.
    res_lock(instance);

    i2c_write_register(instance, (uint8_t)instance->Info->Address, addr,
                       (uint32_t)instance->Info->Acnt, data, num);
    I2C_Soft_SignalEvent(instance, WD_I2C_SOFT_EVENT_TRANSFER_DONE);

    res_unlock(instance);

    return (WD_CODE_OK);
}

int32_t wd_softi2c_master_receive(wd_softi2c_resources_t *instance, uint32_t addr, uint8_t *data,
                                  uint32_t num, bool xfer_pending) {
    (void)xfer_pending;
    //   WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "Soft I2C [%s] Send 0x%.8X", (instance->Name != NULL) ? instance->Name : "?", data, WD_MSG_MAGIC, WD_NET_TX, data, num);

    res_lock(instance);
    i2c_read_register(instance, (uint8_t)instance->Info->Address, addr,
                      (uint32_t)instance->Info->Acnt, data, num);


    res_unlock(instance);

    return (WD_CODE_OK);
}

int32_t wd_softi2c_control(wd_softi2c_resources_t *instance, uint32_t control, uint32_t arg) {
    //  uint32_t val;
    bool config = false;
    //  uint32_t fpclk, fscl;

    switch (control & WD_I2C_SOFT_CONTROL_Msk) {
    case WD_I2C_SOFT_OWN_ADDRESS: {
        instance->Info->Address = (uint8_t)arg;
        config = true;
    } break;

    case WD_I2C_SOFT_BUS_SPEED: {
        if (arg) {
            instance->Info->Baudrate = arg;
            config = true;
            return WD_CODE_OK;
        } else {
            return WD_CODE_ERROR_PARAMETER;
        }
    } break;
    case WD_I2C_SOFT_BUS_CLEAR:
        return WD_CODE_OK;

    case WD_I2C_SOFT_ABORT_TRANSFER:
        instance->Info->Acnt = (uint8_t)arg;
        //   instance->Info->Def_Val = (uint8_t)arg;
        config = true;
        return WD_CODE_OK;
    default:
        break;
    }

    switch (control & WD_DRIVER_I2C_SOFT_CONTROL_MSK) {
    case WD_DRIVER_I2C_SOFT_SCL_CTRL:
        i2c_scl(instance, arg);
        config = true;
        return WD_CODE_OK;

    case WD_DRIVER_I2C_SOFT_SDA_CTRL:
        i2c_sda(instance, arg);
        config = true;
        return WD_CODE_OK;
    default:
        break;
    }

    if (!config) {
        return (WD_CODE_ERROR_PARAMETER);
    }

    instance->Info->Baudrate = arg;
    instance->Info->Control = control;
    instance->Info->Flags |= WDIF_FLAG_CONFIGURED;

    return (WD_CODE_OK);
}

#endif // #if RTE_DRIVERS_I2C_SOFT
#endif // #if 1
