# 리팩토링 상세 결과보고서: Phase 0~17 종합 비교 평가

작성일: `2026-04-16`  
대상 저장소: `vtk-workbench_jclee`  
비교 기준:
- 리팩토링 문서 전수(`docs/refactoring/**/*.md`, 총 `110`건)
- 리팩토링 전 코드: `C:\Users\user\Downloads\vtk-workbench_jclee_orig`
- 리팩토링 후 코드: `C:\Users\user\Downloads\vtk-workbench_jclee`

## 0. 문서 종합 범위

본 보고서는 `docs/refactoring` 하위 문서를 다음과 같이 전수 분류/종합하여 작성했다.

| 분류 | 건수 | 비고 |
|---|---:|---|
| 계획서(`refactoring_plan*`) | 5 | 전체/후속/Root 해체 계획 포함 |
| 단계 실행 문서(`refactoring_phase*`) | 33 | Phase 및 R/W 실행 문서 |
| 의존성 게이트 보고서(`dependency_gate_report*`) | 25 | 각 단계 게이트 결과 |
| Go/No-Go 문서(`go_no_go*`) | 24 | 단계 종료 판정 |
| 결과보고서(`refactoring_result_report*`) | 4 | Phase1~6, 7~12, 17, 통합 |
| 로그 Markdown(`logs/*.md`) | 13 | 인벤토리/버그 추적/검증 보조 |
| 기타(경계 규칙 등) | 6 | `phase1_boundary_rules.md` 등 |
| **합계** | **110** |  |

추가로 문서군별 폴더 집계:
- `phase0` 4건, `phase1` 2건, `phase2` 2건, `phase3` 3건, `phase4` 3건, `phase5` 4건, `phase6` 4건
- `phase7` 3건, `phase8` 3건, `phase9` 3건, `phase10` 3건, `phase11` 3건, `phase12` 4건
- `phase13` 5건, `phase14` 7건, `phase15` 8건, `phase16` 3건, `phase17-root` 36건
- 루트(`docs/refactoring/*.md`) 10건

## 1. 리팩토링 전/후 정량 비교

### 1.1 코드 구조 지표

| 항목 | 리팩토링 전(`_orig`) | 리팩토링 후(현재) | 변화 |
|---|---:|---:|---:|
| `webassembly/src` 하위 최상위 디렉터리 수 | 7 | 16 | +9 |
| `webassembly/src` 하위 `*.cpp/*.h` 총 파일 수 | 121 | 213 | +92 |
| `webassembly/src` 루트(직접) `*.cpp/*.h` 파일 수 | 40 | 3 | -37 |
| `webassembly/src/atoms` 하위 `*.cpp/*.h` 파일 수 | 55 | 0 | -55 |
| `DECLARE_SINGLETON(...)` 선언 수(`*.h`) | 12 | 5 | -7 |
| `::Instance()` 호출 수(`*.cpp/*.h`) | 399 | 114 | -285 |
| `VtkViewer::Instance()` 호출 수(`*.cpp/*.h`) | 199 | 3 | -196 |
| `MeshManager::Instance()` 호출 수(`*.cpp/*.h`) | 84 | 1 | -83 |
| `FileLoader::Instance()` 호출 수(`*.cpp/*.h`) | 6 | 0 | -6 |
| `AtomsTemplate::Instance()` 호출 수(`*.cpp/*.h`) | 67 | 66 | -1 |

### 1.2 빌드/검증 체계 지표

| 항목 | 리팩토링 전(`_orig`) | 리팩토링 후(현재) | 변화 |
|---|---:|---:|---:|
| `webassembly/cmake/modules/wb_*.cmake` 수 | 0 | 10 | +10 |
| `scripts/refactoring/check_phase*.ps1` 수 | 0 | 27 | +27 |
| `webassembly/tests` C++ 테스트(`*.cpp`) 수 | 0 | 5 | +5 |
| `tests/e2e` 스모크 테스트(`*.ts`) 수 | 0 | 1 | +1 |

### 1.3 Phase 0 기준 위반 패턴 추적(문서 대비)

`docs/refactoring/phase0/dependency_baseline_report.md` 기준선과 현재를 비교하면:

| 위반/패턴 | 기준선(Phase 0) | 현재 | 평가 |
|---|---:|---:|---|
| `atoms` 상위 include 위반(P1) | 15 | 0 | 해소 |
| `AtomsTemplate::Instance()`(코드 기준) | 67 | 66 | 구조 부채 잔존 |
| `VtkViewer::Instance()`(코드 기준) | 199 | 3 | 대폭 해소 |
| `MeshManager::Instance()`(코드 기준) | 84 | 1 | 대폭 해소 |

## 2. Phase별 변경사항 종합

### 2.1 Phase 0~16

| Phase | 핵심 변경사항 | 대표 근거 문서 | 종료 판정 |
|---|---|---|---|
| 0 | 기준선/스모크/의존 스냅샷 정립, baseline manifest 고정 | `phase0/*` | GO |
| 1 | `model_tree` 직접 전역 접근 제거, atoms 경계 규칙 도입 | `phase1/dependency_gate_report.md`, `phase1_boundary_rules.md` | GO |
| 2 | 상태 저장소 통합과 alias 정리 기반 마련 | `phase2/dependency_gate_report.md` | GO |
| 3 | Import 파이프라인 분리(`picker/parser/apply/orchestrator`) | `phase3/*` | GO |
| 4 | UI/Renderer 책임 분리, presentation/application 경계 강화 | `phase4/*` | GO |
| 5 | CMake 모듈화와 ownership 검사 도입 | `phase5/*` | GO |
| 6 | C++/E2E 회귀 체계 도입 | `phase6/*` | GO |
| 7 | Composition root 도입 및 singleton quarantine 고정 | `phase7/*` | GO |
| 8 | `AtomsTemplate` 기능 분산(구조/측정/밀도 분리 시작) | `phase8/*` | GO |
| 9 | render 경계 완결, non-render의 `VtkViewer` 직접 호출 축소 | `phase9/*` | GO |
| 10 | workspace/mesh/io 분리 강화 | `phase10/*` | GO |
| 11 | shell/panel 객체화, `App` 축소 | `phase11/*` | GO |
| 12 | compatibility facade/architecture seal, 종료 판정(`Phase13 N/A`) | `phase12/*` | CLOSE (N/A) |
| 13 | Runtime hard seal, `App/Toolbar/FileLoader` entrypoint 정리 | `phase13/*` | GO |
| 14 | legacy alias 0, `friend class` 0, 상태 소유권 정리 | `phase14/*` | GO |
| 15 | render/platform port isolation, browser adapter 재배치 | `phase15/*` | GO |
| 16 | 최종 타깃 그래프 준비, 디렉터리 정렬 | `phase16/*` | GO |

### 2.2 Phase 17-Root (R0~R6)

| R 단계 | 핵심 내용 | 대표 근거 문서 | 판정 |
|---|---|---|---|
| R0 | 루트 인벤토리 동결, allowlist/ownership 게이트 부트스트랩 | `phase17-root/r0/*` | GO |
| R1 | facade 계약 고정 및 계약 위반 차단 | `phase17-root/r1/*` | GO |
| R2 | shell/UI 파일군 해체 이관(`font/custom_ui/test_window/toolbar`) | `phase17-root/r2/*` | GO |
| R3 | render/platform/io 파일군 해체 이관(`file_loader/unv/viewer/binding`) | `phase17-root/r3/*` | GO |
| R4 | mesh 파일군 해체 이관(`mesh*`, `model_tree`, `lcrs_tree`) | `phase17-root/r4/*` | GO |
| R5 | atoms 원본 해체와 feature 모듈 분산, Reset/창 회귀 반영 | `phase17-root/r5/*` | GO |
| R6 | 최종 10개 모듈 그래프 봉인, legacy seal, 종료 패키지 | `phase17-root/r6/*` | GO(자동 기준) |

## 3. 원본 코드군 해체/이관 비교 (핵심)

### 3.1 `webassembly/src` 루트 파일군

리팩토링 전 루트 `*.cpp/*.h`는 40개였고, 현재는 아래 3개만 유지된다.

- `webassembly/src/main.cpp`
- `webassembly/src/app.cpp`
- `webassembly/src/app.h`

즉, 루트는 실행 오케스트레이션/facade 수준으로 축소되었다.

### 3.2 `webassembly/src/atoms` 파일군

리팩토링 전 `webassembly/src/atoms` 하위 `*.cpp/*.h` 55개가 존재했고, 현재는 0개다.  
기존 atoms 책임은 `structure/measurement/density/shell/render/workspace/io`로 분산되었다.

또한 사용자 지시사항에 따라 다음 파일은 실행 경로 해체 대상에서 제외된 “코드 설명 자료”로 처리되었다.
- `atoms_template_bravais_lattice.cpp`
- `atoms_template_periodic_table.cpp`

### 3.3 대표 파일 이관 패턴

| 리팩토링 전 파일군(루트/atoms) | 리팩토링 후 책임 경로(대표) |
|---|---|
| `toolbar.*`, `custom_ui.*`, `test_window.*`, `font_manager.*` | `shell/presentation/*` |
| `vtk_viewer.*`, `mouse_interactor_style.*`, `texture.*`, `image.*` | `render/presentation/*`, `render/infrastructure/*` |
| `file_loader.*`, `unv_reader.*`, `chgcar_parser.*` | `io/application/*`, `io/infrastructure/*`, `platform/browser/*` |
| `mesh*.{h,cpp}`, `model_tree.*`, `lcrs_tree.*` | `mesh/domain/*`, `mesh/application/*`, `mesh/presentation/*` |
| `atoms/domain/*` | `structure/domain/atoms/*`, 일부 `measurement/domain/*`, `density/domain/*` |
| `atoms/ui/*`, `atoms/presentation/*` | `shell/presentation/atoms/*`, `density/presentation/*` |
| `atoms/legacy/atoms_template_facade.*` | `workspace/legacy/atoms_template_facade.*` |

## 4. 목표 아키텍처(3.1~3.7) 달성도 평가

기준: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` 3장

| 항목 | 달성도 | 근거 |
|---|---|---|
| 3.1 핵심 원칙 | 대체로 달성 | 단계별 게이트 27종 운영, 경계 위반 감시 체계 정착 |
| 3.2 목표 모듈 경계 | 달성(운영 기준) | `common/platform/shell/workspace/render/mesh/structure/measurement/density/io` 분리 |
| 3.3 목표 디렉터리 레이아웃 | 달성(실행 기준) | 상위 디렉터리 7 -> 16, 누락 경로 보완 기록(Phase16) |
| 3.4 Composition Root | 달성 | `shell/runtime/workbench_runtime.*` 중심 조립 구조 정착 |
| 3.5 상태 소유권 모델 | 대체로 달성 | Phase14에서 alias 0/friend 0 달성, repository/service 경유 강화 |
| 3.6 포트/어댑터 계약 | 대체로 달성 | Phase15 port isolation, Phase17 R3/R6 게이트 봉인 |
| 3.7 최종 빌드 타깃 | 달성 | R6에서 10개 모듈 전환, `wb_core/wb_ui/wb_atoms` 제거 |

## 5. 검증/판정 체계 종합

### 5.1 Go/No-Go

`docs/refactoring/**/go_no_go*.md` 24건 기준:
- Phase 0~11: 단계별 GO
- Phase 12: `CLOSE (Phase13 N/A)` 판정 문서 존재
- Phase 13~16: 후속 계획 기준 GO
- Phase 17-Root(R0~R6): 각 R 단계 GO
- 최종: `phase17-root/r6/go_no_go_phase17_close.md` = **GO(자동 검증 기준)**

### 5.2 R6 최종 게이트

`docs/refactoring/phase17-root/r6/dependency_gate_report.md` 기준 10/10 PASS:

1. `check_phase17_root_allowlist.ps1`
2. `check_phase17_root_ownership_map.ps1`
3. `check_phase17_facade_contract.ps1`
4. `check_phase17_r2_shell_ui_migration.ps1`
5. `check_phase17_r3_render_platform_io_migration.ps1`
6. `check_phase17_r4_mesh_migration.ps1`
7. `check_phase17_r5_atoms_migration.ps1`
8. `check_phase17_r5_shell_layout_reset_contract.ps1`
9. `check_phase17_r6_atoms_legacy_zero.ps1`
10. `check_phase17_target_graph_final.ps1`

자동 검증:
- `npm run build-wasm:release` PASS
- `npm run test:cpp` PASS
- `npm run test:smoke` PASS

## 6. 리팩토링 전/후 비교 결론

### 6.1 달성된 변화

1. 단일 루트/atoms 집중 구조를 feature 모듈 구조로 분산했다.
2. 루트 코드 파일을 40개에서 3개로 축소해 facade/orchestration 중심으로 정리했다.
3. `atoms` 하위 실행 코드(55개)를 0으로 만들고 책임을 도메인별로 이관했다.
4. singleton/`Instance()` 의존을 크게 줄였고, 아키텍처 게이트를 상시 운영 가능한 형태로 정착시켰다.
5. 빌드/테스트/게이트/판정 문서를 연결한 종료 패키지를 구축했다.

### 6.2 잔여 과제

1. `AtomsTemplate::Instance()` 호출은 여전히 66건으로 높고, `workspace/legacy/atoms_template_facade.h` 직접 참조도 36건 남아 있다.
2. `workspace/legacy/atoms_template_facade.cpp`(5209 lines), `.h`(1325 lines)로 legacy 집중도가 아직 크다.
3. R6 문서 기준 수동 회귀 체크리스트(Reset/Layout 1/2/3, Edit/Build/Data, BZ, import)는 운영 UI 환경 최종 점검이 필요하다.

## 7. 최종 판정

- **종합 실행 판정:** 완료  
- **아키텍처 전환 판정:** 목표 모듈 경계/타깃 그래프 기준 달성  
- **운영 판정:** `Phase 17-Root Closed (자동 검증 기준 GO)`  
- **조건:** 수동 UI 회귀 체크리스트 최종 사인오프 필요


## 10. Phase18 후속 보완 반영 (`2026-04-20`)

Phase18에서 Phase17 보고서의 한계 항목을 보완하여 아래를 추가 달성했다.

1. Phase18 전용 게이트 7종 추가 및 final closure 게이트 PASS
2. W8 필수 검증(`build-wasm:release`, `test:cpp`, `test:smoke`) 최신 PASS 증빙 확보
3. 문서 계약/미사용 자산/UI 회귀 증빙 로그를 Phase18 경로로 봉인

이 업데이트를 반영한 최신 최종 보고서는
`docs/refactoring/refactoring_result_report_phase18_final_closure_260420.md`를 기준으로 관리한다.
