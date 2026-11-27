# 새 프로젝트에 WD Library 이식하기

대상 : STM32CubeIDE + CubeMX 로 생성한 STM32H7 프로젝트 (FreeRTOS / CMSIS-RTOS2 사용)

## 1. 파일 배치

```
<Project>/
├─ Core/Inc, Core/Src          CubeMX 생성
├─ Core/_Config/               ← WD_Library/_WD_Config_Template/* 복사 후 수정
│    wd_config.h  wd_board.h  wd_memory_map.h
├─ Core/_System/               ← Template/Core/_System/* 복사 (main / Thread / version)
├─ Core/_Application/          프로젝트 애플리케이션 (App_Initialize / App_Process ...)
└─ WD_Library/                 ← 라이브러리 (git subtree 권장, 프로젝트에서 직접 수정 금지)
```

> `WD_Library/_WD_Config_Template` 폴더는 **빌드 include 경로에서 제외**한다.
> (프로젝트의 `Core/_Config` 가 같은 이름의 헤더를 제공해야 함)

## 2. CubeIDE 설정

1. **Include 경로** (C/C++ Build → Settings → MCU GCC Compiler → Include paths)
   - `Core/_Config`, `Core/_System`, `Core/_Application`
   - `WD_Library/_WD_Core` (필수), `WD_Library/_WD_Include`, `WD_Library/_WD_System`
   - `WD_Library/_WD_Platform`, `WD_Library/_WD_Drivers/*`, `WD_Library/_WD_Component/*`, `WD_Library/_WD_Service/*` 각 하위 폴더
   - 모듈 일부만 쓸 때는 README 4장 의존표의 폴더만 추가
   - W6100 사용 시 `WD_Library/_WD_Third_Party/WIZnet_W6100/` 의 `Ethernet`, `Ethernet/W6100`, `Application`, `Port`,
     `Internet/DHCP4`, `Internet/DHCP6`, `Internet/DNS` (WIZnet io6Library 원본 구조, `README_WD.md` 참고)
   - `Drivers/CMSIS/DSP/Include`
2. **소스 제외** : `WD_Library/_WD_Config_Template` (Resource Configurations → Exclude from Build)
3. **Define** : `USE_HAL_DRIVER`, `STM32H743xx`(MCU에 맞게), `ARM_MATH_CM4`
4. **Linker** : `arm_cortexM4lf_math` 라이브러리, 검색 경로 `Drivers/CMSIS/DSP/Lib/GCC`
5. 사용하지 않는 모듈 폴더는 RTE_xxx = 0 으로 끄면 되므로 빌드에서 빼지 않아도 된다(전체 `#if` 처리됨).

## 3. 설정 파일 수정 순서

1. **wd_config.h**
   - `RTE_CONFIG_PLATFORM_NAME`, `RTE_CONFIG_FIRMWARE_VERSION`, `RTE_CONFIG_HARDWARE_VERSION`
   - 사용할 모듈만 `RTE_xxx = 1`
   - 모듈 간 필수 조합은 `WD_Library/_WD_Include/wd_config_check.h` 가 검사한다 (잘못된 조합이면 바꿀 스위치를 `#error` 로 알려줌)
   - `RTE_SERVICE_FIRMWARE = 0` 이어도 NVM 의 Environment 위치는 같다 (펌웨어 섹터는 예약으로 남음)
2. **wd_board.h**
   - CubeMX 헤더 include 목록 (프로젝트에 생성된 것만)
   - HAL 핸들 매핑 : `RTE_PLATFORM_TIME_HANDLE`, `RTE_PLATFORM_PRINT_SERIAL_DRIVER_HANDLE` 등
   - 핀맵 `RTE_PORT_xxx` (Virtual IO / Soft SPI / W6100 CS 등)
   - `WD_BOARD_MEMORY_RANGES` (MCU 메모리 범위)
   - 모듈별 핸들 기본값 재정의 (예: `#define RTE_COMPONENT_INTERFACE_ETHERNET_DRIVER_HANDLE (&hspi4)`).
     라이브러리는 HAL 핸들 이름(`hspi1`, `hqspi` …)을 직접 쓰지 않는다. 전체 목록과 기본값은 템플릿 `wd_board.h` 의
     "Component / service HAL handles" 절, 사용 모듈별 핸들은 README 4장 표
   - 컴포넌트가 HAL 초기화 실패 시 부르는 `WD_BOARD_ERROR_HANDLER()` (기본 : CubeMX `Error_Handler()`)
   - `adc.h` 는 `RTE_COMPONENT_THERMOMETER_NTC` 또는 `RTE_COMPONENT_ADC_STM32H743XI` 일 때 `wd_board.h` 가 include
3. **wd_memory_map.h**
   - Flash 크기/부트로더/앱 영역 (`MEMORY_OBJECT_xxx`) 을 링커 스크립트와 일치시킨다
   - 앱 NVM 데이터는 `MAP_NVM_FLASH.User_Area` 를 앱 구조체로 바꿔서 배치 (전체 크기 유지)
   - `WD_CONF_MMU_NVM_CRC_OFFSET` : NVM 객체의 wd_toolkit_crc16 저장 위치 (객체 주소 + 오프셋, 기본 4096). 저장 데이터 형식이므로 바꾸면 기존 NVM 데이터와 호환되지 않음
   - MMU 를 쓰면 블록 디바이스 Initialize 직후 `wd_mmu_attach()` 로 메모리 객체에 연결 (Template `_main.c` 의 `Component_Initialization()` 참고)

## 4. CubeMX 생성 코드 수정 (USER CODE 영역)

| 파일 | 수정 |
|---|---|
| `Core/Src/main.c` | CubeMX `main()` 은 사용하지 않음 (Template `_main.c` 의 `main()` 사용). CubeMX 프로젝트 설정에서 main 생성 안 함, 또는 `main` 이름 변경 |
| `Core/Src/freertos.c` | USER CODE include : `#include "_main.h"`. CubeMX 에서 생성하는 태스크가 Template `_Thread.c` 와 겹치지 않게 정리 |
| `Core/Src/stm32h7xx_it.c` | USER CODE include : `#include "_main.h"` |
| 링커 스크립트 | `.RAM_D2` 섹션, `USER_DATA`/`FACTORY_IMAGE` 영역이 `wd_memory_map.h` 와 일치하는지 확인 |

## 5. 애플리케이션 연결

```c
/* Core/_Application/App.c */
#include "_main.h"

static int32_t App_Proto_Init  (wd_network_port_t *port, uint32_t arg) { return WD_CODE_OK; }
static int32_t App_Proto_Parser(wd_network_port_t *port, uint32_t head, uint32_t tail, uint32_t arg)
{
  /* frame 해석 후 소비한 byte 수 반환, 해당 없으면 0 */
  return 0;
}
static const wd_net_user_protocol_t App_Protocol = { App_Proto_Init, App_Proto_Parser };

void App_Initialize(void)            /* Template _Thread.c 의 weak 함수 재정의 */
{
  wd_net_register_user_protocol(&App_Protocol);   /* Net_xxx_Open() 이전(App 스레드)에 등록 */
}

void App_Process(wd_reg_timer_t *timer)
{
  if(timer->Tick_1000ms) { /* 1초 주기 처리 */ }
}
```

## 6. 확인 체크리스트

- [ ] 빌드 경고/에러 확인 (라이브러리 코드 경고 0, 에러 0)
- [ ] 부팅 로고에 `WD Library x.y.z` 출력 확인
- [ ] NVM 사용 시 `wd_memory_map.h` 레이아웃 변경 후 첫 부팅에서 Environment 기본값 재초기화(정상)

