# webassembly/src 전체 리팩토링 계획서 (2026-03-24)

## 1. 목적과 범위
- 목적: `webassembly/src` 전체 코드의 가독성, 유지보수성, 확장성 향상
- 범위: `webassembly/src` 하위 C++ 코드 전체 (`atoms` 포함)
- 산출물: 점진적(비파괴) 리팩토링 로드맵, 우선순위, 완료 기준

## 2. 현재 구조 진단

### 2.1 모듈 구성(현행)
- 엔트리/플랫폼: `main.cpp`, `bind_function.cpp`
- 앱 셸/UI 배치: `app.cpp`, `app.h`
- 렌더 뷰어: `vtk_viewer.cpp/h`, `toolbar.cpp/h`, `mouse_interactor_style.cpp/h`
- 메쉬 도메인: `mesh.cpp/h`, `mesh_manager.cpp/h`, `model_tree.cpp/h`, `mesh_detail.cpp/h`, `mesh_group*.cpp/h`
- 로딩 파이프라인: `file_loader.cpp/h`, `unv_reader.cpp/h`
- 원자 기능: `atoms/*` (domain/infrastructure/ui + `atoms_template.cpp/h`)

### 2.2 정량 지표(코드 기준)
- 대형 파일
  - `webassembly/src/font_manager.cpp`: 33,885 lines (`#ifdef SHOW_FONT_ICONS` 블록이 대부분)
  - `webassembly/src/atoms/atoms_template.cpp`: 6,876 lines
  - `webassembly/src/vtk_viewer.cpp`: 2,690 lines
  - `webassembly/src/atoms/ui/charge_density_ui.cpp`: 2,519 lines
  - `webassembly/src/model_tree.cpp`: 1,956 lines
  - `webassembly/src/file_loader.cpp`: 1,448 lines
- 강결합 호출 빈도
  - `AtomsTemplate::Instance()` 호출: `file_loader.cpp` 26회, `app.cpp` 16회
  - `VtkViewer::Instance()` 호출: `atoms_template.cpp` 89회, `mesh.cpp` 28회, `atoms/infrastructure/vtk_renderer.cpp` 27회
  - `MeshManager::Instance()` 호출: `mesh_detail.cpp` 32회, `atoms_template.cpp` 16회, `file_loader.cpp` 14회
- 전역 상태
  - `atoms/domain`에 `extern` 전역 컨테이너 다수 (`createdAtoms`, `createdBonds`, `atomGroups`, `bondGroups`, `cellInfo` 등)
  - `atoms_template.cpp`에도 별도 `static` 상태 존재 (`surroundingsVisible`, bond 관련 설정값 등)

### 2.3 구조적 문제
1. 레이어 역의존
- `atoms/atoms_template.cpp`가 상위 모듈 직접 참조: `../app.h`, `../vtk_viewer.h`, `../model_tree.h`, `../mesh_manager.h`
- `vtk_viewer.cpp`도 `atoms/atoms_template.h`를 직접 참조
- 결과: `atoms`가 독립 모듈이 아니라 앱 코어와 순환 결합

2. 전역 상태 분산 및 단일 소스 불명확
- `atoms/domain`의 전역 컨테이너 + `AtomsTemplate` 멤버 상태 + 파일 `static` 상태가 혼재
- 예: 주변 원자 표시 상태가 domain과 orchestrator 양쪽에 존재

3. 책임 집중(단일 책임 원칙 위반)
- `AtomsTemplate`가 UI, 도메인 조작, 렌더링 제어, 측정 툴, 구조/가시성, CHGCAR 연계까지 수행
- `FileLoader`가 파일 브라우저(JS), 파싱 스레드, import 트랜잭션, Mesh/Atoms 적용, UI 팝업까지 수행
- `ModelTree`가 트리 렌더링 + atoms/bonds 전역 데이터 탐색 + 가시성 제어까지 수행

4. 캡슐화 약화
- `friend class` 다수 (`atoms_template.h`)로 내부 상태 직접 접근 허용
- `using atoms::domain::createdAtoms` 등의 직접 접근이 타 모듈에 노출

5. 빌드 구조의 평면화
- `CMakeLists.txt`에 단일 `add_executable`로 대부분의 파일이 나열됨
- 모듈 경계(컴파일 단위)가 빌드 레벨에서 보장되지 않음

6. 레거시/미사용/불완전 아티팩트 존재
- 빌드 미포함 `.cpp`: `webassembly/src/atoms_template_periodic_table.cpp`, `webassembly/src/atoms_template_bravais_lattice.cpp`
- 불완전/미연결 헤더: `atoms/infrastructure/slice_renderer.h`, `atoms/infrastructure/rho_file_parser.h`
- 소스 디렉터리에 계획 문서 다수(`refactory*.md`)가 혼재

7. 품질 리스크
- 테스트 코드 부재(실행 가능한 unit/integration test 없음)
- 문자열/주석 인코딩 깨짐(모지바케) 구간 존재
- `FileLoader`의 detach 기반 스레드 운용으로 생명주기/예외 추적 어려움

## 3. 목표 아키텍처(제안)

### 3.1 핵심 원칙
- 경계 우선: 모듈 간 의존 방향을 강제
- 상태 단일화: 동일 의미의 상태를 하나의 저장소로 통합
- 점진적 이행: 동작 보존(behavior-preserving) + 어댑터/파사드 우선
- 렌더링/도메인 분리: 도메인은 VTK/ImGui 직접 의존 최소화

### 3.2 권장 모듈 경계
- `platform/`:
  - Emscripten 바인딩, `EM_ASM`/브라우저 연동
- `app/`:
  - 메뉴/레이아웃/창 상태 오케스트레이션
- `render/`:
  - `VtkViewer` 및 렌더 게이트웨이 인터페이스
- `mesh/`:
  - Mesh, MeshManager, MeshDetail, ModelTree용 read-model 서비스
- `atoms/`:
  - `domain`(순수 상태/규칙), `application`(use-case), `presentation`(UI), `infrastructure`(VTK/파일)
- `io/`:
  - 파일 import 오케스트레이션 + 포맷별 parser worker
- `common/`:
  - 공통 유틸, 타입, 에러/결과 모델

### 3.3 의존 방향(목표)
- `platform -> app -> application(use-case) -> domain`
- `application -> infrastructure(render/io)` (인터페이스 기반)
- `domain`은 `vtk/imgui/app`를 직접 참조하지 않음

## 4. 단계별 리팩토링 실행안

## Phase 0. 안정화/기준선 수립 (1~2일)
- 작업
  - 리팩토링 기준 태그 생성
  - 기능 스모크 시나리오 정의(구조 로드/가시성/측정/CHGCAR)
  - 소스 외 문서(`refactory*.md`)를 `docs/legacy/`로 이동 계획 수립
- 완료 기준
  - 기준 시나리오와 현재 동작을 비교 가능한 체크리스트 확보

## Phase 1. 의존 경계 정리 (3~5일)
- 작업
  - `atoms` 내부의 상위 모듈 직접 include 제거 목표 설정
  - `ModelTree`의 `atoms::domain` 전역 컨테이너 직접 접근 제거
    - 전용 조회 API(예: structure snapshot query)로 대체
  - `VtkViewer::Instance()` 직접 호출 지점 축소용 `RenderGateway` 도입
- 완료 기준
  - `atoms/domain`에서 `AtomsTemplate*` 직접 의존 제거 시작
  - `ModelTree`가 전역 컨테이너를 직접 읽지 않음

## Phase 2. 상태 저장소 통합 (4~6일)
- 작업
  - `createdAtoms/createdBonds/atomGroups/bondGroups/cellInfo`를 구조별 상태 컨테이너로 통합
  - `extern` 전역 상태를 점진적으로 멤버 기반 저장소로 이전
  - 중복 상태(`surroundingsVisible` 등) 단일화
- 완료 기준
  - `atoms/domain/*.h`의 `extern` 제거 또는 최소화
  - 구조 ID 기준 데이터 관리 일원화

## Phase 3. Import 파이프라인 분리 (4~6일)
- 작업
  - `FileLoader`를 아래로 분해
    - BrowserAdapter (JS/EM_ASM)
    - ImportOrchestrator (트랜잭션/롤백)
    - ParserWorker (XSF/GRID/CHGCAR)
    - ApplyService (Mesh/Atoms 반영)
  - `ApplyXSFParseResult`, `ApplyXSFGridParseResult`, `ApplyChgcarParseResult` 공통 흐름 통합
- 완료 기준
  - 포맷별 파싱과 적용 로직 분리
  - UI 팝업 제어와 파싱 스레드 로직 분리

## Phase 4. UI 렌더러 분해 (5~7일)
- 작업
  - `AtomsTemplate`를 기능별 Presenter/Controller로 분리
    - StructureVisibility
    - Measurement
    - ChargeDensity
    - Builder/Editor windows
  - `ModelTree`를 섹션별 컴포넌트로 분리(Structure/Atoms/Bonds/Measurements)
- 완료 기준
  - `atoms_template.cpp`와 `model_tree.cpp` 파일 크기/복잡도 유의미 감소

## Phase 5. 빌드 시스템 모듈화 (2~3일)
- 작업
  - CMake를 모듈 라이브러리 단위로 분할
    - 예: `wb_core`, `wb_render`, `wb_mesh`, `wb_atoms`, `wb_io`, `wb_ui`
  - 최종 executable은 모듈 링크만 수행
- 완료 기준
  - 신규 기능 추가 시 수정 파일이 해당 모듈 내로 제한

## Phase 6. 테스트/회귀 체계 도입 (병행)
- 작업
  - 최소 단위 테스트 대상
    - CHGCAR/XSF 파서
    - 좌표 변환(cell matrix)
    - 측정 지오메트리(distance/angle/dihedral)
  - 통합 스모크 테스트
    - 구조 import -> tree 반영 -> visibility -> render
- 완료 기준
  - 핵심 use-case 자동 검증 경로 확보

## 5. 우선순위 백로그

### P0 (즉시)
- `atoms/domain` 전역 상태 접근 경로 차단 계획 수립
- `ModelTree`의 atoms 전역 직접 접근 제거
- `FileLoader` import 흐름 공통화 설계
- 인코딩 깨진 주석 정리 규칙 수립(UTF-8 통일)

### P1 (단기)
- `RenderGateway` 도입 및 direct singleton 호출 축소
- `AtomsTemplate` 측정/전하밀도/구조가시성 서브컨트롤러 분리
- 미사용/불완전 파일 정리 (`slice_renderer.h`, `rho_file_parser.h`, 빌드 제외 cpp)

### P2 (중기)
- CMake 모듈 분할 완료
- 테스트 자동화 및 CI 훅 연결

## 6. 예상 리스크와 대응
- 리스크: 렌더링 회귀(가시성/selection/measurement)
  - 대응: Phase별 스모크 체크리스트 + 화면 기준 회귀 확인
- 리스크: 대규모 파일 분할 중 API 파편화
  - 대응: 파사드 유지(`AtomsTemplateFacade`), 내부 구현만 교체
- 리스크: import 트랜잭션 중 롤백 누락
  - 대응: ImportOrchestrator 단일 책임화 + 실패 시나리오 테스트

## 7. 완료(DoD) 기준
- 아키텍처
  - `atoms/domain`이 `app/vtk_viewer/model_tree`를 직접 include 하지 않음
  - 전역 `extern` 상태 제거 또는 구조체 저장소 1곳으로 통합
- 코드 규모
  - `atoms_template.cpp`/`model_tree.cpp`/`file_loader.cpp` 책임 분리 후 파일 크기 감소
- 유지보수성
  - 신규 import 포맷 추가 시 `Parser + ApplyService` 확장만으로 대응 가능
- 품질
  - 핵심 파서/좌표변환/측정 계산 테스트 통과

## 8. 착수 권장 순서 (실행안)
1. Phase 0 완료: 기준 시나리오와 회귀 체크리스트 고정
2. Phase 1 착수: `ModelTree` 전역 접근 제거 + RenderGateway 초안
3. Phase 2 착수: atoms 상태 저장소 통합(구조 ID 기준)
4. Phase 3 병행: FileLoader 분해 및 import 트랜잭션 공통화

---

### 부록 A. 주요 근거 파일
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/atoms/atoms_template.h`
- `webassembly/src/file_loader.cpp`
- `webassembly/src/model_tree.cpp`
- `webassembly/src/vtk_viewer.cpp`
- `webassembly/src/atoms/ui/charge_density_ui.cpp`
- `webassembly/src/atoms/domain/atom_manager.h`, `bond_manager.h`, `cell_manager.h`
- `CMakeLists.txt`

## 9. 단계별 모듈 이동 매핑표 (클래스/함수 단위)

> 아래 경로는 "신규 모듈구조 목표" 기준의 이동 대상(예정)입니다.

| Phase | 기존 파일 | 클래스/함수 | 신규 모듈(목표 위치) | 이동 형태 | 비고 |
|---|---|---|---|---|---|
| 1 | `webassembly/src/model_tree.cpp` | `ModelTree::renderXsfStructureTable` | `webassembly/src/mesh/presentation/model_tree_structure_panel.cpp` | 함수 추출 | atoms 전역 직접 참조 제거 |
| 1 | `webassembly/src/model_tree.cpp` | `ModelTree::renderMeshTable` | `webassembly/src/mesh/presentation/model_tree_mesh_panel.cpp` | 함수 추출 | Mesh 트리 렌더 책임 분리 |
| 1 | `webassembly/src/model_tree.cpp` | `ModelTree::renderDeleteConfirmPopup` | `webassembly/src/mesh/presentation/model_tree_dialogs.cpp` | 함수 이동 | UI 대화상자 분리 |
| 1 | `webassembly/src/vtk_viewer.cpp` | `VtkViewer::AddActor/RemoveActor/RequestRender` 호출부 일부 | `webassembly/src/render/application/render_gateway.h` | 인터페이스 도입 | direct singleton 호출 축소 |
| 1 | `webassembly/src/atoms/atoms_template.cpp` | `initializeAtomGroup`, `updateUnifiedAtomGroupVTK` (전역 래퍼) | `webassembly/src/atoms/infrastructure/render_bridge.cpp` | 래퍼 이동 | 전역 함수 제거 대상 |
| 1 | `webassembly/src/atoms/atoms_template.cpp` | `initializeBondGroup`, `clearAllBondGroups` (전역 래퍼) | `webassembly/src/atoms/infrastructure/render_bridge.cpp` | 래퍼 이동 | AtomsTemplate 의존 축소 |
| 1 | `webassembly/src/atoms/domain/bond_manager.cpp` | `updateBondGroupRenderer` (내부 helper) | `webassembly/src/atoms/application/bond_update_service.cpp` | 로직 이동 | domain -> application 분리 |
| 2 | `webassembly/src/atoms/domain/atom_manager.{h,cpp}` | `createdAtoms`, `surroundingAtoms`, `atomGroups` | `webassembly/src/atoms/domain/structure_state_store.h` | 상태 통합 | extern 전역 제거 |
| 2 | `webassembly/src/atoms/domain/bond_manager.{h,cpp}` | `createdBonds`, `surroundingBonds`, `bondGroups` | `webassembly/src/atoms/domain/structure_state_store.h` | 상태 통합 | 구조 ID 기반 관리 |
| 2 | `webassembly/src/atoms/domain/cell_manager.{h,cpp}` | `cellInfo`, `cellVisible` | `webassembly/src/atoms/domain/structure_state_store.h` | 상태 통합 | 단위셀 단일 소스화 |
| 2 | `webassembly/src/atoms/atoms_template.cpp` | `static surroundingsVisible` | `webassembly/src/atoms/domain/structure_state_store.h` | 상태 이전 | 중복 상태 제거 |
| 2 | `webassembly/src/atoms/atoms_template.cpp` | `static bondScalingFactor/bondToleranceFactor/bondThickness/bondOpacity/nextBondId` | `webassembly/src/atoms/domain/bond_settings.h` | 상태 이전 | 설정/카운터 분리 |
| 2 | `webassembly/src/atoms/atoms_template.cpp` | `AtomsTemplate::setSurroundingsVisible/isSurroundingsVisible` | `webassembly/src/atoms/application/structure_visibility_service.cpp` | 서비스 위임 | 상태 접근 일원화 |
| 2 | `webassembly/src/atoms/domain/atom_manager.cpp` | `generateUniqueAtomId` | `webassembly/src/atoms/domain/id_generator.h` | 함수 이동 | ID 생성 책임 분리 |
| 2 | `webassembly/src/atoms/atoms_template.cpp` | `generateUniqueBondId/resetBondIdCounter` | `webassembly/src/atoms/domain/id_generator.h` | 함수 이동 | Bond ID 통합 |
| 3 | `webassembly/src/file_loader.cpp` | `FileLoader::OpenFileBrowser/OpenStructureFileBrowser/OpenXSFGridFileBrowser/OpenChgcarFileBrowser` | `webassembly/src/io/platform/browser_file_picker.cpp` | 클래스 분리 | JS/EM_ASM 분리 |
| 3 | `webassembly/src/file_loader.cpp` | `FileLoader::processXSFFileInBackground/processXSFGridFileInBackground/processChgcarFileInBackground` | `webassembly/src/io/application/parser_worker_service.cpp` | 함수 이동 | 파싱 워커 분리 |
| 3 | `webassembly/src/file_loader.cpp` | `FileLoader::ApplyXSFParseResult` | `webassembly/src/io/application/import_apply_service.cpp` | 함수 이동 | 적용 경로 통합 |
| 3 | `webassembly/src/file_loader.cpp` | `FileLoader::ApplyXSFGridParseResult` | `webassembly/src/io/application/import_apply_service.cpp` | 함수 이동 | XSF GRID 적용 통합 |
| 3 | `webassembly/src/file_loader.cpp` | `FileLoader::ApplyChgcarParseResult` | `webassembly/src/io/application/import_apply_service.cpp` | 함수 이동 | CHGCAR 적용 통합 |
| 3 | `webassembly/src/file_loader.cpp` | `beginReplaceSceneImportTransaction/finalize.../rollback...` | `webassembly/src/io/application/import_orchestrator.cpp` | 클래스 추출 | 트랜잭션/롤백 단일 책임 |
| 3 | `webassembly/src/file_loader.cpp` | `collectRootMeshIds/hasSceneDataForStructureImport` | `webassembly/src/io/application/import_context_query.cpp` | 함수 이동 | Import 판단 로직 분리 |
| 3 | `webassembly/src/atoms/infrastructure/file_io_manager.cpp` | `parseXSFFile/parse3DGridXSFFile` | `webassembly/src/io/infrastructure/xsf_parser.cpp` | 파서 분리 | io 모듈로 승격 |
| 3 | `webassembly/src/atoms/infrastructure/chgcar_parser.cpp` | `ChgcarParser::parse` | `webassembly/src/io/infrastructure/chgcar_parser.cpp` | 모듈 이동 | atoms 의존 제거 |
| 4 | `webassembly/src/atoms/atoms_template.cpp` | `RenderPeriodicTableWindow/RenderCrystalTemplatesWindow/RenderBrillouinZonePlotWindow` | `webassembly/src/atoms/presentation/builder_windows_controller.cpp` | 함수 분리 | Builder UI 묶음 |
| 4 | `webassembly/src/atoms/atoms_template.cpp` | `RenderCreatedAtomsWindow/RenderBondsManagementWindow/RenderCellInformationWindow` | `webassembly/src/atoms/presentation/editor_windows_controller.cpp` | 함수 분리 | Editor UI 묶음 |
| 4 | `webassembly/src/atoms/atoms_template.cpp` | `RenderChargeDensityViewerWindow/RenderSliceViewerWindow/renderChargeDensityViewerContent/renderSliceViewerContent` | `webassembly/src/atoms/presentation/charge_density_windows_controller.cpp` | 함수 분리 | Data UI 묶음 |
| 4 | `webassembly/src/atoms/atoms_template.cpp` | `EnterMeasurementMode/ExitMeasurementMode/HandleMeasurementClickByPicker/RenderMeasurementModeOverlay` | `webassembly/src/atoms/application/measurement_controller.cpp` | 클래스 추출 | 측정 도메인+UI 분리 |
| 4 | `webassembly/src/atoms/atoms_template.cpp` | `createDistanceMeasurement/createAngleMeasurement/createDihedralMeasurement/createCenterMeasurement` | `webassembly/src/atoms/domain/measurement_service.cpp` | 함수 이동 | 수학/지오메트리 분리 |
| 4 | `webassembly/src/atoms/atoms_template.cpp` | `SetStructureVisible/SetUnitCellVisible/RegisterStructure/RemoveStructure/RemoveUnassignedData` | `webassembly/src/atoms/application/structure_lifecycle_service.cpp` | 함수 이동 | 구조 라이프사이클 분리 |
| 4 | `webassembly/src/atoms/atoms_template.cpp` | `SetAtomVisibleById/SetBondVisibleById/SetAtomLabelVisibleById/SetBondLabelVisibleById` | `webassembly/src/atoms/application/visibility_service.cpp` | 함수 이동 | 가시성 정책 분리 |
| 4 | `webassembly/src/atoms/ui/charge_density_ui.cpp` | `renderSlicePreview/updateSlicePreview/renderSliceControls` | `webassembly/src/atoms/presentation/slice_view_panel.cpp` | 함수 추출 | 대형 UI 파일 분해 |
| 4 | `webassembly/src/atoms/ui/charge_density_ui.cpp` | `renderIsosurfaceSection/rebuildMultipleIsosurfaces` | `webassembly/src/atoms/presentation/isosurface_panel.cpp` | 함수 추출 | Isosurface UI 분리 |
| 4 | `webassembly/src/vtk_viewer.cpp` | `processEvents` | `webassembly/src/render/application/viewer_interaction_controller.cpp` | 함수 추출 | 입력 처리 분리 |
| 4 | `webassembly/src/vtk_viewer.cpp` | `AlignCameraToCellAxis/AlignCameraToIcellAxis` | `webassembly/src/render/application/camera_align_service.cpp` | 함수 추출 | 카메라 정렬 분리 |
| 5 | `CMakeLists.txt` | 단일 `add_executable` 소스 나열 | `webassembly/cmake/modules/*.cmake` + 모듈별 `add_library` | 빌드 구조 분리 | `wb_atoms/wb_io/wb_mesh/wb_render/wb_app` |
| 5 | `webassembly/src/atoms_template_periodic_table.cpp` | 레거시 UI 구현 | `webassembly/src/atoms/presentation/*` 또는 `docs/legacy/` | 정리/이관 | 빌드 미포함 코드 처리 |
| 5 | `webassembly/src/atoms_template_bravais_lattice.cpp` | 레거시 UI 구현 | `webassembly/src/atoms/presentation/*` 또는 `docs/legacy/` | 정리/이관 | 중복 구현 제거 |
| 6 | `webassembly/src/atoms/infrastructure/chgcar_parser.cpp` | 파서 핵심 함수(`parseHeader/parseAtomPositions/parseDensityData`) | `webassembly/tests/io/chgcar_parser_test.cpp` | 테스트 추가 | 회귀 방지 |
| 6 | `webassembly/src/atoms/domain/cell_manager.cpp` | `calculateInverseMatrix/cartesianToFractional/fractionalToCartesian` | `webassembly/tests/atoms/cell_transform_test.cpp` | 테스트 추가 | 수치 안정성 검증 |
| 6 | `webassembly/src/atoms/atoms_template.cpp` | 측정 생성 계열 함수 | `webassembly/tests/atoms/measurement_geometry_test.cpp` | 테스트 추가 | distance/angle/dihedral 검증 |

### 9.1 우선 이관 대상(빅락)
- `file_loader.cpp`의 import 트랜잭션 + apply 함수군
- `model_tree.cpp`의 atoms 전역 직접 조회 구간
- `atoms_template.cpp`의 측정/구조가시성/전하밀도 UI 결합 구간

## 10. Phase 상세 계획서 링크
- [Phase 0(안정화/기준선 수립) 세부 계획서](./refactoring_phase0_stabilization_baseline_260325.md)
- [Phase 1(의존 경계 정리) 세부 계획서](./refactoring_phase1_dependency_boundary_260324.md)
