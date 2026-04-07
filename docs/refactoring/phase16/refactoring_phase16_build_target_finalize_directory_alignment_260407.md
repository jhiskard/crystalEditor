# Phase 16 세부 작업계획서: 최종 Build Target 전환 + 디렉터리 정렬

작성일: `2026-04-07 (KST)`  
기준 계획:
- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (3. 목표 아키텍처)
선행 판정:
- `docs/refactoring/phase15/go_no_go_phase16.md` (`GO`)
대상 범위:
- `CMakeLists.txt`
- `webassembly/cmake/modules/*`
- `webassembly/src/*`
- `scripts/refactoring/*`
- `docs/refactoring/phase16/*`
진행 상태: `W0 완료`

## 0. 착수 배경과 고정 전제

### 0.1 착수 배경
1. Phase 15 종료로 Render/Platform 경계 정리와 정적 게이트 체계가 고정되었고, Phase 16 착수 조건은 `GO`로 확정되었다.
2. 후속 전체계획서 기준 Phase 16의 핵심 과제는 `3.7 최종 빌드 타깃` 완결이며, `wb_atoms` 제거와 최종 모듈 타깃 세트 전환이 필요하다.
3. 목표 디렉터리 레이아웃 대비 누락 경로(`platform/worker`, `platform/persistence`, `shell/presentation`, `workspace/presentation`, `render/presentation`, `structure/infrastructure`)가 남아 있어 정렬 작업이 필요하다.

### 0.2 고정 전제
1. Phase 15에서 확정한 render 계약과 `platform/browser` 경계는 유지한다.
2. `WorkbenchRuntime` 중심 composition root 구조와 메뉴/레이아웃/import의 사용자 동작은 유지한다.
3. 공통 정책 `4.4 UI 상태 동기화 정책`을 유지하고 W6 메뉴 매트릭스 점검을 필수로 수행한다.
4. `P9-BUG-01`은 Phase 16에서도 추적 상태를 유지한다.

## 1. 목표와 비목표

### 1.1 목표
1. `wb_atoms` 타깃과 잔존 참조를 제거한다.
2. 목표 타깃 세트를 `wb_common`, `wb_platform`, `wb_shell`, `wb_workspace`, `wb_render`, `wb_mesh`, `wb_structure`, `wb_measurement`, `wb_density`, `wb_io`로 전환한다.
3. 모듈 소스 ownership를 타깃 책임 기준으로 재배치하고 단일 ownership 규칙을 고정한다.
4. 목표 디렉터리 레이아웃 누락 경로를 보완하고 정적 게이트로 봉인한다.

### 1.2 비목표
1. 최종 아키텍처 종료 선언(Phase 17 범위)
2. 신규 UI 기능 추가
3. `P9-BUG-01`의 최종 해결 확정(Phase 17 범위)

## 2. W0 기준선(베이스라인)

W0에서 아래 지표를 고정한다.

| 항목 | 기준선(2026-04-07) | Phase 16 목표 |
|---|---:|---:|
| `WB_MODULE_TARGETS` 모듈 수 | 6 (`wb_core`, `wb_render`, `wb_mesh`, `wb_atoms`, `wb_io`, `wb_ui`) | 10 |
| `wb_atoms` 참조 수(`CMakeLists.txt`, `webassembly/cmake/modules/*`) | 6 | 0 |
| `wb_atoms` 내 비-`atoms` 소스 수 | 4 | 0 |
| `wb_io` 내 `atoms/infrastructure` 소스 수 | 2 | 0 |
| 목표 디렉터리 레이아웃 존재 수 | 25 / 31 | 31 / 31 |
| `webassembly/src` 최상위 `.cpp/.h` 파일 수 | 40 (22 + 18) | 단계적 감소(모듈 하위 정렬) |
| `P9-BUG-01` 상태 | Deferred | Resolved 또는 근거 있는 Deferred |

W0 산출 로그:
- `docs/refactoring/phase16/logs/target_graph_inventory_phase16_latest.md`
- `docs/refactoring/phase16/logs/directory_layout_inventory_phase16_latest.md`
- `docs/refactoring/phase16/logs/bug_p16_vasp_grid_sequence_latest.md`

## 3. 설계 원칙

1. 타깃-책임 일치 원칙:
   - 타깃 이름과 소스 경계(`module/file ownership`)를 일치시킨다.
   - 한 소스 파일은 하나의 모듈 타깃에만 소속된다.
2. 비순환 링크 원칙:
   - 모듈 그래프는 DAG로 유지한다.
   - 의존성 방향은 `common/platform/workspace -> render/mesh -> structure/measurement/density/io -> shell`을 기준으로 관리한다.
3. 점진 분리 원칙:
   - `wb_atoms` 해체는 `structure/measurement/density` 우선 분리 후 제거한다.
   - 플랫폼/셸/워크스페이스 타깃은 기능 회귀 없이 경계만 우선 고정한다.

## 4. 작업 단위(WBS)

### W0. 타깃/디렉터리 기준선 고정 + 버그 추적 이관
- 모듈 타깃/소스 ownership 분포 계측
- 목표 디렉터리 레이아웃 존재 여부 계측
- `P9-BUG-01` 로그를 Phase 16로 이관

산출물:
- `logs/target_graph_inventory_phase16_latest.md`
- `logs/directory_layout_inventory_phase16_latest.md`
- `logs/bug_p16_vasp_grid_sequence_latest.md`

완료 기준:
- W1~W6 비교 가능한 기준선 문서화 완료

### W1. `wb_structure` / `wb_measurement` / `wb_density` 분리
- `wb_atoms`에 포함된 구조/측정/밀도 소스를 각 타깃으로 분리
- 타깃 정의 파일 추가:
  - `webassembly/cmake/modules/wb_structure.cmake`
  - `webassembly/cmake/modules/wb_measurement.cmake`
  - `webassembly/cmake/modules/wb_density.cmake`

예상 영향 파일:
- `CMakeLists.txt`
- `webassembly/cmake/modules/wb_atoms.cmake`
- `webassembly/cmake/modules/wb_structure.cmake` (신규)
- `webassembly/cmake/modules/wb_measurement.cmake` (신규)
- `webassembly/cmake/modules/wb_density.cmake` (신규)

완료 기준:
- `wb_atoms` 내 비-`atoms` 소스 0

### W2. `wb_platform` / `wb_shell` / `wb_workspace` 정렬 + `wb_common` 전환
- `wb_core`를 `wb_common`으로 정렬(명칭/역할 기준 통일)
- 플랫폼/셸/워크스페이스 타깃 정의 파일 추가 또는 재구성:
  - `webassembly/cmake/modules/wb_platform.cmake`
  - `webassembly/cmake/modules/wb_shell.cmake`
  - `webassembly/cmake/modules/wb_workspace.cmake`
- 목표 레이아웃 누락 디렉터리 경로 생성/정렬

예상 영향 파일:
- `CMakeLists.txt`
- `webassembly/cmake/modules/wb_core.cmake` (교체 또는 이관)
- `webassembly/cmake/modules/wb_ui.cmake`
- `webassembly/cmake/modules/wb_io.cmake`
- `webassembly/src/platform/*`
- `webassembly/src/shell/*`
- `webassembly/src/workspace/*`

완료 기준:
- `WB_MODULE_TARGETS`에 `wb_common`, `wb_platform`, `wb_shell`, `wb_workspace` 반영
- 누락 디렉터리 최소 4개 이상 보완

### W3. 링크 의존 재배치 + 비순환 고정
- 모듈 링크 의존을 목표 그래프로 재배치
- 실행 타깃 링크 목록을 최종 모듈 세트 기준으로 정렬
- 모듈 간 순환 의존 검사 및 차단

예상 영향 파일:
- `CMakeLists.txt`
- `webassembly/cmake/modules/*.cmake`
- `scripts/refactoring/*`

완료 기준:
- 모듈 그래프 비순환 PASS
- 중복/역방향 의존 제거

### W4. `wb_atoms` 제거 + 잔존 참조 제거
- `wb_atoms` 타깃 삭제
- `CMakeLists.txt`, 모듈 파일, 문서/스크립트의 `wb_atoms` 잔존 참조 제거
- `atoms` 관련 소스 ownership를 최종 타깃으로 이관 완료

예상 영향 파일:
- `CMakeLists.txt`
- `webassembly/cmake/modules/wb_atoms.cmake` (삭제)
- `webassembly/cmake/modules/*.cmake`
- `docs/refactoring/phase16/logs/*`

완료 기준:
- `wb_atoms` 참조 0

### W5. 정적 게이트 스크립트 도입
- `scripts/refactoring/check_phase16_target_graph_final.ps1` 작성
- 필수 체크:
  1. `wb_atoms` 참조 0
  2. 목표 모듈 타깃 세트 존재
  3. 모듈 타깃 그래프 비순환
  4. 소스 단일 ownership(중복 포함 0)
  5. 메뉴 오픈 회귀 코드 경로 보존(정책 4.4)
  6. bug 추적 로그 존재 + 상태 태그

산출물:
- `scripts/refactoring/check_phase16_target_graph_final.ps1`
- `docs/refactoring/phase16/logs/check_phase16_target_graph_final_latest.txt`

완료 기준:
- 반복 실행 시 PASS

### W6. 빌드/테스트/게이트 + 종료 문서
- 실행 게이트:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase16_target_graph_final.ps1`
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
  - `docs/refactoring/phase16/dependency_gate_report.md`
  - `docs/refactoring/phase16/go_no_go_phase17.md`

로그 산출 경로:
- `docs/refactoring/phase16/logs/check_phase16_target_graph_final_latest.txt`
- `docs/refactoring/phase16/logs/build_phase16_latest.txt`
- `docs/refactoring/phase16/logs/unit_test_phase16_latest.txt`
- `docs/refactoring/phase16/logs/smoke_phase16_latest.txt`
- `docs/refactoring/phase16/logs/menu_open_matrix_phase16_latest.md`

완료 기준:
- W6 게이트 전 항목 PASS

## 5. 완료 기준(DoD)

1. `wb_atoms` 참조가 코드/빌드 설정에서 0이다.
2. 목표 모듈 타깃 세트 10개가 `CMakeLists.txt`와 모듈 파일에 반영된다.
3. 모듈 링크 그래프가 비순환이고 실행 타깃 링크가 최종 세트로 정렬된다.
4. 목표 디렉터리 레이아웃 존재율이 baseline 대비 개선되고 누락 경로 보완이 반영된다.
5. 정적 게이트/빌드/C++ 테스트/스모크 테스트가 모두 PASS다.
6. `P9-BUG-01` 추적 로그가 최신 상태로 유지된다.

## 6. 주석 정책 적용 메모 (v2)

1. public API 변경 시 `@brief` + 계약(`@details` 또는 `@note`) 필수
2. 임시 호환 경로는 아래를 함께 기록
   - `@note Temporary compatibility path`
   - 제거 목표 Phase (`remove in Phase XX`)
   - 추적 ID (`ARCH-DEBT-XXX`)

## 7. Git 운영 정책 적용 메모 (v2)

1. 작업 브랜치: `refactor/phase16-build-target-finalize`
2. WBS 단위 로컬 커밋, 메시지에 `phase16 Wn` 포함
3. 원격 push는 Phase 16 완료 선언 이후에 수행
4. 종료 후 회귀 발생 시 `hotfix/phase16-*` 브랜치로 분리

## 8. 리스크와 대응

| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| P16-R1 | 타깃 분리 중 링크/심볼 깨짐 | High | W1/W2/W3 단계별 빌드 검증 + 의존 그래프 고정 |
| P16-R2 | `wb_atoms` 제거 중 책임 누락/중복 | High | ownership 인벤토리 기반 이관 + 게이트 중복 검사 |
| P16-R3 | 디렉터리 정렬 중 include 경로 회귀 | Medium | include 경로 점진 치환 + 빌드 게이트 상시 실행 |
| P16-R4 | 메뉴/윈도우 오픈 회귀 재발 | Medium | 정책 4.4 유지 + W6 메뉴 매트릭스 필수 |
| P16-R5 | `P9-BUG-01` 장기 이월 | Medium | Phase 17 자동 시나리오 준비 전제 조건 명시 |

## 9. 참조 문서

- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/phase15/refactoring_phase15_render_platform_port_isolation_260407.md`
- `docs/refactoring/phase15/dependency_gate_report.md`
- `docs/refactoring/phase15/go_no_go_phase16.md`
- `docs/refactoring/phase15/logs/menu_open_matrix_phase15_latest.md`

## 10. 진행 체크리스트

- [x] W0 기준선/중간 산출문서 등록
- [ ] W1 `wb_structure/wb_measurement/wb_density` 분리
- [ ] W2 `wb_platform/wb_shell/wb_workspace` 정렬 + `wb_common` 전환
- [ ] W3 링크 의존 재배치 + 비순환 고정
- [ ] W4 `wb_atoms` 제거 + 잔존 참조 제거
- [ ] W5 정적 게이트 스크립트 도입
- [ ] W6 빌드/테스트/종료 문서 완료
