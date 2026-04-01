# 리팩토링 결과보고서: Phase 1~6 비교 평가

작성일: `2026-03-31`  
대상 저장소: `vtk-workbench_jclee`  
비교 기준:
- 전체 계획서: `docs/refactoring/refactoring_plan_260324.md`
- Phase 1 계획서: `refactoring_phase1_dependency_boundary_260324.md`
- Phase 2 계획서: `refactoring_phase2_state_store_integration_260330.md`
- Phase 3 계획서: `docs/refactoring/phase3/refactoring_phase3_import_pipeline_separation_260330.md`
- Phase 4 계획서: `docs/refactoring/phase4/refactoring_phase4_ui_renderer_split_260330.md`
- Phase 5 계획서: `docs/refactoring/phase5/refactoring_phase5_build_system_modularization_260331.md`
- Phase 6 계획서: `docs/refactoring/phase6/refactoring_phase6_test_regression_system_260331.md`
- 각 Phase 게이트 문서 및 현재 `main` 브랜치 코드

## 1. 종합 결론

Phase 1~6에서 계획한 핵심 방향은 현재 코드에 대부분 반영되었다. 특히 다음 항목은 계획 대비 실구현 일치도가 높다.

- `atoms/domain`, `atoms/infrastructure`에서의 상위 계층 역참조 정리
- `ModelTree`의 atoms 전역 직접 접근 제거
- import 파이프라인의 역할 분리
- CMake 모듈 분리 및 정적 게이트 도입
- C++ 단위 테스트 + Playwright smoke 기반 회귀 체계 도입

반면, 전체 계획서 서두에서 문제로 지적한 대형 singleton 중심 구조는 아직 완전히 해소되지 않았다. 대표적으로 `webassembly/src/atoms/atoms_template.cpp`는 현재도 `5161`라인이며, 동일 파일 안의 `VtkViewer::Instance()` 직접 호출은 `75`건이다. `app.cpp`, `mesh.cpp`, `file_loader.cpp`에도 direct singleton 접근이 다수 남아 있다.

따라서 본 저장소의 리팩토링 상태는 다음과 같이 평가한다.

- `Phase 1~6 범위 목표`: 완료
- `계획서가 지향한 최종 구조`: 부분 달성
- `다음 단계에서 계속 다뤄야 할 구조 부채`: 존재

## 2. 핵심 측정 결과

현재 `main` 코드 기준으로 확인한 정적 지표는 다음과 같다.

| 항목 | 계획 기준선/목표 | 현재 확인값 | 평가 |
|---|---:|---:|---|
| `model_tree.cpp`의 atoms 전역 direct using | 5건 -> 0건 | 0건 | 완료 |
| `atoms/domain + atoms/infrastructure`의 `VtkViewer::Instance()` 직접 호출 | 50건 -> 0건 | 0건 | 완료 |
| `atoms/domain`의 `extern` 선언 | 다수 -> 제거/최소화 | 0건 | 완료 |
| 루트 `CMakeLists.txt` 라인 수 | 177 이하 | 147 | 완료 |
| Phase 5 모듈 `.cmake` 파일 수 | 6 | 6 | 완료 |
| Phase 6 C++ 테스트 파일 수 | 4 | 4 | 완료 |
| Phase 6 fixture 파일 수 | 3 | 3 | 완료 |
| Phase 6 테스트 스크립트 수 | 3 | 3 | 완료 |
| `atoms_template.cpp` 크기 | 대형 파일 축소 필요 | 5161 lines | 잔여 과제 |

보조 사실:

- `webassembly/src/model_tree.cpp`는 현재 `34`라인의 얇은 셸이며, 실제 렌더링 책임은 `webassembly/src/mesh/presentation/*`로 이동했다.
- `webassembly/src/file_loader.cpp`는 `443`라인으로 줄어들었지만, 여전히 import lifecycle의 중심 coordinator 역할을 유지한다.
- Phase 5 게이트 로그 `docs/refactoring/phase5/logs/check_phase5_build_modules_latest.txt`는 전 항목 `PASS`이다.
- Phase 6 게이트 로그 `docs/refactoring/phase6/logs/check_phase6_test_regression_latest.txt`는 전 항목 `PASS`이다.

## 3. Phase별 비교 평가

### Phase 1. 의존 경계 정리

계획 목표:
- `ModelTree`의 atoms 전역 직접 접근 제거
- `atoms/domain`, `atoms/infrastructure`의 `VtkViewer::Instance()` 직접 호출 제거
- 상위 include 위반 정리

현재 구현:
- 렌더 경계는 `webassembly/src/render/application/render_gateway.h`, `webassembly/src/render/infrastructure/vtk_render_gateway.cpp`로 분리되었다.
- `ModelTree`는 `webassembly/src/atoms/application/structure_read_model.cpp`를 통해 읽기 모델을 사용하며, 화면 구성은 `webassembly/src/mesh/presentation/model_tree_structure_section.cpp`, `webassembly/src/mesh/presentation/model_tree_mesh_section.cpp`, `webassembly/src/mesh/presentation/model_tree_dialogs.cpp`로 분리되었다.
- 현재 측정상 `webassembly/src/model_tree.cpp`에는 atoms 전역 direct using이 없다.
- 현재 측정상 `webassembly/src/atoms/domain`, `webassembly/src/atoms/infrastructure`에는 비주석 기준 `VtkViewer::Instance()` 직접 호출이 없다.

평가:
- 계획한 경계 정리는 목표 범위 내에서 달성되었다.
- 다만 이 단계의 성과는 "도메인/인프라 계층 경계 정리"에 집중되어 있고, `AtomsTemplate`, `App`, `MeshManager`, `VtkViewer`의 상위 singleton 결합까지 해소된 것은 아니다.

### Phase 2. 상태 저장소 통합

계획 목표:
- `StructureStateStore` 도입
- atom/bond/cell 상태 관리 경로 일원화
- `extern` 기반 전역 상태 제거 또는 최소화

현재 구현:
- `webassembly/src/atoms/domain/structure_state_store.h`, `webassembly/src/atoms/domain/structure_state_store.cpp`가 추가되었다.
- atom/bond/cell 매니저는 저장소 API를 통해 상태와 ID를 읽도록 정리되었다.
- `webassembly/src/atoms/domain/atom_manager.h`, `webassembly/src/atoms/domain/bond_manager.h`, `webassembly/src/atoms/domain/cell_manager.h`에는 기존 전역 이름을 유지하는 inline 호환 브리지가 남아 있다.
- 현재 측정상 `webassembly/src/atoms/domain`에 `extern` 선언은 없다.

평가:
- 저장소는 실제로 도입되었고, `extern` 제거와 ID 생성 일원화도 달성되었다.
- 그러나 구현 방식은 "완전 전환"보다는 "저장소 + legacy alias 브리지"에 가깝다.
- `StructureReadModel`과 일부 domain/application 코드는 여전히 `createdAtoms`, `createdBonds`, `cellInfo` 같은 호환 이름을 사용한다.
- 따라서 Phase 2는 계획 범위 기준 완료로 보되, 장기적으로는 alias 제거와 DTO/API 직접 사용 전환이 필요하다.

### Phase 3. import 파이프라인 분리

계획 목표:
- `FileLoader`의 브라우저 파일 선택, 파서 실행, scene 적용, rollback 책임 분리
- import transaction을 독립 서비스 단위로 정리

현재 구현:
- `webassembly/src/io/platform/browser_file_picker.cpp`가 브라우저 파일 선택 책임을 담당한다.
- `webassembly/src/io/application/parser_worker_service.cpp`가 파서 실행 경로를 담당한다.
- `webassembly/src/io/application/import_apply_service.cpp`가 import 결과 적용을 담당한다.
- `webassembly/src/io/application/import_orchestrator.cpp`가 replace-scene transaction을 담당한다.
- `webassembly/src/file_loader.h`는 위 서비스들을 멤버로 보유하며 coordinator 역할을 수행한다.

평가:
- 계획서의 분리 방향은 구현과 잘 일치한다.
- 특히 import 실패 시 rollback, replace-scene snapshot, 브라우저 파일 선택 경로 분리까지 실제 코드에 반영되었다.
- 다만 `FileLoader` singleton 자체는 유지되어 있으므로, 완전한 use-case/service 진입점 추상화까지는 도달하지 않았다.

### Phase 4. UI / renderer 분리

계획 목표:
- `AtomsTemplate`, `ModelTree`, `VtkViewer`의 혼합 책임을 application/presentation 단위로 분리
- 측정/가시성/구조 lifecycle, viewer interaction을 별도 파일과 책임으로 정리

현재 구현:
- `webassembly/src/atoms/application/measurement_controller.cpp`
- `webassembly/src/atoms/application/structure_lifecycle_service.cpp`
- `webassembly/src/atoms/application/visibility_service.cpp`
- `webassembly/src/atoms/presentation/builder_windows_controller.cpp`
- `webassembly/src/atoms/presentation/editor_windows_controller.cpp`
- `webassembly/src/atoms/presentation/isosurface_panel.cpp`
- `webassembly/src/atoms/presentation/slice_view_panel.cpp`
- `webassembly/src/render/application/viewer_interaction_controller.cpp`
- `webassembly/src/render/application/camera_align_service.cpp`
- `webassembly/src/mesh/presentation/model_tree_*.cpp`

평가:
- 파일 분해와 책임 분산은 분명히 진행되었고, `model_tree.cpp`는 거의 셸만 남았다.
- 그러나 이 단계의 산출물 다수는 "독립 객체"보다는 기존 `AtomsTemplate` 또는 `VtkViewer` 멤버 함수 구현을 별도 cpp로 분리한 형태다.
- 즉, 파일 수준 분리는 성공했지만 객체 경계와 의존 역전까지 완결되지는 않았다.
- 특히 `atoms_template.cpp`가 여전히 5천 라인 이상이므로, 전체 계획서가 지적한 god-object 문제는 부분 개선 상태다.

### Phase 5. 빌드 시스템 모듈화

계획 목표:
- 단일 루트 CMake에서 모듈 단위 빌드 구성으로 전환
- 소스 ownership과 모듈 의존을 정적 검사 가능 상태로 정리

현재 구현:
- 루트 `CMakeLists.txt`는 현재 `147`라인이다.
- `webassembly/cmake/modules/wb_core.cmake`
- `webassembly/cmake/modules/wb_render.cmake`
- `webassembly/cmake/modules/wb_mesh.cmake`
- `webassembly/cmake/modules/wb_atoms.cmake`
- `webassembly/cmake/modules/wb_io.cmake`
- `webassembly/cmake/modules/wb_ui.cmake`
- `vtk_module_autoinit(TARGETS ${WB_MODULE_TARGETS} ${PROJECT_NAME} ...)` 구성이 반영되어 Phase 5 적용 후 발생했던 volume override 누락 문제도 수정되었다.

평가:
- 계획 대비 구현 일치도가 매우 높다.
- 모듈 파일 수, 루트 엔트리포인트 수, 소스 소유권, 중복 등록 여부가 모두 게이트 스크립트로 검증 가능해졌다.
- 다만 Phase 5 문서에도 남긴 것처럼 모듈 간 런타임/링크 결합은 아직 강하며, `wb_ui`, `wb_render`, `wb_mesh`, `wb_atoms`, `wb_io` 사이 순환 성격의 상호 의존이 남아 있다.

### Phase 6. 테스트 / 회귀 체계 도입

계획 목표:
- 순수 로직을 분리해 네이티브 C++ 단위 테스트 가능 상태로 만들기
- `/workbench` 브라우저 smoke를 자동화해 리팩토링 회귀를 잡기
- Phase 게이트 스크립트와 로그 체계를 완성하기

현재 구현:
- `webassembly/tests/CMakeLists.txt` 기반 테스트 러너가 추가되었다.
- C++ 테스트 파일 4개:
  - `webassembly/tests/io/chgcar_parser_test.cpp`
  - `webassembly/tests/io/xsf_parser_test.cpp`
  - `webassembly/tests/atoms/cell_transform_test.cpp`
  - `webassembly/tests/atoms/measurement_geometry_test.cpp`
- 순수 seam 3개:
  - `webassembly/src/io/infrastructure/xsf_parser.cpp`
  - `webassembly/src/atoms/domain/cell_transform.cpp`
  - `webassembly/src/atoms/domain/measurement_geometry.cpp`
- 브라우저 smoke:
  - `playwright.config.ts`
  - `tests/e2e/workbench-smoke.spec.ts`
- `/workbench` 테스트 seam:
  - `app/workbench/page.tsx`
  - `webassembly/src/bind_function.cpp`
- 패키지 스크립트:
  - `test:cpp`
  - `test:smoke`
  - `test:phase6`

평가:
- 계획된 테스트 체계는 실제로 구축되었고, 정적 게이트도 PASS 상태다.
- Playwright smoke는 structure import, visibility toggle, CHGCAR import, runtime error 부재까지 확인한다.
- 다만 현재 머신에서는 host-native Windows C++ 러너가 아니라 `emsdk + node` fallback을 사용하므로, 실행 환경 다양화는 후속 과제로 남는다.

## 4. 계획 대비 차이점

전체 계획과 현재 구현 사이의 중요한 차이는 아래와 같다.

### 4.1 완료되었지만 구현 형태가 달라진 부분

- Phase 4의 application/controller 분리는 "새 클래스를 중심으로 한 객체 분리"보다는 "기존 대형 클래스의 cpp 분할" 성격이 더 강하다.
- Phase 2의 상태 저장소 통합은 "legacy 이름 제거"까지는 가지 않았고 "store-backed alias 유지" 형태로 안착했다.
- Phase 3의 import 분리는 충분히 이뤄졌지만, 최상위 진입점은 여전히 `FileLoader` singleton이 관리한다.

### 4.2 계획 대비 아직 남은 항목

- `AtomsTemplate`, `VtkViewer`, `MeshManager`, `FileLoader` singleton 중심 구조의 본격 해소
- `atoms_template.cpp` 추가 축소
- store alias(`createdAtoms`, `createdBonds`, `cellInfo` 등) 제거
- module dependency cycle 완화
- host-native test runner 또는 CI 상시 검증 체계 확장

## 5. 현재 코드 기준 장점

- 도메인/인프라 계층 경계가 Phase 1 이전보다 훨씬 명확하다.
- import 경로가 서비스별로 분리되어 장애 분석과 rollback reasoning이 쉬워졌다.
- 빌드 스크립트가 모듈 단위로 나뉘어 ownership 추적이 가능해졌다.
- 리팩토링 이후 회귀를 잡을 최소 자동화 테스트 체계가 생겼다.
- 테스트를 위해 순수 계산/파서 seam을 추출해 이후 추가 검증의 기반이 마련되었다.

## 6. 현재 코드 기준 한계

- 최상위 orchestration이 아직 대형 singleton에 집중되어 있다.
- 구조 분리가 파일 단위까지는 진행됐지만, 런타임 객체 그래프와 의존 역전은 제한적으로만 개선되었다.
- `AtomsTemplate`는 여전히 원자 편집, 측정, charge density, 구조 lifecycle, overlay 렌더링 등 다수 책임을 동시에 가진다.
- build/test는 정리되었지만, architecture 관점에서 완전한 layer isolation 상태는 아니다.

## 7. 최종 판정

현재 `main` 브랜치의 구현은 Phase 1~6 세부계획서가 요구한 종료 판정 기준과 대체로 일치하며, 각 단계의 산출물과 게이트 문서도 코드 상태를 뒷받침한다. 따라서 `Phase 1~6 결과보고` 관점의 판정은 다음과 같다.

- `종합 판정`: 완료
- `판정 의미`: 각 Phase는 종료 가능한 수준으로 실제 구현되었고, 게이트와 로그 체계도 갖추어져 있다.
- `단서`: 전체 리팩토링이 "완전한 모듈 아키텍처 전환"까지 끝난 것은 아니며, legacy singleton 축소와 alias 제거는 차기 단계의 핵심 과제로 남아 있다.

## 8. 권고 후속 과제

1. `AtomsTemplate`의 측정, charge-density, selection, overlay 책임을 추가 분리한다.
2. `StructureStateStore` 직접 사용으로 점진 전환하고 legacy alias를 제거한다.
3. `FileLoader`를 singleton facade + use-case service 조합으로 더 얇게 만든다.
4. `VtkViewer::Instance()` 잔여 호출을 application/render gateway 쪽으로 더 밀어낸다.
5. CI 또는 host-native toolchain 기반으로 `test:cpp` 경로를 추가 확보한다.
