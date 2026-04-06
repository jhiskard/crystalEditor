# webassembly/src 목표 아키텍처 완결 후속 계획서 (Phase 13+)

작성일: `2026-04-06`  
기준 문서:
- 마스터 계획서: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- 최신 결과보고서: `docs/refactoring/refactoring_result_report_phase7_12_260406.md`
- Phase 12 종료 문서: `docs/refactoring/phase12/dependency_gate_report.md`, `docs/refactoring/phase12/go_no_go_phase13.md`

## 0. 계획 수립 배경

1. 기존 마스터 계획서는 Phase 7~12 실행으로 종료되었고, 종료 판정 문서에서 `Phase 13`은 `N/A`로 기록되었다.
2. 그러나 결과보고서(Phase 7~12) 기준으로 `3. 목표 아키텍처`의 엄격 DoD는 부분 달성 상태다.
3. 본 문서는 **목표 아키텍처 3장 전체 완결**을 위한 후속 실행 계획이며, 번호 연속성을 위해 **Phase 13부터** 시작한다.

## 1. 목적과 범위

- 목적:
  - `refactoring_plan_full_modular_architecture_260331.md`의 `3. 목표 아키텍처` 전 항목(3.1~3.8) 완전 달성
  - 엄격 DoD 잔여 항목(싱글턴/alias/모듈 타깃/경계 위반) 해소
- 범위:
  - `webassembly/src` 전체
  - `CMakeLists.txt`, `webassembly/cmake/modules/*`
  - `webassembly/tests`, `tests/e2e`
  - `scripts/refactoring/*`
  - `docs/refactoring/phase13+/*`, 최종 결과보고서

## 2. 현재 기준선(2026-04-06)

결과보고서 기준 잔여 핵심 지표:

| 항목 | 현재값 | 목표 |
|---|---:|---:|
| `DECLARE_SINGLETON` 실사용 선언 수 | 7 | 0 또는 runtime 내부 shim만 |
| `::Instance()` 총 호출(`*.cpp`) | 104 | 0 또는 composition root 내부 한정 |
| `friend class` | 1 | 0 |
| legacy alias(`createdAtoms|createdBonds|cellInfo`) 전체 잔존 | 291 | compatibility 외부 0 + 내부 구조 API 치환 완료 |
| `wb_atoms` 참조 | 6 | 0 |
| `atoms_template.cpp` | 5803 lines | facade 수준 축소 또는 제거 |
| `P9-BUG-01` | Deferred | Resolved 또는 자동 재현 가능 Known Issue로 확정 |

게이트 상태:
- `check_phase12_architecture_seal.ps1`: PASS
- `build-wasm:release`, `test:cpp`, `test:smoke`: PASS

## 3. 목표 아키텍처 달성 매트릭스 (기존 계획 3장 대응)

| 기존 목표(3장) | 현재 상태 | 후속 Phase | 완료 조건 |
|---|---|---|---|
| 3.1 핵심 원칙(Composition root/Port-Adapter/상태 단일화) | 원칙은 적용됐으나 allowlist 기반 잔여 경로 존재 | P13~P15 | 원칙 위반 경로 0, 예외 항목 제거 |
| 3.2 목표 모듈 경계 | 구조 분해는 진행됐으나 `atoms` 축 잔존 | P14~P16 | `structure/measurement/density` 책임 완전 분리 |
| 3.3 목표 디렉터리 레이아웃 | 일부 디렉터리만 반영, legacy 파일 잔존 | P14~P16 | 목표 레이아웃으로 최종 정렬 |
| 3.4 목표 런타임 구성 | runtime 도입 완료, 일부 singleton 접근 잔존 | P13 | composition root 외 singleton lookup 0 |
| 3.5 상태 소유권 모델 | 외부 노출은 축소, 내부 alias 잔존 | P14 | 저장소 API 중심 소유권 확정 |
| 3.6 포트/어댑터 계약 | 주요 포트 도입, 비-render VTK 잔존 | P15 | non-render의 VTK actor/volume 직접 조작 0 |
| 3.7 최종 빌드 타깃 | 모듈 비순환 PASS, `wb_atoms` 미제거 | P16 | 최종 타깃 세트 전환 + `wb_atoms` 제거 |
| 3.8 주요 유스케이스 흐름 | 대부분 동작, 반복 시퀀스 검증 약함 | P17 | 핵심 흐름 자동 회귀 PASS |

### 3.2 대응 상세표: 목표 모듈 경계 (후속 계획 반영본)

| 모듈 | 기존 3.2 목표 책임 | 현재 상태(2026-04-06) | 후속 보완 작업(Phase 13+) | 완료 목표 Phase |
|---|---|---|---|---|
| `common` | 공통 타입/유틸/포트 인터페이스만 제공 | `webassembly/src/common` 존재, 빌드는 `wb_core` 명칭 사용 | `wb_core -> wb_common` 명칭/책임 정렬, 공통 인터페이스 집중 | P16 |
| `platform` | Emscripten/binding/browser/worker/persistence adapter | `platform/wasm`만 실체화, browser/worker/persistence는 `io/platform` 등에 분산 | adapter 경로를 `platform/*`로 재배치, 바인딩/파일대화상자/저장소 경계 통합 | P15~P16 |
| `shell` | 메뉴/레이아웃/command dispatch/composition root | `shell/runtime`, `shell/application` 존재, `shell/presentation` 미구성 | `app.cpp`/`toolbar.cpp` 책임을 `shell/presentation`으로 이관 | P13~P14 |
| `workspace` | active ids/selection/context/read-model 문맥 관리 | `workspace/domain`, `workspace/application` 존재, presentation 미구성 | `workspace/presentation` 도입 및 panel read model 경로 정리 | P14 |
| `render` | viewport/camera/picking/overlay adapter | `render/application`, `render/infrastructure` 존재, presentation 미구성 | non-render VTK 직접 조작 제거 + 필요 presentation 계층 정리 | P15 |
| `mesh` | mesh repository/query/visibility/read model | `mesh/domain`, `mesh/application`, `mesh/presentation` 구성됨 | `MeshManager` 잔여 의존 제거 및 repository API 단일화 | P13~P16 |
| `structure` | 구조 lifecycle/edit use-case 및 상태 소유 | `structure/domain`, `structure/application`, `structure/presentation` 존재, infrastructure 미구성 | `structure/infrastructure` 정리 + `atoms` 잔여 구조 책임 이관 | P14~P16 |
| `measurement` | 측정 계산/저장/overlay descriptor | `measurement/domain`, `measurement/application`, `measurement/presentation` 구성됨 | `atoms` 내부 측정 경로 정리 및 포트 계약 완결 | P14~P15 |
| `density` | grid/isosurface/slice 상태/use-case | `density/domain`, `density/application`, `density/infrastructure`, `density/presentation` 구성됨 | legacy alias/호환 경로 제거 및 io/workspace 연계 정리 | P14~P16 |
| `io` | import workflow/parser/apply/transaction | `io/application`, `io/infrastructure`, `io/platform` 구성됨 | `FileLoader` facade 제거, `platform`/`io` 경계 재분리 | P13~P16 |

### 3.3 대응 상세표: 목표 디렉터리 레이아웃 (후속 계획 반영본)

| 목표 디렉터리(기존 3.3) | 현재 상태(2026-04-06) | 보완 계획 | 완료 목표 Phase |
|---|---|---|---|
| `webassembly/src/common/` | 존재(파일 4) | `wb_common` 기준 공통 계층 정렬 | P16 |
| `webassembly/src/platform/browser/` | 미구성 | `io/platform`의 브라우저 어댑터 재배치 | P15~P16 |
| `webassembly/src/platform/worker/` | 미구성 | parser/worker bridge를 `platform/worker`로 이동 | P15~P16 |
| `webassembly/src/platform/wasm/` | 구성됨(파일 1) | runtime API 경계만 사용하도록 계약 고정 | P13 |
| `webassembly/src/platform/persistence/` | 미구성 | layout/IDBFS/persistence adapter 경로 정리 | P15~P16 |
| `webassembly/src/shell/{runtime,application,presentation}/` | `runtime`, `application` 존재 / `presentation` 미구성 | `app/toolbar` UI 책임을 `shell/presentation`으로 이관 | P13~P14 |
| `webassembly/src/workspace/{domain,application,presentation}/` | `domain`, `application` 존재 / `presentation` 미구성 | context/read-model presentation 계층 추가 | P14 |
| `webassembly/src/render/{application,infrastructure,presentation}/` | `application`, `infrastructure` 존재 / `presentation` 미구성 | 필요 presentation 계층 도입 및 포트 경계 정리 | P15 |
| `webassembly/src/mesh/{domain,application,presentation}/` | 3계층 모두 존재 | 잔여 singleton 의존 제거 및 정합성 강화 | P13~P16 |
| `webassembly/src/structure/{domain,application,infrastructure,presentation}/` | `infrastructure`만 미구성 | 구조 인프라 계층 보강 + `atoms` 책임 이관 | P14~P16 |
| `webassembly/src/measurement/{domain,application,presentation}/` | 3계층 모두 존재 | 측정 경계 최종 정리(overlay port 완결) | P15 |
| `webassembly/src/density/{domain,application,infrastructure,presentation}/` | 4계층 모두 존재 | workflow/상태 소유권 최종 정리 | P14~P16 |
| `webassembly/src/io/{application,infrastructure,platform}/` | 3계층 모두 존재 | `platform` 분리 재정렬 및 facade 제거 | P13~P16 |

## 4. 공통 정책 (Phase 13+ 재정의)

## 4.1 주석 작성 정책 v2 (Phase 7~12 반영)

목적:
- 경계 전환 코드에서 임시 shim/allowlist 의존을 제거하고, 계약과 제거 계획을 코드에 남긴다.

핵심 규칙:
1. public API는 Doxygen `@brief` 필수, 경계 계약은 `@details` 또는 `@note` 필수.
2. 임시 shim/bridge/compat 경로는 아래 메타 정보를 반드시 포함한다.
   - `@note Temporary compatibility path`
   - 제거 목표 Phase (예: `remove in Phase 15`)
   - 추적 ID (예: `ARCH-DEBT-014`)
3. `.h`는 계약(책임/입출력/소유권), `.cpp`는 이유(순서/제약/우회 배경) 중심으로 작성한다.
4. Phase 종료 게이트에서 `새 public API Doxygen 누락 = FAIL`로 처리한다.
5. 동작이 바뀐 코드의 기존 주석 미갱신은 주석 누락과 동일하게 FAIL 처리한다.

게이트 반영:
- `check_phase13plus_doc_contract.ps1`에서 아래 검사 고정
  - 신규/변경 public API 주석 존재
  - `Temporary` 주석의 제거 Phase/추적 ID 존재
  - 만료된 임시 주석(목표 Phase 초과) 0건

## 4.2 Git 저장소 운영 정책 v2 (Phase 7~12 반영)

브랜치 정책:
1. Phase 단위 브랜치 고정:
   - `refactor/phase13-runtime-hard-seal`
   - `refactor/phase14-alias-state-ownership`
   - `refactor/phase15-render-port-isolation`
   - `refactor/phase16-build-target-finalize`
   - `refactor/phase17-final-architecture-seal`
2. WBS 단위 로컬 커밋 필수, 커밋 메시지에 `phaseN Wx` 포함.

동기화 정책:
1. 기본 push 시점은 Phase W6 종료 직후.
2. Phase 종료 후 발견된 회귀는 `hotfix/phaseN-...` 브랜치로 분리하고, 종료 문서의 사후 보완 섹션에 연결한다.
3. rebase/squash로 WBS 추적성이 사라지는 이력 재작성은 금지한다.
4. force push는 긴급 승인 케이스 외 금지한다.

산출물 추적 정책:
1. Phase 산출물은 `docs/refactoring/phaseN/`에 저장한다.
2. `logs/` 하위 파일이 `.gitignore`에 의해 누락되지 않도록 Phase 시작 시 추적 규칙을 선반영한다.
3. 실행 로그(`build/test/gate`)는 명령, 시각, 결과를 함께 남긴다.

## 4.3 검증 정책

각 Phase W6 최소 실행:
1. `npm run build-wasm:release`
2. `npm run test:cpp`
3. `npm run test:smoke`
4. 해당 Phase 정적 게이트 스크립트

Phase 17 추가 실행:
- `npm run test:phase6` + 신규 architecture full gate

## 5. Phase별 후속 실행 계획

## Phase 13. Runtime Hard Seal + Singleton Entrypoint 정리 (4~6일)

목표:
- composition root 외 core singleton 진입점 제거
- `App/Toolbar/FileLoader`의 singleton 의존 제거 또는 runtime 내부 shim 한정

WBS:
1. W0: singleton/`::Instance()` 잔존 경로 기준선 재고정
2. W1: `App` singleton 제거, frame orchestration 객체화
3. W2: `Toolbar` singleton 제거, shell presentation 객체화
4. W3: `FileLoader` 진입 경로를 runtime/workflow API로 완전 전환
5. W4: binding/main 경로의 runtime API 계약 정비 + 주석 보강
6. W5: `check_phase13_runtime_hard_seal.ps1` 도입
7. W6: build/test/smoke/gate + `go_no_go_phase14.md`

완료 기준:
- `App::Instance()`, `Toolbar::Instance()`, `FileLoader::Instance()` 호출 0
- 신규 allowlist 항목 0

## Phase 14. State Ownership 완결 + Legacy Alias 제거 (5~8일)

목표:
- `createdAtoms/createdBonds/cellInfo` 내부 잔존 치환
- 구조/측정/밀도 상태 소유권을 repository API로 통일

WBS:
1. W0: alias 참조 파일별 분포 스냅샷
2. W1: `atoms/domain` 우선 치환
3. W2: `atoms/application` 치환 + read model 경로 정리
4. W3: `atoms/ui` 치환 + compatibility facade 축소
5. W4: `lcrs_tree.h` 포함 `friend class` 0건화
6. W5: `check_phase14_state_ownership_alias_zero.ps1` 도입
7. W6: 회귀 검증 + `go_no_go_phase15.md`

완료 기준:
- legacy alias 외부 노출 0 유지 + 내부 직접 참조 목표치(최종 0)
- `friend class` 0
- 상태 변경 API가 repository/service 경유로만 동작

## Phase 15. Render/Platform Port Isolation 완결 (5~7일)

목표:
- non-render 계층의 VTK actor/volume 직접 조작 제거
- 포트/어댑터 계약(3.6) 완결

WBS:
1. W0: non-render VTK 토큰/직접 조작 경로 스냅샷
2. W1: `atoms/infrastructure`의 render 책임 이동
3. W2: `mesh/io/ui` 경로의 actor/volume 접근을 RenderPort로 치환
4. W3: `platform/wasm`, `platform/persistence` 경계 정리
5. W4: camera/picking/overlay 계약 문서화
6. W5: `check_phase15_render_platform_isolation.ps1` 도입
7. W6: build/test/smoke + `go_no_go_phase16.md`

완료 기준:
- `render` 외부 `vtkActor`/`vtkVolume` 직접 조작 0
- 포트 계약 위반 include 0

## Phase 16. 최종 Build Target 전환 + 디렉터리 정렬 (4~6일)

목표:
- `wb_atoms` 제거
- 목표 타깃(`wb_common`, `wb_platform`, `wb_shell`, `wb_workspace`, `wb_render`, `wb_mesh`, `wb_structure`, `wb_measurement`, `wb_density`, `wb_io`) 완성

WBS:
1. W0: 타깃/소스 ownership 인벤토리
2. W1: `wb_structure`, `wb_measurement`, `wb_density` 분리
3. W2: `wb_platform`, `wb_shell`, `wb_workspace` 정렬
4. W3: 링크 의존 재배치 + 비순환 고정
5. W4: `wb_atoms` 제거 및 잔존 참조 제거
6. W5: `check_phase16_target_graph_final.ps1` 도입
7. W6: clean build/test/gate + `go_no_go_phase17.md`

완료 기준:
- `wb_atoms` 참조 0
- 최종 타깃 그래프 비순환 PASS
- 소스 파일의 단일 ownership 규칙 충족

## Phase 17. Final Architecture Seal + 종료 패키지 (3~5일)

목표:
- 마스터 계획서 3장/DoD 전 항목 최종 PASS
- 반복 시퀀스 회귀(`P9-BUG-01`) 최종 판정

WBS:
1. W0: 종합 기준선 재확인
2. W1: `P9-BUG-01` 자동 회귀 시나리오 추가
3. W2: smoke 확장(`XSF(Grid) -> VASP -> XSF(Grid) -> VASP`, layout preset, import 반복)
4. W3: full architecture gate 통합 (`check_phase17_full_architecture_seal.ps1`)
5. W4: 결과보고서/종료 판정 문서 작성
6. W5: 전체 패키지 재실행 검증
7. W6: 최종 종료 선언

완료 기준:
- 마스터 계획 DoD 11개 항목 전부 `달성`
- `P9-BUG-01` 상태 확정(`Resolved` 권장)
- 최종 결과보고서 + 종료 판정 문서 완결

## 6. 공통 산출물 형식 (Phase 13+)

각 Phase 필수 산출물:
1. `docs/refactoring/phaseN/refactoring_phaseN_<topic>_<date>.md`
2. `docs/refactoring/phaseN/dependency_gate_report.md`
3. `docs/refactoring/phaseN/go_no_go_phase{N+1}.md`
4. `docs/refactoring/phaseN/logs/*` 근거 로그

루트 산출물:
- 중간/최종 결과보고서만 `docs/refactoring/` 루트에 배치

## 7. 리스크 및 대응

| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| R13-1 | singleton 제거 중 런타임 초기화 순서 회귀 | High | main/runtime 순서 게이트 고정 + 단계별 smoke |
| R14-1 | alias 제거 중 데이터 일관성 깨짐 | High | repository 단위 테스트 확장 + migration adapter 임시 운영 |
| R15-1 | render 책임 이동 중 actor lifetime 회귀 | High | render infra 소유권 단일화 + leak/use-after-free 점검 |
| R16-1 | 타깃 재편 중 링크/순환 재발 | Medium | target graph 게이트 + clean rebuild 의무화 |
| R17-1 | 반복 import 시나리오 자동화 불안정 | Medium | deterministic fixture + retry 없는 고정 시나리오 |

## 8. 최종 완료 기준 (본 후속 계획 종료 기준)

1. `3. 목표 아키텍처(3.1~3.8)` 전 항목이 코드/게이트/문서로 검증된다.
2. core singleton 선언은 제거되거나 runtime 내부 제한으로 문서화되고, 외부 호출 0을 만족한다.
3. `friend class` 0, legacy alias 외부 노출 0, 내부 잔존도 목표치 0을 만족한다.
4. `wb_atoms` 제거 및 최종 타깃 그래프 비순환을 만족한다.
5. `npm run build-wasm:release`, `npm run test:cpp`, `npm run test:smoke`, 최종 architecture gate를 동시 PASS한다.
6. Phase 13~17 산출문서와 최종 결과보고서가 추적 가능하게 정리된다.

## 9. 착수 순서

1. Phase 13
2. Phase 14
3. Phase 15
4. Phase 16
5. Phase 17

순차 진행 원칙:
- 앞 Phase의 W6 종료 문서와 게이트 PASS가 다음 Phase 착수 조건이다.
