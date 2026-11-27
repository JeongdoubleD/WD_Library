/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 *
 * @file    _wd_cbuf.h
 * @brief   Byte circular buffer (FIFO) on a caller supplied storage area.
 *          - push : when the data does not fit, the oldest bytes are dropped (the newest data is kept)
 *          - pop / print_pop : take up to the requested number of bytes
 *          Not thread / ISR safe : one producer and one consumer in the same context, or lock outside.
 */
/* umbrella header first (before the guard) : any include order expands in the umbrella order */
#include "wd_core.h"

#ifndef WD_CORE_WD_CBUF_H
#define WD_CORE_WD_CBUF_H

typedef struct wd_cbuf_s {
    uint8_t *Data;     /**< storage area */
    uint32_t Capacity; /**< storage size in bytes */
    uint32_t Head;     /**< index of the oldest byte */
    uint32_t Size;     /**< number of bytes held (read only for users) */
} wd_cbuf_t;

/** highest fill level seen by wd_cbuf_push() (debug statistic, all buffers) */
extern uint32_t wd_cbuf_debug_terminal_max_count;

/**
 * @brief  Attach a storage area to a buffer and empty it.
 * @param  buf   buffer
 * @param  ptr   storage area (size bytes)
 * @param  size  storage size in bytes
 */
void wd_cbuf_init(wd_cbuf_t *buf, void *ptr, uint32_t size);

/**
 * @brief  Append bytes. Oldest bytes are dropped when the free space is too small.
 * @return number of bytes stored (num, or the capacity when num is larger)
 */
uint32_t wd_cbuf_push(wd_cbuf_t *buf, void *src, uint32_t num);

/**
 * @brief  Remove up to num bytes into dest.
 * @return number of bytes copied
 */
uint32_t wd_cbuf_pop(wd_cbuf_t *buf, void *dest, uint32_t num);

/**
 * @brief  Remove up to num bytes and hand them to func_print (one call, or two when the data wraps).
 *         The buffer is updated before the calls.
 * @return number of bytes passed to func_print
 */
uint32_t wd_cbuf_print_pop(wd_cbuf_t *buf, wd_write_func_t func_print, uint32_t num);

#endif /* WD_CORE_WD_CBUF_H */
