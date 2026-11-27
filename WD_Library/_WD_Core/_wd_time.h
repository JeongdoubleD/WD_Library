/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 *//**
 *  \version         V1.0.0 [MDK 5.37]
 *  \author
 ** @{
 ******************************************************************************/

#ifndef WD_CORE_WD_TIME_H
#define WD_CORE_WD_TIME_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "wd_core.h"

/* board HAL mapping : default CubeMX names, override in wd_board.h */
#ifndef RTE_PLATFORM_TIME_RTC_HANDLE
#define RTE_PLATFORM_TIME_RTC_HANDLE (&hrtc)
#endif


/***************************************************************************************************************************************************************
 **********  Time
 **************************************************************************************************************************************************************/
typedef uint64_t wd_time_cycle_t;
extern clock_t wd_time_clock_ms;
extern clock_t wd_time_run_time_clock_ms; //250702
extern wd_time_cycle_t wd_time_clock_cycle;
extern uint32_t wd_time_debug_time;
extern uint32_t wd_time_debug_time_stamp;

int32_t wd_time_init_dwt(void);
wd_time_cycle_t wd_time_get_cycle(wd_time_cycle_t *cycle);
void wd_time_wait_us(clock_t us);
void wd_time_wait_ms(clock_t ms);
void wd_time_wait(float time, void *_this);
clock_t wd_time_get_us(void);
clock_t wd_time_calc_us(clock_t *us);
clock_t wd_time_get_ms(void);
clock_t wd_time_calc_ms(clock_t *ms);

/***************************************************************************************************************************************************************
 **********  RTC
 **************************************************************************************************************************************************************/
#define WD_RTC_CLOCK LSE_VALUE

#define WD_RTC_ASYNCH_PREDIV 127
#define WD_RTC_SYNCH_PREDIV  (WD_RTC_CLOCK / (WD_RTC_ASYNCH_PREDIV + 1) - 1)

typedef struct {
    uint32_t SU : 4;    // Second units in BCD format [0:3]
    uint32_t ST : 3;    // Second tens in BCD format [4:6]
    uint32_t : 1;       // [7]
    uint32_t MNU : 4;   // Minute units in BCD format [8:11]
    uint32_t MNT : 3;   // Minute tens in BCD format [12:14]
    uint32_t Res1 : 1;  // [15]
    uint32_t HU : 4;    // Hour units in BCD format [16:19]
    uint32_t HT : 2;    // Hour tens in BCD format [20:21]
    uint32_t AM_PM : 1; // AM/PM notation (0: AM or 24-hour format, 1: PM) [22]
    uint32_t : 9;       // [23:31]
} RTC_TR;

typedef struct {
    uint32_t DU : 4;  // Date units in BCD format [0:3]
    uint32_t DT : 2;  // Date tens in BCD format [4:5]
    uint32_t : 2;     // [6:7]
    uint32_t MU : 4;  // Month units in BCD format [8:11]
    uint32_t MT : 1;  // Month tens in BCD format [12]
    uint32_t WDU : 3; // Week day units ( 000: forbidden, 001: Monday ~ 111: Sunday) [13:15]
    uint32_t YU : 4;  // Year units in BCD format [16:19]
    uint32_t YT : 4;  // Year tens in BCD format [20:23]
    uint32_t : 8;     // [34:31]
} RTC_DR;

#define WD_RTC_TR_OFFSET 0x00
#define WD_RTC_DR_OFFSET 0x04

#define WD_UNIX_TIME_OFFSET 946684800u // 2000/1/1 00:00:00 - 1970/1/1 00:00:00
//#define WD_UTC_DEVIATION_MIN_KOREA                                                 540  // +9 hours, 540 min
#define WD_UTC_DEVIATION_MIN_KOREA 0xFDE4 // -9 hours, -540 min
//#define WD_UTC_DEVIATION_MIN_KOREA                                                 0x8000


extern struct tm wd_time_time_tm;
extern time_t wd_time_time;
extern bool wd_time_sub_second_busy;
extern bool wd_time_sub_second_calc;
extern bool wd_time_sub_second_flag;
extern uint32_t wd_time_sub_second_sec;
extern uint32_t wd_time_sub_second_clk;
extern uint32_t wd_time_sub_second_log;

void wd_time_time_to_tm(time_t time, struct tm *tm);
void wd_time_tm_to_time(struct tm *tm, time_t *time);
void wd_time_config(void);
int32_t wd_time_read(time_t *time);
void wd_time_read_sub_handler(void);
time_t wd_time_read_sub_second(void);
void wd_time_write(time_t time);

/***************************************************************************************************************************************************************
 **********  Timer
 **************************************************************************************************************************************************************/
typedef uint32_t wd_time_t;

typedef enum {
    WDIF_TIMER_TYPE_NONE,
    WDIF_TIMER_TYPE_TIMER,
    WDIF_TIMER_TYPE_TICKER,
    WDIF_TIMER_TYPE_TIMEOUT,
    WDIF_TIMER_TYPE_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_time_timer_type_t;

typedef enum {
    WDIF_TIMER_STATUS_DISABLE,
    WDIF_TIMER_STATUS_ENABLE,
    WDIF_TIMER_STATUS_START,
    WDIF_TIMER_STATUS_EXECUTE,
    WDIF_TIMER_STATUS_STOP,
    WDIF_TIMER_STATUS_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_time_timer_status_t;

typedef enum {
    WDIF_TIMER_METHOD_NONE,
    WDIF_TIMER_METHOD_CLEAR,
    WDIF_TIMER_METHOD_SET,
    WDIF_TIMER_METHOD_TOGGLE,
    WDIF_TIMER_METHOD_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_time_timer_method_t;

typedef enum {
    WDIF_TIMER_MODE_SIMPLE,
    WDIF_TIMER_MODE_REPEAT,
    WDIF_TIMER_MODE_ENUM_UINT32_MAX = 0x7FFFFFFF // 직접 정의
} wd_time_timer_mode_t;

typedef struct {
    wd_time_timer_type_t Type;
    wd_time_timer_status_t Status;
    wd_time_timer_method_t Method;
    wd_time_timer_mode_t Mode;
    wd_time_t Set_Time_us;
    wd_time_t Remain_Time_us;
    union WDIF_TIMER_INSTANCE_OPS_T {
        wd_callback_t CB_Instance;
        uint32_t *Flag;
    } Ops;
} wd_time_timer_instance_t;

typedef struct {
    uint32_t Ticks;
    uint32_t Num_Instance;
    wd_time_t Set_Time_us;
    wd_time_t Elapsed_Time_us;
    wd_time_t Time_Stamp;
    wd_time_timer_instance_t Instance[RTE_PLATFORM_TIME_INSTANCE_SIZE];
} wd_time_timer_driver_resource_t;

typedef struct {
    int32_t (*Initialize)(uint32_t ticks);
    int32_t (*Attach_Just)(wd_time_timer_type_t type, wd_time_timer_method_t method,
                           wd_time_timer_mode_t mode, wd_time_t set_time_us, void *ops);
    int32_t (*Attach)(wd_time_timer_instance_t *instance);
    int32_t (*Detach)(wd_time_timer_instance_t *instance);
    float (*Get_Interval)(void);
    uint32_t (*Get_Counter)(void);
    void (*Get_Instance_Info)(void);
    wd_time_timer_driver_resource_t *Res;
} const wd_time_timer_driver_t;

extern wd_time_timer_driver_t wd_time_timer_driver;
extern wd_time_timer_driver_t *const wd_drv_timer;

void wd_time_timer_driver_handler(void);

#if defined(__cplusplus)
}
#endif /*--------------- __cplusplus ---------------*/
#endif /*--------------- WD_CORE_WD_TIME_H ---------------*/
/** @} end of file */
