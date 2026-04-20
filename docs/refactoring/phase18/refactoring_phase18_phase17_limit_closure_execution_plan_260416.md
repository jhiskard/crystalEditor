# Phase 18 세부 작업계획서: Phase17 한계 보완 및 최종 완결

작성일: `2026-04-16 (KST)`  
기준 문서:
- `docs/refactoring/refactoring_result_report_phase17_root_legacy_dismantle_260416.md`
- `docs/refactoring/phase17-root/r6/go_no_go_phase17_close.md`
- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md` (4.1~4.4 정책)
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (3.1~3.7 목표 아키텍처)

대상 범위:
- `webassembly/src/*`
- `app/workbench/page.tsx`
- `tests/e2e/*`
- `scripts/refactoring/*`
- `docs/refactoring/phase18/*`

진행 상태: `계획 수립`

## 0. 배경과 목적

Phase17 종료 보고서의 `7. 현재 코드 기준 한계`는 아래 5가지다.

1. 수동 UI 회귀 체크리스트 최종 완료 증빙 부재
2. `workspace/legacy/atoms_template_facade.h` 직접 참조 다수
3. `AtomsTemplate::Instance()` 경로 유지(탈-legacy 미완결)
4. atoms 해체 이후 잔존한 빈 폴더/미사용 파일 정리 미완료
5. `refactoring_plan_architecture_completion_phase13plus_260406.md`의 주석 정책(4.1) 전 코드파일 준수 점검/보완 미완료

본 계획의 목적은 위 5개 항목을 닫아 **리팩토링 완결 상태**를 달성하는 것이다.

## 1. 준수 기준 (정책/아키텍처 추적)

| 기준 | 적용 방식 |
|---|---|
| 3.1 핵심 원칙 | composition root 외부 singleton 진입 금지 |
| 3.2 목표 모듈 경계 | `workspace/legacy` 의존을 feature service/port로 대체 |
| 3.4 runtime 구성 | `WorkbenchRuntime`에서 명시적 조립/주입 |
| 3.6 포트/어댑터 계약 | service -> port -> legacy adapter 경유 고정 |
| 4.1 주석 정책 v2 | 신규 public API Doxygen + temporary 경로 주석 메타 필수 |
| 4.1 전수 감사 적용 | Phase18 범위 코드파일 전수 점검 + 위반 0 또는 예외 문서화 |
| 4.2 Git 정책 v2 | WBS 단위 커밋, 추적 가능한 브랜치/문서 운영 |
| 4.3 검증 정책 | build/test/smoke + 정적 게이트 동시 PASS |
| 4.4 UI 상태 동기화 | Reset/Layout/메뉴 창 오픈 회귀 방지 규칙 유지 |

## 2. 현재 기준선 (2026-04-16)

### 2.1 한계 항목 정량값

| 항목 | 현재값 | 목표 |
|---|---:|---:|
| 수동 UI 회귀 완료 증빙(`phase17-root/r6/logs`) | 없음(인벤토리만 존재) | 자동+수동 증빙 세트 존재 |
| `workspace/legacy/atoms_template_facade.h` 직접 참조 | 36 | 0(모듈 외부) |
| `AtomsTemplate::Instance()` 전체 호출 | 66 | 0 |
| `AtomsTemplate::Instance()` 외부 호출(`workspace/legacy` 제외) | 61 | 0 |
| `DECLARE_SINGLETON(AtomsTemplate)` 선언 | 1 | 0 |
| `webassembly/src/atoms/legacy` 빈 폴더 | 1 | 0 |
| 코드 파일이 존재하는 디렉터리의 불필요 `.gitkeep` | 5 | 0 |
| 미참조 헤더 후보 | 3 | 0 또는 문서화된 보존 |
| 로컬 아티팩트(`.DS_Store`, `.vscode`) | 7 | 0(저장소 추적 대상 외) |
| 주석 정책 감사 대상 코드파일 수(Phase18 범위) | 253 | 100% 점검 |
| 신규/변경 public API Doxygen 누락 | 미측정 | 0 |
| `Temporary compatibility path` 메타(제거 Phase/추적 ID) 누락 | 미측정 | 0 |
| 동작 변경 코드의 주석 미갱신 | 미측정 | 0 |

### 2.2 facade include 참조 분포(36건)

| 상위 경로 | 건수 |
|---|---:|
| `shell/*` | 10 |
| `structure/*` | 8 |
| `io/*` | 5 |
| `density/*` | 4 |
| `mesh/*` | 3 |
| `render/*` | 3 |
| `measurement/*` | 2 |
| `app.cpp` | 1 |

### 2.3 `AtomsTemplate::Instance()` 호출 상위 파일

| 파일 | 건수 |
|---|---:|
| `webassembly/src/density/application/density_service.cpp` | 13 |
| `webassembly/src/structure/application/structure_service.cpp` | 12 |
| `webassembly/src/measurement/application/measurement_service.cpp` | 11 |
| `webassembly/src/workspace/legacy/atoms_template_facade.cpp` | 5 |
| `webassembly/src/io/application/import_orchestrator.cpp` | 4 |
| `webassembly/src/io/application/parser_worker_service.cpp` | 4 |
| `webassembly/src/app.cpp` | 4 |

### 2.4 미사용 요소 스냅샷

#### A. 즉시 정리 가능(코드 영향 없음)

1. 빈 폴더:
   - `webassembly/src/atoms/legacy`
2. 불필요 `.gitkeep` 후보(동일 폴더에 실파일 존재):
   - `webassembly/src/io/platform/.gitkeep`
   - `webassembly/src/platform/persistence/.gitkeep`
   - `webassembly/src/platform/worker/.gitkeep`
   - `webassembly/src/render/presentation/.gitkeep`
   - `webassembly/src/shell/presentation/.gitkeep`
3. 로컬 아티팩트(언트래킹/개발환경 잔여물):
   - `webassembly/src/.vscode/*`
   - `webassembly/src/atoms/.DS_Store`
   - `webassembly/src/atoms/.vscode/*`
   - `webassembly/src/atoms/domain/.DS_Store`
   - `webassembly/src/atoms/infrastructure/.DS_Store`

#### B. 검증 후 정리(코드 영향 가능성 있음)

미참조 헤더 후보:
1. `webassembly/src/density/infrastructure/density_render_bridge.h`
2. `webassembly/src/density/infrastructure/rho_file_parser.h`
3. `webassembly/src/density/infrastructure/slice_renderer.h`

정리 원칙:
- 완전 미사용 확인 시 삭제
- 향후 구현 예약이면 `docs/refactoring/phase18/logs/unused_file_allowlist_phase18_latest.md`에 보존 사유 기록

### 2.5 주석 정책 감사 대상 스냅샷

| 범위 | 파일 수 |
|---|---:|
| `webassembly/src` (`*.cpp/*.h`) | 213 |
| `webassembly/tests` (`*.cpp/*.h`) | 7 |
| `app` (`*.ts/*.tsx/*.js`) | 5 |
| `tests` (`*.ts/*.tsx/*.js`) | 1 |
| `scripts/refactoring` (`*.ps1`) | 27 |
| 합계(Phase18 감사 범위) | 253 |

감사 기준:
1. C/C++ public API: `@brief` 필수, 경계 계약 `@details` 또는 `@note` 필수
2. 임시 호환 경로: `Temporary compatibility path` + 제거 Phase + 추적 ID 필수
3. 동작 변경 코드: 주석과 구현 동기화(불일치 금지)
4. TS/JS/PS1: 공개 엔트리/계약 함수의 책임·입출력·제약 주석 보강

## 3. 실질 코드 해체/이관 매핑

### 3.1 한계 A: 수동 UI 회귀 증빙 부재 -> 자동/수동 증빙 패키지화

| 현재 위치 | 보완 대상 | 실질 이관/추가 |
|---|---|---|
| `tests/e2e/workbench-smoke.spec.ts` | Phase17 잔여 시나리오 미포함 | `tests/e2e/workbench-layout-panel-regression.spec.ts` 신규 추가 |
| `app/workbench/page.tsx` test API | Layout/Reset/메뉴 창 열기 제어 API 부족 | `__VTK_WORKBENCH_TEST__`에 레이아웃/패널 토글/상태조회 API 추가 |
| `docs/refactoring/phase17-root/r6/logs` | 실행 증빙 문서 부재 | `manual_ui_regression_phase18_latest.md`, `smoke_phase18_layout_latest.txt` 추가 |

핵심 시나리오:
1. 초기 상태 즉시 `Reset` 클릭
2. `Layout 1/2/3 -> Reset` 반복
3. `Edit/Build/Data/Utilities` 창 오픈/유지 확인
4. `BZ`, `import(XSF/CHGCAR)` 연계 상태 확인

### 3.2 한계 B: `workspace/legacy` include 36건 -> 포트 경유화

| 현재 직접 의존 파일군 | 목표 계약 | 목표 경로 |
|---|---|---|
| `structure/application/structure_service.cpp` | `StructureServicePort` 주입 | `structure/infrastructure/legacy/legacy_structure_service_port.*` |
| `measurement/application/measurement_service.cpp` | `MeasurementServicePort` 주입 | `measurement/infrastructure/legacy/legacy_measurement_service_port.*` |
| `density/application/density_service.cpp` | `DensityServicePort` 주입 | `density/infrastructure/legacy/legacy_density_service_port.*` |
| `io/application/import_*` | import용 legacy runtime port | `io/application/legacy_import_runtime_port.h` + adapter |
| `shell/presentation/atoms/*`, `mesh/presentation/*`, `render/*`, `app.cpp` | facade 직접 include 제거 | `shell/application/workbench_controller`, feature service API 경유 |

완료 조건:
- `workspace/legacy/atoms_template_facade.h` 직접 include가 `workspace/legacy` 외부에서 0.

### 3.3 한계 C: `AtomsTemplate::Instance()` 66건 -> singleton 제거

| 현재 경로 | 해체 방향 | 완료 기준 |
|---|---|---|
| `density/structure/measurement` 서비스 레이어 | `AtomsTemplate::Instance()` -> 주입 포트 호출 | 서비스 레이어 `Instance()` 0 |
| `io/application/*` | parser/apply/orchestrator에서 runtime port 사용 | IO 레이어 `Instance()` 0 |
| `shell/runtime/workbench_runtime.cpp` | `AtomsTemplate::Instance().{structureService,...}` 제거, runtime 보유 서비스 반환 | runtime 레이어 `Instance()` 0 |
| `app.cpp` | 창 렌더/레이아웃 호출을 shell/presentation adapter로 이동 | `app.cpp` `Instance()` 0 |
| `workspace/legacy/atoms_template_facade.*` | `DECLARE_SINGLETON(AtomsTemplate)` 제거, runtime-owned 객체화 | singleton 선언 0 |

### 3.4 한계 D: 미사용 요소 잔존 -> 정리/아카이브

| 분류 | 대상 | 조치 | 완료 기준 |
|---|---|---|---|
| 빈 폴더 | `webassembly/src/atoms/legacy` | 폴더 제거 | 경로 미존재 |
| 불필요 `.gitkeep` | 실파일이 존재하는 5개 디렉터리 | `.gitkeep` 삭제 | `.gitkeep` 0 |
| placeholder `.gitkeep` | `structure/infrastructure`, `workspace/presentation` | 유지 또는 기능 이관 시 대체 | 보존 여부 문서화 |
| 미참조 헤더 후보 3개 | density infra 헤더 3개 | 삭제 또는 allowlist 보존 | 0 또는 보존 사유 문서화 |
| 로컬 아티팩트 | `.DS_Store`, `.vscode` 잔여 | 삭제 및 재유입 차단 점검 | 작업 트리 잔여 0 |
| 구버전 게이트 스크립트 | 현 구조 기준 미사용 스크립트 | `scripts/refactoring/legacy/`로 분리(선택) | 운영 게이트와 분리 완료 |

### 3.5 한계 E: 주석 정책(4.1) 전 코드파일 준수 -> 전수 감사/보완

| 분류 | 대상 | 조치 | 완료 기준 |
|---|---|---|---|
| C/C++ 공개 API 주석 | `webassembly/src`, `webassembly/tests` | `@brief/@details/@note` 보강 | 누락 0 |
| 임시 호환 주석 메타 | `workspace/legacy`, adapter/bridge 경로 | `Temporary` + 제거 Phase + 추적 ID 보강 | 누락 0 |
| 동작 변경 코드 주석 | Phase18 변경 파일 전체 | 구현-주석 불일치 수정 | 불일치 0 |
| TS/JS 테스트 API 주석 | `app/workbench/page.tsx`, `tests/e2e/*` | 테스트 계약/전제/부작용 주석 보강 | 주요 엔트리 누락 0 |
| PS1 게이트 스크립트 주석 | `scripts/refactoring/*.ps1` | 체크 항목/실패 조건/출력 의미 주석 보강 | 신규/핵심 게이트 누락 0 |

## 4. 단계별 WBS (실행 순서 고정)

### W0. 기준선 동결 + 해체 인벤토리

1. facade include 36건 파일 목록 고정
2. `AtomsTemplate::Instance()` 66건 목록 고정
3. UI 회귀 시나리오 체크리스트 baseline 등록
4. 미사용 요소 후보(A/B) 인벤토리 고정
5. 주석 정책 감사 대상(253 파일) 및 위반 분류 기준선 고정

산출물:
- `docs/refactoring/phase18/logs/phase18_inventory_snapshot_latest.md`
- `docs/refactoring/phase18/logs/phase18_ui_regression_checklist_latest.md`
- `docs/refactoring/phase18/logs/unused_file_inventory_phase18_latest.md`
- `docs/refactoring/phase18/logs/comment_policy_inventory_phase18_latest.md`
- `docs/refactoring/phase18/logs/comment_policy_violation_list_phase18_latest.md`

완료 기준:
- 이후 W1~W8 비교 기준선 확정

### W1. UI 회귀 자동화 API 확장

1. `app/workbench/page.tsx` 테스트 API 확장
2. Layout/Reset/패널 오픈/가시성 조회 API 추가
3. 메뉴 동기화 규칙(4.4) 검증용 Hook 추가

완료 기준:
- Playwright에서 Phase17 잔여 시나리오 호출 가능

### W2. UI 회귀 증빙 종료

1. `tests/e2e/workbench-layout-panel-regression.spec.ts` 추가
2. 수동 실행 체크리스트 문서화(실제 클릭 기준)
3. 회귀 로그/결과를 `phase18/logs`에 보관
4. 로컬 아티팩트(`.DS_Store`, `.vscode`) 정리 및 재유입 여부 점검

완료 기준:
- 자동 스펙 PASS + 수동 체크리스트 PASS 증빙 확보

### W3. service 포트 복원(Structure/Measurement/Density)

1. `StructureService`, `MeasurementService`, `DensityService`를 포트 주입 구조로 재전환
2. legacy adapter 구현을 각 모듈 `infrastructure/legacy`에 배치
3. 기존 직접 include 제거

완료 기준:
- 3개 서비스 파일의 `workspace/legacy` include 0
- 3개 서비스 파일의 `AtomsTemplate::Instance()` 0

### W4. IO/Runtime 경계 치환

1. `io/application/import_*`, `parser_worker_service`에서 legacy import port 사용
2. `shell/runtime/workbench_runtime.cpp`에서 직접 singleton 접근 제거
3. `io/infrastructure/file_io_manager.cpp`의 facade 직접 참조 제거

완료 기준:
- IO + Runtime 경로 `AtomsTemplate::Instance()` 0

### W5. Shell/App/Presentation 최종 치환

1. `app.cpp`, `shell/presentation/atoms/*`, `mesh/presentation/*`, `render/*`의 direct include 제거
2. 패널 렌더 요청을 controller/service API로 일원화
3. layout/reset/메뉴 오픈 동기화 유지 확인
4. 빈 폴더/불필요 `.gitkeep`/미사용 헤더 후보 정리
5. W3~W5 변경 파일 주석 정책(4.1) 준수 보완

완료 기준:
- `workspace/legacy/atoms_template_facade.h` 외부 include 0
- `app.cpp` `AtomsTemplate::Instance()` 0
- 미사용 요소 정리 항목 A 완료
- 미참조 헤더 후보 3개 처리(삭제 또는 보존사유 문서화)
- W3~W5 변경 파일 주석 누락/불일치 0

### W6. `AtomsTemplate` singleton 제거

1. `workspace/legacy/atoms_template_facade.h`의 `DECLARE_SINGLETON(AtomsTemplate)` 제거
2. runtime 소유 객체(예: `LegacyAtomsRuntime`)로 조립 전환
3. `AtomsTemplate::Instance()` 호출 잔존 0 달성
4. Phase18 범위 코드파일(253) 주석 정책 전수 점검/보완 완료

완료 기준:
- 전체 코드 `AtomsTemplate::Instance()` 0
- singleton 선언 0
- `comment_policy_violation_list_phase18_latest.md`의 잔여 위반 0
- `comment_policy_fix_report_phase18_latest.md` 작성 완료

### W7. 게이트/CI 봉인

신규 게이트:
1. `check_phase18_ui_regression_evidence.ps1`
2. `check_phase18_facade_include_zero.ps1`
3. `check_phase18_atoms_instance_zero.ps1`
4. `check_phase18_legacy_singleton_zero.ps1`
5. `check_phase18_unused_asset_cleanup.ps1`
6. `check_phase18_doc_contract_full.ps1`
7. `check_phase18_final_closure.ps1`

CI 반영:
1. Phase17 게이트 + Phase18 게이트 + build/test/smoke 상시 실행
2. 권한 제약 없는 runner 기준 로그 확보

완료 기준:
- 신규/기존 게이트 전체 PASS

### W8. 통합 검증 + 종료 문서

필수 실행:
1. `npm run build-wasm:release`
2. `npm run test:cpp`
3. `npm run test:smoke`
4. Phase17 + Phase18 게이트 전체

종료 문서:
- `docs/refactoring/phase18/dependency_gate_report.md`
- `docs/refactoring/phase18/go_no_go_phase18_close.md`
- `docs/refactoring/refactoring_result_report_phase18_final_closure_2604xx.md`
- 기존 통합 보고서 갱신

완료 기준:
- 리팩토링 완결 판정 문서화 완료

## 5. 검증 계획

### 5.1 자동 검증

1. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_target_graph_final.ps1`
2. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase18_final_closure.ps1`
3. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase18_unused_asset_cleanup.ps1`
4. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase18_doc_contract_full.ps1`
5. `npm run build-wasm:release`
6. `npm run test:cpp`
7. `npm run test:smoke`

### 5.2 수동 회귀(최종 사인오프)

1. 초기 진입 직후 `Reset`
2. `Layout 1/2/3` 각각 적용 후 `Reset`
3. `Edit/Build/Data/Utilities` 창 오픈/유지
4. `BZ` 창 오픈/유지
5. `XSF/Grid/CHGCAR` import 연계

## 6. 리스크 및 대응

| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| P18-R1 | singleton 제거 중 초기화 순서 회귀 | High | W6 이전에 runtime 조립 단위 테스트 + smoke 고정 |
| P18-R2 | UI 창 오픈 상태 회귀 재발 | High | W1/W2에서 자동 스펙 선도입 후 코드 변경 |
| P18-R3 | facade include 제거 중 타입 의존 연쇄 파손 | High | W3~W5를 모듈별 독립 커밋으로 분할 |
| P18-R4 | CI 환경 차이로 테스트 flaky | Medium | 권한/환경 고정 runner + 로그 보존 |
| P18-R5 | 미사용 후보 오판으로 필요한 파일 삭제 | Medium | 후보별 사용처 재검증 + allowlist 문서화 후 삭제 |
| P18-R6 | 주석 전수 보완 중 과도한 주석/의미 없는 주석 유입 | Medium | 정책 4.1 기준(계약/이유 중심) 리뷰 게이트 적용 |

## 7. 완료 기준 (Phase18 최종 DoD)

1. 수동 UI 회귀 체크리스트가 자동/수동 증빙과 함께 완료된다.
2. `workspace/legacy/atoms_template_facade.h` 직접 include가 `workspace/legacy` 외부에서 0이다.
3. `AtomsTemplate::Instance()` 호출이 전체 코드에서 0이다.
4. `DECLARE_SINGLETON(AtomsTemplate)` 선언이 0이다.
5. `webassembly/src/atoms/legacy` 빈 폴더 및 불필요 `.gitkeep` 정리가 완료된다.
6. 미참조 헤더 후보는 삭제되거나 보존 사유가 allowlist 문서로 관리된다.
7. Phase18 범위 코드파일(253) 주석 정책 전수 점검과 보완이 완료된다.
8. 신규/변경 public API Doxygen 누락 0, Temporary 메타 누락 0, 동작 변경 주석 미갱신 0을 만족한다.
9. Phase17 + Phase18 게이트, `build-wasm`, `test:cpp`, `test:smoke`가 동시 PASS다.
10. Phase18 종료 문서와 통합 결과보고서가 추적 가능하게 작성된다.

## 8. 착수 순서

1. W0 기준선 동결
2. W1~W2 UI 증빙 선완료(회귀 안전망 확보)
3. W3~W6 코드 해체/이관 및 singleton 제거
4. W7~W8 게이트 봉인 및 종료 문서화
