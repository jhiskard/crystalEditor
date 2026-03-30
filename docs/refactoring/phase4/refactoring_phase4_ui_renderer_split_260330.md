# Phase 4 세부 실행계획서: UI/렌더러 분해

작성일: `2026-03-30 (KST)`  
전제: Phase 3 종료 판정 `GO`  
- 근거: `docs/refactoring/phase3/go_no_go_phase4.md`

## 0. 착수 전 결정사항(확정)
1. Phase 3까지 변경사항은 `main`에 병합 완료 후 원격 동기화를 유지한다.
   - `main` merge 완료
   - 완료 태그: `phase3-go-20260330`
2. Phase 4는 UI/렌더 분해를 우선하고, 동작 보존(behavior-preserving)을 최우선 원칙으로 한다.
3. WBS 종료 단위마다 커밋/푸시하고, Gate PASS 후 `main`에 머지한다.

## 1. Git 운영 규칙(Phase 4 적용)
1. 브랜치 전략
   - 시작 브랜치: `main` (`phase3-go-20260330` 이후 상태)
   - 작업 브랜치: `refactor/phase4-ui-render-split`
   - 병렬 작업 시 하위 브랜치: `refactor/phase4-w{N}-{topic}`

2. 동기화 시점
   - WBS 완료 단위로 작업 브랜치에 수시 `push`
   - Phase 4 Gate PASS 직후 PR 생성 및 `main` 머지
   - 머지 직후 태그: `phase4-go-YYYYMMDD`

3. 병합 기준
   - `docs/refactoring/phase4/dependency_gate_report.md` PASS
   - WASM release build PASS
   - 수동 스모크 PASS
   - `docs/refactoring/phase4/go_no_go_phase5.md`에서 `GO`

## 2. Phase 4 목표
1. `atoms_template.cpp`의 UI/도메인/렌더 혼합 책임을 기능별 Controller/Service로 분리한다.
2. `model_tree.cpp`를 Structure/Atoms/Bonds/Measurements 섹션 단위 컴포넌트로 분해한다.
3. `charge_density_ui.cpp`와 `vtk_viewer.cpp`의 고복잡 구간을 패널/서비스 단위로 분리한다.
4. 기존 사용자 시나리오(import/visibility/measurement/charge density/camera align) 동작을 보존한다.

## 3. 완료 기준(DoD)
1. `AtomsTemplate`에서 다음 책임이 분리되어 별도 파일로 이전
   - Builder windows
   - Editor windows
   - Measurement controller
   - Structure lifecycle/visibility service
2. `ModelTree`에서 섹션 렌더링 및 다이얼로그 로직 분리
3. `ChargeDensityUI`, `VtkViewer`의 고복잡 함수 분해
4. Phase 4 게이트 문서 작성 완료
   - `docs/refactoring/phase4/dependency_gate_report.md`
   - `docs/refactoring/phase4/go_no_go_phase5.md`

## 4. 작업 원칙
1. 동작 보존 우선(behavior-preserving)
2. 호출 시그니처 대규모 변경보다 위임 경로를 먼저 구축
3. 각 WBS 종료 시점마다 빌드 가능 상태 유지
4. 임시 어댑터는 허용하되 W6에서 정리한다.

## 5. WBS

## W0. 착수/브랜치 셋업
### 작업
- `main` 최신 기준 확인 후 `refactor/phase4-ui-render-split` 브랜치 생성
- Phase 4 산출 디렉터리 생성 (`docs/refactoring/phase4`, `docs/refactoring/phase4/logs`)

### 완료 기준
- 브랜치 생성 및 초기 푸시 완료

## W1. AtomsTemplate Builder/Editor 분해
### 작업
- Builder windows 계열 함수 분리
  - `RenderPeriodicTableWindow`
  - `RenderCrystalTemplatesWindow`
  - `RenderBrillouinZonePlotWindow`
- Editor windows 계열 함수 분리
  - `RenderCreatedAtomsWindow`
  - `RenderBondsManagementWindow`
  - `RenderCellInformationWindow`

### 영향 파일(예상)
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/presentation/builder_windows_controller.*` (신규)
- `webassembly/src/atoms/presentation/editor_windows_controller.*` (신규)

### 완료 기준
- AtomsTemplate에서 Builder/Editor UI 직접 구현 비중 감소

## W2. Charge Density UI 분해
### 작업
- `charge_density_ui.cpp`에서 slice/isosurface 관련 로직 패널화
  - slice preview/control
  - isosurface section
- charge density window controller와 패널 간 경계 정리

### 영향 파일(예상)
- `webassembly/src/atoms/ui/charge_density_ui.cpp`
- `webassembly/src/atoms/presentation/slice_view_panel.*` (신규)
- `webassembly/src/atoms/presentation/isosurface_panel.*` (신규)

### 완료 기준
- `charge_density_ui.cpp` 내 대형 함수 분해 및 책임 구분

## W3. Measurement/Visibility 서비스 분해
### 작업
- 측정 관련 controller 추출
  - `EnterMeasurementMode/ExitMeasurementMode`
  - `HandleMeasurementClickByPicker`
  - `RenderMeasurementModeOverlay`
- 구조 lifecycle/visibility 관련 서비스 추출
  - `SetStructureVisible/SetUnitCellVisible/RegisterStructure/RemoveStructure/RemoveUnassignedData`
  - `SetAtomVisibleById/SetBondVisibleById/SetAtomLabelVisibleById/SetBondLabelVisibleById`

### 영향 파일(예상)
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/application/measurement_controller.*` (신규)
- `webassembly/src/atoms/application/structure_lifecycle_service.*` (신규)
- `webassembly/src/atoms/application/visibility_service.*` (신규)

### 완료 기준
- 측정/가시성 정책 로직이 AtomsTemplate에서 서비스로 위임

## W4. ModelTree 섹션 분해
### 작업
- `model_tree.cpp`를 섹션 단위 렌더 컴포넌트로 분해
  - Structure
  - Mesh
  - Atoms/Bonds
  - Measurements
- 삭제 확인/팝업 로직 분리

### 영향 파일(예상)
- `webassembly/src/model_tree.cpp`
- `webassembly/src/mesh/presentation/model_tree_*` (신규)

### 완료 기준
- `ModelTree` 파일 복잡도 및 단일 함수 길이 감소

## W5. VtkViewer 상호작용/카메라 분해
### 작업
- `vtk_viewer.cpp`에서 입력 이벤트 처리와 카메라 정렬 로직 분해
  - `processEvents`
  - `AlignCameraToCellAxis/AlignCameraToIcellAxis`

### 영향 파일(예상)
- `webassembly/src/vtk_viewer.cpp`
- `webassembly/src/render/application/viewer_interaction_controller.*` (신규)
- `webassembly/src/render/application/camera_align_service.*` (신규)

### 완료 기준
- Viewer 핵심 루프와 보조 서비스 경계 명확화

## W6. 통합/호환 정리
### 작업
- 임시 어댑터/중복 호출 경로 정리
- 신규 컴포넌트 wiring 안정화
- 리그레션 포인트(import/measurement/visibility/charge density/camera) 점검

### 완료 기준
- 기존 메뉴/단축키/UI 이벤트에서 호출 경로 변경 없이 동작

## W7. Gate 검사/문서화
### 작업
- Phase 4 게이트 스크립트/리포트 작성
- 빌드 로그 및 수동 스모크 결과 기록

### 완료 기준
- Phase 4 종료 판정 가능 상태

## 6. 정량 목표(권장)
| 항목 | Baseline | Target | 검증 방식 |
|---|---:|---:|---|
| `atoms_template.cpp` 라인 수 | 6,130 | 30% 이상 감소 | `Measure-Object -Line` |
| `model_tree.cpp` 라인 수 | 1,782 | 25% 이상 감소 | `Measure-Object -Line` |
| `charge_density_ui.cpp` 라인 수 | 2,260 | 25% 이상 감소 | `Measure-Object -Line` |
| `vtk_viewer.cpp` 라인 수 | 2,449 | 20% 이상 감소 | `Measure-Object -Line` |
| release build | PASS | PASS 유지 | `npm.cmd run build-wasm:release` |

## 7. 리스크 및 대응
1. 리스크: UI 이벤트 경로 회귀
   - 대응: 핵심 시나리오별 수동 스모크 체크리스트 운영
2. 리스크: AtomsTemplate 분해 중 상태 접근 분산
   - 대응: 서비스 인터페이스 우선 정의 + 상태 변경 경로 단일화
3. 리스크: Viewer/ModelTree 분해 시 성능 저하
   - 대응: 큰 데이터셋 기준 렌더/트리 반응성 비교 측정
4. 리스크: 기능 분해 후 파일 간 include 경계 재악화
   - 대응: Phase 1 경계 검사 규칙을 병행 적용

## 8. 커밋/동기화 전략
1. 커밋 1: W0 (브랜치 셋업)
2. 커밋 2: W1 (Builder/Editor 분해)
3. 커밋 3: W2 (Charge Density UI 분해)
4. 커밋 4: W3 (Measurement/Visibility 서비스)
5. 커밋 5: W4 (ModelTree 분해)
6. 커밋 6: W5 (VtkViewer 분해)
7. 커밋 7: W6 (통합/호환 정리)
8. 커밋 8: W7 (게이트 리포트 + Go/No-Go + 로그)

동기화 규칙:
- 각 커밋 묶음 종료 시 `push`
- Gate PASS 이후 PR/merge
- merge 후 `phase4-go-YYYYMMDD` 태그

## 9. 착수 체크리스트
- [x] Phase 3 `GO` 판정 확인
- [x] Phase 3 변경사항 `main` 머지 확인
- [x] `phase3-go-20260330` 태그 생성/원격 동기화 확인
- [x] `refactor/phase4-ui-render-split` 브랜치 생성
- [ ] W1 착수
