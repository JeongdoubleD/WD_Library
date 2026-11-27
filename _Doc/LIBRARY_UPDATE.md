# WD Library 업데이트 / 역반영 절차

목표 : 여러 프로젝트가 같은 `WD_Library` 를 쓰면서
- 라이브러리 개선 → 각 프로젝트로 **가져오기(pull)**
- 프로젝트에서 고친 버그/개선 → 라이브러리로 **되돌려 반영(push back)**

을 반복할 수 있게 한다.

## 1. 원칙

1. **이 저장소가 원본** 이다. 프로젝트의 `WD_Library/` 는 사본이다.
2. 프로젝트에서 `WD_Library/` 내부 파일을 수정했다면 **반드시 이 저장소에 역반영**하고 버전을 올린다.
3. 프로젝트별 값은 라이브러리가 아니라 `Core/_Config/wd_config.h / wd_board.h / wd_memory_map.h` 에 둔다.
   - 라이브러리 헤더의 기본값은 모두 `#ifndef` 로 되어 있으므로 Config 에서 재정의한다.
4. 애플리케이션 기능을 라이브러리에 넣지 않는다. 필요하면 **hook / 등록 API** 를 라이브러리에 추가한다.
5. 버전 규칙 (`wd_version.h`, Semantic Versioning)
   - MAJOR : 설정 매크로/구조체/API 가 호환되지 않게 바뀜 (프로젝트 수정 필요)
   - MINOR : 모듈·기능·hook 추가 (기존 프로젝트 그대로 빌드됨)
   - PATCH : 버그 수정
6. 릴리스마다 `CHANGELOG.md` 기록 + git tag (`v2.0.0`, `v2.1.0` ...)

## 2. 방법 A : git subtree (권장)

프로젝트 저장소 안에 라이브러리 이력을 함께 보관한다. 서브모듈보다 다루기 쉽다.

공개 저장소의 `wd-library` 브랜치는 저장소의 `WD_Library/` 폴더만 담고 있다 (릴리스마다 갱신, 태그 `v<버전>`).

```bash
# (최초 1회) 라이브러리 저장소 등록 후 프로젝트의 WD_Library/ 로 가져오기
git remote add wdlib https://github.com/JeongdoubleD/WD_Library.git
git fetch wdlib wd-library
git subtree add --prefix=WD_Library wdlib wd-library --squash

# 라이브러리 업데이트 받기
git fetch wdlib wd-library
git subtree pull --prefix=WD_Library wdlib wd-library --squash
```

- 보드 설정은 `Boards/<보드>/` 또는 `WD_Library/_WD_Config_Template/` 을 프로젝트 `Core/_Config/` 로 복사해서 쓴다 (subtree 대상 아님)
- 프로젝트에서 라이브러리를 고쳤다면 이슈 / Pull Request 로 보내 준다 (`git subtree split --prefix=WD_Library` 로 분리한 커밋을 fork 에 push)

## 3. 방법 B : 수동 복사 (git 미사용 프로젝트)

1. 프로젝트의 `WD_Library/_WD_Core/wd_version.h` 로 현재 버전 확인
2. 새 버전의 `WD_Library/` 폴더로 통째 교체 (`_WD_Config_Template` 포함, 단 프로젝트 `Core/_Config` 는 건드리지 않음)
3. `CHANGELOG.md` 에서 두 버전 사이의 **[Config]** 항목 확인 → 프로젝트 `Core/_Config/*` 에 새 매크로 반영
   - 비교 : `WD_Library/_WD_Config_Template/wd_config.h` ↔ `Core/_Config/wd_config.h` (diff 도구)
4. 빌드 후 PORTING.md 체크리스트 확인

역반영 : 수정한 파일 목록을 기록 → 이 저장소에서 같은 파일을 수정/커밋 → PATCH 버전 증가.

## 4. 프로젝트에서 버전 고정/확인

```c
/* Core/_Config/wd_config.h 끝부분 등 */
#if (WD_LIB_VERSION < WD_LIB_VERSION_CHECK(1, 0, 0))
#error "WD Library 1.0.0 or later is required"
#endif
```

## 5. 라이브러리 변경 시 검증

이 저장소에는 하드웨어 프로젝트가 없으므로, 기준 프로젝트(STM32H743 CubeMX 프로젝트)의 HAL/FreeRTOS/CubeMX 헤더를
include 경로로 사용해 아래를 확인한다.

1. 전체 컴파일 + 헤더 단독 컴파일 : `_Tools/wd_build_check.sh -r <참조> --strict --werror --headers --deps` → 에러 0, 헤더 실패 0
2. 코어 단독 : `_Tools/wd_build_check.sh -r <참조> --core --headers --deps` → 외부 심볼은 `hrtc htim7` + HAL/libc 만
3. 모듈 단독 : `_Tools/wd_module_check.sh -r <참조>` → 16개 모듈 OK, 숨은 의존 0 (README 4장 표와 manifest 일치)
4. 설정 조합 : `_Tools/wd_config_matrix.sh -r <참조> -j 16 --strict --werror` → unexpected 0
   (`!` 표시 변형은 실패가 정상 : COMPLIANCE.md 알려진 결함 또는 `wd_config_check.h` 음성 시험)
5. 기준 프로젝트에 넣어 전체 링크 (ELF 생성) : RAM/Flash 사용량 변화 확인
6. 동작 변경이 있으면 CHANGELOG "동작 변경" 에 기록하고 보드 시험

