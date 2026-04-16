# Phase 17-R2 수행 보고서 (W0~W3)

작성일: `2026-04-13`  
기준 계획서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`  
실행 계획서: `docs/refactoring/phase17-root/r2/refactoring_phase17_r2_shell_ui_dismantle_execution_plan_260409.md`

## 1. 수행 범위

본 보고서는 R2의 W0~W3만 다룬다.

1. W0: 이관 대상/인벤토리 고정
2. W1: `font_manager.*` 이관
3. W2: `custom_ui.*`, `test_window.*` 이관
4. W3: `toolbar.*` 이관 및 singleton 직접 호출 제거

W4~W6는 본 보고서 범위에서 제외한다.

## 2. 코드 해체/이관 내역

## 2.1 파일 이동

루트 -> `shell/presentation` 이동 완료:
1. `webassembly/src/font_manager.cpp` -> `webassembly/src/shell/presentation/font/font_registry.cpp`
2. `webassembly/src/font_manager.h` -> `webassembly/src/shell/presentation/font/font_registry.h`
3. `webassembly/src/custom_ui.cpp` -> `webassembly/src/shell/presentation/widgets/icon_button.cpp`
4. `webassembly/src/custom_ui.h` -> `webassembly/src/shell/presentation/widgets/icon_button.h`
5. `webassembly/src/test_window.cpp` -> `webassembly/src/shell/presentation/debug/test_window_panel.cpp`
6. `webassembly/src/test_window.h` -> `webassembly/src/shell/presentation/debug/test_window_panel.h`
7. `webassembly/src/toolbar.cpp` -> `webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.cpp`
8. `webassembly/src/toolbar.h` -> `webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.h`

## 2.2 include 및 참조 경로 정리

변경 파일:
1. `webassembly/src/app.cpp`
2. `webassembly/src/mesh.cpp`
3. `webassembly/src/mesh_detail.cpp`
4. `webassembly/src/mesh_group_detail.cpp`
5. `webassembly/src/model_tree.cpp`
6. `webassembly/src/mesh/presentation/model_tree_mesh_section.cpp`
7. `webassembly/src/mesh/presentation/model_tree_structure_section.cpp`
8. `webassembly/src/vtk_viewer.cpp`
9. `webassembly/src/shell/runtime/workbench_runtime.cpp`

주요 내용:
- 기존 루트 헤더 include를 신규 `shell/presentation/*` 경로로 전환
- 하위 디렉터리 소스는 컴파일 경로를 고려해 상대 include로 재정렬
- `wb_ui` 소스 등록 경로를 신규 위치로 변경

## 2.3 `toolbar` singleton 직접 호출 제거

`webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.cpp`에서 다음 직접 호출 제거:
- `AtomsTemplate::Instance().HasChargeDensity()`
- `AtomsTemplate::Instance().IsChargeDensitySimpleViewActive()`
- `AtomsTemplate::Instance().chargeDensityUI()`
- `AtomsTemplate::Instance().IsBZPlotMode()`

치환 방식:
- `GetWorkbenchRuntime().AtomsTemplateFacade()` 경유 접근(`RuntimeAtomsTemplateFacade()` 헬퍼)

결과:
- `toolbar` 내부 `AtomsTemplate::Instance()` 호출 `0`

## 3. 빌드/게이트 검증

## 3.1 게이트

실행:
1. `check_phase17_root_allowlist.ps1`
2. `check_phase17_root_ownership_map.ps1`
3. `check_phase17_facade_contract.ps1`

결과:
- 전 항목 PASS
- `app.cpp` legacy include baseline: `8 -> 7` (증가 없음)

## 3.2 빌드

실행:
1. `cmd /c "..\\emsdk\\emsdk_env.bat && npm.cmd run build-wasm:release"`

결과:
- `PASS` (초기 include 오류 수정 후 재실행 성공)

## 4. W0~W3 완료 기준 판정

1. W0 인벤토리/대상 고정: `PASS`
2. W1 `font_manager.*` 루트 제거 + 이관: `PASS`
3. W2 `custom_ui.*`, `test_window.*` 루트 제거 + 이관: `PASS`
4. W3 `toolbar.*` 루트 제거 + singleton 직접 호출 제거: `PASS`

판정:
- `GO` (R2-W4 착수 가능)

## 5. 잔여 작업 (R2 다음 단계)

1. W4: `app.cpp` 직접 상태 조작 블록 축소
2. W5(선택): `image/texture`를 render resource 경계로 조기 이관
3. W6: R2 전용 게이트(`check_phase17_r2_shell_ui_migration.ps1`) 추가 및 종료 문서화
