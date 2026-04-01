# Phase 7 세부 작업계획서: Composition Root 도입 + singleton quarantine

작성일: `2026-03-31 (KST)`  
최종 업데이트: `2026-03-31 (계획 수립)`  
대상 범위: `webassembly/src/main.cpp`, `webassembly/src/app.cpp`, `webassembly/src/bind_function.cpp`, `webassembly/src/file_loader.*`, `CMakeLists.txt`, `scripts/refactoring/*`, `docs/refactoring/phase7/*`  
진행 상태: `착수 준비 완료 (Phase 7 implementation 대기)`

## 0. Phase 6 종료 반영사항

### 0.1 진입 판정
- 판정: **GO**
- 근거 문서:
  - `docs/refactoring/phase6/dependency_gate_report.md`
  - `docs/refactoring/phase6/go_no_go_phase7.md`

### 0.2 착수 기준선(2026-03-31 실측)
| 항목 | 기준값 | 출처 |
|---|---:|---|
| `webassembly/src/shell/` 존재 여부 | 0 | `Test-Path webassembly/src/shell` |
| `webassembly/src/platform/` 존재 여부 | 0 | `Test-Path webassembly/src/platform` |
| `webassembly/src/app.cpp` 라인 수 | 1177 | 코드 실측 |
| `webassembly/src/main.cpp` 라인 수 | 178 | 코드 실측 |
| `webassembly/src/bind_function.cpp` 라인 수 | 61 | 코드 실측 |
| `app.cpp`의 `::Instance()` 호출 수 | 38 | `Select-String` 실측 |
| `main.cpp`의 `::Instance()` 호출 수 | 3 | `Select-String` 실측 |
| `bind_function.cpp`의 `::Instance()` 호출 수 | 6 | `Select-String` 실측 |
| `bind_function.cpp`의 `emscripten::function(...)` 수 | 19 | `Select-String` 실측 |
| 소스(`*.h/*.cpp`)의 `DECLARE_SINGLETON` 선언 수(매크로 정의 제외) | 11 | `rg -g *.h -g *.cpp` 실측 |

`app.cpp`에서의 클래스별 singleton lookup 분포(실측):
- `AtomsTemplate`: 16
- `VtkViewer`: 8
- `ModelTree`: 3
- `MeshGroupDetail`: 2
- `MeshDetail`: 2
- `FontManager`: 2
- `FileLoader`: 2
- `TestWindow`: 2
- `MeshManager`: 1

### 0.3 Phase 7 범위 확정
- Phase 7 핵심:
  - runtime composition root(`WorkbenchRuntime`) 도입
  - `main.cpp`, wasm binding, `app.cpp` 호출 경로를 runtime public API 중심으로 재배선
  - singleton 접근을 quarantine 경계(런타임/호환 facade)로 한정
  - 신규 singleton 유입 금지 정적 게이트 도입
- Phase 7 비범위(다음 Phase 이관):
  - `AtomsTemplate` 책임 본격 분해(Phase 8)
  - `VtkViewer` 내부 책임 분해(Phase 9)
  - `MeshManager/FileLoader` 구조 완전 해체(Phase 10)

## 1. Phase 7 목표

### 목표
- 객체 그래프 조립 위치를 명시적으로 고정해, 이후 Phase에서의 구조 분해가 singleton 재증식을 유발하지 않도록 한다.

### 도달 결과물
1. `shell/runtime/WorkbenchRuntime` 도입 및 최소 public API 확정
2. wasm binding 진입점을 `platform/wasm/workbench_bindings.cpp`로 분리
3. `main.cpp`, `app.cpp`가 런타임 API를 통해 기능 호출하도록 정리
4. `check_phase7_runtime_composition.ps1` 정적 게이트와 로그/판정 문서 패키지

### 비목표(Phase 7에서 하지 않음)
- feature domain/application 로직의 대규모 이동
- `AtomsTemplate`, `VtkViewer` 내부 세부 알고리즘 리팩토링
- `wb_atoms` 해체 및 최종 모듈 seal 완료

## 2. 현재 문제 요약(착수 근거)
1. 런타임 composition root 부재로 객체 생성/호출 관계가 singleton 내부에 숨겨져 있다.
2. `main.cpp`, `app.cpp`, `bind_function.cpp`가 서로 다른 singleton 경로를 직접 참조해 진입점 규칙이 불명확하다.
3. wasm binding이 `AtomsTemplate`, `MeshManager`, `FileLoader`, `App`에 직접 연결되어 경계가 넓다.
4. `::Instance()` 신규 유입을 막는 자동 게이트가 없다.
5. 다음 단계(Phase 8~12)에서 분해 작업을 해도 진입점 규칙이 없으면 다시 singleton으로 회귀할 수 있다.

## 3. 작업 원칙
1. `Behavior-preserving` 우선: API 경로만 재배선하고 기능 동작은 유지한다.
2. 런타임 진입점 우선: 내부 분해보다 `main/binding/app -> runtime` 호출 선정리를 먼저 완료한다.
3. quarantine 명시: singleton 접근 허용 구간을 `shell/runtime`와 임시 compatibility facade로 문서화한다.
4. 정적 게이트 선반영: 코드 리뷰 의존이 아니라 스크립트로 규칙 위반을 자동 검출한다.
5. Doxygen 적용: 새 runtime API/port/adapter에는 `@brief` 중심 계약 주석을 의무 적용한다.
6. 로컬 커밋 단위 유지: WBS 완료마다 커밋하고, Phase 종료 선언 후 원격 push를 수행한다.

## 4. 작업 단위(WBS)

## W0. 착수/브랜치 셋업 및 인벤토리 고정
### 작업
- `refactor/phase7-composition-root` 브랜치에서 작업 시작
- 산출 디렉터리 준비:
  - `docs/refactoring/phase7/`
  - `docs/refactoring/phase7/logs/`
- 기준선 인벤토리 기록:
  - singleton lookup 파일별/클래스별 수치
  - wasm binding export 목록
  - 신규 디렉터리 도입 전 상태

### 영향 파일(예상)
- `docs/refactoring/phase7/refactoring_phase7_composition_root_singleton_quarantine_260331.md`
- `docs/refactoring/phase7/logs/runtime_inventory_phase7_latest.md`

### 완료 기준
- 기준선 지표와 범위가 문서로 고정되어 이후 변화량 비교가 가능함

## W1. Runtime composition root 스캐폴딩
### 작업
- `webassembly/src/shell/runtime/workbench_runtime.h/.cpp` 신규 도입
- `WorkbenchRuntime`의 최소 public API 정의:
  - 초기화/프레임 루프 진입
  - shell 명령 dispatch
  - wasm 상태 조회용 query API
- runtime이 내부 singleton 참조를 캡슐화하도록 브리지 함수 배치
- public 타입/함수에 Doxygen 주석 적용

### 영향 파일(예상)
- `webassembly/src/shell/runtime/workbench_runtime.h`
- `webassembly/src/shell/runtime/workbench_runtime.cpp`
- `CMakeLists.txt` 또는 `webassembly/cmake/modules/wb_ui.cmake`(runtime 소스 등록)

### 완료 기준
- `main/binding/app`가 호출 가능한 runtime API 골격이 빌드에 포함됨

## W2. `main.cpp` 런타임 진입점 전환
### 작업
- `main.cpp`에서 `App::Instance()`, `MeshManager::Instance()`, `FontManager::Instance()` 직접 초기화를 runtime API 호출로 치환
- main loop에서 `runtime`이 프레임 렌더/업데이트를 오케스트레이션하도록 연결
- GLFW/ImGui 초기화 순서 유지 검증

### 영향 파일(예상)
- `webassembly/src/main.cpp`
- `webassembly/src/shell/runtime/workbench_runtime.*`

### 완료 기준
- `main.cpp`의 `::Instance()` 호출이 제거되거나 runtime 내부로 이동됨(목표: 3 -> 0)

## W3. wasm binding 분리 및 runtime API 경유화
### 작업
- `webassembly/src/bind_function.cpp`의 역할을 `webassembly/src/platform/wasm/workbench_bindings.cpp`로 이전
- 바인딩 함수가 feature singleton 직접 접근 대신 runtime query/command API를 사용하도록 변경
- 기존 JS 호출 이름(`loadArrayBuffer`, `getStructureCount` 등) 호환 유지
- 바인딩 파일 public 함수에 Doxygen 주석 적용

### 영향 파일(예상)
- `webassembly/src/platform/wasm/workbench_bindings.cpp` (신규)
- `webassembly/src/bind_function.cpp` (삭제 또는 thin wrapper 전환)
- `CMakeLists.txt` (entrypoint 소스 교체)

### 완료 기준
- wasm binding 계층의 `::Instance()` 호출 0건 달성
- 바인딩 계층에서 `atoms/atoms_template.h`, `mesh_manager.h` 직접 include 제거

## W4. `app.cpp` singleton quarantine 적용
### 작업
- 메뉴/패널 액션의 singleton 직접 호출을 runtime command API로 전환
  - 구조 import 요청
  - builder/editor/data menu 요청
  - viewer 옵션 토글
  - model tree/mesh detail 관련 조회/렌더 진입
- `app.cpp`에서 feature singleton lookup을 제거하고 runtime 통해 위임
- `App` 내부 UI 상태(`m_bShow*`)와 feature command 경계를 분리

### 영향 파일(예상)
- `webassembly/src/app.cpp`
- `webassembly/src/app.h`
- `webassembly/src/shell/runtime/workbench_runtime.*`

### 완료 기준
- `app.cpp`의 feature singleton lookup(`AtomsTemplate`, `VtkViewer`, `FileLoader`, `MeshManager`, `ModelTree`, `MeshDetail`, `MeshGroupDetail`, `TestWindow`) 36 -> 0
- `app.cpp`의 전체 `::Instance()` 38 -> 2 이하(허용: `FontManager` 계열 임시 잔존)

## W5. 정적 게이트 스크립트 도입
### 작업
- `scripts/refactoring/check_phase7_runtime_composition.ps1` 작성
- 점검 항목:
  - runtime/binding 신규 파일 존재
  - `main.cpp`, binding 파일의 `::Instance()` 잔존 여부
  - `app.cpp`의 feature singleton lookup 잔존 여부
  - 소스 트리 `DECLARE_SINGLETON` 선언 수가 baseline(11)을 초과하지 않는지
  - 신규 `shell/runtime`, `platform/wasm` 코드에 Doxygen 계약 주석 존재 여부
- 게이트 로그 경로 확정:
  - `docs/refactoring/phase7/logs/check_phase7_runtime_composition_latest.txt`

### 영향 파일(예상)
- `scripts/refactoring/check_phase7_runtime_composition.ps1`
- `docs/refactoring/phase7/logs/check_phase7_runtime_composition_latest.txt`

### 완료 기준
- 반복 실행 가능한 정적 회귀 게이트가 PASS 상태로 고정됨

## W6. 컴파일/테스트 및 종료 판정 문서화
### 작업
- 빌드/테스트 실행(권장):
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
- 로그 기록:
  - `docs/refactoring/phase7/logs/build_phase7_latest.txt`
  - `docs/refactoring/phase7/logs/unit_test_phase7_latest.txt`
  - `docs/refactoring/phase7/logs/smoke_phase7_latest.txt`
- 종료 문서 작성:
  - `docs/refactoring/phase7/dependency_gate_report.md`
  - `docs/refactoring/phase7/go_no_go_phase8.md`

### 완료 기준
- Phase 8 착수 판정 가능한 gate 문서/로그 패키지 완성

## 5. 상세 일정(권장)
- Day 1: W0~W1
- Day 2: W2
- Day 3: W3
- Day 4: W4
- Day 5: W5~W6

예상 공수: 3~5 MD

## 6. 파일별 변경 계획

### 신규 파일
- `docs/refactoring/phase7/refactoring_phase7_composition_root_singleton_quarantine_260331.md`
- `webassembly/src/shell/runtime/workbench_runtime.h`
- `webassembly/src/shell/runtime/workbench_runtime.cpp`
- `webassembly/src/platform/wasm/workbench_bindings.cpp`
- `scripts/refactoring/check_phase7_runtime_composition.ps1`
- `docs/refactoring/phase7/dependency_gate_report.md`
- `docs/refactoring/phase7/go_no_go_phase8.md`
- `docs/refactoring/phase7/logs/*`

### 주요 수정 파일
- `webassembly/src/main.cpp`
- `webassembly/src/app.cpp`
- `webassembly/src/app.h`
- `webassembly/src/bind_function.cpp` (정리 대상)
- `CMakeLists.txt`
- `webassembly/cmake/modules/wb_ui.cmake` 또는 관련 모듈 파일

## 7. 커밋/동기화 전략
1. 커밋 1: W0 기준선/문서 스캐폴딩
2. 커밋 2: W1 runtime 스켈레톤 + CMake 등록
3. 커밋 3: W2 main 진입점 runtime 전환
4. 커밋 4: W3 wasm binding 분리/경유화
5. 커밋 5: W4 app singleton quarantine
6. 커밋 6: W5 정적 게이트 스크립트 + 로그
7. 커밋 7: W6 빌드/테스트 로그 + gate 문서

동기화 규칙:
- 각 WBS 종료 시 로컬 커밋
- 원격 push는 Phase 7 종료 선언 직후 수행

## 8. 리스크 및 대응
1. 리스크: runtime 도입 시 초기화 순서 변경으로 UI/렌더 시작 회귀 발생
   - 대응: W2에서 기존 main loop 순서를 유지하고 smoke 테스트로 즉시 확인
2. 리스크: binding 함수 시그니처 변경으로 페이지 테스트 seam 붕괴
   - 대응: export 이름/인자 타입 호환 유지, W3에서 Playwright smoke 재검증
3. 리스크: `app.cpp` 전환 중 메뉴 액션 누락
   - 대응: menu command 매핑표를 기준으로 항목별 체크리스트 검증
4. 리스크: `DECLARE_SINGLETON` 신규 추가가 코드 리뷰에서 누락
   - 대응: W5 정적 게이트에서 선언 수/위치를 자동 검사
5. 리스크: runtime 파일 추가 후 CMake 소유권 불일치
   - 대응: 모듈 소스 등록 변경을 W1에서 즉시 반영하고 build 로그로 확인

## 9. Phase 7 완료(DoD)
- `WorkbenchRuntime`가 도입되고 `main.cpp`가 runtime 중심 진입점으로 전환됨
- wasm binding이 runtime public API만 호출함
- `app.cpp`의 feature singleton 직접 lookup이 0건 또는 quarantine 규칙 수준으로 제한됨
- 신규 `DECLARE_SINGLETON` 도입 0건(기존 baseline 11 유지)
- `check_phase7_runtime_composition.ps1` PASS
- `npm run build-wasm:release`, `npm run test:cpp`, `npm run test:smoke` 결과 로그 확보
- `dependency_gate_report.md`, `go_no_go_phase8.md` 작성 완료

## 10. 착수 체크리스트
- [x] Phase 6 종료 `GO` 확인
- [x] Phase 7 기준선 수치 실측
- [ ] W0 착수/브랜치 셋업
- [ ] W1 runtime 스캐폴딩
- [ ] W2 main.cpp runtime 전환
- [ ] W3 wasm binding 분리/경유화
- [ ] W4 app.cpp singleton quarantine
- [ ] W5 정적 게이트 도입
- [ ] W6 컴파일/테스트 + 종료 판정 문서화

## 11. Phase 7 함수/진입점 매핑표
> 표기  
> - 전환: 기존 호출 경로를 runtime API로 치환  
> - 유지: 호출 이름은 유지하되 내부 구현만 runtime 경유로 변경

| WBS | 기존 위치 | 클래스/함수(현행) | 목표 위치/API | 방식 | 비고 |
|---|---|---|---|---|---|
| W2 | `webassembly/src/main.cpp` | `main()` 내부 `App::Instance`, `MeshManager::Instance`, `FontManager::Instance` 초기화 | `shell/runtime/WorkbenchRuntime::InitializeAndRunFrame()` 계열 | 전환 | main의 singleton 직접 참조 제거 |
| W3 | `webassembly/src/bind_function.cpp` | `getStructureCount`, `getCurrentStructureId`, `isStructureVisible`, `setStructureVisible`, `getMeshCount`, `hasChargeDensity` | `platform/wasm/workbench_bindings.cpp` -> `WorkbenchRuntime` query/command | 전환 | JS export 이름은 유지 |
| W3 | `webassembly/src/bind_function.cpp` | `loadArrayBuffer`, `loadChgcarFile`, `handleXSFGridFile`, `handleStructureFile` | runtime import facade API | 전환 | FileLoader static 직접 바인딩 축소 |
| W4 | `webassembly/src/app.cpp` | `FileLoader::Instance().RequestOpenStructureImport()` | `WorkbenchRuntime::RequestOpenStructureImport()` | 전환 | File 메뉴 경유 |
| W4 | `webassembly/src/app.cpp` | `AtomsTemplate::Instance().RequestBuilderSection/DataMenu/...` | `WorkbenchRuntime::DispatchShellCommand(...)` | 전환 | 메뉴 액션 집약 |
| W4 | `webassembly/src/app.cpp` | `VtkViewer::Instance()` 관련 설정/렌더 호출 | `WorkbenchRuntime::RenderViewer()/SetViewerOption(...)` | 전환 | viewer 경계 단일화 |
| W4 | `webassembly/src/app.cpp` | `ModelTree/MeshDetail/MeshGroupDetail` 직접 렌더 호출 | `WorkbenchRuntime` panel facade | 전환 | panel quarantine |
| W5 | `scripts/refactoring/*` | 부재 | `check_phase7_runtime_composition.ps1` | 신규 | singleton 유입/경계 위반 자동 감시 |

## 12. Phase 7 실행 순서(작업자 기준)
1. baseline과 목표 수치를 먼저 고정한다(W0).
2. runtime API 골격을 만든 뒤(W1), main 진입점을 먼저 전환한다(W2).
3. binding 경계를 runtime으로 이동해 외부 진입점을 정리한다(W3).
4. 마지막으로 app 메뉴/패널 호출을 quarantine 규칙으로 수렴한다(W4).
5. 정적 게이트와 종료 문서 패키지를 완성해 Phase 8로 넘긴다(W5~W6).

## 13. Phase 7 수용 기준(정량)
| 항목 | Baseline | Target | 검증 방식 |
|---|---:|---:|---|
| `webassembly/src/shell/runtime/workbench_runtime.h` 존재 | 0 | 1 | 파일 존재 확인 |
| `webassembly/src/platform/wasm/workbench_bindings.cpp` 존재 | 0 | 1 | 파일 존재 확인 |
| `main.cpp`의 `::Instance()` 호출 수 | 3 | 0 | 정적 게이트 |
| 바인딩 계층(`bind_function.cpp` 또는 신규 bindings 파일)의 `::Instance()` 호출 수 | 6 | 0 | 정적 게이트 |
| `app.cpp` feature singleton lookup 수 | 36 | 0 | 클래스 패턴 기반 정적 게이트 |
| `app.cpp` 전체 `::Instance()` 호출 수 | 38 | <=2 | 정적 게이트 |
| `DECLARE_SINGLETON` 선언 수(매크로 제외) | 11 | <=11 | 정적 게이트 |
| Phase 7 전용 게이트 스크립트 | 0 | 1 | `check_phase7_runtime_composition.ps1` |
| 컴파일/테스트 로그 패키지 | 0 | 3종 이상 | `docs/refactoring/phase7/logs/*` |

