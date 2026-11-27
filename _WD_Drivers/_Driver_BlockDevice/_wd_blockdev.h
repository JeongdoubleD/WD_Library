/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _Driver_Blcok_Device.h
 *
 *  Created on: Feb 7, 2025
 *      Author: doubled
 */

#ifndef WD_DRIVERS_WD_BLOCKDEV_H
#define WD_DRIVERS_WD_BLOCKDEV_H

#include "wd_core.h"

#if 1 // 토글

#ifndef RTE_FileSystem_Core
/// Driver DeviceCtrl parameter code.
typedef enum _fsDevCtrlCode {
    fsDevCtrlCodeCheckMedia = 0, ///< Check Media Status
    fsDevCtrlCodeControlMedia,   ///< Control Media Device
    fsDevCtrlCodeFormat,         ///< Format Media at low level
    fsDevCtrlCodeSerial,         ///< Return device serial number
    fsDevCtrlCodeGetCID,         ///< Read Memory Card CID Register
    fsDevCtrlCodeLockUnlock,     ///< Manage device password protection
    fsDevCtrlCodeHealthStatus    ///< Access device health status (S.M.A.R.T)
} fsDevCtrlCode;

/// File System return codes.
typedef enum {
    fsOK = 0,             ///< Operation succeeded
    fsError,              ///< Unspecified error
    fsUnsupported,        ///< Operation not supported
    fsAccessDenied,       ///< Resource access denied
    fsInvalidParameter,   ///< Invalid parameter specified
    fsInvalidDrive,       ///< Nonexistent drive
    fsInvalidPath,        ///< Invalid path specified
    fsUninitializedDrive, ///< Drive is uninitialized
    fsDriverError,        ///< Read/write error
    fsMediaError,         ///< Media error
    fsNoMedia,            ///< No media, or not initialized
    fsNoFileSystem,       ///< File system is not formatted
    fsNoFreeSpace,        ///< No free space available
    fsFileNotFound,       ///< Requested file not found
    fsDirNotEmpty,        ///< The directory is not empty
    fsTooManyOpenFiles,   ///< Too many open files
    fsAlreadyExists,      ///< File or directory already exists
    fsNotDirectory        ///< Path is not a directory
} fsStatus;
#endif

#define WD_BLOCK_DEVICE_ARGUMENT_WRITE 1
#define WD_BLOCK_DEVICE_ARGUMENT_ERASE 0
#define WD_BLOCK_DEVICE_RETRY          3

typedef enum wdif_block_device_control_e {
/*--------------- Objects ---------------*/
#define WDIF_BLOCK_DEVICE_OBJECT_POS 0
#define WDIF_BLOCK_DEVICE_OBJECT_MSK (0xFFUL << WDIF_BLOCK_DEVICE_OBJECT_POS)
    WDIF_BLOCK_DEVICE_OBJECT_AUTO = (0UL << WDIF_BLOCK_DEVICE_OBJECT_POS),
    WDIF_BLOCK_DEVICE_OBJECT_RAM = (1UL << WDIF_BLOCK_DEVICE_OBJECT_POS),
    WDIF_BLOCK_DEVICE_OBJECT_ROM = (2UL << WDIF_BLOCK_DEVICE_OBJECT_POS),
    WDIF_BLOCK_DEVICE_OBJECT_NVM_FLASH = (3UL << WDIF_BLOCK_DEVICE_OBJECT_POS),
    WDIF_BLOCK_DEVICE_OBJECT_NVM_SERIAL_EEPROM = (4UL << WDIF_BLOCK_DEVICE_OBJECT_POS),
    WDIF_BLOCK_DEVICE_OBJECT_NONE = (100UL << WDIF_BLOCK_DEVICE_OBJECT_POS),
    WDIF_BLOCK_DEVICE_OBJECT_ALL = (0xFFUL << WDIF_BLOCK_DEVICE_OBJECT_POS),

/*--------------- Command ---------------*/
#define WDIF_BLOCK_DEVICE_COMMAND_POS 8
#define WDIF_BLOCK_DEVICE_COMMAND_MSK (0xFFUL << WDIF_BLOCK_DEVICE_COMMAND_POS)
    WDIF_BLOCK_DEVICE_COMMAND_WRITE =
        (1UL << WDIF_BLOCK_DEVICE_COMMAND_POS), // arg = WDIF_PARAMETERS Pointer
    WDIF_BLOCK_DEVICE_COMMAND_READ =
        (2UL << WDIF_BLOCK_DEVICE_COMMAND_POS), // arg = WDIF_PARAMETERS Pointer
    WDIF_BLOCK_DEVICE_COMMAND_ERASE =
        (3UL << WDIF_BLOCK_DEVICE_COMMAND_POS), // arg = Address (1 Sector)
    WDIF_BLOCK_DEVICE_COMMAND_ERASE_0xFF =
        (4UL << WDIF_BLOCK_DEVICE_COMMAND_POS), // arg = Address (1 Sector)
    WDIF_BLOCK_DEVICE_COMMAND_ERASE_0xFF_BULK =
        (5UL << WDIF_BLOCK_DEVICE_COMMAND_POS), // arg = Address (1 Sector)
    WDIF_BLOCK_DEVICE_COMMAND_INSTANCE_RESET =
        (100UL << WDIF_BLOCK_DEVICE_COMMAND_POS),                                    // arg = NULL
    WDIF_BLOCK_DEVICE_COMMAND_VALIDATION = (200UL << WDIF_BLOCK_DEVICE_COMMAND_POS), // arg = NULL

/*--------------- Control ---------------*/
#define WDIF_BLOCK_DEVICE_CONTROL_POS 16
#define WDIF_BLOCK_DEVICE_CONTROL_MSK (0x0FUL << WDIF_BLOCK_DEVICE_CONTROL_POS)

/*--------------- Mode ---------------*/
#define WDIF_BLOCK_DEVICE_MODE_POS 20
#define WDIF_BLOCK_DEVICE_MODE_MSK (0x0FUL << WDIF_BLOCK_DEVICE_MODE_POS)
#define WDIF_BLOCK_DEVICE_MODE(n)  (((n) & 0x0F) << WDIF_BLOCK_DEVICE_MODE_POS)

/*--------------- Argument ---------------*/
#define WDIF_BLOCK_DEVICE_ARGUMENT_POS 24
#define WDIF_BLOCK_DEVICE_ARGUMENT_MSK (0xFFUL << WDIF_BLOCK_DEVICE_ARGUMENT_POS)
    WDIF_BLOCK_DEVICE_ARGUMENT_CRC = ((1UL << 0) << WDIF_BLOCK_DEVICE_ARGUMENT_POS), // Write, Read
    WDIF_BLOCK_DEVICE_ARGUMENT_CONFIRM_WRITE = ((1UL << 1) << WDIF_BLOCK_DEVICE_ARGUMENT_POS),
    WDIF_BLOCK_DEVICE_ARGUMENT_INIT = ((1UL << 2) << WDIF_BLOCK_DEVICE_ARGUMENT_POS),
    WDIF_BLOCK_DEVICE_ARGUMENT_EXTERNAL_PTR = ((1UL << 3) << WDIF_BLOCK_DEVICE_ARGUMENT_POS),
    WDIF_BLOCK_DEVICE_ARGUMENT_NONE =
        ((1UL << 4) << WDIF_BLOCK_DEVICE_ARGUMENT_POS), // 25.05.16 add

    /*--------------- Miscellaneous ---------------*/
} wd_blockdev_control_t;

typedef enum wdif_block_device_status_e {
    WDIF_BLOCK_DEVICE_STATUS_BUSY = (1UL << 0),
    WDIF_BLOCK_DEVICE_STATUS_ERROR = (1UL << 1),
} wd_blockdev_status_t;


typedef enum wdif_block_device_flag_e {
#if 0
  WD_FLAG_INITIALIZED             = (1UL <<  0),
  WD_FLAG_POWERED                 = (1UL <<  1),
  WD_FLAG_CONFIGURED              = (1UL <<  2),
#endif
    WD_BLOCK_DEVICE_FLAG_SPECIFIC = (1UL << 3),
} wd_blockdev_flag_t;

typedef enum wdif_block_device_event_e {
    WDIF_BLOCK_DEVICE_EVENT_RESERVED = (1UL << 0),
} wd_blockdev_event_t;

typedef struct {
    char *Name;

    uint32_t ID;
    uint32_t Addr;
    uint32_t Size;

    uint32_t Program_Size;
    uint32_t Erase_Sector_Size;
    uint32_t Erase_Block_Size;
    uint32_t Erase_Block_Addr;

    uint32_t Program_TimeOut;      // [us]
    uint32_t Erase_Sector_TimeOut; // [us]
    uint32_t Erase_Block_TimeOut;  // [us]
    uint32_t Erase_Chip_TimeOut;   // [us]

    uint8_t *Buf;
    uint32_t Buf_Size;
    uint8_t *Confirm_Buf;
    uint32_t Confirm_Buf_Size;

    uint32_t
        Direct_Access; // 1 : memory mapped (internal flash) - read / compare directly, no work buffer
} const wd_blockdev_instance_capabilities_t;

typedef struct wdif_bd_instance_xfer_s {
    uint8_t *Buf;
    uint32_t Buf_Size;
    uint8_t *Confirm_Buf;
    uint32_t Confirm_Buf_Size;
} wd_blockdev_instance_xfer_t;

typedef struct {
    void *Owner;
    uint32_t Time_Stamp;
    uint32_t Time_Wait;
    uint32_t Timeout;
    uint32_t Timer;

    wd_event_callback_t CB_Event;
    uint32_t volatile Status;
    int32_t volatile Flags;
    uint32_t volatile Event;
    wd_blockdev_instance_xfer_t volatile Xfer;
} wd_blockdev_instance_info_t;

typedef struct wdif_bd_instance_info_io_s {
    uint32_t WP;
    uint32_t RESET;
} wd_blockdev_instance_io_t;

typedef const struct wdif_bd_instance_resources_s {
    wd_blockdev_instance_capabilities_t Capabilities;
    wd_blockdev_instance_info_t *Info;
    wd_blockdev_instance_io_t IO;
    void *Drv;
    void *Handle;
    void *Reg;
    int32_t (*Start)(void);
    int32_t (*Lock)(void);
    int32_t (*Unlock)(void);
} wd_blockdev_instance_resources_t;

typedef struct {
    int32_t (*Initialize)(wd_event_callback_t cb_event);
    int32_t (*Uninitialize)(void);
    int32_t (*PowerControl)(wd_power_state_t state);
    int32_t (*Write)(uint32_t addr, const void *data, uint32_t num);
    int32_t (*Read)(uint32_t addr, void *data, uint32_t num);
    int32_t (*Erase)(uint32_t addr, const uint8_t data, uint32_t num);
    int32_t (*Control)(uint32_t control, uint32_t arg);
    int32_t (*Get_Status)(void);
    void (*Validation)(void);
    wd_blockdev_instance_resources_t *Res;
} const wd_blockdev_instance_t;

typedef struct {
    wd_blockdev_instance_t *Instance;
    void *Ptr;
} wd_blockdev_object_t;

typedef struct {
    uint8_t Mask; // 0:OK, 1:Error
    uint8_t Object_Size;
    uint16_t Num;
    void *Data;
    wd_blockdev_object_t Object[3];
} wd_blockdev_object_multi_t;

typedef struct {
    int32_t (*Initialize)(wd_blockdev_instance_t *instance, wd_event_callback_t cb_event);
    int32_t (*Write)(wd_blockdev_instance_t *instance, void *ptr, const void *data, uint32_t num,
                     uint32_t arg);
    int32_t (*Read)(wd_blockdev_instance_t *instance, void *ptr, void *data, uint32_t num,
                    uint32_t arg);
    int32_t (*Erase)(wd_blockdev_instance_t *instance, void *ptr, const uint8_t data, uint32_t num,
                     uint32_t arg);
    int32_t (*Sync)(wd_blockdev_instance_t *instance, uint32_t timeout_ms);
    fsStatus (*DeviceCtrl)(fsDevCtrlCode code, void *p);
    int32_t (*Control)(wd_blockdev_instance_t *instance, uint32_t control, uint32_t arg);

    int32_t (*Object_Write)(wd_blockdev_object_multi_t *object, uint32_t arg);
    int32_t (*Object_Read)(wd_blockdev_object_multi_t *object, uint32_t arg);
} const wd_block_device_t;

extern wd_block_device_t wd_blockdev_driver;

extern wd_block_device_t *const wd_drv_block_device;

extern uint32_t wd_blockdev_retry_error_e_w;
extern uint32_t wd_blockdev_retry_error_r;

/* block device instances are provided by the component modules (I_Flash, NOR, NAND) : no include here */
#endif //	#if 1 // 토글

#endif /* WD_DRIVERS_WD_BLOCKDEV_H */
