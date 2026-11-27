/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright (c) 2024 Tilen MAJERLE (LwOW, MIT)
 */
/*
 * _wd_ds18b20.h
 *
 *  Created on: May 8, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_DS18B20_H
#define WD_COMPONENT_WD_DS18B20_H

#include "wd_core.h"

/* board HAL mapping : default CubeMX names, override in wd_board.h */
/* 1-Wire pins (STM32H7 : GPIO clocks on AHB4). TX/data = GPIO_PIN, RX = RX_PIN in USART mode. */
#ifndef RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_PORT
#define RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_PORT GPIOA
#endif
#ifndef RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_PIN
#define RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_PIN LL_GPIO_PIN_0
#endif
#ifndef RTE_COMPONENT_THERMOMETER_DS18B20_RX_PIN
#define RTE_COMPONENT_THERMOMETER_DS18B20_RX_PIN LL_GPIO_PIN_1
#endif
#ifndef RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_CLK_ENABLE
#define RTE_COMPONENT_THERMOMETER_DS18B20_GPIO_CLK_ENABLE() \
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA)
#endif
#ifndef RTE_COMPONENT_THERMOMETER_DS18B20_UART_HANDLE
#define RTE_COMPONENT_THERMOMETER_DS18B20_UART_HANDLE (&huart4)
#endif
#ifndef RTE_COMPONENT_THERMOMETER_DS18B20_UART_REGISTER
#define RTE_COMPONENT_THERMOMETER_DS18B20_UART_REGISTER UART4
#endif

#if (RTE_COMPONENT_THERMOMETER_DS18B20)

/***************************************************************************************************************************************************************
 **********  Lightweight onewire library - https://github.com/MaJerle/lwow
 **************************************************************************************************************************************************************/
/*--------------- lwow.h ---------------*/
/**
 * \defgroup        LWOW Lightweight onewire
 * \brief           Lightweight onewire
 * \{
 *
 * \note            Functions with `_raw` suffix do no implement
 *                  locking mechanism when used with operating system.
 */

/**
 * \brief           1-Wire result enumeration
 */
typedef enum {
    lwowOK = 0x00,   /*!< Device returned OK */
    lwowERRPRESENCE, /*!< Presence was not successful */
    lwowERRNODEV,    /*!< No device connected, maybe device removed during scan? */
    lwowERRTXRX,     /*!< Error while exchanging data */
    lwowERRBAUD,     /*!< Error setting baudrate */
    lwowERRPAR,      /*!< Parameter error */
    lwowERR,         /*!< General-Purpose error */
} lwowr_t;

/**
 * \brief           ROM structure
 */
typedef struct {
    uint8_t rom[8]; /*!< 8-bytes ROM address */
} lwow_rom_t;

/**
 * \defgroup        LWOW_LL Low-Level functions
 * \brief           Low-level device dependant functions
 * \{
 */

/**
 * \brief           1-Wire low-level driver structure
 */
typedef struct {
    /**
	 * \brief       Initialize low-level driver
	 * \param[in]   arg: Custom argument passed to \ref lwow_init function
	 * \return      `1` on success, `0` otherwise
	 */
    uint8_t (*init)(void *arg);

    /**
	 * \brief       De-initialize low-level driver
	 * \param[in]   arg: Custom argument passed to \ref lwow_init function
	 * \return      `1` on success, `0` otherwise
	 */
    uint8_t (*deinit)(void *arg);

    /**
	 * \brief       Set UART baudrate
	 * \param[in]   baud: Baudrate to set in units of bauds, normally `9600` or `115200`
	 * \param[in]   arg: Custom argument passed to \ref lwow_init function
	 * \return      `1` on success, `0` otherwise
	 */
    uint8_t (*set_baudrate)(uint32_t baud, void *arg);

    /**
	 * \brief       Transmit and receive bytes over UART hardware (or custom implementation)
	 *
	 * Bytes array for `tx` is already prepared to be directly transmitted over UART hardware,
	 * no data manipulation is necessary.
	 *
	 * At the same time, library must read received data on RX port and put it to `rx` data array,
	 * one by one, up to `len` number of bytes
	 *
	 * \param[in]   tx: Data to transmit over UART
	 * \param[out]  rx: Array to write received data to
	 * \param[in]   len: Number of bytes to exchange
	 * \param[in]   arg: Custom argument passed to \ref lwow_init function
	 * \return      `1` on success, `0` otherwise
	 */
    uint8_t (*tx_rx)(const uint8_t *tx, uint8_t *rx, size_t len, void *arg);
} lwow_ll_drv_t;

/**
 * \}
 */

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#endif
/**
 * \brief           1-Wire structure
 */
typedef struct {
    lwow_rom_t rom;     /*!< ROM address of last device found.
	 When searching for new devices, we always need last found address,
	 to be able to decide which way to go next time during scan. */
    uint8_t disrepancy; /*!< Disrepancy value on last search */
    void *arg;          /*!< User custom argument */

    const lwow_ll_drv_t *ll_drv; /*!< Low-level functions driver */
    //#if LWOW_CFG_OS || __DOXYGEN__
    //    LWOW_CFG_OS_MUTEX_HANDLE mutex; /*!< Mutex handle */
    //#endif                              /* LWOW_CFG_OS || __DOXYGEN__ */
} lwow_t;
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/**
 * \brief           Search callback function implementation
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_id: Rom address when new device detected.
 *                     Set to `NULL` when search finished
 * \param[in]       index: Current device index
 *                     When `rom_id = NULL`, value indicates number of total devices found
 * \param[in]       arg: Custom user argument
 * \return          \ref lwowOK on success, member of \ref lwowr_t otherwise
 */
typedef lwowr_t (*wd_ds18b20_lwow_search_cb_fn_t)(lwow_t *const ow, const lwow_rom_t *const rom_id,
                                                  size_t index, void *arg);

#define LWOW_UNUSED(x) ((void)(x)) /*!< Unused variable macro */

/**
 * \brief           Assert check function
 *
 * It returns \ref lwowERRPAR if condition check fails
 *
 * \param[in]       msg: Optional message parameter to print on failure
 * \param[in]       c: Condition to check for
 * \hideinitializer
 */
#define LWOW_ASSERT(msg, c)    \
    do {                       \
        if (!(c)) {            \
            return lwowERRPAR; \
        }                      \
    } while (0)

/**
 * \brief           Assert check function with return `0`
 *
 * It returns `0` if condition check fails
 *
 * \param[in]       msg: Optional message parameter to print on failure
 * \param[in]       c: Condition to check for
 * \hideinitializer
 */
#define LWOW_ASSERT0(msg, c) \
    do {                     \
        if (!(c)) {          \
            return 0;        \
        }                    \
    } while (0)

/**
 * \brief           Get size of statically declared array
 * \param[in]       x: Input array
 * \return          Number of array elements
 * \hideinitializer
 */
#define LWOW_ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

#define LWOW_CMD_RSCRATCHPAD   0xBE /*!< Read scratchpad command for 1-Wire devices */
#define LWOW_CMD_WSCRATCHPAD   0x4E /*!< Write scratchpad command for 1-Wire devices */
#define LWOW_CMD_CPYSCRATCHPAD 0x48 /*!< Copy scratchpad command for 1-Wire devices */
#define LWOW_CMD_RECEEPROM     0xB8 /*!< Read EEPROM command */
#define LWOW_CMD_RPWRSUPPLY    0xB4 /*!< Read power supply command */
#define LWOW_CMD_SEARCHROM     0xF0 /*!< Search ROM command */
#define LWOW_CMD_READROM       0x33 /*!< Read ROM command */
#define LWOW_CMD_MATCHROM      0x55 /*!< Match ROM command. Select device with specific ROM */
#define LWOW_CMD_SKIPROM       0xCC /*!< Skip ROM, select all devices */

lwowr_t lwow_init(lwow_t *const ow, const lwow_ll_drv_t *const ll_drv, void *arg);
void lwow_deinit(lwow_t *const ow);

lwowr_t lwow_protect(lwow_t *const ow, const uint8_t protect);
lwowr_t lwow_unprotect(lwow_t *const ow, const uint8_t protect);

lwowr_t lwow_reset_raw(lwow_t *const ow);
lwowr_t lwow_reset(lwow_t *const ow);

lwowr_t lwow_write_byte_ex_raw(lwow_t *const ow, const uint8_t btw, uint8_t *const br);
lwowr_t lwow_write_byte_ex(lwow_t *const ow, const uint8_t btw, uint8_t *const br);

lwowr_t lwow_read_byte_ex_raw(lwow_t *const ow, uint8_t *const br);
lwowr_t lwow_read_byte_ex(lwow_t *const ow, uint8_t *const br);

lwowr_t lwow_read_bit_ex_raw(lwow_t *const ow, uint8_t *const br);
lwowr_t lwow_read_bit_ex(lwow_t *const ow, uint8_t *const br);

lwowr_t lwow_search_reset_raw(lwow_t *const ow);
lwowr_t lwow_search_reset(lwow_t *const ow);

lwowr_t lwow_search_raw(lwow_t *const ow, lwow_rom_t *const rom_id);
lwowr_t lwow_search(lwow_t *const ow, lwow_rom_t *const rom_id);

lwowr_t lwow_search_with_command_raw(lwow_t *const ow, const uint8_t cmd, lwow_rom_t *const rom_id);
lwowr_t lwow_search_with_command(lwow_t *const ow, const uint8_t cmd, lwow_rom_t *const rom_id);

lwowr_t lwow_search_with_command_callback(lwow_t *const ow, const uint8_t cmd,
                                          size_t *const roms_found,
                                          const wd_ds18b20_lwow_search_cb_fn_t func,
                                          void *const arg);
lwowr_t lwow_search_with_callback(lwow_t *const ow, size_t *const roms_found,
                                  const wd_ds18b20_lwow_search_cb_fn_t func, void *const arg);

lwowr_t lwow_search_devices_with_command_raw(lwow_t *const ow, const uint8_t cmd,
                                             lwow_rom_t *const rom_id_arr, const size_t rom_len,
                                             size_t *const roms_found);
lwowr_t lwow_search_devices_with_command(lwow_t *const ow, const uint8_t cmd,
                                         lwow_rom_t *const rom_id_arr, const size_t rom_len,
                                         size_t *const roms_found);

lwowr_t lwow_search_devices_raw(lwow_t *const ow, lwow_rom_t *const rom_id_arr,
                                const size_t rom_len, size_t *const roms_found);
lwowr_t lwow_search_devices(lwow_t *const ow, lwow_rom_t *const rom_id_arr, const size_t rom_len,
                            size_t *const roms_found);

lwowr_t lwow_match_rom_raw(lwow_t *const ow, const lwow_rom_t *const rom_id);
lwowr_t lwow_match_rom(lwow_t *const ow, const lwow_rom_t *const rom_id);

lwowr_t lwow_skip_rom_raw(lwow_t *const ow);
lwowr_t lwow_skip_rom(lwow_t *const ow);

uint8_t lwow_crc(const void *const in, const size_t len);

/* Legacy functions, deprecated, to be removed in next major release */
uint8_t lwow_write_byte_raw(lwow_t *const ow, const uint8_t b);
uint8_t lwow_write_byte(lwow_t *const ow, const uint8_t b);
uint8_t lwow_read_byte_raw(lwow_t *const ow);
uint8_t lwow_read_byte(lwow_t *const ow);
uint8_t lwow_read_bit_raw(lwow_t *const ow);
uint8_t lwow_read_bit(lwow_t *const ow);

/**
 * \}
 */

/*--------------- lwow_device_ds18x20.h ---------------*/
/**
 * \ingroup         LWOW_DEVICES
 * \defgroup        LWOW_DEVICE_DS18x20 DS18x20 temperature sensor
 * \brief           Device driver for DS18x20 temperature sensor
 * \{
 *
 * \note            Functions with `_raw` suffix do no implement
 *                  locking mechanism when using with operating system.
 */

#define LWOW_DS18X20_ALARM_DISABLE  ((int8_t)-128) /*!< Disable alarm temperature */
#define LWOW_DS18X20_ALARM_NOCHANGE ((int8_t)-127) /*!< Do not modify current alarm settings */
#define LWOW_DS18X20_TEMP_MIN       ((int8_t)-55)  /*!< Minimum temperature */
#define LWOW_DS18X20_TEMP_MAX       ((int8_t)125)  /*!< Maximal temperature */

uint8_t lwow_ds18x20_start_raw(lwow_t *const ow, const lwow_rom_t *const rom_id);
uint8_t lwow_ds18x20_start(lwow_t *const ow, const lwow_rom_t *const rom_id);

uint8_t lwow_ds18x20_read_raw(lwow_t *const ow, const lwow_rom_t *const rom_id, float *const t);
uint8_t lwow_ds18x20_read(lwow_t *const ow, const lwow_rom_t *const rom_id, float *const t);

uint8_t lwow_ds18x20_set_resolution_raw(lwow_t *const ow, const lwow_rom_t *const rom_id,
                                        const uint8_t bits);
uint8_t lwow_ds18x20_set_resolution(lwow_t *const ow, const lwow_rom_t *const rom_id,
                                    const uint8_t bits);

uint8_t lwow_ds18x20_get_resolution_raw(lwow_t *const ow, const lwow_rom_t *const rom_id);
uint8_t lwow_ds18x20_get_resolution(lwow_t *const ow, const lwow_rom_t *const rom_id);

uint8_t lwow_ds18x20_set_alarm_temp_raw(lwow_t *const ow, const lwow_rom_t *const rom_id,
                                        int8_t temp_l, int8_t temp_h);
uint8_t lwow_ds18x20_set_alarm_temp(lwow_t *const ow, const lwow_rom_t *const rom_id, int8_t temp_l,
                                    int8_t temp_h);

lwowr_t lwow_ds18x20_search_alarm_raw(lwow_t *const ow, lwow_rom_t *const rom_id);
lwowr_t lwow_ds18x20_search_alarm(lwow_t *const ow, lwow_rom_t *const rom_id);

uint8_t lwow_ds18x20_is_b(lwow_t *const ow, const lwow_rom_t *const rom_id);
uint8_t lwow_ds18x20_is_s(lwow_t *const ow, const lwow_rom_t *const rom_id);

/**
 * \}
 */

/*--------------- scan_devices.h ---------------*/
lwowr_t wd_ds18b20_scan_onewire_devices(lwow_t *ow, lwow_rom_t *rom_ids, size_t rtf, size_t *rf);

/***************************************************************************************************************************************************************
 **********  Driver
 **************************************************************************************************************************************************************/
/**
 * @brief chip information definition
 */
#define WD_CHIP_NAME          "Maxim Integrated DS18B20" /**< chip name */
#define WD_MANUFACTURER_NAME  "Maxim Integrated"         /**< manufacturer name */
#define WD_SUPPLY_VOLTAGE_MIN 3.0f                       /**< chip min supply voltage */
#define WD_SUPPLY_VOLTAGE_MAX 5.5f                       /**< chip max supply voltage */
#define WD_MAX_CURRENT        4.0f                       /**< chip max current */
#define WD_TEMPERATURE_MIN    -55.0f                     /**< chip min operating temperature */
#define WD_TEMPERATURE_MAX    125.0f                     /**< chip max operating temperature */
#define WD_DRIVER_VERSION     2000                       /**< driver version */

/**
 * @brief chip command definition
 */
#define DS18B20_CMD_SEARCH_ROM        0xF0 /**< search rom command */
#define DS18B20_CMD_READ_ROM          0x33 /**< read rom command */
#define DS18B20_CMD_MATCH_ROM         0x55 /**< match rom command */
#define DS18B20_CMD_SKIP_ROM          0xCC /**< skip rom command */
#define DS18B20_CMD_ALARM_SEARCH      0xEC /**< alarm search command */
#define DS18B20_CMD_CONVERT_T         0x44 /**< convert command */
#define DS18B20_CMD_WRITE_SCRATCHPAD  0x4E /**< write scratchpad command */
#define DS18B20_CMD_READ_SCRATCHPAD   0xBE /**< read scratchpad command */
#define DS18B20_CMD_COPY_SCRATCHPAD   0x48 /**< copy scratchpad command */
#define DS18B20_CMD_RECALL_EE         0xB8 /**< recall ee command */
#define DS18B20_CMD_READ_POWER_SUPPLY 0xB4 /**< read power supply command */

#endif // RTE_COMPONENT_THERMOMETER_DS18B20

/***************************************************************************************************************************************************************
 **********  Driver (공용 인터페이스 - DS18B20/NTC 중 어느 쪽이 활성화되어도 항상 존재해야 함)
 **************************************************************************************************************************************************************/
typedef enum {
/*--------------- Command  : [ 0: 7] ---------------*/
#define WD_THERMOMETER_COMMAND_POS 0
#define WD_THERMOMETER_COMMAND_MSK (0xFFUL << WD_THERMOMETER_COMMAND_POS)
    WD_THERMOMETER_RESET = ((1UL) << WD_THERMOMETER_COMMAND_POS),
    WD_THERMOMETER_CONVERT_T = ((2UL) << WD_THERMOMETER_COMMAND_POS),
/*--------------- Control  : [ 8:31] ---------------*/
#define WD_THERMOMETER_CONTROL_POS 8
#define WD_THERMOMETER_CONTROL_MSK (0xFFFFFFUL << WD_THERMOMETER_CONTROL_POS)
    WD_THERMOMETER_CONTROL_1 = ((1UL << 0) << WD_THERMOMETER_CONTROL_POS),
} wd_ds18b20_thermometer_control_t;

typedef struct {
    uint32_t Busy : 1;
    uint32_t Error : 1;
    uint32_t Reserved : 30;
} wd_ds18b20_thermometer_status_t;

typedef enum {
    /*
	 WDIF_STATE_INITIALIZED = (1UL <<  0),
	 WDIF_STATE_POWERED     = (1UL <<  1),
	 WDIF_STATE_CONFIGURED  = (1UL <<  2),
	 */
    WD_THERMOMETER_STATE_1 = (1UL << 3),
} wd_ds18b20_thermometer_state_t;

typedef enum {
    WD_THERMOMETER_EVENT_FALLING = (1UL << 0),
    WD_THERMOMETER_EVENT_RISING = (1UL << 1),
} wd_ds18b20_thermometer_event_t;

typedef wd_event_callback_t wd_ds18b20_thermometer_signal_event_t;

typedef struct {
    uint32_t TimeOut; // us
} wd_ds18b20_thermometer_capabilities_t;

typedef struct {
    uint32_t Single_Wire;
} const wd_thermometer_io_t;

typedef struct {
    void *Owner;
    wd_ds18b20_thermometer_signal_event_t CB_Event;
    wd_ds18b20_thermometer_status_t volatile Status;
    int32_t volatile State;
    uint32_t volatile Event;
    uint32_t Time_Stamp;
    uint32_t Wait_Time;
    uint32_t Timer;
    float Temperature;
    uint32_t Error_Count;
} wd_ds18b20_thermometer_info_t;

typedef struct {
    const wd_ds18b20_thermometer_capabilities_t *Capabilities;
    wd_thermometer_io_t IO;
    wd_ds18b20_thermometer_info_t *Info;
} const wd_ds18b20_thermometer_resources_t;

typedef struct {
#if (RTE_CONFIG_DRIVER_EXTENSION)
    const char *(*GetVersion)(void);
    void *(*GetCapabilities)(void);
#endif
    int32_t (*Initialize)(wd_ds18b20_thermometer_signal_event_t cb_event);
#if (RTE_CONFIG_DRIVER_EXTENSION)
    int32_t (*Uninitialize)(void);
#endif
    int32_t (*PowerControl)(wd_power_state_t state);
    int32_t (*Get_Temperature)(float *data);
    int32_t (*Control)(uint32_t control, uint32_t arg);
    wd_ds18b20_thermometer_status_t (*Get_Status)(void);
    wd_ds18b20_thermometer_resources_t *(*Get_Priv)(void);
} const wd_thermometer_driver_t;

extern wd_thermometer_driver_t wd_drv_thermometer;

#endif /* WD_COMPONENT_WD_DS18B20_H */
