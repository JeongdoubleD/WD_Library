/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_fft.h
 *
 *  Created on: Jan 16, 2025
 *      Author: doubled
 */

#ifndef WD_PLATFORM_WD_FFT_H
#define WD_PLATFORM_WD_FFT_H

#include "wd.h"

#if (RTE_PLATFORM_FFT)

#define M_PI 3.14159265358979323846

/* ------------------------------------------------------------------------ 설정 및 기타 정의 (Macro or Structure definition) */
#define WD_FFT_SIZE 1024

typedef struct complext_s {
    double real;
    double imaginary;
} wd_fft_complex_t;
extern double wd_fft_fft_magnitudes[WD_FFT_SIZE / 2];
extern double wd_fft_fft_imaginary[WD_FFT_SIZE / 2];

extern wd_fft_complex_t wd_fft_fft_complexs[WD_FFT_SIZE];
/* ------------------------------------------------------------------------ 노출 변수 및 노출 함수 선언 (Global Function) */
extern void wd_pf_fft_init(void);
extern int wd_pf_fft_calc(int _size, double _input[]);


#endif //#if 1 	// 토글

#endif /* WD_PLATFORM_WD_FFT_H */
