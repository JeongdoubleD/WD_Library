/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_softspi.c
 *
 *  Created on: Dec 30, 2024
 *      Author: doubled
 */

#include "_wd_softspi.h"

#if 1
#if RTE_DRIVERS_SPI_SOFT

#if (RTE_DRV_VALIDATION)
#define API_VALIDATION 1
#endif

static int32_t res_lock(wd_softspi_resources_t *dev) {
#if (RTE_RTOS)
    uint32_t owner = (uint32_t)(osThreadGetId());
#else
    uint32_t owner = dev->Name;
#endif

    if (dev->Info->Owner != NULL) {
        if ((uint32_t)dev->Info->Owner != owner) {
#if (RTE_RTOS)
            WD_WAIT_WHILE((dev->Info->Owner != NULL), 1000000);
#else
            //  WD_LOG_ERROR("Soft SPI __Acquire");
#endif
            dev->Info->Owner = (void *)owner;
        }
    }

    return (WD_CODE_OK);
}

static int32_t res_unlock(wd_softspi_resources_t *dev) {
    dev->Info->Owner = NULL;

    return (WD_CODE_OK);
}


/* ---------------------------------------------------------------------------------------------------------------
 * Bit-banged SPI engine (MSB first, CS active low).
 * Mode : CPOL = clock idle level, CPHA = 0 -> data valid before the first clock edge (sample on the leading edge),
 *        CPHA = 1 -> data changes on the leading edge (sample on the trailing edge).
 * Half clock period : SystemCoreClock / baudrate / 2 cycles (halved above 500 kHz), no wait from 3 MHz.
 * ------------------------------------------------------------------------------------------------------------- */
static uint32_t spi_half_period_cycles(const wd_softspi_resources_t *dev) {
    uint32_t cycles;

    if ((dev->Info->Baudrate == 0U) || (dev->Info->Baudrate >= 3000000U)) {
        return 0U; /* GPIO toggling is already slower than the requested rate */
    }
    cycles = SystemCoreClock / dev->Info->Baudrate / 2U;
    if (dev->Info->Baudrate > 500000U) {
        cycles /= 2U;
    }
    return cycles;
}

static void spi_wait(uint32_t cycles) {
    uint32_t start;

    if (cycles == 0U) {
        return;
    }
    start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles) {
    }
}

static uint32_t spi_idle_level(const wd_softspi_resources_t *dev) {
    return ((dev->Mode & WD_DRIVER_SPI_SOFT_CPOL) != 0U) ? 1U : 0U;
}

/* clock line to its idle level (before a transfer) */
static void spi_bus_idle(wd_softspi_resources_t *dev) {
    wd_gpio_write(dev->IO.CLK, spi_idle_level(dev));
}

static void spi_select(wd_softspi_resources_t *dev) {
    wd_gpio_write(dev->IO.CS, 1U);
    wd_gpio_write(dev->IO.CLK, spi_idle_level(dev));
    wd_gpio_write(dev->IO.CS, 0U);
}

static void spi_deselect(wd_softspi_resources_t *dev) {
    wd_gpio_write(dev->IO.CS, 1U);
}

/* one byte out / in, 8 clocks */
static uint8_t spi_shift_byte(wd_softspi_resources_t *dev, uint8_t out, uint32_t flags,
                              uint32_t half) {
    const uint32_t idle = spi_idle_level(dev);
    const uint32_t active = idle ^ 1U;
    const bool late_sample = ((dev->Mode & WD_DRIVER_SPI_SOFT_CPHA) != 0U);
    const bool drive = ((flags & WD_DRIVER_SPI_SOFT_XFER_MASTER_NO_TX) == 0U);
    const bool sample = ((flags & WD_DRIVER_SPI_SOFT_XFER_MASTER_NO_RX) == 0U);
    uint8_t in = 0U;

    for (uint32_t mask = 0x80U; mask != 0U; mask >>= 1) {
        const uint32_t level = ((out & mask) != 0U) ? 1U : 0U;
        uint32_t miso = 0U;

        if (late_sample) {
            wd_gpio_write(dev->IO.CLK, active); /* leading edge : data changes */
            if (drive) {
                wd_gpio_write(dev->IO.MOSI, level);
            }
            spi_wait(half);
            wd_gpio_write(dev->IO.CLK, idle); /* trailing edge : sample */
            if (sample) {
                miso = (uint32_t)wd_gpio_read(dev->IO.MISO);
            }
            spi_wait(half);
        } else {
            if (drive) {
                wd_gpio_write(dev->IO.MOSI, level); /* data valid before the leading edge */
            }
            spi_wait(half);
            wd_gpio_write(dev->IO.CLK, active); /* leading edge : sample */
            if (sample) {
                miso = (uint32_t)wd_gpio_read(dev->IO.MISO);
            }
            spi_wait(half);
            wd_gpio_write(dev->IO.CLK, idle);
        }
        in = (uint8_t)((uint32_t)(in << 1) | (miso & 1U));
    }
    return in;
}

/* transfer 'num' bytes. tx == NULL : send the default value, rx == NULL : discard the received bytes.
 * flags : WD_DRIVER_SPI_SOFT_XFER_BEGIN selects the device, _END releases it, _MASTER_NO_TX / _NO_RX */
static void spi_transfer(wd_softspi_resources_t *dev, const void *tx, void *rx, uint32_t num,
                         uint32_t flags) {
    const uint8_t *out = (const uint8_t *)tx;
    uint8_t *in = (uint8_t *)rx;
    const uint32_t half = spi_half_period_cycles(dev);

    if ((flags & WD_DRIVER_SPI_SOFT_XFER_BEGIN) != 0U) {
        spi_select(dev);
    }
    for (uint32_t i = 0U; i < num; i++) {
        const uint8_t byte = (out != NULL) ? out[i] : dev->Info->Def_Val;
        const uint8_t got = spi_shift_byte(dev, byte, flags, half);

        if (in != NULL) {
            in[i] = got;
        }
    }
    if ((flags & WD_DRIVER_SPI_SOFT_XFER_END) != 0U) {
        spi_deselect(dev);
    }
}


static void SPI_Soft_SignalEvent(wd_softspi_resources_t *instance, uint32_t event) {
    instance->Info->Event |= event;
    if (instance->Info->CB_Event) {
        instance->Info->CB_Event(event);
    }
}

int32_t wd_softspi_initialize(wd_softspi_resources_t *instance, wd_softspi_event_t cb_event) {
    if ((uint32_t)instance->Info->Flags & (1UL << 0)) {
        return WD_CODE_OK;
    }

    // WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "Soft SPI Driver [%s] Constructor", (instance->Name != NULL) ? instance->Name : "?");

    memset(instance->Info, 0, sizeof(wd_softspi_info_t));

    instance->Info->Baudrate = instance->Baudrate;
    instance->Info->Mode = instance->Mode;

    if (cb_event) {
        instance->Info->CB_Event = cb_event;
    }

    instance->Info->Flags = (1UL << 0);

    return (WD_CODE_OK);
}

int32_t wd_softspi_power_control(wd_softspi_resources_t *instance, wd_power_state_t state) {
    switch (state) {
    case WD_POWER_OFF: {
        // WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Soft SPI [%s] PowerControl:WD_POWER_OFF", (instance->Name != NULL) ? instance->Name : "?");

        instance->Info->Flags =
            (int32_t)((uint32_t)instance->Info->Flags & ~((1UL << 1) | (1UL << 2)));
    } break;

    case WD_POWER_FULL: {
        if (((uint32_t)instance->Info->Flags & (1UL << 0)) == 0U) {
            return (WD_CODE_OK);
        }

        // WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Soft SPI [%s] PowerControl:WD_POWER_FULL", (instance->Name != NULL) ? instance->Name : "?");

        wd_gpio_config(instance->IO.CS, WD_GPIO_OUTPUT_PP_PULL_NONE | WD_GPIO_SET);
        wd_gpio_config(instance->IO.CLK, WD_GPIO_OUTPUT_PP_PULL_NONE | WD_GPIO_SET);
        wd_gpio_config(instance->IO.MOSI, WD_GPIO_OUTPUT_PP_PULL_NONE | WD_GPIO_SET);
        wd_gpio_config(instance->IO.MISO, WD_GPIO_INPUT_PULL_UP);

        res_unlock(instance);

        spi_bus_idle(instance);

        instance->Info->Flags = (int32_t)((uint32_t)instance->Info->Flags | (1UL << 1));
    } break;

    case WD_POWER_LOW:
        return WD_CODE_ERROR_UNSUPPORTED;
    default:
        break;
    }

    return WD_CODE_OK;
}

int32_t wd_softspi_send(wd_softspi_resources_t *instance, const void *data, uint32_t num) {
    // WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "Soft SPI [%s] Send 0x%.8X", (instance->Name != NULL) ? instance->Name : "?", data, WD_MSG_MAGIC, WD_NET_TX, data, num);

    res_lock(instance);

    spi_bus_idle(instance);

    spi_transfer(instance, data, NULL, num,
                 WD_DRIVER_SPI_SOFT_XFER_ONCE | WD_DRIVER_SPI_SOFT_XFER_MASTER_NO_RX);

    SPI_Soft_SignalEvent(instance, WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE);

    res_unlock(instance);

    return (WD_CODE_OK);
}

int32_t wd_softspi_receive(wd_softspi_resources_t *instance, void *data, uint32_t num) {
    // WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "Soft SPI [%s] Receive 0x%.8X", (instance->Name != NULL) ? instance->Name : "?", data, WD_MSG_MAGIC, WD_NET_RX, data, num);

    res_lock(instance);

    spi_bus_idle(instance);

    spi_transfer(instance, NULL, data, num,
                 WD_DRIVER_SPI_SOFT_XFER_ONCE | WD_DRIVER_SPI_SOFT_XFER_MASTER_NO_TX);

    SPI_Soft_SignalEvent(instance, WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE);

    res_unlock(instance);

    return (WD_CODE_OK);
}

int32_t wd_softspi_transfer(wd_softspi_resources_t *instance, const void *data_out, void *data_in,
                            uint32_t num) {
    // WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "Soft SPI [%s] Transfer [Tx] 0x%.8X", (instance->Name != NULL) ? instance->Name : "?", data_out, WD_MSG_MAGIC, WD_NET_TX, data_out, num);

    res_lock(instance);

    spi_bus_idle(instance);

    spi_transfer(instance, data_out, data_in, num, WD_DRIVER_SPI_SOFT_XFER_ONCE);

    SPI_Soft_SignalEvent(instance, WD_SPI_SOFT_EVENT_TRANSFER_COMPLETE);

    res_unlock(instance);

    // WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "Soft SPI [%s] Transfer [Rx] 0x%.8X", (instance->Name != NULL) ? instance->Name : "?", data_out, WD_MSG_MAGIC, WD_NET_RX, data_in, num);

    return (WD_CODE_OK);
}

int32_t wd_softspi_control(wd_softspi_resources_t *instance, uint32_t control, uint32_t arg) {
    bool config = false;

    switch (control & WD_SPI_SOFT_CONTROL_Msk) {
    default:
        return WD_CODE_ERROR_UNSUPPORTED;

    case WD_SPI_SOFT_MODE_MASTER:
        config = true;
        break;

    case WD_SPI_SOFT_SET_BUS_SPEED:
        if (arg) {
            instance->Info->Baudrate = arg;
        } else {
            return WD_CODE_ERROR_PARAMETER;
        }
        return WD_CODE_OK;

    case WD_SPI_SOFT_GET_BUS_SPEED:
        return (int32_t)(instance->Info->Baudrate);

    case WD_SPI_SOFT_SET_DEFAULT_TX_VALUE:
        instance->Info->Def_Val = (uint8_t)arg;
        return WD_CODE_OK;

    case WD_SPI_SOFT_CONTROL_SS:
        if ((instance->Info->Control & WD_SPI_SOFT_CONTROL_Msk) != WD_SPI_SOFT_MODE_MASTER) {
            return WD_CODE_ERROR_UNSUPPORTED;
        }

        if ((instance->Info->Control & WD_SPI_SOFT_SS_MASTER_MODE_Msk) !=
            WD_SPI_SOFT_SS_MASTER_SW) {
            return WD_CODE_ERROR_UNSUPPORTED;
        }

        if (arg == WD_SPI_SOFT_SS_INACTIVE) {
            wd_gpio_write(instance->IO.CS, WD_GPIO_SET);
        } else {
            wd_gpio_write(instance->IO.CS, WD_GPIO_CLEAR);
        }
        return WD_CODE_OK;
    }

    // Frame format:
    switch (control & WD_SPI_SOFT_FRAME_FORMAT_Msk) {
    case WD_SPI_SOFT_CPOL0_CPHA0:
        instance->Info->Mode = WD_DRIVER_SPI_SOFT_MODE_0;
        break;

    case WD_SPI_SOFT_CPOL0_CPHA1:
        instance->Info->Mode = WD_DRIVER_SPI_SOFT_MODE_1;
        break;

    case WD_SPI_SOFT_CPOL1_CPHA0:
        instance->Info->Mode = WD_DRIVER_SPI_SOFT_MODE_2;
        break;

    case WD_SPI_SOFT_CPOL1_CPHA1:
        instance->Info->Mode = WD_DRIVER_SPI_SOFT_MODE_3;
        break;

    default:
        return WD_CODE_ERROR;
    }

    if ((control & WD_SPI_SOFT_DATA_BITS_Msk) != WD_SPI_SOFT_DATA_BITS(8U)) {
        return WD_CODE_ERROR;
    }

    if ((control & WD_SPI_SOFT_BIT_ORDER_Msk) == WD_SPI_SOFT_LSB_MSB) {
        return WD_CODE_ERROR;
    }

    if (!config) {
        return (WD_CODE_ERROR_PARAMETER);
    }

    instance->Info->Baudrate = arg;
    instance->Info->Control = control;
    instance->Info->Flags = (int32_t)((uint32_t)instance->Info->Flags | (1UL << 2));

    return (WD_CODE_OK);
}

#endif // #if RTE_DRIVERS_SPI_SOFT
#endif // #if 1
