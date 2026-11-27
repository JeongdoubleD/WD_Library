/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _PF_fft.c
 *
 *  Created on: Jan 16, 2025
 *      Author: doubled
 */

#include "wd.h"

#if (RTE_PLATFORM_FFT)

/* ------------------------------------------------------------------------ 전역, 내부 변수 및 정의 (Global variable) */

/* ------------------------------------------------------------------------ 내부함수 선언, HW재정의 (IRQ 등....) */
/* ------------------------------------------------------------------------ 함수 정의 (static, extern 생략하지 말것) */
double wd_fft_fft_magnitudes[WD_FFT_SIZE / 2];
double wd_fft_fft_imaginary[WD_FFT_SIZE / 2];

wd_fft_complex_t wd_fft_fft_complexs[WD_FFT_SIZE];
static wd_fft_complex_t g_fft_W[WD_FFT_SIZE / 2];

extern void wd_pf_fft_init(void) {
    /* 2. calculate W */
    /* Real and Imaginary part */
    int n2 = WD_FFT_SIZE / 2;
    double t_val = (2.0 * M_PI) / WD_FFT_SIZE;
    for (int ii = 0; ii < n2; ++ii) {
        g_fft_W[ii].real = cos(t_val * ii);
        g_fft_W[ii].imaginary = -sin(t_val * ii);
    }
}
extern int wd_pf_fft_calc(int _size, double _input[]) {
    /* 1. check 2^n count -> if else then return -1 */
    if ((_size != 0) && ((_size & (_size - 1)) != 0)) {
        return -1;
    }

    //## 1. MakeComplex
    for (int ii = 0; ii < _size; ++ii) {
        wd_fft_fft_complexs[ii].real = _input[ii];
        wd_fft_fft_complexs[ii].imaginary = 0.0;
    }

    /* 2. calculate W */
    //FFT_Init() 대체
    /* 3. shuffle input array index to calculate bottom-up style FFT */
    int log2N = (int)log2(_size);
    wd_fft_complex_t temp;
    for (long n = 1; n < _size - 1; ++n) {
        long m = 0; //reversed num of n
        for (int i = 0; i < log2N; ++i) {
            m |= ((n >> i) & 1) << (log2N - i - 1); //reverse the bits
        }
        if (n < m) { //exchange
            temp = wd_fft_fft_complexs[n];
            wd_fft_fft_complexs[n] = wd_fft_fft_complexs[m];
            wd_fft_fft_complexs[m] = temp;
        }
    }

    /* 4. execute fft */
    for (int loop = 0; loop < log2N; ++loop) {
        long regionSize = 1 << (loop + 1);    //if N=8: 2 -> 4 -> 8 */
        long kJump = 1 << (log2N - loop - 1); //if N=8: 4 -> 2 -> 1 */
        long half = regionSize >> 1;
        for (register long i = 0; i < _size; i += regionSize) {
            long blockEnd = i + half - 1;
            long k = 0;
            wd_fft_complex_t tv;
            for (register long j = i; j <= blockEnd; ++j) { //j start from i */
                tv.real = g_fft_W[k].real * wd_fft_fft_complexs[j + half].real -
                          g_fft_W[k].imaginary * wd_fft_fft_complexs[j + half].imaginary;
                tv.imaginary = g_fft_W[k].imaginary * wd_fft_fft_complexs[j + half].real +
                               g_fft_W[k].real * wd_fft_fft_complexs[j + half].imaginary;

                wd_fft_fft_complexs[j + half].real = wd_fft_fft_complexs[j].real - tv.real;
                wd_fft_fft_complexs[j + half].imaginary =
                    wd_fft_fft_complexs[j].imaginary - tv.imaginary;

                wd_fft_fft_complexs[j].real = wd_fft_fft_complexs[j].real + tv.real;
                wd_fft_fft_complexs[j].imaginary = wd_fft_fft_complexs[j].imaginary + tv.imaginary;

                k += kJump;
            }
        }
    }

    //## 5. magnitudes 계산
    for (int ii = 0; ii < (_size / 2); ++ii) {
        wd_fft_fft_magnitudes[ii] =
            (2.0 * sqrt(wd_fft_fft_complexs[ii].real * wd_fft_fft_complexs[ii].real +
                        wd_fft_fft_complexs[ii].imaginary * wd_fft_fft_complexs[ii].imaginary)) /
            ((double)_size);
    }
    wd_fft_fft_magnitudes[0] = wd_fft_fft_magnitudes[0] / (double)_size;
    return 0;
}


#endif //#if 1 	// 토글
