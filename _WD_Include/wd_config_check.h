/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    wd_config_check.h
 * @brief   Compile time check of the RTE switch combination in wd_config.h
 *
 *  Included by wd.h. A module used alone (README section 4) does not need this file.
 *  Each rule names the switches to change, so a wrong configuration stops at the first
 *  #error instead of failing later inside a module.
 *  _Tools/wd_config_matrix.sh builds the supported combinations.
 ******************************************************************************/
#ifndef WD_INCLUDE_WD_CONFIG_CHECK_H
#define WD_INCLUDE_WD_CONFIG_CHECK_H

/* ----------------------------------------------------------------------------- drivers needed by components */
#if ((RTE_COMPONENT_ADC_AD4134) || (RTE_COMPONENT_ADC_ADS127) || (RTE_COMPONENT_DDS) || \
     (RTE_COMPONENT_DIGIPOT)) &&                                                        \
    !(RTE_DRIVERS_SPI_SOFT)
#error "RTE_COMPONENT_ADC_AD4134 / ADS127 / DDS / DIGIPOT need RTE_DRIVERS_SPI_SOFT = 1"
#endif
#if (RTE_COMPONENT_IO_EXPANDER_MCP23017) && !(RTE_DRIVERS_I2C_SOFT)
#error "RTE_COMPONENT_IO_EXPANDER_MCP23017 needs RTE_DRIVERS_I2C_SOFT = 1"
#endif
#if ((RTE_COMPONENT_I_FLASH) || (RTE_COMPONENT_NVM_NOR_FLASH) || \
     (RTE_COMPONENT_NVM_NAND_FLASH)) &&                          \
    !(RTE_DRIVERS_BLOCK_DEVICE)
#error "RTE_COMPONENT_I_FLASH / NVM_NOR_FLASH / NVM_NAND_FLASH need RTE_DRIVERS_BLOCK_DEVICE = 1"
#endif

/* ----------------------------------------------------------------------------- components */
#if (RTE_COMPONENT_ADC_AD4134) && !(RTE_COMPONENT_DDS)
#error "RTE_COMPONENT_ADC_AD4134 needs RTE_COMPONENT_DDS = 1 (AD9837 supplies the ADC master clock)"
#endif
#if (RTE_COMPONENT_NVM_NOR_FLASH) && (RTE_COMPONENT_NVM_NAND_FLASH)
#error \
    "RTE_COMPONENT_NVM_NOR_FLASH and RTE_COMPONENT_NVM_NAND_FLASH share the QSPI port : enable only one"
#endif
#if (RTE_COMPONENT_THERMOMETER_DS18B20) && (RTE_COMPONENT_THERMOMETER_NTC)
#error "RTE_COMPONENT_THERMOMETER_DS18B20 and RTE_COMPONENT_THERMOMETER_NTC : enable only one"
#endif

/* ----------------------------------------------------------------------------- services */
#if (RTE_SERVICE_FIRMWARE) && !((RTE_SERVICE_MMU) && (RTE_COMPONENT_I_FLASH))
#error \
    "RTE_SERVICE_FIRMWARE needs RTE_SERVICE_MMU = 1 and RTE_COMPONENT_I_FLASH = 1 (application image in internal flash)"
#endif
#if (RTE_SERVICE_NET_ETHERNET) && !(RTE_COMPONENT_INTERFACE_ETHERNET)
#error \
    "RTE_SERVICE_NET_ETHERNET needs RTE_COMPONENT_NETWORK = 1 and RTE_COMPONENT_INTERFACE_ETHERNET = 1"
#endif
#if (RTE_SERVICE_NET_CAN) && !(RTE_COMPONENT_CAN)
#error "RTE_SERVICE_NET_CAN needs RTE_COMPONENT_CAN = 1"
#endif
#if (RTE_COMPONENT_PROTOCOL_FACTORY) && !((RTE_SERVICE_NET_ETHERNET) || (RTE_SERVICE_NET_CAN))
#error \
    "RTE_COMPONENT_PROTOCOL_FACTORY is a network protocol : it needs RTE_SERVICE_NET_ETHERNET or RTE_SERVICE_NET_CAN"
#endif

#endif /* WD_INCLUDE_WD_CONFIG_CHECK_H */
