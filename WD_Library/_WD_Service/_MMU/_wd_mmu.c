/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_mmu.c
 *
 *  Created on: Jan 7, 2025
 *      Author: doubled
 */

#define WD_CONF_CONSOLE_LEVEL_COMPILE WD_CONSOLE_LEVEL_INFO

#include "wd.h"

#if 1
#if (RTE_SERVICE_MMU)

#if (RTE_DRV_VALIDATION)
#define API_VALIDATION 0
#endif

/* NVM object layout : the CRC16 of an object is stored at <object address> + WD_CONF_MMU_NVM_CRC_OFFSET.
 * Part of the stored data format - define it in wd_memory_map.h together with the NVM map. */
#ifndef WD_CONF_MMU_NVM_CRC_OFFSET
#define WD_CONF_MMU_NVM_CRC_OFFSET 4096U
#endif

static const char *const s_object_name[] = {
    [0] = "[ ERROR ]",
    [WD_MMU_OBJECT_RAM] = "[MCU RAM]",
    [WD_MMU_OBJECT_ROM] = "[MCU ROM]",
    [WD_MMU_OBJECT_NVM_FLASH] = "[ Flash ]",
    [WD_MMU_OBJECT_NVM_SERIAL_EEPROM] = "[EEPROM ]",
};

/* object name for logs : out of range objects (WD_MMU_OBJECT_NONE ...) give "[ ERROR ]" */
static const char *object_name(uint32_t object) {
    return (object < (sizeof(s_object_name) / sizeof(s_object_name[0]))) ? s_object_name[object]
                                                                         : s_object_name[0];
}

int32_t wd_mmu_get_object(uint32_t addr) {
    int32_t object = WD_MMU_OBJECT_NONE;

    if (WD_RANGE((uint32_t)MEMORY_ADDRESS_DEVICE_BASE, addr, (uint32_t)MEMORY_ADDRESS_DEVICE_END)) {
        addr &= (uint32_t)~MEMORY_ADDRESS_OPTION_MASK;
    }

    if (WD_RANGE((uint32_t)MEMORY_DEVICE_NVM_FLASH, addr, (uint32_t)MEMORY_DEVICE_NVM_FLASH_END)) {
        object = WD_MMU_OBJECT_NVM_FLASH;
    }
#if (RTE_COMPONENT_NVM_SERIAL_EEPROM)
    else if (WD_RANGE((uint32_t)MEMORY_DEVICE_NVM_SERIAL_EEPROM, addr,
                      (uint32_t)MEMORY_DEVICE_NVM_SERIAL_EEPROM_END)) {
        object = WD_MMU_OBJECT_NVM_SERIAL_EEPROM;
    }
#endif
    else if (WD_RANGE((uint32_t)MEMORY_DEVICE_ROM, addr, (uint32_t)MEMORY_DEVICE_ROM_END)) {
        object = WD_MMU_OBJECT_ROM;
    } else if ((WD_RANGE((uint32_t)MEMORY_DEVICE_RAM1, addr, (uint32_t)MEMORY_DEVICE_RAM1_END)) ||
               (WD_RANGE((uint32_t)MEMORY_DEVICE_RAM2, addr, (uint32_t)MEMORY_DEVICE_RAM2_END))) {
        object = WD_MMU_OBJECT_RAM;
    } else {
        object = WD_MMU_OBJECT_NONE;
    }

    return (object);
}

/* block devices bound to the MMU memory objects (wd_mmu_attach). Index = WD_MMU_OBJECT_xxx */
#define MMU_DEVICE_SLOTS (WD_MMU_OBJECT_NVM_SERIAL_EEPROM + 1U)
static wd_blockdev_instance_t *s_mmu_device[MMU_DEVICE_SLOTS];

int32_t wd_mmu_attach(uint32_t object, wd_blockdev_instance_t *device) {
    if ((object != WD_MMU_OBJECT_ROM) && (object != WD_MMU_OBJECT_NVM_FLASH) &&
        (object != WD_MMU_OBJECT_NVM_SERIAL_EEPROM)) {
        return (WD_CODE_ERROR_PARAMETER);
    }
    s_mmu_device[object] = device; /* NULL : detach */
    return (WD_CODE_OK);
}

wd_blockdev_instance_t *wd_mmu_get_intereface(uint32_t addr) {
    wd_blockdev_instance_t *device = NULL;

    if (WD_RANGE((uint32_t)MEMORY_ADDRESS_DEVICE_BASE, addr, (uint32_t)MEMORY_ADDRESS_DEVICE_END)) {
        addr &= (uint32_t)~MEMORY_ADDRESS_DEVICE_MASK;
    }

    if (WD_RANGE((uint32_t)MEMORY_DEVICE_NVM_FLASH, addr, (uint32_t)MEMORY_DEVICE_NVM_FLASH_END)) {
        device = s_mmu_device[WD_MMU_OBJECT_NVM_FLASH];
    } else if (WD_RANGE((uint32_t)MEMORY_DEVICE_NVM_SERIAL_EEPROM, addr,
                        (uint32_t)MEMORY_DEVICE_NVM_SERIAL_EEPROM_END)) {
        device = s_mmu_device[WD_MMU_OBJECT_NVM_SERIAL_EEPROM];
    } else if (WD_RANGE((uint32_t)MEMORY_DEVICE_ROM, addr, (uint32_t)MEMORY_DEVICE_ROM_END)) {
        device = s_mmu_device[WD_MMU_OBJECT_ROM];
    }

    if (device == NULL) {
        WD_LOG_ERROR("Intereface Error");
    }
    return (device);
}


int32_t wd_mmu_access(void *dst, void *src, uint32_t num, uint32_t arg) {
    uint16_t crc = 0xACC8;
    uint32_t addr = (uint32_t)dst;

    uint8_t *ptr = (uint8_t *)src;
    int32_t ret = WD_CODE_OK;
    uint32_t size = num;
    uint32_t object = 0xFF;
    wd_blockdev_instance_t *interface = 0;

    if ((dst == NULL) || (src == NULL) || (num == 0)) {
        WD_LOG_ERROR();
        return (WD_CODE_ERROR_PARAMETER);
    }

    if ((arg & WD_MMU_OBJECT_MSK) == WD_MMU_OBJECT_AUTO) {
        object = (uint32_t)wd_mmu_get_object(addr);
    } else {
        if ((arg & WD_MMU_OBJECT_MSK) != (uint32_t)wd_mmu_get_object(addr) ||
            (object > WD_MMU_OBJECT_SIZE - 1)) {
            WD_LOG_ERROR("WD_MMU Addr Out of Range");
            return (WD_CODE_ERROR_PARAMETER);
        } else {
            object = arg & WD_MMU_OBJECT_MSK;
        }
    }

    if (object == WD_MMU_OBJECT_NONE) {
        return (WD_CODE_ERROR_PARAMETER);
    }

    if (object == WD_MMU_OBJECT_ROM) {
        if ((arg & WD_MMU_COMMAND_MSK) != WD_MMU_COMMAND_READ) {
            if (WD_RANGE((uint32_t)MEMORY_OBJECT_APPLICATION, addr,
                         (uint32_t)MEMORY_OBJECT_APPLICATION_END - 1)) {
                WD_LOG_ERROR("Can't Write Data in ROM");
                ret = WD_CODE_ERROR;
                goto end;
            }
        }
    }

    if (object != WD_MMU_OBJECT_RAM) {
        interface = wd_mmu_get_intereface(addr);
        if (interface == NULL) {
            WD_LOG_ERROR("WD_MMU Interface Out of Range");
            ret = WD_CODE_ERROR;
            goto end;
        }
    }

    switch (arg & WD_MMU_COMMAND_MSK) // 명령 마스킹
    {
    case WD_MMU_COMMAND_READ: {
        WD_LOG_DEBUG("WD_MMU%s:Read(0x%.8X, 0x%.8X, %d)", object_name(object), addr, ptr, num);
        if (object == WD_MMU_OBJECT_RAM) {
            memcpy((void *)ptr, (void *)addr, size);
            if (arg & WD_MMU_ARGUMENT_CRC) {
                memcpy(&crc, (void *)(addr + size), WD_CRC_SIZE);
            }
        } else if (object == WD_MMU_OBJECT_ROM) {
            ret = wd_drv_block_device->Read(interface, (void *)addr, (void *)ptr, size,
                                            WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
            if (arg & WD_MMU_ARGUMENT_CRC) // 드라이버에 CRC 체크까지 같이 넣었음. arg 로 관리
            {
                wd_drv_block_device->Read(interface, (void *)(addr + size), &crc, WD_CRC_SIZE,
                                          WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
            }
        } else //
        {
            ret = wd_drv_block_device->Read(interface, (void *)addr, (void *)ptr, size,
                                            WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
            if (arg & WD_MMU_ARGUMENT_CRC) // 드라이버에 CRC 체크까지 같이 넣었음. arg 로 관리
            {
                wd_drv_block_device->Read(interface, (void *)(addr + WD_CONF_MMU_NVM_CRC_OFFSET),
                                          &crc, WD_CRC_SIZE, WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
                //          wd_drv_block_device->Read(interface, addr + size, &crc, WD_CRC_SIZE, WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
            }
        }
    } break;

    case WD_MMU_COMMAND_WRITE: {
        WD_LOG_DEBUG("WD_MMU%s:Write(0x%.8X, 0x%.8X, %d)", object_name(object), addr, ptr, num);
        if (object == WD_MMU_OBJECT_RAM) {
            memcpy((void *)addr, (void *)ptr, size);
        } else {
            ret = wd_drv_block_device->Write(interface, (void *)addr, (void *)ptr, size,
                                             WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
        }
    } break;

    case WD_MMU_COMMAND_INIT: {
        WD_LOG_DEBUG("WD_MMU%s:Init(0x%.8X, 0x00, %d)", object_name(object), addr, num);
        if (object == WD_MMU_OBJECT_RAM) {
            memset((void *)addr, 0x00, size);
        } else {
            ret = wd_drv_block_device->Erase(interface, (void *)addr, 0x00, size,
                                             WDIF_BLOCK_DEVICE_ARGUMENT_CRC);
        }
    } break;

    case WD_MMU_COMMAND_INIT_0xFF: {
        WD_LOG_DEBUG("WD_MMU%s:Init(0x%.8X, 0xFF, %d)", object_name(object), addr, num);
        if (object == WD_MMU_OBJECT_RAM) {
            memset((void *)addr, 0xFF, size);
        } else {
            ret = wd_drv_block_device->Erase(interface, (void *)addr, 0xff, size,
                                             WDIF_BLOCK_DEVICE_ARGUMENT_CRC);
        }
    } break;

    case WD_MMU_COMMAND_INIT_0xFF_BULK: {
        addr |= MEMORY_ADDRESS_OPTION_MASK;
        WD_LOG_DEBUG("WD_MMU%s:Init_Bulk(0x%.8X, 0xFF, %d)", object_name(object), addr, num);
        if (object == WD_MMU_OBJECT_RAM) {
            memset((void *)addr, 0xFF, size);
        } else {
            ret = wd_drv_block_device->Erase(interface, (void *)addr, 0xff, size,
                                             WDIF_BLOCK_DEVICE_ARGUMENT_CRC);
        }
    } break;
    default:
        break;
    }

    if (arg & WD_MMU_ARGUMENT_CRC) {
        if (ret == WD_CODE_OK) {
            if (arg & WD_MMU_COMMAND_READ) {
                uint16_t crc_calc = wd_toolkit_crc16(ptr, size);

                if (crc_calc != crc) {
                    WD_LOG_ERROR("WD_MMU%s:Read CRC Error", object_name((uint32_t)(object)));
                    ret = WD_CODE_ERROR_CRC;
                    goto end;
                }
            } else {
                if (object == WD_MMU_OBJECT_ROM) {
                    crc = wd_toolkit_crc16(ptr, size);
                    wd_drv_block_device->Write(interface, (void *)(addr + size), &crc, WD_CRC_SIZE,
                                               WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
                } else {
                    crc = wd_toolkit_crc16(ptr, size);
                    //        wd_drv_block_device->Write(interface, addr, &crc, WD_CRC_SIZE, WDIF_BLOCK_DEVICE_ARGUMENT_CRC);
                    wd_drv_block_device->Write(interface,
                                               (void *)(addr + WD_CONF_MMU_NVM_CRC_OFFSET), &crc,
                                               WD_CRC_SIZE, WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
                }
            }
        }
    }

end:
#ifndef NDEBUG
    if (ret < 0) {
        __NOP();
    }
#endif

    return (ret);
}

int32_t wd_mmu_read_multi(uint32_t arg, wd_mmu_multi_object_t *object) {
    uint32_t argument;
    int32_t ret[3];

    argument = arg & (WD_MMU_COMMAND_MSK | WD_MMU_ARGUMENT_MSK);

    if (!(argument & WD_MMU_COMMAND_READ)) {
        WD_LOG_ERROR();
#ifndef NDEBUG
        WD_BREAK();
#endif
    }
    {
        int i;
        for (i = 0; i < object->Object_Size; i++) {
            ret[i] = wd_mmu_access(object->Object[i], wd_global.Shared.MMU_Multi_Read_Buf[i],
                                   object->Num, WD_MMU_OBJECT_AUTO | argument);

            WD_PRINT_INFO(WD_MSG_INFO, "MMU_Multi_Read %s   : ",
                          object_name((uint32_t)wd_mmu_get_object((uint32_t)(object->Object[i]))));

            if (ret[i] == WD_CODE_OK) {
                WD_PRINT_INFO(WD_MSG_PASS_CRLF, "Pass");
            } else {
                WD_PRINT_INFO(WD_MSG_FAIL_CRLF, "Fail !!!  [Error Code : %d]", ret[i]);
            }
            WD_PRINT_DEBUG(WD_MSG_MEM_DUMP_LOG, "Memory", WD_MSG_MAGIC, object->Object[i],
                           wd_global.Shared.MMU_Multi_Read_Buf[i], object->Num, 32);
            wd_time_wait_ms(1); // DUMP_LOG 시 깨짐 발생_이건 USB 콘솔문제인거 같음.
        }
    }

    if (ret[0] == WD_CODE_OK) {
        if (ret[1] == WD_CODE_OK) {
            if (memcmp(wd_global.Shared.MMU_Multi_Read_Buf[0],
                       wd_global.Shared.MMU_Multi_Read_Buf[1], object->Num)) {
                if (ret[2] == WD_CODE_OK) {
                    if (memcmp(wd_global.Shared.MMU_Multi_Read_Buf[0],
                               wd_global.Shared.MMU_Multi_Read_Buf[2], object->Num)) {
                        if (memcmp(wd_global.Shared.MMU_Multi_Read_Buf[1],
                                   wd_global.Shared.MMU_Multi_Read_Buf[2], object->Num)) {
                            ret[1] = WD_CODE_ARGUMENT_UNMATCH;
                            ret[2] = WD_CODE_ARGUMENT_UNMATCH;
                        } else {
                            if (object->Num > sizeof(wd_global.Shared.MMU_Multi_Read_Buf[0])) {
                                WD_LOG_ERROR();
                                object->Num = 0;
                            }
                            memcpy(wd_global.Shared.MMU_Multi_Read_Buf[0],
                                   wd_global.Shared.MMU_Multi_Read_Buf[1], object->Num);
                            ret[0] = WD_CODE_ARGUMENT_UNMATCH;
                        }
                    } else {
                        ret[1] = WD_CODE_ARGUMENT_UNMATCH;
                    }
                } else {
                    ret[1] = WD_CODE_ARGUMENT_UNMATCH;
                    ret[2] = WD_CODE_ARGUMENT_UNMATCH;
                }
            } else {
                if (memcmp(wd_global.Shared.MMU_Multi_Read_Buf[0],
                           wd_global.Shared.MMU_Multi_Read_Buf[2], object->Num)) {
                    ret[2] = WD_CODE_ARGUMENT_UNMATCH;
                }
            }
        } else {
            if (ret[2] == WD_CODE_OK) {
                ret[1] = WD_CODE_ARGUMENT_UNMATCH;
                if (memcmp(wd_global.Shared.MMU_Multi_Read_Buf[0],
                           wd_global.Shared.MMU_Multi_Read_Buf[2], object->Num)) {
                    ret[2] = WD_CODE_ARGUMENT_UNMATCH;
                }
            } else {
                ret[1] = WD_CODE_ARGUMENT_UNMATCH;
                ret[2] = WD_CODE_ARGUMENT_UNMATCH;
            }
        }
    } else if (ret[1] == WD_CODE_OK) {
        ret[0] = WD_CODE_ARGUMENT_UNMATCH;
        if (object->Num > sizeof(wd_global.Shared.MMU_Multi_Read_Buf[0])) {
            WD_LOG_ERROR();
            object->Num = 0;
        }
        memcpy(wd_global.Shared.MMU_Multi_Read_Buf[0], wd_global.Shared.MMU_Multi_Read_Buf[1],
               object->Num);

        if (ret[2] == WD_CODE_OK) {
            if (object->Num > sizeof(wd_global.Shared.MMU_Multi_Read_Buf[1])) {
                WD_LOG_ERROR();
                object->Num = 0;
            }
            if (memcmp(wd_global.Shared.MMU_Multi_Read_Buf[1],
                       wd_global.Shared.MMU_Multi_Read_Buf[2], object->Num)) {
                ret[2] = WD_CODE_ARGUMENT_UNMATCH;
            }
        } else {
            ret[2] = WD_CODE_ARGUMENT_UNMATCH;
        }
    } else if (ret[2] == WD_CODE_OK) {
        ret[0] = WD_CODE_ARGUMENT_UNMATCH;
        ret[1] = WD_CODE_ARGUMENT_UNMATCH;

        if (object->Num > sizeof(wd_global.Shared.MMU_Multi_Read_Buf[0])) {
            WD_LOG_ERROR();
            object->Num = 0;
        }
        memcpy(wd_global.Shared.MMU_Multi_Read_Buf[0], wd_global.Shared.MMU_Multi_Read_Buf[2],
               object->Num);
    } else {
        // All Fail
    }

    if (ret[0] < 0) {
        object->Mask |= WD_BIT0;
    }

    if (ret[1] < 0) {
        object->Mask |= WD_BIT1;
    }

    if (ret[2] < 0) {
        object->Mask |= WD_BIT2;
    }

    if ((object->Mask & (WD_BIT0 | WD_BIT1 | WD_BIT2)) == (WD_BIT0 | WD_BIT1 | WD_BIT2)) {
        object->Mask = WD_MMU_OBJECT_ALL;
    } else {
        if (object->Num > sizeof(wd_global.Shared.MMU_Multi_Read_Buf[0])) {
            WD_LOG_ERROR();
            object->Num = 0;
        }
        memcpy(object->Data, wd_global.Shared.MMU_Multi_Read_Buf[0], object->Num);
    }

    return (WD_CODE_OK);
}

int32_t wd_mmu_write_multi(uint32_t arg, wd_mmu_multi_object_t *object) {
    uint32_t argument;
    int32_t ret[3];

    argument = arg & (WD_MMU_COMMAND_MSK | WD_MMU_ARGUMENT_MSK);

    if (!(argument & WD_MMU_COMMAND_WRITE)) {
        WD_LOG_ERROR();
#ifndef NDEBUG
        WD_BREAK();
#endif
    }

    {
        int i;
        for (i = 0; i < object->Object_Size; i++) {
            if (object->Mask & (1 << i)) {
                ret[i] = wd_mmu_access(object->Object[i], object->Data, object->Num,
                                       WD_MMU_OBJECT_AUTO | argument);

                WD_PRINT_INFO(
                    WD_MSG_INFO, "MMU_Multi_Write %s   : ",
                    object_name((uint32_t)wd_mmu_get_object((uint32_t)(object->Object[i]))));

                if (ret[i] == WD_CODE_OK) {
                    WD_PRINT_INFO(WD_MSG_PASS_CRLF, "Pass");
                } else {
                    WD_PRINT_INFO(WD_MSG_FAIL_CRLF, "Fail !!!  [Error Code : %d]", ret[i]);
                }
            }
        }
    }

    return (WD_CODE_OK);
}

/**
 * @brief  Application hook : map a RAM object (application data mirror) to its NVM storage objects.
 *         - fill job[0 .. WD_MMU_OBJECT_SIZE-1] with NVM object addresses (MAP->NVM->xxx / MAP->ROM->xxx)
 *         - *src / *num may be replaced (e.g. save the whole parent structure)
 *         - OR WD_MMU_ARGUMENT_CRC into *arg to store with CRC
 *         - set application events (wd_reg_system->EVT_User[n]) if needed
 * @return true when dst is a known application object
 */
__WEAK bool wd_mmu_user_write_map(void *dst, void **src, uint32_t *num, void *job[],
                                  uint32_t *arg) {
    UNUSED(dst);
    UNUSED(src);
    UNUSED(num);
    UNUSED(job);
    UNUSED(arg);

    return false;
}

int32_t wd_mmu_write(void *dst, void *src, uint32_t num) {
    int32_t ret = WD_CODE_OK;
    uint32_t arg = WD_MMU_COMMAND_WRITE;

    if ((uint32_t)wd_mmu_get_object((uint32_t)dst) == WD_MMU_OBJECT_RAM) {
        uint32_t i;
        void *job[WD_MMU_OBJECT_SIZE] = {
            NULL,
        };

        memcpy(dst, src, num);

        if (!wd_mmu_user_write_map(dst, &src, &num, job, &arg)) {
            WD_LOG_ERROR();
        }

        for (i = 0; i < WD_MMU_OBJECT_SIZE; i++) {
            if (job[i] != NULL) {
                ret = wd_mmu_access((void *)job[i], src, num, arg);
            }
        }
    } else {
        ret = wd_mmu_access(dst, src, num, arg);
    }

    return (ret);
}

void wd_mmu_validation(void) {
#if (API_VALIDATION)
#define API_VALIDATION_TEST_ADDR 0x12143
#define API_VALIDATION_TEST_CHAR 0xA5U

    uint32_t i, j;
    uint32_t debug_time;
    uint32_t data;
    uint8_t *object = (uint8_t *)((uint32_t)MAP->NVM);

    struct api_validation_s *api_validation = &wd_global.Shared.API_Validation;

    /*--------------- Validation Initialize ---------------*/
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
    debug_time = wd_time_get_ms();
    wd_mmu_access(object, (void *)MEMORY_DEVICE_NVM_FLASH, MEMORY_DEVICE_NVM_FLASH_SIZE,
                  WD_MMU_COMMAND_INIT);
    debug_time = wd_time_calc_ms(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [ms]" WD_COLOR_RESET "\r\n",
                  "[NVM NOR Flash Validation Initialize] - Chip Erase", debug_time);

    for (i = (uint32_t)&MAP->NVM; i < RTE_NVM_NOR_FLASH_SIZE;
         i += RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE) {
        memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
        wd_mmu_access((void *)i, (void *)api_validation->R_buf, RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE,
                      WD_MMU_COMMAND_READ);
        if (wd_memchrn(api_validation->R_buf, 0, RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE)) {
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
    }

    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
    WD_PRINT_INFO(WD_MSG_TITLE_CRLF, "[MMU [NVM NOR Flash] Validation]");
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);

    /*--------------- Step 1 - Byte Write ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf, 1,
                  WD_MMU_COMMAND_WRITE);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 1 - Byte Write", debug_time);

    /*--------------- Step 2 - Byte Read ---------------*/
    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->R_buf, 1,
                  WD_MMU_COMMAND_READ);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 2 - Byte Read", debug_time);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 3 - Page Write ---------------*/
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf,
                  RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE, WD_MMU_COMMAND_WRITE);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 3 - Page Write", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->R_buf,
                  RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE, WD_MMU_COMMAND_READ);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR,
                   RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 4 - Sector Erase ---------------*/
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf,
                  RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE, WD_MMU_COMMAND_INIT_0xFF);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 4 - Sector Erase", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->R_buf,
                  RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE, WD_MMU_COMMAND_READ);
    if (wd_memchrn(api_validation->R_buf, 0xFF, RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 5 - Chip Write ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    for (i = 0; i < RTE_NVM_NOR_FLASH_SIZE; i += RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE) {
        for (j = 0; j < RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE; j += 4) {
            data = (uint32_t)(api_validation->W_buf + j);
            *(uint32_t *)data = i + ((i / RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE) + j);
        }
        wd_mmu_access(object + i, (void *)api_validation->W_buf, RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE,
                      WD_MMU_COMMAND_WRITE);
    }
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 5 - Chip Write", debug_time);


    for (i = 0; i < RTE_NVM_NOR_FLASH_SIZE; i += RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE) {
        for (j = 0; j < RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE; j += 4) {
            data = (uint32_t)(api_validation->W_buf + j);
            *(uint32_t *)data = i + ((i / RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE) + j);
        }
        memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
        wd_mmu_access(object + i, (void *)api_validation->R_buf, RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE,
                      WD_MMU_COMMAND_READ);
        if (memcmp(api_validation->R_buf, api_validation->W_buf,
                   RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE)) {
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
    }

    /*--------------- Step 6 - Chip Erase ---------------*/
    debug_time = wd_time_get_us();
    wd_mmu_access(object, (void *)MEMORY_DEVICE_NVM_FLASH, MEMORY_DEVICE_NVM_FLASH_SIZE,
                  WD_MMU_COMMAND_INIT_0xFF_BULK);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 6 - Chip Erase", debug_time);

    for (i = 0; i < RTE_NVM_NOR_FLASH_SIZE; i += RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE) {
        memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
        wd_mmu_access(object + i, (void *)api_validation->R_buf, RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE,
                      WD_MMU_COMMAND_READ);
        if (wd_memchrn(api_validation->R_buf, 0xFF, RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE)) {
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
    }

    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
    WD_PRINT_INFO(WD_MSG_TITLE_CRLF, "[MMU [NVM NOR Flash] API Validation]");
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);

    /*--------------- Step 1 - Byte Write (erased) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf, 1,
                  WD_MMU_COMMAND_WRITE);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 1 - Byte Write (erased)", debug_time);

    /*--------------- Step 2 - Byte Read ---------------*/
    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->R_buf, 1,
                  WD_MMU_COMMAND_READ);

    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 2 - Byte Read", debug_time);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 3 - Byte Write (same) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf, 1,
                  WD_MMU_COMMAND_WRITE);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 3 - Byte Write (same)", debug_time);

    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->R_buf, 1,
                  WD_MMU_COMMAND_READ);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 4 - Byte Write (different) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR + 1, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf, 1,
                  WD_MMU_COMMAND_WRITE);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 3 - Byte Write (same)", debug_time);

    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->R_buf, 1,
                  WD_MMU_COMMAND_READ);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR + 1, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    memset(api_validation->W_buf, 0, sizeof(api_validation->W_buf));
    wd_mmu_access(object + RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - 1, (void *)api_validation->W_buf, 1,
                  WD_MMU_COMMAND_WRITE);
    wd_mmu_access(object + RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE, (void *)api_validation->W_buf, 1,
                  WD_MMU_COMMAND_WRITE);

    /*--------------- Step 5 - 2 Byte Write [1 Page End][2 Page Start] (2 byte all different) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR + 2, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_mmu_access(object + RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - 1, (void *)api_validation->W_buf, 2,
                  WD_MMU_COMMAND_WRITE);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 5 - 2 Byte Write [1 Page End][2 Page Start] (2 byte all different)",
                  debug_time);

    memset(api_validation->R_buf, 0xFF, sizeof(api_validation->R_buf));
    wd_mmu_access(object + RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - 1, (void *)api_validation->R_buf, 2,
                  WD_MMU_COMMAND_READ);

    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR + 2, 2)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf,
                  RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE, WD_MMU_COMMAND_INIT_0xFF);

    /*--------------- Step 6 - Page Write (erased) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf,
                  RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE, WD_MMU_COMMAND_WRITE);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 6 - Page Write (erased)", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->R_buf,
                  RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE, WD_MMU_COMMAND_READ);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR,
                   RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 7 - Page Write (different) ---------------*/
    memset(api_validation->W_buf, API_VALIDATION_TEST_CHAR & 0x7C, sizeof(api_validation->W_buf));
    debug_time = wd_time_get_us();
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf,
                  RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE, WD_MMU_COMMAND_WRITE);

    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 7 - Page Write (different)", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->R_buf,
                  RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE, WD_MMU_COMMAND_READ);
    if (wd_memchrn(api_validation->R_buf, API_VALIDATION_TEST_CHAR & 0x7C,
                   RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->W_buf,
                  RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE, WD_MMU_COMMAND_INIT_0xFF);

    /*--------------- Step 8 - Byte Erase 0x00 [1 Page End] ---------------*/
    debug_time = wd_time_get_us();
    wd_mmu_access(object + RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE - 1, (void *)api_validation->W_buf,
                  1, WD_MMU_COMMAND_INIT);

    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "Step 8 - Byte Erase 0x00 [1 Page End]", debug_time);

    memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
    wd_mmu_access(object + RTE_NVM_NOR_FLASH_PROGRAM_BLOCK_SIZE - 1, (void *)api_validation->R_buf,
                  1, WD_MMU_COMMAND_READ);

    if (wd_memchrn(api_validation->R_buf, 0x00, 1)) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    /*--------------- Step 9 - Write Test (shift byte) ---------------*/
    memset(api_validation->A_buf, 0x81, sizeof(api_validation->A_buf));
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->A_buf,
                  sizeof(api_validation->A_buf), WD_MMU_COMMAND_WRITE);

    memset(api_validation->A_buf, 0, sizeof(api_validation->A_buf));
    wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->A_buf,
                  sizeof(api_validation->A_buf), WD_MMU_COMMAND_READ);
    if (wd_memchrn(api_validation->A_buf, 0x81, sizeof(api_validation->A_buf))) {
        WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
        return;
    }

    for (i = API_VALIDATION_TEST_ADDR + RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - 250;
         i < API_VALIDATION_TEST_ADDR + RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE + 250; i += 25) {
        WD_PRINT_INFO(
            WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "%3u / %3u" WD_COLOR_RESET "\r\n",
            "Step 12 - Write Test (shift byte)",
            ((i - (API_VALIDATION_TEST_ADDR + RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE - 250)) / 25) + 1,
            500 / 25);
        for (j = 0; j < RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE; j += 4) {
            data = (uint32_t)(api_validation->W_buf + j);
            *(uint32_t *)data = ((i * 1000) + j) + wd_time_get_us();
        }
        wd_mmu_access(object + i, (void *)api_validation->W_buf, RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE,
                      WD_MMU_COMMAND_WRITE);
        memset(api_validation->R_buf, 0, sizeof(api_validation->R_buf));
        wd_mmu_access(object + i, (void *)api_validation->R_buf, RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE,
                      WD_MMU_COMMAND_READ);
        if (memcmp(api_validation->R_buf, api_validation->W_buf,
                   RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE)) {
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
        memcpy(api_validation->A_buf + i - API_VALIDATION_TEST_ADDR, api_validation->W_buf,
               RTE_NVM_NOR_FLASH_ERASE_BLOCK_SIZE);
        wd_mmu_access(object + API_VALIDATION_TEST_ADDR, (void *)api_validation->R_buf,
                      sizeof(api_validation->R_buf), WD_MMU_COMMAND_READ);
        if (memcmp(api_validation->R_buf, api_validation->A_buf, sizeof(api_validation->R_buf))) {
            {
                int t;
                for (t = 0; t < sizeof(api_validation->R_buf); t++) {
                    if (api_validation->R_buf[t] != api_validation->A_buf[t]) {
                        __NOP();
                    }
                }
            }
            WD_PRINT_INFO(WD_MSG_ERROR_CRLF, "Fail");
            return;
        }
    }

    /*--------------- Validation Finalize ---------------*/
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
    debug_time = wd_time_get_us();
    wd_mmu_access(object, (void *)MEMORY_DEVICE_NVM_FLASH, MEMORY_DEVICE_NVM_FLASH_SIZE,
                  WD_MMU_COMMAND_INIT_0xFF_BULK);
    debug_time = wd_time_calc_us(&debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE, "%-120s" WD_COLOR_TIME "Time : %u [us]" WD_COLOR_RESET "\r\n",
                  "[MMU [NVM NOR Flash] Validation Finalize] - Chip Erase", debug_time);
    WD_PRINT_INFO(WD_MSG_TITLE | WD_MSG_FLAG_DELIMETER, WD_TERM_NULL);
#endif
}
#endif // (RTE_SERVICE_MMU)
#endif // #if 1
