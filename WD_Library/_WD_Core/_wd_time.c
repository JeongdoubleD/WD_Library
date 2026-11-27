/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 *//**
 *  \version         V1.0.0 [MDK 5.37]
 *  \author
 ** @{
 ******************************************************************************/

#include "wd_core.h"
#include "_wd_time.h"

/***************************************************************************************************************************************************************
 **********  Time
 **************************************************************************************************************************************************************/
clock_t wd_time_clock_ms = 0;
clock_t wd_time_run_time_clock_ms = 0; //250702
wd_time_cycle_t wd_time_clock_cycle = 0;
uint32_t wd_time_debug_time;
uint32_t wd_time_debug_time_stamp;

int32_t wd_time_init_dwt(void) {
    /* Disable TRC */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
    /* Enable TRC */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

    /* Disable clock cycle counter */
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
    /* Enable  clock cycle counter */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; //0x00000001;

    /* Reset the clock cycle counter value */
    DWT->CYCCNT = 0;

    /* 3 NO OPERATION instructions */
    __ASM volatile("NOP");
    __ASM volatile("NOP");
    __ASM volatile("NOP");

    /* Check if clock cycle counter has started */
    if (DWT->CYCCNT) {
        return WD_CODE_OK; /*clock cycle counter started*/
    } else {
        return WD_CODE_ERROR; /*clock cycle counter not started*/
    }
}

wd_time_cycle_t wd_time_get_cycle(wd_time_cycle_t *cycle) {
    uint32_t load;
    wd_time_cycle_t value;

    load = SysTick->LOAD;
    load -= SysTick->VAL;

    value = wd_time_clock_cycle + (wd_time_cycle_t)load;

    if (cycle != NULL) {
        value = (value - (*cycle));
    }

    return (value);
}

void wd_time_wait_us(uint32_t us) {
    clock_t clk_cycle_start = DWT->CYCCNT;
    //  __ASSERT(us <= (0xFFFFFFFF / (SystemCoreClock / 1000000)));
    us *= (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - clk_cycle_start) < us)
        ;
}

void wd_time_wait_ms(uint32_t ms) {
#if (RTE_RTOS)
    if ((wd_core_system_mode > WD_SYSTEM_MODE_NORMAL) && (ms > 100)) {
        osDelay(ms);
    } else
#endif
    {
        int i;
        clock_t clk_cycle_start;
        int sec = (int)(ms / 1000);
        ms %= 1000;

        if (sec) {
            for (i = 0; i < sec; i++) {
                clk_cycle_start = DWT->CYCCNT;
                while ((DWT->CYCCNT - clk_cycle_start) < SystemCoreClock)
                    ;
            }
        }

        clk_cycle_start = DWT->CYCCNT;
        ms *= (SystemCoreClock / 1000);
        while ((DWT->CYCCNT - clk_cycle_start) < ms)
            ;
    }
}

void wd_time_wait(float time, void *_this) {
    uint32_t upper_10ms;
    uint32_t lower_10ms;

    UNUSED(_this);

    upper_10ms = (uint32_t)(time * 100.0f) * 10;
    lower_10ms = (uint32_t)((fmodf(time, 0.01f) * 1000000.0f));

    if (upper_10ms) {
        do {
            if (upper_10ms > 1000) {
                wd_time_wait_ms(1000);
                upper_10ms -= 1000;
            } else {
                wd_time_wait_ms(upper_10ms);
                upper_10ms = 0;
            }
        } while (upper_10ms);
    }

    if (lower_10ms) {
        wd_time_wait_us(lower_10ms);
    }
}

clock_t wd_time_get_us(void) {
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }

    return (DWT->CYCCNT / (SystemCoreClock / 1000000));
}

uint32_t wd_time_calc_us(uint32_t *us) {
    uint32_t end = wd_time_get_us();
    uint32_t start = 0;
    if (us != 0) {
        start = *us;
    }
    if ((end - start) > 0x80000000) {
        return ((0xFFFFFFFF / (SystemCoreClock / 1000000)) - start + end);
    } else {
        return (end - start);
    }
}

clock_t wd_time_get_ms(void) // osKernelGetSysTimerCount
{
    return (wd_time_clock_ms);
}

clock_t wd_time_calc_ms(uint32_t *ms) {
    clock_t end = wd_time_get_ms();
    clock_t start = 0;

    if (ms != 0) {
        start = *ms;
    }

    return (end - start);
}

/***************************************************************************************************************************************************************
 **********  RTC
 **************************************************************************************************************************************************************/
struct tm wd_time_time_tm;
time_t wd_time_time;
bool wd_time_sub_second_busy;
bool wd_time_sub_second_calc;
bool wd_time_sub_second_flag;
uint32_t wd_time_sub_second_sec;
uint32_t wd_time_sub_second_clk;
uint32_t wd_time_sub_second_log;

void wd_time_time_to_tm(time_t time, struct tm *tm) {
#if 1
    struct tm *ptm;

    ptm = localtime(&time); // Keil gmtime() is not implemented and returns NULL
    if (ptm) {
        memcpy(tm, ptm, sizeof(struct tm));
    }
#else
    uint32_t jd, w, x, a, b, c, d, e, f, month;

    tm->tm_sec = (uint32_t)time % 60;
    tm->tm_min = ((uint32_t)time / 60) % 60;
    tm->tm_hour = ((uint32_t)time / 3600) % 24;
    jd = (uint32_t)(((uint32_t)time / 86400L) + 2451545L); /* jd = julian days */

    /* Calculate day of week: 0 = Sunday; January 1, 2000 was Saturday */ // 1 is Monday, range 0~6
    tm->tm_wday = (jd + 0) % 7;

    /* This obscure logic is the standard arithmetic for calculating a date
   * from a julian day, taken from a web site and tested. */

    /* In standard calculation w = int((jd - 1867216.25)/36524.25); */
    w = ((4L * jd) - 7468865L) / 146097L;
    x = w / 4L;
    a = (jd + w + 1L) - x;
    b = a + 1524L;

    /* In standard calculation c = int((b - 122.1) / 365.25); */
    c = ((20L * b) - 2442L) / 7305L;
    /* In standard calculation d = int(c * 365.25); */
    d = (c * 1461L) / 4;
    /* In standard calculation e = int((b - d)/30.6001); */
    e = ((b - d) * 10000L) / 306001L; /* b - d is a few hundred, so no overflow */
    f = (306001L * e) / 10000L;       /* e is less than 25, so no overflow */

    tm->tm_mday = (b - d) - f;
    tm->tm_mon = month = (e < 14) ? (e - 1) : (e - 13);
    tm->tm_year = (month < 3) ? (c - 4715) : (c - 4716);
    tm->tm_year = 100 + (tm->tm_year % 100);
#endif
}

void wd_time_tm_to_time(struct tm *tm, time_t *time) {
#if 1 // 1970/1/1 0:0:0 기준, 2038까지 사용 가능
    *time = mktime(tm);
#else // 2000/1/1 0:0:0 기준, 2068까지 사용 가능
    int32_t a, y, m, j;

    a = (14 - tm->tm_mon) / 12L;
    y = (int32_t)((uint32_t)((2000 + tm->tm_year) + 4800L) - a);
    m = (int32_t)(((uint32_t)tm->tm_mon + (12 * a)) - 3);

    /* julian days since Jan 1, 2000; 5.8e6 years range in 32-bit signed int */
    j = (int32_t)((uint32_t)tm->tm_mday + (((153 * m) + 2) / 5) + (365 * y) + (y / 4) - (y / 100) +
                  (y / 400) - 2483590);

    /* calculate julian seconds since Jan 1, 2000; valid until j wraps
   * around in 2068 */
    j = (int32_t)((j * 86400) + ((((uint32_t)tm->tm_hour) - 12L) * 3600) +
                  ((uint32_t)tm->tm_min * 60L) + (uint32_t)tm->tm_sec + 43200L);

    *time = (time_t)j;
#endif
}

void wd_time_config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

#if 0
  {
    uint32_t  bdcr = RCC->BDCR;
    uint32_t  cr = RTC->CR;
    uint32_t  isr = RTC->ISR;
    uint32_t  prer = RTC->PRER;
    printf("BDCR LSEON    [%d] [1]\r\n", (bdcr >> 0) & 0x01);
    printf("BDCR LSERDY   [%d] [1]\r\n", (bdcr >> 1) & 0x01);
    printf("BDCR LSEBYP   [%d] [1]\r\n", (bdcr >> 2) & 0x01);
    printf("BDCR LSEDRV   [%d] [0]\r\n", (bdcr >> 3) & 0x03);
    printf("BDCR LSECSSON [%d] [0]\r\n", (bdcr >> 5) & 0x01);
    printf("BDCR LSECSSD  [%d] [0]\r\n", (bdcr >> 6) & 0x01);
    printf("BDCR RTCSEL   [%d] [1]\r\n", (bdcr >> 8) & 0x03);
    printf("BDCR RTCEN    [%d] [1]\r\n", (bdcr >> 15) & 0x01);
    printf("BDCR BDRST    [%d] [0]\r\n", (bdcr >> 16) & 0x01);
    printf("BDCR LSCOEN   [%d] [0]\r\n", (bdcr >> 24) & 0x01);
    printf("BDCR LSCOSEL  [%d] [0]\r\n", (bdcr >> 25) & 0x01);
    printf("CR   [%08X]\r\n", cr);
    printf("ISR  [%08X]\r\n", isr);
    printf("PRER [%08X]\r\n", prer);
  }
#endif

    (RTE_PLATFORM_TIME_RTC_HANDLE)->Instance = RTC;
    (RTE_PLATFORM_TIME_RTC_HANDLE)->State = HAL_RTC_STATE_RESET;
    (RTE_PLATFORM_TIME_RTC_HANDLE)->Init.HourFormat = RTC_HOURFORMAT_24;
    (RTE_PLATFORM_TIME_RTC_HANDLE)->Init.AsynchPrediv = WD_RTC_ASYNCH_PREDIV;
    (RTE_PLATFORM_TIME_RTC_HANDLE)->Init.SynchPrediv = WD_RTC_SYNCH_PREDIV;
    (RTE_PLATFORM_TIME_RTC_HANDLE)->Init.OutPut = RTC_OUTPUT_DISABLE;
    (RTE_PLATFORM_TIME_RTC_HANDLE)->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    (RTE_PLATFORM_TIME_RTC_HANDLE)->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
#if defined(RTC_OUTPUT_REMAP_NONE)
    (RTE_PLATFORM_TIME_RTC_HANDLE)->Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
#endif /* defined (RTC_OUTPUT_REMAP_NONE) */
#if defined(RTC_OUTPUT_PULLUP_NONE)
    (RTE_PLATFORM_TIME_RTC_HANDLE)->Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
#endif /* defined (RTC_OUTPUT_PULLUP_NONE) */

    // Enable access to Backup domain
    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_RTC_ENABLE();

#if defined __HAL_RCC_RTCAPB_CLK_ENABLE /* part of STM32L4 / STM32L5 */
    __HAL_RCC_RTCAPB_CLK_ENABLE();
#endif /* __HAL_RCC_RTCAPB_CLK_ENABLE */

    if ((HAL_RTCEx_BKUPRead(RTE_PLATFORM_TIME_RTC_HANDLE, RTC_BKP_DR0) == 0x12345678) &&
        (HAL_RTCEx_BKUPRead(RTE_PLATFORM_TIME_RTC_HANDLE, RTC_BKP_DR1) == 0x90ABCDEF)) {
        return;
    }

    wd_time_rtc_init_hook();
    printf(WD_COLOR_WHITE_BOLD WD_COLOR_BG_YELLOW_BOLD "RTC Init [%lX]", RTC->BKP0R);
    printf(WD_COLOR_WHITE_BOLD WD_COLOR_BG_YELLOW_BOLD " [%lX]" WD_COLOR_RESET "\r\n", RTC->BKP1R);

    /*--------------- Select the RTC clock source ---------------*/
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
#if defined(RTE_CONFIG_HARDWARE_VERSION) && (RTE_CONFIG_HARDWARE_VERSION == 1)
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
#else
    RCC_OscInitStruct.LSEState = RCC_LSE_ON; //RCC_LSE_BYPASS;
#endif
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        WD_LOG_ERROR("Cannot initialize RTC with LSE");
    }

    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        WD_LOG_ERROR("PeriphClkInitStruct RTC failed with LSE");
    }

    /*--------------- Initialize the RTC peripheral ---------------*/
    if (HAL_RTC_Init(RTE_PLATFORM_TIME_RTC_HANDLE) != HAL_OK) {
        WD_LOG_ERROR("RTC initialization failed");
    }

    HAL_RTCEx_BKUPWrite(RTE_PLATFORM_TIME_RTC_HANDLE, RTC_BKP_DR0, 0x12345678);
    HAL_RTCEx_BKUPWrite(RTE_PLATFORM_TIME_RTC_HANDLE, RTC_BKP_DR1, 0x90ABCDEF);
    HAL_RTCEx_DeactivateTimeStamp(RTE_PLATFORM_TIME_RTC_HANDLE);
}

/***************************************************************************/ /**
 * \param     [in]  datetime  NULL이면 무시
 * \return          true = RTC 업데이트 성공(RTC_Time을 사용하면 RTC 인터럽트에서 1초 플래그 역활, subsecond 변경은 무시함)
 ******************************************************************************/
int32_t wd_time_read(time_t *time) {
    time_t rtc_time;
    struct tm date_time = {
        0,
    };

    RTC_TR time_reg = *(RTC_TR *)(RTC_BASE + WD_RTC_TR_OFFSET);
    RTC_DR date_reg = *(RTC_DR *)(RTC_BASE + WD_RTC_DR_OFFSET);

    date_time.tm_sec = time_reg.ST * 10 + time_reg.SU;
    date_time.tm_min = time_reg.MNT * 10 + time_reg.MNU;
    date_time.tm_hour = time_reg.HT * 10 + time_reg.HU;
    date_time.tm_mday = date_reg.DT * 10 + date_reg.DU;
    date_time.tm_mon = (date_reg.MT * 10 + date_reg.MU) - 1;
    date_time.tm_wday = date_reg.WDU % 7;
    date_time.tm_year = 100 + date_reg.YT * 10 + date_reg.YU;

    wd_time_tm_to_time(&date_time, &rtc_time);

    if (rtc_time == 0xFFFFFFFF) {
        /* no log here : the logger calls this function for its time stamp (GEN-M-019, a failing
         * RTC date made WD_LOG_ERROR -> printd -> pre_control -> wd_time_read recurse) */
        return (WD_CODE_NOP);
    }

    if (*time != rtc_time) {
        *time = rtc_time;

        if (time == &wd_time_time) {
            wd_time_time_tm = date_time;
        }
        return (WD_CODE_PROCESSED);
    }
    return (WD_CODE_NOP);
}


/**************************************************************************/ /**
 * \return          milisecond
 ******************************************************************************/
void wd_time_read_sub_handler(void) {
    if (!wd_time_sub_second_busy) {
        if (wd_time_sub_second_flag) {
            if ((LL_RTC_TIME_GetSubSecond(RTC) != 0) && !wd_time_sub_second_calc) {
                wd_time_sub_second_flag = false;
            }
        } else {
            if (LL_RTC_TIME_GetSubSecond(RTC) == 0) {
                wd_time_sub_second_calc = true;
            }
        }

        if (wd_time_sub_second_calc) {
            wd_time_rtc_second_hook();
            wd_time_sub_second_flag = true;
            wd_time_sub_second_clk = DWT->CYCCNT;
            wd_time_sub_second_calc = false;
        }

        (void)((RTE_PLATFORM_TIME_RTC_HANDLE)->Instance->DR);
    }
}

time_t wd_time_read_sub_second(void) {
#if 1
    uint32_t time;

    time = DWT->CYCCNT - wd_time_sub_second_clk;
    return (time / (SystemCoreClock / 1000000));
#else
    uint32_t prer, ssr;

    prer = (uint32_t)(RTC->PRER & RTC_PRER_PREDIV_S);
    ssr = (uint32_t)(READ_BIT(RTC->SSR, RTC_SSR_SS));

    printf("[%d] [%d]", ssr, (DWT->CYCCNT - wd_time_sub_second_clk) / (SystemCoreClock / 1000000));

    return ((prer - ssr) * 1000000 / (prer + 1));
#endif
}


/***************************************************************************/ /**
 * \param     [in]  time      현재 시간(초)
 ******************************************************************************/
void wd_time_write(time_t time) {
    time_t rtc_time;
    struct tm date_time = {
        0,
    };
    uint32_t __time_start;

    if (!(time != 0)) {
        return;
    }

    wd_time_time_to_tm(wd_time_time, &date_time);

    {
        RTC_DateTypeDef sdatestructure;
        RTC_TimeTypeDef stimestructure;

        /*##-2- Configure the Date #################################################*/
        sdatestructure.Year =
            (uint8_t)((((date_time.tm_year - 100) / 10) << 4) + (date_time.tm_year - 100) % 10);
        sdatestructure.Month =
            (uint8_t)(((date_time.tm_mon / 10) << 4) + date_time.tm_mon % 10 + 1);
        sdatestructure.Date = (uint8_t)(((date_time.tm_mday / 10) << 4) + date_time.tm_mday % 10);
        sdatestructure.WeekDay = (uint8_t)((date_time.tm_wday == 0) ? date_time.tm_wday : 7);

        if (HAL_RTC_SetDate(RTE_PLATFORM_TIME_RTC_HANDLE, &sdatestructure, RTC_FORMAT_BCD) !=
            HAL_OK) {
            /* Initialization Error */
        }

        /*##-3- Configure the Time #################################################*/
        stimestructure.Hours =
            (uint8_t)(((date_time.tm_hour / 10) << 4) + (date_time.tm_hour % 10));
        stimestructure.Minutes =
            (uint8_t)(((date_time.tm_min / 10) << 4) + (date_time.tm_min % 10));
        stimestructure.Seconds =
            (uint8_t)(((date_time.tm_sec / 10) << 4) + (date_time.tm_sec % 10));
        stimestructure.SubSeconds = 0x00;
        stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
        stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

        if (HAL_RTC_SetTime(RTE_PLATFORM_TIME_RTC_HANDLE, &stimestructure, RTC_FORMAT_BCD) !=
            HAL_OK) {
            /* Initialization Error */
        }

        __time_start = wd_time_get_us();

        do {
            RTC_TR time_reg = *(RTC_TR *)(RTC_BASE + WD_RTC_TR_OFFSET);
            RTC_DR date_reg = *(RTC_DR *)(RTC_BASE + WD_RTC_DR_OFFSET);

            /* RTC Time Register */
            date_time.tm_sec = time_reg.ST * 10 + time_reg.SU;
            date_time.tm_min = time_reg.MNT * 10 + time_reg.MNU;
            date_time.tm_hour = time_reg.HT * 10 + time_reg.HU;

            /* RTC Data Register */
            date_time.tm_mday = date_reg.DT * 10 + date_reg.DU;
            date_time.tm_mon = (date_reg.MT * 10 + date_reg.MU) - 1;
            date_time.tm_wday = date_reg.WDU % 7;
            date_time.tm_year = 100 + date_reg.YT * 10 + date_reg.YU;

            wd_time_tm_to_time(&date_time, &rtc_time);

            if (wd_time_calc_us(&__time_start) > 1500000) {
                WD_LOG_ERROR("RTC Write Timeout !!!");
                break;
            }
        } while (!WD_RANGE(time, rtc_time, time + 2));
    }

    wd_time_time_tm = date_time;
    wd_time_tm_to_time(&wd_time_time_tm, &wd_time_time);
}

/*
  Member    Type  Meaning                    Range
  tm_sec    int   seconds after the minute   0-61
  tm_min    int   minutes after the hour     0-59
  tm_hour   int   hours since midnight       0-23
  tm_mday   int   day of the month           1-31
  tm_mon    int   months since January       0-11
  tm_year   int   years since 1900
  tm_wday   int   days since Sunday          0-6
  tm_yday   int   days since January 1       0-365
  tm_isdst  int   Daylight Saving Time flag

*/

/***************************************************************************************************************************************************************
 **********  Timer
 **************************************************************************************************************************************************************/
static int32_t Timer_Driver_Initialize(uint32_t ticks) {
    RCC_ClkInitTypeDef clkconfig;
    uint32_t uwTimclock, uwAPB1Prescaler;
    uint32_t pFLatency;

    /* Get clock configuration */
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

    /* Get APB1 prescaler */
    uwAPB1Prescaler = clkconfig.APB1CLKDivider;

    /* Compute TIM6 clock */
    if (uwAPB1Prescaler == RCC_HCLK_DIV1) {
        uwTimclock = HAL_RCC_GetPCLK1Freq();
    } else {
        uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
    }

    RTE_PLATFORM_TIME_HANDLE.Instance = RTE_PLATFORM_TIME_REGISTER;
    RTE_PLATFORM_TIME_HANDLE.Init.Prescaler = (uwTimclock / ticks) - 1;
    RTE_PLATFORM_TIME_HANDLE.Init.CounterMode = TIM_COUNTERMODE_UP;
    RTE_PLATFORM_TIME_HANDLE.Init.Period = 4294967295; // 0xFFFFFFFF
    RTE_PLATFORM_TIME_HANDLE.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    RTE_PLATFORM_TIME_HANDLE.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&RTE_PLATFORM_TIME_HANDLE) != HAL_OK) {
        WD_LOG_ERROR();
    }

    if (HAL_TIM_OC_Init(&RTE_PLATFORM_TIME_HANDLE) != HAL_OK) {
        WD_LOG_ERROR();
    }

    NVIC_SetPriority(RTE_PLATFORM_TIME_IRQ,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                         WD_INTERRUPT_PREEMPT_PRIORITY_ABOVE_NORMAL_HANDLER,
                                         WD_INTERRUPT_SUB_PRIORITY_ABOVE_NORMAL_HANDLER));
    NVIC_EnableIRQ(RTE_PLATFORM_TIME_IRQ);

    __HAL_TIM_DISABLE_IT(&RTE_PLATFORM_TIME_HANDLE, TIM_IT_UPDATE);
    __HAL_TIM_DISABLE_IT(&RTE_PLATFORM_TIME_HANDLE, TIM_IT_CC1);

    HAL_TIM_Base_Start(&RTE_PLATFORM_TIME_HANDLE);
    HAL_TIM_OC_Start(&RTE_PLATFORM_TIME_HANDLE, TIM_CHANNEL_1);

    wd_drv_timer->Res->Ticks = ticks;
    wd_drv_timer->Res->Num_Instance = 0;
    wd_drv_timer->Res->Set_Time_us = 0;
    wd_drv_timer->Res->Elapsed_Time_us = 0;
    wd_drv_timer->Res->Time_Stamp = 0;
    memset((void *)wd_drv_timer->Res->Instance, 0, sizeof(wd_drv_timer->Res->Instance));

    return (WD_CODE_OK);
}


static int32_t Timer_Driver_Attach_Just(wd_time_timer_type_t type, wd_time_timer_method_t method,
                                        wd_time_timer_mode_t mode, wd_time_t set_time_us,
                                        void *ops) {
    wd_time_timer_instance_t instance = {
        0,
    };

    instance.Type = type;
    instance.Status = WDIF_TIMER_STATUS_DISABLE;
    instance.Method = method;
    instance.Mode = mode;
    instance.Set_Time_us = set_time_us;
    instance.Remain_Time_us = set_time_us;
    instance.Ops.Flag = ops;

    return (wd_drv_timer->Attach(&instance));
}


static int32_t Timer_Driver_Attach(wd_time_timer_instance_t *instance) {
    uint32_t instance_idx = 0xFFFFFFFF;
    uint32_t set_time_us = 0;
    uint32_t elapse_time_us, remain_time_us, counter;
    int i;

    if (instance == NULL) {
        return (WD_CODE_ERROR_PARAMETER);
    }

    if ((instance->Type > WDIF_TIMER_TYPE_TIMEOUT) ||
        (instance->Status != WDIF_TIMER_STATUS_DISABLE) ||
        (instance->Method > WDIF_TIMER_METHOD_TOGGLE) ||
        (instance->Mode > WDIF_TIMER_MODE_REPEAT) || (instance->Ops.CB_Instance == NULL) ||
        (instance->Set_Time_us < RTE_PLATFORM_TIME_MINIMUM_TIME) ||
        (wd_drv_timer->Res->Num_Instance >= WD_ARRAY_SIZE(wd_drv_timer->Res->Instance))) {
        WD_LOG_ERROR();
        return (WD_CODE_ERROR_PARAMETER);
    }

    if (instance->Type == WDIF_TIMER_TYPE_TIMER) {
    } else if (instance->Type == WDIF_TIMER_TYPE_TICKER) {
    } else if (instance->Type == WDIF_TIMER_TYPE_TIMEOUT) {
    } else if (instance->Type == WDIF_TIMER_TYPE_NONE) {
        WD_LOG_ERROR();
        return (WD_CODE_ERROR_PARAMETER);
    }

    if (!WD_RANGE(&wd_drv_timer->Res->Instance[0], instance,
                  &wd_drv_timer->Res->Instance[WD_ARRAY_SIZE(wd_drv_timer->Res->Instance) - 1U])) {
        for (i = 0; i < (int)WD_ARRAY_SIZE(wd_drv_timer->Res->Instance); i++) {
            if (wd_drv_timer->Res->Instance[i].Status == WDIF_TIMER_STATUS_DISABLE) {
                instance_idx = (uint32_t)i;
                break;
            }
        }

        if (instance_idx >= WD_ARRAY_SIZE(wd_drv_timer->Res->Instance)) {
            return (WD_CODE_ERROR_PARAMETER);
        }

        memcpy(&wd_drv_timer->Res->Instance[instance_idx], instance,
               sizeof(wd_time_timer_instance_t));

        instance = &wd_drv_timer->Res->Instance[instance_idx];
    }

    instance->Status = WDIF_TIMER_STATUS_ENABLE;

    counter = wd_drv_timer->Get_Counter();
    elapse_time_us = counter - wd_drv_timer->Res->Time_Stamp;

    if ((wd_drv_timer->Res->Num_Instance == 0) ||
        (__HAL_TIM_GET_IT_SOURCE(&RTE_PLATFORM_TIME_HANDLE, TIM_IT_CC1) == RESET)) {
        wd_drv_timer->Res->Time_Stamp = counter;
        wd_drv_timer->Res->Elapsed_Time_us = 0;

        set_time_us = instance->Set_Time_us;
    } else {
        if (elapse_time_us < wd_drv_timer->Res->Set_Time_us) {
            remain_time_us = wd_drv_timer->Res->Set_Time_us - elapse_time_us;
        } else {
            remain_time_us = 0;
        }

        if (remain_time_us > instance->Set_Time_us) {
            wd_drv_timer->Res->Time_Stamp = counter;
            wd_drv_timer->Res->Elapsed_Time_us += elapse_time_us;
            instance->Remain_Time_us += wd_drv_timer->Res->Elapsed_Time_us;
            set_time_us = wd_drv_timer->Res->Set_Time_us;
        } else {
            instance->Remain_Time_us += elapse_time_us;
        }
    }

    wd_drv_timer->Res->Num_Instance++;

    if (set_time_us) {
        wd_drv_timer->Res->Set_Time_us = set_time_us;

        if (set_time_us > RTE_PLATFORM_TIME_LATENCY) {
            set_time_us -= RTE_PLATFORM_TIME_LATENCY;
        }

        __HAL_TIM_ENABLE_IT(&RTE_PLATFORM_TIME_HANDLE, TIM_IT_CC1);

        RTE_PLATFORM_TIME_HANDLE.Instance->CCR1 = wd_drv_timer->Get_Counter() + set_time_us;
    }


    return (WD_CODE_OK);
}


static int32_t Timer_Driver_Detach(wd_time_timer_instance_t *instance) {
    if (instance == NULL) {
        return (WD_CODE_ERROR_PARAMETER);
    }

    memset(instance, 0, sizeof(wd_time_timer_instance_t));

    return (WD_CODE_OK);
}


static float Timer_Driver_Get_Interval(void) {
    return (1.0f / (float)wd_drv_timer->Res->Ticks);
}


static uint32_t Timer_Driver_Get_Counter(void) {
    return (__HAL_TIM_GET_COUNTER(&RTE_PLATFORM_TIME_HANDLE));
}


static void Timer_Driver_Get_Instance_Info(void) {
}


void wd_time_timer_driver_handler(void) {
    uint32_t set_time_us = 0xFFFFFFFF;
    uint32_t instance_number = wd_drv_timer->Res->Num_Instance;
    uint32_t instance_count = 0;
    uint32_t elapsed_time_us = wd_drv_timer->Res->Set_Time_us + wd_drv_timer->Res->Elapsed_Time_us;
    int i;

    wd_drv_timer->Res->Set_Time_us = 0;
    wd_drv_timer->Res->Elapsed_Time_us = 0;

    if (wd_drv_timer->Res->Num_Instance) {
        for (i = 0; i < (int)WD_ARRAY_SIZE(wd_drv_timer->Res->Instance); i++) {
            if (wd_drv_timer->Res->Instance[i].Status != WDIF_TIMER_STATUS_DISABLE) {
                if (wd_drv_timer->Res->Instance[i].Remain_Time_us <= elapsed_time_us) {
                    if (wd_drv_timer->Res->Instance[i].Ops.Flag != NULL) {
                        if (wd_drv_timer->Res->Instance[i].Type == WDIF_TIMER_TYPE_TICKER) {
                            if (wd_drv_timer->Res->Instance[i].Method == WDIF_TIMER_METHOD_CLEAR) {
                                *(wd_drv_timer->Res->Instance[i].Ops.Flag) = 0;
                            } else if (wd_drv_timer->Res->Instance[i].Method ==
                                       WDIF_TIMER_METHOD_SET) {
                                *(wd_drv_timer->Res->Instance[i].Ops.Flag) = 1;
                            } else if (wd_drv_timer->Res->Instance[i].Method ==
                                       WDIF_TIMER_METHOD_TOGGLE) {
                                *(wd_drv_timer->Res->Instance[i].Ops.Flag) ^= 1;
                            }
                        } else {
                            wd_drv_timer->Res->Instance[i].Ops.CB_Instance();
                        }
                    }

                    if (wd_drv_timer->Res->Instance[i].Mode == WDIF_TIMER_MODE_REPEAT) {
                        wd_drv_timer->Res->Instance[i].Remain_Time_us =
                            wd_drv_timer->Res->Instance[i].Set_Time_us;
                    } else {
                        memset(&wd_drv_timer->Res->Instance[i], 0,
                               sizeof(wd_time_timer_instance_t));
                        wd_drv_timer->Res->Num_Instance--;
                    }
                } else {
                    wd_drv_timer->Res->Instance[i].Remain_Time_us -= elapsed_time_us;
                }

                if (wd_drv_timer->Res->Instance[i].Remain_Time_us) {
                    if (set_time_us > wd_drv_timer->Res->Instance[i].Remain_Time_us) {
                        set_time_us = wd_drv_timer->Res->Instance[i].Remain_Time_us;
                    }
                }

                instance_count++;

                if (instance_count == instance_number) {
                    break;
                }
            }
        }
    } else {
        WD_LOG_ERROR();
    }

    if (wd_drv_timer->Res->Num_Instance == 0) {
        __HAL_TIM_DISABLE_IT(&RTE_PLATFORM_TIME_HANDLE, TIM_IT_CC1);
    } else if (set_time_us != 0xFFFFFFFF) {
        wd_drv_timer->Res->Set_Time_us = set_time_us;

        if (set_time_us > RTE_PLATFORM_TIME_LATENCY) {
            set_time_us -= RTE_PLATFORM_TIME_LATENCY;
        }

        wd_drv_timer->Res->Time_Stamp = wd_drv_timer->Get_Counter();
        RTE_PLATFORM_TIME_HANDLE.Instance->CCR1 = wd_drv_timer->Get_Counter() + set_time_us;
    }
}


static wd_time_timer_driver_resource_t Timer_Driver_Res = {
    0,
};

wd_time_timer_driver_t wd_time_timer_driver = {Timer_Driver_Initialize,
                                               Timer_Driver_Attach_Just,
                                               Timer_Driver_Attach,
                                               Timer_Driver_Detach,
                                               Timer_Driver_Get_Interval,
                                               Timer_Driver_Get_Counter,
                                               Timer_Driver_Get_Instance_Info,
                                               &Timer_Driver_Res};

wd_time_timer_driver_t *const wd_drv_timer = &wd_time_timer_driver;

/** @} end of file */
