/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _Thread.c   (TEMPLATE - project owned)
 * @brief   RTOS threads of the project : Main / Console / App / Ethernet / CAN
 ******************************************************************************/

#include "_main.h"

#if 1 //토글
// [PATCH] 부팅/활성화 순서 제어용 EventFlags
static osEventFlagsId_t gAppStartFlags;
#define APPFLAG_CONSOLE_READY  (1U << 0) // Console 	초기화 완료
#define APPFLAG_APP_READY      (1U << 1) // App    	초기화 완료
#define APPFLAG_ETHERNET_READY (1U << 2) // ETHERNET 초기화 완료
#define APPFLAG_CAN_READY      (1U << 3) // CAN	  	초기화 완료

#if 0 /// CLI Thread *********************************************************{
#define RTE_RTOS_THREAD_IDX_CLI 0
static uint32_t Debug_Time_Thread_CLI_Call;       static uint32_t Debug_Time_Thread_CLI_Call_Stamp;
static uint32_t Debug_Time_Thread_CLI_Exec;       static uint32_t Debug_Time_Thread_CLI_Exec_Stamp;

static os_thread_t    Thread_CLI_cb_mem;
static uint8_t        Thread_CLI_stack_mem[WD_THREAD_STACK_SIZE];
       osThreadId_t   Thread_CLI_ID;
const  osThreadAttr_t Thread_CLI_Attr = {
  .name       = "CLI",
  .cb_mem     = &Thread_CLI_cb_mem,
  .cb_size    = sizeof(Thread_CLI_cb_mem),
  .stack_mem  = Thread_CLI_stack_mem,
  .stack_size = sizeof(Thread_CLI_stack_mem),
  .priority   = osPriorityLow
};
static wd_reg_timer_t* const Thread_CLI_Timer_Source = (wd_reg_timer_t  *)&wd_reg_alias[5 + RTE_RTOS_THREAD_IDX_CLI];
static wd_reg_timer_t* const Thread_CLI_Timer        = (wd_reg_timer_t  *)&wd_reg_alias[15 +  RTE_RTOS_THREAD_IDX_CLI];

static void Thread_CLI_Scheduler (wd_reg_timer_t *timer)
{
  UNUSED(timer);

  if(timer->Tick_100ms)
  {
  }
}
static void Thread_CLI_Scheduler_Post (wd_reg_timer_t *timer)
{
  UNUSED(timer);
}

__NO_RETURN void Thread_CLI (void *arg)                                         // ★   Bookmark #003 Thread_CLI
{
  UNUSED(arg);

  Debug_Time_Thread_CLI_Call_Stamp      = wd_time_get_us();

  while(1)
  {
    Debug_Time_Thread_CLI_Call       = wd_time_calc_us(&Debug_Time_Thread_CLI_Call_Stamp);
    Debug_Time_Thread_CLI_Call_Stamp = wd_time_get_us();
    Debug_Time_Thread_CLI_Exec_Stamp = Debug_Time_Thread_CLI_Call_Stamp;

    wd_sched_tick_driver(Thread_CLI_Timer_Source, Thread_CLI_Timer);
    Thread_CLI_Scheduler(Thread_CLI_Timer);

    Thread_CLI_Scheduler_Post(Thread_CLI_Timer);
    wd_sched_finalize(Thread_CLI_Timer);

    Debug_Time_Thread_CLI_Exec       = wd_time_calc_us(&Debug_Time_Thread_CLI_Exec_Stamp);

    osThreadYield();
  }
}
#endif /// CLI Thread *********************************************************}

#if 1 /// Console Thread *****************************************************{
#define RTE_RTOS_THREAD_IDX_CONSOLE 1
static uint32_t Debug_Time_Thread_Console_Call;
static uint32_t Debug_Time_Thread_Console_Call_Stamp;
static uint32_t Debug_Time_Thread_Console_Exec;
static uint32_t Debug_Time_Thread_Console_Exec_Stamp;

static os_thread_t Thread_Console_cb_mem;
static uint8_t Thread_Console_stack_mem[WD_THREAD_STACK_SIZE];
osThreadId_t Thread_Console_ID;
const osThreadAttr_t Thread_Console_Attr = {.name = "Console",
                                            .cb_mem = &Thread_Console_cb_mem,
                                            .cb_size = sizeof(Thread_Console_cb_mem),
                                            .stack_mem = Thread_Console_stack_mem,
                                            .stack_size = sizeof(Thread_Console_stack_mem),
                                            .priority = osPriorityLow};
static wd_reg_timer_t *const Thread_Console_Timer_Source =
    (wd_reg_timer_t *)&wd_reg_alias[5 + RTE_RTOS_THREAD_IDX_CONSOLE];
static wd_reg_timer_t *const Thread_Console_Timer =
    (wd_reg_timer_t *)&wd_reg_alias[15 + RTE_RTOS_THREAD_IDX_CONSOLE];

static void Thread_Console_Scheduler(wd_reg_timer_t *timer) {
    UNUSED(timer);
    if (timer->Tick_100ms) {
    }
}
static void Thread_Console_Scheduler_Post(wd_reg_timer_t *timer) {
    UNUSED(timer);
}

__NO_RETURN void Thread_Console(void *arg) // ★   Bookmark #004 Thread_Console
{
    UNUSED(arg);

    Debug_Time_Thread_Console_Call_Stamp = wd_time_get_us();
    // (기존 Console 초기화/로그인/CLI 준비 등)
    // Console이 “활성”됐다고 간주되는 지점 직후에 플래그 세트
    osEventFlagsSet(gAppStartFlags, APPFLAG_CONSOLE_READY); // [PATCH]

    while (1) {
        Debug_Time_Thread_Console_Call = wd_time_calc_us(&Debug_Time_Thread_Console_Call_Stamp);
        Debug_Time_Thread_Console_Call_Stamp = wd_time_get_us();
        Debug_Time_Thread_Console_Exec_Stamp = Debug_Time_Thread_Console_Call_Stamp;

        wd_sched_tick_driver(Thread_Console_Timer_Source, Thread_Console_Timer);
        Thread_Console_Scheduler(Thread_Console_Timer);

#if (RTE_COMPONENT_CLI)
        wd_cli_simple_loop();
#endif

        Thread_Console_Scheduler_Post(Thread_Console_Timer);
        wd_sched_finalize(Thread_Console_Timer);

        Debug_Time_Thread_Console_Exec = wd_time_calc_us(&Debug_Time_Thread_Console_Exec_Stamp);

        osThreadYield();
    }
}
#endif /// Console Thread *****************************************************}

#if 1 /// Ethernet Thread ****************************************************{
#define RTE_RTOS_THREAD_IDX_ETHERNET 2
static uint32_t Debug_Time_Thread_Ethernet_Call;
static uint32_t Debug_Time_Thread_Ethernet_Call_Stamp;
static uint32_t Debug_Time_Thread_Ethernet_Exec;
static uint32_t Debug_Time_Thread_Ethernet_Exec_Stamp;

static os_thread_t Thread_Ethernet_cb_mem;
static uint8_t Thread_Ethernet_stack_mem[WD_THREAD_STACK_SIZE];
osThreadId_t Thread_Ethernet_ID;
const osThreadAttr_t Thread_Ethernet_Attr = {.name = "Ethernet",
                                             .cb_mem = &Thread_Ethernet_cb_mem,
                                             .cb_size = sizeof(Thread_Ethernet_cb_mem),
                                             .stack_mem = Thread_Ethernet_stack_mem,
                                             .stack_size = sizeof(Thread_Ethernet_stack_mem),
                                             .priority = osPriorityLow};
static wd_reg_timer_t *const Thread_Ethernet_Timer_Source =
    (wd_reg_timer_t *)&wd_reg_alias[5 + RTE_RTOS_THREAD_IDX_ETHERNET];
static wd_reg_timer_t *const Thread_Ethernet_Timer =
    (wd_reg_timer_t *)&wd_reg_alias[15 + RTE_RTOS_THREAD_IDX_ETHERNET];

static void Thread_Ethernet_Scheduler(wd_reg_timer_t *timer) {
    UNUSED(timer);
    if (timer->Tick_100ms) {
    }
}
static void Thread_Ethernet_Scheduler_Post(wd_reg_timer_t *timer) {
    UNUSED(timer);
}

__NO_RETURN void Thread_Ethernet(void *arg) // ★   Bookmark #008 Thread_Ethernet
{
    UNUSED(arg);

    Debug_Time_Thread_Ethernet_Call_Stamp = wd_time_get_us();
    // wait until the application thread is ready
    osEventFlagsWait(gAppStartFlags, APPFLAG_APP_READY, osFlagsWaitAll | osFlagsNoClear,
                     osWaitForever);

    wd_reg_system->EVT_Ethernet_Config_Update = false;

#if ( \
    RTE_SERVICE_NET_ETHERNET) /// ******************************** ★ System Initialization #026 wd_net_eth_net_port_ethernet {
    {
        wd_net_eth_open();
    }
#endif /// *******************************************************************   System Initialization #026 wd_net_eth_net_port_ethernet }

    osEventFlagsSet(gAppStartFlags, APPFLAG_ETHERNET_READY); // [PATCH]

    while (1) {
        Debug_Time_Thread_Ethernet_Call = wd_time_calc_us(&Debug_Time_Thread_Ethernet_Call_Stamp);
        Debug_Time_Thread_Ethernet_Call_Stamp = wd_time_get_us();
        Debug_Time_Thread_Ethernet_Exec_Stamp = Debug_Time_Thread_Ethernet_Call_Stamp;

        wd_sched_tick_driver(Thread_Ethernet_Timer_Source, Thread_Ethernet_Timer);
        Thread_Ethernet_Scheduler(Thread_Ethernet_Timer);

#if (RTE_COMPONENT_INTERFACE_ETHERNET)
        if (wd_reg_system
                ->EVT_Ethernet_Config_Update) // apply changed network configuration at runtime
        {
            wiz_NetInfo *net_info = wd_eth_driver_ethernet_module.Net_Info;

            wd_eth_user_net_info(
                net_info,
                &wd_eth_socket[RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP].Port,
                &wd_eth_socket[RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG]
                     .Port);

            wd_eth_driver_ethernet_module.Initialize(WDIF_ARG_RESET);
            wd_reg_system->EVT_Ethernet_Config_Update = false;
        }

#if (RTE_SERVICE_NET_ETHERNET)
        wd_eth_data.Process(&wd_net_eth_net_port_ethernet, (uint32_t)Thread_Ethernet_Timer);
#if (RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG == 1)
        wd_eth_data.Process(&wd_net_eth_net_port_ethernet_debug, (uint32_t)Thread_Ethernet_Timer);
#elif (RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG == 2)
        Driver_Ethernet_Terminal_Ops(
            RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG);
#endif
#endif

#if (RTE_COMPONENT_INTERFACE_ETHERNET_TELNET)
        wd_eth_telnet_ops(RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE);
#endif
#endif

        Thread_Ethernet_Scheduler_Post(Thread_Ethernet_Timer);
        wd_sched_finalize(Thread_Ethernet_Timer);

        Debug_Time_Thread_Ethernet_Exec = wd_time_calc_us(&Debug_Time_Thread_Ethernet_Exec_Stamp);

        osThreadYield();
    }
}
#endif /// Ethernet Thread *****************************************************}

#if 1 /// CAN Thread *********************************************************{
#define RTE_RTOS_THREAD_IDX_CAN 3
static uint32_t Debug_Time_Thread_CAN_Call;
static uint32_t Debug_Time_Thread_CAN_Call_Stamp;
static uint32_t Debug_Time_Thread_CAN_Exec;
static uint32_t Debug_Time_Thread_CAN_Exec_Stamp;
static os_thread_t Thread_CAN_cb_mem;
static uint8_t Thread_CAN_stack_mem[WD_THREAD_STACK_SIZE];
osThreadId_t Thread_CAN_ID;
const osThreadAttr_t Thread_CAN_Attr = {.name = "CAN",
                                        .cb_mem = &Thread_CAN_cb_mem,
                                        .cb_size = sizeof(Thread_CAN_cb_mem),
                                        .stack_mem = Thread_CAN_stack_mem,
                                        .stack_size = sizeof(Thread_CAN_stack_mem),
                                        .priority = osPriorityLow};
static wd_reg_timer_t *const Thread_CAN_Timer_Source =
    (wd_reg_timer_t *)&wd_reg_alias[5 + RTE_RTOS_THREAD_IDX_CAN];
static wd_reg_timer_t *const Thread_CAN_Timer =
    (wd_reg_timer_t *)&wd_reg_alias[15 + RTE_RTOS_THREAD_IDX_CAN];

static void Thread_CAN_Scheduler(wd_reg_timer_t *timer) {
    UNUSED(timer);
    if (timer->Tick_100ms) {
    }
}
static void Thread_CAN_Scheduler_Post(wd_reg_timer_t *timer) {
    UNUSED(timer);
}

__NO_RETURN void Thread_CAN(void *arg) // ★   Bookmark #007 Thread_CAN
{
    UNUSED(arg);

    osEventFlagsWait(gAppStartFlags, APPFLAG_ETHERNET_READY, osFlagsWaitAll | osFlagsNoClear,
                     osWaitForever); //Ethenet 스레드 생성 완료까지 대기

    Debug_Time_Thread_CAN_Call_Stamp = wd_time_get_us();

    /* CAN Net Port 오픈 */
#if (RTE_SERVICE_NET_CAN)
    wd_net_can_open();
#endif

    osEventFlagsSet(gAppStartFlags, APPFLAG_CAN_READY); // CAN 스레드 생성 완료 플래그[PATCH]

    while (1) {
        Debug_Time_Thread_CAN_Call = wd_time_calc_us(&Debug_Time_Thread_CAN_Call_Stamp);
        Debug_Time_Thread_CAN_Call_Stamp = wd_time_get_us();
        Debug_Time_Thread_CAN_Exec_Stamp = Debug_Time_Thread_CAN_Call_Stamp;

        wd_sched_tick_driver(Thread_CAN_Timer_Source, Thread_CAN_Timer);
        Thread_CAN_Scheduler(Thread_CAN_Timer);

#if (RTE_SERVICE_NET_CAN)
        /* 여기서 CAN Polling – Ethernet과 동일 구조 */
        wd_fdcan_data.Process(&wd_net_can_net_port_can, (uint32_t)Thread_CAN_Timer);
#endif

        Thread_CAN_Scheduler_Post(Thread_CAN_Timer);
        wd_sched_finalize(Thread_CAN_Timer);

        Debug_Time_Thread_CAN_Exec = wd_time_calc_us(&Debug_Time_Thread_CAN_Exec_Stamp);

        osThreadYield();
    }
}
#endif /// CAN Thread *********************************************************}


#if 1 /// App Thread ********************************************************{
/* Application hooks : implement in the application layer (weak defaults do nothing) */
__WEAK void App_Initialize(void) {
}

__WEAK void App_Process(wd_reg_timer_t *timer) {
    UNUSED(timer);
}

#define RTE_RTOS_THREAD_IDX_APP 4
uint32_t Debug_Time_Thread_App_Call;
uint32_t Debug_Time_Thread_App_Call_Stamp;
static uint32_t Debug_Time_Thread_App_Exec;
static uint32_t Debug_Time_Thread_App_Exec_Stamp;

static os_thread_t Thread_App_cb_mem;
static uint8_t Thread_App_stack_mem[WD_THREAD_STACK_SIZE * 2];
osThreadId_t Thread_App_ID;
const osThreadAttr_t Thread_App_Attr = {.name = "App",
                                        .cb_mem = &Thread_App_cb_mem,
                                        .cb_size = sizeof(Thread_App_cb_mem),
                                        .stack_mem = Thread_App_stack_mem,
                                        .stack_size = sizeof(Thread_App_stack_mem),
                                        .priority = osPriorityLow};
static wd_reg_timer_t *const Thread_App_Timer_Source =
    (wd_reg_timer_t *)&wd_reg_alias[5 + RTE_RTOS_THREAD_IDX_APP];
static wd_reg_timer_t *const Thread_App_Timer =
    (wd_reg_timer_t *)&wd_reg_alias[15 + RTE_RTOS_THREAD_IDX_APP];

__NO_RETURN void Thread_App(void *arg) // ★   Bookmark #009 Thread_App
{
    UNUSED(arg);

    Debug_Time_Thread_App_Call_Stamp = wd_time_get_us();
    // [PATCH] Console 완료 대기 (바쁜 대기 금지, 이벤트로 대기)
    osEventFlagsWait(gAppStartFlags, APPFLAG_CONSOLE_READY, osFlagsWaitAll | osFlagsNoClear,
                     osWaitForever);

#if (RTE_COMPONENT_VIO)
    wd_drv_vio.Set(WD_VIO_LED_RGB, WD_COLOR_RGB_CYAN);
#endif
    App_Initialize(); // application initialize (weak hook)
#if (RTE_COMPONENT_VIO)
    wd_drv_vio.Set(WD_VIO_LED_RGB, WD_COLOR_RGB_BLUE);
#endif

    wd_reg_system->EVT_Application_Ready = true;

    osEventFlagsSet(gAppStartFlags, APPFLAG_APP_READY); // [PATCH]

    while (1) {
        Debug_Time_Thread_App_Call = wd_time_calc_us(&Debug_Time_Thread_App_Call_Stamp);
        Debug_Time_Thread_App_Call_Stamp = wd_time_get_us();
        Debug_Time_Thread_App_Exec_Stamp = Debug_Time_Thread_App_Call_Stamp;

        wd_sched_tick_driver(Thread_App_Timer_Source, Thread_App_Timer);
        App_Process(Thread_App_Timer); // application process (weak hook)
        wd_sched_finalize(Thread_App_Timer);

        Debug_Time_Thread_App_Exec = wd_time_calc_us(&Debug_Time_Thread_App_Exec_Stamp);

        osThreadYield();
    }
}
#endif /// App Thread *********************************************************}


#if 1 /// API_Main Thread ****************************************************{
static uint32_t Debug_Time_Thread_API_Main_Call;
static uint32_t Debug_Time_Thread_API_Main_Call_Stamp;
static uint32_t Debug_Time_Thread_API_Main_Exec;
static uint32_t Debug_Time_Thread_API_Main_Exec_Stamp;

static os_thread_t Thread_API_Main_cb_mem;
static uint8_t Thread_API_Main_stack_mem[WD_THREAD_STACK_SIZE];

osThreadId_t Thread_API_Main_ID;
const osThreadAttr_t Thread_API_Main_Attr = {.name = "Main",
                                             .cb_mem = &Thread_API_Main_cb_mem,
                                             .cb_size = sizeof(Thread_API_Main_cb_mem),
                                             .stack_mem = Thread_API_Main_stack_mem,
                                             .stack_size = sizeof(Thread_API_Main_stack_mem),
                                             .priority = osPriorityLow};

static void API_Main_Process(void) // ★   Bookmark #001 API_Main_Process
{
#if (RTE_SERVICE_FIRMWARE) && (RTE_SERVICE_MMU)
    if (wd_reg_system->EVT_Firmware_System_Update) {
        wd_mmu_multi_object_t object = {
            .Mask = WD_MMU_OBJECT_ALL,
            .Object_Size = 1, //2, //3,
            .Num = sizeof(wd_firmware_t),
            .Data = &wd_fw_data,
            .Object[0] =
                &MAP->ROM->Firmware, // Firmware는 NOR/NAND 상태와 무관하게 항상 내부 플래시 사용
        };

        wd_reg_system->EVT_Firmware_System_Update = false;

        wd_fw_data.FW_Activate = true;
        wd_fw_data.FW_Area_Idx = 0;
        wd_fw_data.FW_ISP_Mode_Magic = 0;
        wd_fw_data.FW_Update_Interface_Version =
            WDIF_FIRMWARE_UPDATE_INTERFACE_MAGIC_NUMBER + WDIF_FIRMWARE_UPDATE_INTERFACE_VERSION;
        wd_fw_data.FW_Type = wd_fw_fota_info.File_Type;
        wd_fw_data.FW_Image_Addr = (uint32_t)wd_fw_header_app_down;
        wd_fw_data.FW_Image_Size = wd_fw_fota_info.File_Length;
        wd_fw_data.Digest =
            wd_toolkit_crc16(&wd_fw_data, sizeof(wd_fw_data) - sizeof(wd_fw_data.Digest));
        wd_mmu_write_multi(WD_MMU_COMMAND_WRITE_CRC, &object);
        wd_env->firmware_update_flag = true;
        wd_env_save();
        printf("\033[1;44m\r\n[Firmware Update Execute !!!]\033[0m\r\n");
        NVIC_SystemReset();
    }
#endif
}

static void API_Main_Scheduler(void) {
    UNUSED(Debug_Time_Thread_API_Main_Call);
    UNUSED(Debug_Time_Thread_API_Main_Call_Stamp);
    UNUSED(Debug_Time_Thread_API_Main_Exec);
    UNUSED(Debug_Time_Thread_API_Main_Exec_Stamp);
    UNUSED(Debug_Time_Thread_Console_Call);
    UNUSED(Debug_Time_Thread_Console_Call_Stamp);
    UNUSED(Debug_Time_Thread_Console_Exec);
    UNUSED(Debug_Time_Thread_Console_Exec_Stamp);
    UNUSED(Debug_Time_Thread_CAN_Call);
    UNUSED(Debug_Time_Thread_CAN_Call_Stamp);
    UNUSED(Debug_Time_Thread_CAN_Exec);
    UNUSED(Debug_Time_Thread_CAN_Exec_Stamp);
    UNUSED(Debug_Time_Thread_Ethernet_Call);
    UNUSED(Debug_Time_Thread_Ethernet_Call_Stamp);
    UNUSED(Debug_Time_Thread_Ethernet_Exec);
    UNUSED(Debug_Time_Thread_Ethernet_Exec_Stamp);
    UNUSED(Debug_Time_Thread_App_Call);
    UNUSED(Debug_Time_Thread_App_Call_Stamp);
    UNUSED(Debug_Time_Thread_App_Exec);
    UNUSED(Debug_Time_Thread_App_Exec_Stamp);

#if 0  /// LED **************************************************************{
  {
  }
#endif /// LED **************************************************************}

#if 0  /// Button ***********************************************************{
  {

  }
#endif /// Button ***********************************************************}

#if 1 /// Software wd_reg_timer *************************************************{
    if (wd_reg_timer->Tick_50ms) {
    }

    if (wd_reg_timer->Tick_100ms) {
        //    wd_drv_vio.Set(WD_VIO_LED_RGB, (wd_drv_vio.Get(WD_VIO_LED_RGB) % WD_COLOR_RGB_MASK) + 1);
        //TODO
    }

    if (wd_reg_timer->Tick_250ms) {
        if ((wd_reg_status->Status_ADC == WD_STATUS_ERROR) ||
            (wd_reg_status->Status_NVM_NOR_Flash == WD_STATUS_ERROR)) {
            //LED 추가
#if (RTE_COMPONENT_VIO)
            wd_drv_vio.Set(WD_VIO_LED_RGB, ((wd_drv_vio.Get(WD_VIO_LED_RGB) == WD_COLOR_RGB_RED)
                                                ? WD_COLOR_RGB_WHITE
                                                : WD_COLOR_RGB_RED));
#endif
            //TODO
        }
    }

    if (wd_reg_timer->Tick_500ms) {
        //    wd_drv_vio.Set(WD_VIO_LED_RGB, (wd_drv_vio.Get(WD_VIO_LED_RGB) % WD_COLOR_RGB_MASK) + 1);
    }

    if (wd_reg_timer->Tick_1000ms) // 1초에 한번씩
    {
        osThreadState_t osthreadstate;

        uint32_t debug_time_thread;
        const char *thread_name;
        osThreadId_t tid;
        const osThreadAttr_t *attr;
        void (*func)(void *arg);

        int i;

        if (osThreadGetCount() !=
            RTE_RTOS_THREAD_NUMBER + 3 -
                RTE_RTOS_THREAD_MINUS_NUMBER) // RTE_RTOS_THREAD_NUMBER 개수에 따라 수정 추가되면 RTE_RTOS_THREAD_MINUS_NUMBER 증가
        {
            for (i = 0; i < 5; i++) {
                WD_LOG_EMERG(
                    "Thread Count Error [%d / %d]", osThreadGetCount(),
                    RTE_RTOS_THREAD_NUMBER + 3 -
                        RTE_RTOS_THREAD_MINUS_NUMBER); // RTE_RTOS_THREAD_NUMBER 개수에 따라 수정 추가되면 RTE_RTOS_THREAD_MINUS_NUMBER 증가
            }
        }

#undef LOCAL_MACRO
#define LOCAL_MACRO(name)                                      \
    debug_time_thread = Debug_Time_Thread_##name##_Call_Stamp; \
    thread_name = #name;                                       \
    tid = Thread_##name##_ID;                                  \
    attr = &Thread_##name##_Attr;                              \
    func = Thread_##name
        for (i = 0; i < RTE_RTOS_THREAD_NUMBER_MAX; i++) {
            switch (i) {
                //        case RTE_RTOS_THREAD_IDX_CLI      :  LOCAL_MACRO(CLI);       break;
            case RTE_RTOS_THREAD_IDX_CONSOLE:
                LOCAL_MACRO(Console);
                break;
                //        case RTE_RTOS_THREAD_IDX_TERMINAL :  LOCAL_MACRO(Terminal);  break;
                //        case RTE_RTOS_THREAD_IDX_SERIAL   :  LOCAL_MACRO(Serial);    break;
            case RTE_RTOS_THREAD_IDX_ETHERNET:
                LOCAL_MACRO(Ethernet);
                break;
            case RTE_RTOS_THREAD_IDX_CAN:
                LOCAL_MACRO(CAN);
                break;
            case RTE_RTOS_THREAD_IDX_APP:
                LOCAL_MACRO(App);
                break;
                //        case RTE_RTOS_THREAD_IDX_DEBUG    :  LOCAL_MACRO(Debug);     break;

            default:
                continue;
            }

            //      if(wd_time_calc_us(&debug_time_thread) > ((i == RTE_RTOS_THREAD_IDX_APP) ? 50 * 1000 * 1000 : 30 * 1000 * 1000))
            if (wd_time_calc_us(&debug_time_thread) >
                ((i == 6) ? 50 * 1000 * 1000 : 30 * 1000 * 1000)) {
                WD_BREAK();
                WD_LOG_EMERG("Thread %s Monitor Error", thread_name);

                osthreadstate = osThreadGetState(tid);
                if (osthreadstate == osThreadError) {
                    WD_LOG_EMERG("Already Terminate %s Thread Error", thread_name);
                } else if (osThreadTerminate(tid) != osOK) {
                    WD_LOG_EMERG("Terminate %s Thread Error", thread_name);
                }

                osthreadstate = osThreadGetState(tid);
                if (osthreadstate != osThreadError) {
                    WD_LOG_EMERG("Already Create %s Thread Error", thread_name);
                } else {
                    tid = osThreadNew(func, NULL, attr);
                    if (tid == NULL) {
                        WD_LOG_EMERG("Create %s Thread Error", thread_name);
                    }
                }
            }
        }
#undef LOCAL_MACRO
    }
#endif /// Software wd_reg_timer *************************************************}

#if 1 /// RTC **************************************************************{
    if (wd_reg_timer->Sec) {
#if (RTE_COMPONENT_CLI)
        if (wd_cli_occupancy_time) {
            wd_cli_occupancy_time--;
            if (wd_cli_occupancy_time == 0) {
                wd_cli_unlock();
            }
        }
#endif
        // if (++monitor_cnt >= 10)
        //  {    // 10초마다 출력
        //     monitor_cnt = 0;
        //     wd_taskmon_report();
        // }
    }

    if (wd_reg_timer->Min) {
    }

    if (wd_reg_timer->Hour) {
    }

    if (wd_reg_timer->Date) {
    }

    if (wd_reg_timer->Mon) {
    }

    if (wd_reg_timer->Year) {
    }

    if (wd_reg_timer->Sec_3) {
        wd_reg_system->EVT_ADC_Calc = true;
    }

    if (wd_reg_timer->Sec_5) {
    }

    if (wd_reg_timer->Sec_10) {
    }
#endif /// RTC **************************************************************}
}

static void API_Main_Scheduler_Post(void) {
#if 1  /// Button ***********************************************************{
#endif /// Button ***********************************************************}

#if 1 /// Software wd_reg_timer *************************************************{
    if (wd_reg_timer->Tick_50ms) {
    }

    if (wd_reg_timer->Tick_100ms) {
    }

    if (wd_reg_timer->Tick_250ms) {
    }

    if (wd_reg_timer->Tick_500ms) {
    }

    if (wd_reg_timer->Tick_1000ms) {
    }
#endif /// Software wd_reg_timer *************************************************}

#if 1 /// RTC **************************************************************{
    if (wd_reg_timer->Sec) {
    }

    if (wd_reg_timer->Min) {
    }

    if (wd_reg_timer->Hour) {
    }

    if (wd_reg_timer->Date) {
    }

    if (wd_reg_timer->Mon) {
    }

    if (wd_reg_timer->Year) {
    }

    if (wd_reg_timer->Sec_3) {
    }

    if (wd_reg_timer->Sec_5) {
    }

    if (wd_reg_timer->Sec_10) {
    }
#endif /// RTC **************************************************************}
}
__NO_RETURN void Thread_API_Main(void *arg) {
    UNUSED(arg);

    Component_Initialization();
    Service_Initialization();
    Application_Initialization();
    //  Firmware_Update_Patch();

    // [PATCH] 부팅 순서 제어용 EventFlags 생성
    gAppStartFlags = osEventFlagsNew(NULL);
#ifndef NDEBUG
    if (gAppStartFlags == NULL) {
        WD_LOG_ERROR("Create EventFlags Error");
    }
#endif

    Debug_Time_Thread_API_Main_Call_Stamp = wd_time_get_us();
    Debug_Time_Thread_Console_Call_Stamp = wd_time_get_us();
    Debug_Time_Thread_Ethernet_Call_Stamp = wd_time_get_us();
    Debug_Time_Thread_CAN_Call_Stamp = wd_time_get_us();
    Debug_Time_Thread_App_Call_Stamp = wd_time_get_us();

    wd_print_level = wd_env->Console_Level;

    // [PATCH] 스레드 생성 순서: Console -> App -> Ethernet
    // App thread must be ready before Ethernet thread starts (network config may come from the application)
    Thread_Console_ID =
        osThreadNew(Thread_Console, NULL,
                    &Thread_Console_Attr); // ◇ CMSIS_RTOS #001 osThreadNew - Thread_Console
#ifndef NDEBUG
    if (Thread_Console_ID == NULL) {
        WD_LOG_ERROR("Create Console Thread Error");
    }
#endif
    // Console이 완전히 올라올 때까지 '생성 자체'를 다음으로 미룸
    osEventFlagsWait(gAppStartFlags, APPFLAG_CONSOLE_READY, osFlagsWaitAll | osFlagsNoClear,
                     osWaitForever);


    Thread_App_ID = osThreadNew(Thread_App, NULL,
                                &Thread_App_Attr); // ◇ CMSIS_RTOS #001 osThreadNew - Thread_App
#ifndef NDEBUG
    if (Thread_App_ID == NULL) {
        WD_LOG_ERROR("Create App Thread Error");
    }
#endif
    osEventFlagsWait(gAppStartFlags, APPFLAG_APP_READY, osFlagsWaitAll | osFlagsNoClear,
                     osWaitForever);


    Thread_Ethernet_ID =
        osThreadNew(Thread_Ethernet, NULL,
                    &Thread_Ethernet_Attr); // ◇ CMSIS_RTOS #001 osThreadNew - Thread_Ethernet
#ifndef NDEBUG
    if (Thread_Ethernet_ID == NULL) {
        WD_LOG_ERROR("Create Ethernet Thread Error");
    }
#endif
    osEventFlagsWait(gAppStartFlags, APPFLAG_ETHERNET_READY, osFlagsWaitAll | osFlagsNoClear,
                     osWaitForever);

    Thread_CAN_ID = osThreadNew(Thread_CAN, NULL,
                                &Thread_CAN_Attr); // ◇ CMSIS_RTOS #001 osThreadNew - Thread_CAN
#ifndef NDEBUG
    if (Thread_CAN_ID == NULL) {
        WD_LOG_ERROR("Create CAN Thread Error");
    }
#endif

    osEventFlagsWait(gAppStartFlags, APPFLAG_CAN_READY, osFlagsWaitAll | osFlagsNoClear,
                     osWaitForever);

    wd_time_read(&wd_time_time);

    if (wd_reg_system->EVT_RTC_Initialize) {
        wd_reg_system->EVT_RTC_Initialize = false;

        WD_LOG_DEBUG("%s", asctime(&wd_time_time_tm));
        WD_PRINT(WD_MSG_EVENT, "%04d/%02d/%02d %02d:%02d:%02d", wd_time_time_tm.tm_year + 1900,
                 wd_time_time_tm.tm_mon, wd_time_time_tm.tm_mday, wd_time_time_tm.tm_hour,
                 wd_time_time_tm.tm_min, wd_time_time_tm.tm_sec);

        wd_time_time_tm.tm_year = 123;
        wd_time_time_tm.tm_mon = 0;
        wd_time_time_tm.tm_mday = 1;
        wd_time_time_tm.tm_hour = 1;
        wd_time_time_tm.tm_mday = 1;
        wd_time_time_tm.tm_hour = 0;
        wd_time_time_tm.tm_min = 0;
        wd_time_time_tm.tm_sec = 0;
        wd_time_tm_to_time(&wd_time_time_tm, &wd_time_time);
        wd_time_write(wd_time_time);
        WD_PRINT(WD_MSG_EVENT_CRLF, "  →  %04d/%02d/%02d %02d:%02d:%02d",
                 wd_time_time_tm.tm_year + 1900, wd_time_time_tm.tm_mon, wd_time_time_tm.tm_mday,
                 wd_time_time_tm.tm_hour, wd_time_time_tm.tm_min, wd_time_time_tm.tm_sec);
        WD_LOG_DEBUG("%s", asctime(&wd_time_time_tm));
    }


#if (RTE_APPLICATION_TEST)
    API_Test();
#endif

    printf(WD_COLOR_RESET WD_COLOR_TIME "System Initialization Time : %lu [us]" WD_COLOR_RESET
                                        "\r\n\r\n",
           (unsigned long)wd_time_calc_us(&wd_time_debug_time));

    WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "_CONF_CONSOLE_LEVEL_COMPILE : %d",
                   WD_CONF_CONSOLE_LEVEL_COMPILE);

    wd_core_system_mode = WD_SYSTEM_MODE_NORMAL;

    while (1) // ★   Bookmark #002 Thread_API_Main
    {
        Debug_Time_Thread_API_Main_Call = wd_time_calc_us(&Debug_Time_Thread_API_Main_Call_Stamp);
        Debug_Time_Thread_API_Main_Call_Stamp = wd_time_get_us();
        Debug_Time_Thread_API_Main_Exec_Stamp = Debug_Time_Thread_API_Main_Call_Stamp;

        WD_WDT_REFRESH();

        if (wd_global.Debug_Point != 0xFFFFFFFF) {
            WD_LOG_EMERG("[Critical Error] Global Overflow");
        }

        wd_sched_tick();

        API_Main_Scheduler();
        API_Main_Process();
        API_Main_Scheduler_Post();
        wd_sched_finalize(wd_reg_timer);

        Debug_Time_Thread_API_Main_Exec = wd_time_calc_us(&Debug_Time_Thread_API_Main_Exec_Stamp);

        osThreadYield();
    }
}
#endif /// API_Main Thread ****************************************************}

#endif //#if 1	//토글

/** @} end of file */
