# Phase 17-R2 세부 작업계획서 (실질 코드 해체/이관 중심)

작성일: `2026-04-09`  
기준 문서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`

## 0. 목적

R2의 목표인 `shell/UI 파일군 이관`을 실제 코드 해체 단위로 실행한다.

핵심 목표:
1. 루트 파일 `font_manager.*`, `custom_ui.*`, `test_window.*`, `toolbar.*`를 모듈 경계 내부(`shell/presentation`)로 이관
2. `app.cpp`의 직접 상태 조작을 축소하고 shell facade 위임 경로를 강화
3. R1에서 고정한 facade 계약(`check_phase17_facade_contract.ps1`) 기준으로 복잡도 역증가를 차단

비고:
- `atoms_template_bravais_lattice.cpp`, `atoms_template_periodic_table.cpp`는 실행 파일이 아닌 설명 자료로 분류되어 R2 범위에서 제외한다.

---

## 1. 현재 기준선 (R2 착수 시점)

### 1.1 대상 파일 규모

| 파일 | 라인 수 | 현 위치 |
|---|---:|---|
| `font_manager.cpp` | 33802 | `webassembly/src` 루트 |
| `font_manager.h` | 72 | `webassembly/src` 루트 |
| `custom_ui.cpp` | 11 | `webassembly/src` 루트 |
| `custom_ui.h` | 9 | `webassembly/src` 루트 |
| `test_window.cpp` | 104 | `webassembly/src` 루트 |
| `test_window.h` | 19 | `webassembly/src` 루트 |
| `toolbar.cpp` | 453 | `webassembly/src` 루트 |
| `toolbar.h` | 43 | `webassembly/src` 루트 |
| `image.cpp` | 27 | `webassembly/src` 루트(선택 이관) |
| `image.h` | 23 | `webassembly/src` 루트(선택 이관) |
| `texture.cpp` | 60 | `webassembly/src` 루트(선택 이관) |
| `texture.h` | 28 | `webassembly/src` 루트(선택 이관) |

### 1.2 현재 의존의 핵심 문제

1. `app.cpp`가 `font_manager.h`를 직접 include하여 shell facade 경계가 두꺼움
2. `toolbar.cpp`가 `AtomsTemplate::Instance()`를 직접 호출하여 feature 싱글턴 의존 잔존
3. `test_window.cpp`가 render gateway에 직접 접근하여 shell/debug UI와 render 경계 혼재
4. `wb_ui.cmake`가 루트 파일 경로를 직접 소스 등록하여 경로 정렬 전환 미완료

---

## 2. 파일 단위 해체/이관 매핑

## 2.1 필수 이관군 (R2 완료 기준에 반드시 포함)

| 원본 파일 | 목표 경로 | 이관 방식 | 완료 기준 |
|---|---|---|---|
| `webassembly/src/font_manager.h` | `webassembly/src/shell/presentation/font/font_registry.h` | 타입/인터페이스 이동 + include 경로 교체 | 루트 `font_manager.h` 삭제 |
| `webassembly/src/font_manager.cpp` | `webassembly/src/shell/presentation/font/font_registry.cpp` | 구현 이동, 런타임 접근 경로 유지 | 루트 `font_manager.cpp` 삭제 |
| `webassembly/src/custom_ui.h` | `webassembly/src/shell/presentation/widgets/icon_button.h` | `CustomUI::IconButton`를 위젯 유틸로 재배치 | 루트 `custom_ui.h` 삭제 |
| `webassembly/src/custom_ui.cpp` | `webassembly/src/shell/presentation/widgets/icon_button.cpp` | 구현 이동 | 루트 `custom_ui.cpp` 삭제 |
| `webassembly/src/test_window.h` | `webassembly/src/shell/presentation/debug/test_window_panel.h` | 디버그 패널 명확화 + 네이밍 정리 | 루트 `test_window.h` 삭제 |
| `webassembly/src/test_window.cpp` | `webassembly/src/shell/presentation/debug/test_window_panel.cpp` | 구현 이동 + 런타임 반환 타입 교체 | 루트 `test_window.cpp` 삭제 |
| `webassembly/src/toolbar.h` | `webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.h` | toolbar presenter 경계 고정 | 루트 `toolbar.h` 삭제 |
| `webassembly/src/toolbar.cpp` | `webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.cpp` | 구현 이동 + 직접 singleton 호출 제거 | 루트 `toolbar.cpp` 삭제 |

## 2.2 선택 이관군 (R2 내 수행 권장, 미완료 시 R3 선반영)

| 원본 파일 | 목표 경로 | 판단 기준 |
|---|---|---|
| `image.h/.cpp` | `render/infrastructure/resources/image_loader.h/.cpp` | toolbar 이관 중 texture 의존을 render 자원 경계로 보낼 수 있으면 R2에서 수행 |
| `texture.h/.cpp` | `render/infrastructure/resources/texture_resource.h/.cpp` | OpenGL 바인딩이 shell/presentation에 남지 않도록 조기 분리 |

---

## 3. 함수/책임 해체 계획

### 3.1 `toolbar.cpp` 책임 분해

현재 책임:
- 툴바 렌더링 + 버튼 핸들링 + charge density 애니메이션 제어 + atoms singleton 조회

R2 분해 단위:
1. `renderBoundaryAtomsQuickButton`, `renderMeshDisplayModeButtons`, `renderProjectionModeButtons`  
   -> `shell/presentation/toolbar/viewer_toolbar_presenter.cpp`
2. `renderChargeDensityControls`  
   -> `shell/presentation/toolbar/charge_density_toolbar_section.cpp`(신규)로 분리
3. `AtomsTemplate::Instance()` 직접 호출  
   -> `WorkbenchController` 또는 runtime wrapper 호출로 치환(직접 singleton 제거)

### 3.2 `app.cpp` 상태 조작 축소

우선 정리 대상:
1. `LoadImGuiIniFile/SaveImGuiIniFile` 내 window visibility 직접 필드 갱신 블록
2. `renderDockSpaceAndMenu` 내 layout preset별 대량 bool 직접 할당 블록

R2 적용 방식:
- 상태 변경은 `shell::application::ShellStateCommandService`를 통한 command 형태로 위임
- `app.cpp`는 메뉴 이벤트를 command로 전달하고, 직접 필드 다중 할당을 단계적으로 제거

### 3.3 `test_window.cpp` 경계 정리

현재:
- VTK 소스 생성 + render gateway 호출 + 메모리 테스트 로직 혼재

R2 목표:
- 파일 위치는 shell/presentation/debug로 이동
- render 접근은 최소한 runtime/render application entry로 우회(직접 gateway 호출 축소)
- 디버그 패널임을 명시하고 feature 본선 경로와 분리

---

## 4. 단계별 WBS (실행 순서 고정)

### W0. 이관 브랜치/인벤토리 고정

작업:
1. R2 대상 파일 목록 확정
2. `wb_ui.cmake` 현행 소스 목록 스냅샷 기록
3. `check_phase17_facade_contract.ps1` baseline 값(legacy include=8) 재확인

산출물:
- `docs/refactoring/phase17-root/r2/logs/r2_inventory_snapshot_latest.md`

### W1. FontManager 이관

작업:
1. `font_manager.*` -> `shell/presentation/font/*` 이동
2. include 경로 갱신(`app.cpp`, `mesh_detail.cpp`, `model_tree.cpp`, `mesh_group_detail.cpp`, `vtk_viewer.cpp`, `workbench_runtime.cpp` 등)
3. runtime의 `FontRegistry()` 반환 타입/헤더 경로 정렬

완료 기준:
- 루트 `font_manager.*` 제거
- 빌드 통과 + facade gate PASS

### W2. CustomUI + TestWindow 이관

작업:
1. `custom_ui.*` -> `shell/presentation/widgets/*` 이동
2. `test_window.*` -> `shell/presentation/debug/*` 이동
3. `workbench_runtime.cpp/.h`의 include/반환 경로 갱신

완료 기준:
- 루트 `custom_ui.*`, `test_window.*` 제거
- 관련 include가 신규 경로만 사용

### W3. Toolbar 이관(핵심)

작업:
1. `toolbar.*` -> `shell/presentation/toolbar/*` 이동
2. `AtomsTemplate::Instance()` 직접 호출을 controller/runtime 경유 호출로 치환
3. charge density 제어 섹션을 파일 분리해 toolbar presenter의 책임 축소

완료 기준:
- 루트 `toolbar.*` 제거
- `toolbar` 내부 singleton 직접 호출 0

### W4. `app.cpp` 상태 조작 축소

작업:
1. 메뉴 action -> shell command 위임 함수 정리
2. 반복 bool 직접 할당 블록을 command 기반 갱신으로 전환
3. `app.cpp`의 직접 상태 변경 코드량(라인/블록 수) 감소

완료 기준:
- `app.cpp`에서 다중 상태 직접 할당 블록이 baseline 대비 감소
- facade gate PASS 유지

### W5. (선택) Image/Texture 이관

작업:
1. `image.*`, `texture.*`를 `render/infrastructure/resources`로 이동
2. toolbar/기타 UI 경로에서 render resource adapter 경유 사용

완료 기준:
- 루트 `image.*`, `texture.*` 제거(수행 시)

### W6. CMake/게이트 정렬 및 문서화

작업:
1. `webassembly/cmake/modules/wb_ui.cmake` 경로 업데이트
2. 신규 게이트 스크립트 도입:
   - `scripts/refactoring/check_phase17_r2_shell_ui_migration.ps1`
3. R2 결과보고서/게이트 보고서/GO-NO-GO 작성

완료 기준:
- R2 게이트 PASS
- R3 착수 문서 완료

---

## 5. 코드 변경 파일 목록(예상)

필수 변경:
1. `webassembly/src/shell/runtime/workbench_runtime.h`
2. `webassembly/src/shell/runtime/workbench_runtime.cpp`
3. `webassembly/src/app.cpp`
4. `webassembly/cmake/modules/wb_ui.cmake`

신규(예상):
1. `webassembly/src/shell/presentation/font/*`
2. `webassembly/src/shell/presentation/widgets/*`
3. `webassembly/src/shell/presentation/debug/*`
4. `webassembly/src/shell/presentation/toolbar/*`

삭제(목표):
1. `webassembly/src/font_manager.*`
2. `webassembly/src/custom_ui.*`
3. `webassembly/src/test_window.*`
4. `webassembly/src/toolbar.*`
5. (선택) `webassembly/src/image.*`, `webassembly/src/texture.*`

---

## 6. 검증 계획

## 6.1 자동 검증

1. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_facade_contract.ps1`
2. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r2_shell_ui_migration.ps1`(신규)
3. `npm run build-wasm:release`
4. `npm run test:cpp`
5. `npm run test:smoke`

## 6.2 수동 회귀 포인트

1. 상단 메뉴 동작(`Edit/Build/Data/Utilities`)
2. Viewer toolbar 버튼 동작(투영/표시/리셋/boundary atoms)
3. Charge density 단순 뷰 재생/중지/슬라이더
4. Model Tree, Mesh Detail, AtomsTemplate 관련 창 표시/포커스

---

## 7. 리스크 및 대응

| 리스크 | 수준 | 대응 |
|---|---|---|
| `font_manager.cpp` 초대형 파일 이동 충돌 | High | W1 단독 커밋 분리 + include 경로 일괄 치환 스크립트 사용 |
| toolbar 이관 중 atoms singleton 재유입 | High | W3에서 singleton 호출 0 게이트 강제 |
| `app.cpp` 상태 동기화 회귀 | High | 메뉴 회귀 시나리오 + shell state diff 로그 비교 |
| image/texture 조기 이동 시 렌더 경계 충돌 | Medium | W5를 선택 작업으로 분리, 실패 시 R3로 이월 |

---

## 8. R2 완료 기준(실행판)

1. 루트 `font_manager.*`, `custom_ui.*`, `test_window.*`, `toolbar.*` 파일이 0개다.
2. `app.cpp`는 shell facade 이벤트 위임 중심으로 유지되고 직접 상태 조작이 baseline 대비 감소한다.
3. `check_phase17_facade_contract.ps1` PASS를 유지한다.
4. R2 신규 게이트(`check_phase17_r2_shell_ui_migration.ps1`)가 PASS한다.
5. 선택 이관군(`image/texture`) 수행 여부와 잔여 작업이 문서에 명시된다.
