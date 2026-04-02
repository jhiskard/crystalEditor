# Phase 9 세부 작업계획서: `VtkViewer` 해체 + render 포트 완성

작성일: `2026-04-02 (KST)`  
기준 계획: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (Phase 9 절)  
선행 판정: **GO** (`docs/refactoring/phase8/go_no_go_phase9.md`)  
대상 범위: `webassembly/src/render/*`, `webassembly/src/vtk_viewer.*`, `webassembly/src/atoms/*`, `webassembly/src/mesh*`, `webassembly/src/toolbar.cpp`, `webassembly/src/file_loader.cpp`, `webassembly/src/shell/runtime/*`, `scripts/refactoring/*`, `docs/refactoring/phase9/*`  
진행 상태: `W0 착수 대기`

## 0. Phase 8 종료 반영사항

### 0.1 종료 선언
- **2026-04-02 기준으로 Phase 8 종료를 선언**한다.
- 종료 근거 문서:
  - `docs/refactoring/phase8/dependency_gate_report.md`
  - `docs/refactoring/phase8/go_no_go_phase9.md`

### 0.2 Phase 9에서 유지할 보호 규칙
1. 폰트/아이콘 회귀 방지 규칙 유지
   - `main.cpp`에서 `ImGui::CreateContext()` 이후 `FontRegistry()` 초기화
   - `PrimeLegacySingletons()`에서 `FontRegistry()` 재도입 금지
2. VASP -> XSF(Grid) 회귀 방지 규칙 유지
   - `ChargeDensityUI::setGridDataEntries()`의 첫 grid bootstrap 유지
3. runtime quarantine 규칙 유지
   - Phase 7에서 정의한 runtime/binding 경계와 singleton 가드 훼손 금지

### 0.3 Phase 9 추적 이슈 등록 (`P9-BUG-01`)
- 이슈명: `XSF(Grid) -> VASP` 반복 전환 시 grid 데이터 잔존
- 재현 시나리오: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP`
- 현재 상태(2026-04-02): 미해결, 시도 패치 2건 롤백 완료
- 처리 원칙:
  1. W0에서 재현 절차/로그를 고정한다.
  2. W3~W4에서 render ownership/정리 경계를 점검한다.
  3. W6 종료 문서에서 해결 여부를 명시하고, 미해결 시 Phase 10으로 이관한다.

## 1. 착수 기준선 (2026-04-02 실측)

| 항목 | 기준값 | 비고 |
|---|---:|---|
| `webassembly/src/vtk_viewer.cpp` 라인 수 | 1995 | 대형 viewer 구현 |
| `webassembly/src/vtk_viewer.h` 라인 수 | 155 | public surface 큼 |
| `webassembly/src/render/application/render_gateway.h` 라인 수 | 38 | 포트 표면 제한적 |
| `webassembly/src` 전체 `VtkViewer::Instance()` 호출 수 | 119 | render 경계 외 호출 다수 |
| `render` 내부 `VtkViewer::Instance()` 호출 수 | 10 | adapter 집중 대상 |
| `render` 외부 `VtkViewer::Instance()` 호출 수 | 109 | Phase 9 종료 목표 0 |
| `atoms_template.cpp` `VtkViewer::Instance()` | 46 | 최우선 전환 대상 |
| `mesh.cpp` `VtkViewer::Instance()` | 28 | scene/update 결합 높음 |
| `toolbar.cpp` `VtkViewer::Instance()` | 11 | camera/projection 결합 |
| `measurement_controller.cpp` `VtkViewer::Instance()` | 9 | overlay render 요청 결합 |

상세 기준선 로그:
- `docs/refactoring/phase9/logs/render_inventory_phase9_latest.md`

## 2. Phase 9 목표/비목표

### 목표
1. `render` 외부의 `VtkViewer::Instance()` 직접 호출을 `0건`으로 만든다.
2. viewer 책임을 `viewport/camera/picking/overlay/scene` 단위로 분리하고 포트 중심으로 재배선한다.
3. feature 계층(`atoms/mesh/ui/shell`)에서 render infrastructure 역참조를 제거한다.
4. 새 render port/adapter의 public API에 Doxygen 주석을 적용한다.
5. `P9-BUG-01`을 추적 가능한 상태로 고정하고, 종료 시 해결 여부를 판정 문서에 반영한다.

### 비목표
1. `MeshManager`/`FileLoader` 완전 해체(Phase 10 범위)
2. compatibility facade 최종 제거(Phase 12 범위)
3. workspace 상태모델 완전 단일화(Phase 10 범위)

## 3. 설계 원칙
1. `VtkViewer` 직접 접근은 `render/infrastructure`로만 한정한다.
2. feature/application/domain 계층에서 `vtk*` include 확장을 금지한다.
3. actor/overlay/volume lifecycle은 render 포트 계약으로만 제어한다.
4. 회귀 이슈는 문서/게이트/로그로 추적 가능한 형태로 남긴다.
5. WBS 종료마다 로컬 커밋을 생성하고, Phase 종료 선언 후에만 원격 push를 수행한다.

## 4. 작업 단위(WBS)

## W0. 착수/기준선 고정 + `P9-BUG-01` 재현 로그화
### 작업
- `VtkViewer::Instance()` 분포 기준선 고정
- `P9-BUG-01` 재현 절차와 관찰 결과를 로그로 고정
- Phase 9 정적 게이트 기준치(외부 호출 0 목표) 확정

### 영향 파일(예상)
- `docs/refactoring/phase9/logs/render_inventory_phase9_latest.md`
- `docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md`

### 완료 기준
- 기준선 수치와 재현 로그가 문서로 고정되어 이후 단계별 변화 추적 가능

## W1. render 포트 계약 확장
### 작업
- `render/application` 포트를 camera/picking/overlay/scene 단위로 명확화
- feature 호출에서 필요한 최소 계약(API)만 노출
- 포트 public API Doxygen 주석 적용

### 영향 파일(예상)
- `webassembly/src/render/application/render_gateway.h`
- `webassembly/src/render/application/camera_align_service.cpp`
- `webassembly/src/render/application/viewer_interaction_controller.cpp`

### 완료 기준
- feature 계층이 참조할 render 계약이 포트 헤더로 명시되고 문서화 완료

## W2. render infrastructure adapter 재편
### 작업
- `vtk_render_gateway`를 viewer 접근 단일 어댑터로 정리
- actor/overlay/volume/camera API의 구현 소유권을 infrastructure로 고정
- `VtkViewer` 호출 경로를 adapter 내부로 수렴

### 영향 파일(예상)
- `webassembly/src/render/infrastructure/vtk_render_gateway.h`
- `webassembly/src/render/infrastructure/vtk_render_gateway.cpp`
- `webassembly/src/vtk_viewer.cpp`
- `webassembly/src/vtk_viewer.h`

### 완료 기준
- viewer 접근 경로가 render infrastructure로 집중되고 중복 경로 제거

## W3. feature 호출 경로 전환 (atoms/mesh/ui 중심)
### 작업
- `atoms/mesh/toolbar/file_loader`의 `VtkViewer::Instance()` 호출을 render 포트 호출로 전환
- overlay/measurement 렌더 갱신 경로를 render service로 위임
- `P9-BUG-01` 관련 actor 정리 지점 누락 여부 점검

### 영향 파일(예상)
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/application/measurement_controller.cpp`
- `webassembly/src/atoms/application/visibility_service.cpp`
- `webassembly/src/mesh.cpp`
- `webassembly/src/mesh_manager.cpp`
- `webassembly/src/toolbar.cpp`
- `webassembly/src/file_loader.cpp`

### 완료 기준
- 상기 파일의 `VtkViewer::Instance()` 직접 호출 제거 또는 render 경유화 완료

## W4. runtime/shell 경계 정리 + viewer interaction 정돈
### 작업
- `workbench_runtime`가 render 포트만 통해 viewer 제어하도록 정리
- mouse interaction/controller 경계 정리
- `vtk_viewer.cpp`를 infrastructure 구현체 성격으로 축소

### 영향 파일(예상)
- `webassembly/src/shell/runtime/workbench_runtime.cpp`
- `webassembly/src/mouse_interactor_style.cpp`
- `webassembly/src/render/*`
- `webassembly/src/vtk_viewer.*`

### 완료 기준
- `render` 외부 viewer 직접 호출 0 목표에 근접/달성

## W5. 정적 게이트 스크립트 도입
### 작업
- `scripts/refactoring/check_phase9_render_boundary_complete.ps1` 작성
- 필수 점검 항목:
  - `render` 외부 `VtkViewer::Instance()` 호출 0건
  - render 포트/adapter 핵심 파일 존재
  - 새 public API Doxygen 주석 존재
  - Phase 7/8 보호 규칙 훼손 여부
  - `P9-BUG-01` 추적 로그 존재 여부

### 영향 파일(예상)
- `scripts/refactoring/check_phase9_render_boundary_complete.ps1`
- `docs/refactoring/phase9/logs/check_phase9_render_boundary_complete_latest.txt`

### 완료 기준
- 반복 실행 가능한 정적 게이트 PASS 상태 확보

## W6. 빌드/테스트/게이트 문서화 및 종료 판정
### 작업
- 실행 게이트:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase9_render_boundary_complete.ps1`
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
- `P9-BUG-01` 재검증 결과를 종료 문서에 반영
- 종료 문서 작성:
  - `docs/refactoring/phase9/dependency_gate_report.md`
  - `docs/refactoring/phase9/go_no_go_phase10.md`

### 로그 산출 경로
- `docs/refactoring/phase9/logs/check_phase9_render_boundary_complete_latest.txt`
- `docs/refactoring/phase9/logs/build_phase9_latest.txt`
- `docs/refactoring/phase9/logs/unit_test_phase9_latest.txt`
- `docs/refactoring/phase9/logs/smoke_phase9_latest.txt`
- `docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md`

### 완료 기준
- Phase 10 착수 판정 가능한 정적/동적 게이트 + 문서 패키지 확보

## 5. 완료 기준(DoD)

1. `render` 외부 `VtkViewer::Instance()` 호출 수가 0건이다.
2. `vtk_viewer.cpp`는 render infrastructure 구현체 역할로만 남는다.
3. structure/measurement/density/mesh/UI 경로의 viewer 직접 접근이 render 포트 호출로 치환된다.
4. 새 render public API에 Doxygen 주석이 적용된다.
5. `P9-BUG-01` 상태가 종료 문서에 해결 또는 이관으로 명시된다.
6. 정적 게이트 + 빌드 + C++ 테스트 + smoke 테스트가 모두 PASS 한다.
7. `dependency_gate_report.md`와 `go_no_go_phase10.md`가 작성된다.

## 6. 일정/커밋 전략

권장 일정(5~7일):
- Day 1: W0~W1
- Day 2: W2
- Day 3~4: W3
- Day 5: W4
- Day 6: W5
- Day 7: W6

커밋 단위(권장):
1. W0 기준선/이슈 로그 고정
2. W1 render 포트 계약 확장
3. W2 adapter 재편
4. W3 feature 호출 전환
5. W4 runtime/viewer 경계 정리
6. W5 정적 게이트 스크립트 + 로그
7. W6 빌드/테스트 로그 + 종료 문서

동기화 규칙:
- WBS 종료마다 로컬 커밋 생성
- Phase 9 종료 선언 직후 원격 push 수행

## 7. 리스크 및 대응

1. 리스크: render 경계 전환 중 카메라/오버레이 상호작용 회귀
   - 대응: W2~W4 단계마다 smoke 시나리오로 즉시 확인
2. 리스크: `mesh.cpp`/`atoms_template.cpp` 대량 호출 전환 중 누락
   - 대응: 파일별 호출 목록 기반 체크리스트로 전환 누락 점검
3. 리스크: `P9-BUG-01`이 render 이슈가 아닌 import/workspace 이슈일 가능성
   - 대응: W6에서 원인 귀속을 명시하고 Phase 10 이관 항목으로 승격
4. 리스크: sandbox 권한 제약으로 게이트 실행 실패
   - 대응: 권한 상승 실행 경로를 표준화하고 로그 문서에 실행 경로 기록

## 8. 참조 문서
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- `docs/refactoring/phase8/refactoring_phase8_atoms_template_dismantle_260401.md`
- `docs/refactoring/phase8/dependency_gate_report.md`
- `docs/refactoring/phase8/go_no_go_phase9.md`

## 9. 진행 체크리스트
- [ ] W0 기준선/재현 로그 고정
- [ ] W1 render 포트 계약 확장
- [ ] W2 render adapter 재편
- [ ] W3 feature 호출 경로 전환
- [ ] W4 runtime/viewer 경계 정리
- [ ] W5 정적 게이트 도입
- [ ] W6 빌드/테스트 + 종료 판정 문서화
