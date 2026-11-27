/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 *
 * @file    _wd_cbuf.c
 * @brief   Byte circular buffer (FIFO). See _wd_cbuf.h.
 */
#include "wd_core.h"

uint32_t wd_cbuf_debug_terminal_max_count;

/* copy 'num' bytes out of the ring starting at index 'from' (handles the wrap) */
static void ring_read(const wd_cbuf_t *buf, uint32_t from, uint8_t *dest, uint32_t num) {
    uint32_t first = buf->Capacity - from;

    if (first > num) {
        first = num;
    }
    (void)memcpy(dest, &buf->Data[from], first);
    if (num > first) {
        (void)memcpy(&dest[first], buf->Data, num - first);
    }
}

/* copy 'num' bytes into the ring starting at index 'to' (handles the wrap) */
static void ring_write(wd_cbuf_t *buf, uint32_t to, const uint8_t *src, uint32_t num) {
    uint32_t first = buf->Capacity - to;

    if (first > num) {
        first = num;
    }
    (void)memcpy(&buf->Data[to], src, first);
    if (num > first) {
        (void)memcpy(buf->Data, &src[first], num - first);
    }
}

void wd_cbuf_init(wd_cbuf_t *buf, void *ptr, uint32_t size) {
    if (buf == NULL) {
        return;
    }
    buf->Data = (uint8_t *)ptr;
    buf->Capacity = (ptr != NULL) ? size : 0U;
    buf->Head = 0U;
    buf->Size = 0U;
}

uint32_t wd_cbuf_push(wd_cbuf_t *buf, void *src, uint32_t num) {
    const uint8_t *s = (const uint8_t *)src;
    uint32_t free_bytes;

    if ((buf == NULL) || (src == NULL) || (buf->Capacity == 0U) || (num == 0U)) {
        return 0U;
    }

    if (num >= buf->Capacity) { /* only the newest 'Capacity' bytes survive */
        s = &s[num - buf->Capacity];
        num = buf->Capacity;
        buf->Head = 0U;
        buf->Size = 0U;
    }

    free_bytes = buf->Capacity - buf->Size;
    if (num > free_bytes) { /* drop the oldest bytes */
        uint32_t drop = num - free_bytes;

        buf->Head = (buf->Head + drop) % buf->Capacity;
        buf->Size -= drop;
    }

    ring_write(buf, (buf->Head + buf->Size) % buf->Capacity, s, num);
    buf->Size += num;

    if (wd_cbuf_debug_terminal_max_count < buf->Size) {
        wd_cbuf_debug_terminal_max_count = buf->Size;
    }
    return num;
}

uint32_t wd_cbuf_pop(wd_cbuf_t *buf, void *dest, uint32_t num) {
    if ((buf == NULL) || (dest == NULL) || (buf->Size == 0U)) {
        return 0U;
    }
    if (num > buf->Size) {
        num = buf->Size;
    }

    ring_read(buf, buf->Head, (uint8_t *)dest, num);
    buf->Head = (buf->Head + num) % buf->Capacity;
    buf->Size -= num;
    if (buf->Size == 0U) {
        buf->Head = 0U;
    }
    return num;
}

uint32_t wd_cbuf_print_pop(wd_cbuf_t *buf, wd_write_func_t func_print, uint32_t num) {
    uint32_t start;
    uint32_t first;

    if ((buf == NULL) || (func_print == NULL) || (buf->Size == 0U)) {
        return 0U;
    }
    if (num > buf->Size) {
        num = buf->Size;
    }

    start = buf->Head;
    first = buf->Capacity - start;
    if (first > num) {
        first = num;
    }

    /* release the bytes first : the output function may push new data, which then goes behind the
     * released area (Head is not rewound here, so the bytes being printed are not overwritten) */
    buf->Head = (buf->Head + num) % buf->Capacity;
    buf->Size -= num;

    func_print(&buf->Data[start], first, 0U);
    if (num > first) {
        func_print(buf->Data, num - first, 0U);
    }
    return num;
}
