/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_print_hal.c
 *
 *  Created on: Jan 3, 2025
 *      Author: doubled
 */


/* Private includes ----------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "_wd_print_hal.h"
#include "wd.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


//#define USE_UART_DMA
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */
/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
#if RTE_PLATFORM_PRINT_HAL_UART1
wd_ringbuf_t *wd_print_hal_uart1_rx_ring; //	CONSOLE
#endif
#if RTE_PLATFORM_PRINT_HAL_UART2
wd_ringbuf_t *gp_Uart2_RxRing; //	BLE
#endif
#if RTE_PLATFORM_PRINT_HAL_UART3
wd_ringbuf_t *gp_Uart3_RxRing; //
#endif
#if RTE_PLATFORM_PRINT_HAL_UART4
wd_ringbuf_t *gp_Uart4_RxRing; // 	WIFI
#endif
#if RTE_PLATFORM_PRINT_HAL_UART5
wd_ringbuf_t *wd_print_hal_uart5_rx_ring; //	RS232
#endif
#if RTE_PLATFORM_PRINT_HAL_UART6
wd_ringbuf_t *gp_Uart6_RxRing; //
#endif
#if RTE_PLATFORM_PRINT_HAL_UART7
wd_ringbuf_t *gp_Uart7_RxRing; //	DS18B20
#endif
#if RTE_PLATFORM_PRINT_HAL_UART8
wd_ringbuf_t *wd_print_hal_uart8_rx_ring; //
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
#if RTE_PLATFORM_PRINT_HAL_UART1
uint8_t wd_print_hal_comm1_buffer[WD_UART_COMM_QUE_SIZE];
static wd_ringbuf_t g_comm1_rx_ring;
#endif
#if RTE_PLATFORM_PRINT_HAL_UART2
uint8_t g_comm2_buffer[WD_UART_COMM_QUE_SIZE];
static wd_ringbuf_t g_comm2_rx_ring;
#endif
#if RTE_PLATFORM_PRINT_HAL_UART3
uint8_t g_comm3_buffer[WD_UART_COMM_QUE_SIZE];
static wd_ringbuf_t g_comm3_rx_ring;
#endif
#if RTE_PLATFORM_PRINT_HAL_UART4
uint8_t g_comm4_buffer[WD_UART_COMM_QUE_SIZE];
static wd_ringbuf_t g_comm4_rx_ring;
#endif
#if RTE_PLATFORM_PRINT_HAL_UART5
uint8_t wd_print_hal_comm5_buffer[WD_UART_COMM_QUE_SIZE];
static wd_ringbuf_t g_comm5_rx_ring;
#endif
#if RTE_PLATFORM_PRINT_HAL_UART6
uint8_t g_comm6_buffer[WD_UART_COMM_QUE_SIZE];
static wd_ringbuf_t g_comm6_rx_ring;
#endif
#if RTE_PLATFORM_PRINT_HAL_UART7
uint8_t g_comm7_buffer[WD_UART_COMM_QUE_SIZE];
static wd_ringbuf_t g_comm7_rx_ring;
#endif
#if RTE_PLATFORM_PRINT_HAL_UART8
uint8_t wd_print_hal_comm8_buffer[WD_UART8_COMM_QUE_SIZE] __attribute__((section(".RAM_D2")));
static wd_ringbuf_t g_comm8_rx_ring;
#endif
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#if 1 // 나중에 CMSIS-PACK 사용 유무로 활성화 비활성화 체크
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
#if RTE_PLATFORM_PRINT_HAL_UART1
    if (huart->Instance == USART1) {
        //최대값이면 하나 삭제한다. 꽉차있으면 오래된 것부터 삭제됨
        if (g_comm1_rx_ring.length >= WD_UART_COMM_QUE_SIZE) {
            g_comm1_rx_ring.read_idx =
                (uint16_t)((g_comm1_rx_ring.read_idx + 1) % WD_UART_COMM_QUE_SIZE);
            g_comm1_rx_ring.length -= 1;
        }
        g_comm1_rx_ring
            .buffer[(g_comm1_rx_ring.read_idx + g_comm1_rx_ring.length) % WD_UART_COMM_QUE_SIZE] =
            g_comm1_rx_ring.temp;
        g_comm1_rx_ring.length++;
        //UART 수신 대기 초기화
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART1_HANDLE, (uint8_t *)&g_comm1_rx_ring.temp,
                            sizeof(g_comm1_rx_ring.temp));
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART2
    if (huart->Instance == USART2) {
        //최대값이면 하나 삭제한다. 꽉차있으면 오래된 것부터 삭제됨
        if (g_comm2_rx_ring.length >= WD_UART_COMM_QUE_SIZE) {
            g_comm2_rx_ring.read_idx = (g_comm2_rx_ring.read_idx + 1) % WD_UART_COMM_QUE_SIZE;
            g_comm2_rx_ring.length -= 1;
        }
        g_comm2_rx_ring
            .buffer[(g_comm2_rx_ring.read_idx + g_comm2_rx_ring.length) % WD_UART_COMM_QUE_SIZE] =
            g_comm2_rx_ring.temp;
        g_comm2_rx_ring.length++;
        //UART 수신 대기 초기화
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART2_HANDLE, (uint8_t *)&g_comm2_rx_ring.temp,
                            sizeof(g_comm2_rx_ring.temp));
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART3
    if (huart->Instance == USART3) {
        //최대값이면 하나 삭제한다. 꽉차있으면 오래된 것부터 삭제됨
        if (g_comm3_rx_ring.length >= WD_UART_COMM_QUE_SIZE) {
            g_comm3_rx_ring.read_idx = (g_comm3_rx_ring.read_idx + 1) % WD_UART_COMM_QUE_SIZE;
            g_comm3_rx_ring.length -= 1;
        }
        g_comm3_rx_ring
            .buffer[(g_comm3_rx_ring.read_idx + g_comm3_rx_ring.length) % WD_UART_COMM_QUE_SIZE] =
            g_comm3_rx_ring.temp;
        g_comm3_rx_ring.length++;
        //UART 수신 대기 초기화
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART3_HANDLE, (uint8_t *)&g_comm3_rx_ring.temp,
                            sizeof(g_comm3_rx_ring.temp));
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART4
    if (huart->Instance == UART4) {
        //최대값이면 하나 삭제한다. 꽉차있으면 오래된 것부터 삭제됨
        if (g_comm4_rx_ring.length >= WD_UART_COMM_QUE_SIZE) {
            g_comm4_rx_ring.read_idx = (g_comm4_rx_ring.read_idx + 1) % WD_UART_COMM_QUE_SIZE;
            g_comm4_rx_ring.length -= 1;
        }
        g_comm4_rx_ring
            .buffer[(g_comm4_rx_ring.read_idx + g_comm4_rx_ring.length) % WD_UART_COMM_QUE_SIZE] =
            g_comm4_rx_ring.temp;
        g_comm4_rx_ring.length++;
        //UART 수신 대기 초기화
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART4_HANDLE, (uint8_t *)&g_comm4_rx_ring.temp,
                            sizeof(g_comm4_rx_ring.temp));
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART5
    if (huart->Instance == UART5) {
        //최대값이면 하나 삭제한다. 꽉차있으면 오래된 것부터 삭제됨
        if (g_comm5_rx_ring.length >= WD_UART_COMM_QUE_SIZE) {
            g_comm5_rx_ring.read_idx =
                (uint16_t)((g_comm5_rx_ring.read_idx + 1) % WD_UART_COMM_QUE_SIZE);
            g_comm5_rx_ring.length -= 1;
        }
        g_comm5_rx_ring
            .buffer[(g_comm5_rx_ring.read_idx + g_comm5_rx_ring.length) % WD_UART_COMM_QUE_SIZE] =
            g_comm5_rx_ring.temp;
        g_comm5_rx_ring.length++;
        //UART 수신 대기 초기화
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART5_HANDLE, (uint8_t *)&g_comm5_rx_ring.temp,
                            sizeof(g_comm5_rx_ring.temp));
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART6
    if (huart->Instance == UART6) {
        //최대값이면 하나 삭제한다. 꽉차있으면 오래된 것부터 삭제됨
        if (g_comm6_rx_ring.length >= WD_UART_COMM_QUE_SIZE) {
            g_comm6_rx_ring.read_idx = (g_comm6_rx_ring.read_idx + 1) % WD_UART_COMM_QUE_SIZE;
            g_comm6_rx_ring.length -= 1;
        }
        g_comm6_rx_ring
            .buffer[(g_comm6_rx_ring.read_idx + g_comm6_rx_ring.length) % WD_UART_COMM_QUE_SIZE] =
            g_comm6_rx_ring.temp;
        g_comm6_rx_ring.length++;
        //UART 수신 대기 초기화
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART6_HANDLE, (uint8_t *)&g_comm6_rx_ring.temp,
                            sizeof(g_comm6_rx_ring.temp));
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART7
    if (huart->Instance == UART7) {
        //최대값이면 하나 삭제한다. 꽉차있으면 오래된 것부터 삭제됨
        if (g_comm7_rx_ring.length >= WD_UART_COMM_QUE_SIZE) {
            g_comm7_rx_ring.read_idx = (g_comm7_rx_ring.read_idx + 1) % WD_UART_COMM_QUE_SIZE;
            g_comm7_rx_ring.length -= 1;
        }
        g_comm7_rx_ring
            .buffer[(g_comm7_rx_ring.read_idx + g_comm5_rx_ring.length) % WD_UART_COMM_QUE_SIZE] =
            g_comm7_rx_ring.temp;
        g_comm7_rx_ring.length++;
        //UART 수신 대기 초기화
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART7_HANDLE, (uint8_t *)&g_comm7_rx_ring.temp,
                            sizeof(g_comm7_rx_ring.temp));
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART8
    if (huart->Instance == UART8) {
        if (g_comm8_rx_ring.length >= WD_UART8_COMM_QUE_SIZE) {
            g_comm8_rx_ring.read_idx =
                (uint16_t)((g_comm8_rx_ring.read_idx + 1) % WD_UART8_COMM_QUE_SIZE);
            g_comm8_rx_ring.length -= 1;
        }
        g_comm8_rx_ring
            .buffer[(g_comm8_rx_ring.read_idx + g_comm8_rx_ring.length) % WD_UART8_COMM_QUE_SIZE] =
            g_comm8_rx_ring.temp;
        g_comm8_rx_ring.length++;
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART8_HANDLE, (uint8_t *)&g_comm8_rx_ring.temp,
                            sizeof(g_comm8_rx_ring.temp));
    }
#endif
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
#if RTE_PLATFORM_PRINT_HAL_UART1
    if (huart->Instance == USART1) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART2
    if (huart->Instance == USART2) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART3
    if (huart->Instance == USART3) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART4
    if (huart->Instance == UART4) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART5
    if (huart->Instance == UART5) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        __HAL_UART_CLEAR_PEFLAG(huart);
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART5_HANDLE, (uint8_t *)&g_comm5_rx_ring.temp,
                            sizeof(g_comm5_rx_ring.temp));
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART6
    if (huart->Instance == UART6) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART7
    if (huart->Instance == UART7) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
#endif
#if RTE_PLATFORM_PRINT_HAL_UART8
    if (huart->Instance == UART8) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        __HAL_UART_CLEAR_PEFLAG(huart);
        HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART8_HANDLE, (uint8_t *)&g_comm8_rx_ring.temp,
                            sizeof(g_comm8_rx_ring.temp));
    }
#endif
}
#endif
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
#if RTE_PLATFORM_PRINT_HAL_UART1
/* core console input source : UART1 receive ring buffer (non-blocking) */
static int32_t uart1_console_getc(void) {
    int32_t ch = -1;

    if (wd_print_hal_uart1_size() > 0U) {
        ch = (int32_t)(uint8_t)wd_ringbuf_pop(wd_print_hal_uart1_rx_ring);
    }
    return ch;
}
#endif

extern void wd_print_hal_init_app_uart(void) {
    //## Setting buffer
    // Init Ringbuffer
#if RTE_PLATFORM_PRINT_HAL_UART1
    wd_ringbuf_init(&g_comm1_rx_ring, wd_print_hal_comm1_buffer, sizeof(wd_print_hal_comm1_buffer));
    wd_print_hal_uart1_rx_ring = &g_comm1_rx_ring;
    wd_print_hal_clear_uart1();
    HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART1_HANDLE, (uint8_t *)&g_comm1_rx_ring.temp,
                        sizeof(g_comm1_rx_ring.temp));
    (void)wd_print_register_output(wd_print_hal_uart1_put_char); /* UART1 = console output */
    (void)wd_print_register_input(uart1_console_getc);           /* UART1 = console input  */
#endif
#if RTE_PLATFORM_PRINT_HAL_UART2
    wd_ringbuf_init(&g_comm2_rx_ring, g_comm2_buffer, sizeof(g_comm2_buffer));
    gp_Uart2_RxRing = &g_comm2_rx_ring;
    Uart2_Clear();
    HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART2_HANDLE, (uint8_t *)&g_comm2_rx_ring.temp,
                        sizeof(g_comm2_rx_ring.temp));
#endif
#if RTE_PLATFORM_PRINT_HAL_UART3
    wd_ringbuf_init(&g_comm3_rx_ring, g_comm3_buffer, sizeof(g_comm3_buffer));
    gp_Uart3_RxRing = &g_comm3_rx_ring;
    Uart3_Clear();
    HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART3_HANDLE, (uint8_t *)&g_comm3_rx_ring.temp,
                        sizeof(g_comm3_rx_ring.temp));
#endif
#if RTE_PLATFORM_PRINT_HAL_UART4
    wd_ringbuf_init(&g_comm4_rx_ring, g_comm4_buffer, sizeof(g_comm4_buffer));
    gp_Uart4_RxRing = &g_comm4_rx_ring;
    Uart4_Clear();
    HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART4_HANDLE, (uint8_t *)&g_comm4_rx_ring.temp,
                        sizeof(g_comm4_rx_ring.temp));
#endif
#if RTE_PLATFORM_PRINT_HAL_UART5
    wd_ringbuf_init(&g_comm5_rx_ring, wd_print_hal_comm5_buffer, sizeof(wd_print_hal_comm5_buffer));
    wd_print_hal_uart5_rx_ring = &g_comm5_rx_ring;
    wd_print_hal_clear_uart5();
    HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART5_HANDLE, (uint8_t *)&g_comm5_rx_ring.temp,
                        sizeof(g_comm5_rx_ring.temp));
#endif
#if RTE_PLATFORM_PRINT_HAL_UART6
    wd_ringbuf_init(&g_comm6_rx_ring, g_comm6_buffer, sizeof(g_comm6_buffer));
    gp_Uart6_RxRing = &g_comm6_rx_ring;
    Uart6_Clear();
    HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART6_HANDLE, (uint8_t *)&g_comm6_rx_ring.temp,
                        sizeof(g_comm6_rx_ring.temp));
#endif
#if RTE_PLATFORM_PRINT_HAL_UART7
    wd_ringbuf_init(&g_comm7_rx_ring, g_comm7_buffer, sizeof(g_comm7_buffer));
    gp_Uart7_RxRing = &g_comm7_rx_ring;
    Uart7_Clear();
    HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART7_HANDLE, (uint8_t *)&g_comm7_rx_ring.temp,
                        sizeof(g_comm7_rx_ring.temp));
#endif
#if RTE_PLATFORM_PRINT_HAL_UART8
    wd_ringbuf_init(&g_comm8_rx_ring, wd_print_hal_comm8_buffer, sizeof(wd_print_hal_comm8_buffer));
    wd_print_hal_uart8_rx_ring = &g_comm8_rx_ring;
    wd_print_hal_clear_uart8();
    HAL_UART_Receive_IT(RTE_PLATFORM_PRINT_HAL_UART8_HANDLE, (uint8_t *)&g_comm8_rx_ring.temp,
                        sizeof(g_comm8_rx_ring.temp));
#endif
}
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
/***************************************************************************************************************************************************************
 **********  UART1
 **************************************************************************************************************************************************************/
#if RTE_PLATFORM_PRINT_HAL_UART1
extern void wd_print_hal_uart1_put_string(char *_buffer) {
    while ((*_buffer) != '\0') {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART1_HANDLE, (const uint8_t *)_buffer, 1, 0xFFFF);
        ++_buffer;
    }
    //HAL_Delay(1);
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
extern void wd_print_hal_uart1_put_array(uint8_t *_buffer, int16_t _length) {
    while (_length--) {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART1_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
}
extern void wd_print_hal_uart1_dma_put_array(uint8_t *_buffer, int16_t _length) {
    HAL_UART_Transmit_DMA(RTE_PLATFORM_PRINT_HAL_UART1_HANDLE, _buffer, (uint16_t)_length);
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
extern uint16_t wd_print_hal_uart1_size(void) {
    return wd_print_hal_uart1_rx_ring->length;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
extern void wd_print_hal_clear_uart1(void) {
    wd_ringbuf_clear(wd_print_hal_uart1_rx_ring);
}
/* USER CODE END 5 */

/* USER CODE BEGIN 6 */
extern char wd_print_hal_uart1_get_char(void) {
    if (wd_print_hal_uart1_size() > 0) {
        return wd_ringbuf_pop(wd_print_hal_uart1_rx_ring);
    }

    /* Telnet input : registered separately to the core console (_wd_eth_telnet.c) */
    return 0;
}
/* USER CODE END 6 */

/* USER CODE BEGIN 7 */
extern void wd_print_hal_uart1_put_char(uint8_t _ch) {
    uint8_t buf[1];

    buf[0] = (uint8_t)_ch;
#if 0
	HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART1_HANDLE, _ch, 1, 0xFFFF);
#else
    WD_WAIT_WHILE((!LL_USART_IsActiveFlag_TXE(USART1)), 10000);
    LL_USART_TransmitData8(USART1, buf[0]);
#endif
    /* Telnet output : registered separately to the core console (_wd_eth_telnet.c) */
}
/* USER CODE END 7 */

/* USER CODE BEGIN 8 */
extern char wd_print_hal_uart1_wait_char(void) {
    //  char data;

    while (true) {
        if (wd_print_hal_uart1_size() > 0) {
            return wd_ringbuf_pop(wd_print_hal_uart1_rx_ring);
        }
        //        wd_time_wait_ms(1);
        osThreadYield();
    }
}
/* USER CODE END 8 */
extern bool wd_print_hal_uart1_is_rx_char(const char _ch) {
    if (wd_print_hal_uart1_size() > 0) {
        if (wd_print_hal_uart1_get_char() == _ch) {
            return true;
        }
    }
    return false;
}

#if 1
//이건 디버그 용임
extern bool wd_print_hal_uart1_input_string(char *_buffer, uint16_t _buf_size) {
    char ch;
    int idx;

    idx = 0;
    memset(_buffer, 0x00, _buf_size);
    wd_print_hal_clear_uart1();
    while (true) {
        ch = wd_print_hal_uart1_get_char();
        if (ch != 0) {
            _buffer[idx] = ch;
            if (ch == '\n' || ch == '\r') {
                _buffer[idx] = '\0';
                break;
            }
            mprint("%c", ch);
            idx++;
        }
    }

    mprint("\r\n");
    if (idx >= 1) {
        return true;
    }
    return false;
}

extern void wd_print_hal_print_uart1(char *_fmt, ...) {
    char buffer[2048];
    va_list args;
    va_start(args, _fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, _fmt, args);
    va_end(args);

    wd_print_hal_uart1_put_string(buffer);
}
#endif
#endif
/***************************************************************************************************************************************************************
 **********  UART2
 **************************************************************************************************************************************************************/
#if RTE_PLATFORM_PRINT_HAL_UART2
/* USER CODE BEGIN 2 */
extern void Uart2_PutString(char *_buffer) {
    while ((*_buffer) != '\0') {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART2_HANDLE, (const uint8_t *)_buffer, 1, 0xFFFF);
        ++_buffer;
    }
    //HAL_Delay(1);
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
extern void Uart2_PutArray(uint8_t *_buffer, int16_t _length) {
    while (_length--) {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART2_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
}
extern void Uart2_DMA_PutArray(uint8_t *_buffer, int16_t _length) {
    HAL_UART_Transmit_DMA(RTE_PLATFORM_PRINT_HAL_UART2_HANDLE, _buffer, _length);
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
extern uint16_t Uart2_Size(void) {
    return gp_Uart2_RxRing->length;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
extern void Uart2_Clear(void) {
    wd_ringbuf_clear(gp_Uart2_RxRing);
}
/* USER CODE END 5 */

/* USER CODE BEGIN 6 */
extern char Uart2_GetChar(void) {
    if (Uart2_Size() > 0) {
        return wd_ringbuf_pop(gp_Uart2_RxRing);
    }
    return 0;
}
/* USER CODE END 6 */

/* USER CODE BEGIN 7 */
extern void Uart2_PutChar(uint8_t _ch) {
    HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART2_HANDLE, _ch, 1, 0xFFFF);
}
/* USER CODE END 7 */

/* USER CODE BEGIN 8 */
extern char Uart2_WaitChar(void) {
    while (true) {
        if (Uart2_Size() > 0) {
            return wd_ringbuf_pop(gp_Uart2_RxRing);
        }
        wd_time_wait_ms(5);
    }
}
/* USER CODE END 8 */

extern bool Uart2_IsRxChar(const char _ch) {
    if (Uart2_Size() > 0) {
        if (Uart2_GetChar() == _ch) {
            return true;
        }
    }
    return false;
}

#if 0
//이건 디버그 용임 통신 입력이 없으면 비활성화 시켜야 함. 왜냐하면 mprint를 원하는 출력으로 바꿔줘야 하기 때문
extern bool Uart2_InputString(char* _buffer, uint16_t _buf_size)
{
    char ch;
    int  idx;

    idx = 0;
    memset(_buffer, 0x00, _buf_size);
    Uart2_Clear();
    while(true)
    {
        ch = Uart2_GetChar();
        if(ch != 0)
        {
            _buffer[idx] = ch;
            if(ch == '\n' || ch == '\r')
            {
                _buffer[idx] = '\0';
                break;
            }
            dprint("%c", ch);
            idx++;
        }
    }

    dprint("\r\n");
    if(idx >= 1)
    {
        return true;
    }
    return false;
}

extern void Uart2_Print(char* _fmt, ... )
{
	char buffer[2048];
    va_list args;
    va_start( args, _fmt );
    vsnprintf( buffer, sizeof(buffer)-1, _fmt, args );
    va_end( args );

    Uart2_PutString(buffer);
}
#endif
#endif
/***************************************************************************************************************************************************************
 **********  UART3
 **************************************************************************************************************************************************************/
#if RTE_PLATFORM_PRINT_HAL_UART3
/* USER CODE BEGIN 2 */
extern void Uart3_PutString(char *_buffer) {
    while ((*_buffer) != '\0') {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART3_HANDLE, (const uint8_t *)_buffer, 1, 0xFFFF);
        ++_buffer;
    }
    //HAL_Delay(1);
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
extern void Uart3_PutArray(uint8_t *_buffer, int16_t _length) {
    while (_length--) {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART3_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
}
extern void Uart3_DMA_PutArray(uint8_t *_buffer, int16_t _length) {
    HAL_UART_Transmit_DMA(RTE_PLATFORM_PRINT_HAL_UART3_HANDLE, _buffer, _length);
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
extern uint16_t Uart3_Size(void) {
    return gp_Uart3_RxRing->length;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
extern void Uart3_Clear(void) {
    wd_ringbuf_clear(gp_Uart3_RxRing);
}
/* USER CODE END 5 */

/* USER CODE BEGIN 6 */
extern char Uart3_GetChar(void) {
    if (Uart3_Size() > 0) {
        return wd_ringbuf_pop(gp_Uart3_RxRing);
    }
    return 0;
}
/* USER CODE END 6 */

/* USER CODE BEGIN 7 */
extern void Uart3_PutChar(uint8_t _ch) {
    HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART3_HANDLE, _ch, 1, 0xFFFF);
}
/* USER CODE END 7 */

/* USER CODE BEGIN 8 */
extern char Uart3_WaitChar(void) {
    while (true) {
        if (Uart3_Size() > 0) {
            return wd_ringbuf_pop(gp_Uart3_RxRing);
        }
        wd_time_wait_ms(5);
    }
}
/* USER CODE END 8 */

extern bool Uart3_IsRxChar(const char _ch) {
    if (Uart3_Size() > 0) {
        if (Uart3_GetChar() == _ch) {
            return true;
        }
    }
    return false;
}

#if 0
//이건 디버그 용임 통신 입력이 없으면 비활성화 시켜야 함. 왜냐하면 mprint를 원하는 출력으로 바꿔줘야 하기 때문
extern bool Uart3_InputString(char* _buffer, uint16_t _buf_size)
{
    char ch;
    int  idx;

    idx = 0;
    memset(_buffer, 0x00, _buf_size);
    Uart3_Clear();
    while(true)
    {
        ch = Uart3_GetChar();
        if(ch != 0)
        {
            _buffer[idx] = ch;
            if(ch == '\n' || ch == '\r')
            {
                _buffer[idx] = '\0';
                break;
            }
            mprint("%c", ch);
            idx++;
        }
    }

    mprint("\r\n");
    if(idx >= 1)
    {
        return true;
    }
    return false;
}

extern void Uart3_Print(char* _fmt, ... )
{
	char buffer[2048];
    va_list args;
    va_start( args, _fmt );
    vsnprintf( buffer, sizeof(buffer)-1, _fmt, args );
    va_end( args );

    Uart3_PutString(buffer);
}
#endif
#endif
/***************************************************************************************************************************************************************
 **********  UART4
 **************************************************************************************************************************************************************/
#if RTE_PLATFORM_PRINT_HAL_UART4
/* USER CODE BEGIN 2 */
extern void Uart4_PutString(char *_buffer) {
    while ((*_buffer) != '\0') {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART4_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
    //HAL_Delay(1);
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
extern void Uart4_PutArray(uint8_t *_buffer, int16_t _length) {
    while (_length--) {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART4_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
}
extern void Uart4_DMA_PutArray(uint8_t *_buffer, int16_t _length) {
    HAL_UART_Transmit_DMA(RTE_PLATFORM_PRINT_HAL_UART4_HANDLE, _buffer, _length);
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
extern uint16_t Uart4_Size(void) {
    return gp_Uart4_RxRing->length;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
extern void Uart4_Clear(void) {
    wd_ringbuf_clear(gp_Uart4_RxRing);
}
/* USER CODE END 5 */

/* USER CODE BEGIN 6 */
extern char Uart4_GetChar(void) {
    if (Uart4_Size() > 0) {
        return wd_ringbuf_pop(gp_Uart4_RxRing);
    }
    return 0;
}
/* USER CODE END 6 */

/* USER CODE BEGIN 7 */
extern void Uart4_PutChar(uint8_t _ch) {
    HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART4_HANDLE, _ch, 1, 0xFFFF);
}
/* USER CODE END 7 */

/* USER CODE BEGIN 8 */
extern char Uart4_WaitChar(void) {
    while (true) {
        if (Uart4_Size() > 0) {
            return wd_ringbuf_pop(gp_Uart4_RxRing);
        }
        wd_time_wait_ms(5);
    }
}
/* USER CODE END 8 */

extern bool Uart4_IsRxChar(const char _ch) {
    if (Uart4_Size() > 0) {
        if (Uart4_GetChar() == _ch) {
            return true;
        }
    }
    return false;
}

#if 0
//이건 디버그 용임 통신 입력이 없으면 비활성화 시켜야 함. 왜냐하면 mprint를 원하는 출력으로 바꿔줘야 하기 때문
extern bool Uart4_InputString(char* _buffer, uint16_t _buf_size)
{
    char ch;
    int  idx;

    idx = 0;
    memset(_buffer, 0x00, _buf_size);
    Uart4_Clear();
    while(true)
    {
        ch = Uart4_GetChar();
        if(ch != 0)
        {
            _buffer[idx] = ch;
            if(ch == '\n' || ch == '\r')
            {
                _buffer[idx] = '\0';
                break;
            }
            mprint("%c", ch);
            idx++;
        }
    }

    mprint("\r\n");
    if(idx >= 1)
    {
        return true;
    }
    return false;
}

extern void Uart4_Print(char* _fmt, ... )
{
	char buffer[2048];
    va_list args;
    va_start( args, _fmt );
    vsnprintf( buffer, sizeof(buffer)-1, _fmt, args );
    va_end( args );

    Uart4_PutString(buffer);
}
#endif
#endif
/***************************************************************************************************************************************************************
 **********  UART5
 **************************************************************************************************************************************************************/
#if RTE_PLATFORM_PRINT_HAL_UART5
/* USER CODE BEGIN 2 */
extern void wd_print_hal_uart5_put_string(char *_buffer) {
    while ((*_buffer) != '\0') {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART5_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
    //HAL_Delay(1);
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
extern void wd_print_hal_uart5_put_array(uint8_t *_buffer, int16_t _length) {
    while (_length--) {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART5_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
}
extern void wd_print_hal_uart5_dma_put_array(uint8_t *_buffer, int16_t _length) {
    HAL_UART_Transmit_DMA(RTE_PLATFORM_PRINT_HAL_UART5_HANDLE, _buffer, (uint16_t)_length);
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
extern uint16_t wd_print_hal_uart5_size(void) {
    return wd_print_hal_uart5_rx_ring->length;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
extern void wd_print_hal_clear_uart5(void) {
    wd_ringbuf_clear(wd_print_hal_uart5_rx_ring);
}
/* USER CODE END 5 */

/* USER CODE BEGIN 6 */
extern char wd_print_hal_uart5_get_char(void) {
    if (wd_print_hal_uart5_size() > 0) {
        return wd_ringbuf_pop(wd_print_hal_uart5_rx_ring);
    }
    return 0;
}
/* USER CODE END 6 */

/* USER CODE BEGIN 7 */
extern void wd_print_hal_uart5_put_char(uint8_t _ch) {
    HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART5_HANDLE, &_ch, 1, 0xFFFF);
}
/* USER CODE END 7 */

/* USER CODE BEGIN 8 */
extern char wd_print_hal_uart5_wait_char(void) {
    while (true) {
        if (wd_print_hal_uart5_size() > 0) {
            return wd_ringbuf_pop(wd_print_hal_uart5_rx_ring);
        }
        wd_time_wait_ms(5);
    }
}
/* USER CODE END 8 */

extern bool wd_print_hal_uart5_is_rx_char(const char _ch) {
    if (wd_print_hal_uart5_size() > 0) {
        if (wd_print_hal_uart5_get_char() == _ch) {
            return true;
        }
    }
    return false;
}

#if 0
//이건 디버그 용임 통신 입력이 없으면 비활성화 시켜야 함. 왜냐하면 mprint를 원하는 출력으로 바꿔줘야 하기 때문
extern bool Uart5_InputString(char* _buffer, uint16_t _buf_size)
{
    char ch;
    int  idx;

    idx = 0;
    memset(_buffer, 0x00, _buf_size);
    wd_print_hal_clear_uart5();
    while(true)
    {
        ch = wd_print_hal_uart5_get_char();
        if(ch != 0)
        {
            _buffer[idx] = ch;
            if(ch == '\n' || ch == '\r')
            {
                _buffer[idx] = '\0';
                break;
            }
            mprint("%c", ch);
            idx++;
        }
    }

    mprint("\r\n");
    if(idx >= 1)
    {
        return true;
    }
    return false;
}

extern void Uart5_Print(char* _fmt, ... )
{
	char buffer[2048];
    va_list args;
    va_start( args, _fmt );
    vsnprintf( buffer, sizeof(buffer)-1, _fmt, args );
    va_end( args );

    wd_print_hal_uart5_put_string(buffer);
}
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************************************************************************
 **********  UART6
 **************************************************************************************************************************************************************/
#if RTE_PLATFORM_PRINT_HAL_UART6
/* USER CODE BEGIN 2 */
extern void Uart6_PutString(char *_buffer) {
    while ((*_buffer) != '\0') {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART6_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
    //HAL_Delay(1);
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
extern void Uart6_PutArray(uint8_t *_buffer, int16_t _length) {
    while (_length--) {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART6_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
}
extern void Uart6_DMA_PutArray(uint8_t *_buffer, int16_t _length) {
    HAL_UART_Transmit_DMA(RTE_PLATFORM_PRINT_HAL_UART6_HANDLE, _buffer, _length);
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
extern uint16_t Uart6_Size(void) {
    return gp_Uart6_RxRing->length;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
extern void Uart6_Clear(void) {
    wd_ringbuf_clear(gp_Uart6_RxRing);
}
/* USER CODE END 5 */

/* USER CODE BEGIN 6 */
extern char Uart6_GetChar(void) {
    if (Uart6_Size() > 0) {
        return wd_ringbuf_pop(gp_Uart6_RxRing);
    }
    return 0;
}
/* USER CODE END 6 */

/* USER CODE BEGIN 7 */
extern void Uart6_PutChar(uint8_t _ch) {
    HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART6_HANDLE, _ch, 1, 0xFFFF);
}
/* USER CODE END 7 */

/* USER CODE BEGIN 8 */
extern char Uart6_WaitChar(void) {
    while (true) {
        if (Uart6_Size() > 0) {
            return wd_ringbuf_pop(gp_Uart6_RxRing);
        }
        wd_time_wait_ms(5);
    }
}
/* USER CODE END 8 */

extern bool Uart6_IsRxChar(const char _ch) {
    if (Uart6_Size() > 0) {
        if (Uart6_GetChar() == _ch) {
            return true;
        }
    }
    return false;
}

#if 0
//이건 디버그 용임 통신 입력이 없으면 비활성화 시켜야 함. 왜냐하면 mprint를 원하는 출력으로 바꿔줘야 하기 때문
extern bool Uart6_InputString(char* _buffer, uint16_t _buf_size)
{
    char ch;
    int  idx;

    idx = 0;
    memset(_buffer, 0x00, _buf_size);
    Uart6_Clear();
    while(true)
    {
        ch = Uart6_GetChar();
        if(ch != 0)
        {
            _buffer[idx] = ch;
            if(ch == '\n' || ch == '\r')
            {
                _buffer[idx] = '\0';
                break;
            }
            mprint("%c", ch);
            idx++;
        }
    }

    mprint("\r\n");
    if(idx >= 1)
    {
        return true;
    }
    return false;
}

extern void Uart6_Print(char* _fmt, ... )
{
	char buffer[2048];
    va_list args;
    va_start( args, _fmt );
    vsnprintf( buffer, sizeof(buffer)-1, _fmt, args );
    va_end( args );

    Uart6_PutString(buffer);
}
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************************************************************************
 **********  UART7
 **************************************************************************************************************************************************************/
#if RTE_PLATFORM_PRINT_HAL_UART7
/* USER CODE BEGIN 2 */
extern void Uart7_PutString(char *_buffer) {
    while ((*_buffer) != '\0') {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART7_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
    //HAL_Delay(1);
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
extern void Uart7_PutArray(uint8_t *_buffer, int16_t _length) {
    while (_length--) {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART7_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
}
extern void Uart7_DMA_PutArray(uint8_t *_buffer, int16_t _length) {
    HAL_UART_Transmit_DMA(RTE_PLATFORM_PRINT_HAL_UART7_HANDLE, _buffer, _length);
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
extern uint16_t Uart7_Size(void) {
    return gp_Uart7_RxRing->length;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
extern void Uart7_Clear(void) {
    wd_ringbuf_clear(gp_Uart7_RxRing);
}
/* USER CODE END 5 */

/* USER CODE BEGIN 6 */
extern char Uart7_GetChar(void) {
    if (Uart7_Size() > 0) {
        return wd_ringbuf_pop(gp_Uart7_RxRing);
    }
    return 0;
}
/* USER CODE END 6 */

/* USER CODE BEGIN 7 */
extern void Uart7_PutChar(uint8_t _ch) {
    HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART7_HANDLE, _ch, 1, 0xFFFF);
}
/* USER CODE END 7 */

/* USER CODE BEGIN 8 */
extern char Uart7_WaitChar(void) {
    while (true) {
        if (Uart7_Size() > 0) {
            return wd_ringbuf_pop(gp_Uart7_RxRing);
        }
        wd_time_wait_ms(5);
    }
}
/* USER CODE END 8 */

extern bool Uart7_IsRxChar(const char _ch) {
    if (Uart7_Size() > 0) {
        if (Uart7_GetChar() == _ch) {
            return true;
        }
    }
    return false;
}

#if 0
//이건 디버그 용임 통신 입력이 없으면 비활성화 시켜야 함. 왜냐하면 mprint를 원하는 출력으로 바꿔줘야 하기 때문
extern bool Uart7_InputString(char* _buffer, uint16_t _buf_size)
{
    char ch;
    int  idx;

    idx = 0;
    memset(_buffer, 0x00, _buf_size);
    Uart7_Clear();
    while(true)
    {
        ch = Uart7_GetChar();
        if(ch != 0)
        {
            _buffer[idx] = ch;
            if(ch == '\n' || ch == '\r')
            {
                _buffer[idx] = '\0';
                break;
            }
            mprint("%c", ch);
            idx++;
        }
    }

    mprint("\r\n");
    if(idx >= 1)
    {
        return true;
    }
    return false;
}

extern void Uart7_Print(char* _fmt, ... )
{
	char buffer[2048];
    va_list args;
    va_start( args, _fmt );
    vsnprintf( buffer, sizeof(buffer)-1, _fmt, args );
    va_end( args );

    Uart7_PutString(buffer);
}
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************************************************************************
 **********  UART8
 **************************************************************************************************************************************************************/
#if RTE_PLATFORM_PRINT_HAL_UART8
/* USER CODE BEGIN 2 */
extern void wd_print_hal_uart8_put_string(char *_buffer) {
    while ((*_buffer) != '\0') {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART8_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
    //HAL_Delay(1);
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
extern void wd_print_hal_uart8_put_array(uint8_t *_buffer, int16_t _length) {
    while (_length--) {
        HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART8_HANDLE, (const uint8_t *)_buffer, 1,
                          0xFFFFFF);
        ++_buffer;
    }
}
extern void wd_print_hal_uart8_dma_put_array(uint8_t *_buffer, int16_t _length) {
    HAL_UART_Transmit_DMA(RTE_PLATFORM_PRINT_HAL_UART8_HANDLE, _buffer, (uint16_t)_length);
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
extern uint16_t wd_print_hal_uart8_size(void) {
    return wd_print_hal_uart8_rx_ring->length;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
extern void wd_print_hal_clear_uart8(void) {
    wd_ringbuf_clear(wd_print_hal_uart8_rx_ring);
}
/* USER CODE END 5 */

/* USER CODE BEGIN 6 */
extern char wd_print_hal_uart8_get_char(void) {
    if (wd_print_hal_uart8_size() > 0) {
        return wd_ringbuf_pop(wd_print_hal_uart8_rx_ring);
    }
    return 0;
}
/* USER CODE END 6 */

/* USER CODE BEGIN 7 */
extern void wd_print_hal_uart8_put_char(uint8_t _ch) {
    HAL_UART_Transmit(RTE_PLATFORM_PRINT_HAL_UART8_HANDLE, &_ch, 1, 0xFFFF);
}
/* USER CODE END 7 */

/* USER CODE BEGIN 8 */
extern char wd_print_hal_uart8_wait_char(void) {
    while (true) {
        if (wd_print_hal_uart8_size() > 0) {
            return wd_ringbuf_pop(wd_print_hal_uart8_rx_ring);
        }
        wd_time_wait_ms(5);
    }
}
/* USER CODE END 8 */

extern bool wd_print_hal_uart8_is_rx_char(const char _ch) {
    if (wd_print_hal_uart8_size() > 0) {
        if (wd_print_hal_uart8_get_char() == _ch) {
            return true;
        }
    }
    return false;
}

#if 0
//이건 디버그 용임 통신 입력이 없으면 비활성화 시켜야 함. 왜냐하면 mprint를 원하는 출력으로 바꿔줘야 하기 때문
extern bool Uart8_InputString(char* _buffer, uint16_t _buf_size)
{
    char ch;
    int  idx;

    idx = 0;
    memset(_buffer, 0x00, _buf_size);
    wd_print_hal_clear_uart8();
    while(true)
    {
        ch = wd_print_hal_uart8_get_char();
        if(ch != 0)
        {
            _buffer[idx] = ch;
            if(ch == '\n' || ch == '\r')
            {
                _buffer[idx] = '\0';
                break;
            }
            mprint("%c", ch);
            idx++;
        }
    }

    mprint("\r\n");
    if(idx >= 1)
    {
        return true;
    }
    return false;
}

extern void Uart8_Print(char* _fmt, ... )
{
	char buffer[2048];
    va_list args;
    va_start( args, _fmt );
    vsnprintf( buffer, sizeof(buffer)-1, _fmt, args );
    va_end( args );

    wd_print_hal_uart8_put_string(buffer);
}
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
