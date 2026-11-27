/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_ringbuf.h
 *
 *  Created on: Jan 3, 2025
 *      Author: doubled
 */

#ifndef WD_CORE_WD_RINGBUF_H
#define WD_CORE_WD_RINGBUF_H

/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "wd_core.h"

/* USER CODE END Includes */


/* USER CODE BEGIN Private defines */
typedef struct ring_buffer_s {
    uint16_t BUFFER_SIZE;       // 링버퍼 사이즈 초기화 시 고정값
    uint8_t *buffer;            // 저장버퍼
    volatile uint16_t read_idx; // 읽기 시작 위치 queue의 tail
    volatile uint16_t length;   // 저장된 데이터 길이
    volatile uint8_t temp;      // 수신 레지스터 임시버퍼
} wd_ringbuf_t;
/* USER CODE END Private defines */


/* USER CODE BEGIN Prototypes */

//링버퍼 초기화
extern wd_ringbuf_t *wd_ringbuf_init(wd_ringbuf_t *_Ring, uint8_t *_Buffer, uint16_t _Size);
//기본적인 pop
extern uint8_t wd_ringbuf_pop(wd_ringbuf_t *_Ring);
//pop하지 않고 _Index 데이터 참조만
extern uint8_t wd_ringbuf_pop_at(wd_ringbuf_t *_Ring, uint16_t _Index);
//pop하지 않고 참조로 _Buffer를 채운다. index = 0 부터 가져온다.
extern void wd_ringbuf_pop_at_buffer(wd_ringbuf_t *_Ring, uint8_t *_Buffer, uint16_t _Size);

//기본적인 push
extern bool wd_ringbuf_push(wd_ringbuf_t *_Ring, uint8_t _Item);
//_Index 의 값만 변경
extern void wd_ringbuf_push_at(wd_ringbuf_t *_Ring, uint16_t _Index, uint8_t _Item);
//버퍼를 push
extern void wd_ringbuf_push_buffer(wd_ringbuf_t *_Ring, uint8_t *_Buffer, uint16_t _Size);
//문자열을 push
extern void wd_ringbuf_push_string(wd_ringbuf_t *_Ring, const char *_String);

//_Size 만큼 삭제 처리
extern void wd_ringbuf_remove(wd_ringbuf_t *_Ring, uint16_t _Size);
//전체 삭제 처리
extern void wd_ringbuf_clear(wd_ringbuf_t *_Ring);
//'\0'으로 끝나는 문자열을 비교하여 같으면 TRUE 반환
extern bool wd_ringbuf_compare_string(wd_ringbuf_t *_Ring, const char *_String);

/* ------------------------------------------------------------------------*/
#ifdef PRINT_DEBUG_MENU
#endif
/* USER CODE END Prototypes */


#endif /* WD_CORE_WD_RINGBUF_H */
