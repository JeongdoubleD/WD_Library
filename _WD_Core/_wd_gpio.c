/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_gpio.c
 *
 *  Created on: Dec 30, 2024
 *      Author: doubled
 */

#include "wd_core.h"

/*   ----- GPIO_TypeDef - General Purpose I/O -----
 *   __IO uint32_t MODER;    GPIO port mode register,               Address offset: 0x00
 *   __IO uint32_t OTYPER;   GPIO port output type register,        Address offset: 0x04
 *   __IO uint32_t OSPEEDR;  GPIO port output speed register,       Address offset: 0x08
 *   __IO uint32_t PUPDR;    GPIO port pull-up/pull-down register,  Address offset: 0x0C
 *   __IO uint32_t IDR;      GPIO port input data register,         Address offset: 0x10
 *   __IO uint32_t ODR;      GPIO port output data register,        Address offset: 0x14
 *   __IO uint32_t BSRR;     GPIO port bit set/reset register,      Address offset: 0x18
 *   __IO uint32_t LCKR;     GPIO port configuration lock register, Address offset: 0x1C
 *   __IO uint32_t AFR[2];   GPIO alternate function registers,     Address offset: 0x20-0x24}
 */

static wd_gpio_st_port_t Get_GPIO_Port(uint32_t idx) {
    switch (idx) {
    default:
        return (WD_PORT_NC);
    case 0:
        return (WD_GPIO_PORT_A);
    case 1:
        return (WD_GPIO_PORT_B);
    case 2:
        return (WD_GPIO_PORT_C);
    case 3:
        return (WD_GPIO_PORT_D);
    case 4:
        return (WD_GPIO_PORT_E);
    case 5:
        return (WD_GPIO_PORT_F);
    case 6:
        return (WD_GPIO_PORT_G);
    case 7:
        return (WD_GPIO_PORT_H);
    }
}

/***************************************************************************/ /**
 * \param     [in]  port          \ref wd_gpio_port_t
 * \param     [in]  pin           \ref wd_gpio_pin_t
 * \param     [in]  control        \ref wd_gpio_t
 * \return          \ref wd_code_t
 ******************************************************************************/
int32_t wd_gpio_config(uint32_t gpio, uint32_t control) {
    GPIO_TypeDef *reg = Get_GPIO_Port((gpio & WD_GPIO_PORT_MSK) >> WD_GPIO_PORT_POS);
    uint32_t pin = 1 << ((gpio & WD_GPIO_PIN_MSK) >> WD_GPIO_PIN_POS);

    int32_t ret = WD_CODE_OK;
    uint32_t value = (control & WD_GPIO_VALUE_MSK);
    uint32_t peripheral = (control & WD_GPIO_PERIPHERAL_MSK);
    uint32_t dir = (control & WD_GPIO_DIR_MSK);
    uint32_t type = (control & WD_GPIO_TYPE_MSK);
    uint32_t speed = (control & WD_GPIO_SPEED_MSK);
    uint32_t pull = (control & WD_GPIO_PULL_MSK);
    uint32_t function = (control & WD_GPIO_FUNCTION_MSK) >> WD_GPIO_FUNCTION_POS;
    GPIO_InitTypeDef gpio_initstruct = {0};

#if 0
  wd_gpio_t         gpio_t;
  wd_gpio_control_t control_t;

  memcpy(&gpio_t, &gpio, sizeof(gpio));
  memcpy(&control_t, &control, sizeof(control));
#endif

    if (gpio == 0) {
        return (WD_CODE_ERROR);
    }

    if (reg == NULL) {
        return (WD_CODE_ERROR);
    }

    if (peripheral == WD_GPIO_PERIPHERAL_IGNORE) {
        peripheral = gpio & WD_GPIO_PERIPHERAL_MSK;
    }

    if (dir == WD_GPIO_DIR_IGNORE) {
        dir = gpio & WD_GPIO_DIR_MSK;
    }

    if (type == WD_GPIO_TYPE_IGNORE) {
        type = gpio & WD_GPIO_TYPE_MSK;
    }

    if (speed == WD_GPIO_SPEED_IGNORE) {
        speed = gpio & WD_GPIO_SPEED_MSK;
    }

    if (pull == WD_GPIO_PULL_IGNORE) {
        pull = gpio & WD_GPIO_PULL_MSK;
    }


    if (function == (WD_GPIO_FUNCTION_AUTO >> WD_GPIO_FUNCTION_POS)) {
        if (dir == WD_GPIO_DIR_PERIPHERAL) {
            function = (gpio & WD_GPIO_FUNC_PERI_MSK) >> WD_GPIO_FUNC_PERI_POS;
        } else {
            function = (gpio & WD_GPIO_FUNC_GPIO_MSK) >> WD_GPIO_FUNC_GPIO_POS;
        }
    }

    gpio_initstruct.Pin = pin;

    switch (dir) {
    case WD_GPIO_DIR_INPUT:
        if (type == WD_GPIO_TYPE_FLOATING) {
            gpio_initstruct.Mode = GPIO_MODE_INPUT;
        } else if (type == WD_GPIO_TYPE_ANALOG) {
            gpio_initstruct.Mode = GPIO_MODE_ANALOG;
        } else if (type > WD_GPIO_INTERRUPT_IGNORE) {
            IRQn_Type gpio_irqn = EXTI0_IRQn;

            switch (type) {
            case WD_GPIO_INTERRUPT_RISING_EDGE:
                gpio_initstruct.Mode = GPIO_MODE_IT_RISING;
                break;

            case WD_GPIO_INTERRUPT_FALLING_EDGE:
                gpio_initstruct.Mode = GPIO_MODE_IT_FALLING;
                break;

            case WD_GPIO_INTERRUPT_BOTH_EDGE:
                gpio_initstruct.Mode = GPIO_MODE_IT_RISING_FALLING;
                break;
            default:
                break;
            }
            switch (pin) {
            case GPIO_PIN_0:
                gpio_irqn = EXTI0_IRQn;
                break;
            case GPIO_PIN_1:
                gpio_irqn = EXTI1_IRQn;
                break;
            case GPIO_PIN_2:
                gpio_irqn = EXTI2_IRQn;
                break;
            case GPIO_PIN_3:
                gpio_irqn = EXTI3_IRQn;
                break;
            case GPIO_PIN_4:
                gpio_irqn = EXTI4_IRQn;
                break;
            case GPIO_PIN_5:
            case GPIO_PIN_6:
            case GPIO_PIN_7:
            case GPIO_PIN_8:
            case GPIO_PIN_9:
                gpio_irqn = EXTI9_5_IRQn;
                break;
            case GPIO_PIN_10:
            case GPIO_PIN_11:
            case GPIO_PIN_12:
            case GPIO_PIN_13:
            case GPIO_PIN_14:
            case GPIO_PIN_15:
                gpio_irqn = EXTI15_10_IRQn;
                break;
            default:
                break;
            }
            NVIC_SetPriority(gpio_irqn,
                             NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
                                                 WD_INTERRUPT_PREEMPT_PRIORITY_NORMAL_HANDLER,
                                                 WD_INTERRUPT_SUB_PRIORITY_NORMAL_HANDLER));
            NVIC_EnableIRQ(gpio_irqn);
        }
        break;

    case WD_GPIO_DIR_OUTPUT:
        if (type == WD_GPIO_TYPE_PUSH_PULL) {
            gpio_initstruct.Mode = GPIO_MODE_OUTPUT_PP;
            if (value != WD_GPIO_IGNORE) {
                wd_gpio_write(gpio, value << WD_GPIO_VALUE_POS);
            }
        } else if (type == WD_GPIO_TYPE_OPEN_DRAIN) {
            gpio_initstruct.Mode = GPIO_MODE_OUTPUT_OD;
            if (value != WD_GPIO_IGNORE) {
                wd_gpio_write(gpio, value << WD_GPIO_VALUE_POS);
            }
        }
        break;

    case WD_GPIO_DIR_PERIPHERAL:
        if (type == WD_GPIO_TYPE_PUSH_PULL) {
            gpio_initstruct.Mode = GPIO_MODE_AF_PP;
        } else if (type == WD_GPIO_TYPE_OPEN_DRAIN) {
            gpio_initstruct.Mode = GPIO_MODE_AF_OD;
        }

        if (dir == WD_GPIO_DIR_PERIPHERAL) {
            gpio_initstruct.Alternate = function;
        }
        break;

    default:
        return (WD_CODE_ERROR);
    }

    switch (pull) {
    case WD_GPIO_PULL_IGNORE:
    case WD_GPIO_PULL_NONE:
        gpio_initstruct.Pull = GPIO_NOPULL;
        break;

    case WD_GPIO_PULL_UP:
        gpio_initstruct.Pull = GPIO_PULLUP;
        break;

    case WD_GPIO_PULL_DOWN:
        gpio_initstruct.Pull = GPIO_PULLDOWN;
        break;
    default:
        break;
    }

    switch (speed) {
    case WD_GPIO_SPEED_LOW:
        gpio_initstruct.Speed = GPIO_SPEED_FREQ_LOW;
        break;

    case WD_GPIO_SPEED_DEFAULT:
        gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;
        break;

    case WD_GPIO_SPEED_HIGH:
        gpio_initstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        break;
    default:
        break;
    }

    if (ret == WD_CODE_OK) {
        HAL_GPIO_Init(reg, &gpio_initstruct);
    } else {
        // 에러 처리
    }

    return (ret);
}

#ifndef RTE_PLATFORM_GPIO_INLINE_FUNCTION
/***************************************************************************/ /**
 * \param     [in]  port          \ref wd_gpio_port_t
 * \param     [in]  pin           \ref wd_gpio_pin_t
 * \param     [in]  value         \ref wd_gpio_t
 ******************************************************************************/
void wd_gpio_write(uint32_t gpio, uint32_t value) {
    GPIO_TypeDef *reg = Get_GPIO_Port((gpio & WD_GPIO_PORT_MSK) >> WD_GPIO_PORT_POS);
    uint32_t pin = 1 << ((gpio & WD_GPIO_PIN_MSK) >> WD_GPIO_PIN_POS);
    uint32_t onoff = gpio & WD_GPIO_0N_OFF_MSK;

    if (reg == NULL) {
        return;
    }

    if (value == WD_GPIO_SET) {
        reg->BSRR = pin;
#if 0
#ifndef NDEBUG
    if((reg->ODR & pin) != pin)
    {
      WD_BREAK();
    }
#endif
#endif
    } else if (value == WD_GPIO_CLEAR) {
        reg->BSRR = pin << 16U;
#if 0
#ifndef NDEBUG
    if((reg->ODR & pin) != 0)
    {
      WD_BREAK();
    }
#endif
#endif
    } else if (value == WD_GPIO_TOGGLE) {
        reg->ODR ^= pin;
    } else if (value == WD_GPIO_ON) {
        if (onoff == WD_GPIO_ON_SET_OFF_CLEAR) {
            reg->BSRR = pin;
        } else if (onoff == WD_GPIO_ON_CLEAR_OFF_SET) {
            reg->BSRR = pin << 16U;
        }

    } else if (value == WD_GPIO_OFF) {
        if (onoff == WD_GPIO_ON_SET_OFF_CLEAR) {
            reg->BSRR = pin << 16U;
        } else if (onoff == WD_GPIO_ON_CLEAR_OFF_SET) {
            reg->BSRR = pin;
        }
    }
}

/***************************************************************************/ /**
 * \param     [in]  port          \ref wd_gpio_port_t
 * \param     [in]  pin           \ref wd_gpio_pin_t
 * \return          0 = WD_GPIO_CLEAR, 1 = WD_GPIO_SET
 ******************************************************************************/
int32_t wd_gpio_read(uint32_t gpio) {
    GPIO_TypeDef *reg = Get_GPIO_Port((gpio & WD_GPIO_PORT_MSK) >> WD_GPIO_PORT_POS);
    uint32_t pin = 1 << ((gpio & WD_GPIO_PIN_MSK) >> WD_GPIO_PIN_POS);

    if (reg == NULL) {
        return (WD_CODE_ERROR);
    }

    if ((reg->IDR & pin) != 0) {
        return (WD_GPIO_SET);
    } else {
        return (WD_GPIO_CLEAR);
    }
}
#endif
