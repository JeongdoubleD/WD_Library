# WD Library 명명·파일 규칙

근거 : 범용 임베디드 C 코딩 표준 SW-STD-C-001 v1.0 (GEN-S-001, GEN-S-002, GEN-S-006, GEN-M-003, GEN-M-020)
적용 제외 : `_WD_Third_Party/*` (원본 유지), CubeMX 생성 코드

## 1. 식별자

| 대상 | 규칙 | 예 |
|---|---|---|
| 공개 함수 | `wd_<모듈>_<동작>[_<대상>]` lower_snake_case | `wd_time_get_us()`, `wd_env_save()` |
| 파일 내부(static) 함수 | `<동작>[_<대상>]` lower_snake_case, `wd_` 접두사 없음 | `parse_header()` |
| 타입(typedef) | `wd_<모듈>_<이름>_t` | `wd_register_timer_t`, `wd_drv_vio_t` |
| 구조체/열거 태그 | 타입과 같은 이름에서 `_t` 제거 + `_s`/`_e` | `struct wd_register_timer_s` |
| 열거 상수 / 매크로 | `WD_<모듈>_<이름>` UPPER_SNAKE_CASE | `WD_VIO_MUX_1`, `WD_LOG_ERROR()` |
| 기능 선택 매크로 | `RTE_<레이어>_<이름>` (CMSIS 관례 유지) | `RTE_COMPONENT_NVM_NOR_FLASH` |
| 설정값 매크로 | `WD_CONF_<이름>` (`_CONF_xxx` 는 예약 식별자라 변경) | `WD_CONF_DMA_ALIGNMENT_SIZE` |
| 보드 HAL 핸들 매핑 | `RTE_<레이어>_<모듈>_<역할>_HANDLE` (포인터), 레지스터는 `_REGISTER`. 기본값은 모듈 헤더 `#ifndef`, 재정의는 `wd_board.h` | `RTE_COMPONENT_SDRAM_DRIVER_HANDLE` |
| 보드 함수형 훅 매크로 | `WD_BOARD_<이름>()` | `WD_BOARD_ERROR_HANDLER()` |
| 외부 연결 전역 변수 | `wd_<모듈>_<이름>` | `wd_env`, `wd_reg_status` |
| 파일 범위 static 변수 | `s_<이름>` | `s_rx_count` |
| 드라이버 객체 (함수 포인터 구조체) | 객체 `wd_drv_<모듈>`, 타입 `wd_drv_<모듈>_t` | `wd_drv_vio.set(...)` |
| 구조체 멤버 / 지역 변수 / 매개변수 | lower_snake_case | `sampling_rate_hz` |
| include guard | `WD_<경로>_<파일>_H` (선행 `_` 금지) | `WD_PLATFORM_WD_TIME_H` |

### 금지
- `_` + 대문자, `__` 로 시작하는 식별자 정의 (C 예약 식별자, GEN-M-003) : 라이브러리 코드에 없음 (예외 `__read` / `__write`, COMPLIANCE.md DEV-WD-005)
- 약어만으로 된 모호한 이름 (`tmp2`, `buf3`) : 의미 또는 단위를 붙인다

> 가변 인자 로그 매크로는 함수형 매크로 제한(GEN-M-020)의 편차로 등록한다 (COMPLIANCE.md DEV-WD-002).

### 단위 접미사 (GEN-S-006)
`_us`, `_ms`, `_s`, `_hz`, `_khz`, `_mv`, `_v`, `_ma`, `_a`, `_c`(섭씨), `_bytes`, `_bits`, `_ppm`, `_pct`

## 2. 모듈 이름표 (`wd_<모듈>_`)

| 레이어 | 모듈명 (파일 `_wd_<모듈>.c/.h`) |
|---|---|
| Core (`_WD_Core`) | `core`, `def`, `version`, `if`, `print`, `time`, `gpio`, `toolkit`, `cbuf`, `ringbuf`, `os` |
| Include (`_WD_Include`) | `wd.h` (우산), `wd_config_check.h`, `wd_compiler.h` |
| Platform (`_WD_Platform`, 선택) | `print_hal`, `print_serial`, `fft` |
| Drivers (`_WD_Drivers`) | `softspi`, `softi2c`, `blockdev`, `gpiofunc` |
| Component (`_WD_Component`) | `ad4134`, `ads127l`, `ad9837`, `adc`, `dds`, `ad5235`, `mcp23017`, `nor`, `nand`, `sdram_is42s`, `sdram`, `iflash`, `ds18b20`, `ntc`, `fdcan`, `eth`, `eth_telnet`, `vio` |
| Service (`_WD_Service`) | `mmu`, `fw`, `net`, `net_eth`, `net_can`, `factory` |
| System (`_WD_System`) | `reg`, `sys`, `env`, `sched`, `taskmon`, `faultlog` |

## 3. 파일·폴더

- 폴더 : `_WD_Include`, `_WD_Platform`, `_WD_Drivers`, `_WD_Component/_Component_xxx`, `_WD_Service/_xxx`, `_WD_System`, `_WD_Config_Template`, `_WD_Third_Party`
- 파일 : **`_wd_<모듈>.c / .h`** (선행 `_` + 모듈명 lower_snake, GEN-S-002 "파일명 = 모듈명")

