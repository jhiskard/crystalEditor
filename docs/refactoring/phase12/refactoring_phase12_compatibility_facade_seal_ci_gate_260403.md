# Phase 12 세부 작업계획서: Compatibility facade 제거 + 모듈 seal + Architecture/CI 게이트

작성일: `2026-04-03 (KST)`  
기준 계획: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (Phase 12 절)  
선행 판정: **GO** (`docs/refactoring/phase11/go_no_go_phase12.md`)  
대상 범위: `webassembly/src/*`, `webassembly/cmake/modules/*`, `webassembly/tests/*`, `scripts/refactoring/*`, `docs/refactoring/phase12/*`  
진행 상태: `W0~W6 예정`

## 0. 착수 배경과 고정 전제

### 0.1 착수 배경
1. Phase 11에서 shell/panel 객체화와 controller 경유화가 완료되었고 W6 게이트가 PASS로 종료됐다.
2. 전체계획서 기준 Phase 12는 리팩토링 마무리 단계로, compatibility facade 제거와 모듈 경계 seal, architecture gate 고정이 목적이다.
3. Phase 11 종료 시점 잔여 리스크(`P12-R1~R3`)를 본 Phase의 필수 추적 항목으로 승계한다.

### 0.2 Phase 11 결과 반영 항목
1. `App`, `Toolbar` singleton 잔존을 runtime/controller 경계 내에서 최종 제거한다.
2. `FileLoader::Instance()` 정적 entry 잔존을 workflow/runtime facade 경유로 추가 축소한다.
3. `P9-BUG-01` 반복 시퀀스(`XSF(Grid) -> VASP -> XSF(Grid) -> VASP`)는 W6에서 `Resolved/Deferred`를 재판정한다.
4. Phase 9~11 보호 규칙을 유지한다.
   - `PrimeLegacySingletons()`에 `FontRegistry()` 재도입 금지
   - `main.cpp`의 ImGui context 생성 이후 폰트 초기화 순서 유지
   - `ChargeDensityUI::setGridDataEntries()` first-grid bootstrap guard 유지
   - `render` 외부 VTK 직접 접근 금지 원칙 유지

## 1. 목표/비목표

### 목표
1. `AtomsTemplate`, `MeshManager`, `FileLoader`, `ModelTree`, `Toolbar`, `VtkViewer` compatibility facade를 제거하거나 runtime composition root 내부 shim으로만 제한한다.
2. feature/application/domain 계층의 `::Instance()` 호출을 0으로 만들거나 runtime composition root 내부로만 국소화한다.
3. 최종 CMake 타깃 그래프를 비순환(acyclic)으로 고정하고 정적 검증으로 회귀를 차단한다.
4. Phase 6 테스트 체계에 architecture regression 게이트를 포함해 빌드/테스트/아키텍처 검증을 하나의 종료 패키지로 고정한다.
5. 최종 public API(Domain/Application/Facade 경계)에 Doxygen 주석 누락분을 정리한다.

### 비목표
1. 신규 대형 기능 추가
2. UX/디자인 변경 중심 작업
3. 성능 최적화 전용 작업(본 Phase는 경계 고정과 회귀 차단이 중심)

## 2. W0 기준선(착수 시 재실측)

W0에서 아래 항목을 재실측하고 기준선을 `phase12/logs`에 고정한다.

| 항목 | 측정 대상 | 착수 시점 실측(2026-04-03) | 목표 |
|---|---|---:|---|
| 핵심 singleton 선언 수 | `AtomsTemplate/MeshManager/FileLoader/App/Toolbar/VtkViewer` | 6 | W6 시점 0 또는 runtime shim 한정 |
| 전체 `::Instance()` 호출 수 | `webassembly/src/**/*.cpp` | 192 | 단계별 축소 + 허용 경로 한정 |
| `AtomsTemplate::Instance()` 호출 수 | `webassembly/src/**/*.cpp` | 67 | 0 또는 runtime shim 한정 |
| `MeshManager::Instance()` 호출 수 | `webassembly/src/**/*.cpp` | 35 | 0 또는 runtime shim 한정 |
| `FileLoader::Instance()` 호출 수 | `webassembly/src/**/*.cpp` | 4 | 0 또는 runtime shim 한정 |
| `VtkViewer::Instance()` 호출 수 | `webassembly/src/**/*.cpp` | 22 | render 내부 또는 runtime shim 한정 |
| `friend class` 사용 수 | `webassembly/src/**/*.h` | 1 (`lcrs_tree.h`) | 0 또는 예외 목록 고정 |
| legacy alias 노출 위치 | `createdAtoms/createdBonds/cellInfo` 참조 분포 | 다수 | compatibility layer 외부 노출 축소/차단 |

W0 산출 로그:
- `docs/refactoring/phase12/logs/facade_inventory_phase12_latest.md`
- `docs/refactoring/phase12/logs/bug_p12_vasp_grid_sequence_latest.md`
- `docs/refactoring/phase12/logs/module_graph_phase12_latest.md`

## 3. 설계 원칙

1. Composition root는 `WorkbenchRuntime` 하나로 유지한다.
2. façade 제거는 "호출점 이전 -> 계약 고정 -> 구 façade 제거" 순서를 따른다.
3. infrastructure 진입점(렌더, WASM 바인딩, 파일 로드)은 application service를 통해서만 사용한다.
4. 아키텍처 규칙은 문서뿐 아니라 스크립트 게이트로 강제한다.
5. 임시 shim/bridge는 `@note`에 제거 조건과 제거 Phase를 명시한다.

## 4. 작업 단위(WBS)

## W0. 기준선 고정 + 리스크 추적 등록
### 작업
- singleton/`::Instance()`/`friend class`/legacy alias 노출 지표 실측
- `P9-BUG-01` 추적 로그를 Phase 12 로그로 승계
- 최종 모듈/타깃 의존 그래프 기준선 수집

### 산출물
- `docs/refactoring/phase12/logs/facade_inventory_phase12_latest.md`
- `docs/refactoring/phase12/logs/bug_p12_vasp_grid_sequence_latest.md`
- `docs/refactoring/phase12/logs/module_graph_phase12_latest.md`

### 완료 기준
- W1~W6 비교 가능한 기준선이 문서/로그로 고정됨

## W1. Shell/Runtime 잔여 singleton 최종 정리 (`App`, `Toolbar`)
### 작업
- `App`, `Toolbar`의 singleton 선언 제거 또는 runtime 내부 소유로 국소화
- 메뉴/툴바 진입점에서 compatibility 경유 호출 제거
- public API 주석(`@brief/@note`) 보강

### 영향 파일(예상)
- `webassembly/src/app.h`
- `webassembly/src/app.cpp`
- `webassembly/src/toolbar.h`
- `webassembly/src/toolbar.cpp`
- `webassembly/src/shell/runtime/workbench_runtime.*`

### 완료 기준
- `App`, `Toolbar` singleton 잔여가 제거되거나 runtime 내부 shim으로만 제한됨

## W2. Import/Mesh/Atoms façade 호출점 이전 (`FileLoader`, `MeshManager`, `AtomsTemplate`)
### 작업
- `FileLoader::Instance()`, `MeshManager::Instance()`, `AtomsTemplate::Instance()` 직접 호출을 서비스 경유로 전환
- import/mesh/structure read model 경계에서 compatibility alias 의존 축소
- `createdAtoms/createdBonds/cellInfo` 직접 노출 경로를 호환 레이어 내부로 격리

### 영향 파일(예상)
- `webassembly/src/file_loader.*`
- `webassembly/src/mesh_manager.*`
- `webassembly/src/atoms/atoms_template.*`
- `webassembly/src/io/application/*`
- `webassembly/src/mesh/application/*`
- `webassembly/src/atoms/application/*`

### 완료 기준
- 핵심 feature 경로에서 3대 singleton 직접 호출이 제거되거나 허용 예외 목록으로만 제한됨

## W3. Render 경계 seal + `VtkViewer` compatibility 축소
### 작업
- non-render 모듈의 `VtkViewer::Instance()` 호출 제거
- actor/volume 조작을 render application/infrastructure 경계 내부로 재배치
- wasm binding이 runtime/controller 경유 계약을 준수하는지 점검

### 영향 파일(예상)
- `webassembly/src/vtk_viewer.*`
- `webassembly/src/render/**/*`
- `webassembly/src/platform/wasm/workbench_bindings.cpp`
- `webassembly/src/app.cpp`, `webassembly/src/toolbar.cpp`

### 완료 기준
- render 외부 `VtkViewer` 직접 의존이 제거되거나 런타임 shim으로 한정됨

## W4. Build target 재정렬 + 모듈 그래프 비순환 고정
### 작업
- CMake 타깃을 최종 모듈 경계 기준으로 재정렬
- `target_link_libraries` 방향성을 정리하고 순환 의존 제거
- 필요 시 legacy 타깃(`wb_atoms` 등) 정리 또는 최종 alias로 제한

### 영향 파일(예상)
- `webassembly/CMakeLists.txt`
- `webassembly/cmake/modules/*.cmake`
- `webassembly/tests/CMakeLists.txt`

### 완료 기준
- 최종 모듈 그래프 비순환 상태가 재현 가능하게 고정됨

## W5. Architecture gate 스크립트 + 테스트 파이프라인 결합
### 작업
- `scripts/refactoring/check_phase12_architecture_seal.ps1` 작성
- 필수 검증 항목:
  1. 핵심 singleton 선언 잔여(또는 허용 목록) 검증
  2. feature/application/domain의 `::Instance()` 호출 금지 검증
  3. `friend class` 0건(또는 승인된 예외 목록) 검증
  4. legacy alias(`createdAtoms/createdBonds/cellInfo`) 외부 노출 검증
  5. render 외부 VTK 직접 접근 금지 검증
  6. 모듈 그래프 순환 의존 검증
  7. `bug_p12_vasp_grid_sequence_latest.md` 상태 태그 검증
- Phase 6 테스트 실행 루틴(`test:phase6` 또는 동등 스크립트)에 architecture gate를 결합

### 산출물
- `scripts/refactoring/check_phase12_architecture_seal.ps1`
- `docs/refactoring/phase12/logs/check_phase12_architecture_seal_latest.txt`

### 완료 기준
- 정적 architecture gate를 반복 실행해 PASS 가능

## W6. 빌드/테스트/게이트 리포트/Go-NoGo
### 작업
- 실행 게이트:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase12_architecture_seal.ps1`
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
- 종료 문서:
  - `docs/refactoring/phase12/dependency_gate_report.md`
  - `docs/refactoring/phase12/go_no_go_phase13.md`
- `P9-BUG-01` 최종 상태를 `Resolved` 또는 `Deferred`로 명시

### 로그 산출 경로
- `docs/refactoring/phase12/logs/check_phase12_architecture_seal_latest.txt`
- `docs/refactoring/phase12/logs/build_phase12_latest.txt`
- `docs/refactoring/phase12/logs/unit_test_phase12_latest.txt`
- `docs/refactoring/phase12/logs/smoke_phase12_latest.txt`
- `docs/refactoring/phase12/logs/bug_p12_vasp_grid_sequence_latest.md`

### 완료 기준
- 최종 architecture 전환 종료 판단이 가능한 정적/동적 게이트 패키지 확보

## 5. 완료 기준(DoD)

1. `AtomsTemplate`, `MeshManager`, `FileLoader`, `ModelTree`, `Toolbar`, `VtkViewer` singleton이 제거되었거나 runtime composition root 내부 shim으로만 제한됨
2. feature/application/domain 코드의 `::Instance()` 호출이 0건(또는 승인된 runtime shim 한정)으로 고정됨
3. `friend class` 사용이 0건(또는 승인된 예외 목록)으로 관리됨
4. `createdAtoms`, `createdBonds`, `cellInfo` 등 legacy alias가 compatibility layer 밖으로 노출되지 않음
5. 최종 모듈 타깃 그래프가 비순환이며 정적 스크립트로 검증 가능함
6. `app.cpp`는 shell orchestration 중심 역할을 유지하고 feature 직접 제어를 포함하지 않음
7. render 외부에서 VTK actor/volume 직접 조작이 발생하지 않음
8. `npm run test:cpp`, `npm run test:smoke`, architecture gate가 모두 PASS
9. `dependency_gate_report.md`, `go_no_go_phase13.md`가 작성 완료됨
10. 신규/정리된 public 타입/함수/상태 단위에 Doxygen 주석이 적용됨

## 6. 주석/문서 정책 반영

1. 신규/변경 API는 Doxygen 스타일 주석(`@brief`, 필요 시 `@note`)을 적용한다.
2. 주석은 책임, 상태 소유권, 호출 순서, 제거 계획(temporary shim)을 중심으로 작성한다.
3. compatibility 제거 과정의 임시 경계는 제거 조건을 명시한다.

## 7. Git 저장소 운영 계획

1. 작업 브랜치: `refactor/phase12-architecture-seal`
2. 커밋 단위:
   1. W0 기준선/버그 로그 고정
   2. W1 app/toolbar singleton 정리
   3. W2 file-loader/mesh-manager/atoms-template 호출점 이전
   4. W3 render 경계 seal + vtk viewer 축소
   5. W4 build target 재정렬
   6. W5 architecture gate + 테스트 루틴 결합
   7. W6 빌드/테스트 로그 + 종료 문서
3. 로컬 커밋 원칙:
   - WBS 종료마다 로컬 커밋 수행
4. 원격 동기화 원칙:
   - Phase 12 종료 선언 직후 원격 push 수행

## 8. 리스크 및 대응

1. 리스크: façade 제거 중 런타임 회귀(Import/Viewer/UI)  
대응: W1~W4 단계별 smoke/manual 체크리스트 병행 + W5 정적 게이트 수치 추적
2. 리스크: 모듈 그래프 재정렬 중 순환 의존 재유입  
대응: W4에서 `target_link_libraries` 방향 검증 + W5 순환 의존 검사 스크립트 고정
3. 리스크: `P9-BUG-01` 반복 시퀀스 미해결 상태 지속  
대응: W0/W6에서 반복 시퀀스 추적 로그를 갱신하고 `Resolved/Deferred`를 문서에 명시
4. 리스크: legacy alias 차단 과정에서 기존 UI/로더 동작 영향  
대응: alias 접근 계층을 명시적으로 한정하고 단계별 회귀 테스트로 검증

## 9. 참조 문서
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- `docs/refactoring/phase11/refactoring_phase11_shell_panel_objectization_260403.md`
- `docs/refactoring/phase11/dependency_gate_report.md`
- `docs/refactoring/phase11/go_no_go_phase12.md`
- `docs/refactoring/phase11/logs/bug_p11_vasp_grid_sequence_latest.md`

## 10. 진행 체크리스트
- [ ] W0 기준선/버그 로그 고정
- [ ] W1 app/toolbar singleton 최종 정리
- [ ] W2 file-loader/mesh-manager/atoms-template 호출점 이전
- [ ] W3 render 경계 seal + vtk viewer compatibility 축소
- [ ] W4 build target 재정렬 + 모듈 비순환 고정
- [ ] W5 architecture gate 도입 + 테스트 파이프라인 결합
- [ ] W6 빌드/테스트 + 종료 판정 문서화
