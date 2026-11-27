/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _wd_reg.h
 * @brief   WD system register model (event / status / timer / debug registers)
 *          and global shared work buffers used by the WD Library modules.
 *
 *  - wd_reg_system : event flags (set by producers, consumed by threads)
 *  - wd_reg_status : component health (0:OK, 1:Error)
 *  - wd_reg_timer  : tick / RTC counters
 *  - Global   : shared scratch buffers (union - only one user at a time)
 *
 *  Application specific events / status : use EVT_User[] / Status_User[]
 *  or keep an application register block in the application layer.
 ******************************************************************************/
/* umbrella header first (before the guard) : any include order expands in the umbrella order */
#include "wd.h"

#ifndef WD_SYSTEM_WD_REG_H
#define WD_SYSTEM_WD_REG_H

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef WD_REGISTER_REGION_ADDRESS
#define WD_REGISTER_REGION_ADDRESS 0x24001000u /* placed by WD_LOCATION (override in wd_board.h) */
#endif
#ifndef WD_REGISTER_ALIAS_ADDRESS
#define WD_REGISTER_ALIAS_ADDRESS 0x24002000u
#endif
#ifndef WD_REGISTER_USER_EVENT_NUMBER
#define WD_REGISTER_USER_EVENT_NUMBER 8
#endif
#ifndef WD_REGISTER_USER_STATUS_NUMBER
#define WD_REGISTER_USER_STATUS_NUMBER 4
#endif
#ifndef WD_GLOBAL_SHARED_BUF_SIZE
#define WD_GLOBAL_SHARED_BUF_SIZE 32768
#endif

extern uint32_t wd_reg_region[32];
extern uint32_t wd_reg_alias[32][32];

/***************************************************************************************************************************************************************
 **********  System Register
 **************************************************************************************************************************************************************/
typedef struct {
    /*--------------- [FUNC] Function ---------------*/
    uint32_t FUNC_Print_ADC_Data;
    uint32_t FUNC_Console;

    /*--------------- [R_EVT] Realtime Event ---------------*/
    uint32_t R_EVT_Test;

    /*--------------- [EVT] Event ---------------*/
    uint32_t EVT_RTC_Initialize;

    uint32_t EVT_BackUp;
    uint32_t EVT_P_Fail;
    uint32_t EVT_P_Restore;

    uint32_t EVT_Program_Load;
    uint32_t EVT_Setting;
    uint32_t EVT_Communication;
    uint32_t EVT_Display;

    uint32_t EVT_Environment_Save;
    uint32_t EVT_Firmware_BootLoader_Update;
    uint32_t EVT_Firmware_System_Update;

    uint32_t EVT_BootUp_Firmware_BootLoader_Update;
    uint32_t EVT_BootUp_Firmware_System_Update;

    uint32_t EVT_ADC_Calc;

    uint32_t EVT_Ethernet_Config_Update;

    /*--------------- [EVT] Application ready (set by application when network may start) ---------------*/
    uint32_t EVT_Application_Ready;

    /*--------------- [EVT] Application user events ---------------*/
    uint32_t EVT_User[WD_REGISTER_USER_EVENT_NUMBER];
} wd_reg_system_t;

/***************************************************************************************************************************************************************
 **********  Status Register (0:OK, 1:Error)
 **************************************************************************************************************************************************************/
typedef struct {
    uint32_t Status_Power;
    uint32_t Status_ADC;
    uint32_t Status_SDRAM;
    uint32_t Status_NVM_NOR_Flash;
    uint32_t Status_Temperature_Sensor;
    uint32_t Status_User[WD_REGISTER_USER_STATUS_NUMBER];
} wd_reg_status_t;

/***************************************************************************************************************************************************************
 **********  Timer Register
 **************************************************************************************************************************************************************/
typedef struct wd_register_timer_s {
    /*--------------- SysTick (1ms) ---------------*/
    uint32_t Tick;
    uint32_t Tick_50ms;
    uint32_t Tick_100ms;
    uint32_t Tick_250ms;
    uint32_t Tick_500ms;
    uint32_t Tick_1000ms;
    uint32_t Toggle_100ms;
    uint32_t Toggle_250ms;
    uint32_t Toggle_500ms;
    uint32_t Toggle_1000ms;

    /*--------------- Driver ---------------*/
    uint32_t Timer_Tick;

    /*--------------- RTC (1s) ---------------*/
    uint32_t RTC_Tick;
    uint32_t Sec;
    uint32_t Min;
    uint32_t Hour;
    uint32_t Date;
    uint32_t Mon;
    uint32_t Year;

    uint32_t Sec_3;
    uint32_t Sec_5;
    uint32_t Sec_10;

    uint32_t Reserved[11];
} wd_reg_timer_t;

/***************************************************************************************************************************************************************
 **********  Debug Register
 **************************************************************************************************************************************************************/
typedef struct {
    uint32_t Test_1 : 1;
    uint32_t Test_2 : 1;
    uint32_t Test_3 : 1;
    uint32_t Test_4 : 1;
    uint32_t Test_5 : 1;
    uint32_t Test_6 : 1;
    uint32_t Test_7 : 1;
    uint32_t Test_8 : 1;
    uint32_t Reserved : 24;
} wd_reg_debug_t;

extern wd_reg_system_t *const wd_reg_system;
extern wd_reg_status_t *const wd_reg_status;
extern wd_reg_timer_t *const wd_reg_timer;
extern wd_reg_timer_t *const wd_reg_r_timer_thread_source;
extern wd_reg_timer_t *const wd_reg_r_timer_thread;
extern wd_reg_debug_t *const wd_reg_debug;

/***************************************************************************************************************************************************************
 **********  Global shared buffers
 **************************************************************************************************************************************************************/
typedef struct {
    union global_shared_s {
#if (RTE_SERVICE_MMU)
        uint8_t MMU_Multi_Read_Buf[3][WD_MMU_MULTI_READ_BUFFER_SIZE];
#endif

#if (RTE_DRV_VALIDATION)
        struct api_validation_s {
            uint8_t W_buf[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE];
            uint8_t R_buf[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE * 3];
            uint8_t A_buf[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE * 3];
        } API_Validation;
#endif

        uint8_t Buf[WD_GLOBAL_SHARED_BUF_SIZE]; /* = wd_core_scratch_buffer() */
    } Shared;

    uint32_t Debug_Point;
} wd_reg_global_t;

extern wd_reg_global_t wd_global;

#if defined(__cplusplus)
}
#endif

#endif /* WD_SYSTEM_WD_REG_H */
