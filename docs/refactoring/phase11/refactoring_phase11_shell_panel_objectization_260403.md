# Phase 11 세부 작업계획서: Shell/Panel 객체화 + `App` 축소

작성일: `2026-04-03 (KST)`  
기준 계획: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (Phase 11 절)  
선행 판정: **GO** (`docs/refactoring/phase10/go_no_go_phase11.md`)  
대상 범위: `webassembly/src/app.*`, `webassembly/src/toolbar.*`, `webassembly/src/model_tree.*`, `webassembly/src/mesh_detail.*`, `webassembly/src/mesh_group_detail.*`, `webassembly/src/test_window.*`, `webassembly/src/shell/*`, `scripts/refactoring/*`, `docs/refactoring/phase11/*`  
진행 상태: `W0~W6 예정`

## 0. 착수 배경과 고정 전제

### 0.1 착수 배경
1. Phase 10에서 workspace/mesh/io 분리 기반이 고정되었고 W6 게이트가 PASS로 종료됐다.
2. Phase 11의 핵심 목표는 Shell/Panel singleton을 plain object/presenter로 전환해 `App`를 frame orchestration 중심으로 축소하는 것이다.
3. Phase 10 이관 리스크(`P11-R1~R3`)를 W0~W6 전 과정의 추적 항목으로 유지한다.

### 0.2 Phase 10 결과 반영 항목
1. `workspace`/`mesh`/`io` 계약은 유지하고, shell/panel 계층이 해당 service/query만 사용하도록 확장한다.
2. `P9-BUG-01`은 `Deferred` 상태로 유지되며 Phase 11 smoke/manual 체크리스트에 포함한다.
3. Phase 9 보호 규칙은 계속 고정한다.
   - `render` 외부 `VtkViewer::Instance()` 호출 금지
   - `PrimeLegacySingletons()`에 `FontRegistry()` 재도입 금지
   - `main.cpp`의 ImGui context 생성 이후 폰트 초기화 순서 유지
   - `ChargeDensityUI::setGridDataEntries()` first-grid bootstrap guard 유지

## 1. 목표/비목표

### 목표
1. `App`, `Toolbar`, `ModelTree`, `MeshDetail`, `MeshGroupDetail`, `TestWindow` singleton 의존을 제거하거나 runtime 내부 shim으로만 제한한다.
2. 창 열림 상태/레이아웃 상태를 `ShellStateStore`로 단일화한다.
3. command dispatch/menu action을 `shell/application/workbench_controller`로 집중한다.
4. `app.cpp`를 frame orchestration 중심으로 축소하고 feature 직접 제어를 제거한다.
5. 신규 shell/presenter/state public API에 Doxygen 주석을 적용한다.

### 비목표
1. compatibility facade 완전 제거(Phase 12 범위)
2. 최종 architecture seal/CI gate 구축(Phase 12 범위)
3. `AtomsTemplate`/`MeshManager`/`FileLoader` 최종 삭제(Phase 12 범위)

## 2. W0 기준선(착수 시 재실측)

W0에서 아래 항목을 재실측하고 기준선을 `phase11/logs`에 고정한다.

| 항목 | 측정 대상 | 목표 |
|---|---|---|
| panel/shell singleton 선언 수 | `app/toolbar/model_tree/mesh_detail/mesh_group_detail/test_window` | W6 시점 0 또는 runtime shim 한정 |
| `MeshManager::Instance()` 잔여 호출(특히 `mesh_detail.cpp`) | `webassembly/src/**/*.cpp` | 단계별 감소 |
| `FileLoader::Instance()` 정적 entry 호출 수 | `webassembly/src/**/*.cpp` | 단계별 감소 |
| `App::Instance()` 직접 호출 분포 | `webassembly/src/**/*.cpp` | controller 경유 전환 |
| `P9-BUG-01` 반복 시퀀스 재현 결과 | manual/smoke 체크리스트 | W6 최종 판정 |

W0 산출 로그:
- `docs/refactoring/phase11/logs/shell_panel_inventory_phase11_latest.md`
- `docs/refactoring/phase11/logs/bug_p11_vasp_grid_sequence_latest.md`

## 3. 설계 원칙

1. Composition root는 `WorkbenchRuntime` 하나로 유지한다.
2. panel은 singleton static state 대신 상태 저장소(`ShellStateStore`, `WorkspaceStore`)를 사용한다.
3. UI 이벤트는 `shell/application/workbench_controller` command API를 통해서만 feature에 전달한다.
4. presenter는 read model 조회만 수행하고 repository 직접 수정을 금지한다.
5. 임시 bridge/shim은 허용하되 제거 조건과 목표 Phase를 Doxygen `@note`로 명시한다.

## 4. 작업 단위(WBS)

## W0. 기준선 고정 + 리스크 추적 시작
### 작업
- singleton 선언/직접 호출 분포 실측
- `P9-BUG-01` 반복 시퀀스 체크리스트를 Phase 11 추적 로그로 이관
- shell 상태 소유권(창 open/layout/active panel) 현행 맵 작성

### 산출물
- `docs/refactoring/phase11/logs/shell_panel_inventory_phase11_latest.md`
- `docs/refactoring/phase11/logs/bug_p11_vasp_grid_sequence_latest.md`

### 완료 기준
- W1~W6 비교 가능한 수치/상태 기준선 문서화 완료

## W1. `ShellStateStore`/계약 정립
### 작업
- `shell/domain/shell_state_store.*` 도입
- 창 열림/레이아웃/활성 panel 상태를 shell 저장소로 이전할 계약 정의
- shell query/command service 헤더에 Doxygen 주석 적용

### 영향 파일(예상)
- `webassembly/src/shell/domain/shell_state_store.h`
- `webassembly/src/shell/domain/shell_state_store.cpp`
- `webassembly/src/shell/application/*`

### 완료 기준
- shell UI 상태의 단일 소스가 코드 상에서 고정됨

## W2. `workbench_controller` 도입 + command dispatch 이동
### 작업
- `shell/application/workbench_controller.*` 도입
- menu/toolbar action routing을 controller 명령으로 집중
- `App`/`Toolbar`에서 feature 직접 호출 감소

### 영향 파일(예상)
- `webassembly/src/shell/application/workbench_controller.h`
- `webassembly/src/shell/application/workbench_controller.cpp`
- `webassembly/src/toolbar.cpp`
- `webassembly/src/app.cpp`

### 완료 기준
- command 진입점이 controller 중심으로 정리됨

## W3. Panel singleton 해체 1차 (`ModelTree`, `MeshGroupDetail`, `TestWindow`)
### 작업
- `DECLARE_SINGLETON` 제거 또는 runtime 소유 인스턴스 전환
- panel static mutable state를 shell/workspace store로 이동
- `model_tree` 이벤트 처리와 선택 상태를 presenter 패턴으로 정리

### 영향 파일(예상)
- `webassembly/src/model_tree.h`
- `webassembly/src/model_tree.cpp`
- `webassembly/src/mesh_group_detail.h`
- `webassembly/src/mesh_group_detail.cpp`
- `webassembly/src/test_window.h`
- `webassembly/src/test_window.cpp`

### 완료 기준
- 대상 panel의 singleton 직접 호출이 제거되거나 runtime 내부로 국소화됨

## W4. Panel singleton 해체 2차 (`MeshDetail`) + `App` 축소
### 작업
- `mesh_detail`의 singleton/직접 의존 축소
- `mesh_detail.cpp` 잔여 `MeshManager::Instance()` 호출 감소(Phase 10 잔여 리스크 대응)
- `app.cpp`를 frame orchestration 중심으로 정리

### 영향 파일(예상)
- `webassembly/src/mesh_detail.h`
- `webassembly/src/mesh_detail.cpp`
- `webassembly/src/app.cpp`
- `webassembly/src/shell/runtime/workbench_runtime.cpp`

### 완료 기준
- `app.cpp`에서 feature 제어 로직이 controller/service 경유로 이전됨

## W5. 정적 게이트 스크립트 도입
### 작업
- `scripts/refactoring/check_phase11_shell_panel_objectization.ps1` 작성
- 필수 검증 항목:
  1. 대상 panel/shell singleton 선언 제거 또는 예외 목록 한정
  2. `app.cpp` feature 직접 호출 budget 확인
  3. `mesh_detail.cpp` 잔여 `MeshManager::Instance()` budget 확인
  4. `FileLoader::Instance()` 정적 entry budget 확인
  5. `shell_state_store`/`workbench_controller` 파일 존재 + Doxygen
  6. Phase 9 보호 규칙 유지
  7. `bug_p11_vasp_grid_sequence_latest.md` 존재 + 상태 태그(`Open/Resolved/Deferred`)

### 산출물
- `scripts/refactoring/check_phase11_shell_panel_objectization.ps1`
- `docs/refactoring/phase11/logs/check_phase11_shell_panel_objectization_latest.txt`

### 완료 기준
- 반복 실행 가능한 정적 게이트 PASS

## W6. 빌드/테스트/게이트 리포트/Go-NoGo
### 작업
- 실행 게이트:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase11_shell_panel_objectization.ps1`
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
- 종료 문서:
  - `docs/refactoring/phase11/dependency_gate_report.md`
  - `docs/refactoring/phase11/go_no_go_phase12.md`
- `P9-BUG-01`(Phase 11 추적 로그 기준) 최종 상태 명시:
  - `Resolved` 또는 `Deferred`

### 로그 산출 경로
- `docs/refactoring/phase11/logs/check_phase11_shell_panel_objectization_latest.txt`
- `docs/refactoring/phase11/logs/build_phase11_latest.txt`
- `docs/refactoring/phase11/logs/unit_test_phase11_latest.txt`
- `docs/refactoring/phase11/logs/smoke_phase11_latest.txt`
- `docs/refactoring/phase11/logs/bug_p11_vasp_grid_sequence_latest.md`

### 완료 기준
- Phase 12 착수 여부를 판단할 수 있는 정적/동적 게이트 패키지 확보

## 5. 완료 기준(DoD)

1. 대상 panel/shell singleton이 제거되었거나 runtime 내부 shim으로만 제한됨
2. `app.cpp`가 frame orchestration 중심으로 축소됨
3. shell UI 상태가 `ShellStateStore`로 단일화됨
4. command dispatch/menu action이 `workbench_controller` 중심으로 정리됨
5. `mesh_detail.cpp`/`FileLoader::Instance()` 잔여 singleton 호출이 계획된 budget 이하로 감소함
6. Phase 9 보호 규칙(폰트/XSF bootstrap/render 경계)이 유지됨
7. `P9-BUG-01` 상태가 W6 종료 문서에 `Resolved` 또는 `Deferred`로 명시됨
8. 정적 게이트 + 빌드 + C++ 테스트 + smoke 테스트 PASS
9. `dependency_gate_report.md`, `go_no_go_phase12.md` 작성 완료

## 6. 주석/문서 정책 반영

1. 신규 shell controller/store/query/command API는 Doxygen 주석을 필수 적용한다.
2. 주석은 책임/상태 소유권/호출 순서/제약을 중심으로 작성한다.
3. 임시 bridge/shim은 제거 조건과 차기 단계(Phase 12) 계획을 `@note`로 명시한다.

## 7. Git 저장소 운영 계획

1. 작업 브랜치: `refactor/phase11-shell-objectization`
2. 커밋 단위:
   1. W0 기준선/버그 로그 고정
   2. W1 shell state store
   3. W2 workbench controller
   4. W3 panel 해체 1차
   5. W4 panel 해체 2차 + app 축소
   6. W5 정적 게이트
   7. W6 빌드/테스트 로그 + 종료 문서
3. 로컬 커밋 원칙:
   - WBS 종료마다 로컬 커밋 수행
4. 원격 동기화 원칙:
   - Phase 11 종료 선언 직후 원격 push 수행

## 8. 리스크 및 대응

1. 리스크: panel singleton 제거 중 UI 상태 회귀  
대응: W1~W4 단계별 smoke/manual 체크리스트 운영
2. 리스크: `mesh_detail.cpp` 대형 파일 전환 중 누락/회귀  
대응: W4에서 budget 기반 단계 전환 + 정적 게이트 수치 추적
3. 리스크: `P9-BUG-01`이 shell 전환 과정에서 재노출될 가능성  
대응: W0/W6에서 반복 시퀀스 고정 검증
4. 리스크: `FileLoader::Instance()` 정적 entry 제거 과정에서 wasm binding 경로 회귀  
대응: runtime API 경유 계약을 유지하고 binding smoke를 W6에 포함

## 9. 참조 문서
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- `docs/refactoring/phase10/refactoring_phase10_workspace_mesh_io_decouple_260402.md`
- `docs/refactoring/phase10/dependency_gate_report.md`
- `docs/refactoring/phase10/go_no_go_phase11.md`
- `docs/refactoring/phase10/logs/bug_p10_vasp_grid_sequence_latest.md`

## 10. 진행 체크리스트
- [x] W0 기준선/버그 로그 고정
- [x] W1 shell 상태 저장소/계약 정립
- [x] W2 workbench controller 도입
- [x] W3 panel singleton 해체 1차
- [x] W4 panel singleton 해체 2차 + app 축소
- [x] W5 정적 게이트 도입
- [x] W6 빌드/테스트 + 종료 판정 문서화
