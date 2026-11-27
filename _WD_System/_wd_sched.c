/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _Scheduler.c
 *
 *  Created on: Jan 7, 2025
 *      Author: doubled
 */


#include "wd.h"

#if 1 // 토글

static int Timer_Cnt = 0;

void wd_sched_tick(void) {
#if 1 /// Software Timer ***************************************************{
    if (wd_reg_timer->Tick) {
        wd_reg_timer->Tick = false;

        Timer_Cnt += 50;

#if 0
    if(wd_time_read(&wd_time_time) == WD_CODE_PROCESSED)
    {
#ifndef NDEBUG
      RTC_DR date_reg = *(RTC_DR*)(RTC_BASE + WD_RTC_DR_OFFSET);
      uint16_t year;

      year = 2000 + date_reg.YT * 10 + date_reg.YU -2 ;
      if(year >= 2068)
      {
        WD_LOG_ERROR("RTC Value too High [%u / 2068 years]", year);
      }
#endif

      wd_reg_timer->RTC_Tick = true;
    }
#endif

        if (wd_reg_timer->RTC_Tick) {
            wd_reg_timer->RTC_Tick = false;

            wd_time_read(&wd_time_time);

            wd_reg_timer->Sec = true;

            if (wd_time_time_tm.tm_sec == 0) {
                wd_reg_timer->Min = true;
                if (wd_time_time_tm.tm_min == 0) {
                    wd_reg_timer->Hour = true;
                    if (wd_time_time_tm.tm_hour == 0) {
                        wd_reg_timer->Date = true;
                        if (wd_time_time_tm.tm_mday == 1) {
                            wd_reg_timer->Mon = true;
                            if (wd_time_time_tm.tm_mon == 0) {
                                wd_reg_timer->Year = true;
                            }
                        }
                    }
                }
            }

            if ((wd_time_time_tm.tm_sec % 3) == 0) {
                wd_reg_timer->Sec_3 = true;
            }

            if ((wd_time_time_tm.tm_sec % 5) == 0) {
                wd_reg_timer->Sec_5 = true;
            }

            if ((wd_time_time_tm.tm_sec % 10) == 0) {
                wd_reg_timer->Sec_10 = true;
            }
        }

        wd_reg_timer->Tick_50ms = true;
        if ((Timer_Cnt % 100) == 0) {
            wd_reg_timer->Tick_100ms = true;
            wd_reg_timer->Toggle_100ms ^= 1;
        }
        if ((Timer_Cnt % 250) == 0) {
            wd_reg_timer->Tick_250ms = true;
            wd_reg_timer->Toggle_250ms ^= 1;
            if ((Timer_Cnt % 500) == 0) {
                wd_reg_timer->Tick_500ms = true;
                wd_reg_timer->Toggle_500ms ^= 1;
                if (Timer_Cnt >= 1000) {
                    wd_reg_timer->Tick_1000ms = true;
                    wd_reg_timer->Toggle_1000ms ^= 1;
                    Timer_Cnt = 0;
                }
            }
        }
    }
#endif /// Software Timer ***************************************************}

#if 1 /// Scheduler ********************************************************{
    {
        int i;
        wd_reg_timer_t *timer;

        for (i = 0; i < RTE_RTOS_THREAD_NUMBER; i++) {
            timer = (wd_reg_timer_t *)&wd_reg_alias[5 + i]; // 캐스팅 추가

            /*--------------- Software Timer ---------------*/
            if (wd_reg_timer->Tick_50ms) {
                timer->Tick_50ms = true;
            }

            if (wd_reg_timer->Tick_100ms) {
                timer->Tick_100ms = true;
            }

            if (wd_reg_timer->Tick_250ms) {
                timer->Tick_250ms = true;
            }

            if (wd_reg_timer->Tick_500ms) {
                timer->Tick_500ms = true;
            }

            if (wd_reg_timer->Tick_1000ms) {
                timer->Tick_1000ms = true;
            }

            timer->Toggle_100ms = wd_reg_timer->Toggle_100ms;
            timer->Toggle_250ms = wd_reg_timer->Toggle_250ms;
            timer->Toggle_500ms = wd_reg_timer->Toggle_500ms;
            timer->Toggle_1000ms = wd_reg_timer->Toggle_1000ms;

            /*--------------- RTC ---------------*/
            if (wd_reg_timer->Sec) {
                timer->Sec = true;
            }

            if (wd_reg_timer->Min) {
                timer->Min = true;
            }

            if (wd_reg_timer->Hour) {
                timer->Hour = true;
            }

            if (wd_reg_timer->Date) {
                timer->Date = true;
            }

            if (wd_reg_timer->Mon) {
                timer->Mon = true;
            }

            if (wd_reg_timer->Year) {
                timer->Year = true;
            }

            if (wd_reg_timer->Sec_3) {
                timer->Sec_3 = true;
            }

            if (wd_reg_timer->Sec_5) {
                timer->Sec_5 = true;
            }

            if (wd_reg_timer->Sec_10) {
                timer->Sec_10 = true;
            }
        }
    }
#endif /// Scheduler ********************************************************}
}

void wd_sched_tick_driver(wd_reg_timer_t *timer_src, wd_reg_timer_t *timer) {
#if 1 /// Software *********************************************************{
    if (timer_src->Tick_50ms) {
        timer_src->Tick_50ms = false;
        timer->Tick_50ms = true;
    }

    if (timer_src->Tick_100ms) {
        timer_src->Tick_100ms = false;
        timer->Tick_100ms = true;
    }

    if (timer_src->Tick_250ms) {
        timer_src->Tick_250ms = false;
        timer->Tick_250ms = true;
    }

    if (timer_src->Tick_500ms) {
        timer_src->Tick_500ms = false;
        timer->Tick_500ms = true;
    }

    if (timer_src->Tick_1000ms) {
        timer_src->Tick_1000ms = false;
        timer->Tick_1000ms = true;
    }

    timer->Toggle_100ms = timer_src->Toggle_100ms;
    timer->Toggle_250ms = timer_src->Toggle_250ms;
    timer->Toggle_500ms = timer_src->Toggle_500ms;
    timer->Toggle_1000ms = timer_src->Toggle_1000ms;

#endif /// Software *********************************************************}

#if 1 /// RTC **************************************************************{
    if (timer_src->Sec) {
        //    timer_src->Sec = false;
        timer->Sec = true;
    }

    if (timer_src->Min) {
        //    timer_src->Min = false;
        timer->Min = true;
    }

    if (timer_src->Hour) {
        //    timer_src->Hour = false;
        timer->Hour = true;
    }

    if (timer_src->Date) {
        //    timer_src->Date = false;
        timer->Date = true;
    }

    if (timer_src->Mon) {
        //    timer_src->Mon = false;
        timer->Mon = true;
    }

    if (timer_src->Year) {
        //    timer_src->Year = false;
        timer->Year = true;
    }

    if (timer_src->Sec_3) {
        //    timer_src->Sec_3 = false;
        timer->Sec_3 = true;
    }

    if (timer_src->Sec_5) {
        //    timer_src->Sec_5 = false;
        timer->Sec_5 = true;
    }

    if (timer_src->Sec_10) {
        //    timer_src->Sec_10 = false;
        timer->Sec_10 = true;
    }
#endif /// RTC **************************************************************}
}

void wd_sched_finalize(wd_reg_timer_t *timer) {
#if 1 /// Button ***********************************************************{
    {
    }
#endif /// Button ***********************************************************}

#if 1 /// Software Timer ***************************************************{
    {
        if (timer->Tick_50ms) {
            timer->Tick_50ms = false;
        }

        if (timer->Tick_100ms) {
            timer->Tick_100ms = false;
        }

        if (timer->Tick_250ms) {
            timer->Tick_250ms = false;
        }

        if (timer->Tick_500ms) {
            timer->Tick_500ms = false;
        }

        if (timer->Tick_1000ms) {
            timer->Tick_1000ms = false;
        }
    }
#endif /// Software Timer ***************************************************}

#if 1 /// RTC **************************************************************{
    {
        if (timer->Sec) {
            timer->Sec = false;
        }

        if (timer->Min) {
            timer->Min = false;
        }

        if (timer->Hour) {
            timer->Hour = false;
        }

        if (timer->Date) {
            timer->Date = false;
        }

        if (timer->Mon) {
            timer->Mon = false;
        }

        if (timer->Year) {
            timer->Year = false;
        }

        if (timer->Sec_3) {
            timer->Sec_3 = false;
        }

        if (timer->Sec_5) {
            timer->Sec_5 = false;
        }

        if (timer->Sec_10) {
            timer->Sec_10 = false;
        }
    }
#endif /// RTC **************************************************************}
}
#endif // #if 1 // 토글
/** @} end of file */
