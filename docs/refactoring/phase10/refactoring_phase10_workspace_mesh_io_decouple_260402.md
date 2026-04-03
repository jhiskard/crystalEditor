# Phase 10 세부 작업계획서: `MeshManager`/`FileLoader` 해체 + `workspace` 문맥 단일화

작성일: `2026-04-02 (KST)`  
기준 계획: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (Phase 10 절)  
선행 판정: **GO** (`docs/refactoring/phase9/go_no_go_phase10.md`)  
대상 범위: `webassembly/src/workspace/*`, `webassembly/src/mesh/*`, `webassembly/src/io/*`, `webassembly/src/model_tree*`, `webassembly/src/mesh_detail*`, `webassembly/src/mesh_group_detail*`, `webassembly/src/shell/runtime/*`, `scripts/refactoring/*`, `docs/refactoring/phase10/*`  
진행 상태: `W0~W6 예정`

## 0. 착수 배경과 고정 전제

### 0.1 착수 배경
1. Phase 9에서 `render` 경계 전환이 완료되어 `render` 외부 `VtkViewer::Instance()` 호출 0건이 검증되었다.
2. Phase 10은 전체 계획서 기준으로 `MeshManager`/`FileLoader`/`workspace`를 분리해 상태 소유권을 정렬하는 단계다.
3. Phase 9 이관 이슈 `P9-BUG-01`(XSF/VASP 반복 로드 시 grid clear 비정상)을 Phase 10 우선 추적 대상으로 유지한다.

### 0.2 Phase 9 산출물 반영 항목
1. `render` 경계 규칙 유지:
   - `render` 외부 `VtkViewer::Instance()` 재유입 금지
2. 보호 규칙 유지:
   - `main.cpp`의 ImGui 컨텍스트 생성 후 폰트 레지스트리 초기화 순서 유지
   - `ChargeDensityUI::setGridDataEntries()`의 first-grid bootstrap 가드 유지
3. W6 보정 반영 유지:
   - `test_window.cpp`의 `vtkActor` include
   - `mesh_manager.cpp`의 `vtkVolume` include
4. `P9-BUG-01`은 Phase 10에서 재현/해결/이관 판정을 문서로 확정한다.

## 1. 목표/비목표

### 목표
1. feature 코드에서 `MeshManager::Instance()`, `FileLoader::Instance()` 호출을 0건으로 만든다.
2. `workspace`를 selected mesh/current structure/active density context의 단일 소스로 고정한다.
3. `ModelTree`, `MeshDetail`, `MeshGroupDetail`를 read model 기반 presenter 흐름으로 전환한다.
4. import workflow를 `io/application` 서비스로 집중해 panel 직접 갱신 결합을 제거한다.
5. 신규/변경 public API에 Doxygen 주석을 적용한다.

### 비목표
1. `App`/`Toolbar`/panel singleton 완전 제거(Phase 11 범위)
2. compatibility facade 최종 제거(Phase 12 범위)
3. 디렉터리 대규모 rename(Phase 12 범위)

## 2. W0 기준선(착수 시 재실측)

W0에서 아래 항목을 재실측해 기준선을 고정한다.

| 항목 | 측정 대상 | 목표 |
|---|---|---|
| `MeshManager::Instance()` 호출 수 | `webassembly/src/**/*.cpp,*.h` | W6 시점 0 |
| `FileLoader::Instance()` 호출 수 | `webassembly/src/**/*.cpp,*.h` | W6 시점 0 |
| panel direct mutation 지점 | `model_tree*`, `mesh_detail*`, `mesh_group_detail*` | 서비스 경유 목록 고정 |
| workspace 문맥 보유 위치 | `app/shell/mesh/io` | `workspace` 단일화 |
| `P9-BUG-01` 재현 결과 | 수동/자동 시나리오 | 해결 또는 이관 판정 |

W0 산출 로그:
- `docs/refactoring/phase10/logs/workspace_mesh_io_inventory_phase10_latest.md`
- `docs/refactoring/phase10/logs/bug_p10_vasp_grid_sequence_latest.md`

## 3. 설계 원칙

1. `workspace`는 context id/선택 상태를 소유하고 실제 데이터는 각 repository가 소유한다.
2. panel은 repository를 직접 수정하지 않고 application service/command만 호출한다.
3. import 흐름은 `io/application/import_workflow_service`에 집중한다.
4. render/VTK 직접 접근은 Phase 9 경계 규칙을 그대로 유지한다.
5. 리팩토링 중 임시 bridge를 두더라도 최종 게이트 기준은 `Instance()` 직접 호출 0건이다.

## 4. 작업 단위(WBS)

## W0. 기준선 고정 + 이슈 추적 시작
### 작업
- 호출 인벤토리와 panel mutation 지점 재수집
- `P9-BUG-01` 반복 시퀀스 재현 절차 고정:
  - `XSF(Grid) -> VASP -> XSF(Grid) -> VASP`
- 정적 게이트 기준값 확정

### 산출물
- `docs/refactoring/phase10/logs/workspace_mesh_io_inventory_phase10_latest.md`
- `docs/refactoring/phase10/logs/bug_p10_vasp_grid_sequence_latest.md`

### 완료 기준
- W1~W6 비교 가능한 수치/증상 기준선 문서화 완료

## W1. `workspace` 저장소/계약 정립
### 작업
- `workspace/domain/workspace_store.*`의 문맥 모델 정의(활성 구조/메시/밀도/선택 컨텍스트)
- `workspace/application`에서 읽기/전환/초기화 계약 정리
- public API Doxygen 주석 적용

### 영향 파일(예상)
- `webassembly/src/workspace/domain/workspace_store.h`
- `webassembly/src/workspace/domain/workspace_store.cpp`
- `webassembly/src/workspace/application/*`

### 완료 기준
- panel/feature가 참조할 단일 workspace 문맥 API가 고정됨

## W2. `MeshManager` 해체(Repository + Query/Service)
### 작업
- `MeshManager` 책임을 `mesh/domain/mesh_repository` 및 query/service로 이동
- 메시 선택/가시성/그룹 조회 경로를 repository 기반 호출로 전환
- 임시 호환 계층이 필요하면 명시적 TODO와 제거 조건 기록

### 영향 파일(예상)
- `webassembly/src/mesh/domain/mesh_repository.h`
- `webassembly/src/mesh/domain/mesh_repository.cpp`
- `webassembly/src/mesh/application/*`
- `webassembly/src/mesh_manager.cpp`
- `webassembly/src/mesh_manager.h`

### 완료 기준
- feature 코드의 `MeshManager::Instance()` 호출 제거 또는 호환 shim 내부로 국소화

## W3. `FileLoader` 축소(import workflow 집중)
### 작업
- `FileLoader`를 `io/application/import_workflow_service` 호출 퍼사드로 축소
- import parse/apply/rollback 흐름을 application 계층으로 집중
- workspace 컨텍스트 갱신 시점을 import 완료 트랜잭션과 일치시킴

### 영향 파일(예상)
- `webassembly/src/io/application/import_workflow_service.h`
- `webassembly/src/io/application/import_workflow_service.cpp`
- `webassembly/src/file_loader.cpp`
- `webassembly/src/file_loader.h`
- `webassembly/src/shell/runtime/workbench_runtime.cpp`

### 완료 기준
- import 문맥 갱신이 단일 workflow 경로로 수렴

## W4. Panel read-model 전환 + `P9-BUG-01` 집중 추적
### 작업
- `ModelTree`, `MeshDetail`, `MeshGroupDetail`를 read model + service 호출 패턴으로 전환
- panel direct mutation 제거
- `P9-BUG-01`에 대해 clear 타이밍/적용 순서/context reset을 집중 점검 및 수정

### 영향 파일(예상)
- `webassembly/src/model_tree.cpp`
- `webassembly/src/model_tree_structure_section.cpp`
- `webassembly/src/mesh_detail.cpp`
- `webassembly/src/mesh_group_detail.cpp`
- `webassembly/src/density/*`
- `docs/refactoring/phase10/logs/bug_p10_vasp_grid_sequence_latest.md`

### 완료 기준
- panel은 서비스 경유만 수행하고 `P9-BUG-01` 상태가 재현 로그에 갱신됨

## W5. 정적 게이트 스크립트 도입
### 작업
- `scripts/refactoring/check_phase10_workspace_mesh_io_decouple.ps1` 작성/갱신
- 필수 검증 항목:
  1. `MeshManager::Instance()` 호출 0건(허용 예외 없음)
  2. `FileLoader::Instance()` 호출 0건(허용 예외 없음)
  3. `workspace_store`/`mesh_repository`/`import_workflow_service` 핵심 파일 존재
  4. panel direct mutation 금지 규칙 충족
  5. Phase 9 보호 규칙(폰트 순서/XSF bootstrap/render 경계) 유지
  6. `bug_p10_vasp_grid_sequence_latest.md` 존재 + 상태 태그(`Open/Resolved/Deferred`) 포함
  7. 신규 public API Doxygen 주석 존재

### 산출물
- `scripts/refactoring/check_phase10_workspace_mesh_io_decouple.ps1`
- `docs/refactoring/phase10/logs/check_phase10_workspace_mesh_io_decouple_latest.txt`

### 완료 기준
- 반복 실행 가능한 정적 게이트 PASS

## W6. 빌드/테스트/게이트 리포트/Go-NoGo
### 작업
- 실행 게이트:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase10_workspace_mesh_io_decouple.ps1`
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
- 종료 문서:
  - `docs/refactoring/phase10/dependency_gate_report.md`
  - `docs/refactoring/phase10/go_no_go_phase11.md`
- `P9-BUG-01`(Phase 10 추적 로그 기준) 최종 상태 명시:
  - `Resolved` 또는 `Deferred`

### 로그 산출 경로
- `docs/refactoring/phase10/logs/check_phase10_workspace_mesh_io_decouple_latest.txt`
- `docs/refactoring/phase10/logs/build_phase10_latest.txt`
- `docs/refactoring/phase10/logs/unit_test_phase10_latest.txt`
- `docs/refactoring/phase10/logs/smoke_phase10_latest.txt`
- `docs/refactoring/phase10/logs/bug_p10_vasp_grid_sequence_latest.md`

### 완료 기준
- Phase 11 착수 여부를 판단할 수 있는 정적/동적 게이트 패키지 확보

## 5. 완료 기준(DoD)

1. `MeshManager::Instance()` 호출 0건
2. `FileLoader::Instance()` 호출 0건
3. workspace 문맥이 단일 저장소로 관리됨
4. panel은 repository 직접 수정 없이 service 호출만 수행
5. Phase 9 보호 규칙(폰트/XSF bootstrap/render 경계)이 유지됨
6. `P9-BUG-01` 상태가 W6 종료 문서에 `Resolved` 또는 `Deferred`로 명시됨
7. 정적 게이트 + 빌드 + C++ 테스트 + smoke 테스트 PASS
8. `dependency_gate_report.md`, `go_no_go_phase11.md` 작성 완료

## 6. 주석/문서 정책 반영

1. 신규 public API(특히 `workspace`, `mesh`, `io`의 서비스/리포지토리 헤더)에 Doxygen 주석을 적용한다.
2. 주석은 "무엇을 하는가"가 아니라 경계/소유권/제약/호출 순서를 중심으로 작성한다.
3. 임시 bridge/shim에는 제거 조건과 목표 Phase를 `@note`로 명시한다.

## 7. Git 저장소 운영 계획

1. 작업 브랜치: `refactor/phase10-workspace-mesh-io-decouple`
2. 커밋 단위:
   1. W0 기준선/버그 로그 고정
   2. W1 workspace store/계약
   3. W2 mesh repository 전환
   4. W3 import workflow 전환
   5. W4 panel read-model 전환 + 버그 수정
   6. W5 정적 게이트 스크립트
   7. W6 빌드/테스트 로그 + 종료 문서
3. 로컬 커밋 원칙:
   - WBS 종료 시점마다 로컬 커밋 수행
4. 원격 동기화 원칙:
   - Phase 10 종료 선언 직후 원격 push 수행

## 8. 리스크 및 대응

1. 리스크: workspace 도입 중 기존 선택/활성 문맥이 중복 관리될 가능성  
대응: W1에서 소유권 표를 고정하고 direct mutation 검색 게이트로 상시 검증
2. 리스크: import workflow 전환 시 panel 갱신 타이밍 회귀  
대응: W3~W4에서 단계별 smoke + manual 시나리오 병행
3. 리스크: `P9-BUG-01`이 render가 아닌 import/workspace 경계 문제일 가능성  
대응: W4에서 적용 순서와 clear 책임을 명시적으로 분리해 로그로 판정

## 9. 참조 문서
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- `docs/refactoring/phase9/refactoring_phase9_render_boundary_complete_260402.md`
- `docs/refactoring/phase9/dependency_gate_report.md`
- `docs/refactoring/phase9/go_no_go_phase10.md`

## 10. 진행 체크리스트
- [x] W0 기준선/버그 로그 고정
- [x] W1 workspace 저장소/계약 정립
- [x] W2 MeshManager 해체
- [x] W3 FileLoader 축소 + import workflow 집중
- [x] W4 panel read-model 전환 + `P9-BUG-01` 추적
- [x] W5 정적 게이트 도입
- [x] W6 빌드/테스트 + 종료 판정 문서화

## 11. Execution Update (2026-04-03 KST, W0~W5)

- [x] W0 기준선/버그 로그 고정
  - `workspace_mesh_io_inventory_phase10_latest.md` 갱신
  - `bug_p10_vasp_grid_sequence_latest.md` 갱신 (`Deferred`)
- [x] W1 workspace 저장소/계약 정립
  - `workspace_store`, `workspace_query_service`, `workspace_command_service` 추가
  - `WorkbenchRuntime`에 workspace query/command 접근자 추가
- [x] W2 MeshManager 해체(1차)
  - `mesh_repository`, `mesh_query_service`, `mesh_command_service` 추가
  - `import_orchestrator` + `mesh/presentation/model_tree_*` 경로를 mesh service 경유로 전환
- [x] W3 FileLoader 축소 + import workflow 집중
  - `import_workflow_service` 추가
  - replace-scene import transaction을 `FileLoader`에서 workflow 서비스로 이관
- [x] W4 panel read-model 전환 + `P9-BUG-01` 추적
  - `model_tree` selected mesh id를 workspace command로 동기화
  - `mesh_group_detail` 포함 panel 경로 singleton 의존 축소
  - `bug_p10_vasp_grid_sequence_latest.md` 상태/분석 갱신(`Deferred`)
- [x] W5 정적 게이트 도입
  - `check_phase10_workspace_mesh_io_decouple.ps1` 추가
  - `check_phase10_workspace_mesh_io_decouple_latest.txt` PASS 로그 확보
- [x] W6 빌드/테스트 + 종료 판정 문서화
  - `build_phase10_latest.txt`, `unit_test_phase10_latest.txt`, `smoke_phase10_latest.txt` 갱신
  - `dependency_gate_report.md`, `go_no_go_phase11.md` 작성
