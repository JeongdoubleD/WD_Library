/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_vio.c
 *
 *  Created on: May 13, 2025
 *      Author: doubled
 */

#include "_wd_vio.h"

#if 1 // 토글

#if (RTE_COMPONENT_VIO)

static const wd_vio_capabilities_t VIO_DriverCapabilities = {
    .TimeOut = 100000,
};

static wd_vio_info_t VIO_Info = {
    0,
};

static wd_vio_resources_t VIO_Resources = {.Capabilities = &VIO_DriverCapabilities,
                                           .IO =
                                               {
                                                   .SHDN = RTE_PORT_CTRL_SHDN,
                                                   .Mux_A = RTE_PORT_CTRL_MUX_A,
                                                   .Mux_B = RTE_PORT_CTRL_MUX_B,
                                                   .LED_Red = RTE_PORT_LED_ERROR,
                                                   .LED_Green = RTE_PORT_LED_POWER,
                                                   .LED_Blue = RTE_PORT_LED_STATUS,
                                                   .LED_RUN = RTE_PORT_LED_RUN,
                                                   .LED_ADC = RTE_PORT_LED_ADC,
                                                   .LED_DDS = RTE_PORT_LED_DDS,
                                                   .LED_COM = RTE_PORT_LED_COMM,
                                                   .FAN = RTE_PORT_CTRL_FAN,
                                                   .Mux_sel_1 = RTE_PORT_MUX_CAP_SEL1,
                                                   .Mux_sel_2 = RTE_PORT_MUX_CAP_SEL2,
                                                   .Mux_sel_3 = RTE_PORT_MUX_CAP_SEL3,
                                                   .Mux_sel_4 = RTE_PORT_MUX_CAP_SEL4,
                                                   .VAC_AMP = RTE_PORT_CTRL_VAC_AMP,
                                               },
                                           .Info = &VIO_Info};

#undef LOCAL_RES
static wd_vio_resources_t *LOCAL_RES = &VIO_Resources;

#if 0
static void vio_callback(uint32_t event)
{
  LOCAL_RES->Info->Event = event;
  if(LOCAL_RES->Info->CB_Event)
  {
    LOCAL_RES->Info->CB_Event(event);
  }
}
#endif

#if (RTE_CONFIG_DRIVER_EXTENSION)
static const char *vio_get_version(void) {
    return (WD_CODE_OK);
}

static void *vio_get_capabilities(void) {
    return (WD_CODE_OK);
}
#endif

static int32_t vio_initialize(wd_vio_signal_event_t cb_event) {
    if (LOCAL_RES->Info->State & WD_STATE_INITIALIZED) {
        return (WD_CODE_OK);
    }

    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "VIO Constructor");

    memset(&VIO_Info, 0, sizeof(VIO_Info));

    LOCAL_RES->Info->CB_Event = cb_event;

    LOCAL_RES->Info->State = WD_STATE_INITIALIZED;

    return (WD_CODE_OK);
}

#if (RTE_CONFIG_DRIVER_EXTENSION)
static int32_t vio_uninitialize(void) {
    return (WD_CODE_OK);
}
#endif

static int32_t vio_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF: {
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "VIO_PowerControl:WD_POWER_OFF");

        LOCAL_RES->Info->State &= ~(WD_STATE_POWERED | WD_STATE_CONFIGURED);
    } break;

    case WD_POWER_FULL: {
        if ((LOCAL_RES->Info->State & WD_STATE_INITIALIZED) == 0U) {
            return (WD_CODE_OK);
        }

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "VIO_PowerControl:WD_POWER_FULL");


#if (RTE_CONFIG_SOFTWARE_TYPE != RTE_SOFTWARE_TYPE_BOOTLOADER)
        wd_gpio_config(LOCAL_RES->IO.SHDN, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.Mux_A, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.Mux_B, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
#endif
        wd_gpio_config(LOCAL_RES->IO.LED_Red, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.LED_Green, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.LED_Blue, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.LED_RUN, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.LED_ADC, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.LED_DDS, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.LED_COM, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.FAN, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);

        wd_gpio_config(LOCAL_RES->IO.Mux_sel_1, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.Mux_sel_2, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.Mux_sel_3, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.Mux_sel_4, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
        wd_gpio_config(LOCAL_RES->IO.VAC_AMP, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);


#if (RTE_CONFIG_SOFTWARE_TYPE != RTE_SOFTWARE_TYPE_BOOTLOADER)
        wd_drv_vio.Set(WD_VIO_SHDN, WD_VIO_SHDN_OFF);
        wd_drv_vio.Set(WD_VIO_MUX, WD_VIO_MUX_4);
#endif
        wd_drv_vio.Set(WD_VIO_LED_RGB, WD_COLOR_RGB_WHITE);
        wd_drv_vio.Set(WD_VIO_LED_RUN, WD_VIO_LED_OFF);
        wd_drv_vio.Set(WD_VIO_LED_ADC, WD_VIO_LED_OFF);
        wd_drv_vio.Set(WD_VIO_LED_DDS, WD_VIO_LED_OFF);
        wd_drv_vio.Set(WD_VIO_LED_COM, WD_VIO_LED_OFF);
        wd_drv_vio.Set(WD_VIO_FAN, WD_VIO_FAN_OFF);
        wd_drv_vio.Set(WD_VIO_MUX_SEL, WD_VIO_MUX_SEL_1);
        wd_drv_vio.Set(WD_VIO_VAC_AMP, WD_VIO_VAC_AMP_OFF);

    } break;

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);

    default:
        return (WD_CODE_ERROR);
    }

    return (WD_CODE_OK);
}

static void vio_set(uint32_t io, uint32_t arg) {
    switch (io) {
#if (RTE_CONFIG_SOFTWARE_TYPE != RTE_SOFTWARE_TYPE_BOOTLOADER)
    case WD_VIO_SHDN: {
        LOCAL_RES->Info->SHDN = arg;
        if (arg == WD_VIO_SHDN_ON) {
            wd_gpio_write(LOCAL_RES->IO.SHDN, WD_GPIO_ON);
#if 0
          wd_gpio_write(LOCAL_RES->IO.LED_DDS, WD_GPIO_ON);
#endif
        } else if (arg == WD_VIO_SHDN_OFF) {
            wd_gpio_write(LOCAL_RES->IO.SHDN, WD_GPIO_OFF);
#if 0
          wd_gpio_write(LOCAL_RES->IO.LED_DDS, WD_GPIO_OFF);
#endif
        } else {
            WD_LOG_ERROR();
        }
    } break;

    case WD_VIO_MUX: {
        LOCAL_RES->Info->Mux = arg;

        switch (arg) {
        case WD_VIO_MUX_1:
            wd_gpio_write(LOCAL_RES->IO.Mux_A, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_B, WD_GPIO_OFF);
            break;
        case WD_VIO_MUX_2:
            wd_gpio_write(LOCAL_RES->IO.Mux_A, WD_GPIO_ON);
            wd_gpio_write(LOCAL_RES->IO.Mux_B, WD_GPIO_OFF);
            break;
        case WD_VIO_MUX_3:
            wd_gpio_write(LOCAL_RES->IO.Mux_A, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_B, WD_GPIO_ON);
            break;
        case WD_VIO_MUX_4:
            wd_gpio_write(LOCAL_RES->IO.Mux_A, WD_GPIO_ON);
            wd_gpio_write(LOCAL_RES->IO.Mux_B, WD_GPIO_ON);
            break;
        default:
            WD_LOG_ERROR();
            break;
        }
    } break;
#endif

    case WD_VIO_LED_RGB: {
        LOCAL_RES->Info->LED_RGB = arg;

        if (WD_COLOR_RGB_WHITE < arg) {
            WD_LOG_ERROR();
        }

        if (arg & WD_COLOR_RGB_R) {
            wd_gpio_write(LOCAL_RES->IO.LED_Red, WD_GPIO_ON);
        } else {
            wd_gpio_write(LOCAL_RES->IO.LED_Red, WD_GPIO_OFF);
        }

        if (arg & WD_COLOR_RGB_G) {
            wd_gpio_write(LOCAL_RES->IO.LED_Green, WD_GPIO_ON);
        } else {
            wd_gpio_write(LOCAL_RES->IO.LED_Green, WD_GPIO_OFF);
        }

        if (arg & WD_COLOR_RGB_B) {
            wd_gpio_write(LOCAL_RES->IO.LED_Blue, WD_GPIO_ON);
        } else {
            wd_gpio_write(LOCAL_RES->IO.LED_Blue, WD_GPIO_OFF);
        }
    } break;

    case WD_VIO_LED_RUN: {
        LOCAL_RES->Info->LED_RUN = arg;

        if (arg == WD_VIO_LED_ON) {
            wd_gpio_write(LOCAL_RES->IO.LED_RUN, WD_GPIO_ON);
        } else if (arg == WD_VIO_LED_OFF) {
            wd_gpio_write(LOCAL_RES->IO.LED_RUN, WD_GPIO_OFF);
        } else {
            WD_LOG_ERROR();
        }
    } break;

    case WD_VIO_LED_ADC: {
        LOCAL_RES->Info->LED_ADC = arg;

        if (arg == WD_VIO_LED_ON) {
            wd_gpio_write(LOCAL_RES->IO.LED_ADC, WD_GPIO_ON);
        } else if (arg == WD_VIO_LED_OFF) {
            wd_gpio_write(LOCAL_RES->IO.LED_ADC, WD_GPIO_OFF);
        } else {
            WD_LOG_ERROR();
        }
    } break;

    case WD_VIO_LED_DDS: {
        LOCAL_RES->Info->LED_DDS = arg;

        if (arg == WD_VIO_LED_ON) {
            wd_gpio_write(LOCAL_RES->IO.LED_DDS, WD_GPIO_ON);
        } else if (arg == WD_VIO_LED_OFF) {
            wd_gpio_write(LOCAL_RES->IO.LED_DDS, WD_GPIO_OFF);
        } else {
            WD_LOG_ERROR();
        }
    } break;

    case WD_VIO_LED_COM: {
        LOCAL_RES->Info->LED_COM = arg;

        if (arg == WD_VIO_LED_ON) {
            wd_gpio_write(LOCAL_RES->IO.LED_COM, WD_GPIO_ON);
        } else if (arg == WD_VIO_LED_OFF) {
            wd_gpio_write(LOCAL_RES->IO.LED_COM, WD_GPIO_OFF);
        } else {
            WD_LOG_ERROR();
        }
    } break;

    case WD_VIO_FAN: {
        LOCAL_RES->Info->FAN = arg;

        if (arg == WD_VIO_LED_ON) {
            wd_gpio_write(LOCAL_RES->IO.FAN, WD_GPIO_ON);
        } else if (arg == WD_VIO_LED_OFF) {
            wd_gpio_write(LOCAL_RES->IO.FAN, WD_GPIO_OFF);
        } else {
            WD_LOG_ERROR();
        }
    } break;
    case WD_VIO_VAC_AMP: {
        LOCAL_RES->Info->VAC_AMP = arg;

        if (arg == WD_VIO_VAC_AMP_ON) {
            wd_gpio_write(LOCAL_RES->IO.VAC_AMP, WD_GPIO_ON);
        } else if (arg == WD_VIO_VAC_AMP_OFF) {
            wd_gpio_write(LOCAL_RES->IO.VAC_AMP, WD_GPIO_OFF);
        } else {
            WD_LOG_ERROR();
        }
    } break;

    case WD_VIO_MUX_SEL: {
        // LOCAL_RES->Info->Mux = arg;
        switch (arg) {
        case WD_VIO_MUX_SEL_1:
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_1, WD_GPIO_ON);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_2, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_3, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_4, WD_GPIO_OFF);
            break;
        case WD_VIO_MUX_SEL_2:
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_1, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_2, WD_GPIO_ON);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_3, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_4, WD_GPIO_OFF);
            break;
        case WD_VIO_MUX_SEL_3:
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_1, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_2, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_3, WD_GPIO_ON);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_4, WD_GPIO_OFF);
            break;
        case WD_VIO_MUX_SEL_4:
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_1, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_2, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_3, WD_GPIO_OFF);
            wd_gpio_write(LOCAL_RES->IO.Mux_sel_4, WD_GPIO_ON);
            break;
        default:
            WD_LOG_ERROR();
            break;
        }
    } break;
    default:
        break;
    }
}

static uint32_t vio_get(uint32_t io) {
    uint32_t value = 0;

    switch (io) {
#if (RTE_CONFIG_SOFTWARE_TYPE != RTE_SOFTWARE_TYPE_BOOTLOADER)
    case WD_VIO_SHDN: {
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.SHDN) == WD_GPIO_VAlUE_ON(LOCAL_RES->IO.SHDN)) {
            value = WD_VIO_SHDN_ON;
        } else {
            value = WD_VIO_SHDN_OFF;
        }
        LOCAL_RES->Info->SHDN = value;
    } break;

    case WD_VIO_MUX: {
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.Mux_A) == WD_GPIO_VAlUE_ON(LOCAL_RES->IO.Mux_A)) {
            if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.Mux_B) ==
                WD_GPIO_VAlUE_ON(LOCAL_RES->IO.Mux_B)) {
                value = WD_VIO_MUX_4;
            } else {
                value = WD_VIO_MUX_2;
            }
        } else {
            if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.Mux_B) ==
                WD_GPIO_VAlUE_ON(LOCAL_RES->IO.Mux_B)) {
                value = WD_VIO_MUX_3;
            } else {
                value = WD_VIO_MUX_1;
            }
            LOCAL_RES->Info->Mux = value;
        }
    } break;
#endif

    case WD_VIO_LED_RGB: {
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.LED_Red) ==
            WD_GPIO_VAlUE_ON(LOCAL_RES->IO.LED_Red)) {
            value |= WD_COLOR_RGB_R;
        }
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.LED_Green) ==
            WD_GPIO_VAlUE_ON(LOCAL_RES->IO.LED_Green)) {
            value |= WD_COLOR_RGB_G;
        }
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.LED_Blue) ==
            WD_GPIO_VAlUE_ON(LOCAL_RES->IO.LED_Blue)) {
            value |= WD_COLOR_RGB_B;
        }
    } break;

    case WD_VIO_LED_RUN: {
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.LED_RUN) ==
            WD_GPIO_VAlUE_ON(LOCAL_RES->IO.LED_RUN)) {
            value = WD_VIO_LED_ON;
        } else {
            value = WD_VIO_LED_OFF;
        }
        LOCAL_RES->Info->LED_RUN = value;
    } break;

    case WD_VIO_LED_ADC: {
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.LED_ADC) ==
            WD_GPIO_VAlUE_ON(LOCAL_RES->IO.LED_ADC)) {
            value = WD_VIO_LED_ON;
        } else {
            value = WD_VIO_LED_OFF;
        }
        LOCAL_RES->Info->LED_ADC = value;
    } break;

    case WD_VIO_LED_DDS: {
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.LED_DDS) ==
            WD_GPIO_VAlUE_ON(LOCAL_RES->IO.LED_DDS)) {
            value = WD_VIO_LED_ON;
        } else {
            value = WD_VIO_LED_OFF;
        }
        LOCAL_RES->Info->LED_DDS = value;
    } break;

    case WD_VIO_LED_COM: {
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.LED_COM) ==
            WD_GPIO_VAlUE_ON(LOCAL_RES->IO.LED_COM)) {
            value = WD_VIO_LED_ON;
        } else {
            value = WD_VIO_LED_OFF;
        }
        LOCAL_RES->Info->LED_COM = value;
    } break;

    case WD_VIO_FAN: {
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.FAN) == WD_GPIO_VAlUE_ON(LOCAL_RES->IO.FAN)) {
            value = WD_VIO_FAN_ON;
        } else {
            value = WD_VIO_FAN_OFF;
        }
        LOCAL_RES->Info->FAN = value;
    } break;

    case WD_VIO_VAC_AMP: {
        if ((uint32_t)wd_gpio_read(LOCAL_RES->IO.VAC_AMP) ==
            WD_GPIO_VAlUE_ON(LOCAL_RES->IO.VAC_AMP)) {
            value = WD_VIO_VAC_AMP_ON;
        } else {
            value = WD_VIO_VAC_AMP_OFF;
        }
        LOCAL_RES->Info->VAC_AMP = value;
    } break;
    default:
        break;
    }

    return (value);
}

static int32_t vio_control(uint32_t control, uint32_t io, uint32_t arg) {
    UNUSED(arg);

    switch (control & WD_VIO_COMMAND_MSK) {
    case WD_VIO_LED_SET_OFF_TIMER: {
        switch (io) {
        case WD_VIO_LED_RGB:
            LOCAL_RES->Info->LED_RGB_Off_Timer = (uint32_t)arg * 1000;
            break;
        case WD_VIO_LED_RUN:
            LOCAL_RES->Info->LED_RUN_Off_Timer = (uint32_t)arg * 1000;
            break;
        case WD_VIO_LED_ADC:
            LOCAL_RES->Info->LED_ADC_Off_Timer = (uint32_t)arg * 1000;
            break;
        case WD_VIO_LED_DDS:
            LOCAL_RES->Info->LED_DDS_Off_Timer = (uint32_t)arg * 1000;
            break;
        case WD_VIO_LED_COM:
            LOCAL_RES->Info->LED_COM_Off_Timer = (uint32_t)arg * 1000;
            break;
        default:
            WD_LOG_ERROR();
            break;
        }
    } break;

    case WD_VIO_LED_CASE_ERROR: {
        uint32_t i;
        for (i = 0; i < 15; i++) {
            wd_drv_vio.Set(WD_VIO_LED_RGB, WD_COLOR_RGB_RED);
            wd_drv_vio.Set(WD_VIO_LED_RUN, WD_VIO_LED_ON);
            wd_drv_vio.Set(WD_VIO_LED_ADC, WD_VIO_LED_ON);
            wd_drv_vio.Set(WD_VIO_LED_DDS, WD_VIO_LED_ON);
            wd_drv_vio.Set(WD_VIO_LED_COM, WD_VIO_LED_ON);
            wd_time_wait_ms(arg);
            wd_drv_vio.Set(WD_VIO_LED_RGB, WD_COLOR_RGB_BLACK);
            wd_drv_vio.Set(WD_VIO_LED_RUN, WD_VIO_LED_OFF);
            wd_drv_vio.Set(WD_VIO_LED_ADC, WD_VIO_LED_OFF);
            wd_drv_vio.Set(WD_VIO_LED_DDS, WD_VIO_LED_OFF);
            wd_drv_vio.Set(WD_VIO_LED_COM, WD_VIO_LED_OFF);
            wd_time_wait_ms(arg);
        }
    } break;
    default:
        break;
    }

    return (WD_CODE_OK);
}

static wd_vio_status_t vio_get_status(void) {
    return (LOCAL_RES->Info->Status);
}

static wd_vio_resources_t *vio_get_priv(void) {
    return (LOCAL_RES);
}

wd_vio_driver_vio_t wd_drv_vio = {
    .Initialize = vio_initialize,
    .PowerControl = vio_power_control,
    .Set = vio_set,
    .Get = vio_get,
    .Control = vio_control,
    .Get_Status = vio_get_status,
    .Get_Priv = vio_get_priv,
};

#endif // #if (RTE_COMPONENT_VIO)

#endif //#if 1 // 토글
