# 리팩토링 결과보고서: Phase 7~12 비교 평가

작성일: `2026-04-06`  
대상 저장소: `vtk-workbench_jclee`  
비교 기준:
- 전체 계획서: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- Phase 7 계획서: `docs/refactoring/phase7/refactoring_phase7_composition_root_singleton_quarantine_260331.md`
- Phase 8 계획서: `docs/refactoring/phase8/refactoring_phase8_atoms_template_dismantle_260401.md`
- Phase 9 계획서: `docs/refactoring/phase9/refactoring_phase9_render_boundary_complete_260402.md`
- Phase 10 계획서: `docs/refactoring/phase10/refactoring_phase10_workspace_mesh_io_decouple_260402.md`
- Phase 11 계획서: `docs/refactoring/phase11/refactoring_phase11_shell_panel_objectization_260403.md`
- Phase 12 계획서: `docs/refactoring/phase12/refactoring_phase12_compatibility_facade_seal_ci_gate_260403.md`
- 각 Phase 게이트 문서 및 `2026-04-06` 기준 현재 코드

## 1. 종합 결론

Phase 7~12의 실행 계획은 게이트 기준으로 모두 종료 가능한 상태까지 완료되었다. 정적 아키텍처 게이트, 릴리즈 빌드, C++ 테스트, 스모크 테스트는 Phase 12 종료 시점 기준 PASS를 유지한다.

다만 마스터 계획서의 최종 DoD를 "엄격 기준"으로 대조하면, 일부 항목은 부분 달성 상태다. 특히 singleton 선언의 완전 제거, `wb_atoms` 제거, legacy alias 내부 잔존 축소는 후속 보완이 필요하다.

따라서 현재 상태의 판정은 다음과 같다.

- `Phase 7~12 실행 목표`: 완료
- `마스터 계획 최종 DoD(엄격 기준)`: 부분 달성
- `프로그램 운영 판정`: `Phase 12 Closed`, `Phase 13 N/A` (계획 범위 밖)

## 2. 핵심 측정 결과

현재 코드와 계획 기준선/목표를 대조한 핵심 지표는 아래와 같다.

| 항목 | 기준선/목표 | 현재값(2026-04-06) | 평가 |
|---|---:|---:|---|
| `DECLARE_SINGLETON` 실사용 선언 수 | 최종 DoD: 제거 또는 runtime shim 한정 | 7 (`App`, `Toolbar`, `FileLoader`, `MeshManager`, `AtomsTemplate`, `VtkViewer`, `FontManager`) | 부분 달성 |
| `::Instance()` 총 호출(`*.cpp`) | 최종 DoD: 0 또는 composition root 한정 | 104 | 부분 달성 |
| core singleton 호출(Feature app/domain, allowlist 외) | 0 | 0 (P12 게이트 PASS) | 달성 |
| `friend class` | 최종 DoD: 0 | 1 (`lcrs_tree.h`) | 부분 달성 |
| legacy alias 외부 노출(`atoms/*` 외) | 0 | 0 (P12 게이트 PASS) | 달성 |
| legacy alias 전체 잔존량(`createdAtoms|createdBonds|cellInfo`) | 최종 DoD: compatibility layer 외 노출 0 + 내부 축소 | 291 | 부분 달성 |
| `VtkViewer::Instance()` render shim 외 호출 | 0 | 0 (P12 게이트 PASS) | 달성 |
| 모듈 그래프 비순환 | true | true (P12 게이트 PASS) | 달성 |
| `wb_atoms` 제거 | 최종 DoD: 제거 | 미제거 (`wb_atoms` 참조 6건) | 미달성 |
| 대형 파일 축소 (`atoms_template.cpp`) | facade 수준 축소/제거 | 5803 lines | 미달성 |
| 회귀 게이트(architecture + build + cpp + smoke) | 동시 PASS | PASS | 달성 |

보조 지표(Phase 12 W0 대비):

- `::Instance()` 총 호출: `192 -> 104` (감소 88)
- `AtomsTemplate::Instance()`: `67 -> 24`
- `MeshManager::Instance()`: `35 -> 29`
- `FileLoader::Instance()`: `4 -> 3`
- `VtkViewer::Instance()`: `22 -> 1`
- `App::Instance()`: `15 -> 1`
- `Toolbar::Instance()`: `4 -> 1`

## 3. Phase별 비교 평가

### Phase 7. Composition Root 도입 + singleton quarantine

계획 대비 구현:
- `shell/runtime/WorkbenchRuntime` 도입 및 binding/runtime 경유 구조 정착
- 정적 게이트 `check_phase7_runtime_composition` PASS
- 빌드/테스트/스모크 PASS

평가:
- 목표 달성
- 이후 단계의 공통 기반(호출 경유 규칙)을 실질적으로 고정함

### Phase 8. `AtomsTemplate` 분해(구조/측정/밀도)

계획 대비 구현:
- `structure/measurement/density` 디렉터리 및 서비스 골격 도입
- `atoms_template.h`의 `friend class` 제거(당시 게이트 기준)
- 결합도 지표 감소

평가:
- 부분 달성
- 모듈 분해는 진행됐지만 `atoms_template.cpp`는 현재도 대형 파일로 잔존

### Phase 9. Render 경계 완성

계획 대비 구현:
- non-render 경로의 `VtkViewer::Instance()` 직접 호출 0건(P9/P12 게이트 유지)
- render gateway/adapter 경계 강화

평가:
- 목표 달성(경계 기준)
- 단, 반복 import 시나리오 버그(`P9-BUG-01`)는 `Deferred`로 이관됨

### Phase 10. Workspace/Mesh/IO 분리

계획 대비 구현:
- `workspace` 저장소, import workflow 경유, panel 경유 정리
- panel 경로의 `MeshManager::Instance()`/`FileLoader::Instance()` 직접 호출 억제

평가:
- 부분 달성
- repository/service 도입은 성공했으나 singleton 완전 제거 단계까지는 미도달

### Phase 11. Shell/Panel 객체화 + `App` 축소

계획 대비 구현:
- panel singleton 제거 목표 충족(Phase 11 기준)
- `shell_state_store`, `workbench_controller` 도입
- menu action의 controller 경유화

평가:
- 목표 달성(Phase 11 범위)
- `App`, `Toolbar` singleton은 Phase 12 이관 항목으로 남음

### Phase 12. compatibility facade 정리 + architecture seal

계획 대비 구현:
- `check_phase12_architecture_seal` 도입 및 PASS
- 모듈 그래프 비순환 고정
- build/cpp/smoke PASS
- 사후 보완: `P12-BUG-02`(Layout 1/2/3 버튼) `Resolved`

평가:
- 실행 계획 기준 달성
- 단, 마스터 최종 DoD 관점에서는 `wb_atoms` 제거 및 singleton/alias 완전 제거가 남아 부분 달성

## 4. 마스터 계획 DoD 달성도(Phase 7~12 종료 시점)

`refactoring_plan_full_modular_architecture_260331.md`의 DoD(11개) 기준 평가:

| DoD 항목 | 현재 상태 | 평가 |
|---|---|---|
| 1) 핵심 singleton 제거 또는 runtime shim 한정 | 7개 선언이 allowlist로 관리됨 | 부분 달성 |
| 2) feature/application/domain의 `::Instance()` 0건 | core singleton은 0(게이트 PASS), 전체 `::Instance()`는 잔존 | 부분 달성 |
| 3) `friend class` 0건 | 1건(`lcrs_tree.h`) allowlist | 부분 달성 |
| 4) legacy alias 외부 노출 0 | `atoms/*` 외부 0(게이트 PASS), 내부 잔존 다수 | 부분 달성 |
| 5) `wb_atoms` 제거 + 모듈 비순환 | 비순환 PASS, `wb_atoms` 미제거 | 부분 달성 |
| 6) `app.cpp` shell orchestration 축소 | direct core singleton lookup은 해소, 파일 규모/책임은 큼 | 부분 달성 |
| 7) render 외부 VTK actor/volume 직접 조작 없음 | viewer singleton 경계는 PASS, 비-render 경로 VTK 토큰 다수 잔존 | 부분 달성 |
| 8) `test:cpp`, `test:smoke`, architecture gate 동시 PASS | PASS | 달성 |
| 9) 결과보고서 + 판정 문서 작성 | 작성/갱신 완료 | 달성 |
| 10) public API Doxygen 정비 | 주요 신규 경계에 적용, 전면 감사는 미완 | 부분 달성 |
| 11) 브랜치/커밋/push 추적 가능성 | 문서상 규칙/산출 존재, 원격 이력 전수 검증은 본 보고서 범위 외 | 검증 유보 |

## 5. 계획 대비 차이점

1. 계획서는 "최종적으로 singleton/facade 제거"를 제시했지만, 실제 구현은 allowlist + budget 중심의 점진 정착 방식으로 종료되었다.
2. 모듈 그래프는 비순환으로 고정되었지만, 목표 아키텍처의 `wb_atoms` 제거까지는 진행되지 않았다.
3. legacy alias는 외부 노출 차단에는 성공했으나, `atoms` 내부의 치환/축소는 제한적으로 진행되었다.
4. 종료 문서는 계획 범위를 반영해 `Phase 13 GO`가 아니라 `Phase 13 N/A`로 정리되었다.

## 6. 현재 코드 기준 장점

1. runtime composition root 중심 구조가 고정되어 신규 경계 위반을 억제하기 쉬워졌다.
2. 아키텍처 규칙이 스크립트 게이트로 운영되어 회귀를 조기에 탐지할 수 있다.
3. import/workspace/shell 경계 분리가 진행되어 기능별 책임 추적이 쉬워졌다.
4. build/cpp/smoke 게이트를 결합한 종료 패키지가 반복 가능하게 정리되었다.
5. Phase 12 사후 회귀(`Layout 1/2/3`)까지 문서/코드/검증이 연결되었다.

## 7. 현재 코드 기준 한계

1. 핵심 singleton 선언이 여전히 7개 남아 있어 strict DoD 기준과는 차이가 있다.
2. `atoms_template.cpp`, `vtk_viewer.cpp`, `app.cpp` 등 대형 파일이 여전히 크다.
3. `wb_atoms` 타깃이 유지되어 최종 목표 모듈 구조와 불일치가 남아 있다.
4. legacy alias 내부 잔존량이 높아 구조/도메인 API의 일관성을 떨어뜨린다.
5. `P9-BUG-01`이 `Deferred` 상태로 남아 반복 시나리오의 완전 자동 검증이 부족하다.

## 8. 향후 보완 계획(운영 백로그)

Phase 13 신설 없이, 유지보수 트랙에서 아래 보완을 권고한다.

### B1. Singleton 최종 정리

- 목표:
  - core singleton 선언 7 -> 3 이하(runtime shim만)
  - `::Instance()` 총 호출 104 -> 40 이하
- 실행:
  1. `App`, `Toolbar`, `FileLoader` 진입점을 runtime API로 완전 이관
  2. `MeshManager`, `AtomsTemplate`, `VtkViewer`는 호환 shim 범위 축소
- 검증:
  - `check_phase12_architecture_seal.ps1` 확장 항목 추가

### B2. Legacy alias 내부 축소

- 목표:
  - `createdAtoms|createdBonds|cellInfo` 내부 참조 291 -> 100 이하
- 실행:
  1. `atoms/domain` 우선 치환
  2. `atoms/application`, `atoms/ui` 순차 치환
- 검증:
  - alias 인벤토리 스냅샷 자동 생성 + budget 게이트

### B3. 모듈 타깃 정리

- 목표:
  - `wb_atoms` 해체/대체 로드맵 확정
  - 비순환 유지 + 의존 방향 단순화
- 실행:
  1. `structure/measurement/density` 단위 타깃 분리 설계
  2. 단계적 링크 재배치
- 검증:
  - 모듈 그래프 스냅샷과 cycle 게이트 유지

### B4. 회귀 검증 강화

- 목표:
  - `P9-BUG-01` 상태를 `Resolved` 또는 재현 가능한 `Known issue`로 명확화
- 실행:
  1. `XSF(Grid) -> VASP -> XSF(Grid) -> VASP` 반복 시나리오 자동화
  2. smoke 케이스에 반복 import 시퀀스 추가
- 검증:
  - 버그 추적 로그 상태 갱신 + smoke PASS 증빙

## 9. 최종 판정

현재 코드 기준으로 Phase 7~12는 실행 계획과 게이트 패키지 관점에서 완료되었다. 다만 마스터 계획의 최종 구조 목표(엄격 DoD)는 일부 잔여 과제가 존재한다.

최종 판정은 다음과 같다.

- `실행 완료 판정`: 완료
- `구조 완성도 판정`: 부분 달성
- `운영 상태`: `Phase 12 Closed`, `Phase 13 N/A`, 보완은 유지보수 백로그로 이관
