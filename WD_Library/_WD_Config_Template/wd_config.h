/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    wd_config.h   (TEMPLATE - copy into your project and edit)
 * @brief   WD Library feature selection (RTE_xxx) and project parameters
 *
 *  - 1 : module compiled / enabled,  0 : disabled
 *  - Board / HAL handle mapping lives in wd_board.h
 *  - NVM layout lives in wd_memory_map.h
 ******************************************************************************/
#ifndef WD_CONFIG_WD_CONFIG_H
#define WD_CONFIG_WD_CONFIG_H

/***************************************************************************************************************************************************************
 **********  Project identity  (used by CLI / Firmware service / Environment defaults)
 **************************************************************************************************************************************************************/
#define RTE_CONFIG_PLATFORM_NAME      "WD-STM32H743"
#define RTE_CONFIG_FIRMWARE_VERSION   "WD Application 1.0.0"
#define RTE_CONFIG_BOOTLOADER_VERSION "WD Bootloader 1.0.0"
#define RTE_CONFIG_HARDWARE_VERSION   1
#define RTE_CONFIG_MEMORY_MAP_VERSION 100 /// V1.00

/* Software type : RTE_SOFTWARE_TYPE_APPLICATON / RTE_SOFTWARE_TYPE_BOOTLOADER  (macros in wd_def.h) */
#define RTE_CONFIG_SOFTWARE_VERSION                                              \
    RTE_SOFTWARE_VERSION(RTE_SOFTWARE_TYPE_APPLICATON, RTE_SOFTWARE_BUILD_DEBUG, \
                         RTE_SOFTWARE_LEVEL_NORMAL)
#define RTE_CONFIG_SOFTWARE_TYPE  RTE_SOFTWARE_TYPE(RTE_CONFIG_SOFTWARE_VERSION)
#define RTE_CONFIG_SOFTWARE_BUILD RTE_SOFTWARE_BUILD(RTE_CONFIG_SOFTWARE_VERSION)
#define RTE_CONFIG_SOFTWARE_LEVEL RTE_SOFTWARE_LEVEL(RTE_CONFIG_SOFTWARE_VERSION)

/***************************************************************************************************************************************************************
 **********  Debug / Console
 **************************************************************************************************************************************************************/
#define RTE_DEBUG          1
#define RTE_DRV_VALIDATION 0

#ifndef WD_CONF_CONSOLE_LEVEL_COMPILE
#define WD_CONF_CONSOLE_LEVEL_COMPILE \
    WD_CONSOLE_LEVEL_MOTORMOUTH // WD_CONSOLE_LEVEL_SILENT ~ WD_CONSOLE_LEVEL_MOTORMOUTH
#endif
#ifndef WD_CONF_CONSOLE_LEVEL_DEFAULT
#define WD_CONF_CONSOLE_LEVEL_DEFAULT WD_CONSOLE_LEVEL_MOTORMOUTH
#endif

#define RTE_PLATFORM_RTT           0
#define RTE_FPU_TYPE               2 // 2 : Double + Single precision FPU
#define WD_CONF_DMA_ALIGNMENT_SIZE 32

#define RTE_COMPONENT_CLI_TERMINAL (Console)

#define WD_CONF_DEVICE_DIAGNOSIS 1
#if WD_CONF_DEVICE_DIAGNOSIS
#define WD_CONF_DEVICE_DIAGNOSIS_CHECK_COMPILER      1
#define WD_CONF_DEVICE_DIAGNOSIS_CHECK_FUNCTION_LIST 0
#endif

/***************************************************************************************************************************************************************
 **********  Peripheral driver instances
 **************************************************************************************************************************************************************/
#define RTE_Drivers_GPIO
#define RTE_Drivers_NVIC
#define RTE_Drivers_DEBUG

#define RTE_Drivers_SPI1
#define RTE_Drivers_SPI2
#define RTE_Drivers_SPI3
#define RTE_Drivers_SPI4
#define RTE_Drivers_SPI5
#define RTE_Drivers_SPI6

#define RTE_Drivers_USART1
#define RTE_Drivers_USART2
#define RTE_Drivers_USART3
#define RTE_Drivers_USART4
#define RTE_Drivers_USART5
#define RTE_Drivers_USART6
#define RTE_Drivers_USART7
#define RTE_Drivers_USART8
#define RTE_Drivers_USART9
#define RTE_Drivers_USART10
#define RTE_Drivers_USART21

/***************************************************************************************************************************************************************
 **********  Platform  (core modules of _WD_Core are always built : interface / print / time / gpio / toolkit / buffers)
 **************************************************************************************************************************************************************/
#define RTE_PLATFORM_PRINT_HAL    1 // UART console back-end (_PF_Print_hal)
#define RTE_PLATFORM_PRINT_SERIAL 1
#define RTE_PLATFORM_PRINT_TELNET 1 // Telnet console back-end (_Component_Ethernet/_wd_eth_telnet)
#define RTE_PLATFORM_FFT          1

/***************************************************************************************************************************************************************
 **********  Drivers
 **************************************************************************************************************************************************************/
#define RTE_DRIVERS_SPI_SOFT     1
#define RTE_DRIVERS_I2C_SOFT     1
#define RTE_DRIVERS_BLOCK_DEVICE 1
#define RTE_DRIVERS_GPIO_FUNC    1

/***************************************************************************************************************************************************************
 **********  Components
 **************************************************************************************************************************************************************/
#define RTE_COMPONENT_CLI 0 /* CLI service : not part of this distribution */

#define RTE_COMPONENT_ADC 1
#if RTE_COMPONENT_ADC
#define RTE_COMPONENT_ADC_ADS127      0
#define RTE_COMPONENT_ADC_AD4134      1
#define RTE_COMPONENT_ADC_STM32H743XI 1 // internal ADC (NTC thermometer)
#endif

#define RTE_COMPONENT_DDS 1
#if RTE_COMPONENT_DDS
#define RTE_COMPONENT_DDS_AD9837_MAIN 1
#define RTE_COMPONENT_DDS_AD9837_SUB  1
#define RTE_COMPONENT_DDS_STM32H743XI 0
#endif

#define RTE_COMPONENT_IO_EXPANDER 0
#if RTE_COMPONENT_IO_EXPANDER
#define RTE_COMPONENT_IO_EXPANDER_MCP23017 0
#endif

#define RTE_COMPONENT_DIGIPOT 1
#if RTE_COMPONENT_DIGIPOT
#define RTE_COMPONENT_DIGIPOT_AD5235 1
#endif

#define RTE_COMPONENT_THERMOMETER 1
#if RTE_COMPONENT_THERMOMETER
#define RTE_COMPONENT_THERMOMETER_DS18B20 0 // 1-Wire DS18B20  (exclusive with NTC)
#define RTE_COMPONENT_THERMOMETER_NTC     1 // internal ADC + NTC
#endif

#define RTE_COMPONENT_VIO 1

#define RTE_COMPONENT_FLASH_INTERNAL 1
#define RTE_COMPONENT_I_FLASH        1
#if RTE_COMPONENT_I_FLASH
#define RTE_COMPONENT_BLOCK_DEVICE_I_FLASH 1
#endif

#define RTE_COMPONENT_NVM_NOR_FLASH 1 // QSPI NOR  (exclusive with NAND)
#if RTE_COMPONENT_NVM_NOR_FLASH
#define RTE_COMPONENT_BLOCK_DEVICE_NOR_FLASH_MX66L2G45G 1
#endif
#define RTE_COMPONENT_NVM_NAND_FLASH 0 // QSPI NAND (exclusive with NOR)
#if RTE_COMPONENT_NVM_NAND_FLASH
#define RTE_COMPONENT_BLOCK_DEVICE_NAND_FLASH_W25N04KBZEIR 1
#endif
#define RTE_NVM_DATA_FLASH 0

#define RTE_COMPONENT_SDRAM 1
#if RTE_COMPONENT_SDRAM
#define RTE_COMPONENT_SDRAM_IS42S32800G 1
#endif

#define RTE_COMPONENT_CAN 1

#define RTE_COMPONENT_NETWORK 1
#if RTE_COMPONENT_NETWORK
#define RTE_COMPONENT_INTERFACE_ETHERNET       1
#define RTE_COMPONENT_INTERFACE_ETHERNET_W6100 1
#define RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG 1
#define RTE_COMPONENT_PROTOCOL_FACTORY         1 // firmware download (Z-Modem style) protocol
#endif

/***************************************************************************************************************************************************************
 **********  Services
 **************************************************************************************************************************************************************/
#define RTE_RTOS 1
#if RTE_RTOS
#define RTE_RTOS_THREAD_NUMBER_MAX    10
#define RTE_RTOS_THREAD_NUMBER        8
#define RTE_RTOS_MUTEX_TIMEOUT        10000 // 10 s
#define RTE_RTOS_MUTEX_PRINT          1
#define RTE_RTOS_MESSAGEQUEUE_TIMEOUT 1000
#define RTE_RTOS_THREAD_MINUS_NUMBER  4
#endif

#define RTE_SERVICE_NET_BLE      0
#define RTE_SERVICE_NET_CAN      1
#define RTE_SERVICE_NET_ETHERNET 1
#define RTE_SERVICE_NET_S2E      0
#define RTE_SERVICE_NET_SERIAL   0
#define RTE_SERVICE_NET_WIFI     0

#define RTE_SERVICE_NET_RX_INTER_OCTET_TIMEOUT 5
#define RTE_SERVICE_NET_RX_FRAME_TIMEOUT       250
#define RTE_SERVICE_NET_QUEUE_SIZE             16

#define RTE_SERVICE_MMU      1
#define RTE_SERVICE_FIRMWARE 1

/***************************************************************************************************************************************************************
 **********  RTOS package selection (CMSIS-RTOS2 on FreeRTOS)
 **************************************************************************************************************************************************************/
#define RTE_CMSIS_RTOS2
#define RTE_CMSIS_RTOS2_FreeRTOS
#define RTE_RTOS_FreeRTOS_CONFIG_RTOS2
#define RTE_RTOS_FreeRTOS_CORE

/***************************************************************************************************************************************************************
 **********  Console print routing (used by _PF_Print_hal)
 **************************************************************************************************************************************************************/
#define dprint(fmt, ...) wd_print_hal_print_uart1(fmt, ##__VA_ARGS__)
#define mprint(fmt, ...) wd_print_hal_print_uart1(fmt, ##__VA_ARGS__)

/***************************************************************************************************************************************************************
 **********  Watchdog / work buffer (library hooks, optional)
 **************************************************************************************************************************************************************/
/* #define WD_WDT_REFRESH()                          HAL_IWDG_Refresh(&hiwdg)   // watchdog refresh inside long library loops (default : nothing) */
/* #define WD_CONF_CORE_SCRATCH_SIZE        (32768U)                   // only when _WD_System is NOT used (core default scratch buffer) */

/***************************************************************************************************************************************************************
 **********  Project (application) specific configuration
 **************************************************************************************************************************************************************/
/* Add application RTE_APPLICATION_xxx options here. The WD Library never references them. */

#endif /* WD_CONFIG_WD_CONFIG_H */
