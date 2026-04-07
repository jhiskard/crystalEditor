# Phase 15 세부 작업계획서: Render/Platform Port Isolation 완결

작성일: `2026-04-07 (KST)`  
기준 계획:
- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (3. 목표 아키텍처)
선행 판정:
- `docs/refactoring/phase14/go_no_go_phase15.md` (`GO`)
대상 범위:
- `webassembly/src/render/*`
- `webassembly/src/atoms/*`
- `webassembly/src/mesh/*`
- `webassembly/src/io/*`
- `webassembly/src/platform/*`
- `webassembly/src/shell/*`
- `scripts/refactoring/*`
- `docs/refactoring/phase15/*`
진행 상태: `W6 완료`

## 0. 착수 배경과 고정 전제

### 0.1 착수 배경
1. Phase 14 종료로 `legacy alias 0`, `friend class 0`, 상태 소유권 경로 정리가 완료되었다.
2. 전체계획서 기준 차기 핵심 과제는 `3.6 포트/어댑터 계약` 완결이며, 비-render 계층의 VTK actor/volume 직접 조작 제거가 필요하다.
3. `platform` 경계는 현재 `platform/wasm`만 존재하고 browser/worker/persistence 어댑터가 `io/platform` 등에 분산되어 있어 재정렬이 필요하다.

### 0.2 고정 전제
1. `WorkbenchRuntime` 중심 composition root 구조는 유지한다.
2. 메뉴/레이아웃/import/measurement의 기존 사용자 기능 동작은 유지한다.
3. 공통 정책 `4.4 UI 상태 동기화 정책`을 유지하고 W6 메뉴 매트릭스 점검을 필수로 수행한다.
4. `P9-BUG-01`은 Phase 15에서도 추적 상태를 유지한다.

## 1. 목표와 비목표

### 1.1 목표
1. 비-render 계층의 `vtkActor`/`vtkVolume` 직접 조작 경로를 RenderPort 경유로 축소/격리한다.
2. `render/infrastructure` 직접 include를 비-render 계층에서 제거하고 `render/application` 계약만 사용하도록 정리한다.
3. `io/platform`에 있는 browser adapter 경로를 `platform/browser` 중심으로 재배치할 준비를 완료한다.
4. camera/picking/overlay 관련 포트 계약을 문서화하고 정적 게이트로 고정한다.

### 1.2 비목표
1. 최종 빌드 타깃 재편(`wb_atoms` 제거 포함, Phase 16 범위)
2. 전체 아키텍처 최종 봉인(Phase 17 범위)
3. UI 신규 기능 추가

## 2. W0 기준선(베이스라인)

W0에서 아래 지표를 고정한다.

| 항목 | 기준선(2026-04-07) | Phase 15 목표 |
|---|---:|---:|
| `vtkActor/vtkVolume` 토큰 참조(`*.cpp/*.h`) | 234 | 감소 추세 확보 + render 경계 집중 |
| 비-render 범위(`webassembly/src/render/*` 제외) `vtkActor/vtkVolume` 참조 | 201 | 단계별 감소 |
| 비-render 범위 직접 조작 토큰(`Add/RemoveActor`, `Add/RemoveVolume`, `SetMapper`, `GetProperty`) | 172 | 단계별 감소 |
| `render/infrastructure` include 전체 참조 | 1 | 0 |
| `render/infrastructure` include의 render 외부 참조 | 1 | 0 |
| `webassembly/src/platform` 하위 디렉터리 | `wasm`만 존재 | `browser/worker/persistence` 경계 설계 반영 |
| `P9-BUG-01` 상태 | Deferred | Resolved 또는 근거 있는 Deferred |

W0 산출 로그:
- `docs/refactoring/phase15/logs/render_port_inventory_phase15_latest.md`
- `docs/refactoring/phase15/logs/platform_boundary_inventory_phase15_latest.md`
- `docs/refactoring/phase15/logs/bug_p15_vasp_grid_sequence_latest.md`

## 3. 설계 원칙

1. RenderPort 단일 진입 원칙:
   - 비-render 계층의 actor/volume 제어는 `render::application::RenderGateway` 계약으로만 수행한다.
   - `render/infrastructure` 타입/구현 의존은 render 모듈 내부로 제한한다.
2. 계층별 점진 분리 원칙:
   - `atoms/infrastructure -> mesh/io/ui -> platform` 순으로 격리한다.
   - 중간 단계 호환 경로는 제거 목표 Phase와 추적 ID를 남긴다.
3. 플랫폼 경계 재정렬 원칙:
   - browser/worker/persistence adapter 위치를 `platform/*` 중심으로 정렬한다.
   - runtime 및 feature 서비스는 adapter 위치 변경과 무관하게 계약을 유지한다.

## 4. 작업 단위(WBS)

### W0. Render/Platform 기준선 고정 + 버그 추적 이관
- 비-render VTK 토큰/직접 조작 경로 계측
- platform 디렉터리/adapter 분포 계측
- `P9-BUG-01` 로그를 Phase 15로 이관

산출물:
- `logs/render_port_inventory_phase15_latest.md`
- `logs/platform_boundary_inventory_phase15_latest.md`
- `logs/bug_p15_vasp_grid_sequence_latest.md`

완료 기준:
- W1~W6 비교 가능한 기준선 문서화 완료

### W1. `atoms/infrastructure` render 책임 이동
- `atoms/infrastructure` 내부 actor/volume 직접 조작 지점을 RenderPort 중심으로 축소
- 필요 시 렌더링 책임을 `render/*`로 이관

예상 영향 파일:
- `webassembly/src/atoms/infrastructure/*`
- `webassembly/src/atoms/atoms_template.*`
- `webassembly/src/render/*`

완료 기준:
- `atoms/infrastructure`의 render 직접 조작 경로가 baseline 대비 감소

### W2. `mesh/io/ui` actor/volume 접근을 RenderPort 경유로 치환
- `mesh`, `io`, `shell/ui`에서 actor/volume 직접 조작 경로 제거
- 서비스/컨트롤러 계층이 render 계약만 참조하도록 정리

예상 영향 파일:
- `webassembly/src/mesh/*`
- `webassembly/src/io/*`
- `webassembly/src/shell/*`
- `webassembly/src/app.cpp`

완료 기준:
- 비-render 직접 조작 토큰 수 baseline 대비 감소 + 신규 직접 조작 0

### W3. `platform/wasm`, `platform/persistence` 경계 정리
- `io/platform/browser_file_picker.*`의 `platform/browser` 이동 또는 동등 경계 정렬
- `platform/persistence` 경계 설계와 연결 지점 정리

예상 영향 파일:
- `webassembly/src/io/platform/*`
- `webassembly/src/platform/*`
- `webassembly/src/file_loader.*`
- `webassembly/src/shell/runtime/workbench_runtime.*`

완료 기준:
- browser adapter 경로가 `platform` 중심으로 정렬되고 runtime 진입 계약 유지

### W4. camera/picking/overlay 계약 문서화
- `render/application` 계약에서 camera/picking/overlay 경계를 명시
- non-render 계층 호출 규칙(허용 API/금지 API) 문서화

예상 영향 파일:
- `webassembly/src/render/application/*`
- `webassembly/src/render/infrastructure/*`
- `docs/refactoring/phase15/logs/*`

완료 기준:
- 계약 문서와 코드 주석이 일치하고 호출 규칙이 검증 가능 상태

### W5. 정적 게이트 스크립트 도입
- `scripts/refactoring/check_phase15_render_platform_isolation.ps1` 작성
- 필수 체크:
  1. 비-render의 `render/infrastructure` include 0
  2. 금지된 actor/volume 직접 조작 패턴의 신규 유입 0
  3. `platform` 경계 문서/로그 존재
  4. 메뉴 오픈 회귀 코드 경로 보존(정책 4.4)
  5. bug 추적 로그 존재 + 상태 태그

산출물:
- `scripts/refactoring/check_phase15_render_platform_isolation.ps1`
- `docs/refactoring/phase15/logs/check_phase15_render_platform_isolation_latest.txt`

완료 기준:
- 반복 실행 시 PASS

### W6. 빌드/테스트/게이트 + 종료 문서
- 실행 게이트:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase15_render_platform_isolation.ps1`
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
- 추가 검증(필수):
  - 메뉴 창 오픈 매트릭스
    - `Edit > Atoms | Bonds | Cell`
    - `Build > Add atoms | Bravais Lattice Templates`
    - `Data > Isosurface | Surface | Volumetric | Plane` (grid 로드 상태 포함)
    - `Utilities > Brillouin Zone`
- 종료 문서:
  - `docs/refactoring/phase15/dependency_gate_report.md`
  - `docs/refactoring/phase15/go_no_go_phase16.md`

로그 산출 경로:
- `docs/refactoring/phase15/logs/check_phase15_render_platform_isolation_latest.txt`
- `docs/refactoring/phase15/logs/build_phase15_latest.txt`
- `docs/refactoring/phase15/logs/unit_test_phase15_latest.txt`
- `docs/refactoring/phase15/logs/smoke_phase15_latest.txt`
- `docs/refactoring/phase15/logs/menu_open_matrix_phase15_latest.md`

완료 기준:
- W6 게이트 전 항목 PASS

## 5. 완료 기준(DoD)

1. 비-render 계층의 `render/infrastructure` include가 0이다.
2. 비-render actor/volume 직접 조작 경로가 baseline 대비 감소하고 신규 유입이 0이다.
3. platform/browser/worker/persistence 경계 정렬 계획이 코드/문서에 반영된다.
4. 메뉴 창 오픈 매트릭스가 PASS다.
5. 정적 게이트/빌드/C++ 테스트/스모크 테스트가 모두 PASS다.
6. `P9-BUG-01` 추적 로그가 최신 상태로 유지된다.

## 6. 주석 정책 적용 메모 (v2)

1. public API 변경 시 `@brief` + 계약(`@details` 또는 `@note`) 필수
2. 임시 호환 경로는 아래를 함께 기록
   - `@note Temporary compatibility path`
   - 제거 목표 Phase (`remove in Phase XX`)
   - 추적 ID (`ARCH-DEBT-XXX`)

## 7. Git 운영 정책 적용 메모 (v2)

1. 작업 브랜치: `refactor/phase15-render-port-isolation`
2. WBS 단위 로컬 커밋, 메시지에 `phase15 Wn` 포함
3. 원격 push는 Phase 15 완료 선언 이후에 수행
4. 종료 후 회귀 발생 시 `hotfix/phase15-*` 브랜치로 분리

## 8. 리스크와 대응

| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| P15-R1 | render 책임 이동 중 actor lifetime 회귀 | High | RenderPort 단일 진입 강제 + 단계별 회귀 테스트 |
| P15-R2 | platform 재배치 중 import 진입 경로 파손 | High | adapter 경계 테스트 + runtime wrapper 계약 유지 |
| P15-R3 | 메뉴 창 오픈 동기화 회귀 재발 | Medium | 정책 4.4 유지 + W6 메뉴 매트릭스 필수 |
| P15-R4 | `P9-BUG-01` 장기 이월 | Medium | W6 전용 시퀀스 근거 강화 및 상태 재판정 |

## 9. 참조 문서

- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/phase14/refactoring_phase14_state_ownership_alias_zero_260406.md`
- `docs/refactoring/phase14/dependency_gate_report.md`
- `docs/refactoring/phase14/go_no_go_phase15.md`
- `docs/refactoring/phase14/logs/menu_open_matrix_phase14_latest.md`

## 10. 진행 체크리스트

- [x] W0 기준선/중간 산출문서 등록
- [x] W1 `atoms/infrastructure` render 책임 이동
- [x] W2 `mesh/io/ui` RenderPort 치환
- [x] W3 `platform` 경계 정리
- [x] W4 camera/picking/overlay 계약 문서화
- [x] W5 정적 게이트 스크립트 도입
- [x] W6 빌드/테스트/종료 문서 완료

## 11. W0~W6 실행 결과 (`2026-04-07`, KST)

1. W0 기준선/추적 로그 등록 완료:
   - `docs/refactoring/phase15/logs/render_port_inventory_phase15_latest.md`
   - `docs/refactoring/phase15/logs/platform_boundary_inventory_phase15_latest.md`
   - `docs/refactoring/phase15/logs/bug_p15_vasp_grid_sequence_latest.md`
2. W1 완료:
   - `atoms/infrastructure`의 반복 `SetMapper/GetProperty` 직접 조작 경로를 helper 기반 호출로 축소.
   - 대상 파일:
     - `webassembly/src/atoms/infrastructure/vtk_renderer.cpp`
     - `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp`
3. W2 완료:
   - `mesh` 계층의 반복 `SetMapper/GetProperty` 직접 조작 경로를 helper 기반으로 치환.
   - `shell/runtime`의 `render/infrastructure` 직접 include 제거(`render/application` 브리지 경유).
   - 대상 파일:
     - `webassembly/src/mesh.cpp`
     - `webassembly/src/shell/runtime/workbench_runtime.cpp`
     - `webassembly/src/render/application/legacy_viewer_facade.h`
     - `webassembly/src/render/application/legacy_viewer_facade.cpp`
     - `webassembly/cmake/modules/wb_render.cmake`
4. 정량 지표(W2 재계측):
   - 비-render 직접 조작 토큰: `172 -> 110` (`-62`)
   - `render/infrastructure` include render 외 참조: `1 -> 0`
   - 세부 수치는 `docs/refactoring/phase15/logs/render_port_inventory_phase15_latest.md` 반영.
5. 중간 빌드 검증:
   - `npm run build-wasm:release` PASS
6. W3 완료:
   - browser adapter를 `io/platform`에서 `platform/browser`로 이동하고 namespace를 `platform::browser`로 정렬.
   - 대상 파일:
     - `webassembly/src/platform/browser/browser_file_picker.h`
     - `webassembly/src/platform/browser/browser_file_picker.cpp`
     - `webassembly/src/file_loader.cpp`
     - `webassembly/cmake/modules/wb_io.cmake`
     - `docs/refactoring/phase15/logs/platform_boundary_inventory_phase15_latest.md`
7. W4 완료:
   - camera/picking/overlay RenderPort 계약을 코드 주석과 산출문서로 명문화.
   - 대상 파일:
     - `webassembly/src/render/application/render_gateway.h`
     - `webassembly/src/render/infrastructure/vtk_render_gateway.h`
     - `docs/refactoring/phase15/logs/render_camera_picking_overlay_contract_phase15_latest.md`
8. W5 완료:
   - 정적 게이트 스크립트 추가:
     - `scripts/refactoring/check_phase15_render_platform_isolation.ps1`
   - 체크 항목:
     - non-render `render/infrastructure` include 0
     - non-render 직접 조작 신규 유입 차단(Phase 15 W2 snapshot 상한 유지)
     - platform 경계 코드/문서 존재 + legacy `io/platform` 경로 제거
     - 메뉴 오픈 회귀 코드 경로 보존
     - bug 추적 로그 존재 + 상태 태그
9. W6 완료:
   - 실행 게이트 결과:
     - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase15_render_platform_isolation.ps1` PASS
     - `npm run build-wasm:release` PASS
     - `npm run test:cpp` PASS
     - `npm run test:smoke` PASS
   - 산출 로그:
     - `docs/refactoring/phase15/logs/check_phase15_render_platform_isolation_latest.txt`
     - `docs/refactoring/phase15/logs/build_phase15_latest.txt`
     - `docs/refactoring/phase15/logs/unit_test_phase15_latest.txt`
     - `docs/refactoring/phase15/logs/smoke_phase15_latest.txt`
     - `docs/refactoring/phase15/logs/menu_open_matrix_phase15_latest.md`
   - 종료 문서:
     - `docs/refactoring/phase15/dependency_gate_report.md` 최종 반영
     - `docs/refactoring/phase15/go_no_go_phase16.md` 최종 반영 (`GO`)
