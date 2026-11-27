# WD Library 코딩 표준 준수 계획

| 항목 | 내용 |
|---|---|
| 적용 표준 | 범용 임베디드 C 코딩 표준 **SW-STD-C-001 v1.0** (MISRA C:2025 원칙 참고 사내 표준) |
| 대상 | `WD_Library/` 전체 (단 `_WD_Third_Party` 제외), `Template/Core/_System` |
| 문서 성격 | 표준 7.2 "프로젝트 준수 계획". 서명·승인란 대신 기록 형식만 따른다. 표준 문서 자체는 공개하지 않으며, 규칙 ID(GEN-x-nnn 등)는 추적용으로 소스 주석에 남긴다 |
| 주의 | 본 라이브러리는 MISRA 적합성을 주장하지 않는다. MISRA 정적분석 도구 검증은 수행되지 않았다 |

## 1. 언어 프로파일·도구 체인 (GEN-M-001)

| 항목 | 값 |
|---|---|
| 컴파일러 | GNU Tools for STM32 14.3 (arm-none-eabi-gcc 14.3.1) |
| 언어 | `-std=gnu11` (현행). 목표 : C11 + 승인된 확장만 `wd_compiler.h` 에 격리 (DEV-WD-001) |
| 타깃 | Cortex-M7, `-mthumb -mfpu=fpv5-d16 -mfloat-abi=hard`, little-endian, ILP32 |
| 필수 define | `USE_HAL_DRIVER`, `STM32H7xxxx`, `ARM_MATH_CM4` |
| 기준 SDK | STM32CubeH7 HAL 1.x, CMSIS-Core 5.x, FreeRTOS 10.x + CMSIS-RTOS2 (프로젝트에서 버전 고정) |

## 2. 경고 정책 (GEN-M-002)

목표 옵션 (라이브러리 코드) :

```
-Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Werror
```

| 항목 | 결과 |
|---|---|
| 라이브러리 코드 경고 (목표 옵션 + `-Werror`) | **0** (지원 구성 24종, SDK·서드파티 헤더는 `-isystem`) |
| 추가 점검 `-O2 -Wextra -Wswitch-default -Wimplicit-fallthrough -Wmaybe-uninitialized` | 0 |
| 호출 그래프 재귀 (`-fcallgraph-info`) | 0 |
| WIZnet 원본 소스 (서드파티, `-Wall` 만 적용, 4장) | 경고 6 (원본 유지) |

## 3. 규칙별 적용 계획

| 규칙 | 적용 | 검증 |
|---|---|---|
| GEN-M-001 언어 프로파일 | 적용 (DEV-WD-001) | 빌드 옵션 문서 |
| GEN-M-002 경고 0 | 적용 (목표 옵션 + `-Werror`, 지원 구성 24종) | `wd_build_check.sh --strict --werror`, `wd_config_matrix.sh --strict --werror` |
| GEN-M-003 미정의 동작 | 적용 (`_X`/`__x` 0, 파일 범위 `_소문자` 0, 예외 DEV-WD-005) | 스캔 스크립트, nm, 컴파일러 경고, 리뷰 |
| GEN-M-004 고정폭 정수 | **미완료** : plain `int` 다수 (printf `%d`, 콜백 시그니처와 연쇄). 문자 데이터의 `char` 는 허용 | 스크립트 검사, 리뷰 |
| GEN-M-005~007 변환·리터럴·시프트 | 적용 | `-Wconversion` 계열 |
| GEN-M-008 부작용 | 적용 | 리뷰 |
| GEN-M-009 불리언 조건 | 적용 | 리뷰 (도구 미보유) |
| GEN-M-010 중괄호 | 적용 | 포매터 |
| GEN-M-011 switch default/break | 적용 (fallthrough 0) | `-Wswitch-default`, `-Wimplicit-fallthrough` |
| GEN-M-012 원형 일치 | 적용 | `-Wstrict/missing-prototypes` |
| GEN-M-013 반환값 처리 | 적용 | 리뷰 |
| GEN-M-014~016 포인터 | 적용 | 리뷰 (`uintptr_t` 사용) |
| GEN-M-017 초기화 | 적용 (-O2 분석 0건) | `-Wmaybe-uninitialized` (-O2) |
| GEN-M-018 동적 할당 | 적용 (링크되는 코드에 malloc/free 없음) | 심볼 스캔(nm) |
| GEN-M-019 재귀 금지 | 적용 (함수 포인터 호출은 정적 그래프에 나타나지 않음) | 호출 그래프 (`-fcallgraph-info`) |
| GEN-M-020 매크로 제한 | 적용 (DEV-WD-002, DEV-WD-003) | 리뷰 |
| GEN-M-021 헤더 다중 포함 | 적용 (모든 헤더 단독·2회 include 통과. 우산 헤더 include 는 가드 앞에 둠) | `wd_build_check.sh --headers` |
| GEN-M-022~023 volatile·ISR 공유 | 적용 | 리뷰 |
| GEN-M-024 라이브러리 함수 | 적용 (strcpy·sprintf·atoi/atof 미사용 : 경계 복사, snprintf, strtol/strtod) | 금지 API 스캔 |
| GEN-S-001 명명 | 적용 → [NAMING.md](NAMING.md). `.c` 내부 static 이름은 점진 적용 | 스크립트 |
| GEN-S-002 파일=모듈 | 적용 (선행 `_` 방식) | 리뷰 |
| GEN-S-003 형식 | 적용 → `.clang-format` (4칸, 탭 금지, 100자) | clang-format `--dry-run` (차이 0) |
| GEN-S-004 함수 크기 | 권장 | 리뷰 |
| GEN-S-005 함수 계약 | 적용 (공개 API) | 리뷰 |
| GEN-S-006 단위 접미사 | 적용 | 리뷰 |
| GEN-S-007 매직 넘버 | 적용 | 리뷰 |
| GEN-S-008 단일 오류 체계 | 적용 : `wd_code_t` 단일 체계 | 리뷰 |
| GEN-S-009 단일 종료점 | 권장 | 리뷰 |
| GEN-S-010 생성 코드 분리 | 적용 : CubeMX 코드는 라이브러리에 포함하지 않음 | - |
| GEN-S-011 하드웨어 격리 | 적용 : HAL 핸들은 `wd_board.h` 매크로로만 (라이브러리 코드에 핸들 이름 직접 사용 0, 기본값은 모듈 헤더 `#ifndef`) | nm 의존 검사, `grep` |
| GEN-S-012 컴파일 시간 검증 | 적용 (`_Static_assert` 레지스터/Environment 크기 등) | 빌드 |
| STM-001~010 | 적용 (STM-002 HAL 상태 → `wd_code_t` 변환 포함) | 리뷰 |
| CMS-001~010 | 적용 (CMS-002 임계구역 래퍼 `wd_os_irq_lock/unlock`) | 리뷰 |

## 4. 적용 제외 (표준 9장)

| 대상 | 사유 | 관리 |
|---|---|---|
| `_WD_Third_Party/WIZnet_W6100` | 외부 오픈소스 (WIZnet io6Library, MIT, commit 8ac15e2) | 원본 유지·라이선스 유지 (`LICENSE`). 원본 수정 2건(WD-PATCH-1/2)과 포트 파일은 `README_WD.md`, `wd_patches.diff` 에 기록. 원본 경고(미사용 변수, dhcpv6 `memset(sizeof 포인터)`)는 수정하지 않음 |
| STM32 HAL / CMSIS / FreeRTOS | 제조사·외부 SDK (저장소에 미포함) | 프로젝트에서 버전 고정 |
| CubeMX 생성 코드 | 자동 생성 (저장소에 미포함) | USER CODE 영역만 수정 (STM-006) |

## 5. 편차(예외) 목록

| ID | 규칙 | 대상 | 사유 | 완화 |
|---|---|---|---|---|
| DEV-WD-001 | GEN-M-001 | `__attribute__((section/packed/aligned/weak))`, statement-expression | 메모리 배치·weak hook 에 필수 | `wd_compiler.h` 한 곳에 격리, 다른 파일 직접 사용 금지 |
| DEV-WD-002 | GEN-M-020 | `WD_LOG_xxx`, `WD_PRINT_xxx` 가변 인자 매크로 | 호출 위치(`__FILE__`, `__LINE__`)와 컴파일 시 레벨 제거가 필요 | 인수는 단일 평가, 결과는 `do { } while (0)` 형태 |
| DEV-WD-003 | GEN-M-020 | 드라이버 정의용 보조 매크로 (`LOCAL_RES`, `LOCAL_DRV` 등) | CMSIS-Driver 형식의 반복 정의 축약 | 해당 .c 파일 안에서만 정의·사용 |
| DEV-WD-004 | GEN-M-014 | 메모리 맵 주소 ↔ 포인터 (`MAP`, MMU 객체 주소) | NVM/Flash 주소 기반 저장 구조 | `uintptr_t` 사용, MMU·메모리맵 모듈에 한정 |
| DEV-WD-005 | GEN-M-003 | `_wd_print.c` 의 `__read` / `__write` | 툴체인(IAR / newlib) 저수준 입출력 훅 이름이라 바꿀 수 없음 | 해당 두 함수에 한정 |
| DEV-WD-007 | GEN-M-002 | `_wd_eth.c` 의 WIZnet 레지스터 매크로 `setSn_CR()` 호출 1곳 | 매크로 안의 정수 연산이 부호 변환 경고를 냄 (서드파티 매크로, 수정 불가) | 해당 줄만 `#pragma GCC diagnostic` push/ignored/pop |

편차 추가 시 이 표에 ID, 대상(파일/함수), 사유, 완화, 적용 버전을 기록한다.

## 6. 알려진 결함

| 위치 | 내용 | 관련 규칙 |
|---|---|---|
| `wd_board.h` (템플릿), `_wd_dds.h`, `_wd_ads127l.c` | `RTE_COMPONENT_ADC_ADS127=1` 또는 `RTE_COMPONENT_DDS=1` + `RTE_COMPONENT_ADC_AD4134=0` 은 구형 보드 분기를 선택 → 구형 보드 CubeMX 프로젝트(`htim1`, `TIM5`, 핀)가 있어야 컴파일 가능. 참조 프로젝트가 없어 **미검증** (기본 구성에서 선택되지 않음) | 이식성 |
| `_wd_mmu.c` `wd_mmu_validation()` | 시험 코드(`API_VALIDATION`, 현재 0 이라 컴파일 안 됨)가 NOR 상수(`RTE_NVM_NOR_FLASH_*`)와 `wd_global.Shared.API_Validation` 를 직접 사용. 활성화 시 연결된 블록 디바이스의 Capabilities 기준으로 수정 필요 | 이식성 |
| `MAP_ROM` (`wd_memory_map.h`) | `RTE_SERVICE_FIRMWARE=0` 이면 내부 Flash 대체 저장소의 Environment 위치가 앞당겨짐 (설정이 다른 빌드 간 데이터 비호환). NVM 맵은 구성과 무관하게 같은 위치 | - |
| 전체 | plain `int` 다수 (GEN-M-004) | GEN-M-004 |

## 7. 검증 방법과 한계

- 자동 : `_Tools/wd_build_check.sh` — 전체 컴파일(경고 수 집계), 외부 심볼 검사(nm), 선택적 전체 링크
- 자동 : `--headers` 헤더 단독 컴파일, `_Tools/wd_module_check.sh` 모듈 단독 컴파일 + 숨은 의존, `_Tools/wd_config_matrix.sh` RTE 설정 조합 27종 (지원 24 + 실패 예상 3)
- 자동 : clang-format 검사, 금지 API/예약 식별자 스캔
- 자동 : `--strict --werror` (전체·코어·설정 조합), `-O2` 추가 분석, `-fcallgraph-info` 재귀 검사
- 수동 : 리뷰 (불리언 조건, 포인터, 동시성 등 도구로 판정 불가한 항목)
- **미수행** : MISRA 정적분석 도구, 타깃 보드 동적 시험 — 사용자 하드웨어에서 별도 수행 필요
