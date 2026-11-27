/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_iflash.c
 *
 *  Created on: Feb 7, 2025
 *      Author: doubled
 */

#include "_wd_iflash.h"

#if (RTE_COMPONENT_FLASH_INTERNAL)

// ─────────────────────────────────────────────────────────────────────────────
// PATCH: 옵션 토글들
#define RTE_I_FLASH_CRC_ON_READ 0 // 섹터 단위 Read 시 CRC 검증(옵션)
// ─────────────────────────────────────────────────────────────────────────────

// [PATCH][ADD] CRC tail 정의
#ifndef ROM_SECTOR_CRC_BYTES
#define ROM_SECTOR_CRC_BYTES 4u
#endif

#ifndef ROM_SECTOR_SIZE
// 내부 플래시 erase sector 크기(=128KB) 매크로로 맞춤
#define ROM_SECTOR_SIZE RTE_BLOCK_DEVICE_I_FLASH_ERASE_SECTOR_SIZE
#endif

// [PATCH] 섹터 버퍼 배치(권장: D2/D3). 링커에 .RAM_D2 섹션이 없다면 기본 RAM에 배치됨.
#ifndef IFLASH_SECTOR_BUF_ATTR
#define IFLASH_SECTOR_BUF_ATTR __attribute__((section(".RAM_D2")))
#endif
static uint8_t s_sector_buf[ROM_SECTOR_SIZE] IFLASH_SECTOR_BUF_ATTR __attribute__((aligned(32)));

static inline uint32_t ROM_SectorBase(uint32_t a) {
    return a & ~(ROM_SECTOR_SIZE - 1u);
}
static inline uint32_t ROM_SectorCRCAddr(uint32_t base) {
    return base + ROM_SECTOR_SIZE - ROM_SECTOR_CRC_BYTES;
}

#define API_VALIDATION 0
//#define FLASH_MEM

static wd_blockdev_instance_info_t Block_Device_I_Flash_Info = {
    0,
};

#if (RTE_RTOS)
static osStaticMutexDef_t Mutex_Block_Device_I_Flash_cb_mem;
static osMutexId_t Mutex_Block_Device_I_Flash;
static const osMutexAttr_t Mutex_Block_Device_I_Flash_Attr = {
    .name = "Mutex Block_Device_I_Flash",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = &Mutex_Block_Device_I_Flash_cb_mem,
    .cb_size = sizeof(Mutex_Block_Device_I_Flash_cb_mem)};

static int32_t res_start(void) {
    Mutex_Block_Device_I_Flash = osMutexNew(&Mutex_Block_Device_I_Flash_Attr);
    if (Mutex_Block_Device_I_Flash == NULL) {
#ifndef NDEBUG
        WD_LOG_ERROR("Mutex Block_Device_I_Flash New Error");
#endif
        return (WD_CODE_ERROR);
    }
    return (WD_CODE_OK);
}

static int32_t res_lock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        if (osMutexAcquire(Mutex_Block_Device_I_Flash, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
            return (WD_CODE_ERROR_TIMEOUT);
        }
    }
    return (WD_CODE_OK);
}

static int32_t res_unlock(void) {
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(Mutex_Block_Device_I_Flash);
    }
    return (WD_CODE_OK);
}
#else
static void res_start(void) {
}
static void res_lock(void) {
}
static void res_unlock(void) {
}
#endif

// ============================== PATCH: helpers ==============================
static inline uint16_t CRC16_update_raw(uint16_t crc, const uint8_t *p, uint32_t n) {
    while (n--) {
        uint8_t d = (uint8_t)(*p++ ^ (crc & 0xFF));
        d ^= (uint8_t)(d << 4);
        crc = (uint16_t)((((uint16_t)d) << 3) ^ (((uint16_t)d) << 8) ^ (crc >> 8) ^ (d >> 4));
    }
    return crc;
}
static inline uint16_t CRC16_stream_finish(uint16_t crc) {
    return (uint16_t)(crc ^ 0xFFFFu);
}

static inline void dc_clean_32(void *p, uint32_t len) {
    uintptr_t a = ((uintptr_t)p) & ~31u;
    uint32_t l = (uint32_t)((((uintptr_t)p + len) - a + 31u) & ~31u);
    SCB_CleanDCache_by_Addr((void *)a, (int32_t)l);
}
static inline void dc_inval_32(void *p, uint32_t len) {
    uintptr_t a = ((uintptr_t)p) & ~31u;
    uint32_t l = (uint32_t)((((uintptr_t)p + len) - a + 31u) & ~31u);
    SCB_InvalidateDCache_by_Addr((void *)a, (int32_t)l);
}

// 동일 뱅크 RWW 방지(간단히 IRQ off)
#if (RTE_RTOS)
#define FLASH_CRIT_ENTER()                         \
    do {                                           \
        __disable_irq(); /*옵션: osKernelLock();*/ \
    } while (0)
#define FLASH_CRIT_EXIT()            \
    do { /*옵션: osKernelUnlock();*/ \
        __enable_irq();              \
    } while (0)
#else
#define FLASH_CRIT_ENTER() __disable_irq()
#define FLASH_CRIT_EXIT()  __enable_irq()
#endif
// ===========================================================================

static wd_blockdev_instance_resources_t Block_Device_I_Flash_Resources = {
    .Capabilities =
        {
            .Name = "[MCU I-ROM]",
            .Addr = RTE_BLOCK_DEVICE_I_FLASH_ADDR,
            .Size = RTE_BLOCK_DEVICE_I_FLASH_SIZE,
            .Program_Size = RTE_BLOCK_DEVICE_I_FLASH_PROGRAM_SIZE, // 32B
            .Erase_Sector_Size = RTE_BLOCK_DEVICE_I_FLASH_ERASE_SECTOR_SIZE,
            .Erase_Block_Size = RTE_BLOCK_DEVICE_I_FLASH_ERASE_BLOCK_SIZE,
            .Erase_Block_Addr = RTE_BLOCK_DEVICE_I_FLASH_ERASE_BLOCK_ADDR,
            .Program_TimeOut = RTE_BLOCK_DEVICE_I_FLASH_PROGRAM_TIMEOUT,
            .Erase_Sector_TimeOut = RTE_BLOCK_DEVICE_I_FLASH_ERASE_SECTOR_TIMEOUT,
            .Erase_Block_TimeOut = RTE_BLOCK_DEVICE_I_FLASH_ERASE_BLOCK_TIMEOUT,
            .Erase_Chip_TimeOut = RTE_BLOCK_DEVICE_I_FLASH_ERASE_CHIP_TIMEOUT,
            .Buf = NULL,
            .Buf_Size = 0,
            .Confirm_Buf = NULL,
            .Confirm_Buf_Size = 0,
            .Direct_Access = 1U,
        },
    .Info = &Block_Device_I_Flash_Info,
    .IO =
        {
            .WP = RTE_CONFIG_IDLE_PIN_NC,
            .RESET = RTE_CONFIG_IDLE_PIN_NC,
        },
    .Drv = NULL,
    .Handle = NULL,
    .Reg = FLASH,
    .Start = res_start,
    .Lock = res_lock,
    .Unlock = res_unlock,
};

#undef LOCAL_DRV
#define LOCAL_DRV (&wd_drv_block_device_i_flash)
__USED static wd_blockdev_instance_resources_t *const LOCAL_RES = &Block_Device_I_Flash_Resources;

// 은행/섹터 계산
static uint32_t GetSector(uint32_t Address) {
#define ADDR_FLASH_SECTOR_0_BANK1 ((uint32_t)0x08000000)
#define ADDR_FLASH_SECTOR_1_BANK1 ((uint32_t)0x08020000)
#define ADDR_FLASH_SECTOR_2_BANK1 ((uint32_t)0x08040000)
#define ADDR_FLASH_SECTOR_3_BANK1 ((uint32_t)0x08060000)
#define ADDR_FLASH_SECTOR_4_BANK1 ((uint32_t)0x08080000)
#define ADDR_FLASH_SECTOR_5_BANK1 ((uint32_t)0x080A0000)
#define ADDR_FLASH_SECTOR_6_BANK1 ((uint32_t)0x080C0000)
#define ADDR_FLASH_SECTOR_7_BANK1 ((uint32_t)0x080E0000)
#define ADDR_FLASH_SECTOR_0_BANK2 ((uint32_t)0x08100000)
#define ADDR_FLASH_SECTOR_1_BANK2 ((uint32_t)0x08120000)
#define ADDR_FLASH_SECTOR_2_BANK2 ((uint32_t)0x08140000)
#define ADDR_FLASH_SECTOR_3_BANK2 ((uint32_t)0x08160000)
#define ADDR_FLASH_SECTOR_4_BANK2 ((uint32_t)0x08180000)
#define ADDR_FLASH_SECTOR_5_BANK2 ((uint32_t)0x081A0000)
#define ADDR_FLASH_SECTOR_6_BANK2 ((uint32_t)0x081C0000)
#define ADDR_FLASH_SECTOR_7_BANK2 ((uint32_t)0x081E0000)

    if (((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1)) ||
        ((Address < ADDR_FLASH_SECTOR_1_BANK2) && (Address >= ADDR_FLASH_SECTOR_0_BANK2))) {
        return FLASH_SECTOR_0;
    }
    if (((Address < ADDR_FLASH_SECTOR_2_BANK1) && (Address >= ADDR_FLASH_SECTOR_1_BANK1)) ||
        ((Address < ADDR_FLASH_SECTOR_2_BANK2) && (Address >= ADDR_FLASH_SECTOR_1_BANK2))) {
        return FLASH_SECTOR_1;
    }
    if (((Address < ADDR_FLASH_SECTOR_3_BANK1) && (Address >= ADDR_FLASH_SECTOR_2_BANK1)) ||
        ((Address < ADDR_FLASH_SECTOR_3_BANK2) && (Address >= ADDR_FLASH_SECTOR_2_BANK2))) {
        return FLASH_SECTOR_2;
    }
    if (((Address < ADDR_FLASH_SECTOR_4_BANK1) && (Address >= ADDR_FLASH_SECTOR_3_BANK1)) ||
        ((Address < ADDR_FLASH_SECTOR_4_BANK2) && (Address >= ADDR_FLASH_SECTOR_3_BANK2))) {
        return FLASH_SECTOR_3;
    }
    if (((Address < ADDR_FLASH_SECTOR_5_BANK1) && (Address >= ADDR_FLASH_SECTOR_4_BANK1)) ||
        ((Address < ADDR_FLASH_SECTOR_5_BANK2) && (Address >= ADDR_FLASH_SECTOR_4_BANK2))) {
        return FLASH_SECTOR_4;
    }
    if (((Address < ADDR_FLASH_SECTOR_6_BANK1) && (Address >= ADDR_FLASH_SECTOR_5_BANK1)) ||
        ((Address < ADDR_FLASH_SECTOR_6_BANK2) && (Address >= ADDR_FLASH_SECTOR_5_BANK2))) {
        return FLASH_SECTOR_5;
    }
    if (((Address < ADDR_FLASH_SECTOR_7_BANK1) && (Address >= ADDR_FLASH_SECTOR_6_BANK1)) ||
        ((Address < ADDR_FLASH_SECTOR_7_BANK2) && (Address >= ADDR_FLASH_SECTOR_6_BANK2))) {
        return FLASH_SECTOR_6;
    }
    if (((Address < ADDR_FLASH_SECTOR_0_BANK2) && (Address >= ADDR_FLASH_SECTOR_7_BANK1)) ||
        ((Address < FLASH_END) && (Address >= ADDR_FLASH_SECTOR_7_BANK2))) {
        return FLASH_SECTOR_7;
    }

    WD_LOG_ERROR();
    return FLASH_SECTOR_7;
}

static uint32_t GetBank(uint32_t Addr) {
    if (IS_FLASH_PROGRAM_ADDRESS_BANK1(Addr)) {
        return FLASH_BANK_1;
    }
    if (IS_FLASH_PROGRAM_ADDRESS_BANK2(Addr)) {
        return FLASH_BANK_2;
    }
    WD_LOG_ERROR();
    return FLASH_BANK_1;
}

// ───────────────────────── CRC 보조 루틴들 ─────────────────────────
static void sector_patch_crc(uint8_t *sec /*128KB*/) {
    uint16_t crc = 0xFFFFu;
    const uint32_t N = ROM_SECTOR_SIZE - ROM_SECTOR_CRC_BYTES;
    uint32_t pos = 0, step = 1024;
    while (pos < N) {
        uint32_t n = (N - pos > step) ? step : (N - pos);
        crc = CRC16_update_raw(crc, sec + pos, n);
        pos += n;
    }
    crc = CRC16_stream_finish(crc);
    uint32_t crc_word = (uint32_t)crc | ((uint32_t)(~crc) << 16);
    memcpy(sec + N, &crc_word, sizeof(crc_word));
}

// PATCH: “Erase 없이” 섹터 꼬리 4B만 쓰며 CRC를 마무리(가능할 때만 1회)
static int32_t rom_finalize_sector_crc_light(uint32_t sector_base) {
    const uint32_t crc_tail_addr = ROM_SectorCRCAddr(sector_base);
    const uint32_t flashword_base = crc_tail_addr & ~31u;
    const uint32_t flashword_off = crc_tail_addr - flashword_base;

    // 기대 CRC 계산(섹터 전 구간, tail 4B 제외)
    uint16_t crc = 0xFFFFu;
    const uint32_t N = ROM_SECTOR_SIZE - ROM_SECTOR_CRC_BYTES;
    uint32_t pos = 0, step = 1024;
    while (pos < N) {
        uint32_t n = (N - pos > step) ? step : (N - pos);
        crc = CRC16_update_raw(crc, (const uint8_t *)(sector_base + pos), n);
        pos += n;
    }
    crc = CRC16_stream_finish(crc);
    const uint32_t crc_word = (uint32_t)crc | ((uint32_t)(~crc) << 16);

    // 이미 저장된 값 확인
    uint32_t stored;
    memcpy(&stored, (void *)crc_tail_addr, 4);
    if (stored == crc_word) {
        return WD_CODE_OK; // 이미 일치
    }
    if (stored != 0xFFFFFFFFu) {
        return WD_CODE_ERROR; // 0->1 필요 → Erase 없이는 불가
    }

    // tail이 포함된 32B 블록 RMW
    uint8_t fw[32];
    memcpy(fw, (void *)flashword_base, 32);
    memcpy(fw + flashword_off, &crc_word, sizeof(crc_word));

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return WD_CODE_ERROR;
    }
#ifdef FLASH_FLAG_ALL_BANK1
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK2);
#else
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR |
                           FLASH_FLAG_OPTVERR | FLASH_FLAG_ECCC1 | FLASH_FLAG_ECCD1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR |
                           FLASH_FLAG_OPTVERR | FLASH_FLAG_ECCC2 | FLASH_FLAG_ECCD2);
#endif
    dc_clean_32(fw, 32);
    dc_inval_32((void *)flashword_base, 32);
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flashword_base, (uint32_t)fw) != HAL_OK) {
        HAL_FLASH_Lock();
        return WD_CODE_ERROR;
    }
    HAL_FLASH_Lock();

    uint32_t chk;
    memcpy(&chk, (void *)crc_tail_addr, 4);
    return (chk == crc_word) ? WD_CODE_OK : WD_CODE_ERROR;
}

// PATCH: [addr, addr+len) 범위가 걸치는 모든 섹터를 “가볍게” 마무리
static int32_t rom_finalize_crcs_for_range(uint32_t addr, uint32_t len) {
    if (len == 0) {
        return WD_CODE_OK;
    }
    uint32_t start = ROM_SectorBase(addr);
    uint32_t end = ROM_SectorBase(addr + len - 1u);
    for (uint32_t base = start;; base += ROM_SECTOR_SIZE) {
        int32_t r = rom_finalize_sector_crc_light(base);
        if (r != WD_CODE_OK) {
            return r;
        }
        if (base == end) {
            break;
        }
    }
    return WD_CODE_OK;
}

// PATCH: 외부 API — (호환용) 필요 시 “마지막에 CRC만” 찍고 싶을 때 호출
int32_t wd_iflash_finalize_cr_cs(uint32_t addr, uint32_t len) {
    // 주: 본 드라이버는 Write 경로에서 이미 섹터 CRC를 갱신합니다.
    // 이 API는 상위 호환/테스트용으로 남겨둔 “가벼운 최종화”입니다.
    return rom_finalize_crcs_for_range(addr, len);
}

// ─────────────────────── 섹터 스냅샷/머지/전체 재프로그램 ───────────────────────
static int32_t wd_iflash_program_one_sector_merged(uint32_t sector_base, uint32_t wr_start,
                                                   const uint8_t *wr_data, uint32_t wr_len) {
#if 0 //NDEBUG
	uint8_t f0[16], b0[16];
	memcpy(f0, (void*)sector_base, 16);
	memcpy(b0, s_sector_buf,       16);
	printf("[MERGE] base=%08lX wr=%08lX len=%lu  flash:%02X %02X %02X %02X  buf:%02X %02X %02X %02X\r\n",
				 sector_base, wr_start, (unsigned long)wr_len,
				 f0[0], f0[1], f0[2], f0[3], b0[0], b0[1], b0[2], b0[3]);
#endif


    // [PATCH-1] ★★★ 함수 시작부에 추가 (스냅샷 전) ★★★
    _Static_assert((ROM_SECTOR_SIZE & (ROM_SECTOR_SIZE - 1)) == 0,
                   "ROM_SECTOR_SIZE must be power-of-two");
    _Static_assert(ROM_SECTOR_SIZE == 128 * 1024, "ROM_SECTOR_SIZE must be 128KB on H743");

    if (!(IS_FLASH_PROGRAM_ADDRESS_BANK1(sector_base) ||
          IS_FLASH_PROGRAM_ADDRESS_BANK2(sector_base))) {
        WD_LOG_ERROR("sector_base(0x%08lX) not in FLASH", sector_base);
        return WD_CODE_ERROR_PARAMETER;
    }

    // AXI Flash는 보통 캐시 가능: 새로 읽게 해주자
    dc_inval_32((void *)sector_base, ROM_SECTOR_SIZE);

    // 0) 스냅샷
    memcpy(s_sector_buf, (void *)sector_base, ROM_SECTOR_SIZE);

    // 1) 변경 영역을 섹터 버퍼에 덮어쓰기 (CRC 영역은 보호)
    const uint32_t crc_tail = ROM_SectorCRCAddr(sector_base);
    uint32_t s = wr_start;
    uint32_t e = wr_start + wr_len;
    if (e > crc_tail) {
        e = crc_tail; // CRC 꼬리 4바이트는 사용자 데이터로 덮지 않음
    }

    // [PATCH-2] ★★★ e/s 계산 직후에 추가 ★★★
    if (wr_len == 0) {
        // 덮을 게 없으면 섹터를 건드릴 이유가 없다
        return WD_CODE_OK;
    }

    if (e <= s) {
        // 요청 범위가 CRC tail(마지막 4B) 안쪽만 가리키는 경우 등: 덮을 데이터 없음
        // 여기서 섹터 지우기까지 가면 안됨!
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG,
                      "skip merge: s>=e (s=0x%08lX e=0x%08lX) wr_start=0x%08lX wr_len=%lu", s, e,
                      wr_start, wr_len);
        return WD_CODE_OK;
    }

    size_t off = (size_t)(s - sector_base);
    size_t len = (size_t)(e - s);
    if (off + len > ROM_SECTOR_SIZE) {
        WD_LOG_ERROR("merge range overflow: off=%u len=%u", (unsigned)off, (unsigned)len);
        return WD_CODE_ERROR_PARAMETER;
    }

    if (e > s) {
        memcpy(s_sector_buf + (s - sector_base), wr_data, (e - s));
    }

    // 2) CRC 계산/패치
    sector_patch_crc(s_sector_buf);

    // 3) 섹터 Erase
    FLASH_EraseInitTypeDef ei = {0};
    uint32_t se = 0;
    ei.TypeErase = FLASH_TYPEERASE_SECTORS;
    ei.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    ei.Banks = GetBank(sector_base);
    ei.Sector = GetSector(sector_base);
    ei.NbSectors = 1;
    if (HAL_FLASHEx_Erase(&ei, &se) != HAL_OK) {
        return WD_CODE_ERROR;
    }

    // 4) 섹터 전체 재프로그램 (32B 단위)
    for (uint32_t prog_off = 0; prog_off < ROM_SECTOR_SIZE; prog_off += 32) {
        dc_clean_32(s_sector_buf + prog_off, 32);
        dc_inval_32((void *)(sector_base + prog_off), 32);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, sector_base + prog_off,
                              (uint32_t)(s_sector_buf + prog_off)) != HAL_OK) {
            return WD_CODE_ERROR;
        }
    }

    // 5) 검증(섹터 전체)
    dc_inval_32((void *)sector_base, ROM_SECTOR_SIZE);
    if (memcmp((void *)sector_base, s_sector_buf, ROM_SECTOR_SIZE) != 0) {
        return WD_CODE_ERROR;
    }

    return WD_CODE_OK;
}

// PATCH: Factory 이미지 전용 raw 섹터 프로그램 — _program_one_sector_merged와 달리
// 기존 내용을 보존/머지하지 않고(어차피 클론은 항상 전체 덮어쓰기) 섹터 마지막 4바이트도
// CRC 예약 없이 실제 데이터로 취급한다. 요청 범위 밖의 나머지 섹터 바이트는 지운 상태(0xFF)로 남는다.
static int32_t program_one_sector_raw(uint32_t sector_base, uint32_t wr_start,
                                      const uint8_t *wr_data, uint32_t wr_len) {
    if (!(IS_FLASH_PROGRAM_ADDRESS_BANK1(sector_base) ||
          IS_FLASH_PROGRAM_ADDRESS_BANK2(sector_base))) {
        WD_LOG_ERROR("sector_base(0x%08lX) not in FLASH", sector_base);
        return WD_CODE_ERROR_PARAMETER;
    }

    memset(s_sector_buf, 0xFF, ROM_SECTOR_SIZE);
    if (wr_len > 0) {
        memcpy(s_sector_buf + (wr_start - sector_base), wr_data, wr_len);
    }

    FLASH_EraseInitTypeDef ei = {0};
    uint32_t se = 0;
    ei.TypeErase = FLASH_TYPEERASE_SECTORS;
    ei.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    ei.Banks = GetBank(sector_base);
    ei.Sector = GetSector(sector_base);
    ei.NbSectors = 1;
    if (HAL_FLASHEx_Erase(&ei, &se) != HAL_OK) {
        return WD_CODE_ERROR;
    }

    for (uint32_t off = 0; off < ROM_SECTOR_SIZE; off += 32) {
        dc_clean_32(s_sector_buf + off, 32);
        dc_inval_32((void *)(sector_base + off), 32);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, sector_base + off,
                              (uint32_t)(s_sector_buf + off)) != HAL_OK) {
            return WD_CODE_ERROR;
        }
    }

    dc_inval_32((void *)sector_base, ROM_SECTOR_SIZE);
    if (memcmp((void *)sector_base, s_sector_buf, ROM_SECTOR_SIZE) != 0) {
        return WD_CODE_ERROR;
    }

    return WD_CODE_OK;
}

int32_t wd_iflash_write_raw(uint32_t addr, const void *data, uint32_t num) {
    int32_t ret = WD_CODE_OK;

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }
    if ((!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size)) ||
        (!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr + num,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size))) {
        return (WD_CODE_ERROR);
    }

    LOCAL_RES->Lock();

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (osMutexAcquire(wd_os_mutex_global, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
            WD_BREAK();
        }
    }
#endif

    if (HAL_FLASH_Unlock() != HAL_OK) {
        ret = WD_CODE_ERROR;
        goto __end_raw__;
    }

#ifdef FLASH_FLAG_ALL_BANK1
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK2);
#else
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR |
                           FLASH_FLAG_OPTVERR | FLASH_FLAG_ECCC1 | FLASH_FLAG_ECCD1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR |
                           FLASH_FLAG_OPTVERR | FLASH_FLAG_ECCC2 | FLASH_FLAG_ECCD2);
#endif
    while (FLASH->SR1 & FLASH_SR_BSY) {
    }
    while (FLASH->SR2 & FLASH_SR_BSY) {
    }

    FLASH_CRIT_ENTER();

    {
        uint32_t p = 0;
        while (p < num && ret == WD_CODE_OK) {
            const uint32_t cur = addr + p;
            const uint32_t sector_base = ROM_SectorBase(cur);
            uint32_t sector_end = sector_base + ROM_SECTOR_SIZE;
            const uint32_t req_end = addr + num;
            if (sector_end > req_end) {
                sector_end = req_end;
            }

            const uint32_t chunk_len = sector_end - cur;

            if (program_one_sector_raw(sector_base, cur, (const uint8_t *)data + p, chunk_len) !=
                WD_CODE_OK) {
                ret = WD_CODE_ERROR;
                break;
            }

            p += chunk_len;
        }
    }

    FLASH_CRIT_EXIT();

__end_raw__:
    HAL_FLASH_Lock();

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        osMutexRelease(wd_os_mutex_global);
    }
#endif
    LOCAL_RES->Unlock();
    WD_CHECK(ret);
    return ret;
}

__USED static void wd_block_device_i_flash_callback(uint32_t event) {
    LOCAL_RES->Info->Event = event;
    if (LOCAL_RES->Info->CB_Event) {
        LOCAL_RES->Info->CB_Event(event);
    }
}

static int32_t block_device_i_flash_initialize(wd_event_callback_t cb_event) {
    if (LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) {
        return (WD_CODE_OK);
    }
    WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Block_Device_I_Flash Constructor");
    memset(LOCAL_RES->Info, 0, sizeof(wd_blockdev_instance_info_t));
    if (cb_event) {
        LOCAL_RES->Info->CB_Event = cb_event;
    }
    LOCAL_RES->Start();
    LOCAL_RES->Info->Flags = WDIF_FLAG_INITIALIZED;
    return (WD_CODE_OK);
}

static int32_t block_device_i_flash_uninitialize(void) {
    return (WD_CODE_OK);
}

static int32_t block_device_i_flash_power_control(wd_power_state_t state) {
    switch ((int32_t)state) {
    case WD_POWER_OFF:
        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Block_Device_I_Flash PowerControl:WD_POWER_OFF");
        LOCAL_RES->Info->Flags &= ~(WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
        break;

    case WD_POWER_FULL:
        if ((LOCAL_RES->Info->Flags & WDIF_FLAG_INITIALIZED) == 0U) {
            return (WD_CODE_ERROR);
        }
        if (LOCAL_RES->Info->Flags & WDIF_FLAG_POWERED) {
            return (WD_CODE_OK);
        }

        // PATCH: 섹터 스냅샷용 128KB 버퍼 사용
        LOCAL_RES->Info->Xfer.Buf = s_sector_buf;
        LOCAL_RES->Info->Xfer.Buf_Size = sizeof(s_sector_buf);

        WD_PRINT_INFO(WD_MSG_DEBUG_LOG, "Block_Device_I_Flash PowerControl:WD_POWER_FULL");
        LOCAL_RES->Info->Flags |= (WDIF_FLAG_POWERED | WDIF_FLAG_CONFIGURED);
        break;

    case WD_POWER_LOW:
        return (WD_CODE_ERROR_UNSUPPORTED);
    default:
        break;
    }
    return (WD_CODE_OK);
}

// ============================== WRITE ==============================
static int32_t block_device_i_flash_write(uint32_t addr, const void *data, uint32_t num) {
    int32_t ret = WD_CODE_OK;

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }
    if ((!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size)) ||
        (!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr + num,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size))) {
        return (WD_CODE_ERROR);
    }

    LOCAL_RES->Lock();

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == s_sector_buf) {
            if (osMutexAcquire(wd_os_mutex_global, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
                WD_BREAK();
            }
        }
    }
#endif

#ifdef FLASH_MEM
    ret = WD_CODE_ERROR_UNSUPPORTED;
#else
    if (HAL_FLASH_Unlock() != HAL_OK) {
        ret = WD_CODE_ERROR;
        goto __end__;
    }

#ifdef FLASH_FLAG_ALL_BANK1
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK2);
#else
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR |
                           FLASH_FLAG_OPTVERR | FLASH_FLAG_ECCC1 | FLASH_FLAG_ECCD1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR |
                           FLASH_FLAG_OPTVERR | FLASH_FLAG_ECCC2 | FLASH_FLAG_ECCD2);
#endif
    while (FLASH->SR1 & FLASH_SR_BSY) {
    }
    while (FLASH->SR2 & FLASH_SR_BSY) {
    }

    FLASH_CRIT_ENTER(); // 동일 뱅크 RDERR 회피

    uint32_t p = 0;
    while (p < num && ret == WD_CODE_OK) {
        const uint32_t cur = addr + p;
        const uint32_t sector_base = ROM_SectorBase(cur);
        uint32_t sector_end = sector_base + ROM_SECTOR_SIZE;
        const uint32_t req_end = addr + num;
        if (sector_end > req_end) {
            sector_end = req_end;
        }

        const uint32_t chunk_len = sector_end - cur;

        if (wd_iflash_program_one_sector_merged(sector_base, cur, (const uint8_t *)data + p,
                                                chunk_len) != WD_CODE_OK) {
            ret = WD_CODE_ERROR;
            break;
        }

        p += chunk_len;
    }

    FLASH_CRIT_EXIT();
#endif // !FLASH_MEM

__end__:
#ifndef FLASH_MEM
    HAL_FLASH_Lock();
#endif

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == s_sector_buf) {
            osMutexRelease(wd_os_mutex_global);
        }
    }
#endif
    LOCAL_RES->Unlock();
    WD_CHECK(ret);
    return ret;
}

// ============================== READ ==============================
static int32_t block_device_i_flash_read(uint32_t addr, void *data, uint32_t num) {
    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    if ((!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size)) ||
        (!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr + num,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size))) {
        return (WD_CODE_ERROR);
    }

#if defined(RTE_I_FLASH_CRC_ON_READ) && (RTE_I_FLASH_CRC_ON_READ)
    if ((addr % ROM_SECTOR_SIZE) == 0 && (num % ROM_SECTOR_SIZE) == 0) {
        for (uint32_t base = addr; base < addr + num; base += ROM_SECTOR_SIZE) {
            uint32_t tail = ROM_SectorCRCAddr(base);
            uint32_t stored;
            memcpy(&stored, (void *)tail, 4);
            if (stored == 0xFFFFFFFFu) {
                return WD_CODE_ERROR;
            }
        }
    }
#endif

    memcpy(data, (uint8_t *)addr, num);
    return (WD_CODE_OK);
}

// ============================== ERASE ==============================
static int32_t block_device_i_flash_erase(uint32_t addr, const uint8_t data, uint32_t num) {
    int32_t ret = WD_CODE_OK;

    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }
    if ((!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size)) ||
        (!WD_RANGE(LOCAL_RES->Capabilities.Addr, addr + num,
                   LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size))) {
        return (WD_CODE_ERROR);
    }

    LOCAL_RES->Lock();

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == s_sector_buf) {
            if (osMutexAcquire(wd_os_mutex_global, RTE_RTOS_MUTEX_TIMEOUT) != osOK) {
                WD_BREAK();
            }
        }
    }
#endif

#ifdef FLASH_MEM
    ret = WD_CODE_ERROR_UNSUPPORTED;
#else
    if (HAL_FLASH_Unlock() != HAL_OK) {
        ret = WD_CODE_ERROR;
        goto __end2__;
    }

#ifdef FLASH_FLAG_ALL_BANK1
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK2);
#else
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR |
                           FLASH_FLAG_OPTVERR | FLASH_FLAG_ECCC1 | FLASH_FLAG_ECCD1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR |
                           FLASH_FLAG_OPTVERR | FLASH_FLAG_ECCC2 | FLASH_FLAG_ECCD2);
#endif
    while (FLASH->SR1 & FLASH_SR_BSY) {
    }
    while (FLASH->SR2 & FLASH_SR_BSY) {
    }

    FLASH_CRIT_ENTER();

    uint32_t p = 0;
    while (p < num && ret == WD_CODE_OK) {
        const uint32_t cur = addr + p;
        const uint32_t sector_base = ROM_SectorBase(cur);
        uint32_t sector_end = sector_base + ROM_SECTOR_SIZE;
        const uint32_t req_end = addr + num;
        if (sector_end > req_end) {
            sector_end = req_end;
        }

        const uint32_t chunk_len = sector_end - cur;

        memcpy(s_sector_buf, (void *)sector_base, ROM_SECTOR_SIZE);

        const uint32_t crc_tail = ROM_SectorCRCAddr(sector_base);
        uint32_t s = cur, e = cur + chunk_len;
        if (e > crc_tail) {
            e = crc_tail;
        }
        if (e > s) {
            memset(s_sector_buf + (s - sector_base), data, (e - s));
        }

        sector_patch_crc(s_sector_buf);

        FLASH_EraseInitTypeDef ei = {0};
        uint32_t se = 0;
        ei.TypeErase = FLASH_TYPEERASE_SECTORS;
        ei.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        ei.Banks = GetBank(sector_base);
        ei.Sector = GetSector(sector_base);
        ei.NbSectors = 1;
        if (HAL_FLASHEx_Erase(&ei, &se) != HAL_OK) {
            ret = WD_CODE_ERROR;
            break;
        }

        for (uint32_t off = 0; off < ROM_SECTOR_SIZE; off += 32) {
            dc_clean_32(s_sector_buf + off, 32);
            dc_inval_32((void *)(sector_base + off), 32);
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, sector_base + off,
                                  (uint32_t)(s_sector_buf + off)) != HAL_OK) {
                ret = WD_CODE_ERROR;
                break;
            }
        }
        if (ret != WD_CODE_OK) {
            break;
        }

        dc_inval_32((void *)sector_base, ROM_SECTOR_SIZE);
        if (memcmp((void *)sector_base, s_sector_buf, ROM_SECTOR_SIZE) != 0) {
            ret = WD_CODE_ERROR;
            break;
        }

        p += chunk_len;
    }

    FLASH_CRIT_EXIT();

__end2__:
    HAL_FLASH_Lock();
#endif // !FLASH_MEM

#if (RTE_RTOS)
    if (!WD_CORE_IS_IN_ISR()) {
        if (LOCAL_RES->Info->Xfer.Buf == s_sector_buf) {
            osMutexRelease(wd_os_mutex_global);
        }
    }
#endif

    LOCAL_RES->Unlock();
    WD_CHECK(ret);
    return ret;
}

static int32_t block_device_i_flash_control(uint32_t control, uint32_t arg) {
    int32_t ret = WD_CODE_OK;
    if ((LOCAL_RES->Info->Flags & WDIF_FLAG_CONFIGURED) == 0U) {
        return (WD_CODE_ERROR);
    }

    switch (control & WDIF_BLOCK_DEVICE_COMMAND_MSK) {
    case WDIF_BLOCK_DEVICE_COMMAND_ERASE:
        if (!WD_RANGE(LOCAL_RES->Capabilities.Addr, arg,
                      (LOCAL_RES->Capabilities.Addr + LOCAL_RES->Capabilities.Size))) {
            return (WD_CODE_ERROR);
        }
        ret = WD_CODE_ERROR_UNSUPPORTED;
        break;

    default:
        ret = WD_CODE_ERROR_UNSUPPORTED;
        break;
    }
    return ret;
}

static int32_t block_device_i_flash_get_status(void) {
    return (WD_CODE_OK);
}

static void block_device_i_flash_validation(void) {
#if (API_VALIDATION)
    // (필요시 수동 테스트 루틴)
#endif
}

wd_blockdev_instance_info_t *const wd_iflash_res_block_device_i_flash = &Block_Device_I_Flash_Info;

wd_blockdev_instance_t wd_drv_block_device_i_flash = {
    .Initialize = block_device_i_flash_initialize,
    .Uninitialize = block_device_i_flash_uninitialize,
    .PowerControl = block_device_i_flash_power_control,
    .Write = block_device_i_flash_write,
    .Read = block_device_i_flash_read,
    .Erase = block_device_i_flash_erase,
    .Control = block_device_i_flash_control,
    .Get_Status = block_device_i_flash_get_status,
    .Validation = block_device_i_flash_validation,
    .Res = LOCAL_RES,
};

#ifndef NDEBUG
__USED wd_iflash_debug_block_device_i_flash_t wd_iflash_dbg_block_device_i_flash = {
    .Resource = LOCAL_RES,
    .Info = wd_iflash_res_block_device_i_flash,
    .Driver = LOCAL_DRV,
};
#endif

#endif // RTE_COMPONENT_FLASH_INTERNAL
