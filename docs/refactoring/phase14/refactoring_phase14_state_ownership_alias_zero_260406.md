# Phase 14 세부 작업계획서: State Ownership 완결 + Legacy Alias 0화

작성일: `2026-04-06 (KST)`  
기준 계획:
- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (3. 목표 아키텍처)
선행 판정:
- `docs/refactoring/phase13/go_no_go_phase14.md` (`GO`)
대상 범위:
- `webassembly/src/atoms/*`
- `webassembly/src/structure/*`
- `webassembly/src/measurement/*`
- `webassembly/src/density/*`
- `webassembly/src/shell/*` (메뉴/창 오픈 회귀 방지 체크 포함)
- `scripts/refactoring/*`
- `docs/refactoring/phase14/*`
진행 상태: `W0 착수`

## 0. 착수 배경과 고정 전제

### 0.1 착수 배경
1. Phase 13에서 `App/Toolbar/FileLoader` singleton entrypoint 정리는 완료되었고, Phase 14 착수 조건은 `GO`로 확정되었다.
2. 목표 아키텍처의 잔여 핵심 과제는 legacy alias(`createdAtoms/createdBonds/cellInfo`) 제거와 상태 소유권의 repository/service 경유 통일이다.
3. Phase 13 종료 후 확인된 메뉴 기반 창 오픈 회귀 이슈는 수정되었으며, Phase 14부터는 이를 W6 필수 검증으로 고정한다.

### 0.2 고정 전제
1. `WorkbenchRuntime` 중심 composition root 구조는 유지한다.
2. 기존 기능 동작(구조 편집/빌드/데이터/유틸리티 메뉴 동작, import 흐름)은 유지하면서 상태 소유권 경계를 정리한다.
3. 공통 정책 `4.4 UI 상태 동기화 정책`을 적용한다.
4. `P9-BUG-01`은 Phase 14에서 추적 상태를 유지하며 자동 재현 범위를 확대한다.

## 1. 목표와 비목표

### 1.1 목표
1. legacy alias(`atoms::domain::createdAtoms|createdBonds|cellInfo`) 참조를 0으로 만든다.
2. 상태 변경 경로를 repository/service API로 통일하고 direct global 상태 접근을 제거한다.
3. `friend class` 선언을 0으로 만든다(특히 `lcrs_tree.h` 포함).
4. 메뉴 기반 창 오픈 회귀 체크(`Edit/Build/Data/Utilities`)를 Phase 14 게이트에 포함한다.

### 1.2 비목표
1. 신규 UI 기능 추가
2. render/platform 포트 격리 대규모 변경(Phase 15 범위)
3. 최종 빌드 타깃 재편(`wb_atoms` 제거 포함, Phase 16 범위)

## 2. W0 기준선(베이스라인)

W0에서 아래 지표를 고정한다.

| 항목 | 기준선(2026-04-06) | Phase 14 목표 |
|---|---:|---:|
| legacy alias 전체 참조 수 | 14 | 0 |
| legacy alias 외부 노출(`atoms/*` 외) | 0 | 0 유지 |
| `friend class` 선언 수 | 4 | 0 |
| `lcrs_tree.h`의 `friend class` | 1 | 0 |
| 메뉴 창 오픈 회귀 체크 | 미자동화/수동 확인 | W6 필수 게이트로 고정 |
| `P9-BUG-01` 상태 | Deferred | Resolved 또는 근거 있는 Deferred |

W0 산출 로그:
- `docs/refactoring/phase14/logs/alias_inventory_phase14_latest.md`
- `docs/refactoring/phase14/logs/friend_class_inventory_phase14_latest.md`
- `docs/refactoring/phase14/logs/bug_p14_vasp_grid_sequence_latest.md`

## 3. 설계 원칙

1. 상태 소유권 원칙:
   - 상태 조회/변경은 repository/service 경유로만 허용한다.
   - UI/adapter 계층에서 domain 전역 상태 직접 접근을 금지한다.
2. 점진 치환 원칙:
   - `atoms/domain -> atoms/application -> atoms/ui` 순서로 치환한다.
   - 중간 단계에서 호환 경로가 필요하면 명시적 주석과 제거 Phase를 기록한다.
3. 메뉴 동기화 원칙:
   - 메뉴 액션 프레임에서 `ShellState`와 local cache가 불일치하지 않도록 유지한다.
   - W6에서 메뉴 창 오픈 매트릭스를 반드시 검증한다.

## 4. 작업 단위(WBS)

### W0. alias/friend 기준선 고정 + 버그 추적 이관
- alias 참조 분포 재수집(파일/계층별)
- `friend class` 분포 재수집
- `P9-BUG-01` 추적 로그를 Phase 14로 이관

산출물:
- `logs/alias_inventory_phase14_latest.md`
- `logs/friend_class_inventory_phase14_latest.md`
- `logs/bug_p14_vasp_grid_sequence_latest.md`

완료 기준:
- W1~W6 비교 가능한 기준선 문서화 완료

### W1. `atoms/domain` 우선 치환
- domain 계층의 legacy alias 참조 제거
- 상태 소유권을 repository API 경유로 정리

예상 영향 파일:
- `webassembly/src/atoms/domain/*`
- `webassembly/src/structure/domain/*`

완료 기준:
- `atoms/domain` 내 legacy alias 참조 0

### W2. `atoms/application` 치환 + read model 경로 정리
- application 계층의 alias 참조 제거
- read model/command/query 경로를 service 중심으로 정리

예상 영향 파일:
- `webassembly/src/atoms/application/*`
- `webassembly/src/structure/application/*`
- `webassembly/src/measurement/application/*`
- `webassembly/src/density/application/*`

완료 기준:
- `atoms/application` 내 legacy alias 참조 0

### W3. `atoms/ui` 치환 + compatibility facade 축소
- UI 계층 alias 접근 제거
- 필요 시 facade 경로 축소/정리

예상 영향 파일:
- `webassembly/src/atoms/ui/*`
- `webassembly/src/atoms/atoms_template.*`
- `webassembly/src/app.cpp` (메뉴/창 오픈 경로 정합성 유지)

완료 기준:
- UI 계층 alias 직접 참조 0

### W4. `friend class` 0건화
- `lcrs_tree.h` 포함 `friend class` 제거
- runtime 소유 객체(`app/file_loader/toolbar`)의 friend 의존 제거 또는 대체 생성 경로 확정

예상 영향 파일:
- `webassembly/src/lcrs_tree.h`
- `webassembly/src/app.h`
- `webassembly/src/file_loader.h`
- `webassembly/src/toolbar.h`
- `webassembly/src/shell/runtime/workbench_runtime.*`

완료 기준:
- `friend class` 선언 0

### W5. 정적 게이트 스크립트 도입
- `scripts/refactoring/check_phase14_state_ownership_alias_zero.ps1` 작성
- 필수 체크:
  1. legacy alias 참조 0
  2. alias 외부 노출 0 유지
  3. `friend class` 0
  4. 메뉴 창 오픈 회귀 체크 항목의 코드 경로 보존(정책 4.4)
  5. bug 추적 로그 존재 + 상태 태그

산출물:
- `scripts/refactoring/check_phase14_state_ownership_alias_zero.ps1`
- `docs/refactoring/phase14/logs/check_phase14_state_ownership_alias_zero_latest.txt`

완료 기준:
- 반복 실행 시 PASS

### W6. 빌드/테스트/게이트 + 종료 문서
- 실행 게이트:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase14_state_ownership_alias_zero.ps1`
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
  - `docs/refactoring/phase14/dependency_gate_report.md`
  - `docs/refactoring/phase14/go_no_go_phase15.md`

로그 산출 경로:
- `docs/refactoring/phase14/logs/check_phase14_state_ownership_alias_zero_latest.txt`
- `docs/refactoring/phase14/logs/build_phase14_latest.txt`
- `docs/refactoring/phase14/logs/unit_test_phase14_latest.txt`
- `docs/refactoring/phase14/logs/smoke_phase14_latest.txt`
- `docs/refactoring/phase14/logs/menu_open_matrix_phase14_latest.md`

완료 기준:
- W6 게이트 전 항목 PASS

## 5. 완료 기준(DoD)

1. legacy alias 참조가 코드 전체에서 0이다.
2. `friend class` 선언이 0이다.
3. 상태 변경이 repository/service 경유로만 동작한다.
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

1. 작업 브랜치: `refactor/phase14-alias-state-ownership`
2. WBS 단위 로컬 커밋, 메시지에 `phase14 Wn` 포함
3. 원격 push는 Phase 14 완료 선언 이후에 수행
4. 종료 후 회귀 발생 시 `hotfix/phase14-*` 브랜치로 분리

## 8. 리스크와 대응

| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| P14-R1 | alias 제거 중 상태 일관성 회귀 | High | repository/service 단위 검증 + 단계별 회귀 테스트 |
| P14-R2 | `friend class` 제거 중 생성/소유권 경로 파손 | High | 생성 경로 대체 설계 선확정 + W4 분리 적용 |
| P14-R3 | 메뉴 창 오픈 동기화 회귀 재발 | Medium | 정책 4.4 적용 + W6 메뉴 매트릭스 필수 체크 |
| P14-R4 | `P9-BUG-01` 장기 이월 | Medium | Phase 14에서 자동 재현 범위 확대 및 상태 재판정 |

## 9. 참조 문서

- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/phase13/refactoring_phase13_runtime_hard_seal_singleton_entrypoint_260406.md`
- `docs/refactoring/phase13/dependency_gate_report.md`
- `docs/refactoring/phase13/go_no_go_phase14.md`
- `docs/refactoring/phase13/logs/runtime_singleton_inventory_phase13_latest.md`

## 10. 진행 체크리스트

- [x] W0 기준선/중간 산출문서 등록
- [ ] W1 `atoms/domain` 치환
- [ ] W2 `atoms/application` 치환
- [ ] W3 `atoms/ui` 치환
- [ ] W4 `friend class` 0건화
- [ ] W5 정적 게이트 스크립트 도입
- [ ] W6 빌드/테스트/종료 문서 완료
