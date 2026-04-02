# Phase 9 세부 작업계획서(재착수): `VtkViewer` 해체 + render 포트 완성

작성일: `2026-04-02 (KST)`  
최종 업데이트: `2026-04-02 (KST, W1 render 포트 계약 확장)`  
기준 계획: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (Phase 9 절)  
선행 판정: **GO** (`docs/refactoring/phase8/go_no_go_phase9.md`)  
대상 범위: `webassembly/src/render/*`, `webassembly/src/vtk_viewer.*`, `webassembly/src/atoms/*`, `webassembly/src/mesh*`, `webassembly/src/toolbar.cpp`, `webassembly/src/file_loader.cpp`, `webassembly/src/shell/runtime/*`, `scripts/refactoring/*`, `docs/refactoring/phase9/*`  
진행 상태: `W0~W1 완료, W2 착수 대기`

## 0. 재착수 배경과 운영 원칙

### 0.1 재착수 배경
- 리팩토링 진행 중 VASP 파일 로딩 관련 오류가 재발하여, Phase 9부터 다시 수행한다.
- 추적 이슈는 `docs/refactoring/phase8/go_no_go_phase9.md`에 `P9-BUG-01`로 등록되어 있다.

### 0.2 재착수 범위 해석
1. 이번 Phase 9는 **render boundary 완성**을 기본 목표로 유지한다.
2. 동시에 `P9-BUG-01`을 필수 추적 항목으로 포함해 W0~W6 전 과정에서 상태를 고정한다.
3. 해결 실패 시에도 W6에서 원인 귀속과 이관 판단 근거를 문서로 남긴다.

### 0.3 보호 규칙(반드시 유지)
1. 폰트/아이콘 회귀 방지
   - `main.cpp`에서 `ImGui::CreateContext()` 이후 `FontRegistry()` 초기화
   - `PrimeLegacySingletons()`에 `FontRegistry()` 재도입 금지
2. XSF bootstrap 가드 유지
   - `ChargeDensityUI::setGridDataEntries()`의 첫 grid bootstrap 유지
3. runtime quarantine 유지
   - 신규 singleton 도입 금지
4. render 경계 규칙 유지
   - `render` 외부 `VtkViewer::Instance()` 호출 금지

## 1. 착수 기준선 (W0에서 재실측 고정)

재착수 시점 코드 기준선은 W0에서 다시 실측해 고정한다.  
아래 항목을 `docs/refactoring/phase9/logs/render_inventory_phase9_latest.md`에 기록한다.

| 항목 | 측정 대상 | 목표 |
|---|---|---|
| `VtkViewer::Instance()` 전체 호출 수 | `webassembly/src/**/*.cpp,*.h` | 변화량 추적 |
| `render` 외부 `VtkViewer::Instance()` 호출 수 | 동일 | W6 시점 0 |
| 상위 호출 파일 분포 | `atoms_template.cpp`, `mesh.cpp`, `toolbar.cpp`, `measurement_controller.cpp` 등 | 우선 전환 목록 고정 |
| `vtk_viewer.cpp/.h` 라인 수 | 파일 라인 수 | 축소 추세 기록 |
| `render_gateway`/`vtk_render_gateway` 파일 존재 및 API 상태 | 파일/심볼 검사 | 계약 완성 |
| `P9-BUG-01` 재현 결과 | 수동/자동 체크리스트 | W6 해결 또는 이관 판정 |

`P9-BUG-01` 재현 로그 파일:
- `docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md`

## 2. Phase 9 목표/비목표

### 목표
1. `render` 외부 `VtkViewer::Instance()` 호출을 `0건`으로 만든다.
2. viewer 책임을 `viewport/camera/picking/overlay/scene` 포트 계약으로 분리한다.
3. feature 계층(`atoms/mesh/ui/shell`)의 viewer 직접 접근을 render 포트 호출로 전환한다.
4. 새 render public API에 Doxygen 주석을 적용한다.
5. `P9-BUG-01`의 원인 귀속을 고정하고 해결/이관을 W6 문서에서 명시한다.

### 비목표
1. `MeshManager`/`FileLoader` 완전 해체(Phase 10 범위)
2. compatibility facade 최종 제거(Phase 12 범위)
3. workspace 상태모델 완전 단일화(Phase 10 범위)

## 3. 설계 원칙

1. `VtkViewer` 접근 지점은 `render/infrastructure`로만 제한한다.
2. feature/application/domain 계층에서 `vtk*` include 확대를 금지한다.
3. actor/overlay/volume lifecycle은 render 포트 계약으로만 제어한다.
4. `P9-BUG-01`은 증상 기록이 아니라 **재현 절차 + 원인 가설 + 판정 결과**를 함께 기록한다.
5. WBS 단위 로컬 커밋 후 Phase 종료 선언 이후에만 원격 push를 수행한다.

## 4. 작업 단위(WBS)

## W0. 기준선 재고정 + `P9-BUG-01` 재현 로그 확정
### 작업
- `VtkViewer::Instance()` 전체/외부 호출 분포 재실측
- `P9-BUG-01` 재현 절차 고정:
  - `XSF(Grid) -> VASP -> XSF(Grid) -> VASP`
  - 각 단계별 기대 상태(이전 grid 정리 여부, Model Tree 반영 여부) 명시
- 정적 게이트 기준치 확정(`render 외부 호출 0`)

### 산출물
- `docs/refactoring/phase9/logs/render_inventory_phase9_latest.md`
- `docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md`

### 완료 기준
- 이후 W1~W6 결과를 비교 가능한 기준선이 문서로 고정됨

## W1. render 포트 계약 확장
### 작업
- `render/application/render_gateway.h`에 camera/picking/overlay/scene 계약을 명확화
- feature에서 필요한 최소 API만 노출
- 신규/변경 public API Doxygen 주석 적용

### 영향 파일(예상)
- `webassembly/src/render/application/render_gateway.h`
- `webassembly/src/render/application/camera_align_service.cpp`
- `webassembly/src/render/application/viewer_interaction_controller.cpp`

### 완료 기준
- feature 계층이 참조할 render 계약이 헤더에 명시되고 주석화됨

## W2. render infrastructure adapter 재편
### 작업
- `vtk_render_gateway`를 viewer 접근 단일 어댑터로 수렴
- actor/overlay/volume/camera 구현 소유권을 infrastructure로 고정
- `VtkViewer` 직접 호출 경로를 adapter 내부로 집중

### 영향 파일(예상)
- `webassembly/src/render/infrastructure/vtk_render_gateway.h`
- `webassembly/src/render/infrastructure/vtk_render_gateway.cpp`
- `webassembly/src/vtk_viewer.cpp`
- `webassembly/src/vtk_viewer.h`

### 완료 기준
- viewer 접근 경로가 render infrastructure 중심으로 정리됨

## W3. feature 호출 경로 전환 (atoms/mesh/ui 중심)
### 작업
- `atoms/mesh/toolbar/file_loader/runtime`의 viewer 직접 접근을 render 포트 호출로 전환
- overlay 갱신, camera 조정, render 요청 경로를 render service로 위임
- `P9-BUG-01` 관련 clear/갱신 누락 지점 1차 점검

### 영향 파일(예상)
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/application/measurement_controller.cpp`
- `webassembly/src/atoms/application/visibility_service.cpp`
- `webassembly/src/mesh.cpp`
- `webassembly/src/mesh_manager.cpp`
- `webassembly/src/toolbar.cpp`
- `webassembly/src/file_loader.cpp`
- `webassembly/src/shell/runtime/workbench_runtime.cpp`

### 완료 기준
- 전환 대상 파일의 `VtkViewer::Instance()` 직접 호출 제거/경유화 완료

## W4. `P9-BUG-01` 원인 귀속/수정 + 런타임 경계 정리
### 작업
- `P9-BUG-01` 분석:
  - import apply/rollback 순서
  - grid 컨텍스트 clear 타이밍
  - render actor detach/attach 누락 여부
- 수정 적용 또는 원인 귀속 고정
- runtime/shell에서 viewer 제어는 render 포트만 사용하도록 최종 정리

### 영향 파일(예상)
- `webassembly/src/io/application/import_apply_service.cpp`
- `webassembly/src/io/application/import_orchestrator.cpp`
- `webassembly/src/atoms/ui/charge_density_ui.cpp`
- `webassembly/src/shell/runtime/workbench_runtime.cpp`
- `docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md`

### 완료 기준
- `P9-BUG-01`에 대한 수정 또는 이관 근거가 재현 로그와 함께 고정됨

## W5. 정적 게이트 스크립트 도입
### 작업
- `scripts/refactoring/check_phase9_render_boundary_complete.ps1` 작성/갱신
- 필수 점검 항목:
  1. `render` 외부 `VtkViewer::Instance()` 호출 0건
  2. render 포트/adapter 핵심 파일 존재
  3. render public API Doxygen 주석 존재
  4. 폰트 초기화/XSF bootstrap 보호 규칙 유지
  5. `P9-BUG-01` 추적 로그 존재 + 상태 태그(`Open/Resolved/Deferred`) 명시

### 산출물
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
- 종료 문서 작성:
  - `docs/refactoring/phase9/dependency_gate_report.md`
  - `docs/refactoring/phase9/go_no_go_phase10.md`
- `P9-BUG-01` 최종 판정 반영:
  - 해결: 재현 절차 PASS 결과 첨부
  - 미해결: 원인 가설/재현 로그/차기 이관 경로 명시

### 로그 산출 경로
- `docs/refactoring/phase9/logs/check_phase9_render_boundary_complete_latest.txt`
- `docs/refactoring/phase9/logs/build_phase9_latest.txt`
- `docs/refactoring/phase9/logs/unit_test_phase9_latest.txt`
- `docs/refactoring/phase9/logs/smoke_phase9_latest.txt`
- `docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md`

### 완료 기준
- Phase 10 착수 판정 가능한 정적/동적 게이트 + 추적 문서 패키지 확보

## 5. 완료 기준(DoD)

1. `render` 외부 `VtkViewer::Instance()` 호출 수가 0건이다.
2. `vtk_viewer.cpp`는 render infrastructure 구현체 역할로 제한된다.
3. feature 경로의 viewer 직접 접근이 render 포트 호출로 치환된다.
4. render public API에 Doxygen 주석이 적용된다.
5. `P9-BUG-01` 상태가 종료 문서에 해결/이관 중 하나로 명시된다.
6. 정적 게이트 + 빌드 + C++ 테스트 + smoke 테스트가 모두 PASS한다.
7. `dependency_gate_report.md`, `go_no_go_phase10.md`가 작성된다.

## 6. 일정/커밋 전략

권장 일정(5~7일):
- Day 1: W0~W1
- Day 2: W2
- Day 3~4: W3~W4
- Day 5: W5
- Day 6~7: W6

권장 커밋 단위:
1. W0 기준선/재현 로그 고정
2. W1 render 포트 계약 확장
3. W2 adapter 재편
4. W3 feature 호출 전환
5. W4 `P9-BUG-01` 처리 + runtime 경계 정리
6. W5 정적 게이트 스크립트 + 로그
7. W6 빌드/테스트 로그 + 종료 문서

동기화 규칙:
- WBS 종료마다 로컬 커밋
- Phase 9 종료 선언 직후 원격 push 수행

## 7. 리스크 및 대응

1. 리스크: render 경계 전환 중 camera/overlay 상호작용 회귀
   - 대응: W2~W4 단계별 smoke + 수동 시나리오 확인
2. 리스크: 대량 호출 전환 중 누락
   - 대응: 파일별 호출 목록 기반 체크리스트 운용
3. 리스크: `P9-BUG-01`이 render 이슈가 아니라 import/workspace 이슈일 가능성
   - 대응: W4에서 귀속을 문서화하고 W6에 최종 판정
4. 리스크: sandbox 권한 제약으로 테스트 실패
   - 대응: 권한 상승 실행 경로를 문서/로그에 명시

## 8. 참조 문서
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- `docs/refactoring/phase8/go_no_go_phase9.md`
- `docs/refactoring/phase8/dependency_gate_report.md`
- `docs/refactoring/phase8/refactoring_phase8_atoms_template_dismantle_260401.md`

## 9. 진행 체크리스트
- [x] W0 기준선/재현 로그 고정
- [x] W1 render 포트 계약 확장
- [ ] W2 render adapter 재편
- [ ] W3 feature 호출 경로 전환
- [ ] W4 `P9-BUG-01` 처리 + runtime 경계 정리
- [ ] W5 정적 게이트 도입
- [ ] W6 빌드/테스트 + 종료 판정 문서화


