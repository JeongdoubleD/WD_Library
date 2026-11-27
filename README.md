# WD Library

STM32H7 (Cortex-M7) 용 재사용 드라이버·플랫폼 라이브러리. 여러 프로젝트가 같은 코드를 쓰고, 고친 내용을 다시 라이브러리로 모은다.

- **레이어 분리** : Core → Drivers → Component → Service. 상위가 하위만 참조하고, 라이브러리는 애플리케이션을 직접 참조하지 않는다
- **기능 선택** : `wd_config.h` 의 `RTE_xxx` 스위치로 필요한 모듈만 빌드 (끈 모듈은 전부 `#if` 로 빠짐)
- **보드 설정 분리** : 핀·HAL 핸들·메모리 배치는 설정 파일 3종(`wd_config.h`, `wd_board.h`, `wd_memory_map.h`)에만 있다
- **모듈 단독 사용** : 필수 코어 + 모듈 폴더만 복사해도 동작 (4장 의존표)
- **검증** : `-Wall -Wextra -Wconversion ... -Werror` 경고 0, 헤더 단독·모듈 단독·설정 조합 빌드 검사 스크립트 포함

| 항목 | |
|---|---|
| 버전 | `WD_Library/_WD_Core/wd_version.h` (Semantic Versioning), 변경 이력 [CHANGELOG.md](CHANGELOG.md) |
| 대상 | STM32H7 + STM32CubeH7 HAL, FreeRTOS (CMSIS-RTOS2), GCC (STM32CubeIDE) |
| 라이선스 | **MIT** ([LICENSE](LICENSE)). 일부 서드파티 코드는 원래 라이선스 → [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) |

## 1. 폴더 구조

```
WD_Library/                        (저장소)
├─ WD_Library/                     ★ 라이브러리 본체 : 프로젝트에 통째로 복사(또는 git subtree). 프로젝트에서 직접 수정하지 않음
│  ├─ _WD_Core/                    필수 코어 : wd_core.h, wd_def.h, wd_version.h, 콘솔·로그(_wd_print), 시간(_wd_time),
│  │                               GPIO, 버퍼(_wd_ringbuf, _wd_cbuf), RTOS 포트(_wd_os), 공통 인터페이스(_wd_if), 유틸(_wd_toolkit)
│  ├─ _WD_Include/                 wd.h (우산 헤더 = 코어 + 켜진 모듈), wd_config_check.h (RTE 조합 검사)
│  ├─ _WD_Config_Template/         새 보드용 설정 템플릿 3종 (wd_config.h / wd_board.h / wd_memory_map.h)
│  ├─ _WD_System/                  레지스터 모델, Environment(1KB 영구 설정), Scheduler, Task Monitor, Fault Log
│  ├─ _WD_Platform/                UART 콘솔 백엔드, 시리얼 출력, FFT
│  ├─ _WD_Drivers/                 Soft SPI, Soft I2C, GPIO, Block Device
│  ├─ _WD_Component/               ADC(AD4134, ADS127L), DDS(AD9837), DigiPOT(AD5235), IO Expander(MCP23017),
│  │                               NOR(MX66L2G45G) / NAND(W25N04), SDRAM(IS42S32800G), 내부 Flash,
│  │                               온도(DS18B20 / NTC), FDCAN, Ethernet(W6100 + Telnet 콘솔), Virtual IO
│  ├─ _WD_Service/                 MMU(메모리 추상화), Device_Management(펌웨어 OTA/롤백), Protocol_Factory(펌웨어 다운로드),
│  │                               Net_Ethernet / Net_CAN (사용자 프로토콜 연결)
│  └─ _WD_Third_Party/WIZnet_W6100 WIZnet io6Library 원본 + Port/ (README_WD.md)
├─ Boards/                         보드별 설정 3종 (Boards/README.md : 보드 목록, 새 보드 추가 방법)
│  └─ STM32H743XI_Ref/             기준 보드
├─ Template/Core/_System/          새 프로젝트 시작용 골격 (main, Thread, version) - 복사 후 프로젝트 소유
├─ _Doc/                           PORTING.md, LIBRARY_UPDATE.md, NAMING.md, COMPLIANCE.md, templates/
├─ _Tools/                         빌드 검증 스크립트
└─ LICENSE, LICENSES/, THIRD_PARTY_NOTICES.md, CHANGELOG.md
```

## 2. 빠른 시작

자세한 절차와 체크리스트는 [_Doc/PORTING.md](_Doc/PORTING.md).

**라이브러리 전체 사용**

1. CubeMX 로 프로젝트 생성 (사용할 주변장치의 HAL 핸들 생성, FreeRTOS CMSIS-RTOS2)
2. 이 저장소의 `WD_Library/` 를 프로젝트에 추가 (권장 : git subtree, [_Doc/LIBRARY_UPDATE.md](_Doc/LIBRARY_UPDATE.md))
3. 보드 설정 3종을 프로젝트로 복사 : 비슷한 보드가 있으면 `Boards/<보드>/`, 없으면 `WD_Library/_WD_Config_Template/`
   - `wd_config.h` : 쓸 모듈만 `RTE_xxx 1`
   - `wd_board.h` : HAL 핸들 매핑, 핀맵
   - `wd_memory_map.h` : Flash / NVM 배치 (링커 스크립트와 일치)
4. `Template/Core/_System/` 을 프로젝트 `Core/_System/` 으로 복사 (부팅 순서, RTOS 스레드, 주기 처리)
5. include 경로 추가, `USE_HAL_DRIVER STM32H743xx ARM_MATH_CM4` 정의, CMSIS-DSP 링크
6. 애플리케이션 코드는 `#include "wd.h"` 하나로 시작하고, weak hook(`App_Initialize()`, `App_Process()` 등)을 재정의해 연결 (3장)

```c
#include "_main.h"                       /* Template : wd.h 포함 */

void App_Initialize(void)                /* App 스레드 시작 시 1회 */
{
    WD_LOG_INFO("WD Library %s", WD_LIB_VERSION_STRING);
}

void App_Process(wd_reg_timer_t *timer)  /* 주기 호출 */
{
    if (timer->Tick_1000ms) { /* 1초 주기 처리 */ }
}
```

**모듈 하나만 사용** (예 : NOR Flash 드라이버만)

`WD_Library/_WD_Core/` + 설정 3종 + 4장 표의 모듈 폴더(+ 의존 모듈 폴더)만 복사하고 include 경로에 넣는다.
`wd_config.h` 에서 그 모듈만 켜고, 모듈 헤더(예 `_wd_nor.h`)를 include 해서 쓴다.

**라이브러리 업데이트** : subtree 로 받은 프로젝트는 `git subtree pull` 로 새 버전을 받는다.
버전 확인은 `WD_LIB_VERSION_CHECK()`, 바뀐 점은 CHANGELOG 의 `[Config]`(설정 파일 영향) 항목을 본다.

## 3. 레이어 규칙과 애플리케이션 연결

### 레이어 규칙

```
 Application (프로젝트)  <->  _System (프로젝트 : main / Thread)
            │  hooks (weak)  ▲
            ▼                │
 Service  →  Component  →  Drivers  →  Platform  →  _WD_System / wd_def.h
                                                         ▲
                     wd_config.h / wd_board.h / wd_memory_map.h  (프로젝트 제공)
```

- **코어(`_WD_Core`)는 코어 밖을 참조하지 않는다.** 코어만 단독 컴파일 가능 (외부 의존 : HAL 핸들 `hrtc`, `htim7` + HAL/RTOS/libc)
- 콘솔 출력·입력은 코어에 **등록**한다 : `wd_print_register_output()`, `wd_print_register_input()`
  (UART : `wd_print_hal_init_app_uart()` 이 UART1 등록, Telnet : `wd_eth_telnet_initialize()` 가 등록). 등록 전 출력은 버려진다.
- 라이브러리 소스는 `#include "wd.h"` 하나만 사용한다. (`wd.h`가 RTE_xxx 설정에 따라 필요한 헤더를 순서대로 포함)
- 라이브러리는 **애플리케이션을 직접 참조하지 않는다.** 애플리케이션 연결은 아래 weak hook / 등록 API 로만 한다.
- 프로젝트별 값(핀, HAL 핸들, IP, 메모리 레이아웃, 버전)은 `wd_config.h / wd_board.h / wd_memory_map.h` 에만 둔다.

### 연결 포인트 (hook / 등록 API)

| 위치 | API | 용도 |
|---|---|---|
| `wd_core.h` (core) | `wd_os_tick_50ms_hook()`, `wd_time_rtc_init_hook()`, `wd_time_rtc_second_hook()` (weak) | 코어 틱/RTC 이벤트 → 시스템 (기본 구현 : `_WD_System` 레지스터 반영) |
| `wd_core.h` (core) | `wd_core_status_hook(source, status)` (weak) | 컴포넌트 상태 보고 → 시스템 (`_WD_System` : `wd_reg_status` 반영) |
| `wd_core.h` (core) | `wd_core_scratch_buffer()`, `wd_core_scratch_size()` (weak) | 모듈 공용 작업 버퍼 (`_WD_System` : `wd_global.Shared`) |
| `_wd_eth.h` | `wd_eth_set_packet_dump(app, debug)` | TCP 패킷 덤프 on/off (`_WD_System` 이 ENV 값으로 적용) |
| `_wd_print.h` (core) | `wd_print_register_output(putc)`, `wd_print_register_input(getc)` | 콘솔 출력·입력 장치 등록 (UART, Telnet, RTT 등) |
| Template `_Thread.c` | `App_Initialize()`, `App_Process(timer)` (weak) | App 스레드 초기화/주기 처리 |
| `_wd_net.h` | `wd_net_register_user_protocol(&proto)` | Ethernet/CAN 공통 사용자 프로토콜 파서/초기화 등록 (`WD_PROTOCOL_TYPE_USER`) |
| `_wd_eth.h` | `wd_eth_user_net_info(net_info, &port_app, &port_dbg)` (weak) | IP/MAC/포트를 NVM 등에서 공급 (미구현 시 `RTE_NETWORK_xxx` 기본값) |
| `_wd_mmu.h` | `wd_mmu_attach(object, &bd_instance)` | MMU 메모리 객체(ROM / NVM_FLASH / NVM_SERIAL_EEPROM) ↔ 블록 디바이스 연결. 블록 디바이스 Initialize 직후 호출 (Template `_main.c`). 연결하지 않은 범위는 MMU 오류 |
| `_wd_mmu.h` | `wd_mmu_user_write_map(dst, &src, &num, job, &arg)` (weak) | RAM 상의 앱 데이터 → NVM 저장 위치 매핑 (`wd_mmu_write`) |
| `_wd_env.h` | `wd_env_user_default(env)`, `wd_env_print_user(env)` (weak), `wd_env->User[512]` | 앱 영구 설정 영역 |
| `_wd_reg.h` | `wd_reg_system->EVT_User[]`, `wd_reg_status->Status_User[]`, `wd_reg_system->EVT_Application_Ready` | 앱 이벤트/상태 |
| `wd_memory_map.h` | `MAP_NVM_FLASH.User_Area`, `MAP_ROM_USER_AREA_SIZE` | 앱 NVM 객체 배치 |

## 4. 모듈 단위 사용 (의존표)

**모듈 하나만 쓰려면 : `_WD_Core/` + 설정 3종 + 아래 표의 "폴더" + "의존 모듈" 폴더**를 복사하고 그 폴더들을 include 경로에 넣는다.
모듈 소스는 자기 헤더만 include 하며, 헤더는 `wd_core.h` + 직접 의존 헤더만 include 한다.
이 표는 `_Tools/wd_module_check.sh` 의 manifest 와 같다 (모듈별 단독 컴파일 + nm 숨은 의존 검사로 검증).

| 모듈 | 폴더 | 의존 모듈 | 필요한 HAL 핸들 | 비고 |
|---|---|---|---|---|
| **core** | `_WD_Core` | - | `hrtc`, `htim7` (`RTE_PLATFORM_TIME_RTC_HANDLE`, `RTE_PLATFORM_TIME_HANDLE`) | 필수 |
| softspi | `_WD_Drivers/_Driver_Soft_SPI` | - | - | GPIO 비트뱅 SPI |
| softi2c | `_WD_Drivers/_Driver_Soft_I2C` | - | - | GPIO 비트뱅 I2C |
| gpiofunc | `_WD_Drivers/_Driver_Gpio` | - | - | |
| blockdev | `_WD_Drivers/_Driver_BlockDevice` | - | - | 블록 디바이스 공통 |
| adc | `_WD_Component/_Component_ADC` | softspi | `hspi1 hspi2 hspi3 hspi5` (AD4134), `hspi1 hspi2 htim5` (ADS127L), `WD_BOARD_ERROR_HANDLER()` | AD4134, ADS127L (ADS127L + MCLK=AD9837 옵션이면 dds 필요) |
| dds | `_WD_Component/_Component_DDS` | softspi | `htim1` (MCLK PWM), `WD_BOARD_ERROR_HANDLER()` | AD9837 |
| digipot | `_WD_Component/_Component_DigiPOT` | softspi | - | AD5235 |
| ioexp | `_WD_Component/_Component_IO_Expander` | softi2c | - | MCP23017 (보드 미검증) |
| iflash | `_WD_Component/_Component_Flash_IN` | blockdev | - | STM32H7 내부 Flash |
| nvm | `_WD_Component/_Component_NVM_NOR` | blockdev | `hqspi` | MX66L2G45G NOR / W25N04 NAND |
| sdram | `_WD_Component/_Component_SDRAM` | - | `hsdram1` | IS42S32800G |
| thermo | `_WD_Component/_Component_Thermometer` | - | `hadc1` (NTC), `huart4` (DS18B20) | NTC / DS18B20 (DS18B20 보드 미검증) |
| fdcan | `_WD_Component/_Component_CAN` | - | `hfdcan1` | |
| eth | `_WD_Component/_Component_Ethernet` | w6100 | `hspi6` | W6100 + Telnet 콘솔 |
| vio | `_WD_Component/_Component_Virtual_IO` | - | - | 보드 IO (RTE_PORT_xxx) |
| w6100 | `_WD_Third_Party/WIZnet_W6100` | - | - | WIZnet io6Library 원본 + 포트 파일 (`README_WD.md`) |

프레임워크 패키지 (단독 사용 대상 아님, `_WD_Core` + `_WD_System` + 사용하는 Service 일괄) :
`_WD_Platform`(UART 콘솔 `huart1 huart5 huart8`), `_WD_Service/_MMU`, `_Device_Management`(Firmware), `_Net_CAN`, `_Net_Ethernet`,
`_Protocol_Factory`(`huart8`), `_WD_System`.

> 표의 핸들은 CubeMX 기본 이름이며 모듈 코드는 핸들 매크로(`RTE_<레이어>_<모듈>_<역할>_HANDLE`)만 사용한다.
> 핸들 이름이 다른 보드는 `wd_board.h` 에서 매크로를 재정의한다 (목록 : 템플릿 `wd_board.h` 의 "Component / service HAL handles").
> 같은 주변장치를 두 모듈이 쓰는 기본 조합 : `huart8` (Print_hal UART8 + Protocol_Factory), `hspi1/hspi2` (AD4134 / ADS127L 중 하나만 사용).

## 5. 빌드 환경

- STM32CubeIDE 1.18.x / GNU Tools for STM32 14.3, `-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard`
- 필수 define : `USE_HAL_DRIVER STM32H743xx ARM_MATH_CM4`
- CMSIS-DSP (`arm_math.h`, `libarm_cortexM4lf_math.a`), FreeRTOS + CMSIS-RTOS2
- 다른 STM32H7 MCU 는 `STM32H7xxxx` define, CubeMX 헤더, `wd_board.h` 의 메모리 범위·핀맵을 맞춘다

검증 스크립트 (참조 CubeMX 프로젝트의 HAL / CMSIS / FreeRTOS 헤더 사용, `ARM_GCC_BIN=<툴체인 bin>` 또는 PATH) :

| 스크립트 | 검사 |
|---|---|
| `_Tools/wd_build_check.sh -r <CubeMX 프로젝트> [-c <설정 폴더>] --strict --werror --headers --deps` | 전체 컴파일 경고 0, 헤더 단독·2회 include, 외부 심볼 목록 |
| `_Tools/wd_module_check.sh -r <CubeMX 프로젝트>` | 4장 모듈별 단독 컴파일 + 숨은 의존 (nm) |
| `_Tools/wd_config_matrix.sh -r <CubeMX 프로젝트> -j 8 --strict --werror` | RTE 스위치 조합별 빌드 (`!` 변형은 실패가 정상) |

잘못된 RTE 조합(예 : AD4134 → DDS 필요, Firmware → MMU + 내부 Flash 필요)은 `wd_config_check.h` 가 `#error` 로 알린다.

## 6. 문서

| 문서 | 내용 |
|---|---|
| [_Doc/PORTING.md](_Doc/PORTING.md) | 새 프로젝트 이식 절차 (CubeIDE 설정, 설정 파일, CubeMX USER CODE, 체크리스트) |
| [_Doc/LIBRARY_UPDATE.md](_Doc/LIBRARY_UPDATE.md) | git subtree 로 받기·업데이트·수정 역반영, 변경 시 검증 절차 |
| [Boards/README.md](Boards/README.md) | 보드 목록, 새 보드 추가 |
| [_Doc/NAMING.md](_Doc/NAMING.md), [_Doc/templates/](_Doc/templates/) | 명명·파일 규칙, 새 파일 템플릿 |
| [_Doc/COMPLIANCE.md](_Doc/COMPLIANCE.md) | 코딩 규칙 준수 현황, 편차, 알려진 제한 (보드 미검증 모듈 포함) |

## 7. 라이선스

- 이 라이브러리(DoubleD 작성 부분)는 MIT 라이선스다. 각 소스 파일 첫 줄의 `SPDX-License-Identifier` 가 그 파일의 라이선스다.
- 포함된 서드파티 코드와 원저작자 : [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md), 라이선스 원문 : `LICENSES/`

