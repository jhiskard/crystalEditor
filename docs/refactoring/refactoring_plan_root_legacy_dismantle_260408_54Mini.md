# webassembly/src 루트 레거시 해체 세부 계획서 (Phase 18+)

작성일: `2026-04-08`

기준 문서:
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/refactoring_result_report_phase7_12_260406.md`
- `docs/refactoring/refactoring_result_report_phase1_6_260331.md`
- `docs/refactoring/phase16/refactoring_phase16_build_target_finalize_directory_alignment_260407.md`

적용 정책:
- 주석 작성 정책: `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`의 `4.1 주석 작성 정책 v2`
- Git 저장소 운영 정책: `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`의 `4.2 Git 저장소 운영 정책 v2`

전제:
- 기존 Phase 17은 별도 종료 조건을 가진다.
- 본 문서는 기존 Phase 17 종료 이후 착수하는 후속 계획이다.
- 목표 아키텍처의 기준은 `refactoring_plan_full_modular_architecture_260331.md`의 `3.1~3.7`이며, 디렉터리 레이아웃은 `3.3`, 런타임 구성은 `3.4`, 상태 소유권은 `3.5`, 포트/어댑터 계약은 `3.6`, 빌드 타깃 설계는 `3.7`을 따른다.

## 0. 계획 수립 배경

1. Phase 1~16을 거치며 모듈 경계와 런타임 경계는 상당 부분 정리되었지만, `webassembly/src`와 `webassembly/src/atoms`의 루트 레거시 코드 파일은 여전히 대량으로 남아 있다.
2. 현재 구조는 기능별 모듈이 이미 존재함에도, 루트 레거시 파일이 실제 구현을 계속 보유하고 있어 최종 아키텍처와 파일 소유권이 불일치한다.
3. 따라서 다음 단계의 핵심은 “새 기능을 더 만드는 것”이 아니라, 루트에 남은 원본 코드를 모듈 경계 내부로 완전히 분산시키고 루트에는 `main.cpp`, `app.cpp`, `app.h`만 facade 수준으로 남기는 것이다.
4. 본 계획은 이 파일 해체를 단계적으로 수행하기 위한 실행 계획이며, 파일 이동과 삭제를 동시에 추적할 수 있도록 기준선과 검증 게이트를 함께 정의한다.

## 1. 목적과 범위

### 1.1 목적
- `webassembly/src` 및 `webassembly/src/atoms`의 루트 레거시 코드를 해체한다.
- `main.cpp`, `app.cpp`, `app.h`는 최종적으로 얇은 facade 수준만 유지한다.
- 모든 실제 로직은 `3.2 목표 모듈 경계`에 맞는 디렉터리로 이관한다.
- 루트 코드 파일 allowlist를 강제하여, 이후에도 원본 코드가 다시 루트에 쌓이지 않도록 막는다.

### 1.2 범위
- 포함:
  - `webassembly/src` 루트의 `*.cpp`, `*.h`
  - `webassembly/src/atoms` 루트의 `*.cpp`, `*.h`
  - 이관 대상이 되는 `common`, `platform`, `shell`, `workspace`, `render`, `mesh`, `structure`, `measurement`, `density`, `io` 하위 모듈
  - `CMakeLists.txt` 및 `webassembly/cmake/modules/*`
  - 루트 allowlist 검사 스크립트와 회귀 게이트 스크립트
- 제외:
  - `imgui.ini`, 아이콘 헤더, 외부 라이브러리 산출물, 문서 파일 자체
  - 코드가 아닌 `.md` 운영 문서

## 2. 현재 기준선

### 2.1 루트 코드 파일 현황

| 항목 | 현재값(2026-04-08) | 목표값 |
|---|---:|---:|
| `webassembly/src` 루트 코드 파일 수(`*.cpp`, `*.h`) | 40 | 3 |
| `webassembly/src/atoms` 루트 코드 파일 수(`*.cpp`, `*.h`) | 2 | 0 |
| 루트 레거시 코드 파일 총합 | 42 | 3 |
| 해체 대상 코드 파일 수 | 39 | 0 |
| 루트 facade allowlist | 미정 | `main.cpp`, `app.cpp`, `app.h` |

### 2.2 현재 잔존의 의미
- `main.cpp`는 여전히 진입점이지만, bootstrap과 main loop 외의 책임은 보유하지 않아야 한다.
- `app.cpp/h`는 shell facade로 남을 수 있으나, feature 로직과 알고리즘을 직접 보유하면 안 된다.
- 나머지 루트 파일은 모두 모듈 경계 내부로 이동하거나, 이미 대체 파일이 존재하면 삭제되어야 한다.

### 2.3 목표 디렉터리 준비 상태
- 이미 존재하는 축:
  - `common`
  - `platform/browser`
  - `platform/wasm`
  - `render/application`
  - `render/infrastructure`
  - `mesh/domain`
  - `mesh/application`
  - `mesh/presentation`
  - `structure/domain`
  - `structure/application`
  - `structure/presentation`
  - `measurement/domain`
  - `measurement/application`
  - `measurement/presentation`
  - `density/domain`
  - `density/application`
  - `density/infrastructure`
  - `density/presentation`
  - `io/application`
  - `io/infrastructure`
  - `shell/runtime`
  - `shell/application`
  - `workspace/domain`
  - `workspace/application`
- 보강 또는 신규 생성이 필요한 축:
  - `shell/presentation`
  - `platform/worker`
  - `platform/persistence`
  - 필요한 경우 `render/presentation`

## 3. 목표 상태 정의

### 3.1 루트 allowlist 최종 형태

| 파일 | 최종 역할 | 허용 책임 |
|---|---|---|
| `webassembly/src/main.cpp` | 진입점 | runtime 생성, GLFW/Emscripten 초기화, main loop, 종료 |
| `webassembly/src/app.cpp` | shell facade | 메뉴/창 배치, shell state sync, top-level render dispatch |
| `webassembly/src/app.h` | facade 계약 | shell facade 공개 API와 최소 상태 |

허용되지 않는 책임:
- feature-specific domain 계산
- parser/import orchestration
- VTK actor/volume 직접 소유
- mesh/structure/measurement/density 상태 저장
- singleton 기반 전역 오케스트레이션 로직

### 3.2 루트 파일군 해체 매핑

| 파일군 | 현재 책임 | 목표 모듈 | 해체 방식 |
|---|---|---|---|
| `bind_function.cpp` | wasm binding legacy placeholder | `platform/wasm/workbench_bindings.cpp` | 기존 활성 바인딩으로 대체 후 삭제 |
| `font_manager.cpp/h` | 폰트/아이콘 로딩과 ImGui 폰트 관리 | `shell/presentation` | UI 리소스 계층으로 이관 후 삭제 |
| `custom_ui.cpp/h` | 공통 ImGui 위젯 헬퍼 | `shell/presentation` 또는 `common` | 위젯 헬퍼를 모듈화 후 삭제 |
| `image.cpp/h`, `texture.cpp/h` | 이미지/텍스처 자원 래퍼 | `render/infrastructure` | OpenGL/리소스 어댑터로 이관 후 삭제 |
| `test_window.cpp/h` | 디버그 테스트 패널 | `shell/presentation` | 디버그 패널로 편입 후 삭제 |
| `toolbar.cpp/h` | 툴바 UI와 일부 feature 제어 | `shell/presentation` | shell UI로 이관 후 삭제 |
| `vtk_viewer.cpp/h` | 렌더 윈도우/카메라/픽킹/오버레이 | `render/application`, `render/infrastructure` | render gateway와 controller로 분리 후 삭제 |
| `mouse_interactor_style.cpp/h` | VTK 상호작용 스타일 | `render/infrastructure` | interaction adapter로 이관 후 삭제 |
| `mesh.cpp/h` | mesh 엔티티 및 기본 상태 | `mesh/domain` | 엔티티/상태 모델로 이동 후 삭제 |
| `mesh_group.cpp/h` | mesh group 모델 | `mesh/domain` | domain 모델로 이동 후 삭제 |
| `mesh_group_detail.cpp/h` | mesh group 상세 UI | `mesh/presentation` | presenter로 이동 후 삭제 |
| `mesh_detail.cpp/h` | mesh 상세 UI | `mesh/presentation` | presenter로 이동 후 삭제 |
| `mesh_manager.cpp/h` | mesh 관리/조회/가시성 | `mesh/domain`, `mesh/application` | repository/query/service로 분해 후 삭제 |
| `model_tree.cpp/h` | model tree 패널 | `mesh/presentation` | 패널 섹션 컴포넌트로 분해 후 삭제 |
| `lcrs_tree.cpp/h` | tree 구조 데이터 | `mesh/domain` | tree 모델로 편입 후 삭제 |
| `file_loader.cpp/h` | import orchestration 및 파일 브라우저 | `io/application`, `io/infrastructure`, `platform/browser`, `platform/worker` | import workflow로 분해 후 삭제 |
| `unv_reader.cpp/h` | UNV 파서 | `io/infrastructure` | parser로 이동 후 삭제 |
| `atoms_template.cpp/h` | atoms god-object와 핵심 feature 조정 | `structure`, `measurement`, `density`, `shell/presentation`, `render/application` | 기능 단위로 분해 후 삭제 |
| `atoms_template_bravais_lattice.cpp` | Bravais lattice 코드 설명 자료(실행 경로 아님) | 계획 배제 | 해체/삭제 대상에서 제외(설명 자료 유지) |
| `atoms_template_periodic_table.cpp` | periodic table 코드 설명 자료(실행 경로 아님) | 계획 배제 | 해체/삭제 대상에서 제외(설명 자료 유지) |

### 3.3 shell facade의 목표 책임

`app.cpp/h`는 다음만 담당한다.

1. shell state와 window visibility의 동기화
2. top-level dockspace와 menu dispatch
3. `WorkbenchRuntime`가 소유한 객체에 대한 얇은 위임
4. progress popup, layout preset, focus target 같은 shell UI 상태 브리지

`app.cpp/h`가 보유하면 안 되는 책임:
- mesh/structure/measurement/density 상태의 직접 소유
- parser/workflow/import 트랜잭션 직접 실행
- VTK 렌더 객체 직접 생성/삭제
- feature singleton 직접 탐색

## 4. 공통 정책

### 4.1 주석 작성 정책
- public API에는 Doxygen `@brief`를 기본으로 붙인다.
- 임시 compatibility 경로에는 반드시 `@note Temporary compatibility path`를 남긴다.
- 임시 경로에는 제거 목표 Phase와 추적 ID를 함께 적는다.
- 의미가 바뀐 legacy 주석은 그대로 두지 말고 갱신하거나 제거한다.
- 루트 facade 파일에는 “무엇을 하지 않는지”를 명시해 책임 범위를 좁힌다.

### 4.2 Git 저장소 운영 정책
- Phase 단위 브랜치를 사용한다.
- WBS 단위 커밋을 남기고, 커밋 메시지에는 `phaseN Wx`를 포함한다.
- Phase 종료 후 회귀가 나오면 `hotfix/phaseN-*` 브랜치로 분리한다.
- rebase/squash로 WBS 추적성을 제거하지 않는다.
- force push는 긴급 승인 외 금지한다.

### 4.3 검증 정책
- 각 Phase 종료 시 다음을 기본으로 실행한다.
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
  - 해당 Phase의 정적 게이트 스크립트
- 추가로 루트 allowlist 검사와 파일 소유권 검사를 병행한다.

### 4.4 루트 allowlist 정책
- `webassembly/src` 루트의 코드 파일은 `main.cpp`, `app.cpp`, `app.h`만 허용한다.
- `webassembly/src/atoms` 루트의 코드 파일은 0개여야 한다.
- 새 루트 코드 파일이 추가되면 즉시 FAIL로 처리한다.

## 5. Phase별 실행 계획

### Phase 18. 루트 인벤토리 고정 + facade 축소 착수

목표:
- 루트 코드 파일 목록과 ownership을 고정한다.
- `main.cpp`와 `app.cpp/h`의 facade 책임을 다시 한 번 얇게 만든다.
- 루트 allowlist 검사를 도입한다.

WBS:
1. 루트 코드 파일 인벤토리를 파일군 단위로 확정한다.
2. 각 파일군의 최종 owner 모듈과 삭제 여부를 표준 문서로 고정한다.
3. `main.cpp`는 bootstrap 전용으로, `app.cpp/h`는 shell facade 전용으로 책임을 줄인다.
4. `check_phase18_root_allowlist.ps1`를 도입한다.
5. `shell/presentation`, `platform/worker`, `platform/persistence` 등 보강이 필요한 디렉터리를 생성한다.
6. build/test/smoke를 수행하고 `go_no_go_phase19.md`를 작성한다.

완료 기준:
- 루트 allowlist가 문서와 스크립트로 강제된다.
- `main.cpp`, `app.cpp`, `app.h`의 책임 범위가 명시된다.
- 루트 코드 파일 해체 대상이 파일군별로 확정된다.

### Phase 19. shell/presentation 및 UI helper 이관

목표:
- shell/UI helper 성격의 루트 파일을 모듈 내부로 이동한다.
- `app.cpp/h`에서 직접 조립하던 widget assembly를 제거한다.

WBS:
1. `font_manager.cpp/h`, `custom_ui.cpp/h`, `test_window.cpp/h`를 shell presentation 계층으로 이동한다.
2. `toolbar.cpp/h`를 shell/presentation으로 이관하고, feature별 동작은 service/query로 위임한다.
3. `image.cpp/h`, `texture.cpp/h`를 render infrastructure 또는 shell resource 계층으로 이동한다.
4. `app.cpp/h`의 include 목록과 private state를 정리한다.
5. root-level UI helper 참조를 모두 모듈 경로로 전환한다.
6. build/test/smoke와 UI 회귀 점검 후 `go_no_go_phase20.md`를 작성한다.

완료 기준:
- shell/UI helper 역할을 가진 루트 파일이 더 이상 남지 않는다.
- `app.cpp/h`는 shell facade 수준으로 축소된다.

### Phase 20. render/mesh/io 루트 파일 해체

목표:
- 렌더, 메쉬, IO의 루트 레거시 파일을 모듈 내부로 완전히 이관한다.
- singleton 중심의 파일 owner를 repository/service/presenter로 재배치한다.

WBS:
1. `vtk_viewer.cpp/h`와 `mouse_interactor_style.cpp/h`를 render 경계 내부로 분해한다.
2. `mesh.cpp/h`, `mesh_group.cpp/h`, `mesh_manager.cpp/h`, `lcrs_tree.cpp/h`를 `mesh/domain`과 `mesh/application`으로 분산한다.
3. `mesh_detail.cpp/h`, `mesh_group_detail.cpp/h`, `model_tree.cpp/h`를 `mesh/presentation`으로 이동한다.
4. `file_loader.cpp/h`를 `io/application`, `io/infrastructure`, `platform/browser`, `platform/worker`로 분해한다.
5. `unv_reader.cpp/h`와 `bind_function.cpp`의 잔여 책임을 정리하고 루트 파일을 삭제한다.
6. CMake와 include graph를 정렬하고 `go_no_go_phase21.md`를 작성한다.

완료 기준:
- render/mesh/io 관련 루트 코드 파일이 사라진다.
- `MeshManager`, `FileLoader`, `VtkViewer`의 루트 의존이 제거된다.

### Phase 21. atoms 루트 해체

목표:
- `webassembly/src/atoms` 루트의 원본 파일을 완전히 해체한다.
- `atoms_template.cpp/h`의 기능을 `structure`, `measurement`, `density`, `shell/presentation`으로 분산한다.

WBS:
1. `atoms_template.cpp/h`의 책임을 기능 단위로 다시 분류한다.
2. 구조 편집과 lifecycle 로직을 `structure/application`과 `structure/presentation`으로 이동한다.
3. 측정 계산과 overlay 생성 로직을 `measurement/application`으로 이동한다.
4. charge density, isosurface, slice 관련 로직을 `density/application`과 `density/presentation`으로 이동한다.
5. `atoms_template_bravais_lattice.cpp`, `atoms_template_periodic_table.cpp`는 실행 경로가 아닌 설명 자료 파일로 분류하고 본 해체 범위에서 제외한다.
6. atoms 루트 코드 파일 0건을 확인한 뒤 `go_no_go_phase22.md`를 작성한다.

완료 기준:
- `webassembly/src/atoms` 루트 코드 파일 수가 0이 된다.
- `atoms_template.cpp/h`가 더 이상 존재하지 않는다.

### Phase 22. 루트 seal + 최종 정리

목표:
- 루트 allowlist를 최종적으로 고정한다.
- 모듈 경계와 빌드 타깃의 정합성을 마무리한다.
- 이후의 코드 추가가 다시 루트 레거시를 만들지 못하도록 차단한다.

WBS:
1. 루트 allowlist 검사 스크립트를 CI 또는 로컬 게이트로 고정한다.
2. `CMakeLists.txt`와 `webassembly/cmake/modules/*`에서 루트 레거시 참조를 제거한다.
3. 남아 있는 compatibility path를 module 내부로만 제한한다.
4. 최종 build/test/smoke와 architecture gate를 수행한다.
5. 최종 결과보고서와 종료 판정 문서를 작성한다.
6. root facade가 아닌 코드가 루트에 재유입되지 않도록 운영 규칙을 확정한다.

완료 기준:
- `webassembly/src` 루트 코드 파일은 `main.cpp`, `app.cpp`, `app.h`만 남는다.
- `webassembly/src/atoms` 루트 코드 파일은 0개다.
- 모든 기능 로직은 target module 경계 내부에 존재한다.

## 6. 산출물과 게이트 형식

각 Phase는 아래 산출물을 남긴다.

1. `docs/refactoring/phaseN/refactoring_phaseN_<topic>_<date>.md`
2. `docs/refactoring/phaseN/dependency_gate_report.md`
3. `docs/refactoring/phaseN/go_no_go_phase{N+1}.md`
4. `docs/refactoring/phaseN/logs/*`
5. 루트 allowlist 스냅샷과 파일 ownership 인벤토리

여기서 `N`은 18 이상 각 Phase 번호를 의미한다.

공통 권장 스크립트:
- `scripts/refactoring/check_phase18_root_allowlist.ps1`
- `scripts/refactoring/check_phase18_facade_budget.ps1`
- `scripts/refactoring/check_phase18_module_boundary.ps1`

## 7. 리스크 및 대응

| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| R18-1 | 루트 파일 인벤토리 누락 | High | 파일군별 baseline 스냅샷과 allowlist 검사를 먼저 고정 |
| R19-1 | shell/UI helper 이동 중 메뉴 회귀 | High | 메뉴 오픈 매트릭스와 smoke 시나리오를 phase gate에 포함 |
| R20-1 | render/mesh/io 분해 중 include cycle 재발 | High | 모듈별 dependency gate와 link 방향 검사를 병행 |
| R21-1 | atoms 기능 분해 중 상태 중복 | High | 상태 소유권 표를 기준으로 repository/API를 먼저 확정 |
| R22-1 | 루트 레거시 재유입 | Medium | 루트 allowlist 검사와 CI 게이트를 상시화 |

## 8. 최종 완료 기준

1. `webassembly/src` 루트에는 `main.cpp`, `app.cpp`, `app.h`만 남는다.
2. `webassembly/src/atoms` 루트 코드 파일은 0개가 된다.
3. `main.cpp`는 bootstrap만, `app.cpp/h`는 shell facade만 담당한다.
4. `atoms`, `mesh`, `render`, `io`, `shell`, `structure`, `measurement`, `density`, `workspace`, `platform`의 실제 로직은 target module 내부에만 존재한다.
5. `npm run build-wasm:release`, `npm run test:cpp`, `npm run test:smoke`, 루트 allowlist gate, architecture gate가 모두 PASS한다.
6. 관련 Phase 산출물과 최종 결과보고서가 추적 가능하게 정리된다.

## 9. 착수 순서

1. Phase 18
2. Phase 19
3. Phase 20
4. Phase 21
5. Phase 22

순차 진행 원칙:
- 앞 단계의 W6 종료 문서와 게이트 PASS가 다음 단계 착수 조건이다.
- 루트 allowlist는 첫 단계부터 고정하며, 이후 단계에서 예외를 늘리지 않는다.
