/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_gpio.h
 *
 *  Created on: Dec 30, 2024
 *      Author: doubled
 */

#ifndef WD_CORE_WD_GPIO_H
#define WD_CORE_WD_GPIO_H

#include "wd_core.h"


typedef uint32_t wd_gpio_port_t;
typedef GPIO_TypeDef *wd_gpio_st_port_t;
typedef unsigned int wd_gpio_st_pin_t;
typedef uint32_t wd_gpio_altf_t;

typedef enum {
    /*--------------- Pin ---------------*/
    WD_PIN_0 = 0,
    WD_PIN_1 = 1,
    WD_PIN_2 = 2,
    WD_PIN_3 = 3,
    WD_PIN_4 = 4,
    WD_PIN_5 = 5,
    WD_PIN_6 = 6,
    WD_PIN_7 = 7,
    WD_PIN_8 = 8,
    WD_PIN_9 = 9,
    WD_PIN_10 = 10,
    WD_PIN_11 = 11,
    WD_PIN_12 = 12,
    WD_PIN_13 = 13,
    WD_PIN_14 = 14,
    WD_PIN_15 = 15,
    WD_PIN_16 = 16,
    WD_PIN_17 = 17,
    WD_PIN_18 = 18,
    WD_PIN_19 = 19,
    WD_PIN_20 = 20,
    WD_PIN_21 = 21,
    WD_PIN_22 = 22,
    WD_PIN_23 = 23,
    WD_PIN_24 = 24,
    WD_PIN_25 = 25,
    WD_PIN_26 = 26,
    WD_PIN_27 = 27,
    WD_PIN_28 = 28,
    WD_PIN_29 = 29,
    WD_PIN_30 = 30,
    WD_PIN_31 = 31,

    /*--------------- Port ---------------*/
    WD_PORT_0 = 0,
    WD_PORT_1 = 1,
    WD_PORT_2 = 2,
    WD_PORT_3 = 3,
    WD_PORT_4 = 4,
    WD_PORT_5 = 5,
    WD_PORT_6 = 6,
    WD_PORT_7 = 7,
    WD_PORT_8 = 8,
    WD_PORT_9 = 9,
    WD_PORT_10 = 10,
    WD_PORT_11 = 11,
    WD_PORT_12 = 12,
    WD_PORT_13 = 13,
    WD_PORT_14 = 14,
    WD_PORT_15 = 15,
    WD_PORT_A = 0,
    WD_PORT_B = 1,
    WD_PORT_C = 2,
    WD_PORT_D = 3,
    WD_PORT_E = 4,
    WD_PORT_F = 5,
    WD_PORT_G = 6,
    WD_PORT_H = 7,
    WD_PORT_i = 8,
    WD_PORT_J = 9,
    WD_PORT_K = 10,
    WD_PORT_L = 11,
    WD_PORT_M = 12,
    WD_PORT_N = 13,
    WD_PORT_O = 14,
    WD_PORT_P = 15,

/*--------------- Port Pin ---------------*/
#define WD_PD(port, pin)            \
    WD_PASTE4(WD_P, port, _, pin) = \
        WD_GPIO_PORT(WD_PASTE(WD_PORT_, port)) | WD_GPIO_PIN(WD_PASTE(WD_PIN_, pin))
#if 1
#define WD_GPIO_PIN_POS 0
#define WD_GPIO_PIN_MSK (0x0FUL << WD_GPIO_PIN_POS)
#define WD_GPIO_PIN(n)  ((n & 0x0F) << WD_GPIO_PIN_POS)

#define WD_GPIO_PORT_POS 4
#define WD_GPIO_PORT_MSK (0x0FUL << WD_GPIO_PORT_POS)
#define WD_GPIO_PORT(n)  ((n & 0x0F) << WD_GPIO_PORT_POS)


#if 0
  WD_PD( 0,0), WD_PD( 0,1), WD_PD( 0,2), WD_PD( 0,3), WD_PD( 0,4), WD_PD( 0,5), WD_PD( 0,6), WD_PD( 0,7), WD_PD( 0,8), WD_PD( 0,9), WD_PD( 0,10), WD_PD( 0,11), WD_PD( 0,12), WD_PD( 0,13), WD_PD( 0,14), WD_PD( 0,15),
  WD_PD( 1,0), WD_PD( 1,1), WD_PD( 1,2), WD_PD( 1,3), WD_PD( 1,4), WD_PD( 1,5), WD_PD( 1,6), WD_PD( 1,7), WD_PD( 1,8), WD_PD( 1,9), WD_PD( 1,10), WD_PD( 1,11), WD_PD( 1,12), WD_PD( 1,13), WD_PD( 1,14), WD_PD( 1,15),
  WD_PD( 2,0), WD_PD( 2,1), WD_PD( 2,2), WD_PD( 2,3), WD_PD( 2,4), WD_PD( 2,5), WD_PD( 2,6), WD_PD( 2,7), WD_PD( 2,8), WD_PD( 2,9), WD_PD( 2,10), WD_PD( 2,11), WD_PD( 2,12), WD_PD( 2,13), WD_PD( 2,14), WD_PD( 2,15),
  WD_PD( 3,0), WD_PD( 3,1), WD_PD( 3,2), WD_PD( 3,3), WD_PD( 3,4), WD_PD( 3,5), WD_PD( 3,6), WD_PD( 3,7), WD_PD( 3,8), WD_PD( 3,9), WD_PD( 3,10), WD_PD( 3,11), WD_PD( 3,12), WD_PD( 3,13), WD_PD( 3,14), WD_PD( 3,15),
  WD_PD( 4,0), WD_PD( 4,1), WD_PD( 4,2), WD_PD( 4,3), WD_PD( 4,4), WD_PD( 4,5), WD_PD( 4,6), WD_PD( 4,7), WD_PD( 4,8), WD_PD( 4,9), WD_PD( 4,10), WD_PD( 4,11), WD_PD( 4,12), WD_PD( 4,13), WD_PD( 4,14), WD_PD( 4,15),
  WD_PD( 5,0), WD_PD( 5,1), WD_PD( 5,2), WD_PD( 5,3), WD_PD( 5,4), WD_PD( 5,5), WD_PD( 5,6), WD_PD( 5,7), WD_PD( 5,8), WD_PD( 5,9), WD_PD( 5,10), WD_PD( 5,11), WD_PD( 5,12), WD_PD( 5,13), WD_PD( 5,14), WD_PD( 5,15),
  WD_PD( 6,0), WD_PD( 6,1), WD_PD( 6,2), WD_PD( 6,3), WD_PD( 6,4), WD_PD( 6,5), WD_PD( 6,6), WD_PD( 6,7), WD_PD( 6,8), WD_PD( 6,9), WD_PD( 6,10), WD_PD( 6,11), WD_PD( 6,12), WD_PD( 6,13), WD_PD( 6,14), WD_PD( 6,15),
  WD_PD( 7,0), WD_PD( 7,1), WD_PD( 7,2), WD_PD( 7,3), WD_PD( 7,4), WD_PD( 7,5), WD_PD( 7,6), WD_PD( 7,7), WD_PD( 7,8), WD_PD( 7,9), WD_PD( 7,10), WD_PD( 7,11), WD_PD( 7,12), WD_PD( 7,13), WD_PD( 7,14), WD_PD( 7,15),
  WD_PD( 8,0), WD_PD( 8,1), WD_PD( 8,2), WD_PD( 8,3), WD_PD( 8,4), WD_PD( 8,5), WD_PD( 8,6), WD_PD( 8,7), WD_PD( 8,8), WD_PD( 8,9), WD_PD( 8,10), WD_PD( 8,11), WD_PD( 8,12), WD_PD( 8,13), WD_PD( 8,14), WD_PD( 8,15),
  WD_PD( 9,0), WD_PD( 9,1), WD_PD( 9,2), WD_PD( 9,3), WD_PD( 9,4), WD_PD( 9,5), WD_PD( 9,6), WD_PD( 9,7), WD_PD( 9,8), WD_PD( 9,9), WD_PD( 9,10), WD_PD( 9,11), WD_PD( 9,12), WD_PD( 9,13), WD_PD( 9,14), WD_PD( 9,15),
  WD_PD(10,0), WD_PD(10,1), WD_PD(10,2), WD_PD(10,3), WD_PD(10,4), WD_PD(10,5), WD_PD(10,6), WD_PD(10,7), WD_PD(10,8), WD_PD(10,9), WD_PD(10,10), WD_PD(10,11), WD_PD(10,12), WD_PD(10,13), WD_PD(10,14), WD_PD(10,15),
  WD_PD(11,0), WD_PD(11,1), WD_PD(11,2), WD_PD(11,3), WD_PD(11,4), WD_PD(11,5), WD_PD(11,6), WD_PD(11,7), WD_PD(11,8), WD_PD(11,9), WD_PD(11,10), WD_PD(11,11), WD_PD(11,12), WD_PD(11,13), WD_PD(11,14), WD_PD(11,15),
  WD_PD(12,0), WD_PD(12,1), WD_PD(12,2), WD_PD(12,3), WD_PD(12,4), WD_PD(12,5), WD_PD(12,6), WD_PD(12,7), WD_PD(12,8), WD_PD(12,9), WD_PD(12,10), WD_PD(12,11), WD_PD(12,12), WD_PD(12,13), WD_PD(12,14), WD_PD(12,15),
  WD_PD(13,0), WD_PD(13,1), WD_PD(13,2), WD_PD(13,3), WD_PD(13,4), WD_PD(13,5), WD_PD(13,6), WD_PD(13,7), WD_PD(13,8), WD_PD(13,9), WD_PD(13,10), WD_PD(13,11), WD_PD(13,12), WD_PD(13,13), WD_PD(13,14), WD_PD(13,15),
  WD_PD(14,0), WD_PD(14,1), WD_PD(14,2), WD_PD(14,3), WD_PD(14,4), WD_PD(14,5), WD_PD(14,6), WD_PD(14,7), WD_PD(14,8), WD_PD(14,9), WD_PD(14,10), WD_PD(14,11), WD_PD(14,12), WD_PD(14,13), WD_PD(14,14), WD_PD(14,15),
  WD_PD(15,0), WD_PD(15,1), WD_PD(15,2), WD_PD(15,3), WD_PD(15,4), WD_PD(15,5), WD_PD(15,6), WD_PD(15,7), WD_PD(15,8), WD_PD(15,9), WD_PD(15,10), WD_PD(15,11), WD_PD(15,12), WD_PD(15,13), WD_PD(15,14), WD_PD(15,15),
#else
    WD_PD(A, 0),
    WD_PD(A, 1),
    WD_PD(A, 2),
    WD_PD(A, 3),
    WD_PD(A, 4),
    WD_PD(A, 5),
    WD_PD(A, 6),
    WD_PD(A, 7),
    WD_PD(A, 8),
    WD_PD(A, 9),
    WD_PD(A, 10),
    WD_PD(A, 11),
    WD_PD(A, 12),
    WD_PD(A, 13),
    WD_PD(A, 14),
    WD_PD(A, 15),
    WD_PD(B, 0),
    WD_PD(B, 1),
    WD_PD(B, 2),
    WD_PD(B, 3),
    WD_PD(B, 4),
    WD_PD(B, 5),
    WD_PD(B, 6),
    WD_PD(B, 7),
    WD_PD(B, 8),
    WD_PD(B, 9),
    WD_PD(B, 10),
    WD_PD(B, 11),
    WD_PD(B, 12),
    WD_PD(B, 13),
    WD_PD(B, 14),
    WD_PD(B, 15),
    WD_PD(C, 0),
    WD_PD(C, 1),
    WD_PD(C, 2),
    WD_PD(C, 3),
    WD_PD(C, 4),
    WD_PD(C, 5),
    WD_PD(C, 6),
    WD_PD(C, 7),
    WD_PD(C, 8),
    WD_PD(C, 9),
    WD_PD(C, 10),
    WD_PD(C, 11),
    WD_PD(C, 12),
    WD_PD(C, 13),
    WD_PD(C, 14),
    WD_PD(C, 15),
    WD_PD(D, 0),
    WD_PD(D, 1),
    WD_PD(D, 2),
    WD_PD(D, 3),
    WD_PD(D, 4),
    WD_PD(D, 5),
    WD_PD(D, 6),
    WD_PD(D, 7),
    WD_PD(D, 8),
    WD_PD(D, 9),
    WD_PD(D, 10),
    WD_PD(D, 11),
    WD_PD(D, 12),
    WD_PD(D, 13),
    WD_PD(D, 14),
    WD_PD(D, 15),
    WD_PD(E, 0),
    WD_PD(E, 1),
    WD_PD(E, 2),
    WD_PD(E, 3),
    WD_PD(E, 4),
    WD_PD(E, 5),
    WD_PD(E, 6),
    WD_PD(E, 7),
    WD_PD(E, 8),
    WD_PD(E, 9),
    WD_PD(E, 10),
    WD_PD(E, 11),
    WD_PD(E, 12),
    WD_PD(E, 13),
    WD_PD(E, 14),
    WD_PD(E, 15),
    WD_PD(F, 0),
    WD_PD(F, 1),
    WD_PD(F, 2),
    WD_PD(F, 3),
    WD_PD(F, 4),
    WD_PD(F, 5),
    WD_PD(F, 6),
    WD_PD(F, 7),
    WD_PD(F, 8),
    WD_PD(F, 9),
    WD_PD(F, 10),
    WD_PD(F, 11),
    WD_PD(F, 12),
    WD_PD(F, 13),
    WD_PD(F, 14),
    WD_PD(F, 15),
    WD_PD(G, 0),
    WD_PD(G, 1),
    WD_PD(G, 2),
    WD_PD(G, 3),
    WD_PD(G, 4),
    WD_PD(G, 5),
    WD_PD(G, 6),
    WD_PD(G, 7),
    WD_PD(G, 8),
    WD_PD(G, 9),
    WD_PD(G, 10),
    WD_PD(G, 11),
    WD_PD(G, 12),
    WD_PD(G, 13),
    WD_PD(G, 14),
    WD_PD(G, 15),
    WD_PD(H, 0),
    WD_PD(H, 1),
    WD_PD(H, 2),
    WD_PD(H, 3),
    WD_PD(H, 4),
    WD_PD(H, 5),
    WD_PD(H, 6),
    WD_PD(H, 7),
    WD_PD(H, 8),
    WD_PD(H, 9),
    WD_PD(H, 10),
    WD_PD(H, 11),
    WD_PD(H, 12),
    WD_PD(H, 13),
    WD_PD(H, 14),
    WD_PD(H, 15),
    WD_PD(i, 0),
    WD_PD(i, 1),
    WD_PD(i, 2),
    WD_PD(i, 3),
    WD_PD(i, 4),
    WD_PD(i, 5),
    WD_PD(i, 6),
    WD_PD(i, 7),
    WD_PD(i, 8),
    WD_PD(i, 9),
    WD_PD(i, 10),
    WD_PD(i, 11),
    WD_PD(i, 12),
    WD_PD(i, 13),
    WD_PD(i, 14),
    WD_PD(i, 15),
    WD_PD(J, 0),
    WD_PD(J, 1),
    WD_PD(J, 2),
    WD_PD(J, 3),
    WD_PD(J, 4),
    WD_PD(J, 5),
    WD_PD(J, 6),
    WD_PD(J, 7),
    WD_PD(J, 8),
    WD_PD(J, 9),
    WD_PD(J, 10),
    WD_PD(J, 11),
    WD_PD(J, 12),
    WD_PD(J, 13),
    WD_PD(J, 14),
    WD_PD(J, 15),
    WD_PD(K, 0),
    WD_PD(K, 1),
    WD_PD(K, 2),
    WD_PD(K, 3),
    WD_PD(K, 4),
    WD_PD(K, 5),
    WD_PD(K, 6),
    WD_PD(K, 7),
    WD_PD(K, 8),
    WD_PD(K, 9),
    WD_PD(K, 10),
    WD_PD(K, 11),
    WD_PD(K, 12),
    WD_PD(K, 13),
    WD_PD(K, 14),
    WD_PD(K, 15),
    WD_PD(L, 0),
    WD_PD(L, 1),
    WD_PD(L, 2),
    WD_PD(L, 3),
    WD_PD(L, 4),
    WD_PD(L, 5),
    WD_PD(L, 6),
    WD_PD(L, 7),
    WD_PD(L, 8),
    WD_PD(L, 9),
    WD_PD(L, 10),
    WD_PD(L, 11),
    WD_PD(L, 12),
    WD_PD(L, 13),
    WD_PD(L, 14),
    WD_PD(L, 15),
    WD_PD(M, 0),
    WD_PD(M, 1),
    WD_PD(M, 2),
    WD_PD(M, 3),
    WD_PD(M, 4),
    WD_PD(M, 5),
    WD_PD(M, 6),
    WD_PD(M, 7),
    WD_PD(M, 8),
    WD_PD(M, 9),
    WD_PD(M, 10),
    WD_PD(M, 11),
    WD_PD(M, 12),
    WD_PD(M, 13),
    WD_PD(M, 14),
    WD_PD(M, 15),
    WD_PD(N, 0),
    WD_PD(N, 1),
    WD_PD(N, 2),
    WD_PD(N, 3),
    WD_PD(N, 4),
    WD_PD(N, 5),
    WD_PD(N, 6),
    WD_PD(N, 7),
    WD_PD(N, 8),
    WD_PD(N, 9),
    WD_PD(N, 10),
    WD_PD(N, 11),
    WD_PD(N, 12),
    WD_PD(N, 13),
    WD_PD(N, 14),
    WD_PD(N, 15),
    WD_PD(O, 0),
    WD_PD(O, 1),
    WD_PD(O, 2),
    WD_PD(O, 3),
    WD_PD(O, 4),
    WD_PD(O, 5),
    WD_PD(O, 6),
    WD_PD(O, 7),
    WD_PD(O, 8),
    WD_PD(O, 9),
    WD_PD(O, 10),
    WD_PD(O, 11),
    WD_PD(O, 12),
    WD_PD(O, 13),
    WD_PD(O, 14),
    WD_PD(O, 15),
    WD_PD(P, 0),
    WD_PD(P, 1),
    WD_PD(P, 2),
    WD_PD(P, 3),
    WD_PD(P, 4),
    WD_PD(P, 5),
    WD_PD(P, 6),
    WD_PD(P, 7),
    WD_PD(P, 8),
    WD_PD(P, 9),
    WD_PD(P, 10),
    WD_PD(P, 11),
    WD_PD(P, 12),
    WD_PD(P, 13),
    WD_PD(P, 14),
    WD_PD(P, 15),
#endif
#else
#define WD_GPIO_PIN_POS 0
#define WD_GPIO_PIN_MSK (0x1FUL << WD_GPIO_PIN_POS)
#define WD_GPIO_PIN(n)  ((n & 0x1F) << WD_GPIO_PIN_POS)

#define WD_GPIO_PORT_POS 5
#define WD_GPIO_PORT_MSK (0x07UL << WD_GPIO_PORT_POS)
#define WD_GPIO_PORT(n)  ((n & 0x07) << WD_GPIO_PORT_POS)

#if 0
  WD_PD(0,0), WD_PD(0,1), WD_PD(0,2), WD_PD(0,3), WD_PD(0,4), WD_PD(0,5), WD_PD(0,6), WD_PD(0,7), WD_PD(0,8), WD_PD(0,9), WD_PD(0,10), WD_PD(0,11), WD_PD(0,12), WD_PD(0,13), WD_PD(0,14), WD_PD(0,15), WD_PD(0,16), WD_PD(0,17), WD_PD(0,18), WD_PD(0,19), WD_PD(0,20), WD_PD(0,21), WD_PD(0,22), WD_PD(0,23), WD_PD(0,24), WD_PD(0,25), WD_PD(0,26), WD_PD(0,27), WD_PD(0,28), WD_PD(0,29), WD_PD(0,30), WD_PD(0,31),
  WD_PD(1,0), WD_PD(1,1), WD_PD(1,2), WD_PD(1,3), WD_PD(1,4), WD_PD(1,5), WD_PD(1,6), WD_PD(1,7), WD_PD(1,8), WD_PD(1,9), WD_PD(1,10), WD_PD(1,11), WD_PD(1,12), WD_PD(1,13), WD_PD(1,14), WD_PD(1,15), WD_PD(1,16), WD_PD(1,17), WD_PD(1,18), WD_PD(1,19), WD_PD(1,20), WD_PD(1,21), WD_PD(1,22), WD_PD(1,23), WD_PD(1,24), WD_PD(1,25), WD_PD(1,26), WD_PD(1,27), WD_PD(1,28), WD_PD(1,29), WD_PD(1,30), WD_PD(1,31),
  WD_PD(2,0), WD_PD(2,1), WD_PD(2,2), WD_PD(2,3), WD_PD(2,4), WD_PD(2,5), WD_PD(2,6), WD_PD(2,7), WD_PD(2,8), WD_PD(2,9), WD_PD(2,10), WD_PD(2,11), WD_PD(2,12), WD_PD(2,13), WD_PD(2,14), WD_PD(2,15), WD_PD(2,16), WD_PD(2,17), WD_PD(2,18), WD_PD(2,19), WD_PD(2,20), WD_PD(2,21), WD_PD(2,22), WD_PD(2,23), WD_PD(2,24), WD_PD(2,25), WD_PD(2,26), WD_PD(2,27), WD_PD(2,28), WD_PD(2,29), WD_PD(2,30), WD_PD(2,31),
  WD_PD(3,0), WD_PD(3,1), WD_PD(3,2), WD_PD(3,3), WD_PD(3,4), WD_PD(3,5), WD_PD(3,6), WD_PD(3,7), WD_PD(3,8), WD_PD(3,9), WD_PD(3,10), WD_PD(3,11), WD_PD(3,12), WD_PD(3,13), WD_PD(3,14), WD_PD(3,15), WD_PD(3,16), WD_PD(3,17), WD_PD(3,18), WD_PD(3,19), WD_PD(3,20), WD_PD(3,21), WD_PD(3,22), WD_PD(3,23), WD_PD(3,24), WD_PD(3,25), WD_PD(3,26), WD_PD(3,27), WD_PD(3,28), WD_PD(3,29), WD_PD(3,30), WD_PD(3,31),
  WD_PD(4,0), WD_PD(4,1), WD_PD(4,2), WD_PD(4,3), WD_PD(4,4), WD_PD(4,5), WD_PD(4,6), WD_PD(4,7), WD_PD(4,8), WD_PD(4,9), WD_PD(4,10), WD_PD(4,11), WD_PD(4,12), WD_PD(4,13), WD_PD(4,14), WD_PD(4,15), WD_PD(4,16), WD_PD(4,17), WD_PD(4,18), WD_PD(4,19), WD_PD(4,20), WD_PD(4,21), WD_PD(4,22), WD_PD(4,23), WD_PD(4,24), WD_PD(4,25), WD_PD(4,26), WD_PD(4,27), WD_PD(4,28), WD_PD(4,29), WD_PD(4,30), WD_PD(4,31),
  WD_PD(5,0), WD_PD(5,1), WD_PD(5,2), WD_PD(5,3), WD_PD(5,4), WD_PD(5,5), WD_PD(5,6), WD_PD(5,7), WD_PD(5,8), WD_PD(5,9), WD_PD(5,10), WD_PD(5,11), WD_PD(5,12), WD_PD(5,13), WD_PD(5,14), WD_PD(5,15), WD_PD(5,16), WD_PD(5,17), WD_PD(5,18), WD_PD(5,19), WD_PD(5,20), WD_PD(5,21), WD_PD(5,22), WD_PD(5,23), WD_PD(5,24), WD_PD(5,25), WD_PD(5,26), WD_PD(5,27), WD_PD(5,28), WD_PD(5,29), WD_PD(5,30), WD_PD(5,31),
  WD_PD(6,0), WD_PD(6,1), WD_PD(6,2), WD_PD(6,3), WD_PD(6,4), WD_PD(6,5), WD_PD(6,6), WD_PD(6,7), WD_PD(6,8), WD_PD(6,9), WD_PD(6,10), WD_PD(6,11), WD_PD(6,12), WD_PD(6,13), WD_PD(6,14), WD_PD(6,15), WD_PD(6,16), WD_PD(6,17), WD_PD(6,18), WD_PD(6,19), WD_PD(6,20), WD_PD(6,21), WD_PD(6,22), WD_PD(6,23), WD_PD(6,24), WD_PD(6,25), WD_PD(6,26), WD_PD(6,27), WD_PD(6,28), WD_PD(6,29), WD_PD(6,30), WD_PD(6,31),
  WD_PD(7,0), WD_PD(7,1), WD_PD(7,2), WD_PD(7,3), WD_PD(7,4), WD_PD(7,5), WD_PD(7,6), WD_PD(7,7), WD_PD(7,8), WD_PD(7,9), WD_PD(7,10), WD_PD(7,11), WD_PD(7,12), WD_PD(7,13), WD_PD(7,14), WD_PD(7,15), WD_PD(7,16), WD_PD(7,17), WD_PD(7,18), WD_PD(7,19), WD_PD(7,20), WD_PD(7,21), WD_PD(7,22), WD_PD(7,23), WD_PD(7,24), WD_PD(7,25), WD_PD(7,26), WD_PD(7,27), WD_PD(7,28), WD_PD(7,29), WD_PD(7,30), WD_PD(7,31),
#else
    WD_PD(A, 0),
    WD_PD(A, 1),
    WD_PD(A, 2),
    WD_PD(A, 3),
    WD_PD(A, 4),
    WD_PD(A, 5),
    WD_PD(A, 6),
    WD_PD(A, 7),
    WD_PD(A, 8),
    WD_PD(A, 9),
    WD_PD(A, 10),
    WD_PD(A, 11),
    WD_PD(A, 12),
    WD_PD(A, 13),
    WD_PD(A, 14),
    WD_PD(A, 15),
    WD_PD(A, 16),
    WD_PD(A, 17),
    WD_PD(A, 18),
    WD_PD(A, 19),
    WD_PD(A, 20),
    WD_PD(A, 21),
    WD_PD(A, 22),
    WD_PD(A, 23),
    WD_PD(A, 24),
    WD_PD(A, 25),
    WD_PD(A, 26),
    WD_PD(A, 27),
    WD_PD(A, 28),
    WD_PD(A, 29),
    WD_PD(A, 30),
    WD_PD(A, 31),
    WD_PD(B, 0),
    WD_PD(B, 1),
    WD_PD(B, 2),
    WD_PD(B, 3),
    WD_PD(B, 4),
    WD_PD(B, 5),
    WD_PD(B, 6),
    WD_PD(B, 7),
    WD_PD(B, 8),
    WD_PD(B, 9),
    WD_PD(B, 10),
    WD_PD(B, 11),
    WD_PD(B, 12),
    WD_PD(B, 13),
    WD_PD(B, 14),
    WD_PD(B, 15),
    WD_PD(B, 16),
    WD_PD(B, 17),
    WD_PD(B, 18),
    WD_PD(B, 19),
    WD_PD(B, 20),
    WD_PD(B, 21),
    WD_PD(B, 22),
    WD_PD(B, 23),
    WD_PD(B, 24),
    WD_PD(B, 25),
    WD_PD(B, 26),
    WD_PD(B, 27),
    WD_PD(B, 28),
    WD_PD(B, 29),
    WD_PD(B, 30),
    WD_PD(B, 31),
    WD_PD(C, 0),
    WD_PD(C, 1),
    WD_PD(C, 2),
    WD_PD(C, 3),
    WD_PD(C, 4),
    WD_PD(C, 5),
    WD_PD(C, 6),
    WD_PD(C, 7),
    WD_PD(C, 8),
    WD_PD(C, 9),
    WD_PD(C, 10),
    WD_PD(C, 11),
    WD_PD(C, 12),
    WD_PD(C, 13),
    WD_PD(C, 14),
    WD_PD(C, 15),
    WD_PD(C, 16),
    WD_PD(C, 17),
    WD_PD(C, 18),
    WD_PD(C, 19),
    WD_PD(C, 20),
    WD_PD(C, 21),
    WD_PD(C, 22),
    WD_PD(C, 23),
    WD_PD(C, 24),
    WD_PD(C, 25),
    WD_PD(C, 26),
    WD_PD(C, 27),
    WD_PD(C, 28),
    WD_PD(C, 29),
    WD_PD(C, 30),
    WD_PD(C, 31),
    WD_PD(D, 0),
    WD_PD(D, 1),
    WD_PD(D, 2),
    WD_PD(D, 3),
    WD_PD(D, 4),
    WD_PD(D, 5),
    WD_PD(D, 6),
    WD_PD(D, 7),
    WD_PD(D, 8),
    WD_PD(D, 9),
    WD_PD(D, 10),
    WD_PD(D, 11),
    WD_PD(D, 12),
    WD_PD(D, 13),
    WD_PD(D, 14),
    WD_PD(D, 15),
    WD_PD(D, 16),
    WD_PD(D, 17),
    WD_PD(D, 18),
    WD_PD(D, 19),
    WD_PD(D, 20),
    WD_PD(D, 21),
    WD_PD(D, 22),
    WD_PD(D, 23),
    WD_PD(D, 24),
    WD_PD(D, 25),
    WD_PD(D, 26),
    WD_PD(D, 27),
    WD_PD(D, 28),
    WD_PD(D, 29),
    WD_PD(D, 30),
    WD_PD(D, 31),
    WD_PD(E, 0),
    WD_PD(E, 1),
    WD_PD(E, 2),
    WD_PD(E, 3),
    WD_PD(E, 4),
    WD_PD(E, 5),
    WD_PD(E, 6),
    WD_PD(E, 7),
    WD_PD(E, 8),
    WD_PD(E, 9),
    WD_PD(E, 10),
    WD_PD(E, 11),
    WD_PD(E, 12),
    WD_PD(E, 13),
    WD_PD(E, 14),
    WD_PD(E, 15),
    WD_PD(E, 16),
    WD_PD(E, 17),
    WD_PD(E, 18),
    WD_PD(E, 19),
    WD_PD(E, 20),
    WD_PD(E, 21),
    WD_PD(E, 22),
    WD_PD(E, 23),
    WD_PD(E, 24),
    WD_PD(E, 25),
    WD_PD(E, 26),
    WD_PD(E, 27),
    WD_PD(E, 28),
    WD_PD(E, 29),
    WD_PD(E, 30),
    WD_PD(E, 31),
    WD_PD(F, 0),
    WD_PD(F, 1),
    WD_PD(F, 2),
    WD_PD(F, 3),
    WD_PD(F, 4),
    WD_PD(F, 5),
    WD_PD(F, 6),
    WD_PD(F, 7),
    WD_PD(F, 8),
    WD_PD(F, 9),
    WD_PD(F, 10),
    WD_PD(F, 11),
    WD_PD(F, 12),
    WD_PD(F, 13),
    WD_PD(F, 14),
    WD_PD(F, 15),
    WD_PD(F, 16),
    WD_PD(F, 17),
    WD_PD(F, 18),
    WD_PD(F, 19),
    WD_PD(F, 20),
    WD_PD(F, 21),
    WD_PD(F, 22),
    WD_PD(F, 23),
    WD_PD(F, 24),
    WD_PD(F, 25),
    WD_PD(F, 26),
    WD_PD(F, 27),
    WD_PD(F, 28),
    WD_PD(F, 29),
    WD_PD(F, 30),
    WD_PD(F, 31),
    WD_PD(G, 0),
    WD_PD(G, 1),
    WD_PD(G, 2),
    WD_PD(G, 3),
    WD_PD(G, 4),
    WD_PD(G, 5),
    WD_PD(G, 6),
    WD_PD(G, 7),
    WD_PD(G, 8),
    WD_PD(G, 9),
    WD_PD(G, 10),
    WD_PD(G, 11),
    WD_PD(G, 12),
    WD_PD(G, 13),
    WD_PD(G, 14),
    WD_PD(G, 15),
    WD_PD(G, 16),
    WD_PD(G, 17),
    WD_PD(G, 18),
    WD_PD(G, 19),
    WD_PD(G, 20),
    WD_PD(G, 21),
    WD_PD(G, 22),
    WD_PD(G, 23),
    WD_PD(G, 24),
    WD_PD(G, 25),
    WD_PD(G, 26),
    WD_PD(G, 27),
    WD_PD(G, 28),
    WD_PD(G, 29),
    WD_PD(G, 30),
    WD_PD(G, 31),
    WD_PD(H, 0),
    WD_PD(H, 1),
    WD_PD(H, 2),
    WD_PD(H, 3),
    WD_PD(H, 4),
    WD_PD(H, 5),
    WD_PD(H, 6),
    WD_PD(H, 7),
    WD_PD(H, 8),
    WD_PD(H, 9),
    WD_PD(H, 10),
    WD_PD(H, 11),
    WD_PD(H, 12),
    WD_PD(H, 13),
    WD_PD(H, 14),
    WD_PD(H, 15),
    WD_PD(H, 16),
    WD_PD(H, 17),
    WD_PD(H, 18),
    WD_PD(H, 19),
    WD_PD(H, 20),
    WD_PD(H, 21),
    WD_PD(H, 22),
    WD_PD(H, 23),
    WD_PD(H, 24),
    WD_PD(H, 25),
    WD_PD(H, 26),
    WD_PD(H, 27),
    WD_PD(H, 28),
    WD_PD(H, 29),
    WD_PD(H, 30),
    WD_PD(H, 31),
#endif
#endif

#undef WD_PD

} wd_gpio_pin_t;

typedef enum {
/*--------------- Value ---------------*/
#define WD_GPIO_VALUE_POS 0
#define WD_GPIO_VALUE_MSK (0x0FUL << WD_GPIO_VALUE_POS)
    WD_GPIO_CLEAR = (0UL << WD_GPIO_VALUE_POS), ///< The pin value to be set as clear (0)
    WD_GPIO_SET = (1UL << WD_GPIO_VALUE_POS),   ///< The pin value to be set as set (1)
    WD_GPIO_TOGGLE =
        (2UL << WD_GPIO_VALUE_POS), ///< The pin value to be set as set/clear toggled (^1)
    WD_GPIO_IGNORE = (3UL << WD_GPIO_VALUE_POS), ///< The pin is not connected or not GPIO.
    WD_GPIO_ON = (4UL << WD_GPIO_VALUE_POS),     ///< (gpio & WD_GPIO_ON_MSK) >> WD_GPIO_ON_POS
    WD_GPIO_OFF = (5UL << WD_GPIO_VALUE_POS),    ///< (gpio & WD_GPIO_OFF_MSK) >> WD_GPIO_OFF_POS


/*--------------- On ---------------*/
#define WD_GPIO_ON_POS     8
#define WD_GPIO_ON_MSK     (0x01UL << WD_GPIO_ON_POS)
#define WD_GPIO_DETECT_POS 8
#define WD_GPIO_DETECT_MSK (0x01UL << WD_GPIO_ON_POS)

/*--------------- Off ---------------*/
#define WD_GPIO_OFF_POS    9
#define WD_GPIO_OFF_MSK    (0x01UL << WD_GPIO_OFF_POS)
#define WD_GPIO_NORMAL_POS 9
#define WD_GPIO_NORMAL_MSK (0x01UL << WD_GPIO_NORMAL_POS)

#define WD_GPIO_ON_OFF_POS 8
#define WD_GPIO_0N_OFF_MSK (0x03UL << WD_GPIO_ON_OFF_POS)

#define WD_GPIO_ON_SET_OFF_CLEAR \
    ((WD_GPIO_SET << WD_GPIO_ON_POS) | (WD_GPIO_CLEAR << WD_GPIO_OFF_POS))
#define WD_GPIO_ON_CLEAR_OFF_SET \
    ((WD_GPIO_CLEAR << WD_GPIO_ON_POS) | (WD_GPIO_SET << WD_GPIO_OFF_POS))

#define WD_GPIO_DETECT_SET_NORMAL_CLEAR \
    ((WD_GPIO_SET << WD_GPIO_DETECT_POS) | (WD_GPIO_CLEAR << WD_GPIO_NORMAL_POS))
#define WD_GPIO_DETECT_CLEAR_NORMAL_SET \
    ((WD_GPIO_CLEAR << WD_GPIO_DETECT_POS) | (WD_GPIO_SET << WD_GPIO_NORMAL_POS))

/*--------------- Peripheral ---------------*/
#define WD_GPIO_PERIPHERAL_POS 10
#define WD_GPIO_PERIPHERAL_MSK (0x0FUL << WD_GPIO_PERIPHERAL_POS)
    WD_GPIO_PERIPHERAL_IGNORE = (0UL << WD_GPIO_PERIPHERAL_POS),  ///< Do not reference
    WD_GPIO_PERIPHERAL_GPIO = (1UL << WD_GPIO_PERIPHERAL_POS),    ///< The pin to be set as GPIO
    WD_GPIO_PERIPHERAL_USART = (2UL << WD_GPIO_PERIPHERAL_POS),   ///< The pin to be set as USART
    WD_GPIO_PERIPHERAL_ISO7816 = (3UL << WD_GPIO_PERIPHERAL_POS), ///< The pin to be set as ISO7816
    WD_GPIO_PERIPHERAL_SPI = (4UL << WD_GPIO_PERIPHERAL_POS),     ///< The pin to be set as SPI
    WD_GPIO_PERIPHERAL_I2C = (5UL << WD_GPIO_PERIPHERAL_POS),     ///< The pin to be set as I2C
    WD_GPIO_PERIPHERAL_CAN = (6UL << WD_GPIO_PERIPHERAL_POS),     ///< The pin to be set as CAN
    WD_GPIO_PERIPHERAL_USB = (7UL << WD_GPIO_PERIPHERAL_POS),     ///< The pin to be set as USB
    WD_GPIO_PERIPHERAL_ETH = (8UL << WD_GPIO_PERIPHERAL_POS),     ///< The pin to be set as ETH

/*--------------- Direction ---------------*/
#define WD_GPIO_DIR_POS 14
#define WD_GPIO_DIR_MSK (0x03UL << WD_GPIO_DIR_POS)
    WD_GPIO_DIR_IGNORE = (0UL << WD_GPIO_DIR_POS), ///< Do not reference
    WD_GPIO_DIR_INPUT = (1UL << WD_GPIO_DIR_POS),  ///< The pin direction to be set as input
    WD_GPIO_DIR_OUTPUT = (2UL << WD_GPIO_DIR_POS), ///< The pin direction to be set as output
    WD_GPIO_DIR_PERIPHERAL =
        (3UL << WD_GPIO_DIR_POS), ///< The pin direction to be set as peripheral

/*--------------- Type ---------------*/
#define WD_GPIO_TYPE_POS 16
#define WD_GPIO_TYPE_MSK (0x0FUL << WD_GPIO_TYPE_POS)
    WD_GPIO_TYPE_IGNORE = (0UL << WD_GPIO_TYPE_POS),   ///< Do not reference
    WD_GPIO_TYPE_DEFAULT = (1UL << WD_GPIO_TYPE_POS),  ///< The pin type to be set as default
    WD_GPIO_TYPE_FLOATING = (2UL << WD_GPIO_TYPE_POS), ///< The pin type to be set as floating
    WD_GPIO_TYPE_SCHMITT_TRIGGER =
        (3UL << WD_GPIO_TYPE_POS),                   ///< The pin type to be set as schmitt trigger
    WD_GPIO_TYPE_ANALOG = (4UL << WD_GPIO_TYPE_POS), ///< The pin type to be set as analog
    WD_GPIO_TYPE_PUSH_PULL = (5UL << WD_GPIO_TYPE_POS),  ///< The pin type to be set as Push Pull
    WD_GPIO_TYPE_OPEN_DRAIN = (6UL << WD_GPIO_TYPE_POS), ///< The pin type to be set as Open Drain

    WD_GPIO_INTERRUPT_IGNORE = (7UL << WD_GPIO_TYPE_POS), ///< Do not reference
    WD_GPIO_INTERRUPT_RISING_EDGE =
        (8UL << WD_GPIO_TYPE_POS), ///< The pin to be set rising edge trigger interrupt
    WD_GPIO_INTERRUPT_FALLING_EDGE =
        (9UL << WD_GPIO_TYPE_POS), ///< The pin to be set falling edge trigger interrupt
    WD_GPIO_INTERRUPT_BOTH_EDGE =
        (10UL << WD_GPIO_TYPE_POS), ///< The pin to be set rising & falling edge trigger interrupt

/*--------------- Speed ---------------*/
#define WD_GPIO_SPEED_POS 20
#define WD_GPIO_SPEED_MSK (0x03UL << WD_GPIO_SPEED_POS)
    WD_GPIO_SPEED_IGNORE = (0UL << WD_GPIO_SPEED_POS),  ///< Do not reference
    WD_GPIO_SPEED_DEFAULT = (1UL << WD_GPIO_SPEED_POS), ///< I/O Speed to be default
    WD_GPIO_SPEED_LOW = (2UL << WD_GPIO_SPEED_POS),     ///< I/O Speed to be low
    WD_GPIO_SPEED_HIGH = (3UL << WD_GPIO_SPEED_POS),    ///< I/O Speed to be high

/*--------------- On-die Termination ---------------*/
#define WD_GPIO_PULL_POS 22
#define WD_GPIO_PULL_MSK (0x03UL << WD_GPIO_PULL_POS)
    WD_GPIO_PULL_IGNORE = (0UL << WD_GPIO_PULL_POS), ///< Do not reference
    WD_GPIO_PULL_NONE = (1UL << WD_GPIO_PULL_POS),   ///< The pin pull register to be set as none
    WD_GPIO_PULL_UP = (2UL << WD_GPIO_PULL_POS),     ///< The pin pull register to be set as up
    WD_GPIO_PULL_DOWN = (3UL << WD_GPIO_PULL_POS),   ///< The pin pull register to be set as down

/*--------------- Function ---------------*/
#define WD_GPIO_FUNCTION_POS 24
#define WD_GPIO_FUNCTION_MSK (0xFFUL << WD_GPIO_FUNCTION_POS)
#define WD_GPIO_FUNCTION(n)  (((uint32_t)n) << WD_GPIO_FUNCTION_POS)
    WD_GPIO_FUNCTION_AUTO =
        (127UL << WD_GPIO_FUNCTION_POS), ///< The pin to be set function use wd_gpio_t
    WD_GPIO_FUNCTION_0 = 0UL,            ///< The pin to be set function 0
    WD_GPIO_FUNCTION_1 = 1UL,            ///< The pin to be set function 1
    WD_GPIO_FUNCTION_2 = 2UL,            ///< The pin to be set function 2
    WD_GPIO_FUNCTION_3 = 3UL,            ///< The pin to be set function 3
    WD_GPIO_FUNCTION_4 = 4UL,            ///< The pin to be set function 4
    WD_GPIO_FUNCTION_5 = 5UL,            ///< The pin to be set function 5
    WD_GPIO_FUNCTION_6 = 6UL,            ///< The pin to be set function 6
    WD_GPIO_FUNCTION_7 = 7UL,            ///< The pin to be set function 7
    WD_GPIO_FUNCTION_8 = 8UL,            ///< The pin to be set function 8
    WD_GPIO_FUNCTION_9 = 9UL,            ///< The pin to be set function 9
    WD_GPIO_FUNCTION_10 = 10UL,          ///< The pin to be set function 10
    WD_GPIO_FUNCTION_11 = 11UL,          ///< The pin to be set function 11
    WD_GPIO_FUNCTION_12 = 12UL,          ///< The pin to be set function 12
    WD_GPIO_FUNCTION_13 = 13UL,          ///< The pin to be set function 13
    WD_GPIO_FUNCTION_14 = 14UL,          ///< The pin to be set function 14
    WD_GPIO_FUNCTION_15 = 15UL,          ///< The pin to be set function 15

/*--------------- Function GPIO ---------------*/
#define WD_GPIO_FUNC_GPIO_POS 24
#define WD_GPIO_FUNC_GPIO_MSK (0x0FUL << WD_GPIO_FUNC_GPIO_POS)
#define WD_GPIO_FUNC_GPIO(n)  ((n) << WD_GPIO_FUNC_GPIO_POS)
    WD_GPIO_FUNC_GPIO_ALT_0 =
        (0UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 0
    WD_GPIO_FUNC_GPIO_ALT_1 =
        (1UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 1
    WD_GPIO_FUNC_GPIO_ALT_2 =
        (2UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 2
    WD_GPIO_FUNC_GPIO_ALT_3 =
        (3UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 3
    WD_GPIO_FUNC_GPIO_ALT_4 =
        (4UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 4
    WD_GPIO_FUNC_GPIO_ALT_5 =
        (5UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 5
    WD_GPIO_FUNC_GPIO_ALT_6 =
        (6UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 6
    WD_GPIO_FUNC_GPIO_ALT_7 =
        (7UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 7
    WD_GPIO_FUNC_GPIO_ALT_8 =
        (8UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 8
    WD_GPIO_FUNC_GPIO_ALT_9 =
        (9UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 9
    WD_GPIO_FUNC_GPIO_ALT_10 =
        (10UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 10
    WD_GPIO_FUNC_GPIO_ALT_11 =
        (11UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 11
    WD_GPIO_FUNC_GPIO_ALT_12 =
        (12UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 12
    WD_GPIO_FUNC_GPIO_ALT_13 =
        (13UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 13
    WD_GPIO_FUNC_GPIO_ALT_14 =
        (14UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 14
    WD_GPIO_FUNC_GPIO_ALT_15 =
        (15UL << WD_GPIO_FUNC_GPIO_POS), ///< The pin to be set Alternative function 15

    WD_GPIO_FUNC_GPIO_ST_DEFAULT = WD_GPIO_FUNC_GPIO_ALT_0,
    WD_GPIO_FUNC_GPIO_ST_ALT_0 = WD_GPIO_FUNC_GPIO_ALT_1,
    WD_GPIO_FUNC_GPIO_ST_ALT_1 = WD_GPIO_FUNC_GPIO_ALT_2,

/*--------------- Function Peripheral ---------------*/
#define WD_GPIO_FUNC_PERI_POS 28
#define WD_GPIO_FUNC_PERI_MSK (0x0FUL << WD_GPIO_FUNC_PERI_POS)
#define WD_GPIO_FUNC_PERI(n)  ((n) << WD_GPIO_FUNC_PERI_POS)
    WD_GPIO_FUNC_PERI_ALT_0 =
        (int32_t)((0UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 0
    WD_GPIO_FUNC_PERI_ALT_1 =
        (int32_t)((1UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 1
    WD_GPIO_FUNC_PERI_ALT_2 =
        (int32_t)((2UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 2
    WD_GPIO_FUNC_PERI_ALT_3 =
        (int32_t)((3UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 3
    WD_GPIO_FUNC_PERI_ALT_4 =
        (int32_t)((4UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 4
    WD_GPIO_FUNC_PERI_ALT_5 =
        (int32_t)((5UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 5
    WD_GPIO_FUNC_PERI_ALT_6 =
        (int32_t)((6UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 6
    WD_GPIO_FUNC_PERI_ALT_7 =
        (int32_t)((7UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 7
    WD_GPIO_FUNC_PERI_ALT_8 =
        (int32_t)((8UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 8
    WD_GPIO_FUNC_PERI_ALT_9 =
        (int32_t)((9UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 9
    WD_GPIO_FUNC_PERI_ALT_10 =
        (int32_t)((10UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 10
    WD_GPIO_FUNC_PERI_ALT_11 =
        (int32_t)((11UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 11
    WD_GPIO_FUNC_PERI_ALT_12 =
        (int32_t)((12UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 12
    WD_GPIO_FUNC_PERI_ALT_13 =
        (int32_t)((13UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 13
    WD_GPIO_FUNC_PERI_ALT_14 =
        (int32_t)((14UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 14
    WD_GPIO_FUNC_PERI_ALT_15 =
        (int32_t)((15UL << WD_GPIO_FUNC_PERI_POS)), ///< The pin to be set Alternative function 15

    WD_GPIO_FUNC_PERI_ST_DEFAULT = WD_GPIO_FUNC_PERI_ALT_0,
    WD_GPIO_FUNC_PERI_ST_ALT_0 = WD_GPIO_FUNC_PERI_ALT_1,
    WD_GPIO_FUNC_PERI_ST_ALT_1 = WD_GPIO_FUNC_PERI_ALT_2,

    /*--------------- Pre-defined ---------------*/
    WD_GPIO_NC = (int32_t)0xFFFFFFFFU,
    WD_GPIO_INPUT_PULL_NONE = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_INPUT | WD_GPIO_TYPE_FLOATING |
        WD_GPIO_SPEED_DEFAULT | WD_GPIO_PULL_NONE | WD_GPIO_FUNC_GPIO_ALT_0 |
        WD_GPIO_FUNC_PERI_ALT_0,
    WD_GPIO_INPUT_PULL_UP = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_INPUT | WD_GPIO_TYPE_FLOATING |
        WD_GPIO_SPEED_DEFAULT | WD_GPIO_PULL_UP | WD_GPIO_FUNC_GPIO_ALT_0 | WD_GPIO_FUNC_PERI_ALT_0,
    WD_GPIO_INPUT_PULL_DOWN = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_INPUT | WD_GPIO_TYPE_FLOATING |
        WD_GPIO_SPEED_DEFAULT | WD_GPIO_PULL_DOWN | WD_GPIO_FUNC_GPIO_ALT_0 |
        WD_GPIO_FUNC_PERI_ALT_0,

    WD_GPIO_INPUT_ST_PULL_NONE = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_INPUT |
        WD_GPIO_TYPE_SCHMITT_TRIGGER | WD_GPIO_SPEED_DEFAULT | WD_GPIO_PULL_NONE |
        WD_GPIO_FUNC_GPIO_ALT_0 | WD_GPIO_FUNC_PERI_ALT_0,
    WD_GPIO_INPUT_ST_PULL_UP = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_INPUT |
        WD_GPIO_TYPE_SCHMITT_TRIGGER | WD_GPIO_SPEED_DEFAULT | WD_GPIO_PULL_UP |
        WD_GPIO_FUNC_GPIO_ALT_0 | WD_GPIO_FUNC_PERI_ALT_0,
    WD_GPIO_INPUT_ST_PULL_DOWN = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_INPUT |
        WD_GPIO_TYPE_SCHMITT_TRIGGER | WD_GPIO_SPEED_DEFAULT | WD_GPIO_PULL_DOWN |
        WD_GPIO_FUNC_GPIO_ALT_0 | WD_GPIO_FUNC_PERI_ALT_0,

    WD_GPIO_OUTPUT_PP_PULL_NONE = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_OUTPUT |
        WD_GPIO_TYPE_PUSH_PULL | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_NONE | WD_GPIO_FUNC_GPIO_ALT_0 |
        WD_GPIO_FUNC_PERI_ALT_0,
    WD_GPIO_OUTPUT_PP_PULL_UP = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_OUTPUT |
        WD_GPIO_TYPE_PUSH_PULL | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_UP | WD_GPIO_FUNC_GPIO_ALT_0 |
        WD_GPIO_FUNC_PERI_ALT_0,
    WD_GPIO_OUTPUT_PP_PULL_DOWN = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_OUTPUT |
        WD_GPIO_TYPE_PUSH_PULL | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_DOWN | WD_GPIO_FUNC_GPIO_ALT_0 |
        WD_GPIO_FUNC_PERI_ALT_0,

    WD_GPIO_OUTPUT_OD_PULL_NONE = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_OUTPUT |
        WD_GPIO_TYPE_OPEN_DRAIN | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_NONE | WD_GPIO_FUNC_GPIO_ALT_0 |
        WD_GPIO_FUNC_PERI_ALT_0,
    WD_GPIO_OUTPUT_OD_PULL_UP = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_OUTPUT |
        WD_GPIO_TYPE_OPEN_DRAIN | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_UP | WD_GPIO_FUNC_GPIO_ALT_0 |
        WD_GPIO_FUNC_PERI_ALT_0,
    WD_GPIO_OUTPUT_OD_PULL_DOWN = WD_GPIO_PERIPHERAL_GPIO | WD_GPIO_DIR_OUTPUT |
        WD_GPIO_TYPE_OPEN_DRAIN | WD_GPIO_SPEED_HIGH | WD_GPIO_PULL_DOWN | WD_GPIO_FUNC_GPIO_ALT_0 |
        WD_GPIO_FUNC_PERI_ALT_0,
} wd_gpio_control_table_t;

typedef struct {
    wd_gpio_pin_t Pin;

    uint32_t On : 1;
    uint32_t Off : 1;
    uint32_t Peripheral : 4;
    uint32_t Direction : 2;

    uint32_t Type : 4;
    uint32_t Speed : 2;
    uint32_t Pull : 2;

    uint32_t Func_GPIO : 4;
    uint32_t Func_Peri : 4;
} wd_gpio_t;

#define WD_GPIO_DATA(value)                                                       \
    {                                                                             \
        .Pin = (wd_gpio_pin_t)(value & WD_GPIO_PIN_MSK),                          \
        .On = (value & WD_GPIO_ON_MSK) >> WD_GPIO_ON_POS,                         \
        .Off = (value & WD_GPIO_OFF_MSK) >> WD_GPIO_OFF_POS,                      \
        .Peripheral = (value & WD_GPIO_PERIPHERAL_MSK) >> WD_GPIO_PERIPHERAL_POS, \
        .Direction = (value & WD_GPIO_DIR_MSK) >> WD_GPIO_DIR_POS,                \
        .Type = (value & WD_GPIO_TYPE_MSK) >> WD_GPIO_TYPE_POS,                   \
        .Speed = (value & WD_GPIO_SPEED_MSK) >> WD_GPIO_SPEED_POS,                \
        .Pull = (value & WD_GPIO_PULL_MSK) >> WD_GPIO_PULL_POS,                   \
        .Func_GPIO = (value & WD_GPIO_FUNC_GPIO_MSK) >> WD_GPIO_FUNC_GPIO_POS,    \
        .Func_Peri = (value & WD_GPIO_FUNC_PERI_MSK) >> WD_GPIO_FUNC_PERI_POS,    \
    }
#define WD_GPIO_VAlUE_ON(gpio)  ((gpio & WD_GPIO_ON_MSK) >> WD_GPIO_ON_POS)
#define WD_GPIO_VAlUE_OFF(gpio) ((gpio & WD_GPIO_OFF_MSK) >> WD_GPIO_OFF_POS)
#define WD_GPIO_DETECT(gpio)    ((gpio & WD_GPIO_DETECT_MSK) >> WD_GPIO_DETECT_POS)
#define WD_GPIO_NORMAL(gpio)    ((gpio & WD_GPIO_NORMAL_MSK) >> WD_GPIO_NORMAL_POS)

typedef struct {
    uint32_t Value : 4;
    uint32_t Interrupt : 2;
    uint32_t : 2;

    uint32_t On : 1;
    uint32_t Off : 1;
    uint32_t Peripheral : 4;
    uint32_t Direction : 2;

    uint32_t Type : 4;
    uint32_t Speed : 2;
    uint32_t Pull : 2;

    uint32_t Func_GPIO : 4;
    uint32_t Func_Peri : 4;
} wd_gpio_control_t;

#if 0 // port0~ 일 경우
#define WD_PORT_NC     NULL
#define WD_GPIO_PORT_0 GPIO0
#define WD_GPIO_PORT_1 GPIO1
#define WD_GPIO_PORT_2 GPIO2
#define WD_GPIO_PORT_3 GPIO3
#define WD_GPIO_PORT_4 GPIO4
#define WD_GPIO_PORT_5 GPIO5
#define WD_GPIO_PORT_6 GPIO6
#define WD_GPIO_PORT_7 GPIO7
#endif

#if 1
#define WD_PORT_NC     NULL
#define WD_GPIO_PORT_0 GPIOA
#define WD_GPIO_PORT_1 GPIOB
#define WD_GPIO_PORT_2 GPIOC
#define WD_GPIO_PORT_3 GPIOD
#define WD_GPIO_PORT_4 GPIOE
#define WD_GPIO_PORT_5 GPIOF
#define WD_GPIO_PORT_6 GPIOG
#define WD_GPIO_PORT_7 GPIOH

#endif


#if 1
#define WD_GPIO_PORT_A WD_GPIO_PORT_0
#define WD_GPIO_PORT_B WD_GPIO_PORT_1
#define WD_GPIO_PORT_C WD_GPIO_PORT_2
#define WD_GPIO_PORT_D WD_GPIO_PORT_3
#define WD_GPIO_PORT_E WD_GPIO_PORT_4
#define WD_GPIO_PORT_F WD_GPIO_PORT_5
#define WD_GPIO_PORT_G WD_GPIO_PORT_6
#define WD_GPIO_PORT_H WD_GPIO_PORT_7
#define WD_PORT_I
#endif


#define WD_PIN_NC          NULL
#define WD_GPIO_PIN_BIT_0  WD_BIT0
#define WD_GPIO_PIN_BIT_1  WD_BIT1
#define WD_GPIO_PIN_BIT_2  WD_BIT2
#define WD_GPIO_PIN_BIT_3  WD_BIT3
#define WD_GPIO_PIN_BIT_4  WD_BIT4
#define WD_GPIO_PIN_BIT_5  WD_BIT5
#define WD_GPIO_PIN_BIT_6  WD_BIT6
#define WD_GPIO_PIN_BIT_7  WD_BIT7
#define WD_GPIO_PIN_BIT_8  WD_BIT8
#define WD_GPIO_PIN_BIT_9  WD_BIT9
#define WD_GPIO_PIN_BIT_10 WD_BIT10
#define WD_GPIO_PIN_BIT_11 WD_BIT11
#define WD_GPIO_PIN_BIT_12 WD_BIT12
#define WD_GPIO_PIN_BIT_13 WD_BIT13
#define WD_GPIO_PIN_BIT_14 WD_BIT14
#define WD_GPIO_PIN_BIT_15 WD_BIT15
#define WD_GPIO_PIN_BIT_16 WD_BIT16
#define WD_GPIO_PIN_BIT_17 WD_BIT17
#define WD_GPIO_PIN_BIT_18 WD_BIT18
#define WD_GPIO_PIN_BIT_19 WD_BIT19
#define WD_GPIO_PIN_BIT_20 WD_BIT20
#define WD_GPIO_PIN_BIT_21 WD_BIT21
#define WD_GPIO_PIN_BIT_22 WD_BIT22
#define WD_GPIO_PIN_BIT_23 WD_BIT23
#define WD_GPIO_PIN_BIT_24 WD_BIT24
#define WD_GPIO_PIN_BIT_25 WD_BIT25
#define WD_GPIO_PIN_BIT_26 WD_BIT26
#define WD_GPIO_PIN_BIT_27 WD_BIT27
#define WD_GPIO_PIN_BIT_28 WD_BIT28
#define WD_GPIO_PIN_BIT_29 WD_BIT29
#define WD_GPIO_PIN_BIT_30 WD_BIT30
#define WD_GPIO_PIN_BIT_31 WD_BIT31
//#define WD_PIN_ALL                                                              WD_BIT_ALL
#define WD_PIN_ALL GPIO_PIN_All

#define WD_ALT_NC NULL
#define WD_ALT_0  WD_BIT0
#define WD_ALT_1  WD_BIT1
#define WD_ALT_2  WD_BIT2
#define WD_ALT_3  WD_BIT3
#define WD_ALT_4  WD_BIT4
#define WD_ALT_5  WD_BIT5
#define WD_ALT_6  WD_BIT6
#define WD_ALT_7  WD_BIT7

int32_t wd_gpio_config(uint32_t gpio, uint32_t config);
void wd_gpio_write(uint32_t gpio, uint32_t value);
int32_t wd_gpio_read(uint32_t gpio);


#endif /* WD_CORE_WD_GPIO_H */
