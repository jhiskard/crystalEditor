# Phase 8 세부 작업계획서: `AtomsTemplate` 해체 (structure / measurement / density 분리)

작성일: `2026-04-01 (KST)`  
최종 업데이트: `2026-04-01 (W1~W5 실행 반영)`  
기준 계획: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (Phase 8 절)  
선행 판정: **GO** (`docs/refactoring/phase7/go_no_go_phase8.md`)  
대상 범위: `webassembly/src/atoms/*`, `webassembly/src/shell/runtime/*`, `webassembly/src/platform/wasm/*`, `webassembly/src/app.cpp`, `webassembly/src/file_loader.cpp`, `webassembly/src/model_tree.cpp`, `scripts/refactoring/*`, `docs/refactoring/phase8/*`  
진행 상태: `W1~W5 완료, W6 대기`

## 0. Phase 7 종료 반영사항

### 0.1 진입 판정
- 판정: **GO**
- 근거 문서:
  - `docs/refactoring/phase7/dependency_gate_report.md`
  - `docs/refactoring/phase7/go_no_go_phase8.md`

### 0.2 Phase 8에서 반드시 유지할 보호 규칙
1. 폰트/아이콘 회귀 방지
   - `main.cpp`에서 `ImGui::CreateContext()` 이후에만 `FontRegistry()` 초기화
   - `WorkbenchRuntime::PrimeLegacySingletons()`에 `FontRegistry()`를 다시 넣지 않음
2. VASP -> XSF(Grid) 회귀 방지
   - `ChargeDensityUI::setGridDataEntries()`의 첫 grid bootstrap(`loadFromGridEntry(0)`) 유지
3. Runtime quarantine 규칙 유지
   - Phase 7에서 도입한 runtime/binding 경계와 정적 게이트 규칙을 훼손하지 않음

## 1. 착수 기준선 (2026-04-01 실측)

| 항목 | 기준값 | 비고 |
|---|---:|---|
| `webassembly/src/atoms/atoms_template.cpp` 라인 수 | 5161 | 대형 God-object 상태 |
| `webassembly/src/atoms/atoms_template.h` 라인 수 | 1285 | friend/호환 계층 과다 |
| `atoms_template.cpp`의 전체 `::Instance()` 호출 수 | 103 | 전역 singleton 결합 집중 |
| `atoms_template.cpp`의 `VtkViewer::Instance()` | 75 | render 결합 과다 |
| `atoms_template.cpp`의 `MeshManager::Instance()` | 16 | mesh 결합 잔존 |
| `atoms_template.h`의 `friend class` 수 | 9 | Phase 8 종료 시 0 목표 |
| `atoms` 트리의 `createdAtoms` 참조 수 | 117 | legacy alias 잔존 |
| `atoms` 트리의 `createdBonds` 참조 수 | 47 | legacy alias 잔존 |
| `atoms` 트리의 `cellInfo` 참조 수 | 250 | legacy alias 잔존 |
| `webassembly/src/structure` 존재 여부 | False | 신규 도입 필요 |
| `webassembly/src/measurement` 존재 여부 | False | 신규 도입 필요 |
| `webassembly/src/density` 존재 여부 | False | 신규 도입 필요 |

보조 지표(현재 분해 가능한 볼륨):
- `atoms/application`: 5 files / 778 lines
- `atoms/domain`: 26 files / 5770 lines
- `atoms/infrastructure`: 16 files / 5353 lines
- `atoms/presentation`: 4 files / 817 lines
- `atoms/ui`: 17 files / 5095 lines

## 2. Phase 8 목표/비목표

### 목표
1. `AtomsTemplate`의 책임을 `structure`, `measurement`, `density` 모듈로 분리한다.
2. `StructureStateStore`를 `structure` repository 중심 API로 전환한다.
3. `atoms_template.h`의 `friend class`를 제거(0건)한다.
4. legacy alias(`createdAtoms`, `createdBonds`, `cellInfo`) 직접 노출을 compatibility 계층으로 격리한다.
5. Phase 9(`VtkViewer` 해체)에 필요한 모듈 경계와 호출 계약을 선행 정리한다.

### 비목표
1. `VtkViewer` 내부 책임 완전 분해(Phase 9에서 수행)
2. `MeshManager`/`FileLoader` 구조 완전 해체(Phase 10에서 수행)
3. compatibility shim 최종 삭제(Phase 12에서 수행)

## 3. 책임 분해 설계(Phase 8 범위)

| 현재 `AtomsTemplate` 책임 | 목표 모듈 | Phase 8 산출 형태 |
|---|---|---|
| atom/bond/cell/structure lifecycle | `structure` | `structure/domain + application + presentation` 기본 골격 |
| 거리/각도/면각 측정 및 표시 데이터 | `measurement` | 측정 repository/service + overlay descriptor 경계 |
| CHGCAR/XSF grid, isosurface/slice 상태 | `density` | density repository/service + grid 컨텍스트 전환 API |
| BZ 관련 표시/데이터 조정 | `structure`(우선) + 필요 시 `measurement` | 책임 귀속 문서화 + 임시 shim |
| UI 트리거/패널 제어 | `presentation` 계층 | 기존 UI에서 새 service 호출로 위임 |

원칙:
1. 모듈 분해 중에도 singleton 신규 도입 금지
2. feature/application/domain에서 `ImGui`, `vtk`, `emscripten` 직접 의존 확대 금지
3. `AtomsTemplate`는 "호환 facade + 위임 라우터" 수준으로 축소

## 4. 작업 단위(WBS)

## W0. 착수 고정 및 인벤토리 캡처
### 작업
- Phase 8 기준선 고정:
  - `docs/refactoring/phase8/logs/atoms_template_inventory_phase8_latest.md`
- `AtomsTemplate` public API/내부 메서드 목록을 책임군(Structure/Measurement/Density/Legacy Shim)으로 분류
- 이동 우선순위 정의(결합도 높은 `VtkViewer::Instance()` 호출 경로 우선)

### 완료 기준
- 기준선 문서와 책임 분류표가 고정되어, 이후 단계별 변화량 추적 가능

## W1. `structure` 모듈 골격 도입 및 상태 저장소 이관 1차
### 작업
- `webassembly/src/structure/{domain,application,presentation}` 신규 도입
- 기존 `atoms/domain/structure_state_store.*`를 구조 repository 중심 API로 이관/정리
- 구조 lifecycle/가시성 관련 use-case를 `structure/application`으로 이동
- `AtomsTemplate`에서 structure 책임 호출을 새 service 위임 방식으로 전환

### 영향 파일(예상)
- `webassembly/src/structure/domain/*` (신규)
- `webassembly/src/structure/application/*` (신규)
- `webassembly/src/structure/presentation/*` (신규)
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/domain/structure_state_store.*` (이관 또는 shim)

### 완료 기준
- structure 데이터 접근의 단일 진입점이 repository API로 수렴
- `AtomsTemplate` 내 구조 관련 직접 상태 조작 코드가 감소

## W2. `measurement` 모듈 분리
### 작업
- `webassembly/src/measurement/{domain,application,presentation}` 신규 도입
- 거리/각도/면각 계산/표시 로직을 측정 서비스로 이동
- 측정 결과는 repository + read-model로 관리하고, UI는 read-model만 소비
- 기존 `AtomsTemplate` 측정 API는 compatibility shim으로 유지하되 내부는 measurement 서비스 위임

### 영향 파일(예상)
- `webassembly/src/measurement/domain/*` (신규)
- `webassembly/src/measurement/application/*` (신규)
- `webassembly/src/measurement/presentation/*` (신규)
- `webassembly/src/atoms/application/measurement_controller.*`
- `webassembly/src/atoms/domain/measurement_geometry.*`
- `webassembly/src/atoms/atoms_template.cpp`

### 완료 기준
- 측정 로직의 소유권이 `measurement` 모듈로 이동
- UI 경로에서 `AtomsTemplate` 직접 측정 연산 호출 감소

## W3. `density` 모듈 분리
### 작업
- `webassembly/src/density/{domain,application,infrastructure,presentation}` 신규 도입
- CHGCAR/XSF grid 컨텍스트, isosurface/slice 상태를 density 저장소/서비스로 이관
- `ChargeDensityUI`가 density 서비스 API를 우선 사용하도록 전환
- VASP -> XSF(Grid) 시나리오 회귀 방지 로직(첫 grid bootstrap) 유지/검증

### 영향 파일(예상)
- `webassembly/src/density/domain/*` (신규)
- `webassembly/src/density/application/*` (신규)
- `webassembly/src/density/infrastructure/*` (신규)
- `webassembly/src/density/presentation/*` (신규)
- `webassembly/src/atoms/ui/charge_density_ui.*`
- `webassembly/src/atoms/domain/charge_density.*`
- `webassembly/src/atoms/infrastructure/charge_density_renderer.*`

### 완료 기준
- density 관련 상태와 use-case가 `density` 모듈로 이동
- VASP -> XSF(Grid) 재현 테스트에서 Isosurface 누락 재발 없음

## W4. `AtomsTemplate` facade 축소 + friend/legacy alias 격리
### 작업
- `atoms_template.cpp/.h`를 compatibility shim + 라우터 수준으로 축소
- `atoms_template.h`의 `friend class` 제거(0건)
- legacy alias 직접 노출 지점을 compatibility 계층으로 격리
- `app.cpp`, `workbench_bindings.cpp`, `model_tree.cpp`에서 신규 모듈 API 우선 사용 경로 정리

### 영향 파일(예상)
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/atoms_template.h`
- `webassembly/src/app.cpp`
- `webassembly/src/platform/wasm/workbench_bindings.cpp`
- `webassembly/src/model_tree.cpp`

### 완료 기준
- `atoms_template.h` `friend class` 0건
- legacy alias 직접 참조 범위가 명시적으로 축소
- `AtomsTemplate`는 호환 계층 역할만 담당

## W5. 정적 게이트/경계 검증 스크립트 도입
### 작업
- `scripts/refactoring/check_phase8_atoms_template_dismantle.ps1` 작성
- 점검 항목(필수):
  - `structure/measurement/density` 디렉터리 및 핵심 파일 존재
  - `atoms_template.h`의 `friend class` 0건
  - `atoms_template.cpp`의 고위험 singleton 호출 감소(특히 `VtkViewer::Instance`)
  - legacy alias 직접 노출 지점 카운트 추적
  - Phase 7 보호 규칙(폰트 초기화 순서/Prime 제외, XSF bootstrap) 보존 여부
- 로그 경로 확정:
  - `docs/refactoring/phase8/logs/check_phase8_atoms_template_dismantle_latest.txt`

### 완료 기준
- 반복 실행 가능한 정적 게이트가 PASS 상태로 고정됨

## W6. 빌드/테스트/게이트 문서화 및 종료 판정
### 작업
- 실행 게이트:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase8_atoms_template_dismantle.ps1`
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
- 로그 산출:
  - `docs/refactoring/phase8/logs/check_phase8_atoms_template_dismantle_latest.txt`
  - `docs/refactoring/phase8/logs/build_phase8_latest.txt`
  - `docs/refactoring/phase8/logs/unit_test_phase8_latest.txt`
  - `docs/refactoring/phase8/logs/smoke_phase8_latest.txt`
- 종료 문서:
  - `docs/refactoring/phase8/dependency_gate_report.md`
  - `docs/refactoring/phase8/go_no_go_phase9.md`

### 완료 기준
- 정적/동적 게이트 PASS 및 종료 판정 문서 준비 완료
- Phase 9 착수 여부를 판정할 수 있는 근거 패키지 확보

## 5. 완료 기준(DoD)

1. `structure/*`, `measurement/*`, `density/*` 모듈 골격과 핵심 서비스가 빌드에 포함된다.
2. `atoms_template.h`의 `friend class`가 0건이다.
3. `AtomsTemplate`가 compatibility facade 수준으로 축소되며, 신규 기능 책임이 추가되지 않는다.
4. legacy alias(`createdAtoms`, `createdBonds`, `cellInfo`) 직접 노출 범위가 감소하고 문서화된다.
5. 폰트/아이콘 및 VASP->XSF(Grid) 회귀 방지 규칙이 유지된다.
6. 정적 게이트 + 빌드 + C++ 테스트 + smoke 테스트가 모두 PASS 한다.
7. Phase 8 gate report / go-no-go 문서가 작성된다.

## 6. 일정/커밋 전략

권장 일정(6~8일):
- Day 1: W0~W1
- Day 2~3: W2
- Day 4~5: W3
- Day 6: W4
- Day 7: W5
- Day 8: W6

커밋 단위(권장):
1. W0 기준선/문서 고정
2. W1 structure 모듈 도입
3. W2 measurement 모듈 도입
4. W3 density 모듈 도입
5. W4 AtomsTemplate facade 축소/friend 제거
6. W5 게이트 스크립트 + 로그
7. W6 빌드/테스트 로그 + 종료 문서

## 7. 리스크 및 대응

1. 리스크: 분해 중 `AtomsTemplate`와 `VtkViewer` 결합 경로가 깨져 렌더 회귀 발생
   - 대응: W1~W4 단계별로 빌드/스모크를 짧은 주기로 반복
2. 리스크: legacy alias 제거 과정에서 기존 UI 경로가 동작 중단
   - 대응: alias는 즉시 삭제하지 않고 compatibility shim으로 단계적 축소
3. 리스크: density 컨텍스트 전환 회귀(VASP -> XSF)가 재발
   - 대응: W3/W6에서 해당 시나리오를 고정 smoke 체크리스트에 포함
4. 리스크: Phase 7 보호 규칙 훼손으로 폰트/아이콘 회귀 재발
   - 대응: W5 정적 게이트에서 순서 규칙을 자동 점검

## 8. 참조 문서
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- `docs/refactoring/phase7/refactoring_phase7_composition_root_singleton_quarantine_260331.md`
- `docs/refactoring/phase7/dependency_gate_report.md`
- `docs/refactoring/phase7/go_no_go_phase8.md`

## 9. 진행 체크리스트
- [x] W0 기준선 고정
- [x] W1 structure 모듈 골격/리포지토리 도입
- [x] W2 measurement 서비스 API 도입 및 app 메뉴 경로 일부 전환
- [x] W3 density 서비스 API 도입 및 model tree 경로 일부 전환
- [x] W4 `atoms_template.h` friend class 0건 달성 + wrapper API 전환
- [x] W5 `check_phase8_atoms_template_dismantle.ps1` 작성 및 정적 게이트 PASS 로그 확보
- [ ] W6 빌드/테스트/종료 판정 문서화
