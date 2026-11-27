/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    _main.h   (TEMPLATE - project owned)
 * @brief   Project system entry : initialization sequence prototypes
 *
 *  Project tree (example)
 *    Core/Inc, Core/Src      : CubeMX generated
 *    Core/_Config            : wd_config.h, wd_board.h, wd_memory_map.h (from WD_Library/_WD_Config_Template)
 *    Core/_System            : this folder (from Template/Core/_System)
 *    Core/_Application       : project application (App_Initialize / App_Process ...)
 *    WD_Library              : shared library (do not edit in project - update from library repository)
 ******************************************************************************/
#ifndef WD_TEMPLATE_MAIN_H
#define WD_TEMPLATE_MAIN_H

#include "wd.h"
#include "_version.h"
#include "_Thread.h"

extern int _read(int file, char *ptr, int len);
extern int _write(int file, char *ptr, int len);

void Software_Interface_Initialization(void);
void Device_Initialization(void);
void Interrupt_Initialization(void);
void Service_Initialization(void);
void Application_Initialization(void);
void Component_Initialization(void);

#endif /* WD_TEMPLATE_MAIN_H */
