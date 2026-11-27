/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_fw.c
 *
 *  Created on: Jan 13, 2025
 *      Author: doubled
 */


//#if   (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_BOOTLOADER)
//#define WD_CONF_CONSOLE_LEVEL_COMPILE                                             WD_CONSOLE_LEVEL_INFO
//#elif (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
//#define WD_CONF_CONSOLE_LEVEL_COMPILE                                             WD_CONSOLE_LEVEL_INFO
//#endif

#include "wd.h"

#if 1 // 토글

#if (RTE_SERVICE_FIRMWARE)

/***************************************************************************************************************************************************************
 **********  Firmware API
 **************************************************************************************************************************************************************/
wd_firmware_t wd_fw_data;
__USED wd_firmware_header_t *const wd_fw_header_app =
    (wd_firmware_header_t *)(MEMORY_OBJECT_APPLICATION);
__USED wd_firmware_header_t *const wd_fw_header_app_down =
    (wd_firmware_header_t *)(MEMORY_OBJECT_APPLICATION_DOWN);
__USED wd_firmware_header_t *const wd_fw_header_app_factory =
    (wd_firmware_header_t *)(MEMORY_OBJECT_APPLICATION_FACTORY);
wd_firmware_fota_info_t wd_fw_fota_info;


void wd_fw_default(void) {
    WD_LOG_NOTICE("Firmware Information Set Default");

    memset(&wd_fw_data, 0, sizeof(wd_fw_data));

    memcpy(wd_fw_data.FW_Version, RTE_CONFIG_FIRMWARE_VERSION,
           strlen((char *)RTE_CONFIG_FIRMWARE_VERSION));

    wd_fw_data.FW_Update_Interface_Version =
        WDIF_FIRMWARE_UPDATE_INTERFACE_MAGIC_NUMBER + WDIF_FIRMWARE_UPDATE_INTERFACE_VERSION;
    wd_fw_data.Digest =
        wd_toolkit_crc16(&wd_fw_data, sizeof(wd_fw_data) - sizeof(wd_fw_data.Digest));
}

static const char *Firmware_Image_Type[] = {
    [0] = "Error               ", [1] = "Download Application", [2] = "Download Bootloader ",
    [3] = "Download Library    ", [4] = "Factory Application ", [5] = "Factory Bootloader  ",
    [6] = "Factory Library     "};

void wd_fw_print_image(wd_firmware_header_t *header, char *title) {
    struct tm time_tm;

    if (title != NULL) {
        WD_PRINT_INFO(WD_MSG_NOTI, "%s ", title);
        WD_PRINT_INFO(WD_MSG_NOTI_CRLF, "Firmware Information");
    }
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "File Type            : %s",
                  (header->File_Type > 6) ? Firmware_Image_Type[0]
                                          : Firmware_Image_Type[header->File_Type]);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "File Location        : %p", header->File_Location);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "File Name            : %s",
                  strlen(header->File_Name) > sizeof(header->File_Name) ? "Error"
                                                                        : header->File_Name);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "File Length          : %u", header->File_Length);
    wd_time_time_to_tm(header->Modification_Date, &time_tm);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Modification Date    : %04d/%02d/%02d %02d:%02d:%02d",
                  time_tm.tm_year + 1900, time_tm.tm_mon, time_tm.tm_mday, time_tm.tm_hour,
                  time_tm.tm_min, time_tm.tm_sec);
    wd_time_time_to_tm(header->Update_Date, &time_tm);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Update Date          : %04d/%02d/%02d %02d:%02d:%02d",
                  time_tm.tm_year + 1900, time_tm.tm_mon, time_tm.tm_mday, time_tm.tm_hour,
                  time_tm.tm_min, time_tm.tm_sec);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "File CheckSum        : %p", header->File_CheckSum);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Copyright            : %s",
                  strlen(header->Copyright) > sizeof(header->Copyright) ? "Error"
                                                                        : header->Copyright);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Department           : %s",
                  strlen(header->Department) > sizeof(header->Department) ? "Error"
                                                                          : header->Department);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Author               : %s",
                  strlen(header->Author) > sizeof(header->Author) ? "Error" : header->Author);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Project              : %s",
                  strlen(header->Project) > sizeof(header->Project) ? "Error" : header->Project);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Product Name         : %s",
                  strlen(header->Product_Name) > sizeof(header->Product_Name)
                      ? "Error"
                      : header->Product_Name);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Specification        : %s",
                  strlen(header->Specification) > sizeof(header->Specification)
                      ? "Error"
                      : header->Specification);
    wd_time_time_to_tm(header->Creation_Date, &time_tm);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Creation Date        : %04d/%02d/%02d %02d:%02d:%02d",
                  time_tm.tm_year + 1900, time_tm.tm_mon, time_tm.tm_mday, time_tm.tm_hour,
                  time_tm.tm_min, time_tm.tm_sec);
    wd_time_time_to_tm(header->Release_Date, &time_tm);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Release Date         : %04d/%02d/%02d %02d:%02d:%02d",
                  time_tm.tm_year + 1900, time_tm.tm_mon, time_tm.tm_mday, time_tm.tm_hour,
                  time_tm.tm_min, time_tm.tm_sec);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF | WD_MSG_DUMP, "Commit               : ", WD_MSG_MAGIC,
                  header->Commit, sizeof(header->Commit), WD_MSG_OPTION_HEX);
    WD_PRINT_INFO(WD_MSG_INFO_CRLF, "Memo                 : %s",
                  strlen(header->Memo) > sizeof(header->Memo) ? "Error" : header->Memo);
}

int32_t wd_fw_check_image(wd_firmware_header_t *header, uint32_t addr, uint32_t *size) {
    int32_t ret = WD_CODE_OK;
    uint32_t crc;

    if (WD_RANGE((uint32_t)MEMORY_DEVICE_ROM, addr, (uint32_t)MEMORY_DEVICE_ROM_END)) {
        memcpy(header, (void *)addr, sizeof(wd_firmware_header_t));
    } else if (WD_RANGE((uint32_t)MEMORY_DEVICE_NVM_FLASH, addr,
                        (uint32_t)MEMORY_DEVICE_NVM_FLASH_END)) {
#if (RTE_SERVICE_MMU)

        wd_blockdev_instance_t *device = wd_mmu_get_intereface(addr);
        if (device == NULL) // no block device attached to the NVM range (wd_mmu_attach)
        {
            return (WD_CODE_ERROR);
        }
        wd_drv_block_device->Read(device, (void *)addr, header, sizeof(wd_firmware_header_t),
                                  WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
        //    wd_drv_block_device->Read(wd_mmu_get_intereface(addr), addr, header, sizeof(wd_firmware_header_t));

#endif
    } else {
        return (WD_CODE_ERROR);
    }

    {
        int i;
        uint8_t *data = (uint8_t *)header;
        for (i = 0; i < (int)sizeof(wd_firmware_header_t); i++) {
            data[i] ^= 0xFF;
        }
    }

    crc =
        wd_toolkit_crc32(header, WD_FIELD_OFFSET(wd_firmware_header_t, Header_CheckSum, File_Type));

    if (crc != header->Header_CheckSum) {
        ret = WD_CODE_ERROR;
    } else if (header->File_Length >
               (uint32_t)(MEMORY_OBJECT_APPLICATION_SIZE - MEMORY_OBJECT_FIRMWARE_HEADER_SIZE)) {
        ret = WD_CODE_ERROR;
    } else {
        if (WD_RANGE(MEMORY_DEVICE_ROM, addr, MEMORY_DEVICE_ROM_END)) {
            crc = wd_toolkit_crc32((void *)(addr + sizeof(wd_firmware_header_t)),
                                   header->File_Length);
        } else {
            uint32_t block_size;
            uint32_t i;


#if (RTE_RTOS)
            if (!WD_CORE_IS_IN_ISR()) {
                if (osMutexAcquire(wd_os_mutex_global, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
                    WD_BREAK();
                }
            }
#endif

            block_size = sizeof(wd_global.Shared.Buf) & 0xFFFFF000;
            crc = 0xFFFFFFFF;
            for (i = 0; i < (uint32_t)header->File_Length; i += block_size) {
                WD_WDT_REFRESH();

                if (header->File_Length - i < block_size) {
                    block_size = header->File_Length - i;
                }

#if (RTE_SERVICE_MMU)
                wd_drv_block_device->Read(
                    wd_mmu_get_intereface(addr), (void *)(addr + sizeof(wd_firmware_header_t) + i),
                    wd_global.Shared.Buf, block_size, WDIF_BLOCK_DEVICE_ARGUMENT_NONE);
#endif

                wd_toolkit_crc32_extension(wd_global.Shared.Buf, block_size, &crc);
            }
            crc ^= 0xFFFFFFFF;

#if (RTE_RTOS)
            if (!WD_CORE_IS_IN_ISR()) {
                osMutexRelease(wd_os_mutex_global);
            }
#endif
        }

        if (crc != header->File_CheckSum) {
            ret = WD_CODE_ERROR;
        }
    }

    if (size != NULL) {
        *size = header->File_Length + sizeof(wd_firmware_header_t);
    }

    return (ret);
}

void wd_fw_initialization(void) {
#if (RTE_SERVICE_MMU)
    wd_mmu_multi_object_t object = {
        .Mask = 0,
        .Object_Size = 1, //2, //3,
        .Num = sizeof(wd_firmware_t),
        .Data = &wd_fw_data,
        .Object[0] =
            &MAP->ROM
                 ->Firmware, // Firmware(Boot_Fail_Count 등 부트 임계 정보)는 NOR/NAND 상태와 무관하게 항상 내부 플래시 사용
        //    .Object[1] = &MAP->ROM->Firmware_Backup,
    };

    if (wd_mmu_read_multi(WD_MMU_COMMAND_READ, &object)) {
        WD_LOG_ERROR();
    }

    if (wd_toolkit_crc16(&wd_fw_data, sizeof(wd_fw_data)) != WD_CRC16_RESULT) {
        WD_LOG_ERROR("Firmware Resources Digest Error");
        object.Mask = WD_MMU_OBJECT_ALL;
    }

    /* Bootloader도 반드시 자기 복구를 수행해야 함: Digest Error 시 wd_fw_default()가
   * 빠지면 Boot_Fail_Count 등이 방금 읽은 raw(보통 지워진 플래시=0xFF) 값으로 남아,
   * 예를 들어 Boot_Fail_Count=255로 오판되어 정상 이미지를 Factory로 강제 롤백시키는
   * 버그가 있었음 (원래는 App 전용으로 막혀 있었음). */
    if (object.Mask) {
        if (object.Mask == WD_MMU_OBJECT_ALL) {
            wd_fw_default();
        }

        wd_mmu_write_multi(WD_MMU_COMMAND_WRITE_CRC, &object);
    }

    {
        int32_t ret = WD_CODE_OK;
        const char *msg_imgae = "Check Firmware Image";
        const char *msg_ok = "\x1b[1;32mGood";
        const char *msg_bad = "\x1b[1;31mBad";

        ret = wd_fw_check_image(&wd_fw_fota_info.Firmware_Header, (uint32_t)wd_fw_header_app, NULL);
        printf("%s [Active ] : %s" WD_COLOR_RESET "\r\n", msg_imgae,
               (ret == WD_CODE_OK) ? msg_ok : msg_bad);
        if (ret == WD_CODE_OK) {
            wd_fw_print_image(&wd_fw_fota_info.Firmware_Header, "Current");
        }

        ret = wd_fw_check_image(&wd_fw_fota_info.Firmware_Header, (uint32_t)wd_fw_header_app_down,
                                NULL);
        printf("%s [Passive] : %s" WD_COLOR_RESET "\r\n", msg_imgae,
               (ret == WD_CODE_OK) ? msg_ok : msg_bad);
        if (ret == WD_CODE_OK) {
            wd_fw_print_image(&wd_fw_fota_info.Firmware_Header, "Download");
        }

        ret = wd_fw_check_image(&wd_fw_fota_info.Firmware_Header,
                                (uint32_t)wd_fw_header_app_factory, NULL);
        printf("%s [Factory] : %s" WD_COLOR_RESET "\r\n", msg_imgae,
               (ret == WD_CODE_OK) ? msg_ok : msg_bad);
        if (ret == WD_CODE_OK) {
            wd_fw_print_image(&wd_fw_fota_info.Firmware_Header, "Factory");
        }
    }

#if (RTE_CONFIG_SOFTWARE_TYPE != RTE_SOFTWARE_TYPE_BOOTLOADER)
    if (wd_fw_data.EVT_Firmware_System_Update) {
        wd_fw_data.EVT_Firmware_System_Update = false;
        wd_reg_system->EVT_BootUp_Firmware_System_Update = true;
        object.Mask = WD_MMU_OBJECT_ALL;
        wd_fw_data.Digest =
            wd_toolkit_crc16(&wd_fw_data, sizeof(wd_fw_data) - sizeof(wd_fw_data.Digest));
        wd_mmu_write_multi(WD_MMU_COMMAND_WRITE_CRC, &object);
    }

    if (wd_reg_system->EVT_BootUp_Firmware_System_Update) {
        printf(WD_COLOR_BLUE_BOLD "Firmware Update !!!" WD_COLOR_RESET "\r\n\r\n\r\n");
    }
#endif
#endif
}

/* 부팅이 여기까지 정상적으로 도달하면(=크래시 없이 애플리케이션 초기화 완료) "이번 부팅은 성공"으로 간주.
 * 1) Boot_Fail_Count를 0으로 리셋해서 저장 (부트로더가 다음 부팅부터 다시 처음부터 5회 카운트)
 * 2) 지금 Active 이미지가 Factory에 저장된 것과 다르면(Commit 해시 비교), Factory를 최신 Active로 갱신
 *    — 매 부팅마다 하지 않고 "버전이 실제로 바뀌었을 때만" 써서 내부 플래시 쓰기 수명을 아낌 */
void wd_fw_boot_success_mark(void) {
#if (RTE_SERVICE_MMU)
    wd_firmware_header_t active_header;
    wd_firmware_header_t factory_header;
    uint32_t active_total_size = 0;
    bool need_clone = false;

    if (wd_fw_check_image(&active_header, (uint32_t)wd_fw_header_app, &active_total_size) ==
        WD_CODE_OK) {
        if (wd_fw_check_image(&factory_header, (uint32_t)MEMORY_OBJECT_APPLICATION_FACTORY, NULL) !=
            WD_CODE_OK) {
            need_clone = true; // Factory가 비어있거나(최초 1회) 손상됨
        } else if (active_header.File_CheckSum != factory_header.File_CheckSum) {
            /* Commit 필드는 외부 패키징 도구가 채워주지 않아(항상 0) 버전 비교 기준으로 쓸 수 없음.
       * 이미지 내용이 조금이라도 바뀌면 반드시 달라지는 File_CheckSum(전체 이미지 wd_toolkit_crc32,
       * Firmware_Image_Check에서 이미 검증에 사용 중)으로 비교 — 외부 도구 수정 불필요. */
            need_clone = true; // Active 이미지 내용이 Factory와 다름 — 최신으로 갱신
        }

        if (need_clone) {
            if (active_total_size <= (uint32_t)MEMORY_OBJECT_APPLICATION_FACTORY_SIZE) {
                WD_PRINT_INFO(WD_MSG_NOTI_CRLF, "Factory Image Update from Active (%u bytes)",
                              active_total_size);
                /* 일반 wd_mmu_access(INIT_0xFF+WRITE) 대신 IFlash_Write_Raw를 직접 호출한다.
         * 그 경로(wd_iflash_program_one_sector_merged)는 섹터 마지막 4바이트를 자체 wd_toolkit_crc16 저장용으로
         * 예약해서 실제 데이터를 덮어써버리는데, 이 이미지는 섹터 여러 개에 걸치므로 그 경계마다
         * 원본 바이트가 손실되어 File_CheckSum(wd_toolkit_crc32) 검증이 항상 실패하는 문제가 있었다
         * (Factory가 계속 Bad로 남아 매 부팅마다 재클론이 반복됨). Factory 이미지는 자체
         * wd_firmware_header_t CRC32를 이미 갖고 있어 섹터별 예약 CRC가 필요 없다. */
                if (wd_iflash_write_raw((uint32_t)MEMORY_OBJECT_APPLICATION_FACTORY,
                                        (void *)wd_fw_header_app,
                                        active_total_size) != WD_CODE_OK) {
                    WD_LOG_ERROR("Factory Image Clone Write Failed");
                }
            } else {
                WD_LOG_ERROR("Active Image too large for Factory slot");
            }
        }
    }

    if (wd_fw_data.Boot_Fail_Count != 0) {
        wd_fw_data.Boot_Fail_Count = 0;

        {
            wd_mmu_multi_object_t object = {
                .Mask = WD_MMU_OBJECT_ALL,
                .Object_Size = 1,
                .Num = sizeof(wd_firmware_t),
                .Data = &wd_fw_data,
                .Object[0] = &MAP->ROM->Firmware,
            };

            wd_fw_data.Digest =
                wd_toolkit_crc16(&wd_fw_data, sizeof(wd_fw_data) - sizeof(wd_fw_data.Digest));
            wd_mmu_write_multi(WD_MMU_COMMAND_WRITE_CRC, &object);
        }
    }
#endif
}

#endif /*--------------- RTE_SERVICE_FIRMWARE ---------------*/

#endif //#if 1 // 토글
