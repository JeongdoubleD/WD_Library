/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    wd_board.h   (board : STM32H743XI_Ref - reference board configuration)
 * @brief   Board / MCU mapping for the WD Library
 *          - MCU vendor headers (CubeMX generated peripheral headers)
 *          - HAL handle mapping used by WD_Library modules
 *          - GPIO pin mapping (RTE_PORT_xxx)
 *          - MX_Device / RTE_Device style peripheral description
 *
 *  Reference board : STM32H743XI (example values)
 *  Everything in this file is project / board specific.
 ******************************************************************************/
#ifndef WD_CONFIG_WD_BOARD_H
#define WD_CONFIG_WD_BOARD_H

/***************************************************************************************************************************************************************
 **********  MCU / CubeMX peripheral headers
 **************************************************************************************************************************************************************/
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "memorymap.h"
#include "fdcan.h"
#include "usart.h"
#include "quadspi.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "fmc.h"
#if (RTE_COMPONENT_THERMOMETER_NTC) || (RTE_COMPONENT_ADC_STM32H743XI)
#include "adc.h"
#endif

/***************************************************************************************************************************************************************
 **********  HAL handle mapping  (moved from _Components.h)
 **************************************************************************************************************************************************************/
/*--------------- Platform Time (us timer) ---------------*/
#define RTE_PLATFORM_TIME_HANDLE        htim7
#define RTE_PLATFORM_TIME_REGISTER      TIM7
#define RTE_PLATFORM_TIME_IRQ           TIM7_IRQn
#define RTE_PLATFORM_TIME_TICK          1000000
#define RTE_PLATFORM_TIME_INSTANCE_SIZE 10
#define RTE_PLATFORM_TIME_LATENCY       4
#define RTE_PLATFORM_TIME_MINIMUM_TIME  (10 + RTE_PLATFORM_TIME_LATENCY)

/*--------------- Platform Print (console UART) ---------------*/
#define RTE_PLATFORM_PRINT_BAUDRATE                 4000000u
#define RTE_PLATFORM_PRINT_SERIAL_DRIVER            (&Driver_USART1)
#define RTE_PLATFORM_PRINT_SERIAL_DRIVER_NUMBER     1
#define RTE_PLATFORM_PRINT_SERIAL_DRIVER_REGISTER   USART1
#define RTE_PLATFORM_PRINT_SERIAL_DRIVER_HANDLE     (&huart1)
#define RTE_PLATFORM_PRINT_SERIAL_DRIVER_IRQ        USART1_IRQn
#define RTE_PLATFORM_PRINT_SERIAL_DRIVER_IRQ_DMA_TX DMA2_Stream7_IRQn
#define RTE_PLATFORM_PRINT_SERIAL_DRIVER_IRQ_DMA_RX DMA2_Stream2_IRQn

/*--------------- Component / service HAL handles ---------------*/
/* Every module has the default below (CubeMX handle names) as an #ifndef in its own header.
 * Define a macro here only when the board uses a different handle. Values are pointers. */
//#define RTE_PLATFORM_TIME_RTC_HANDLE                                          (&hrtc)       /* core   : _PF_Time (RTC)           */
//#define RTE_PLATFORM_PRINT_HAL_UART1_HANDLE                                   (&huart1)     /* _PF_Print_hal, UART1..8 (USE_UARTn_CODE) */
//#define RTE_COMPONENT_ADC_AD4134_V_DRIVER_HANDLE                              (&hspi1)      /* AD4134 data SPI ch1..4      */
//#define RTE_COMPONENT_ADC_AD4134_I_DRIVER_HANDLE                              (&hspi2)
//#define RTE_COMPONENT_ADC_AD4134_CH3_DRIVER_HANDLE                            (&hspi3)
//#define RTE_COMPONENT_ADC_AD4134_CH4_DRIVER_HANDLE                            (&hspi5)
//#define RTE_COMPONENT_ADC_ADS127L_V_DRIVER_HANDLE                             (&hspi1)      /* ADS127L data SPI V / I      */
//#define RTE_COMPONENT_ADC_ADS127L_I_DRIVER_HANDLE                             (&hspi2)
//#define RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_HANDLE                          (&htim5)      /* ADS127L MCLK PWM            */
//#define RTE_COMPONENT_ADC_ADS127L_CLOCK_TIMER_REGISTER                        TIM5
//#define RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_HANDLE                           (&htim1)      /* AD9837 MCLK PWM             */
//#define RTE_COMPONENT_DDS_AD9837_CLOCK_TIMER_REGISTER                         TIM1
//#define RTE_COMPONENT_NVM_NOR_FLASH_DRIVER_HANDLE                             (&hqspi)      /* MX66L2G45G                  */
//#define RTE_COMPONENT_NVM_NAND_FLASH_DRIVER_HANDLE                            (&hqspi)      /* W25N04KBZEIR                */
//#define RTE_COMPONENT_SDRAM_DRIVER_HANDLE                                     (&hsdram1)    /* IS42S32800G (FMC)           */
//#define RTE_COMPONENT_THERMOMETER_NTC_ADC_HANDLE                              (&hadc1)      /* NTC on internal ADC         */
//#define RTE_COMPONENT_THERMOMETER_DS18B20_UART_HANDLE                         (&huart4)     /* DS18B20 1-wire over UART    */
//#define RTE_COMPONENT_THERMOMETER_DS18B20_UART_REGISTER                       UART4
//#define RTE_COMPONENT_CAN_DRIVER_HANDLE                                       (&hfdcan1)    /* FDCAN                       */
//#define RTE_COMPONENT_INTERFACE_ETHERNET_DRIVER_HANDLE                        (&hspi6)      /* W6100 SPI                   */
//#define RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE                              (&huart8)     /* factory / YMODEM UART       */

/* fatal board error called by components when a HAL init fails (default : CubeMX Error_Handler()) */
//#define WD_BOARD_ERROR_HANDLER()                                              Error_Handler()

/*--------------- Hardware option ---------------*/
#define RTE_HW_DDS_USE_OSC 1

/*--------------- Valid memory ranges (used by wd_core_is_valid_address) {start, end} ---------------*/
#define WD_BOARD_MEMORY_RANGES                                   \
    {0x00000000u, 0x0000FFFFu},     /* ITCM 64KB              */ \
        {0x20000000u, 0x2001FFFFu}, /* DTCM 128KB             */ \
        {0x24000000u, 0x2407FFFFu}, /* D1 AXI SRAM 512KB      */ \
        {0x30000000u, 0x30047FFFu}, /* D2 SRAM 288KB          */ \
        {0x38000000u, 0x3800FFFFu}, /* D3 SRAM4 64KB          */ \
        {0x38800000u, 0x38800FFFu}, /* BKPSRAM 4KB            */ \
        {0x08000000u, 0x081FFFFFu}, /* Internal Flash 2MB     */ \
        {0xC0000000u, 0xCFFFFFFFu}, /* External NVM 256MB (XIP) */

/***************************************************************************************************************************************************************
 **********  Pinout (RTE_PORT_xxx : WD_Pxx | ON/OFF polarity | direction/pull)
 **********  EXAMPLE VALUES of the reference STM32H743XI board : replace them with the wiring of your board.
 **********  Only the pins of the modules you enable are needed (README section 4 lists the modules).
 **************************************************************************************************************************************************************/
#define RTE_PORT_DEBUG_SWO                                                     \
    (uint32_t)((WD_PB_3) | WD_GPIO_ON_SET_OFF_CLEAR | WD_GPIO_DIR_PERIPHERAL | \
               WD_GPIO_SPEED_HIGH | WD_GPIO_TYPE_PUSH_PULL | WD_GPIO_PULL_UP)
#define RTE_CONFIG_IDLE_PIN_NC 0


#if RTE_COMPONENT_ADC_ADS127
#define RTE_PORT_ADC_I_INT \
    (uint32_t)((WD_PG_7) | WD_GPIO_DETECT_SET_NORMAL_CLEAR | WD_GPIO_INPUT_PULL_UP)
#define RTE_PORT_ADC_V_INT                                                                  \
    (uint32_t)((WD_PA_0) | WD_GPIO_DETECT_SET_NORMAL_CLEAR | WD_GPIO_PERIPHERAL_GPIO |      \
               WD_GPIO_DIR_INPUT | WD_GPIO_INTERRUPT_FALLING_EDGE | WD_GPIO_SPEED_DEFAULT | \
               WD_GPIO_PULL_UP | WD_GPIO_FUNC_GPIO_ALT_0 | WD_GPIO_FUNC_PERI_ALT_0)
//#define RTE_PORT_ADC_V_INT                                                      (uint32_t)((WD_PA_0 ) | WD_GPIO_DETECT_SET_NORMAL_CLEAR | WD_GPIO_INPUT_PULL_UP)
#define RTE_PORT_ADC_V_INT2 \
    (uint32_t)((WD_PC_12) | WD_GPIO_DETECT_SET_NORMAL_CLEAR | WD_GPIO_INPUT_PULL_UP)

#define RTE_PORT_ADC_V_CLOCK                                                   \
    (uint32_t)((WD_PA_2) | WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_PERIPHERAL |  \
               WD_GPIO_TYPE_PUSH_PULL | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_UP | \
               WD_GPIO_FUNC_PERI(GPIO_AF2_TIM5))
// #define RTE_PORT_ADC_I_CLOCK                                                    (uint32_t)((WD_PD_12 )| WD_GPIO_PERIPHERAL_GPIO  | WD_GPIO_DIR_PERIPHERAL   | WD_GPIO_TYPE_PUSH_PULL | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_UP | WD_GPIO_FUNC_PERI(GPIO_AF2_TIM4))
//#define RTE_PORT_ADC_START                                                      (uint32_t)((WD_PC_6 ) | WD_GPIO_PERIPHERAL_GPIO  | WD_GPIO_DIR_PERIPHERAL   | WD_GPIO_TYPE_PUSH_PULL | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_UP | WD_GPIO_FUNC_PERI(GPIO_AF2_TIM3))
#define RTE_PORT_ADC_START \
    (uint32_t)((WD_PC_6) | WD_GPIO_ON_SET_OFF_CLEAR | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_DDS_CLOCK                                                     \
    (uint32_t)((WD_PA_10) | WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_PERIPHERAL | \
               WD_GPIO_TYPE_PUSH_PULL | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_UP | \
               WD_GPIO_FUNC_PERI(GPIO_AF1_TIM1))

#define RTE_PORT_CTRL_MUX_A \
    (uint32_t)((WD_PC_7) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_CTRL_MUX_B \
    (uint32_t)((WD_PC_9) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_CTRL_SHDN \
    (uint32_t)((WD_PI_8) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_CTRL_VAC_AMP \
    (uint32_t)((WD_PI_11) | WD_GPIO_ON_SET_OFF_CLEAR | WD_GPIO_OUTPUT_PP_PULL_NONE)


#define RTE_PORT_SW_SPI_DDS_CS \
    (uint32_t)((WD_PH_2) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_SW_SPI_DIGIPOT_CS \
    (uint32_t)((WD_PC_13) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_SW_SPI_MISO \
    (uint32_t)((WD_PC_3) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_INPUT_PULL_UP)
#define RTE_PORT_SW_SPI_MOSI \
    (uint32_t)((WD_PF_10) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_SW_SPI_SCLK \
    (uint32_t)((WD_PH_3) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_SW_SPI2_DDS_CS \
    (uint32_t)((WD_PD_12) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)

#define RTE_PORT_W6100_CS \
    (uint32_t)((WD_PG_12) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_W6100_RESET \
    (uint32_t)((WD_PC_4) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)


#define RTE_PORT_SW_I2C_SDA \
    (uint32_t)((WD_PC_7) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_SW_I2C_SCL \
    (uint32_t)((WD_PC_9) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#else // Board revision : 2-stage board (default)
#define RTE_PORT_LED_STATUS \
    (uint32_t)((WD_PB_0) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_LED_POWER \
    (uint32_t)((WD_PB_1) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_LED_ERROR \
    (uint32_t)((WD_PB_10) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_LED_RUN \
    (uint32_t)((WD_PB_11) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_LED_ADC \
    (uint32_t)((WD_PB_12) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_LED_DDS \
    (uint32_t)((WD_PB_13) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_LED_COMM \
    (uint32_t)((WD_PB_14) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)

#define RTE_PORT_CTRL_MUX_A \
    (uint32_t)((WD_PG_2) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_CTRL_MUX_B \
    (uint32_t)((WD_PG_3) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_CTRL_SHDN \
    (uint32_t)((WD_Pi_8) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_CTRL_VAC_AMP \
    (uint32_t)((WD_Pi_11) | WD_GPIO_ON_SET_OFF_CLEAR | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_CTRL_LDO_5VA \
    (uint32_t)((WD_PJ_11) | WD_GPIO_ON_SET_OFF_CLEAR | WD_GPIO_OUTPUT_PP_PULL_NONE)

#define RTE_PORT_SW_SPI_MISO \
    (uint32_t)((WD_PC_3) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_INPUT_PULL_UP)
#define RTE_PORT_SW_SPI_MOSI \
    (uint32_t)((WD_PF_10) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_SW_SPI_SCLK \
    (uint32_t)((WD_PH_3) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)

#define RTE_PORT_SW_SPI_DDS_CS                        \
    (uint32_t)((WD_PH_2) | WD_GPIO_ON_CLEAR_OFF_SET | \
               WD_GPIO_OUTPUT_PP_PULL_NONE) //	H/W BOARD	AD9837 SPI CS PIN
#define RTE_PORT_SW_SPI_DDS_SUB_CS                     \
    (uint32_t)((WD_PA_10) | WD_GPIO_ON_CLEAR_OFF_SET | \
               WD_GPIO_OUTPUT_PP_PULL_NONE) //	H/W BOARD	DDS_SPI_CS2 : AD9837 Sub Clock 공급 제어용 CS

#define RTE_PORT_SW_SPI_ADC_CS                         \
    (uint32_t)((WD_PD_12) | WD_GPIO_ON_CLEAR_OFF_SET | \
               WD_GPIO_OUTPUT_PP_PULL_NONE) //	H/W BOARD	AD4134 SPI CS PIN

#define RTE_PORT_SW_SPI_DIGIPOT_CS                     \
    (uint32_t)((WD_PC_13) | WD_GPIO_ON_CLEAR_OFF_SET | \
               WD_GPIO_OUTPUT_PP_PULL_NONE) //	H/W BOARD	DIGIPOT SPI CS PIN

#define RTE_PORT_W6100_CS \
    (uint32_t)((WD_PG_12) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_W6100_RESET \
    (uint32_t)((WD_PC_4) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)

#define RTE_PORT_MUX_CAP_SEL1 \
    (uint32_t)((WD_PK_3) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_MUX_CAP_SEL2 \
    (uint32_t)((WD_PK_4) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_MUX_CAP_SEL3 \
    (uint32_t)((WD_PK_5) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_MUX_CAP_SEL4 \
    (uint32_t)((WD_PK_6) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)

#define RTE_PORT_CTRL_FAN \
    (uint32_t)((WD_PK_7) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)

/* Soft I2C (MCP23017 IO expander) : not wired on this board revision. The pins below are the
 * wiring of the previous board and only an example - set them to the real wiring before use. */
#if (RTE_COMPONENT_IO_EXPANDER_MCP23017)
#define RTE_PORT_SW_I2C_SDA \
    (uint32_t)((WD_PC_7) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#define RTE_PORT_SW_I2C_SCL \
    (uint32_t)((WD_PC_9) | WD_GPIO_ON_CLEAR_OFF_SET | WD_GPIO_OUTPUT_PP_PULL_NONE)
#endif

#endif

/**************************************************************************************************************************************************************
 **********  LL driver configuration (LL init structures are used by DS18B20 / GPIO helpers)
 **************************************************************************************************************************************************************/
#ifndef USE_FULL_LL_DRIVER
#define USE_FULL_LL_DRIVER
#endif

#endif /* WD_CONFIG_WD_BOARD_H */
