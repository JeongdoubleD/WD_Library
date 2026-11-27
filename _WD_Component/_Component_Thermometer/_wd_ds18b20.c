/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright (c) 2024 Tilen MAJERLE (LwOW, MIT)
 */
/*
 * _wd_ds18b20.c
 *
 *  Created on: May 8, 2025
 *      Author: doubled
 */

#include "_wd_ds18b20.h"

#if (RTE_COMPONENT_THERMOMETER_DS18B20)

#define USART_DRV_NUM  4
#define USART_BAUDRATE 115200
#define USART_REGISTER RTE_COMPONENT_THERMOMETER_DS18B20_UART_REGISTER
#define USART_HANDLE   RTE_COMPONENT_THERMOMETER_DS18B20_UART_HANDLE

/***************************************************************************************************************************************************************
 **********  Lightweight onewire library - https://github.com/MaJerle/lwow
 **************************************************************************************************************************************************************/
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wpadded"
#if defined(__clang__) /* clang only options */
#pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#endif
#endif

#if 1
/* Pin setting */
#define GPIO_PORT     RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_PORT
#define GPIO_PIN      RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_PIN
#define OW_PIN_LOW    LL_GPIO_ResetOutputPin(GPIO_PORT, GPIO_PIN)
#define OW_PIN_HIGH   LL_GPIO_SetOutputPin(GPIO_PORT, GPIO_PIN)
#define OW_PIN_INPUT  LL_GPIO_SetPinMode(GPIO_PORT, GPIO_PIN, LL_GPIO_MODE_INPUT)
#define OW_PIN_OUTPUT LL_GPIO_SetPinMode(GPIO_PORT, GPIO_PIN, LL_GPIO_MODE_OUTPUT)

/* Macros for irq lock */
#define IRQ_LOCK_DEFINE uint32_t primask = __get_PRIMASK()
#define IRQ_LOCK        __disable_irq()
#define IRQ_UNLOCK      __set_PRIMASK(primask)

/* Function prototypes for driver */
static uint8_t prv_init(void *arg);
static uint8_t prv_deinit(void *arg);
static uint8_t prv_set_baudrate(uint32_t baud, void *arg);
static uint8_t prv_transmit_receive(const uint8_t *tx, uint8_t *rx, size_t len, void *arg);

/* Global driver structure for application use */
static const lwow_ll_drv_t ow_driver_gpio = {.init = prv_init,
                                             .deinit = prv_deinit,
                                             .set_baudrate = prv_set_baudrate,
                                             .tx_rx = prv_transmit_receive};
static uint32_t baudrate; /* Expected baudrate set by the application */

/**
 * \brief           Actual data exchange function
 *
 * This is the demo for STM32L4xx; with slight modifications it will work on any other architecture.
 *
 * Requirements to be provided for application
 * - microseconds timing function (example below uses 16-bit timer as source)
 * - interrupt locking mechanism
 * - GPIO manupulation features (open-drain mode, pull, etc)
 *
 * \param           low_init_pulse_time: Time in microseconds for initial low pulse width
 * \param           pre_sample_time: Time in us to wait after pin release.
 *                      Bus is samples after time expiration
 * \param           post_sample_time: Time in us to wait after sample has been completed
 * \return          Bus value, `1 == high`, `0 == low`
 */
static uint8_t prv_exch(uint16_t low_init_pulse_time, uint16_t pre_sample_time,
                        uint16_t post_sample_time) {
    uint8_t b = 0;
    uint32_t time, start_time;
    IRQ_LOCK_DEFINE;

    /* Lock interrupts and start execution */
    IRQ_LOCK;
    time = wd_time_get_us();

    /* Initiate start low pulse */
    start_time = time;
    OW_PIN_LOW;
    OW_PIN_OUTPUT;
    while ((uint32_t)((time = wd_time_get_us()) - start_time) < low_init_pulse_time) {
    }

    /* Release line and wait for mid pulse */
    start_time = time;
    OW_PIN_INPUT;
    while ((uint32_t)((time = wd_time_get_us()) - start_time) < pre_sample_time) {
    }

    /* Read pin state */
    b = (uint8_t)LL_GPIO_IsInputPinSet(GPIO_PORT, GPIO_PIN);

    /* Interrupts can now be enabled from this point */
    IRQ_UNLOCK;

    /* Wait remaining time */
    start_time = time;
    while ((uint32_t)((time = wd_time_get_us()) - start_time) < post_sample_time) {
    }

    return b;
}

/*******************************************/
/* LwOW driver interface functions         */
/*******************************************/
static uint8_t prv_init(void *arg) {
    LL_GPIO_InitTypeDef gpio_init;

    /* Peripheral clock enable */
    RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_CLK_ENABLE();

    /* Configure GPIO pin with open-drain mode */
    LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_INPUT;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio_init.Pull = LL_GPIO_PULL_UP;
    gpio_init.Pin = GPIO_PIN;
    LL_GPIO_Init(GPIO_PORT, &gpio_init);

    LWOW_UNUSED(arg);
    return 1;
}

static uint8_t prv_deinit(void *arg) {
    LWOW_UNUSED(arg);
    return 1;
}

static uint8_t prv_set_baudrate(uint32_t baud, void *arg) {
    LWOW_UNUSED(arg);
    baudrate = baud;
    return 1;
}

static uint8_t prv_transmit_receive(const uint8_t *tx, uint8_t *rx, size_t len, void *arg) {
    const uint8_t *t = tx;
    uint8_t *r = rx;

    /*
	 * For baudrate set at 9600 - by UART definition
	 * this fits timing only for reset sequence at onewire level
	 *
	 * Length must always be zero, or error is returned
	 */
    if (baudrate == 9600) {
        if (len == 1) {
            uint8_t v = prv_exch(480, 70, 410);
            *r = v ? *t : 0x01;
        } else {
            return 0;
        }
    } else if (baudrate == 115200) {
        /*
		 * Regular transmission process
		 *
		 * Exchange values and set timings for different events,
		 * according to the byte value to be transmitted
		 */
        for (size_t i = 0; i < len; ++i, ++r, ++t) {
            uint8_t v = prv_exch(*t ? 6 : 60, *t ? 9 : 0, *t ? 55 : 10);

            /*
			 * Set value as 0xFF in case of positive reading, 0x00 otherwise.
			 * This is to be compliant with LwOW UART expectations
			 */
            *r = v ? 0xFF : 0x00;
        }
    } else {
        return 0;
    }
    LWOW_UNUSED(arg);
    return 1;
}

#else
/* USART */
#define ONEWIRE_USART          USART_REGISTER
#define ONEWIRE_USART_CLK_EN   LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4)

/* USART TX PIN */
#define ONEWIRE_TX_PORT        RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_PORT
#define ONEWIRE_TX_PORT_CLK_EN RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_CLK_ENABLE()
#define ONEWIRE_TX_PIN         RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_PIN
#define ONEWIRE_TX_PIN_AF      LL_GPIO_AF_8

/* USART RX PIN */
#define ONEWIRE_RX_PORT        RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_PORT
#define ONEWIRE_RX_PORT_CLK_EN RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_CLK_ENABLE()
#define ONEWIRE_RX_PIN         RTE_COMPONENT_THERMOMETER_DS18B20_RX_PIN
#define ONEWIRE_RX_PIN_AF      LL_GPIO_AF_8

/*--------------- lwow_ll_stm32.c ---------------*/
static uint8_t init(void *arg);
static uint8_t deinit(void *arg);
static uint8_t set_baudrate(uint32_t baud, void *arg);
static uint8_t transmit_receive(const uint8_t *tx, uint8_t *rx, size_t len, void *arg);

/* STM32 LL driver for OW */
const lwow_ll_drv_t lwow_ll_drv_stm32 = {
    .init = init,
    .deinit = deinit,
    .set_baudrate = set_baudrate,
    .tx_rx = transmit_receive,
};

static LL_USART_InitTypeDef usart_init;

static uint8_t init(void *arg) {
    LL_GPIO_InitTypeDef gpio_init;

    /* Peripheral clock enable */
    ONEWIRE_USART_CLK_EN;
    ONEWIRE_TX_PORT_CLK_EN;
    ONEWIRE_RX_PORT_CLK_EN;

    /* Configure GPIO pins */
    LL_GPIO_StructInit(&gpio_init);
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio_init.Pull = LL_GPIO_PULL_UP;

    /* TX pin */
    gpio_init.Alternate = ONEWIRE_TX_PIN_AF;
    gpio_init.Pin = ONEWIRE_TX_PIN;
    LL_GPIO_Init(ONEWIRE_TX_PORT, &gpio_init);

    /* RX pin */
    gpio_init.Alternate = ONEWIRE_RX_PIN_AF;
    gpio_init.Pin = ONEWIRE_RX_PIN;
    LL_GPIO_Init(ONEWIRE_RX_PORT, &gpio_init);

    /* Configure UART peripherals */
    LL_USART_DeInit(ONEWIRE_USART);
    LL_USART_StructInit(&usart_init);
    usart_init.BaudRate = 9600;
    usart_init.DataWidth = LL_USART_DATAWIDTH_8B;
    usart_init.StopBits = LL_USART_STOPBITS_1;
    usart_init.Parity = LL_USART_PARITY_NONE;
    usart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;
    usart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    usart_init.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(ONEWIRE_USART, &usart_init);
    LL_USART_ConfigAsyncMode(ONEWIRE_USART);

    LWOW_UNUSED(arg);

    return 1;
}

static uint8_t deinit(void *arg) {
    LL_USART_DeInit(ONEWIRE_USART);
    LWOW_UNUSED(arg);
    return 1;
}

static uint8_t set_baudrate(uint32_t baud, void *arg) {
    usart_init.BaudRate = baud;
    LL_USART_Init(ONEWIRE_USART, &usart_init);
    LL_USART_ConfigAsyncMode(ONEWIRE_USART);
    LWOW_UNUSED(arg);

    return 1;
}

static uint8_t transmit_receive(const uint8_t *tx, uint8_t *rx, size_t len, void *arg) {
    const uint8_t *t = tx;
    uint8_t *r = rx;

    /* Send byte with polling */
    LL_USART_Enable(ONEWIRE_USART);
    for (; len > 0; --len, ++t, ++r) {
        LL_USART_TransmitData8(ONEWIRE_USART, *t);
        while (!LL_USART_IsActiveFlag_TXE(ONEWIRE_USART)) {
            ;
        }
        while (!LL_USART_IsActiveFlag_RXNE(ONEWIRE_USART)) {
            ;
        }
        *r = LL_USART_ReceiveData8(ONEWIRE_USART);
    }
    while (!LL_USART_IsActiveFlag_TC(ONEWIRE_USART)) {
    }
    LL_USART_Disable(ONEWIRE_USART);
    LWOW_UNUSED(arg);
    return 1;
}
#endif

/*--------------- lwow.c ---------------*/
/* Internal macros */
#define OW_FIRST_DEV 0xFF
#define OW_LAST_DEV  0x00

#define OW_RESET_BYTE 0xF0

/* Set value if not NULL */
#define SET_NOT_NULL(p, v) \
    if ((p) != NULL) {     \
        *(p) = (v);        \
    }

/**
 * \brief           Send single bit to OneWire port
 * \param[in]       ow: OneWire instance
 * \param[in]       btw: Bit to send, either `1` or `0`
 * \param[out]      btr: Pointer to output variable to write read bit
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
static lwowr_t prv_send_bit(lwow_t *const ow, uint8_t btw, uint8_t *btr) {
    uint8_t b;

    SET_NOT_NULL(btr, 0)

    /*
	 * To send logical 1 over 1-wire, send 0xFF over UART
	 * To send logical 0 over 1-wire, send 0x00 over UART
	 */
    btw = btw > 0 ? 0xFF : 0x00; /* Convert to 0 or 1 */
    if (!ow->ll_drv->tx_rx(&btw, &b, 1, ow->arg)) {
        return lwowERRTXRX; /* Transmit error */
    }
    b = b == 0xFF ? 1 : 0; /* Go to bit values */
    SET_NOT_NULL(btr, b)
    /* Set new byte */
    return lwowOK;
}

/**
 * \brief           Initialize OneWire instance
 * \param[in]       ow: OneWire instance
 * \param[in]       ll_drv: Low-level driver
 * \param[in]       arg: Custom argument
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_init(lwow_t *const ow, const lwow_ll_drv_t *const ll_drv, void *arg) {
    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("ll_drv != NULL", ll_drv != NULL);
    LWOW_ASSERT("ll_drv->init != NULL", ll_drv->init != NULL);
    LWOW_ASSERT("ll_drv->deinit != NULL", ll_drv->deinit != NULL);
    LWOW_ASSERT("ll_drv->set_baudrate != NULL", ll_drv->set_baudrate != NULL);
    LWOW_ASSERT("ll_drv->tx_rx != NULL", ll_drv->tx_rx != NULL);

    ow->arg = arg;
    ow->ll_drv = ll_drv;              /* Assign low-level driver */
    if (!ow->ll_drv->init(ow->arg)) { /* Init low-level directly */
        return lwowERR;
    }
#if LWOW_CFG_OS
    if (!lwow_sys_mutex_create(&ow->mutex, arg)) {
        ow->ll_drv->deinit(ow->arg); /* Deinit low-level */
        return lwowERR;
    }
#endif /* LWOW_CFG_OS */
    return lwowOK;
}

/**
 * \brief           Deinitialize OneWire instance
 * \param[in]       ow: OneWire instance
 */
void lwow_deinit(lwow_t *const ow) {
    if (ow == NULL || ow->ll_drv == NULL) {
        return;
    }

#if LWOW_CFG_OS
    lwow_sys_mutex_delete(&ow->mutex, ow->arg);
#endif /* LWOW_CFG_OS */
    ow->ll_drv->deinit(ow->arg);
}

/**
 * \brief           Protect 1-wire from concurrent access
 * \note            Used only for OS systems
 * \param[in,out]   ow: 1-Wire handle
 * \param[in]       protect: Set to `1` to protect core, `0` otherwise
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_protect(lwow_t *const ow, const uint8_t protect) {
    LWOW_ASSERT("ow != NULL", ow != NULL);

#if LWOW_CFG_OS
    if (protect && !lwow_sys_mutex_wait(&ow->mutex, ow->arg)) {
        return lwowERR;
    }
#else
    (void)ow;
    (void)protect;
#endif /* LWOW_CFG_OS */
    return lwowOK;
}

/**
 * \brief           Unprotect 1-wire from concurrent access
 * \note            Used only for OS systems
 * \param[in,out]   ow: 1-Wire handle
 * \param[in]       protect: Set to `1` to protect core, `0` otherwise
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_unprotect(lwow_t *const ow, const uint8_t protect) {
    LWOW_ASSERT("ow != NULL", ow != NULL);

#if LWOW_CFG_OS
    if (protect && !lwow_sys_mutex_release(&ow->mutex, ow->arg)) {
        return lwowERR;
    }
#else
    (void)ow;
    (void)protect;
#endif /* LWOW_CFG_OS */
    return lwowOK;
}

/**
 * \brief           Reset 1-Wire bus and set connected devices to idle state
 * \param[in,out]   ow: 1-Wire handle
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_reset_raw(lwow_t *const ow) {
    uint8_t b;

    LWOW_ASSERT("ow != NULL", ow != NULL);

    /* First send reset pulse */
    b = OW_RESET_BYTE; /* Set reset sequence byte = 0xF0 */
    if (!ow->ll_drv->set_baudrate(9600, ow->arg)) {
        return lwowERRBAUD; /* Error setting baudrate */
    }
    if (!ow->ll_drv->tx_rx(&b, &b, 1, ow->arg)) {
        return lwowERRTXRX; /* Error with data exchange */
    }
    if (!ow->ll_drv->set_baudrate(115200, ow->arg)) {
        return lwowERRBAUD; /* Error setting baudrate */
    }

    /* Check if there is reply from any device */
    if (b == 0 || b == OW_RESET_BYTE) {
        return lwowERRPRESENCE;
    }
    return lwowOK;
}

/**
 * \copydoc         lwow_reset_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_reset(lwow_t *const ow) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);

    lwow_protect(ow, 1);
    res = lwow_reset_raw(ow);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Write byte over OW and read its response
 * \param[in,out]   ow: 1-Wire handle
 * \param[in]       btw: Byte to write
 * \param[out]      br: Pointer to read value. Set to `NULL` if not used
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_write_byte_ex_raw(lwow_t *const ow, const uint8_t btw, uint8_t *const br) {
    uint8_t tr[8];

    LWOW_ASSERT("ow != NULL", ow != NULL);
    SET_NOT_NULL(br, 0);

    /* Prepare output data */
    for (uint8_t i = 0; i < 8; ++i) {
        /*
		 * If we have to send high bit, set byte as 0xFF,
		 * otherwise set it as low bit, 0x00
		 */
        tr[i] = (btw & (1 << i)) ? 0xFF : 0x00;
    }

    /*
	 * Exchange data on UART level,
	 * send single byte for each bit = 8 bytes
	 */
    if (!ow->ll_drv->tx_rx(tr, tr, 8, ow->arg)) {
        return lwowERRTXRX;
    }

    /* Update output value */
    if (br != NULL) {
        uint8_t r = 0;
        /*
		 * Check received data. If we read 0xFF,
		 * our logical write 1 was successful, otherwise it was 0.
		 */
        for (uint8_t i = 0; i < 8; ++i) {
            if (tr[i] == 0xFF) {
                r |= 0x01 << i;
            }
        }
        SET_NOT_NULL(br, r);
    }
    return lwowOK;
}

/**
 * \copydoc         lwow_write_byte_ex_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_write_byte_ex(lwow_t *const ow, const uint8_t btw, uint8_t *const br) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);

    lwow_protect(ow, 1);
    res = lwow_write_byte_ex_raw(ow, btw, br);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Read byte from OW device
 * \param[in,out]   ow: 1-Wire handle
 * \param[out]      br: Pointer to save read value
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_read_byte_ex_raw(lwow_t *const ow, uint8_t *const br) {
    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("br != NULL", br != NULL);

    /*
	 * When we want to read byte over 1-Wire,
	 * we have to send all bits as 1 and check if slave pulls line down.
	 *
	 * According to slave reactions, we can later construct received bytes
	 */
    return lwow_write_byte_ex_raw(ow, 0xFF, br);
}

/**
 * \copydoc         lwow_read_byte_ex_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_read_byte_ex(lwow_t *const ow, uint8_t *const br) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("br != NULL", br != NULL);

    lwow_protect(ow, 1);
    res = lwow_read_byte_ex_raw(ow, br);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Read sinle bit from OW device
 * \param[in,out]   ow: 1-Wire handle
 * \param[out]      br: Pointer to save read value, either `1` or `0`
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_read_bit_ex_raw(lwow_t *const ow, uint8_t *const br) {
    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("br != NULL", br != NULL);

    return prv_send_bit(ow, 1, br); /* Send bit as `1` and read the response */
}

/**
 * \copydoc         lwow_read_bit_ex_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_read_bit_ex(lwow_t *const ow, uint8_t *const br) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("br != NULL", br != NULL);

    lwow_protect(ow, 1);
    res = lwow_read_bit_ex_raw(ow, br);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Reset search
 * \param[in,out]   ow: 1-Wire handle
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_search_reset_raw(lwow_t *const ow) {
    LWOW_ASSERT("ow != NULL", ow != NULL);

    ow->disrepancy = OW_FIRST_DEV; /* Reset disrepancy to default value */
    return lwowOK;
}

/**
 * \copydoc         lwow_search_reset_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_search_reset(lwow_t *const ow) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);

    lwow_protect(ow, 1);
    res = lwow_search_reset_raw(ow);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Search for devices on 1-wire bus
 * \note            To reset search and to start over, use \ref lwow_search_reset function
 * \param[in,out]   ow: 1-Wire handle
 * \param[out]      rom_id: Pointer to ROM structure to save ROM
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_search_raw(lwow_t *const ow, lwow_rom_t *const rom_id) {
    LWOW_ASSERT("ow != NULL", ow != NULL);
    return lwow_search_with_command_raw(ow, LWOW_CMD_SEARCHROM, rom_id);
}

/**
 * \copydoc         lwow_search_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_search(lwow_t *const ow, lwow_rom_t *const rom_id) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);

    lwow_protect(ow, 1);
    res = lwow_search_raw(ow, rom_id);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Search for devices on 1-wire bus with custom search command
 * \note            To reset search and to start over, use \ref lwow_search_reset function
 * \param[in,out]   ow: 1-Wire handle
 * \param[in]       cmd: command to use for search operation
 * \param[out]      rom_id: Pointer to ROM structure to store address
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_search_with_command_raw(lwow_t *const ow, const uint8_t cmd,
                                     lwow_rom_t *const rom_id) {
    lwowr_t res;
    uint8_t id_bit_number, next_disrepancy, *id;

    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("rom_id != NULL", rom_id != NULL);

    id = ow->rom.rom;

    /* Check for last device */
    if (ow->disrepancy == 0) {
        lwow_search_reset_raw(ow); /* Reset search for next search */
        return lwowERRNODEV;       /* No devices anymore */
    }

    /* Step 1: Reset all devices on 1-Wire line to be able to listen for new command */
    if ((res = lwow_reset_raw(ow)) != lwowOK) {
        return res;
    }

    /* Step 2: Send search rom command for all devices on 1-Wire */
    lwow_write_byte_ex_raw(ow, cmd, NULL); /* Start with search ROM command */
    next_disrepancy = OW_LAST_DEV;         /* This is currently last device */

    for (id_bit_number = 64; id_bit_number > 0;) {
        uint8_t b, b_cpl;
        for (uint8_t j = 8; j > 0; --j, --id_bit_number) {
            /* Read first bit and its complimentary one */
            if (prv_send_bit(ow, 1, &b) != lwowOK || prv_send_bit(ow, 1, &b_cpl) != lwowOK) {
                return lwowERRTXRX;
            }

            /*
			 * If we have connected many devices on 1-Wire port, b and b_cpl are ANDed between all devices.
			 *
			 * We have to react if b and b_cpl are the same:
			 *
			 *  - Both 1: No devices on 1-Wire line responded
			 *      - No device connected at all
			 *      - All devices were put to block state due to search
			 *  - Both 0: We have "collision" as device with bit 0 and bit 1 are connected
			 *
			 * If b and b_cpl are different, it means we have:
			 *
			 *  - Single device connected on 1-Wire or
			 *  - All devices on 1-Wire have the same bit value at current position
			 *      - In this case, we move to direction of b value
			 */
            if (b && b_cpl) {
                goto out;
                /* We do not have device connected */
            } else if (!b && !b_cpl) {
                /*
				 * Decide which way to go for next scan
				 *
				 * Force move to "1" in case of:
				 *
				 *  - known diff position is larger than current bit reading
				 *  - Previous ROM address bit 0 was 1 and known diff is different than reading
				 *
				 * Because we shift *id variable down by 1 bit every iteration,
				 * *id & 0x01 always returns 1 if bit on previous ROM is the same as current bit
				 */
                if (id_bit_number < ow->disrepancy ||
                    ((*id & 0x01) && ow->disrepancy != id_bit_number)) {
                    b = 1;
                    next_disrepancy = id_bit_number;
                }
            }

            /*
			 * Devices are expecting master will send bit value back.
			 * All devices which do not have this bit value
			 * will go to blocked state and will wait for next reset sequence
			 *
			 * In case of "collision", we decide here which devices we will
			 * continue to scan (binary tree)
			 */
            prv_send_bit(ow, b, NULL);

            /*
			 * Because we shift down *id each iteration, we have to position bit value to the MSB position
			 * and it will be automatically positioned correct way.
			 */
            *id = (*id >> 0x01) |
                  (b << 0x07); /* Shift ROM byte down and add next, protocol is LSB first */
        }
        ++id; /* Go to next byte */
    }
out:
    ow->disrepancy = next_disrepancy;                      /* Save disrepancy value */
    memcpy(rom_id->rom, ow->rom.rom, sizeof(ow->rom.rom)); /* Copy ROM to user memory */
    return id_bit_number == 0 ? lwowOK : lwowERRNODEV;     /* Return search result status */
}

/**
 * \copydoc         lwow_search_with_command_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_search_with_command(lwow_t *const ow, const uint8_t cmd, lwow_rom_t *const rom_id) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("rom_id != NULL", rom_id != NULL);

    lwow_protect(ow, 1);
    res = lwow_search_with_command_raw(ow, cmd, rom_id);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Select device on 1-wire network with exact ROM number
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id: 1-Wire device address to match device
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_match_rom_raw(lwow_t *const ow, const lwow_rom_t *const rom_id) {
    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("rom_id != NULL", rom_id != NULL);

    /* Write byte to match rom exactly */
    if (lwow_write_byte_ex_raw(ow, LWOW_CMD_MATCHROM, NULL) != lwowOK) {
        return lwowERR;
    }
    for (uint8_t i = 0; i < 8; ++i) { /* Send 8 bytes representing ROM address */
        if (lwow_write_byte_ex_raw(ow, rom_id->rom[i], NULL) != lwowOK) { /* Send ROM bytes */
            return lwowERR;
        }
    }

    return lwowOK;
}

/**
 * \copydoc         lwow_match_rom_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_match_rom(lwow_t *const ow, const lwow_rom_t *const rom_id) {
    uint8_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("rom_id != NULL", rom_id != NULL);

    lwow_protect(ow, 1);
    res = lwow_match_rom_raw(ow, rom_id);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Skip ROM address and select all devices on the network
 * \param[in]       ow: 1-Wire handle
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_skip_rom_raw(lwow_t *const ow) {
    LWOW_ASSERT("ow != NULL", ow != NULL);

    return lwow_write_byte_ex_raw(ow, LWOW_CMD_SKIPROM, NULL);
}

/**
 * \copydoc         lwow_skip_rom_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_skip_rom(lwow_t *const ow) {
    uint8_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);

    lwow_protect(ow, 1);
    res = lwow_skip_rom_raw(ow);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Calculate CRC-8 of input data
 * \param[in]       in: Input data
 * \param[in]       len: Number of bytes
 * \return          Calculated CRC
 * \note            This function is reentrant
 */
uint8_t lwow_crc(const void *in, const size_t len) {
    uint8_t crc = 0;
    const uint8_t *d = in;

    if (in == NULL || len == 0) {
        return 0;
    }

    for (size_t i = 0; i < len; ++i, ++d) {
        uint8_t inbyte = *d;
        for (uint8_t j = 8; j > 0; --j) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix > 0) {
                crc ^= 0x8C;
            }
            inbyte >>= 0x01;
        }
    }
    return crc;
}

/**
 * \brief           Search devices on 1-wire network by using callback function and custom search command
 *
 * When new device is detected, callback function `func` is called to notify user
 *
 * \param[in]       ow: 1-Wire handle
 * \param[in]       cmd: 1-Wire search command
 * \param[out]      roms_found: Output variable to save number of found devices. Set to `NULL` if not used
 * \param[in]       func: Callback function to call for each device
 * \param[in]       arg: Custom user argument, used in callback function
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 * \note            This function is thread-safe
 */
lwowr_t lwow_search_with_command_callback(lwow_t *const ow, const uint8_t cmd,
                                          size_t *const roms_found,
                                          const wd_ds18b20_lwow_search_cb_fn_t func, void *arg) {
    lwowr_t res;
    lwow_rom_t rom_id;
    size_t i;

    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("func != NULL", func != NULL);

    lwow_protect(ow, 1);
    /* Search device-by-device until all found */
    for (i = 0, res = lwow_search_reset_raw(ow);
         res == lwowOK && (res = lwow_search_with_command_raw(ow, cmd, &rom_id)) == lwowOK; ++i) {
        if ((res = func(ow, &rom_id, i, arg)) != lwowOK) {
            break;
        }
    }
    func(ow, NULL, i, arg); /* Call with NULL rom_id parameter */
    lwow_unprotect(ow, 1);
    SET_NOT_NULL(roms_found, i); /* Set number of roms found */
    if (res == lwowERRNODEV) { /* `No device` might not be an error, but simply no devices on bus */
        res = lwowOK;
    }
    return res;
}

/**
 * \brief           Search devices on 1-wire network by using callback function and `SEARCH_ROM` 1-Wire command
 *
 * When new device is detected, callback function `func` is called to notify user
 *
 * \param[in]       ow: 1-Wire handle
 * \param[out]      roms_found: Output variable to save number of found devices. Set to `NULL` if not used
 * \param[in]       func: Callback function to call for each device
 * \param[in]       arg: Custom user argument, used in callback function
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 * \note            This function is thread-safe
 */
lwowr_t lwow_search_with_callback(lwow_t *const ow, size_t *const roms_found,
                                  const wd_ds18b20_lwow_search_cb_fn_t func, void *arg) {
    return lwow_search_with_command_callback(ow, LWOW_CMD_SEARCHROM, roms_found, func, arg);
}

/**
 * \brief           Search for devices on 1-Wire network with command and store ROM IDs to input array
 * \param[in]       ow: 1-Wire handle
 * \param[in]       cmd: 1-Wire search command
 * \param[in]       rom_id_arr: Pointer to output array to store found ROM IDs into
 * \param[in]       rom_len: Length of input ROM array
 * \param[out]      roms_found: Output variable to save number of found devices. Set to `NULL` if not used
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_search_devices_with_command_raw(lwow_t *const ow, const uint8_t cmd,
                                             lwow_rom_t *const rom_id_arr, const size_t rom_len,
                                             size_t *const roms_found) {
    lwowr_t res;
    size_t cnt = 0;

    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("rom_id_arr != NULL", rom_id_arr != NULL);
    LWOW_ASSERT("rom_len > 0", rom_len > 0);

    for (cnt = 0, res = lwow_search_reset_raw(ow); cnt < rom_len; ++cnt) {
        if ((res = lwow_search_with_command_raw(ow, cmd, &rom_id_arr[cnt])) != lwowOK) {
            break;
        }
    }
    SET_NOT_NULL(roms_found, cnt); /* Set number of roms found */
    if (res == lwowERRNODEV && cnt > 0) {
        res = lwowOK;
    }
    return res;
}

/**
 * \copydoc         lwow_search_devices_with_command_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_search_devices_with_command(lwow_t *const ow, const uint8_t cmd,
                                         lwow_rom_t *const rom_id_arr, const size_t rom_len,
                                         size_t *const roms_found) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("rom_id_arr != NULL", rom_id_arr != NULL);
    LWOW_ASSERT("rom_len > 0", rom_len > 0);

    lwow_protect(ow, 1);
    res = lwow_search_devices_with_command_raw(ow, cmd, rom_id_arr, rom_len, roms_found);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Search for devices on 1-Wire network with default command and store ROM IDs to input array
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id_arr: Pointer to output array to store found ROM IDs into
 * \param[in]       rom_len: Length of input ROM array
 * \param[out]      roms_found: Output variable to save number of found devices. Set to `NULL` if not used
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_search_devices_raw(lwow_t *const ow, lwow_rom_t *const rom_id_arr,
                                const size_t rom_len, size_t *const roms_found) {
    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("rom_id_arr != NULL", rom_id_arr != NULL);
    LWOW_ASSERT("rom_len > 0", rom_len > 0);

    return lwow_search_devices_with_command_raw(ow, LWOW_CMD_SEARCHROM, rom_id_arr, rom_len,
                                                roms_found);
}

/**
 * \copydoc         lwow_search_devices_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_search_devices(lwow_t *const ow, lwow_rom_t *const rom_id_arr, const size_t rom_len,
                            size_t *const roms_found) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);
    LWOW_ASSERT("rom_id_arr != NULL", rom_id_arr != NULL);
    LWOW_ASSERT("rom_len > 0", rom_len > 0);

    lwow_protect(ow, 1);
    res = lwow_search_devices_raw(ow, rom_id_arr, rom_len, roms_found);
    lwow_unprotect(ow, 1);
    return res;
}

/* Deprecated functions list */

/**
 * \brief           Write byte over 1-wire protocol
 * \note            This function is deprecated. Use \ref lwow_write_byte_ex_raw instead
 * \deprecated      This function is deprecated. Use \ref lwow_write_byte_ex_raw instead
 * \param[in,out]   ow: 1-Wire handle
 * \param[in]       b: Byte to write
 * \return          Received byte over 1-wire protocol
 */
uint8_t lwow_write_byte_raw(lwow_t *const ow, const uint8_t b) {
    uint8_t r;
    return lwow_write_byte_ex_raw(ow, b, &r) == lwowOK ? r : 0x00;
}

/**
 * \copydoc         lwow_write_byte_raw
 * \note            This function is deprecated. Use \ref lwow_write_byte_ex instead
 * \deprecated      This function is deprecated. Use \ref lwow_write_byte_ex instead
 * \note            This function is thread-safe
 */
uint8_t lwow_write_byte(lwow_t *const ow, const uint8_t b) {
    uint8_t r;
    return lwow_write_byte_ex(ow, b, &r) == lwowOK ? r : 0x00;
}

/**
 * \brief           Read next byte on 1-Wire
 * \note            This function is deprecated. Use \ref lwow_read_byte_ex_raw instead
 * \deprecated      This function is deprecated. Use \ref lwow_read_byte_ex_raw instead
 * \param[in,out]   ow: 1-Wire handle
 * \return          Byte read over 1-Wire
 */
uint8_t lwow_read_byte_raw(lwow_t *const ow) {
    uint8_t br;
    return lwow_read_byte_ex_raw(ow, &br) == lwowOK ? br : 0x00;
}

/**
 * \copydoc         lwow_read_byte_raw
 * \note            This function is deprecated. Use \ref lwow_read_byte_ex instead
 * \deprecated      This function is deprecated. Use \ref lwow_read_byte_ex instead
 * \note            This function is thread-safe
 */
uint8_t lwow_read_byte(lwow_t *const ow) {
    uint8_t br;
    return lwow_read_byte_ex(ow, &br) == lwowOK ? br : 0x00;
}

/**
 * \brief           Read single bit on 1-Wire network
 * \note            This function is deprecated. Use \ref lwow_read_bit_ex_raw instead
 * \deprecated      This function is deprecated. Use \ref lwow_read_bit_ex_raw instead
 * \param[in,out]   ow: 1-Wire handle
 * \return          Bit value
 */
uint8_t lwow_read_bit_raw(lwow_t *const ow) {
    uint8_t br;
    return lwow_read_bit_ex_raw(ow, &br) == lwowOK ? br : 0x00;
}

/**
 * \copydoc         lwow_read_bit_raw
 * \note            This function is deprecated. Use \ref lwow_read_bit_ex instead
 * \deprecated      This function is deprecated. Use \ref lwow_read_bit_ex instead
 * \note            This function is thread-safe
 */
uint8_t lwow_read_bit(lwow_t *const ow) {
    uint8_t br;
    return lwow_read_bit_ex(ow, &br) == lwowOK ? br : 0x00;
}

/*--------------- lwow_device_ds18x20.c ---------------*/
/**
 * \brief           Start temperature conversion on specific (or all) devices
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id: 1-Wire device address to start measurement for.
 *                      Set to `NULL` to start measurement on all devices at the same time
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwow_ds18x20_start_raw(lwow_t *const ow, const lwow_rom_t *const rom_id) {
    uint8_t ret = 0;

    LWOW_ASSERT0("ow != NULL", ow != NULL);

    if (lwow_reset_raw(ow) == lwowOK) {
        if (rom_id == NULL) {      /* Check for ROM id */
            lwow_skip_rom_raw(ow); /* Skip ROM, send to all devices */
        } else {
            lwow_match_rom_raw(ow, rom_id); /* Select exact device by ROM address */
        }
        lwow_write_byte_ex_raw(ow, 0x44, NULL); /* Start temperature conversion */
        ret = 1;
    }
    return ret;
}

/**
 * \copydoc         lwow_ds18x20_start_raw
 * \note            This function is thread-safe
 */
uint8_t lwow_ds18x20_start(lwow_t *const ow, const lwow_rom_t *const rom_id) {
    uint8_t res;

    LWOW_ASSERT0("ow != NULL", ow != NULL);

    lwow_protect(ow, 1);

    res = lwow_ds18x20_start_raw(ow, rom_id);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Read temperature previously started with \ref lwow_ds18x20_start
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id: 1-Wire device address to read data from
 * \param[out]      t: Pointer to output float variable to save temperature
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwow_ds18x20_read_raw(lwow_t *const ow, const lwow_rom_t *const rom_id, float *const t) {
    float dec;
    uint16_t temp;
    uint8_t ret = 0, data[9], crc, resolution, m = 0, bit_val;
    int8_t digit;

    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("t != NULL", t != NULL);
    if (rom_id != NULL) {
        LWOW_ASSERT0("lwow_ds18x20_is_b(ow, rom_id) || lwow_ds18x20_is_s(ow, rom_id)",
                     lwow_ds18x20_is_b(ow, rom_id) || lwow_ds18x20_is_s(ow, rom_id));
    }

    /*
	 * First read bit and check if all devices completed with conversion.
	 * If everything ready, try to reset the network and continue
	 */
    if (lwow_read_bit_ex_raw(ow, &bit_val) == lwowOK && bit_val != 0 &&
        lwow_reset_raw(ow) == lwowOK) {
        if (rom_id == NULL) {
            lwow_skip_rom_raw(ow);
        } else {
            lwow_match_rom_raw(ow, rom_id);
        }
        lwow_write_byte_ex_raw(ow, LWOW_CMD_RSCRATCHPAD, NULL);

        /* Read plain data from device */
        for (uint8_t i = 0; i < LWOW_ARRAYSIZE(data); ++i) {
            lwow_read_byte_ex_raw(ow, &data[i]);
        }
        crc = lwow_crc(data, LWOW_ARRAYSIZE(data));         /* Calculate CRC */
        if (crc == 0) {                                     /* Result must be 0 to match the CRC */
            temp = (data[1] << 0x08) | data[0];             /* Format data in integer format */
            resolution = ((data[4] & 0x60) >> 0x05) + 0x09; /* Set resolution in units of bits */
            if (temp & 0x8000) {                            /* Check for negative temperature */
                temp = ~temp + 1;                           /* Perform two's complement */
                m = 1;
            }
            digit = (int8_t)((temp >> 0x04) | (((temp >> 0x08) & 0x07) << 0x04));
            switch (resolution) { /* Check for resolution settings */
            case 9:
                dec = ((temp >> 0x03) & 0x01) * 0.5f;
                break;
            case 10:
                dec = ((temp >> 0x02) & 0x03) * 0.25f;
                break;
            case 11:
                dec = ((temp >> 0x01) & 0x07) * 0.125f;
                break;
            case 12:
                dec = (temp & 0x0F) * 0.0625f;
                break;
            default:
                dec = 0xFF;
                digit = 0;
            }
            dec += digit;
            if (m) {
                dec = -dec;
            }
            *t = dec;
            ret = 1;
        }
    }

    return ret;
}

/**
 * \copydoc         lwow_ds18x20_read_raw
 * \note            This function is thread-safe
 */
uint8_t lwow_ds18x20_read(lwow_t *const ow, const lwow_rom_t *const rom_id, float *const t) {
    uint8_t res;

    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("t != NULL", t != NULL);
    if (rom_id != NULL) {
        LWOW_ASSERT0("lwow_ds18x20_is_b(ow, rom_id) || lwow_ds18x20_is_s(ow, rom_id)",
                     lwow_ds18x20_is_b(ow, rom_id) || lwow_ds18x20_is_s(ow, rom_id));
    }

    lwow_protect(ow, 1);
    res = lwow_ds18x20_read_raw(ow, rom_id, t);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Get resolution for `DS18B20` device
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id: 1-Wire device address to get resolution from
 * \return          Resolution in units of bits (`9 - 12`) on success, `0` otherwise
 */
uint8_t lwow_ds18x20_get_resolution_raw(lwow_t *const ow, const lwow_rom_t *const rom_id) {
    uint8_t res = 0, br;

    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("rom_id != NULL", rom_id != NULL);
    LWOW_ASSERT0("lwow_ds18x20_is_b(ow, rom_id)", lwow_ds18x20_is_b(ow, rom_id));

    if (lwow_reset_raw(ow) == lwowOK) { /* Reset bus */
        lwow_match_rom_raw(ow, rom_id); /* Select device */
        lwow_write_byte_ex_raw(ow, LWOW_CMD_RSCRATCHPAD, NULL);

        /* Read and ignore bytes */
        lwow_read_byte_ex_raw(ow, &br);
        lwow_read_byte_ex_raw(ow, &br);
        lwow_read_byte_ex_raw(ow, &br);
        lwow_read_byte_ex_raw(ow, &br);

        lwow_read_byte_ex_raw(ow, &br);
        res = ((br & 0x60) >> 0x05) + 9; /* Read configuration byte and calculate bits */
    }

    return res;
}

/**
 * \copydoc         lwow_ds18x20_get_resolution_raw
 * \note            This function is thread-safe
 */
uint8_t lwow_ds18x20_get_resolution(lwow_t *const ow, const lwow_rom_t *const rom_id) {
    uint8_t res;

    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("rom_id != NULL", rom_id != NULL);
    LWOW_ASSERT0("lwow_ds18x20_is_b(ow, rom_id)", lwow_ds18x20_is_b(ow, rom_id));

    lwow_protect(ow, 1);
    res = lwow_ds18x20_get_resolution_raw(ow, rom_id);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Set resolution for `DS18B20` sensor
 * \note            `DS18S20` has fixed `9-bit` resolution
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id: 1-Wire device address to set resolution
 * \param[in]       bits: Number of resolution bits. Possible values are `9 - 12`
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwow_ds18x20_set_resolution_raw(lwow_t *const ow, const lwow_rom_t *const rom_id,
                                        const uint8_t bits) {
    uint8_t th, tl, conf, res = 0;

    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("bits >= 9 && bits <= 12", bits >= 9 && bits <= 12);
    LWOW_ASSERT0("lwow_ds18x20_is_b(ow, rom_id)", lwow_ds18x20_is_b(ow, rom_id));

    if (lwow_reset_raw(ow) == lwowOK) {
        if (rom_id == NULL) {
            lwow_match_rom_raw(ow, rom_id);
        } else {
            lwow_skip_rom_raw(ow);
        }
        lwow_write_byte_ex_raw(ow, LWOW_CMD_RSCRATCHPAD, NULL);

        /* Read and ignore bytes */
        lwow_read_byte_ex_raw(ow, &th);
        lwow_read_byte_ex_raw(ow, &th);

        /* Read important data */
        lwow_read_byte_ex_raw(ow, &th);
        lwow_read_byte_ex_raw(ow, &tl);
        lwow_read_byte_ex_raw(ow, &conf);

        conf &= (uint8_t)~0x60; /* Remove configuration bits for temperature resolution */
        switch (bits) {
        case 12:
            conf |= 0x60;
            break;
        case 11:
            conf |= 0x40;
            break;
        case 10:
            conf |= 0x20;
            break;
        case 9:
        default:
            break;
        }

        /* Write data back to device */
        if (lwow_reset_raw(ow) == lwowOK) {
            lwow_match_rom_raw(ow, rom_id);
            lwow_write_byte_ex_raw(ow, LWOW_CMD_WSCRATCHPAD, NULL);

            lwow_write_byte_ex_raw(ow, th, NULL);
            lwow_write_byte_ex_raw(ow, tl, NULL);
            lwow_write_byte_ex_raw(ow, conf, NULL);

            /* Copy scratchpad to non-volatile memory */
            if (lwow_reset_raw(ow) == lwowOK) {
                lwow_match_rom_raw(ow, rom_id);
                lwow_write_byte_ex_raw(ow, LWOW_CMD_CPYSCRATCHPAD, NULL);
                res = 1;
            }
        }
    }
    return res;
}

/**
 * \copydoc         lwow_ds18x20_set_resolution_raw
 * \note            This function is thread-safe
 */
uint8_t lwow_ds18x20_set_resolution(lwow_t *const ow, const lwow_rom_t *const rom_id,
                                    const uint8_t bits) {
    uint8_t res;

    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("bits >= 9 && bits <= 12", bits >= 9 && bits <= 12);
    LWOW_ASSERT0("lwow_ds18x20_is_b(ow, rom_id)", lwow_ds18x20_is_b(ow, rom_id));

    lwow_protect(ow, 1);
    res = lwow_ds18x20_set_resolution_raw(ow, rom_id, bits);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Set/clear temperature alarm high/low levels in units of degree Celcius
 * \note            `temp_h` and `temp_l` are high and low temperature alarms and can accept different values:
 *                      - `-55 % 125`, valid temperature range
 *                      - \ref LWOW_DS18X20_ALARM_DISABLE to disable temperature alarm (either high or low)
 *                      - \ref LWOW_DS18X20_ALARM_NOCHANGE to keep current alarm temperature (either high or low)
 *
 * Example usage would look something similar to:
 * \code{c}
 //Set alarm temperature; low = 10°C, high = 30°C
 lwow_ds18x20_set_alarm_temp(&ow, dev_id, 10, 30);
 //Set alarm temperature; low = disable, high = no change
 lwow_ds18x20_set_alarm_temp(&ow, dev_id, LWOW_DS18X20_ALARM_DISABLE, LWOW_DS18X20_ALARM_NOCHANGE);
 //Set alarm temperature; low = no change, high = disable
 lwow_ds18x20_set_alarm_temp(&ow, dev_id, LWOW_DS18X20_ALARM_NOCHANGE, LWOW_DS18X20_ALARM_DISABLE);
 //Set alarm temperature; low = 10°C, high = 30°C
 lwow_ds18x20_set_alarm_temp(&ow, dev_id, 10, 30);
 \endcode
 *
 *
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id: 1-Wire device address
 * \param[in]       temp_l: Alarm low temperature
 * \param[in]       temp_h: Alarm high temperature
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwow_ds18x20_set_alarm_temp_raw(lwow_t *const ow, const lwow_rom_t *const rom_id,
                                        int8_t temp_l, int8_t temp_h) {
    uint8_t res = 0, conf, th, tl;

    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("lwow_ds18x20_is_b(ow, rom_id)", lwow_ds18x20_is_b(ow, rom_id));

    /* Check if there is need to do anything */
    if (temp_l == LWOW_DS18X20_ALARM_NOCHANGE && temp_h == LWOW_DS18X20_ALARM_NOCHANGE) {
        return 1;
    }

    /* Check valid temp range for temperatures */
    if (temp_l != LWOW_DS18X20_ALARM_NOCHANGE) {
        if (temp_l == LWOW_DS18X20_ALARM_DISABLE || temp_l < LWOW_DS18X20_TEMP_MIN) {
            temp_l = LWOW_DS18X20_TEMP_MIN;
        } else if (temp_l > LWOW_DS18X20_TEMP_MAX) {
            temp_l = LWOW_DS18X20_TEMP_MAX;
        }
    }
    if (temp_h != LWOW_DS18X20_ALARM_NOCHANGE) {
        if (temp_h == LWOW_DS18X20_ALARM_DISABLE || temp_h > LWOW_DS18X20_TEMP_MAX) {
            temp_h = LWOW_DS18X20_TEMP_MAX;
        } else if (temp_h < LWOW_DS18X20_TEMP_MIN) {
            temp_h = LWOW_DS18X20_TEMP_MIN;
        }
    }

    if (lwow_reset_raw(ow) == lwowOK) {
        if (rom_id == NULL) {
            lwow_skip_rom_raw(ow);
        } else {
            lwow_match_rom_raw(ow, rom_id);
        }
        lwow_write_byte_ex_raw(ow, LWOW_CMD_RSCRATCHPAD, NULL);

        /* Read and ignore 2 bytes */
        lwow_read_byte_ex_raw(ow, &th);
        lwow_read_byte_ex_raw(ow, &th);

        /* Read important data */
        lwow_read_byte_ex_raw(ow, &th);
        lwow_read_byte_ex_raw(ow, &tl);
        lwow_read_byte_ex_raw(ow, &conf);

        /* Fill new values */
        th = temp_h == LWOW_DS18X20_ALARM_NOCHANGE ? (uint8_t)th : (uint8_t)temp_h;
        tl = temp_l == LWOW_DS18X20_ALARM_NOCHANGE ? (uint8_t)tl : (uint8_t)temp_l;

        /* Write scratchpad */
        if (lwow_reset_raw(ow) == lwowOK) {
            lwow_match_rom_raw(ow, rom_id);
            lwow_write_byte_ex_raw(ow, LWOW_CMD_WSCRATCHPAD, NULL);

            /* Write configuration register */
            lwow_write_byte_ex_raw(ow, th, NULL);
            lwow_write_byte_ex_raw(ow, tl, NULL);
            lwow_write_byte_ex_raw(ow, conf, NULL);

            /* Copy scratchpad to memory */
            if (lwow_reset_raw(ow) == lwowOK) {
                lwow_match_rom_raw(ow, rom_id);
                lwow_write_byte_ex_raw(ow, LWOW_CMD_CPYSCRATCHPAD, NULL);

                res = 1;
            }
        }
    }
    return res;
}

/**
 * \copydoc         lwow_ds18x20_set_alarm_temp_raw
 * \note            This function is thread-safe
 */
uint8_t lwow_ds18x20_set_alarm_temp(lwow_t *const ow, const lwow_rom_t *const rom_id, int8_t temp_l,
                                    int8_t temp_h) {
    uint8_t res;

    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("lwow_ds18x20_is_b(ow, rom_id)", lwow_ds18x20_is_b(ow, rom_id));

    lwow_protect(ow, 1);
    res = lwow_ds18x20_set_alarm_temp_raw(ow, rom_id, temp_l, temp_h);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Search for `DS18x20` devices with alarm flag
 * \note            To reset search, use \ref lwow_search_reset function
 * \param[in]       ow: 1-Wire handle
 * \param[out]      rom_id: Pointer to 8-byte long variable to save ROM
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
lwowr_t lwow_ds18x20_search_alarm_raw(lwow_t *const ow, lwow_rom_t *const rom_id) {
    return lwow_search_with_command_raw(ow, 0xEC, rom_id);
}

/**
 * \copydoc         lwow_ds18x20_search_alarm_raw
 * \note            This function is thread-safe
 */
lwowr_t lwow_ds18x20_search_alarm(lwow_t *const ow, lwow_rom_t *const rom_id) {
    lwowr_t res;

    LWOW_ASSERT("ow != NULL", ow != NULL);

    lwow_protect(ow, 1);
    res = lwow_ds18x20_search_alarm_raw(ow, rom_id);
    lwow_unprotect(ow, 1);
    return res;
}

/**
 * \brief           Check if ROM address matches `DS18B20` device
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id: 1-Wire device address to test against `DS18B20`
 * \return          `1` on success, `0` otherwise
 * \note            This function is reentrant
 */
uint8_t lwow_ds18x20_is_b(lwow_t *const ow, const lwow_rom_t *const rom_id) {
    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("rom_id != NULL", rom_id != NULL);

    LWOW_UNUSED(ow);
    return rom_id->rom[0] == 0x28;
}

/**
 * \brief           Check if ROM address matches `DS18S20` device
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id: 1-Wire device address to test against `DS18S20`
 * \return          `1` on success, `0` otherwise
 * \note            This function is reentrant
 */
uint8_t lwow_ds18x20_is_s(lwow_t *const ow, const lwow_rom_t *const rom_id) {
    LWOW_ASSERT0("ow != NULL", ow != NULL);
    LWOW_ASSERT0("rom_id != NULL", rom_id != NULL);

    LWOW_UNUSED(ow);
    return rom_id->rom[0] == 0x10;
}

/*--------------- scan_devices.c ---------------*/
/**
 * \brief           Scan for 1-Wire devices on specific 1-Wire port
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_ids: pointer to array of \ref lwow_rom_t structures
 * \param[in]       rtf: Number of roms to find
 * \param[out]      rf: Number of roms found after scan
 */
lwowr_t wd_ds18b20_scan_onewire_devices(lwow_t *ow, lwow_rom_t *rom_ids, size_t rtf, size_t *rf) {
    lwowr_t res;
    size_t found;

    /* Search for devices and save it to array */
    res = lwow_search_devices(ow, rom_ids, rtf, &found);

    /* Print all devices */
    for (size_t i = 0; i < found; ++i) {
        printf("Device ROM addr: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
               (unsigned)rom_ids[i].rom[0], (unsigned)rom_ids[i].rom[1],
               (unsigned)rom_ids[i].rom[2], (unsigned)rom_ids[i].rom[3],
               (unsigned)rom_ids[i].rom[4], (unsigned)rom_ids[i].rom[5],
               (unsigned)rom_ids[i].rom[6], (unsigned)rom_ids[i].rom[7]);
    }
    if (rf != NULL) {
        *rf = found;
    }
    return res;
}

static lwow_t ow;

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/***************************************************************************************************************************************************************
 **********  Driver
 **************************************************************************************************************************************************************/
static const wd_ds18b20_thermometer_capabilities_t Thermometer_DriverCapabilities = {
    .TimeOut = 100000,
};

static wd_ds18b20_thermometer_info_t Thermometer_Info = {
    0,
};

static wd_ds18b20_thermometer_resources_t Thermometer_Resources = {
    .Capabilities = &Thermometer_DriverCapabilities, .Info = &Thermometer_Info};

#undef LOCAL_RES
static wd_ds18b20_thermometer_resources_t *LOCAL_RES = &Thermometer_Resources;

#if 0
static void thermometer_callback(uint32_t event)
{
  LOCAL_RES->Info->Event = event;
  if(LOCAL_RES->Info->CB_Event)
  {
    LOCAL_RES->Info->CB_Event(event);
  }
}
#endif

#if (RTE_CONFIG_DRIVER_EXTENSION)
static const char *thermometer_get_version(void) {
    return (WD_CODE_OK);
}

static void *thermometer_get_capabilities(void) {
    return (WD_CODE_OK);
}
#endif

static int32_t thermometer_initialize(wd_ds18b20_thermometer_signal_event_t cb_event) {
    if (LOCAL_RES->Info->State & WD_STATE_INITIALIZED) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Thermometer DS18B20 Constructor");

    memset(&Thermometer_Info, 0, sizeof(Thermometer_Info));

    LOCAL_RES->Info->CB_Event = cb_event;

    LOCAL_RES->Info->State = WD_STATE_INITIALIZED;

    return (WD_CODE_OK);
}

#if (RTE_CONFIG_DRIVER_EXTENSION)
static int32_t thermometer_uninitialize(void) {
    return (WD_CODE_OK);
}
#endif

static int32_t thermometer_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Thermometer_DS18B20 PowerControl:WD_POWER_OFF");

        LOCAL_RES->Info->State &= ~(WD_STATE_POWERED | WD_STATE_CONFIGURED);
    } break;

    case WD_POWER_FULL: {
        if ((LOCAL_RES->Info->State & WD_STATE_INITIALIZED) == 0U) {
            return (WD_CODE_OK);
        }

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Thermometer_DS18B20 PowerControl:WD_POWER_FULL");

        {
#if 0
          USART_HANDLE->Instance = RTE_COMPONENT_THERMOMETER_DS18B20_UART_REGISTER;
          USART_HANDLE->Init.BaudRate = 9600;
          USART_HANDLE->Init.WordLength = UART_WORDLENGTH_8B;
          USART_HANDLE->Init.StopBits = UART_STOPBITS_1;
          USART_HANDLE->Init.Parity = UART_PARITY_NONE;
          USART_HANDLE->Init.Mode = UART_MODE_TX_RX;
          USART_HANDLE->Init.HwFlowCtl = UART_HWCONTROL_NONE;
          USART_HANDLE->Init.OverSampling = UART_OVERSAMPLING_16;
          USART_HANDLE->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
          USART_HANDLE->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
          USART_HANDLE->AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;

          if (HAL_HalfDuplex_Init(USART_HANDLE) != HAL_OK)
          {
            WD_LOG_ERROR();
          }
#else
            HAL_UART_DeInit(USART_HANDLE);
#endif
            lwow_init(&ow, &ow_driver_gpio, NULL);

            if (lwow_ds18x20_start(&ow, NULL) != 1) {
                wd_core_status_hook(WD_STATUS_SOURCE_TEMPERATURE_SENSOR, WD_STATUS_ERROR);
                WD_LOG_ERROR("Temperature Sensor Error !!!");
                /* error indication (LED etc.) is application policy : reported through wd_core_status_hook */
            } else {
                wd_core_status_hook(WD_STATUS_SOURCE_TEMPERATURE_SENSOR, WD_STATUS_OK);
                WD_LOG_DEBUG("Temperature Sensor Connect");
            }
        }

        LOCAL_RES->Info->State |= (WD_STATE_POWERED | WD_STATE_CONFIGURED);
    } break;

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);

    default:
        return (WD_CODE_ERROR);
    }

    return (WD_CODE_OK);
}

static int32_t thermometer_get_temperature(float *data) {
    int32_t ret = WD_CODE_OK;
    float temperature;

    if (lwow_ds18x20_read(&ow, NULL, &temperature)) {
        LOCAL_RES->Info->Temperature = temperature;
        WD_PRINT_DEBUG(WD_MSG_DEBUG_CRLF, "Sensor temperature is %f degrees", (double)temperature);
    } else {
        ret = WD_CODE_ERROR;
    }

    if (data) {
        *data = temperature;
    }

    return (ret);
}

static int32_t thermometer_control(uint32_t control, uint32_t arg) {
    int32_t ret = WD_CODE_OK;

    UNUSED(arg);

    if ((control & WD_THERMOMETER_COMMAND_MSK) == WD_THERMOMETER_CONVERT_T) {
        if (lwow_ds18x20_start(&ow, NULL) == 0) {
            ret = WD_CODE_ERROR;
        }
    }

    return (ret);
}

static wd_ds18b20_thermometer_status_t thermometer_get_status(void) {
    return (LOCAL_RES->Info->Status);
}

static wd_ds18b20_thermometer_resources_t *thermometer_get_priv(void) {
    return (LOCAL_RES);
}

wd_thermometer_driver_t wd_drv_thermometer = {
    .Initialize = thermometer_initialize,
    .PowerControl = thermometer_power_control,
    .Get_Temperature = thermometer_get_temperature,
    .Control = thermometer_control,
    .Get_Status = thermometer_get_status,
    .Get_Priv = thermometer_get_priv,
};

#endif // RTE_COMPONENT_THERMOMETER_DS18B20
