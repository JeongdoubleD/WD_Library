/* SPDX-License-Identifier: MIT AND BSD-3-Clause
 * Copyright (c) 2025-2026 DoubleD
 * Portions Copyright (c) 2024 STMicroelectronics (STM32CubeMX generated skeleton, BSD-3-Clause)
 */
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */ //작업 기록 :
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "_main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#if 0 //(RTE_RTOS)
StaticSemaphore_t Mutex_Global_cb_mem;
osMutexId_t wd_os_mutex_global;
const osMutexAttr_t wd_os_mutex_global_attr =
{
	.name = "Mutex_Global",
	.attr_bits = osMutexRecursive | osMutexPrioInherit,
	.cb_mem = &Mutex_Global_cb_mem,
	.cb_size = sizeof(Mutex_Global_cb_mem)
};
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
//void Software_Interface_Initialization(void);
//void Device_Initialization (void);
//void Interrupt_Initialization (void);
//void Application_Initialization (void);

static void MPU_Config(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _read(int file, char *ptr, int len) {
    (void)file;
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        //    *ptr++ =  Console.Getc();
        int32_t ch = wd_print_getc();
        *ptr++ = (ch < 0) ? (char)0 : (char)ch;
    }

    return len;
}

int _write(int file, char *ptr, int len) {
    (void)file;
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        //	  Console.Putc(*ptr++);
        wd_print_putc((uint8_t)*ptr++);
    }
    return len;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) /// ************************************ ■ main {
{
    SCB->VTOR = 0x08040800UL;
    __DSB();
    __ISB();

    wd_core_system_mode = WD_SYSTEM_MODE_BOOT;

    __disable_irq();
    /* USER CODE BEGIN 1 */
    Software_Interface_Initialization();
    Device_Initialization();
    Interrupt_Initialization();
    /* USER CODE END 1 */
    wd_reg_system->FUNC_Console = false;
    wd_faultlog_dump_if_present(); //test
    __enable_irq();

    /* Init scheduler */
    //	osKernelInitialize();

#if 0
//	// 테스트 코드 작성용
	while (1)
	{
		wd_time_wait_us(100);
	}
#endif
    /* Init scheduler */
#if (RTE_RTOS)
    /* Call init function for freertos objects (in cmsis_os2.c) */
    //	osKernelInitialize();
    //	MX_FREERTOS_Init();
    printf(WD_COLOR_TIME "Ver[%02d] - %s\t[%s]" WD_COLOR_RESET "\r\n\n",
           RTE_CONFIG_HARDWARE_VERSION, RTE_CONFIG_FIRMWARE_VERSION, wd_sys_firmware_build_time);
    wd_global.Debug_Point = 0xFFFFFFFF;

    Thread_API_Main_ID =
        osThreadNew(Thread_API_Main, NULL,
                    &Thread_API_Main_Attr); // ◇ CMSIS_RTOS #001 osThreadNew - Thread_Main

#ifndef NDEBUG
    if (Thread_API_Main_ID == NULL) {
        WD_LOG_EMERG("[Critical Error] Create API_Main Thread Error");
    }
#endif

    /* Start scheduler */
    osKernelStart();
#endif
    /* We should never get here as control is now taken by the scheduler */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        WD_LOG_EMERG("[Critical Error] RTOS Error - Software Reset !!!");
        NVIC_SystemReset();
    }
    /* USER CODE END 3 */
} /// *************************************************************************   main }


void Component_Initialization(
    void) /// ************************************ ■ Component_Initialization {
{
#ifndef NDEBUG
    WD_PRINT_DEBUG(WD_MSG_DELIMETER, WD_TERM_NULL);
#endif
#if ( \
    RTE_COMPONENT_FLASH_INTERNAL) /// *************************************** ★ System Initialization #001 DRV_MEMORYs {
    {
#if (RTE_COMPONENT_I_FLASH)
        WD_CHECK(wd_drv_block_device->Initialize(&wd_drv_block_device_i_flash, NULL));
#if (RTE_SERVICE_MMU)
        WD_CHECK(
            wd_mmu_attach(WD_MMU_OBJECT_ROM,
                          &wd_drv_block_device_i_flash)); // MMU : MCU ROM range -> internal flash
#endif
#endif
    }
#endif
#if ( \
    RTE_COMPONENT_VIO) /// *************************************** ★ System Initialization #012 wd_drv_vio {
    {
        wd_drv_vio.Initialize(NULL);
        WD_CHECK(wd_drv_vio.PowerControl(WD_POWER_FULL));

#ifndef NDEBUG
        WD_PRINT_DEBUG(WD_MSG_DELIMETER, WD_TERM_NULL);
#endif
    }
#endif /// *******************************************************************   System Initialization #012 wd_drv_vio }

#if ( \
    RTE_COMPONENT_SDRAM) /// *************************************** ★ System Initialization #013 DRV_MEMORYs {
    {
#if (RTE_COMPONENT_SDRAM_IS42S32800G)
        WD_CHECK(wd_drv_sdram.Initialize());
#endif
    }
#endif

#if (RTE_COMPONENT_NVM_NOR_FLASH)
    {
#if (RTE_COMPONENT_BLOCK_DEVICE_NOR_FLASH_MX66L2G45G)
        WD_CHECK(wd_drv_block_device->Initialize(&wd_drv_block_device_nor_flash, NULL));
#if (RTE_SERVICE_MMU)
        WD_CHECK(wd_mmu_attach(WD_MMU_OBJECT_NVM_FLASH,
                               &wd_drv_block_device_nor_flash)); // MMU : NVM range -> QSPI NOR
#endif
#endif
    }
#endif
#if (RTE_COMPONENT_NVM_NAND_FLASH)
    {
#if (RTE_COMPONENT_BLOCK_DEVICE_NAND_FLASH_W25N04KBZEIR)
        WD_CHECK(wd_drv_block_device->Initialize(&wd_drv_block_device_nand_flash, NULL));
#if (RTE_SERVICE_MMU)
        WD_CHECK(wd_mmu_attach(WD_MMU_OBJECT_NVM_FLASH,
                               &wd_drv_block_device_nand_flash)); // MMU : NVM range -> QSPI NAND
#endif
#endif
    }
#endif /// *******************************************************************   System Initialization #013 DRV_MEMORY }

#ifndef NDEBUG
    WD_PRINT_DEBUG(WD_MSG_DELIMETER, WD_TERM_NULL);
#endif
#if ( \
    RTE_COMPONENT_DDS) /// *************************************** ★ System Initialization #013 DRV_DDS {
    {
#if RTE_COMPONENT_ADC_AD4134
        WD_CHECK(wd_drv_dds_ad9837.Initialize(NULL));
        WD_CHECK(wd_drv_dds_ad9837.PowerControl(WD_POWER_FULL));
#else
#if (RTE_COMPONENT_DDS_AD9837_MAIN)
        WD_CHECK(DRV_DDS_AD9837_MAIN.Initialize(NULL));
        WD_CHECK(DRV_DDS_AD9837_MAIN.PowerControl(WD_POWER_FULL));
#endif
#if (RTE_COMPONENT_DDS_AD9837_SUB)
        WD_CHECK(DRV_DDS_AD9837_SUB.Initialize(NULL));
        WD_CHECK(DRV_DDS_AD9837_SUB.PowerControl(WD_POWER_FULL));
#endif
#endif

#if (RTE_COMPONENT_DDS_STM32H743XI)
        WD_CHECK(DRV_DDS_STM32H743XI.Initialize(NULL));
        WD_CHECK(DRV_DDS_STM32H743XI.PowerControl(WD_POWER_FULL));
#endif

#ifndef NDEBUG
        WD_PRINT_DEBUG(WD_MSG_DELIMETER, WD_TERM_NULL);
#endif
    }
#endif /// *******************************************************************   System Initialization #013 DRV_DDS }

#if ( \
    RTE_COMPONENT_ADC) /// *************************************** ★ System Initialization #014 DRV_ADC {
    {
#if (RTE_COMPONENT_ADC_ADS127)
        WD_CHECK(wd_drv_adc_ac.Initialize(NULL));
        WD_CHECK(wd_drv_adc_ac.PowerControl(WD_POWER_FULL));
#endif

#if (RTE_COMPONENT_ADC_AD4134)
        WD_CHECK(wd_drv_adc_ac.Initialize(NULL));
        WD_CHECK(wd_drv_adc_ac.PowerControl(WD_POWER_FULL));
#endif

#if (RTE_COMPONENT_ADC_ADS1252U)
        WD_CHECK(DRV_ADC_ADS1252U.Initialize(NULL));
        WD_CHECK(DRV_ADC_ADS1252U.PowerControl(WD_POWER_FULL));
#endif

#ifndef NDEBUG
        WD_PRINT_DEBUG(WD_MSG_DELIMETER, WD_TERM_NULL);
#endif
    }
#endif /// *******************************************************************   System Initialization #014 DRV_ADC }

#if ( \
    RTE_COMPONENT_IO_EXPANDER) /// *************************************** ★ System Initialization #016 DRV_IO_Expander_MCP23017T {
    {
#if (RTE_COMPONENT_IO_EXPANDER_MCP23017)
        WD_CHECK(wd_drv_io_expander_mcp23017.Initialize(NULL));
        WD_CHECK(wd_drv_io_expander_mcp23017.PowerControl(WD_POWER_FULL));

        wd_drv_io_expander_mcp23017.Control(WD_EXPANDER_MCP23017_PIN_SW_LOGIC, 1);
        wd_drv_io_expander_mcp23017.Control(WD_EXPANDER_MCP23017_PIN_MUX_A, 0);
        wd_drv_io_expander_mcp23017.Control(WD_EXPANDER_MCP23017_PIN_MUX_B, 1);

#endif

#ifndef NDEBUG
        WD_PRINT_DEBUG(WD_MSG_DELIMETER, WD_TERM_NULL);
#endif
    }
#endif /// *******************************************************************   System Initialization #016 DRV_IO_Expander_MCP23017T }

    wd_env_init(0);

#if ( \
    RTE_COMPONENT_DIGIPOT) /// *************************************** ★ System Initialization #015 DRV_DigiPOT {
    {
#if (RTE_COMPONENT_DIGIPOT_AD5235)
        WD_CHECK(wd_drv_digi_pot_ad5235.Initialize(NULL));
        WD_CHECK(wd_drv_digi_pot_ad5235.PowerControl(WD_POWER_FULL));
#endif

#ifndef NDEBUG
        WD_PRINT_DEBUG(WD_MSG_DELIMETER, WD_TERM_NULL);
#endif
    }
#endif /// *******************************************************************   System Initialization #015 DRV_DigiPOT }

#if ( \
    RTE_COMPONENT_THERMOMETER) /// *************************************** ★ System Initialization #017 wd_drv_thermometer {
    {
        WD_CHECK(wd_drv_thermometer.Initialize(NULL));
        WD_CHECK(wd_drv_thermometer.PowerControl(WD_POWER_FULL));

#ifndef NDEBUG
        WD_PRINT_DEBUG(WD_MSG_DELIMETER, WD_TERM_NULL);
#endif
    }
#endif /// *******************************************************************   System Initialization #017 wd_drv_thermometer }

} /// *************************************************************************   Component_Initialization }


void Software_Interface_Initialization(
    void) /// ************************************ ■ Software_Interface_Initialization {
{
    /* the register block is a NOLOAD section (not cleared by the start-up code) : clear all registers
     * (system, status, timer, per-thread timers, debug). Was memset(&pointer) : cleared nothing. */
    memset(wd_reg_alias, 0, sizeof(wd_reg_alias));

    HAL_Init();
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    SystemClock_Config();
    SystemCoreClockUpdate();
} /// *************************************************************************   Device_Initialization }


void Device_Initialization(void) /// ************************************ ■ Device_Initialization {
{
    //	PeriphCommonClock_Config();

    MPU_Config();

    SCB_EnableICache();
    SCB_EnableDCache();

    /* Initialize all configured peripherals : the MX_xxx_Init() functions CubeMX generates for the board (reference board list) */
#if (RTE_COMPONENT_ADC_STM32H743XI)
    MX_ADC1_Init();
#endif
    //	MX_ADC3_Init();
    //	MX_BDMA_Init();
    MX_DMA_Init();     //
    MX_FDCAN1_Init();  //
    MX_FMC_Init();     //
    MX_GPIO_Init();    //
    MX_QUADSPI_Init(); //
    MX_RTC_Init();     //
    MX_SPI1_Init();    //
    MX_SPI2_Init();    //
    MX_SPI3_Init();    //
    MX_SPI6_Init();    //
    MX_TIM2_Init();    //
    MX_TIM5_Init();
    MX_TIM6_Init();        //
    MX_UART4_Init();       //
    MX_UART5_Init();       //
    MX_UART8_Init();       //
    MX_USART2_UART_Init(); //
    MX_USART1_UART_Init(); //
                           // MX_SPI5_Init();				//  adc ch4번 비활성화
                           //	MX_SWPMI1_Init();			//
                           //	MX_USB_OTG_HS_HCD_Init();

    wd_time_init_dwt();
#if 1
    wd_time_debug_time = wd_time_get_us();
    //#if (RTE_PLATFORM_TIME)
    wd_drv_timer->Initialize(RTE_PLATFORM_TIME_TICK);
    //#endif
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();
    __HAL_RCC_GPIOK_CLK_ENABLE();

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

    wd_gpio_config(RTE_PORT_DEBUG_SWO, WD_GPIO_DIR_PERIPHERAL | WD_GPIO_TYPE_PUSH_PULL |
                                           WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_UP |
                                           WD_GPIO_FUNC_PERI(GPIO_AF0_SWJ));

#if (RTE_COMPONENT_ADC_AD4134)
    wd_gpio_config(RTE_PORT_SW_SPI_DIGIPOT_CS, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
    wd_gpio_config(RTE_PORT_SW_SPI_ADC_CS, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
    wd_gpio_config(RTE_PORT_SW_SPI_DDS_CS, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
    wd_gpio_config(RTE_PORT_SW_SPI_DDS_SUB_CS, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
#endif

#if RTE_COMPONENT_IO_EXPANDER_MCP23017
#else
    HAL_GPIO_WritePin(MUX_CAP_SEL1_GPIO_Port, MUX_CAP_SEL1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MUX_CAP_SEL2_GPIO_Port, MUX_CAP_SEL2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_CAP_SEL3_GPIO_Port, MUX_CAP_SEL3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_CAP_SEL4_GPIO_Port, MUX_CAP_SEL4_Pin, GPIO_PIN_RESET);
#endif
    wd_gpio_config(RTE_PORT_CTRL_LDO_5VA, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
    //    wd_gpio_write(RTE_PORT_CTRL_LDO_5VA, WD_GPIO_SET);
    HAL_GPIO_WritePin(CTRL_LDO_5VA_GPIO_Port, CTRL_LDO_5VA_Pin, GPIO_PIN_RESET); //

    wd_gpio_write(RTE_PORT_LED_STATUS, WD_GPIO_SET);
    wd_gpio_write(RTE_PORT_LED_POWER, WD_GPIO_CLEAR);
    wd_gpio_write(RTE_PORT_LED_ERROR, WD_GPIO_SET);

    wd_gpio_write(RTE_PORT_LED_RUN, WD_GPIO_SET);
    wd_gpio_write(RTE_PORT_LED_ADC, WD_GPIO_CLEAR);
    wd_gpio_write(RTE_PORT_LED_DDS, WD_GPIO_SET);
    wd_gpio_write(RTE_PORT_LED_COMM, WD_GPIO_CLEAR);

    wd_time_wait_us(100);
#endif

    wd_print_hal_init_app_uart();
    //    Console.Start();

#if (RTE_PLATFORM_PRINT_TELNET)
    wd_eth_telnet_initialize();
    wd_eth_telnet_state |= WD_TELNET_STATE_ENABLE;
#endif

    printf(WD_COLOR_RESET WD_ANSI_CLEAR);
    WD_PRINT_DEBUG(WD_MSG_DEBUG_CRLF, "Console [%d bps]", RTE_PLATFORM_PRINT_BAUDRATE);
    WD_PRINT_INFO(WD_MSG_GREEN, MSG_LOGO);

    wd_time_config();
    wd_time_read(&wd_time_time);

#ifndef NDEBUG
    WD_LOG_DEBUG("%s", asctime(&wd_time_time_tm));
#endif

#if (RTE_RTOS)
    {
        char infobuf[100];
        osVersion_t osv;
        osStatus_t status;

        if (osKernelGetState() == osKernelInactive) // Is the kernel initialized?
        {
            osKernelInitialize(); // Initialize CMSIS-RTOS kernel
        }

        status = osKernelGetInfo(&osv, infobuf, sizeof(infobuf));
        if (status == osOK) {
            WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Kernel Information: %s", infobuf);
            WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Kernel Version    : %d", osv.kernel);
            WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Kernel API Version: %d\r\n", osv.api);
        }
    }
#endif
} /// *************************************************************************   Device_Initialization }


void Interrupt_Initialization(
    void) /// ************************************ ■ Interrupt_Initialization {
{
    NVIC_SetPriority(WWDG_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(PVD_AVD_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TAMP_STAMP_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(RTC_WKUP_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(FLASH_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(RCC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                   WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                   WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(EXTI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(EXTI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(EXTI3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(EXTI4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA1_Stream0_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA1_Stream1_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA1_Stream2_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA1_Stream3_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA1_Stream4_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA1_Stream5_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA1_Stream6_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(ADC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                   WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                   WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(FDCAN1_IT0_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(FDCAN2_IT0_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(FDCAN1_IT1_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(FDCAN2_IT1_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM1_BRK_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM1_UP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM1_TRG_COM_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM1_CC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(I2C2_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(I2C2_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SPI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SPI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(EXTI15_10_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(RTC_Alarm_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM8_BRK_TIM12_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM8_UP_TIM13_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM8_CC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA1_Stream7_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(FMC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                   WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                   WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SDMMC1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SPI3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(UART4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(UART5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM6_DAC_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA2_Stream0_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA2_Stream1_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA2_Stream2_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA2_Stream3_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA2_Stream4_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(ETH_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                   WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                   WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(ETH_WKUP_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(FDCAN_CAL_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA2_Stream5_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA2_Stream6_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA2_Stream7_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(USART6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(I2C3_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(I2C3_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(OTG_HS_EP1_OUT_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(OTG_HS_EP1_IN_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(OTG_HS_WKUP_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(OTG_HS_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DCMI_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(RNG_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                   WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                   WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(FPU_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                   WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                   WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(UART7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(UART8_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SPI4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SPI5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SPI6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SAI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(LTDC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(LTDC_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMA2D_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SAI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(QUADSPI_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(LPTIM1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(CEC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                   WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                   WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(I2C4_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(I2C4_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SPDIF_RX_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(OTG_FS_EP1_OUT_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(OTG_FS_EP1_IN_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(OTG_FS_WKUP_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(OTG_FS_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMAMUX1_OVR_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(HRTIM1_Master_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(HRTIM1_TIMA_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(HRTIM1_TIMB_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(HRTIM1_TIMC_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(HRTIM1_TIMD_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(HRTIM1_TIME_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(HRTIM1_FLT_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DFSDM1_FLT0_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DFSDM1_FLT1_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DFSDM1_FLT2_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DFSDM1_FLT3_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SAI3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SWPMI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM15_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM16_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(TIM17_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(MDIOS_WKUP_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(MDIOS_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(JPEG_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(MDMA_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SDMMC2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(HSEM1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                     WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                     WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(ADC3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(DMAMUX2_OVR_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(BDMA_Channel0_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(BDMA_Channel1_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(BDMA_Channel2_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(BDMA_Channel3_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(BDMA_Channel4_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(BDMA_Channel5_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(BDMA_Channel6_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(BDMA_Channel7_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(COMP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(LPTIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(LPTIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(LPTIM4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(LPTIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                      WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                      WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(LPUART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                       WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                       WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(CRS_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                   WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                   WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(SAI4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                    WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                    WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
    NVIC_SetPriority(WAKEUP_PIN_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));

    NVIC_SetPriority(RTE_PLATFORM_PRINT_SERIAL_DRIVER_IRQ,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_HIGH_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_HIGH_HANDLER));
#if (RTE_PLATFORM_PRINT_SERIAL_DRIVER_IRQ_DMA_TX)
    NVIC_SetPriority(RTE_PLATFORM_PRINT_SERIAL_DRIVER_IRQ_DMA_TX,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_HIGH_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_HIGH_HANDLER));
#endif
    //#if (RTE_PLATFORM_PRINT_SERIAL_DRIVER_IRQ_DMA_RX)
    NVIC_SetPriority(RTE_PLATFORM_PRINT_SERIAL_DRIVER_IRQ_DMA_RX,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_HIGH_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_HIGH_HANDLER));
    //#endif
    NVIC_SetPriority(RTE_PLATFORM_TIME_IRQ,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_LOW_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_LOW_HANDLER));
} /// *************************************************************************   Interrupt_Initialization }

static void MPU_Config(void) /// ************************************ ■ MPU_Config {
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    /* Disable the MPU */
    HAL_MPU_Disable();

    /* Configure the MPU as Strongly ordered for not defined regions */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x00;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    //  BANK2 (0xD0000000) 설정 (필수!)
    //  MPU_InitStruct.BaseAddress = 0xD0000000;
    //  HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Configure the MPU attributes as WB for SDRAM */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0xD0000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER1;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
} /// *************************************************************************   MPU_Config }

void Service_Initialization(
    void) /// *************************************** □ 02_Service #000 Initialization {
{
    WD_WDT_REFRESH();

#if ( \
    RTE_RTOS) /// ***************************************** ★ System Initialization #000 wd_os_mutex_global {

    wd_os_mutex_global =
        osMutexNew(&wd_os_mutex_global_attr); // ◇ CMSIS_RTOS #002 osMutexNew - wd_os_mutex_global
    if (wd_os_mutex_global == NULL) {
        WD_LOG_ERROR("Mutex_Global New Error");
    }
#endif /// *******************************************************************   System Initialization #000 wd_os_mutex_global }

#if ( \
    RTE_SERVICE_MMU) /// ***************************************** ★ System Initialization #021 MMU {
    {
#if (RTE_DRV_VALIDATION)
        wd_mmu_validation();
#endif
    }
#endif /// *******************************************************************   System Initialization #021 MMU }

#if ( \
    RTE_SERVICE_FIRMWARE) /// ************************************ ★ System Initialization #022 Firmware {
    {
        wd_fw_initialization();
    }
#endif /// *******************************************************************   System Initialization #022 Firmware }

#if ( \
    RTE_SERVICE_NET_BLE) /// ************************************* ★ System Initialization #026 Net_Port_BLE {
    {
    }
#endif /// *******************************************************************   System Initialization #026 Net_Port_BLE }

#if ( \
    RTE_SERVICE_NET_CAN) /// ************************************* ★ System Initialization #026 wd_net_can_net_port_can {
    {
    }
#endif /// *******************************************************************   System Initialization #026 wd_net_can_net_port_can }

#if ( \
    RTE_SERVICE_NET_WIFI) /// ************************************ ★ System Initialization #026 Net_Port_Wifi {
    {
    }
#endif /// *******************************************************************   System Initialization #026 Net_Port_Wifi }

} /// *************************************************************************   02_Service #000 Initialization }


void Application_Initialization(
    void) /// ************************************ ■ 01_Application #000 Initialization {
{
    WD_WDT_REFRESH();

#ifndef NDEBUG
    /*--------------- Reset Reason ---------------*/
    WD_PRINT_INFO(WD_MSG_SYSTEM, "%s",
                  wd_sys_power_reset_reason_msg_box(wd_sys_power_get_reset_reason()));
#endif
    __HAL_RCC_CLEAR_RESET_FLAGS();
    WD_PRINT_INFO(
        WD_MSG_TITLE | WD_MSG_FLAG_CRLF,
        "\r\n\r\n"
        "----------------------------------------------------------------------------------\r\n"
        "  _____            _  _            _    _              _____  _              _    \r\n"
        " |  _  | ___  ___ | ||_| ___  ___ | |_ |_| ___  ___   |   __|| |_  ___  ___ | |_  \r\n"
        " |     || . || . || || ||  _|| .'||  _|| || . ||   |  |__   ||  _|| .'||  _||  _| \r\n"
        " |__|__||  _||  _||_||_||___||__,||_|  |_||___||_|_|  |_____||_|  |__,||_|  |_|   \r\n"
        "        |_|  |_|                                                                  \r\n\r\n"
        "                  Booting Time = [%.6f s]\r\n"
        "----------------------------------------------------------------------------------\r\n",
        (long double)((long double)(wd_time_get_cycle(NULL) & 0xFFFFFFFF) /
                      (long double)
                          SystemCoreClock)); // osKernelGetTickCount()  osKernelGetSysTimerCount()

} /// *************************************************************************   01_Application #000 Initialization }
