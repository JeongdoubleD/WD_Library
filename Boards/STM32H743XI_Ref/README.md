# STM32H743XI_Ref

WD Library 개발·검증에 사용한 기준 보드 설정. `WD_Library/_WD_Config_Template/` 의 예시 값과 같다.

| 항목 | 값 |
|---|---|
| MCU | STM32H743XI (Cortex-M7, 2 MB Flash) |
| 툴체인 | STM32CubeIDE 1.18.x / GNU Tools for STM32 14.3, FreeRTOS + CMSIS-RTOS2, CMSIS-DSP |
| 외부 장치 | AD4134 ADC, AD9837 DDS x2, AD5235 DigiPOT, MX66L2G45G NOR (QSPI), IS42S32800G SDRAM (FMC), W6100 Ethernet (SPI6), FDCAN, NTC |
| 검증 | 빌드 : `--strict --werror` 경고 0, 설정 조합·모듈 단독 검사 통과 |

## CubeMX 에서 만들어야 하는 핸들

| 핸들 | 사용 모듈 |
|---|---|
| `hrtc`, `htim7` | core (RTC, us 타이머) |
| `huart1`, `huart5`, `huart8` | UART 콘솔 / Protocol_Factory (`huart8`) |
| `hspi1`, `hspi2`, `hspi3`, `hspi5` | AD4134 |
| `hspi6` | W6100 Ethernet |
| `hqspi` | NOR Flash |
| `hsdram1` | SDRAM |
| `hadc1` | NTC 온도 |
| `hfdcan1` | FDCAN |
| `Error_Handler()` | CubeMX 기본 오류 처리 |

핀맵은 이 보드의 배선 기준이다 (`wd_board.h` 의 `RTE_PORT_xxx`). 다른 보드에서는 반드시 실제 배선으로 바꾼다.
