/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    wd_version.h
 * @brief   WD Library version (Semantic Versioning : MAJOR.MINOR.PATCH)
 *
 *  MAJOR : incompatible API / config change (project must be updated)
 *  MINOR : new module / feature, backward compatible
 *  PATCH : bug fix, backward compatible
 *  History : see CHANGELOG.md at repository root
 ******************************************************************************/
#ifndef WD_CORE_WD_VERSION_H
#define WD_CORE_WD_VERSION_H

#define WD_LIB_VERSION_MAJOR 2
#define WD_LIB_VERSION_MINOR 0
#define WD_LIB_VERSION_PATCH 0

#define WD_LIB_VERSION \
    ((WD_LIB_VERSION_MAJOR << 16) | (WD_LIB_VERSION_MINOR << 8) | (WD_LIB_VERSION_PATCH))
#define WD_LIB_VERSION_STRING "2.0.0"

/* Usage in project : #if (WD_LIB_VERSION < WD_LIB_VERSION_CHECK(1,1,0)) #error "WD Library >= 1.1.0 required" #endif */
#define WD_LIB_VERSION_CHECK(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))

#endif /* WD_CORE_WD_VERSION_H */
