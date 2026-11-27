/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    wd_core.h
 * @brief   WD Library core entry header (mandatory for every module)
 *
 * Core = this folder (_WD_Core) only. Core never references anything outside the core.
 *   wd_def.h, wd_version.h       common definitions / library version
 *   _PF_Interface                driver interface types
 *   _PF_Print                    log / console (output and input are registered at run time)
 *   _PF_Time                     us/ms time base, delay, RTC
 *   _PF_Gpio                     RTE_PORT_xxx based GPIO
 *   _PF_Toolkit                  CRC / string / memory helpers
 *   RingBuffer, _PF_Circular_Buffer
 *   _OS_Port                     RTOS mutex / tick (empty without RTOS)
 *   _wd_core.c                   core state (system mode) and weak hooks
 *
 * Project provides : wd config headers (wd_config.h, wd_board.h)
 * Hooks (weak, override outside the core) :
 *   wd_os_tick_50ms_hook(), wd_time_rtc_init_hook(), wd_time_rtc_second_hook()
 ******************************************************************************/
#ifndef WD_CORE_WD_CORE_H
#define WD_CORE_WD_CORE_H

/* ----------------------------------------------------------------------------- C standard */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "stdbool.h"
#include "stdalign.h"
#include "stddef.h"
#include "stdint.h"
#include "time.h"
#include "math.h"
#include "assert.h"
#include "complex.h"
#include "ctype.h"
#include "float.h"
#include "inttypes.h"
#include "limits.h"

/* ----------------------------------------------------------------------------- project configuration */
#include "wd_config.h"
#include "wd_board.h"

/* core modules are mandatory : their RTE switches must not be disabled */
#if defined(RTE_PLATFORM_PRINT) && (RTE_PLATFORM_PRINT == 0)
#error "RTE_PLATFORM_PRINT is part of the WD core and cannot be disabled"
#endif
#if defined(RTE_PLATFORM_TIME) && (RTE_PLATFORM_TIME == 0)
#error "RTE_PLATFORM_TIME is part of the WD core and cannot be disabled"
#endif
#if defined(RTE_PLATFORM_GPIO) && (RTE_PLATFORM_GPIO == 0)
#error "RTE_PLATFORM_GPIO is part of the WD core and cannot be disabled"
#endif
#ifndef RTE_PLATFORM_INTERFACE
#define RTE_PLATFORM_INTERFACE 1
#endif
#ifndef RTE_PLATFORM_PRINT
#define RTE_PLATFORM_PRINT 1
#endif
#ifndef RTE_PLATFORM_TIME
#define RTE_PLATFORM_TIME 1
#endif
#ifndef RTE_PLATFORM_GPIO
#define RTE_PLATFORM_GPIO 1
#endif
#ifndef RTE_PLATFORM_TOOLKIT
#define RTE_PLATFORM_TOOLKIT 1
#endif
#ifndef RTE_PLATFORM_CIRCULAR_BUFFER
#define RTE_PLATFORM_CIRCULAR_BUFFER 1
#endif

/* ----------------------------------------------------------------------------- MCU (STM32H7 LL) / CMSIS */
#include "arm_math.h"
#include "arm_const_structs.h"

#include "stm32h7xx.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_lpuart.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_utils.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_ll_exti.h"
#include "stm32h7xx_ll_tim.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_rtc.h"
#if (USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif
#include "stm32h7xx_hal_uart_ex.h"
#include "stm32h7xx_hal_def.h"
#include "core_cm7.h"

/* ----------------------------------------------------------------------------- RTOS */
#if (RTE_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "freertos.h"
#endif

/* ----------------------------------------------------------------------------- core modules */
#include "wd_def.h"
#include "wd_version.h"
#include "_wd_os.h"
#include "_wd_ringbuf.h"
#include "_wd_if.h"
#include "_wd_cbuf.h"
#include "_wd_print.h"
#include "_wd_gpio.h"
#include "_wd_time.h"
#include "_wd_toolkit.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------------- watchdog */
/* watchdog refresh used inside long loops of the library (default : nothing). Define in wd_config.h to kick the WDT. */
#ifndef WD_WDT_REFRESH
#define WD_WDT_REFRESH()
#endif

/* fatal board error (HAL init failure inside a component). Default : CubeMX Error_Handler() from main.h. */
#ifndef WD_BOARD_ERROR_HANDLER
#define WD_BOARD_ERROR_HANDLER() Error_Handler()
#endif

/* ----------------------------------------------------------------------------- core state */
extern wd_system_mode_t wd_core_system_mode; /* defined in _wd_core.c */

/**
 * @brief  Check that an address is inside a valid memory range of the board (WD_BOARD_MEMORY_RANGES).
 * @return true when inside a range. Without WD_BOARD_MEMORY_RANGES every address is valid.
 * @note   Reentrant, ISR safe.
 */
bool wd_core_is_valid_address(uint32_t address);

/* ----------------------------------------------------------------------------- status report */
/** @brief sources reported through wd_core_status_hook() */
typedef enum {
    WD_STATUS_SOURCE_POWER = 0,
    WD_STATUS_SOURCE_ADC,
    WD_STATUS_SOURCE_SDRAM,
    WD_STATUS_SOURCE_NVM_FLASH,
    WD_STATUS_SOURCE_TEMPERATURE_SENSOR,
    WD_STATUS_SOURCE_MAX
} wd_status_source_t;

/* ----------------------------------------------------------------------------- shared scratch buffer */
#ifndef WD_CONF_CORE_SCRATCH_SIZE
#define WD_CONF_CORE_SCRATCH_SIZE \
    (32768U) /* default scratch size [bytes] when no system overrides it */
#endif

/**
 * @brief  Shared scratch (work) buffer for modules (NOR read-modify-write, SDRAM test, firmware download).
 * @return non-NULL buffer of wd_core_scratch_size() bytes
 * @note   ONE user at a time. Do not hold it across calls of another module that may use it.
 *         Default (weak) : static buffer of WD_CONF_CORE_SCRATCH_SIZE. _WD_System maps it to wd_global.Shared.
 */
uint8_t *wd_core_scratch_buffer(void);

/** @brief size of wd_core_scratch_buffer() [bytes] (weak, overridden together with the buffer) */
uint32_t wd_core_scratch_size(void);

/* ----------------------------------------------------------------------------- core hooks (weak) */
/**
 * @brief  Called every 50 ms from the RTOS tick hook of _OS_Port.
 * @note   ISR context. Must not block. Default : no operation.
 */
void wd_os_tick_50ms_hook(void);

/**
 * @brief  Called once when the RTC is (re)initialized by _PF_Time.
 * @note   Thread context. Default : no operation.
 */
void wd_time_rtc_init_hook(void);

/**
 * @brief  Called every RTC second (wake-up interrupt of _PF_Time).
 * @note   ISR context. Must not block. Default : no operation.
 */
void wd_time_rtc_second_hook(void);

/**
 * @brief  Component health report (e.g. ADC / SDRAM / NVM / temperature sensor init or runtime fault).
 * @param  source  wd_status_source_t
 * @param  status  WD_STATUS_OK or WD_STATUS_ERROR
 * @note   May be called from thread or ISR context. Must not block. Default : no operation.
 *         _WD_System maps it to wd_reg_status.
 */
void wd_core_status_hook(wd_status_source_t source, wd_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* WD_CORE_WD_CORE_H */
