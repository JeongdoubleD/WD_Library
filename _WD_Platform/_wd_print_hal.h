/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_print_hal.h
 *
 *  Created on: Jan 3, 2025
 *      Author: doubled
 */

#ifndef WD_PLATFORM_WD_PRINT_HAL_H
#define WD_PLATFORM_WD_PRINT_HAL_H

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "wd.h"
#include "_wd_ringbuf.h"

/* board HAL mapping : default CubeMX names, override in wd_board.h */
#ifndef RTE_PLATFORM_PRINT_HAL_UART1_HANDLE
#define RTE_PLATFORM_PRINT_HAL_UART1_HANDLE (&huart1)
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART2_HANDLE
#define RTE_PLATFORM_PRINT_HAL_UART2_HANDLE (&huart2)
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART3_HANDLE
#define RTE_PLATFORM_PRINT_HAL_UART3_HANDLE (&huart3)
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART4_HANDLE
#define RTE_PLATFORM_PRINT_HAL_UART4_HANDLE (&huart4)
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART5_HANDLE
#define RTE_PLATFORM_PRINT_HAL_UART5_HANDLE (&huart5)
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART6_HANDLE
#define RTE_PLATFORM_PRINT_HAL_UART6_HANDLE (&huart6)
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART7_HANDLE
#define RTE_PLATFORM_PRINT_HAL_UART7_HANDLE (&huart7)
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART8_HANDLE
#define RTE_PLATFORM_PRINT_HAL_UART8_HANDLE (&huart8)
#endif
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#ifndef RTE_PLATFORM_PRINT_HAL_UART1
#define RTE_PLATFORM_PRINT_HAL_UART1 1
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART2
#define RTE_PLATFORM_PRINT_HAL_UART2 0
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART3
#define RTE_PLATFORM_PRINT_HAL_UART3 0
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART4
#define RTE_PLATFORM_PRINT_HAL_UART4 0
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART5
#define RTE_PLATFORM_PRINT_HAL_UART5 1
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART6
#define RTE_PLATFORM_PRINT_HAL_UART6 0
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART7
#define RTE_PLATFORM_PRINT_HAL_UART7 0
#endif
#ifndef RTE_PLATFORM_PRINT_HAL_UART8
#define RTE_PLATFORM_PRINT_HAL_UART8 1
#endif

#define WD_UART_COMM_QUE_SIZE (1024 * 1)
/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */
extern void wd_print_hal_init_app_uart(void);


#if RTE_PLATFORM_PRINT_HAL_UART1
/***************************************************************************************************************************************************************
 **********  UART1
 **************************************************************************************************************************************************************/
extern wd_ringbuf_t *wd_print_hal_uart1_rx_ring;
extern uint8_t wd_print_hal_comm1_buffer
    [WD_UART_COMM_QUE_SIZE]; //cf> 원래 내부에 숨겨두지만 디버깅 요인등의 이유로 빼둔다.

extern void wd_print_hal_uart1_put_string(char *_buffer);
extern void wd_print_hal_uart1_put_array(uint8_t *_buffer, int16_t _length);
extern void wd_print_hal_uart1_dma_put_array(uint8_t *_buffer, int16_t _length);
extern uint16_t wd_print_hal_uart1_size(void);
extern void wd_print_hal_clear_uart1(void);
extern char wd_print_hal_uart1_get_char(void);
extern void wd_print_hal_uart1_put_char(uint8_t _ch);
extern char wd_print_hal_uart1_wait_char(void);
extern bool wd_print_hal_uart1_is_rx_char(const char _ch);
#if 1
extern bool wd_print_hal_uart1_input_string(char *_buffer, uint16_t _buf_size);
extern void wd_print_hal_print_uart1(char *_fmt, ...);

/* console print routing (override in wd_config.h) */
#ifndef dprint
#define dprint(fmt, ...) wd_print_hal_print_uart1(fmt, ##__VA_ARGS__)
#endif
#ifndef mprint
#define mprint(fmt, ...) wd_print_hal_print_uart1(fmt, ##__VA_ARGS__)
#endif
#endif
#endif

#if RTE_PLATFORM_PRINT_HAL_UART2
/***************************************************************************************************************************************************************
 **********  UART2
 **************************************************************************************************************************************************************/
extern wd_ringbuf_t *gp_Uart2_RxRing;
extern uint8_t g_comm2_buffer
    [WD_UART_COMM_QUE_SIZE]; //cf> 원래 내부에 숨겨두지만 디버깅 요인등의 이유로 빼둔다.

extern void Uart2_PutString(char *_buffer);
extern void Uart2_PutArray(uint8_t *_buffer, int16_t _length);
extern void Uart2_DMA_PutArray(uint8_t *_buffer, int16_t _length);
extern uint16_t Uart2_Size(void);
extern void Uart2_Clear(void);
extern char Uart2_GetChar(void);
extern void Uart2_PutChar(uint8_t _ch);
extern char Uart2_WaitChar(void);
extern bool Uart2_IsRxChar(const char _ch);
#if 0
extern bool Uart2_InputString(char* _buffer, uint16_t _buf_size);
extern void Uart2_Print(char* _fmt, ... );
#endif
#endif

#if RTE_PLATFORM_PRINT_HAL_UART3
/***************************************************************************************************************************************************************
 **********  UART3
 **************************************************************************************************************************************************************/
extern wd_ringbuf_t *gp_Uart3_RxRing;
extern uint8_t g_comm3_buffer
    [WD_UART_COMM_QUE_SIZE]; //cf> 원래 내부에 숨겨두지만 디버깅 요인등의 이유로 빼둔다.

extern void Uart3_PutString(char *_buffer);
extern void Uart3_PutArray(uint8_t *_buffer, int16_t _length);
extern void Uart3_DMA_PutArray(uint8_t *_buffer, int16_t _length);
extern uint16_t Uart3_Size(void);
extern void Uart3_Clear(void);
extern char Uart3_GetChar(void);
extern void Uart3_PutChar(uint8_t _ch);
extern char Uart3_WaitChar(void);
extern bool Uart3_IsRxChar(const char _ch);
#if 0
extern bool Uart3_InputString(char* _buffer, uint16_t _buf_size);
extern void Uart3_Print(char* _fmt, ... );
#endif
#endif

#if RTE_PLATFORM_PRINT_HAL_UART4
/***************************************************************************************************************************************************************
 **********  UART4
 **************************************************************************************************************************************************************/
extern wd_ringbuf_t *gp_Uart4_RxRing;
extern uint8_t g_comm4_buffer
    [WD_UART_COMM_QUE_SIZE]; //cf> 원래 내부에 숨겨두지만 디버깅 요인등의 이유로 빼둔다.

extern void Uart4_PutString(char *_buffer);
extern void Uart4_PutArray(uint8_t *_buffer, int16_t _length);
extern void Uart4_DMA_PutArray(uint8_t *_buffer, int16_t _length);
extern uint16_t Uart4_Size(void);
extern void Uart4_Clear(void);
extern char Uart4_GetChar(void);
extern void Uart4_PutChar(uint8_t _ch);
extern char Uart4_WaitChar(void);
extern bool Uart4_IsRxChar(const char _ch);
#if 0
extern bool Uart4_InputString(char* _buffer, uint16_t _buf_size);
extern void Uart4_Print(char* _fmt, ... );
#endif
#endif

#if RTE_PLATFORM_PRINT_HAL_UART5
/***************************************************************************************************************************************************************
 **********  UART5
 **************************************************************************************************************************************************************/
extern wd_ringbuf_t *wd_print_hal_uart5_rx_ring;
extern uint8_t wd_print_hal_comm5_buffer
    [WD_UART_COMM_QUE_SIZE]; //cf> 원래 내부에 숨겨두지만 디버깅 요인등의 이유로 빼둔다.

extern void wd_print_hal_uart5_put_string(char *_buffer);
extern void wd_print_hal_uart5_put_array(uint8_t *_buffer, int16_t _length);
extern void wd_print_hal_uart5_dma_put_array(uint8_t *_buffer, int16_t _length);
extern uint16_t wd_print_hal_uart5_size(void);
extern void wd_print_hal_clear_uart5(void);
extern char wd_print_hal_uart5_get_char(void);
extern void wd_print_hal_uart5_put_char(uint8_t _ch);
extern char wd_print_hal_uart5_wait_char(void);
extern bool wd_print_hal_uart5_is_rx_char(const char _ch);
#if 0
extern bool Uart5_InputString(char* _buffer, uint16_t _buf_size);
extern void Uart5_Print(char* _fmt, ... );
#endif
#endif

#if RTE_PLATFORM_PRINT_HAL_UART6
/***************************************************************************************************************************************************************
 **********  UART6
 **************************************************************************************************************************************************************/
extern wd_ringbuf_t *gp_Uart6_RxRing;
extern uint8_t g_comm6_buffer
    [WD_UART_COMM_QUE_SIZE]; //cf> 원래 내부에 숨겨두지만 디버깅 요인등의 이유로 빼둔다.

extern void Uart6_PutString(char *_buffer);
extern void Uart6_PutArray(uint8_t *_buffer, int16_t _length);
extern void Uart6_DMA_PutArray(uint8_t *_buffer, int16_t _length);
extern uint16_t Uart6_Size(void);
extern void Uart6_Clear(void);
extern char Uart6_GetChar(void);
extern void Uart6_PutChar(uint8_t _ch);
extern char Uart6_WaitChar(void);
extern bool Uart6_IsRxChar(const char _ch);
#if 0
extern bool Uart6_InputString(char* _buffer, uint16_t _buf_size);
extern void Uart6_Print(char* _fmt, ... );
#endif
#endif

#if RTE_PLATFORM_PRINT_HAL_UART5
/***************************************************************************************************************************************************************
 **********  UART7
 **************************************************************************************************************************************************************/
extern wd_ringbuf_t *gp_Uart7_RxRing;
extern uint8_t g_comm7_buffer
    [WD_UART_COMM_QUE_SIZE]; //cf> 원래 내부에 숨겨두지만 디버깅 요인등의 이유로 빼둔다.

extern void Uart7_PutString(char *_buffer);
extern void Uart7_PutArray(uint8_t *_buffer, int16_t _length);
extern void Uart7_DMA_PutArray(uint8_t *_buffer, int16_t _length);
extern uint16_t Uart7_Size(void);
extern void Uart7_Clear(void);
extern char Uart7_GetChar(void);
extern void Uart7_PutChar(uint8_t _ch);
extern char Uart7_WaitChar(void);
extern bool Uart7_IsRxChar(const char _ch);
#if 0
extern bool Uart7_InputString(char* _buffer, uint16_t _buf_size);
extern void Uart7_Print(char* _fmt, ... );
#endif
#endif

#if RTE_PLATFORM_PRINT_HAL_UART5
/***************************************************************************************************************************************************************
 **********  UART8
 **************************************************************************************************************************************************************/
extern wd_ringbuf_t *wd_print_hal_uart8_rx_ring;
#define WD_UART8_COMM_QUE_SIZE (1024 * 4)
extern uint8_t wd_print_hal_comm8_buffer[WD_UART8_COMM_QUE_SIZE];

extern void wd_print_hal_uart8_put_string(char *_buffer);
extern void wd_print_hal_uart8_put_array(uint8_t *_buffer, int16_t _length);
extern void wd_print_hal_uart8_dma_put_array(uint8_t *_buffer, int16_t _length);
extern uint16_t wd_print_hal_uart8_size(void);
extern void wd_print_hal_clear_uart8(void);
extern char wd_print_hal_uart8_get_char(void);
extern void wd_print_hal_uart8_put_char(uint8_t _ch);
extern char wd_print_hal_uart8_wait_char(void);
extern bool wd_print_hal_uart8_is_rx_char(const char _ch);
#if 0
extern bool Uart8_InputString(char* _buffer, uint16_t _buf_size);
extern void Uart8_Print(char* _fmt, ... );
#endif
#endif
/* USER CODE END Prototypes */

#endif /* WD_PLATFORM_WD_PRINT_HAL_H */
