# Phase 4 세부 작업계획서: UI/렌더러 분해

작성일: `2026-03-30 (KST)`  
최종 업데이트: `2026-03-31 (Phase 1 양식 재정리)`  
대상 범위: `webassembly/src/atoms/*`, `webassembly/src/model_tree.cpp`, `webassembly/src/atoms/ui/charge_density_ui.cpp`, `webassembly/src/vtk_viewer.cpp`  
진행 상태: `Phase 종료 판정 완료(근거: docs/refactoring/phase4/dependency_gate_report.md, docs/refactoring/phase4/go_no_go_phase5.md)`

## 0. Phase 3 종료 반영사항

### 0.1 진입 판정
- 판정: **GO**
- 근거 문서:
  - `docs/refactoring/phase3/dependency_gate_report.md`
  - `docs/refactoring/phase3/go_no_go_phase4.md`

### 0.2 착수 기준선(Phase 3 종료 시점)
| 항목 | 기준값 | 출처 |
|---|---:|---|
| `atoms_template.cpp` 라인 수 | 6,130 | Phase 4 계획 기준선 |
| `model_tree.cpp` 라인 수 | 1,782 | Phase 4 계획 기준선 |
| `charge_density_ui.cpp` 라인 수 | 2,260 | Phase 4 계획 기준선 |
| `vtk_viewer.cpp` 라인 수 | 2,449 | Phase 4 계획 기준선 |

### 0.3 Phase 4 범위 확정
- Phase 4 핵심:
  - AtomsTemplate의 UI/정책 책임을 controller/service로 분리
  - ModelTree 섹션/다이얼로그 분리
  - ChargeDensityUI, VtkViewer 고복잡 함수 분리
- 후속 Phase 이관:
  - CMake 타깃 모듈화는 Phase 5

## 1. Phase 4 목표

### 목표
- UI/렌더 관련 대형 파일의 책임을 기능 단위로 분리하여 유지보수성과 변경 추적성을 높인다.

### 도달 결과물
1. Builder/Editor/Measurement/Visibility/Lifecycle 분리
2. ModelTree 섹션/팝업 분리
3. ChargeDensity panel 분리
4. Viewer interaction/camera align 분리

### 비목표(Phase 4에서 하지 않음)
- 빌드 시스템 타깃 모듈화(CMake 분할) (Phase 5)
- 테스트 프레임워크 고도화 (Phase 6)

## 2. 현재 문제 요약(착수 근거)
1. `atoms_template.cpp`가 UI, 정책, 렌더 제어를 과도하게 동시 담당했다.
2. `model_tree.cpp`가 다수 섹션/팝업/데이터 조합 로직을 단일 파일에서 처리했다.
3. `charge_density_ui.cpp`, `vtk_viewer.cpp` 대형 함수가 기능 변경 리스크를 높였다.

## 3. 작업 원칙
1. 동작 보존 우선(behavior-preserving)
2. 시그니처 급변보다 위임 경로 우선 구축
3. WBS 종료 단위 커밋/추적
4. 임시 어댑터 허용, W6에서 정리

## 4. 작업 단위(WBS)

## W0. 착수/브랜치 셋업
### 작업
- `refactor/phase4-ui-render-split` 브랜치 준비
- 산출 디렉터리 준비(`docs/refactoring/phase4`, `logs`)

### 완료 기준
- 브랜치 및 산출 구조 준비 완료

## W1. AtomsTemplate Builder/Editor 분해
### 작업
- Builder windows 함수 분리
- Editor windows 함수 분리

### 영향 파일(예상)
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/presentation/builder_windows_controller.cpp`
- `webassembly/src/atoms/presentation/editor_windows_controller.cpp`

### 완료 기준
- Builder/Editor 직접 구현 비중 축소

## W2. Charge Density UI 분해
### 작업
- slice preview/control 패널화
- isosurface 섹션 패널화

### 영향 파일(예상)
- `webassembly/src/atoms/ui/charge_density_ui.cpp`
- `webassembly/src/atoms/presentation/slice_view_panel.cpp`
- `webassembly/src/atoms/presentation/isosurface_panel.cpp`

### 완료 기준
- charge density 대형 함수 분해

## W3. Measurement/Visibility 서비스 분해
### 작업
- 측정 controller 추출
- 구조 lifecycle/visibility 서비스 추출

### 영향 파일(예상)
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/application/measurement_controller.cpp`
- `webassembly/src/atoms/application/structure_lifecycle_service.cpp`
- `webassembly/src/atoms/application/visibility_service.cpp`

### 완료 기준
- 측정/가시성 정책 로직의 서비스 위임 완료

## W4. ModelTree 섹션 분해
### 작업
- structure/mesh 섹션 분리
- 삭제/정리 팝업 분리

### 영향 파일(예상)
- `webassembly/src/model_tree.cpp`
- `webassembly/src/mesh/presentation/model_tree_structure_section.cpp`
- `webassembly/src/mesh/presentation/model_tree_mesh_section.cpp`
- `webassembly/src/mesh/presentation/model_tree_dialogs.cpp`

### 완료 기준
- ModelTree 파일 복잡도 감소

## W5. VtkViewer 상호작용/카메라 분해
### 작업
- 입력 이벤트 처리 분리
- 카메라 정렬 로직 분리

### 영향 파일(예상)
- `webassembly/src/vtk_viewer.cpp`
- `webassembly/src/render/application/viewer_interaction_controller.cpp`
- `webassembly/src/render/application/camera_align_service.cpp`

### 완료 기준
- Viewer 메인 루프와 보조 서비스 경계 명확화

## W6. 통합/호환 정리
### 작업
- 임시 어댑터/중복 경로 정리
- include 및 wiring 정리
- 리그레션 포인트 점검

### 완료 기준
- 호출 경로가 일관되고 잔여 중복 경로 최소화

## W7. Gate 검사/문서화
### 작업
- 정적 gate 스크립트/리포트 작성
- 빌드/수동 스모크 결과 기록

### 완료 기준
- Phase 5 착수 판정 가능한 상태

## 5. 상세 일정(권장)
- Day 1: W0~W1
- Day 2: W2
- Day 3: W3
- Day 4: W4
- Day 5: W5
- Day 6: W6~W7

예상 공수: 5~7 MD

## 6. 파일별 변경 계획

### 신규 파일
- `webassembly/src/atoms/presentation/builder_windows_controller.cpp`
- `webassembly/src/atoms/presentation/editor_windows_controller.cpp`
- `webassembly/src/atoms/presentation/slice_view_panel.cpp`
- `webassembly/src/atoms/presentation/isosurface_panel.cpp`
- `webassembly/src/atoms/application/measurement_controller.cpp`
- `webassembly/src/atoms/application/structure_lifecycle_service.cpp`
- `webassembly/src/atoms/application/visibility_service.cpp`
- `webassembly/src/mesh/presentation/model_tree_dialogs.cpp`
- `webassembly/src/mesh/presentation/model_tree_structure_section.cpp`
- `webassembly/src/mesh/presentation/model_tree_mesh_section.cpp`
- `webassembly/src/render/application/viewer_interaction_controller.cpp`
- `webassembly/src/render/application/camera_align_service.cpp`

### 주요 수정 파일
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/model_tree.cpp`
- `webassembly/src/atoms/ui/charge_density_ui.cpp`
- `webassembly/src/vtk_viewer.cpp`
- `CMakeLists.txt`

## 7. 커밋/동기화 전략
1. 커밋 1: W0 셋업
2. 커밋 2: W1 Builder/Editor
3. 커밋 3: W2 Charge Density 분해
4. 커밋 4: W3 Measurement/Visibility
5. 커밋 5: W4 ModelTree 분해
6. 커밋 6: W5 Viewer 분해
7. 커밋 7: W6 통합 정리
8. 커밋 8: W7 Gate 문서

## 8. 리스크 및 대응
1. 리스크: UI 이벤트 경로 회귀
   - 대응: 핵심 시나리오 수동 스모크
2. 리스크: 상태 접근 경로 분산
   - 대응: 서비스 인터페이스 우선 고정
3. 리스크: 분해 이후 include 경계 재악화
   - 대응: Phase 1 경계 규칙 병행 점검

## 9. Phase 4 완료(DoD)
- AtomsTemplate 핵심 책임 분리 완료
- ModelTree 섹션/팝업 분리 완료
- ChargeDensityUI/VtkViewer 고복잡 함수 분리 완료
- Gate 문서 및 종료 판정 준비 완료

## 10. 착수 체크리스트
- [x] Phase 3 종료 `GO` 확인
- [x] W0 착수/브랜치 셋업
- [x] W1 AtomsTemplate Builder/Editor 분해
- [x] W2 Charge Density UI 분해
- [x] W3 Measurement/Visibility 서비스 분해
- [x] W4 ModelTree 섹션 분해
- [x] W5 VtkViewer 상호작용/카메라 분해
- [x] W6 통합/호환 정리
- [x] W7 Gate 검사/문서화

## 11. Phase 4 클래스/함수 이동 매핑표
> 표기  
> - 이동: 기존 함수 구현을 신규 파일로 이동  
> - 위임: 기존 API는 유지하고 구현만 신규 모듈에 위임

| WBS | 기존 위치 | 클래스/함수(현행) | 신규 모듈(목표 위치) | 방식 | 비고 |
|---|---|---|---|---|---|
| W1 | `webassembly/src/atoms/atoms_template.cpp` | `RenderPeriodicTableWindow`, `RenderCrystalTemplatesWindow`, `RenderBrillouinZonePlotWindow` | `webassembly/src/atoms/presentation/builder_windows_controller.cpp` | 이동 | Builder UI 분리 |
| W1 | `webassembly/src/atoms/atoms_template.cpp` | `RenderCreatedAtomsWindow`, `RenderBondsManagementWindow`, `RenderCellInformationWindow` | `webassembly/src/atoms/presentation/editor_windows_controller.cpp` | 이동 | Editor UI 분리 |
| W2 | `webassembly/src/atoms/ui/charge_density_ui.cpp` | `renderSliceControls`, `renderSlicePreview`, `updateSlicePreviewForTarget`, `updateSlicePreview` | `webassembly/src/atoms/presentation/slice_view_panel.cpp` | 이동 | Slice 패널 분리 |
| W2 | `webassembly/src/atoms/ui/charge_density_ui.cpp` | `renderIsosurfaceSection`, `rebuildMultipleIsosurfaces` | `webassembly/src/atoms/presentation/isosurface_panel.cpp` | 이동 | Isosurface 패널 분리 |
| W3 | `webassembly/src/atoms/atoms_template.cpp` | `EnterMeasurementMode`, `ExitMeasurementMode`, `HandleMeasurementClickByPicker`, `RenderMeasurementModeOverlay` | `webassembly/src/atoms/application/measurement_controller.cpp` | 이동 | 측정 정책 분리 |
| W3 | `webassembly/src/atoms/atoms_template.cpp` | `RegisterStructure`, `RemoveStructure`, `RemoveUnassignedData` | `webassembly/src/atoms/application/structure_lifecycle_service.cpp` | 이동 | 구조 수명주기 분리 |
| W3 | `webassembly/src/atoms/atoms_template.cpp` | `SetStructureVisible`, `SetUnitCellVisible`, `SetAtomVisibleById`, `SetBondVisibleById`, `SetAtomLabelVisibleById`, `SetBondLabelVisibleById` | `webassembly/src/atoms/application/visibility_service.cpp` | 이동 | 가시성 정책 분리 |
| W4 | `webassembly/src/model_tree.cpp` | `renderDeleteConfirmPopup`, `renderClearMeasurementsConfirmPopup` | `webassembly/src/mesh/presentation/model_tree_dialogs.cpp` | 이동 | 팝업 분리 |
| W4 | `webassembly/src/model_tree.cpp` | `renderXsfStructureTable` | `webassembly/src/mesh/presentation/model_tree_structure_section.cpp` | 이동 | 구조 섹션 분리 |
| W4 | `webassembly/src/model_tree.cpp` | `renderMeshTable`, `renderMeshTree` | `webassembly/src/mesh/presentation/model_tree_mesh_section.cpp` | 이동 | 메쉬 섹션 분리 |
| W5 | `webassembly/src/vtk_viewer.cpp` | `processEvents` | `webassembly/src/render/application/viewer_interaction_controller.cpp` | 이동 | 입력 상호작용 분리 |
| W5 | `webassembly/src/vtk_viewer.cpp` | `AlignCameraToCellAxis`, `AlignCameraToIcellAxis` | `webassembly/src/render/application/camera_align_service.cpp` | 이동 | 카메라 정렬 분리 |

## 12. Phase 4 실행 순서(작업자 기준)
1. UI 책임 분리(W1~W2)
2. 정책/서비스 분리(W3)
3. 트리/뷰어 분해(W4~W5)
4. 통합 정리 및 게이트 문서화(W6~W7)

## 13. Phase 4 수용 기준(정량)
| 항목 | Baseline | Target | 검증 방식 |
|---|---:|---:|---|
| `atoms_template.cpp` 라인 수 | 6,130 | 30% 이상 감소 | 정적 측정 |
| `model_tree.cpp` 라인 수 | 1,782 | 25% 이상 감소 | 정적 측정 |
| `charge_density_ui.cpp` 라인 수 | 2,260 | 25% 이상 감소 | 정적 측정 |
| `vtk_viewer.cpp` 라인 수 | 2,449 | 20% 이상 감소 | 정적 측정 |
| 수동 스모크(import/measurement/visibility/charge density/camera) | PASS | PASS | 로그/판정 문서 |
