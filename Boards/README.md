# Boards

보드별 WD Library 설정 모음. 라이브러리 코드(`WD_Library/`)는 모든 보드가 같이 쓰고, 보드마다 다른 것은
아래 설정 파일 3종뿐이다.

| 파일 | 내용 |
|---|---|
| `wd_config.h` | 사용할 모듈 선택(`RTE_xxx`), 플랫폼 이름, 콘솔 설정 |
| `wd_board.h` | MCU / CubeMX 헤더, HAL 핸들 매핑(`RTE_..._HANDLE`), 핀맵(`RTE_PORT_xxx`) |
| `wd_memory_map.h` | 내부 Flash / 외부 NVM 레이아웃 |

| 보드 | MCU | 설명 |
|---|---|---|
| [STM32H743XI_Ref](STM32H743XI_Ref/) | STM32H743XI | 라이브러리 개발에 사용한 기준 보드 (모든 모듈 사용) |

## 사용

프로젝트의 include 경로에 사용할 보드 폴더를 넣거나, 폴더 안의 파일 3개를 프로젝트(예: `Core/_Config/`)로 복사해서 쓴다.
보드 폴더는 라이브러리와 함께 업데이트되므로 직접 수정하려면 복사해서 쓴다.

## 새 보드 추가

1. `WD_Library/_WD_Config_Template/` 을 `Boards/<보드명>/` 으로 복사 (보드명 : `<MCU>_<보드>`, 예 `STM32H743ZI_Nucleo`)
2. `wd_config.h` : 보드에 없는 모듈은 `RTE_xxx 0`
3. `wd_board.h` : CubeMX 핸들 이름이 다르면 `RTE_..._HANDLE` 재정의, 핀맵을 실제 배선으로 변경
4. `wd_memory_map.h` : Flash / NVM 크기와 배치
5. 검증 : `_Tools/wd_build_check.sh -r <보드 CubeMX 프로젝트> -c Boards/<보드명> --strict --werror --headers`,
   `_Tools/wd_module_check.sh -r <...> -c Boards/<보드명>`
6. 이 파일의 보드 표에 추가
