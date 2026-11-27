/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*************************** (C) COPYRIGHT DoubleD *****************************
 * @file    wd_memory_map.h   (board : STM32H743XI_Ref - reference board configuration)
 * @brief   Memory map of the project : internal flash objects, NVM (external flash) layout
 *
 *  Required by WD_Library (do not remove, only move/resize):
 *    - MEMORY_xxx enum values used by Firmware / MMU / Block device
 *    - MAP_ROM       : Firmware, Environment (+ crc)            [internal flash fallback]
 *    - MAP_NVM_FLASH : Firmware, Environment, Firmware_Download [external NVM]
 *  Add application objects into the "User area" sections.
 ******************************************************************************/
#ifndef WD_CONFIG_WD_MEMORY_MAP_H
#define WD_CONFIG_WD_MEMORY_MAP_H


/***************************************************************************************************************************************************************
 **********  Memory
 **************************************************************************************************************************************************************/
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

typedef enum {
    MEMORY_ADDRESS_DEVICE_BASE = 0xC0000000, //0xC0000000
    MEMORY_ADDRESS_DEVICE_SIZE = 0x20000000,
    MEMORY_ADDRESS_DEVICE_END = MEMORY_ADDRESS_DEVICE_BASE + MEMORY_ADDRESS_DEVICE_SIZE,
    MEMORY_ADDRESS_DEVICE_MASK = 0x0FFFFFFF,

    MEMORY_ADDRESS_OPTION_MASK = 0x08000000,
    MEMORY_ADDRESS_MASK = 0x07FFFFFF,

#if 1
    /*--------------- Device ---------------*/
    MEMORY_DEVICE_ROM_BASE = 0x08000000,
    MEMORY_DEVICE_NVM_FLASH_BASE =
        0xC0000000, //0xD0000000	                                                             ,//0xC0000000
    MEMORY_DEVICE_NVM_SERIAL_EEPROM_BASE = 0xE0000000, //

    MEMORY_DEVICE_ROM_SIZE = (2 * 1024 * 1024), // (1024 * 1024) // BANK2만 사용
    MEMORY_DEVICE_ROM = MEMORY_DEVICE_ROM_BASE,
    MEMORY_DEVICE_ROM_END = MEMORY_DEVICE_ROM + MEMORY_DEVICE_ROM_SIZE,

    /*--------------- MAP_ROM (NOR/NAND 둘 다 비활성화 시 대체 저장소) : 플래시 마지막 섹터 128KB ---------------*/
    /* STM32H743XIHX_FLASH.ld 의 USER_DATA 리전과 반드시 같은 값을 유지해야 함 */
    MEMORY_DEVICE_ROM_USER_DATA_SIZE = (128 * 1024),
    MEMORY_DEVICE_ROM_USER_DATA = MEMORY_DEVICE_ROM_END -
        MEMORY_DEVICE_ROM_USER_DATA_SIZE, // 0x081E0000
    MEMORY_DEVICE_ROM_USER_DATA_END = MEMORY_DEVICE_ROM_USER_DATA +
        MEMORY_DEVICE_ROM_USER_DATA_SIZE,

    MEMORY_DEVICE_RAM1_SIZE = 0x6000000, //0x40000
    MEMORY_DEVICE_RAM1 = 0x24000000,
    MEMORY_DEVICE_RAM1_END = MEMORY_DEVICE_RAM1 + MEMORY_DEVICE_RAM1_SIZE,

    MEMORY_DEVICE_RAM2_SIZE = 0x800000, //0x10000
    MEMORY_DEVICE_RAM2 = 0x30000000,
    MEMORY_DEVICE_RAM2_END = MEMORY_DEVICE_RAM2 + MEMORY_DEVICE_RAM2_SIZE,

    MEMORY_DEVICE_NVM_SERIAL_EEPROM_SIZE = (256 * 1024),
    MEMORY_DEVICE_NVM_SERIAL_EEPROM = MEMORY_DEVICE_NVM_SERIAL_EEPROM_BASE,
    MEMORY_DEVICE_NVM_SERIAL_EEPROM_END = MEMORY_DEVICE_NVM_SERIAL_EEPROM +
        MEMORY_DEVICE_NVM_SERIAL_EEPROM_SIZE,

    MEMORY_DEVICE_NVM_FLASH_SIZE = (256 * 1024 * 1024),
    MEMORY_DEVICE_NVM_FLASH = MEMORY_DEVICE_NVM_FLASH_BASE,
    MEMORY_DEVICE_NVM_FLASH_END = MEMORY_DEVICE_NVM_FLASH + MEMORY_DEVICE_NVM_FLASH_SIZE,

    /*--------------- Object ---------------*/
    MEMORY_OBJECT_FIRMWARE_HEADER_SIZE = 2048,

    MEMORY_OBJECT_BOOTLOADER_SIZE = (256 * 1024), //(192 * 1024)
    MEMORY_OBJECT_BOOTLOADER = MEMORY_DEVICE_ROM,
    MEMORY_OBJECT_BOOTLOADER_END = MEMORY_OBJECT_BOOTLOADER +
        MEMORY_OBJECT_BOOTLOADER_SIZE, //0x08000000 + 256kB

    MEMORY_OBJECT_ROM_LIB_SIZE =
        0, //(128 * 1024)                                                              , //( 32 * 1024)
    MEMORY_OBJECT_ROM_LIB = MEMORY_OBJECT_BOOTLOADER_END,
    MEMORY_OBJECT_ROM_LIB_END = MEMORY_OBJECT_ROM_LIB +
        MEMORY_OBJECT_ROM_LIB_SIZE, //0x08000000 + 256kB + 128kB

    MEMORY_OBJECT_ROM_DATA_SIZE =
        0, //(128 * 1024)                                                              , //( 32 * 1024)
    MEMORY_OBJECT_ROM_DATA = MEMORY_OBJECT_ROM_LIB_END,
    MEMORY_OBJECT_ROM_DATA_END = MEMORY_OBJECT_ROM_DATA +
        MEMORY_OBJECT_ROM_DATA_SIZE, //0x08000000 + 256kB + 128kB + 256kB

    MEMORY_OBJECT_APPLICATION_SIZE = (512 * 1024), //(384 * 1024)
    MEMORY_OBJECT_APPLICATION = MEMORY_OBJECT_ROM_DATA_END,
    MEMORY_OBJECT_APPLICATION_END = MEMORY_OBJECT_APPLICATION +
        MEMORY_OBJECT_APPLICATION_SIZE, //0x08000000 + 256kB + 128kB + 256kB + 512kB

    /* 주의: 예전엔 MEMORY_OBJECT_APPLICATION_END(=0x080C0000)를 그대로 썼는데, 이 주소는 실제 링커 스크립트 기준
   * 지금 돌아가는 앱 코드 한가운데라 안전하지 않음(MEMORY_OBJECT_ROM_DATA와 같은 종류의 문제).
   * 플래시 마지막 섹터(MEMORY_DEVICE_ROM_USER_DATA) 바로 앞에 실제로 예약된 FACTORY_IMAGE 리전을 가리키도록 수정. */
    MEMORY_OBJECT_APPLICATION_FACTORY_SIZE = (512 * 1024), //(384 * 1024)
    MEMORY_OBJECT_APPLICATION_FACTORY = MEMORY_DEVICE_ROM_USER_DATA -
        MEMORY_OBJECT_APPLICATION_FACTORY_SIZE, // 0x08160000
    MEMORY_OBJECT_APPLICATION_FACTORY_END = MEMORY_OBJECT_APPLICATION_FACTORY +
        MEMORY_OBJECT_APPLICATION_FACTORY_SIZE, // 0x081E0000 (USER_DATA 시작과 정확히 접함)

    MEMORY_OBJECT_APPLICATION_DOWN_SIZE = (512 * 1024), //(288 * 1024)
    MEMORY_OBJECT_APPLICATION_DOWN = MEMORY_DEVICE_NVM_FLASH_END -
        MEMORY_OBJECT_APPLICATION_DOWN_SIZE,
    MEMORY_OBJECT_APPLICATION_DOWN_END = MEMORY_OBJECT_APPLICATION_DOWN +
        MEMORY_OBJECT_APPLICATION_DOWN_SIZE,
#else
    /*--------------- Device ---------------*/
    MEMORY_DEVICE_ROM_BASE = 0x08000000,
    MEMORY_DEVICE_NVM_FLASH_BASE = 0xC0000000,
    MEMORY_DEVICE_NVM_SERIAL_EEPROM_BASE = 0xD0000000,

    MEMORY_DEVICE_ROM_SIZE = (1024 * 1024),
    MEMORY_DEVICE_ROM = MEMORY_DEVICE_ROM_BASE,
    MEMORY_DEVICE_ROM_END = MEMORY_DEVICE_ROM + MEMORY_DEVICE_ROM_SIZE,

    MEMORY_DEVICE_RAM1_SIZE = 0x40000,
    MEMORY_DEVICE_RAM1 = 0x20000000,
    MEMORY_DEVICE_RAM1_END = MEMORY_DEVICE_RAM1 + MEMORY_DEVICE_RAM1_SIZE,

    MEMORY_DEVICE_RAM2_SIZE = 0x10000,
    MEMORY_DEVICE_RAM2 = 0x20040000,
    MEMORY_DEVICE_RAM2_END = MEMORY_DEVICE_RAM2 + MEMORY_DEVICE_RAM2_SIZE,

    MEMORY_DEVICE_NVM_SERIAL_EEPROM_SIZE = (256 * 1024),
    MEMORY_DEVICE_NVM_SERIAL_EEPROM = MEMORY_DEVICE_NVM_SERIAL_EEPROM_BASE,
    MEMORY_DEVICE_NVM_SERIAL_EEPROM_END = MEMORY_DEVICE_NVM_SERIAL_EEPROM +
        MEMORY_DEVICE_NVM_SERIAL_EEPROM_SIZE,

    MEMORY_DEVICE_NVM_FLASH_SIZE = (2 * 1024 * 1024),
    MEMORY_DEVICE_NVM_FLASH = MEMORY_DEVICE_NVM_FLASH_BASE,
    MEMORY_DEVICE_NVM_FLASH_END = MEMORY_DEVICE_NVM_FLASH + MEMORY_DEVICE_NVM_FLASH_SIZE,

    /*--------------- Object ---------------*/
    MEMORY_OBJECT_FIRMWARE_HEADER_SIZE = 0,

    MEMORY_OBJECT_BOOTLOADER_SIZE = (0 * 1024),
    MEMORY_OBJECT_BOOTLOADER = MEMORY_DEVICE_ROM,
    MEMORY_OBJECT_BOOTLOADER_END = MEMORY_OBJECT_BOOTLOADER + MEMORY_OBJECT_BOOTLOADER_SIZE,

    MEMORY_OBJECT_ROM_LIB_SIZE = (0 * 1024),
    MEMORY_OBJECT_ROM_LIB = MEMORY_OBJECT_BOOTLOADER_END,
    MEMORY_OBJECT_ROM_LIB_END = MEMORY_OBJECT_ROM_LIB + MEMORY_OBJECT_ROM_LIB_SIZE,

    MEMORY_OBJECT_ROM_DATA_SIZE = (32 * 1024),
    MEMORY_OBJECT_ROM_DATA = MEMORY_DEVICE_ROM + (768 * 1024),
    MEMORY_OBJECT_ROM_DATA_END = MEMORY_OBJECT_ROM_DATA + MEMORY_OBJECT_ROM_DATA_SIZE,

    MEMORY_OBJECT_APPLICATION_SIZE = (384 * 1024),
    MEMORY_OBJECT_APPLICATION = MEMORY_DEVICE_ROM,
    MEMORY_OBJECT_APPLICATION_END = MEMORY_OBJECT_APPLICATION + MEMORY_OBJECT_APPLICATION_SIZE,

    MEMORY_OBJECT_APPLICATION_DOWN_SIZE = (384 * 1024),
    MEMORY_OBJECT_APPLICATION_DOWN = MEMORY_OBJECT_APPLICATION_END,
    MEMORY_OBJECT_APPLICATION_DOWN_END = MEMORY_OBJECT_APPLICATION_DOWN +
        MEMORY_OBJECT_APPLICATION_DOWN_SIZE,

    MEMORY_OBJECT_APPLICATION_FACTORY_SIZE = (288 * 1024),
    MEMORY_OBJECT_APPLICATION_FACTORY = MEMORY_DEVICE_NVM_FLASH_END -
        MEMORY_OBJECT_APPLICATION_FACTORY_SIZE,
    MEMORY_OBJECT_APPLICATION_FACTORY_END = MEMORY_OBJECT_APPLICATION_FACTORY +
        MEMORY_OBJECT_APPLICATION_FACTORY_SIZE,
#endif
} MEMORY_OBJECT;

#define RTE_NVM_FLASH_128_KB_ERASE_BLOCK_SIZE (128 * 1024)
#define RTE_NVM_FLASH_128_KB_ERASE_ADDR       (MEMORY_ADDRESS_OPTION_MASK + 0x00020000)
//#define RTE_NVM_FLASH_128_KB_ERASE_ADDR                                          (MEMORY_OBJECT_APPLICATION_DOWN - MEMORY_DEVICE_NVM_FLASH_BASE)

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/***************************************************************************************************************************************************************
 **********  ★ Internal Flash - ROM  (last sector 128KB, used only when NOR/NAND are both disabled)
 **************************************************************************************************************************************************************/
/* __PACKED_STRUCT is required: Dummy_Sector_0 size is computed with plain sizeof() arithmetic. */
#ifndef MAP_ROM_USER_AREA_SIZE
#define MAP_ROM_USER_AREA_SIZE 0 /* bytes reserved for application objects */
#endif

typedef __PACKED_STRUCT {
    /*--------------- Sector 0 : 0 ~ 0x20000 [Critical - 128 KB] ---------------*/
#if (RTE_SERVICE_FIRMWARE)
    wd_firmware_t Firmware;
    uint32_t Firmware_crc;
#endif
    wd_env_t Environment;
    uint32_t Environment_crc;
#if (MAP_ROM_USER_AREA_SIZE > 0)
    uint8_t User_Area[MAP_ROM_USER_AREA_SIZE]; /* application objects */
#endif
    uint8_t Dummy_Sector_0[FLASH_SECTOR_SIZE
#if (RTE_SERVICE_FIRMWARE)
                           - sizeof(wd_firmware_t) - sizeof(uint32_t)
#endif
                           - sizeof(wd_env_t) - sizeof(uint32_t) - MAP_ROM_USER_AREA_SIZE];
}
MAP_ROM;

/* RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE is defined by the NOR/NAND component header.
 * Provide a default so that MAP_NVM_FLASH type is always valid even if both are disabled. */
#ifndef RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE
#define RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE 4096
#endif

/* MMU : wd_toolkit_crc16 of an NVM object is stored at <object address> + WD_CONF_MMU_NVM_CRC_OFFSET (stored data format) */
#define WD_CONF_MMU_NVM_CRC_OFFSET RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE

/***************************************************************************************************************************************************************
 **********  ★ NVM - External Flash (NOR / NAND)
 **************************************************************************************************************************************************************/
#define MAP_NVM_USER_SECTOR_FIRST     8   /* first sector of application area */
#define MAP_NVM_USER_SECTOR_LAST      452 /* last  sector of application area */
#define MAP_NVM_RESERVED_SECTOR_FIRST 453
#define MAP_NVM_RESERVED_SECTOR_LAST  65407

typedef struct {
#if (RTE_SERVICE_FIRMWARE)
    /*--------------- Sector 0 : 0x0000 ~ 0x1000 [Critical] ---------------*/
    wd_firmware_t Firmware;
    uint8_t Dummy_Sector_0[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - sizeof(wd_firmware_t)];
    /*--------------- Sector 1 : 0x1000 ~ 0x2000 [Critical crc] ---------------*/
    uint8_t crcFirmware[4];
    uint8_t Dummy_Sector_1[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - (sizeof(uint8_t) * 4)];
    /*--------------- Sector 2 : 0x2000 ~ 0x3000 [Environment] ---------------*/
    wd_env_t Environment;
    uint8_t Dummy_Sector_2[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - sizeof(wd_env_t)];
    /*--------------- Sector 3 : 0x3000 ~ 0x4000 [Environment crc] ---------------*/
    uint8_t crcEnvironment[4];
    uint8_t Dummy_Sector_3[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - (sizeof(uint8_t) * 4)];
    /*--------------- Sector 4 : 0x4000 ~ 0x5000 [Critical backup] ---------------*/
    wd_firmware_t Firmware_Backup;
    uint8_t Dummy_Sector_4[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - sizeof(wd_firmware_t)];
    /*--------------- Sector 5 : 0x5000 ~ 0x6000 [Critical backup crc] ---------------*/
    uint8_t crcFirmware_Backup[4];
    uint8_t Dummy_Sector_5[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - (sizeof(uint8_t) * 4)];
    /*--------------- Sector 6 : 0x6000 ~ 0x7000 [Environment backup] ---------------*/
    wd_env_t Environment_Backup;
    uint8_t Dummy_Sector_6[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - sizeof(wd_env_t)];
    /*--------------- Sector 7 : 0x7000 ~ 0x8000 [Environment backup crc] ---------------*/
    uint8_t crcEnvironment_Backup[4];
    uint8_t Dummy_Sector_7[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - (sizeof(uint8_t) * 4)];
#else /* no firmware service : same layout, the firmware sectors stay reserved so Environment keeps its address */
    uint8_t Dummy_Sector_0_1[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE * 2];
    wd_env_t Environment;
    uint8_t Dummy_Sector_2[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - sizeof(wd_env_t)];
    uint8_t crcEnvironment[4];
    uint8_t Dummy_Sector_3[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - (sizeof(uint8_t) * 4)];
    uint8_t Dummy_Sector_4_5[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE * 2];
    wd_env_t Environment_Backup;
    uint8_t Dummy_Sector_6[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - sizeof(wd_env_t)];
    uint8_t crcEnvironment_Backup[4];
    uint8_t Dummy_Sector_7[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - (sizeof(uint8_t) * 4)];
#endif
    /*--------------- Sector 8 ~ 452 : 0x8000 ~ 0x1C5000 [User area - application objects] ---------------*/
    /* Replace this array by application structures (keep the total size, or adjust Reserved below). */
    uint8_t User_Area[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE *
                      (MAP_NVM_USER_SECTOR_LAST - MAP_NVM_USER_SECTOR_FIRST + 1)];
    /*--------------- Sector 453 ~ 65407 [Reserved] ---------------*/
    uint8_t
        Dummy_Sector_453_65407[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE *
                               (MAP_NVM_RESERVED_SECTOR_LAST - MAP_NVM_RESERVED_SECTOR_FIRST + 1)];
    /*--------------- Sector 65408 ~ 65533 [Firmware Download] ---------------*/
    uint8_t Firmware_Download[MEMORY_OBJECT_APPLICATION_DOWN_SIZE -
                              (RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE * 2)];
    /*--------------- Sector 65534 [Test] ---------------*/
    uint8_t Test[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE];
    /*--------------- Sector 65535 [End] ---------------*/
    uint8_t Last_Sector[RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE];
} MAP_NVM_FLASH;

/***************************************************************************************************************************************************************
 **********  ★ Map
 **************************************************************************************************************************************************************/
typedef struct {
    MAP_ROM *const ROM;
    MAP_NVM_FLASH *const NVM;
} MEMORY_MAP;

/* Fixed MAP provider: compile-time constant, no linker/initialization dependency.
 * MAP_ROM must point to the last flash sector (MEMORY_DEVICE_ROM_USER_DATA). */
static inline const MEMORY_MAP *GET_MAP(void) {
    static const MEMORY_MAP k = {
        (MAP_ROM *)(MEMORY_DEVICE_ROM_USER_DATA),
        (MAP_NVM_FLASH *)(MEMORY_DEVICE_NVM_FLASH_BASE),
    };
    return &k;
}
#define MAP (GET_MAP())

#endif /* WD_CONFIG_WD_MEMORY_MAP_H */
