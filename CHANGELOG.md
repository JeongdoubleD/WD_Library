# CHANGELOG

형식 : [Added] 추가 / [Changed] 변경 / [Fixed] 버그 수정 / [Removed] 제거 / [Config] 프로젝트 설정 파일 영향

## [2.0.0] - 2026-11-27

최초 공개 릴리스.

### Added
- 필수 코어 `_WD_Core` : 콘솔 출력·입력 등록, 로그, 시간(us/ms, RTC), GPIO, 링 버퍼, 순환 버퍼, RTOS 포트, weak hook
- 시스템 `_WD_System` : 레지스터 모델, Environment(1KB 영구 설정), Scheduler, Task Monitor, Fault Log
- 플랫폼 : UART 콘솔 백엔드, 시리얼 출력, FFT
- 드라이버 : Soft SPI, Soft I2C, GPIO, Block Device
- 컴포넌트 : AD4134 / ADS127L ADC, AD9837 DDS, AD5235 DigiPOT, MCP23017 IO Expander, MX66L2G45G NOR / W25N04 NAND,
  IS42S32800G SDRAM, STM32H7 내부 Flash, DS18B20 / NTC, FDCAN, W6100 Ethernet + Telnet 콘솔, Virtual IO
- 서비스 : MMU(메모리 추상화), Firmware(OTA/롤백), Protocol_Factory(펌웨어 다운로드), Net_Ethernet / Net_CAN
- 설정 템플릿 `wd_config.h` / `wd_board.h` / `wd_memory_map.h`, 보드 설정 `Boards/STM32H743XI_Ref`, 프로젝트 골격 `Template/Core/_System`
- 빌드 검증 도구 `_Tools/` : 전체·헤더 단독·모듈 단독 컴파일, RTE 설정 조합, 외부 심볼 검사
- 문서 : README(구조·의존표), `_Doc/PORTING.md`, `_Doc/LIBRARY_UPDATE.md`, `_Doc/NAMING.md`, `_Doc/COMPLIANCE.md`

### 알려진 제한
- 보드 시험이 끝나지 않은 모듈과 구형 보드 분기는 `_Doc/COMPLIANCE.md` 6장 참고
