# Phase 17-R2 실행 보고서 (W4~W5)

작성일: `2026-04-13`  
기준 계획서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`  
실행 계획서: `docs/refactoring/phase17-root/r2/refactoring_phase17_r2_shell_ui_dismantle_execution_plan_260409.md`

## 1. 실행 범위

본 보고서는 R2의 W4~W5 작업만 다룬다.

1. W4: `app.cpp`의 직접 상태 조작 축소 및 shell command 위임 강화
2. W5(선택): `image.*`, `texture.*`의 render resource 경계 이관

## 2. W4 수행 내역 (`app.cpp`)

대상 파일:
1. `webassembly/src/app.cpp`

적용 내용:
1. `renderDockSpaceAndMenu()` 내부에 `applyLayoutVisibilityPreset` 람다를 도입했다.
2. 레이아웃 프리셋(`DefaultFloating`, `DockRight`, `DockBottom`, `ResetDocking`) 처리에서 반복적인 bool 직접 대입을 제거하고, `ShellStateCommandService` 기반의 `SetWindowVisible`, `RequestFocus` 호출로 통일했다.
3. 메뉴 패널 오픈 람다(`openEditorPanel`, `openBuilderPanel`, `openDataPanel`)는 controller 액션 호출 후 `syncShellStateFromStore()`로 상태 반영하도록 정리했다.

의도:
1. `app.cpp`에서 shell 상태를 직접 조작하는 분산 코드를 줄이고 command/service 경로를 표준화한다.
2. facade 계약(R1)과 충돌하지 않으면서 R2 목표(루트 레거시 해체 준비)를 진행한다.

## 3. W5 수행 내역 (Image/Texture 이관)

### 3.1 파일 이관

1. `webassembly/src/image.h` -> `webassembly/src/render/infrastructure/resources/image_loader.h`
2. `webassembly/src/image.cpp` -> `webassembly/src/render/infrastructure/resources/image_loader.cpp`
3. `webassembly/src/texture.h` -> `webassembly/src/render/infrastructure/resources/texture_resource.h`
4. `webassembly/src/texture.cpp` -> `webassembly/src/render/infrastructure/resources/texture_resource.cpp`

### 3.2 참조 경로 정리

1. `viewer_toolbar_presenter.h`가 신규 resource 경로를 참조하도록 include를 변경했다.
2. 이관된 구현 파일의 self-include를 신규 파일명(`image_loader.h`, `texture_resource.h`)으로 정리했다.
3. 중첩 디렉터리에서 필요한 공용 헤더가 해석되도록 다음 경로를 보정했다.
4. `../../../macro/ptr_macro.h`
5. `../../../config/log_config.h`

### 3.3 CMake 소스 목록 정리

대상 파일:
1. `webassembly/cmake/modules/wb_ui.cmake`

변경 내용:
1. `webassembly/src/image.cpp` -> `webassembly/src/render/infrastructure/resources/image_loader.cpp`
2. `webassembly/src/texture.cpp` -> `webassembly/src/render/infrastructure/resources/texture_resource.cpp`

## 4. 검증 결과

## 4.1 게이트 스크립트

실행:
1. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_allowlist.ps1`
2. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_ownership_map.ps1`
3. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_facade_contract.ps1`

결과:
1. 전 항목 PASS
2. `app.cpp legacy include`: `7` (`<= 8` 기준 유지)
3. root snapshot: `src=28`, `atoms=2`

로그:
1. `docs/refactoring/phase17-root/r2/logs/check_phase17_root_allowlist_r2_w4_w5_latest.txt`
2. `docs/refactoring/phase17-root/r2/logs/check_phase17_root_ownership_map_r2_w4_w5_latest.txt`
3. `docs/refactoring/phase17-root/r2/logs/check_phase17_facade_contract_r2_w4_w5_latest.txt`

## 4.2 빌드

실행:
1. `cmd /c ""..\emsdk\emsdk_env.bat" && npm.cmd run build-wasm:release"`

결과:
1. PASS (`VTK-Workbench.js/.wasm/.data` install 완료)

## 5. 완료 기준 판정 (W4~W5)

1. W4 `app.cpp` 직접 상태 대입 축소: 완료
2. W5 `image.*`, `texture.*` 루트 제거 및 render resource 경계 이관: 완료
3. facade/root 게이트 PASS 유지: 완료
4. 릴리즈 빌드 성공: 완료

판정: `GO (R2-W6 진행 가능)`
