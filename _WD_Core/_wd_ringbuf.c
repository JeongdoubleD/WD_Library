/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_ringbuf.c
 *
 *  Created on: Jan 3, 2025
 *      Author: doubled
 */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "wd_core.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern wd_ringbuf_t *wd_ringbuf_init(wd_ringbuf_t *_Ring, uint8_t *_Buffer, uint16_t _Size) {
    _Ring->BUFFER_SIZE = _Size;
    _Ring->buffer = _Buffer;
    _Ring->read_idx = 0;
    _Ring->length = 0;
    _Ring->temp = 0;

    return _Ring;
}

extern uint8_t wd_ringbuf_pop(wd_ringbuf_t *_Ring) {
    uint8_t item = 0;

    if (_Ring->length) {
        //  Get the first character from buffer.
        item = _Ring->buffer[_Ring->read_idx];

        //  Move index down and decrement length.
        _Ring->read_idx = (uint16_t)((_Ring->read_idx + 1) % _Ring->BUFFER_SIZE);
        _Ring->length--;
    }

    return item;
}

extern uint8_t wd_ringbuf_pop_at(wd_ringbuf_t *_Ring, uint16_t _Index) {
    //  Return Character at index in Buffer.
    return _Ring->buffer[(_Ring->read_idx + _Index) % _Ring->BUFFER_SIZE];
}

extern void wd_ringbuf_pop_at_buffer(wd_ringbuf_t *_Ring, uint8_t *_Buffer, uint16_t _Size) {
    uint16_t idx;
    for (idx = 0; idx < _Size; ++idx) {
        _Buffer[idx] = _Ring->buffer[(_Ring->read_idx + idx) % _Ring->BUFFER_SIZE];
    }
}

extern bool wd_ringbuf_push(wd_ringbuf_t *_Ring, uint8_t _Item) {
    //최대값이면 하나 삭제한다. 꽉차있으면 오래된 것부터 삭제됨
    if (_Ring->length >= _Ring->BUFFER_SIZE) {
        _Ring->read_idx = (uint16_t)((_Ring->read_idx + 1) % _Ring->BUFFER_SIZE);
        _Ring->length -= 1;
    }

    //  Save data byte at end of buffer.
    _Ring->buffer[(_Ring->read_idx + _Ring->length) % _Ring->BUFFER_SIZE] = _Item;
    //  Increment the length.
    _Ring->length++;

    return true;
}

extern void wd_ringbuf_push_at(wd_ringbuf_t *_Ring, uint16_t _Index, uint8_t _Item) {
    _Ring->buffer[(_Ring->read_idx + _Index) % _Ring->BUFFER_SIZE] = _Item;
}

extern void wd_ringbuf_push_buffer(wd_ringbuf_t *_Ring, uint8_t *_Buffer, uint16_t _Size) {
    uint16_t idx;
    for (idx = 0; idx < _Size; ++idx) {
        wd_ringbuf_push(_Ring, _Buffer[idx]);
    }
}

extern void wd_ringbuf_push_string(wd_ringbuf_t *_Ring, const char *_String) {
    while (*_String != '\0') {
        wd_ringbuf_push(_Ring, *_String);
        _String++;
    }
}

extern void wd_ringbuf_remove(wd_ringbuf_t *_Ring, uint16_t _Size) {
    //Dump Number bytes from the front of the buffer are we dumping less than the entire buffer?
    if (_Ring->length > _Size) {
        //Move index down by numbytes and decrement length by numbytes.
        _Ring->read_idx = (uint16_t)((_Ring->read_idx + _Size) % _Ring->BUFFER_SIZE);
        _Ring->length -= _Size;
    } else {
        _Ring->length = 0;   // Flush the whole Buffer.
        _Ring->read_idx = 0; //
    }
}

extern void wd_ringbuf_clear(wd_ringbuf_t *_Ring) {
    // Flush contents of the buffer.
    _Ring->length = 0;
    _Ring->read_idx = 0;
}

extern bool wd_ringbuf_compare_string(wd_ringbuf_t *_Ring, const char *_String) {
    int16_t len = (int16_t)strlen(_String);
    int16_t idx;

    if (_Ring->length >= len) {
        for (idx = 0; idx < len; ++idx) {
            if (_String[idx] != wd_ringbuf_pop_at(_Ring, (uint16_t)idx)) {
                return false;
            }
        }
        return true;
    }
    return false;
}
/* ------------------------------------------------------------------------*/
#ifdef PRINT_DEBUG_MENU
#endif
