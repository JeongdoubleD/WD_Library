/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    wd.h
 * @brief   WD Library umbrella header  (replaces _app_header.h)
 *
 *  Include order
 *    1. wd_core.h     (mandatory core : config, board, MCU, RTOS, WD_Def, print/time/gpio/toolkit/buffers/os)
 *    2. third party (W6100)
 *    3. optional Platform -> Drivers -> Components -> Services -> System  (enabled by RTE_xxx)
 *    4. wd_memory_map.h (project : NVM layout)
 *
 *  "Use everything" umbrella. A single module can instead include wd_core.h + its own dependencies.
 ******************************************************************************/
#ifndef WD_INCLUDE_WD_H
#define WD_INCLUDE_WD_H

#include "wd_core.h"
#include "wd_config_check.h"

/***************************************************************************************************************************************************************
 **********  Third party
 **************************************************************************************************************************************************************/
#if (RTE_COMPONENT_INTERFACE_ETHERNET_W6100)
#include "w6100.h"
#include "socket.h"
#include "wizchip_conf.h"
#include "dhcpv4.h"
#include "dhcpv6.h"
#include "dns.h"
#endif

/***************************************************************************************************************************************************************
 **********  WD common
 **************************************************************************************************************************************************************/
#include "_wd_sys.h"

/***************************************************************************************************************************************************************
 **********  Platform
 **************************************************************************************************************************************************************/
#if (RTE_PLATFORM_PRINT_HAL)
#include "_wd_print_hal.h"
#endif
#if (RTE_PLATFORM_PRINT_SERIAL)
#include "_wd_print_serial.h"
#endif
#if (RTE_PLATFORM_FFT)
#include "_wd_fft.h"
#endif

/***************************************************************************************************************************************************************
 **********  Drivers
 **************************************************************************************************************************************************************/
#if (RTE_DRIVERS_SPI_SOFT)
#include "_wd_softspi.h"
#endif
#if (RTE_DRIVERS_I2C_SOFT)
#include "_wd_softi2c.h"
#endif
#if (RTE_DRIVERS_BLOCK_DEVICE)
#include "_wd_blockdev.h"
#endif
#if (RTE_DRIVERS_GPIO_FUNC)
#include "_wd_gpiofunc.h"
#endif

/***************************************************************************************************************************************************************
 **********  Components
 **************************************************************************************************************************************************************/

#if (RTE_COMPONENT_SDRAM)
#include "_wd_sdram.h"
#if (RTE_COMPONENT_SDRAM_IS42S32800G)
#include "_wd_sdram_is42s.h"
#endif
#endif

#if (RTE_COMPONENT_ADC)
#include "_wd_adc.h"
#if (RTE_COMPONENT_ADC_ADS127)
#include "_wd_ads127l.h"
#endif
#if (RTE_COMPONENT_ADC_AD4134)
#include "_wd_ad4134.h"
#endif
#endif

#if (RTE_COMPONENT_DDS)
#include "_wd_dds.h"
#if (RTE_COMPONENT_ADC_AD4134)
#include "_wd_ad9837_all.h"
#else
#if (RTE_COMPONENT_DDS_AD9837_MAIN)
#include "_wd_ad9837.h"
#endif
#if (RTE_COMPONENT_DDS_AD9837_SUB)
#include "_wd_ad9837_sub.h"
#endif
#endif
#endif

#if (RTE_COMPONENT_DIGIPOT_AD5235)
#include "_wd_ad5235.h"
#endif

#if (RTE_COMPONENT_DDS)
#if (RTE_COMPONENT_IO_EXPANDER_MCP23017)
#include "_wd_mcp23017.h"
#endif
#endif

#if (RTE_COMPONENT_FLASH_INTERNAL)
#include "_wd_iflash.h"
#endif
#if (RTE_COMPONENT_NVM_NOR_FLASH)
#include "_wd_nor.h"
#endif
#if (RTE_COMPONENT_NVM_NAND_FLASH)
#include "_wd_nand.h"
#endif

#if (RTE_COMPONENT_CAN)
#include "_wd_fdcan.h"
#endif

#if (RTE_COMPONENT_INTERFACE_ETHERNET)
#include "_wd_eth.h"
#include "_wd_eth_telnet.h"
#endif
#if (RTE_COMPONENT_PROTOCOL_FACTORY)
#include "_wd_factory.h"
#endif

#if (RTE_COMPONENT_THERMOMETER)
#include "_wd_ds18b20.h"
#if (RTE_COMPONENT_THERMOMETER_NTC)
#include "_wd_ntc.h"
#endif
#endif
#if (RTE_COMPONENT_VIO)
#include "_wd_vio.h"
#endif

/***************************************************************************************************************************************************************
 **********  Services
 **************************************************************************************************************************************************************/
#if (RTE_SERVICE_MMU)
#include "_wd_mmu.h"
#endif
#if (RTE_SERVICE_NET_CAN) || (RTE_SERVICE_NET_ETHERNET)
#include "_wd_net.h"
#endif
#if (RTE_SERVICE_NET_CAN)
#include "_wd_net_can.h"
#endif
#if (RTE_SERVICE_NET_ETHERNET)
#include "_wd_net_eth.h"
#endif
#if (RTE_SERVICE_FIRMWARE)
#include "_wd_fw.h"
#endif

/***************************************************************************************************************************************************************
 **********  System core
 **************************************************************************************************************************************************************/
#include "_wd_env.h"
#include "wd_memory_map.h"
#include "_wd_reg.h"
#include "_wd_sched.h"
#include "_wd_taskmon.h"
#include "_wd_faultlog.h"

#endif /* WD_INCLUDE_WD_H */
