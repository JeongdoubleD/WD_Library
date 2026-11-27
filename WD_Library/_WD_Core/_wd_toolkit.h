/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_toolkit.h
 *
 *  Created on: Jan 13, 2025
 *      Author: doubled
 */

#ifndef WD_CORE_WD_TOOLKIT_H
#define WD_CORE_WD_TOOLKIT_H

#include "wd_core.h"

#if 1 //토글

#define WD_CRC_SIZE     2U
#define WD_CRC16_RESULT 0x0f47 // 0xf0b8 ^ 1  /* Good final FCS value */
#define WD_CRC32_RESULT 0x725EDB6C

//
extern uint16_t wd_toolkit_crc16_ccitt(const uint8_t *data, size_t length);
extern int wd_toolkit_crc16_ccitt_verify(const uint8_t *data, size_t length, uint16_t expected_crc);
extern void wd_toolkit_crc16_ccitt_append(const uint8_t *data, size_t length, uint8_t *crc_hi,
                                          uint8_t *crc_lo);
//
//
extern uint16_t wd_toolkit_crc16_modbus(const uint8_t *_buffer, int _start_idx, int _length);
extern int wd_toolkit_crc16_modbus_verify(const uint8_t *data, size_t length, int _start_idx,
                                          uint16_t expected_crc);
extern void wd_toolkit_crc16_modbus_append(const uint8_t *data, size_t length, int _start_idx,
                                           uint8_t *crc_hi, uint8_t *crc_lo);
//

uint16_t wd_toolkit_crc16(void *ptr, uint32_t num); // Cyclical Redundancy Check - 16
uint16_t wd_toolkit_crc16_extension(void *ptr, uint32_t num, uint16_t *crc);
uint32_t wd_toolkit_crc32(void *ptr, uint32_t num);
uint32_t wd_toolkit_crc32_extension(void *ptr, uint32_t num, uint32_t *crc);
uint8_t wd_toolkit_char_to_digit(uint8_t string);

void wd_toolkit_change_endian_data(void *ptr, uint32_t num);
uint32_t wd_memchr(const void *ptr, uint8_t c, uint32_t num);
uint32_t wd_memchrn(const void *ptr, uint8_t c, uint32_t num);
#if defined(RTE_METROLOGY_TYPE_RN8302B) && (RTE_METROLOGY_TYPE == RTE_METROLOGY_TYPE_RN8302B)
int32_t wd_memchr_idx_16(const void *ptr, uint16_t c, uint32_t num);
#endif
int32_t wd_memchr_idx(const void *ptr, uint8_t c, uint32_t num);
uint32_t wd_toolkit_ascii_to_hex(void *hex, void *ascii, uint32_t num_ascii);
uint32_t wd_toolkit_hex_to_ascii(void *ascii, void *hex, uint32_t num_hex);
int32_t wd_toolkit_memcmp_hex_ascii(const void *hex, void *ascii, size_t len_ascii);
uint32_t wd_toolkit_calc_circular_buffer_idx(uint32_t idx, uint32_t offset, uint32_t idx_size,
                                             uint8_t direction);
void wd_toolkit_real_type_validation(void *real);
uint32_t wd_toolkit_isqrt64(uint64_t h);
#if (defined(__ICCARM__))
void arm_cmplx_mag_f64(const float64_t *pSrc, float64_t *pDst, uint32_t numSamples);
void arm_max_f64(const float64_t *pSrc, uint32_t blockSize, float64_t *pResult, uint32_t *pIndex);
#endif

int wd_gcd(int a, int b);
int wd_lcm(int a, int b);

void wd_memset_rand(void *ptr, size_t size);
int wd_memcmp_print(const void *buf1, const void *buf2, size_t size);

#endif //	#if 1 //토글

#endif /* WD_CORE_WD_TOOLKIT_H */
