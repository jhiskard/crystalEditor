# Phase 1 세부 작업계획서: 의존 경계 정리

작성일: 2026-03-24
대상 범위: `webassembly/src` (특히 `atoms`, `model_tree`, `render` 경계)

## 1. Phase 1 목표

## 목표
- `atoms` 모듈과 상위 앱/뷰어 계층 간 결합을 낮추고, 다음 Phase에서 상태 통합(Phase 2)과 Import 분해(Phase 3)가 가능한 구조로 만든다.

## 핵심 결과물
1. `ModelTree`가 `atoms::domain` 전역 컨테이너를 직접 읽지 않음
2. `atoms` 하위(domain/infrastructure)에서 `VtkViewer::Instance()` 직접 호출 경로 축소(렌더 게이트웨이 도입)
3. `atoms` 내부의 상위 모듈 직접 include(`../app.h`, `../vtk_viewer.h`, `../model_tree.h` 등)를 경계 규칙에 맞게 정리

## 비목표(Phase 1에서 하지 않음)
- 전역 상태(`extern`)의 본격 제거/통합 (Phase 2)
- FileLoader 트랜잭션 분해 (Phase 3)
- 대규모 UI 분해 (`atoms_template.cpp` 본격 분할, Phase 4)

## 2. 현재 문제 요약(착수 근거)

1. `ModelTree`가 `createdAtoms`, `createdBonds`, `surroundingAtoms`, `surroundingBonds`, `bondGroups`를 직접 사용
- 위치: `webassembly/src/model_tree.cpp`

2. `atoms` 하위에서 상위 모듈 역참조
- 예: `webassembly/src/atoms/atoms_template.cpp`의 `#include "../app.h"`, `#include "../vtk_viewer.h"`, `#include "../model_tree.h"`, `#include "../mesh_manager.h"`

3. `VtkViewer::Instance()` 직접 호출이 `atoms_template`, `atoms/infrastructure`, `atoms/domain` 전반에 다수 존재

## 3. 작업 원칙
- 동작 보존 우선(behavior-preserving refactor)
- 파사드/인터페이스 먼저 도입하고 내부 구현은 점진 치환
- 한 번에 크게 바꾸지 않고 “컴파일 가능한 작은 단위 커밋”으로 진행

## 4. 작업 단위(WBS)

## W1. 경계 규칙/가드레일 설정

### 작업
- 경계 규칙 명시
  - `atoms/domain` -> `app`, `vtk_viewer`, `model_tree`, `mesh_*` 직접 include 금지
  - `atoms/infrastructure` -> `VtkViewer::Instance()` 직접 호출 금지(게이트웨이 사용)
- 검증 커맨드 추가(수동/CI 전 단계)

### 산출물
- `docs/refactoring/phase1_boundary_rules.md` (신규)
- 검증 스크립트(선택): `scripts/check_phase1_boundaries.sh`

### 완료 기준
- 아래 검증이 문서화됨

```bash
rg -n '#include "../(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"' webassembly/src/atoms
rg -n 'VtkViewer::Instance\(' webassembly/src/atoms/domain webassembly/src/atoms/infrastructure
rg -n 'using atoms::domain::(createdAtoms|createdBonds|surroundingAtoms|surroundingBonds|bondGroups)' webassembly/src/model_tree.cpp
```

## W2. ModelTree -> Atoms 전역 직접 접근 제거

### 작업
1. Read Model 인터페이스 도입
- 신규(예시)
  - `webassembly/src/atoms/application/structure_read_model.h`
  - `webassembly/src/atoms/application/structure_read_model.cpp`
- 내용
  - 구조 트리 렌더에 필요한 DTO 정의
    - `StructureSnapshot`
    - `AtomNodeView`, `BondNodeView`, `MeasurementNodeView`

2. 기존 전역 컨테이너 의존 대체
- 변경 대상
  - `webassembly/src/model_tree.cpp`
- 작업
  - `using atoms::domain::...` 제거
  - 직접 순회 코드를 `StructureReadModel` 호출로 치환

3. 임시 어댑터 구현
- 초기 구현은 내부적으로 기존 데이터를 참조해도 허용(Phase 2에서 상태 저장소 전환 예정)
- 외부(`ModelTree`)에는 DTO만 노출

### 산출물
- `ModelTree`는 전역 컨테이너 타입을 직접 모름

### 완료 기준
- `model_tree.cpp`에서 아래 패턴 0건

```bash
rg -n 'using atoms::domain::(createdAtoms|createdBonds|surroundingAtoms|surroundingBonds|bondGroups)' webassembly/src/model_tree.cpp
```

## W3. RenderGateway 도입 (VtkViewer 직접 호출 축소)

### 작업
1. 인터페이스 정의
- 신규(예시)
  - `webassembly/src/render/application/render_gateway.h`
- 최소 API
  - `RequestRender()`
  - `AddActor/RemoveActor`
  - `AddActor2D/RemoveActor2D`
  - `AddVolume/RemoveVolume`

2. VTK 구현체 추가
- 신규(예시)
  - `webassembly/src/render/infrastructure/vtk_render_gateway.h`
  - `webassembly/src/render/infrastructure/vtk_render_gateway.cpp`
- 내부에서만 `VtkViewer::Instance()` 사용

3. atoms 하위 치환 (우선순위 높은 파일부터)
- 1차 대상
  - `webassembly/src/atoms/infrastructure/batch_update_system.cpp`
  - `webassembly/src/atoms/infrastructure/vtk_renderer.cpp`
  - `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp`
  - `webassembly/src/atoms/infrastructure/bz_plot_layer.cpp`
  - `webassembly/src/atoms/domain/bond_manager.cpp`
- 방식
  - 생성자/초기화 단계에서 `RenderGateway*` 주입
  - 직접 호출을 `gateway->...`로 변환

### 산출물
- atoms infra/domain에서 render 호출은 게이트웨이를 통해서만 수행

### 완료 기준
- 아래 검사에서 `atoms/domain`, `atoms/infrastructure` 0건

```bash
rg -n 'VtkViewer::Instance\(' webassembly/src/atoms/domain webassembly/src/atoms/infrastructure
```

## W4. atoms 상위 include 정리 (의존 역전 시작)

### 작업
1. include 계층 정리
- 대상: `webassembly/src/atoms/**`
- 도메인/인프라에서 상위 include 제거
  - 제거 후보: `../app.h`, `../../app.h`, `../../vtk_viewer.h`, `../model_tree.h`, `../mesh_detail.h`, `../mesh_manager.h`

2. 포트 인터페이스로 대체
- 신규(예시)
  - `webassembly/src/atoms/application/ports/mesh_query_port.h`
  - `webassembly/src/atoms/application/ports/ui_action_port.h`
- 설명
  - domain/infrastructure가 필요한 최소 기능만 포트로 의존

3. `AtomsTemplate` 역할 조정
- `AtomsTemplate`는 orchestrator/facade 역할 유지
- 내부 구현은 포트/서비스로 위임

### 산출물
- include 경계가 파일 레벨에서 명확

### 완료 기준
- `atoms/domain`, `atoms/infrastructure`에서 상위 include 패턴 제거

```bash
rg -n '#include ".*\.{2}/(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"' webassembly/src/atoms/domain webassembly/src/atoms/infrastructure
```

## W5. 빌드/회귀 검증

### 작업
- 컴파일/런타임 스모크 시나리오 확인
  - 구조 파일 import(XSF/CHGCAR)
  - Model Tree 렌더
  - atoms/bonds visibility 토글
  - isosurface/slice 기본 동작

### 완료 기준
- 빌드 성공
- 기능 스모크 시나리오 통과
- 성능/메모리 현저 회귀 없음

## 5. 상세 일정(권장)

- Day 1
  - W1 완료(규칙+검증 명세)
  - W2 설계/DTO 인터페이스 초안
- Day 2~3
  - W2 구현(`ModelTree` 전역 접근 제거)
- Day 4~5
  - W3 구현(RenderGateway + 1차 치환)
- Day 6~7
  - W4 구현(include 정리 + port 도입)
- Day 8
  - W5 검증/버그픽스/문서 업데이트

총 예상 공수: 7~8 MD (중~고난도)

## 6. 파일별 변경 계획(Phase 1 범위)

## 신규 파일(예시)
- `webassembly/src/atoms/application/structure_read_model.h`
- `webassembly/src/atoms/application/structure_read_model.cpp`
- `webassembly/src/render/application/render_gateway.h`
- `webassembly/src/render/infrastructure/vtk_render_gateway.h`
- `webassembly/src/render/infrastructure/vtk_render_gateway.cpp`
- `webassembly/src/atoms/application/ports/mesh_query_port.h`
- `webassembly/src/atoms/application/ports/ui_action_port.h`

## 수정 대상(우선순위)
1. `webassembly/src/model_tree.cpp`
2. `webassembly/src/atoms/infrastructure/batch_update_system.cpp`
3. `webassembly/src/atoms/infrastructure/vtk_renderer.cpp`
4. `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp`
5. `webassembly/src/atoms/infrastructure/bz_plot_layer.cpp`
6. `webassembly/src/atoms/domain/bond_manager.cpp`
7. `webassembly/src/atoms/atoms_template.cpp`
8. `CMakeLists.txt` (신규 파일 빌드 편입)

## 7. 커밋 전략
- 커밋 1: 경계 규칙/검증 스크립트
- 커밋 2: StructureReadModel 도입 + ModelTree 전환
- 커밋 3: RenderGateway 인터페이스/구현 추가
- 커밋 4: atoms infra/domain gateway 치환
- 커밋 5: include 정리 + 포트 도입
- 커밋 6: 빌드/스모크 수정 + 문서 동기화

## 8. 리스크 및 대응

1. 리스크: ModelTree 데이터 정렬/표시 회귀
- 대응: 기존 정렬 기준(원자 serial, bond label)을 DTO에 명시하여 동일 유지

2. 리스크: RenderGateway 치환 중 누락
- 대응: `rg 'VtkViewer::Instance\('` 검사로 누락 자동 탐지

3. 리스크: include 제거 중 순환 참조
- 대응: 포트 인터페이스 우선 도입, 구현체는 상위 모듈에 배치

4. 리스크: 런타임 성능 저하
- 대응: 게이트웨이는 얇은 래퍼로 유지(가상 호출 최소화/인라인 고려)

## 9. Phase 1 완료(DoD)
- `ModelTree`가 atoms 전역 컨테이너를 직접 참조하지 않음
- `atoms/domain`, `atoms/infrastructure`에서 `VtkViewer::Instance()` 직접 호출이 0건
- 의존 경계 규칙 위반 패턴이 정의된 검사에서 0건
- 빌드 및 스모크 시나리오 통과

## 10. 착수 체크리스트
- [ ] 경계 규칙 문서 작성
- [ ] DTO/ReadModel 인터페이스 설계 확정
- [ ] RenderGateway 최소 API 확정
- [ ] CMake 신규 파일 편입 계획 확정
- [ ] 스모크 시나리오 체크리스트 고정

## 11. Phase 1 클래스/함수 이동 매핑표

> 범례  
> - 이동: 기존 함수/로직을 신규 파일로 추출  
> - 위임: 기존 함수는 유지하고 구현만 신규 서비스/포트로 위임  
> - 치환: 외부 의존 호출(`VtkViewer::Instance()` 등)을 게이트웨이 호출로 변경

| WBS | 기존 위치 | 클래스/함수(현행) | 신규 모듈(목표 위치) | 방식 |
|---|---|---|---|---|
| W2 | `webassembly/src/model_tree.cpp` | `using atoms::domain::createdAtoms/surroundingAtoms` | `webassembly/src/atoms/application/structure_read_model.h` | 직접 접근 제거(치환) |
| W2 | `webassembly/src/model_tree.cpp` | `using atoms::domain::createdBonds/surroundingBonds` | `webassembly/src/atoms/application/structure_read_model.h` | 직접 접근 제거(치환) |
| W2 | `webassembly/src/model_tree.cpp` | `ModelTree::renderXsfStructureTable` 내부 원자 집계 블록(`atomsBySymbol`, `surroundingByOriginalAtomId`) | `webassembly/src/atoms/application/structure_read_model.cpp` (`buildAtomTreeSnapshot`) | 이동 |
| W2 | `webassembly/src/model_tree.cpp` | `ModelTree::renderXsfStructureTable` 내부 결합 집계 블록(`bondsByType`) | `webassembly/src/atoms/application/structure_read_model.cpp` (`buildBondTreeSnapshot`) | 이동 |
| W2 | `webassembly/src/model_tree.cpp` | `resolveBondAtomByIndex` 람다 | `webassembly/src/atoms/application/structure_read_model.cpp` (`resolveBondEndpoints`) | 이동 |
| W2 | `webassembly/src/model_tree.cpp` | `buildAtomDisplayLabel` 람다 | `webassembly/src/atoms/application/structure_read_model.cpp` (`buildAtomDisplayLabel`) | 이동 |
| W2 | `webassembly/src/model_tree.cpp` | `ModelTree::renderXsfStructureTable`의 데이터 조회 진입점 | `webassembly/src/atoms/application/structure_read_model.h` (`getStructureSnapshot`) | 위임 |
| W3 | `webassembly/src/atoms/infrastructure/batch_update_system.cpp` | `BatchUpdateSystem::endBatch`, `forceBatchEnd` | `webassembly/src/render/application/render_gateway.h` | 치환(`RequestRender`) |
| W3 | `webassembly/src/atoms/infrastructure/bz_plot_layer.cpp` | `BZPlotLayer::ActorGroup::clear`, `add*Actor` 계열 | `webassembly/src/render/application/render_gateway.h` | 치환(`AddActor/RemoveActor`) |
| W3 | `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp` | `addIsosurface`, `clearIsosurfaces`, `updateIsosurface`, `clear`, `updateSlice` | `webassembly/src/render/application/render_gateway.h` | 치환(`AddActor/RemoveActor`) |
| W3 | `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp` | `captureRenderedSliceImage` | `webassembly/src/render/application/render_gateway.h` | 치환(`CaptureActorImage`) |
| W3 | `webassembly/src/atoms/infrastructure/vtk_renderer.cpp` | `initializeAtomGroupVTK`, `clearAtomGroupVTK`, `clearAllAtomGroupsVTK` | `webassembly/src/render/application/render_gateway.h` | 치환(`AddActor/RemoveActor`) |
| W3 | `webassembly/src/atoms/infrastructure/vtk_renderer.cpp` | `initializeBondGroup`, `updateBondGroup`, `clearBondGroup`, `clearAllBondGroups` | `webassembly/src/render/application/render_gateway.h` | 치환(`AddActor/RemoveActor`) |
| W3 | `webassembly/src/atoms/infrastructure/vtk_renderer.cpp` | `createUnitCell`, `clearUnitCell`, `createBZPlot`, `clearBZPlot`, `renderReciprocalVectors`, `renderBandpath`, `renderKpoints`, `renderSpecialPointLabels` | `webassembly/src/render/application/render_gateway.h` | 치환(`AddActor/AddActor2D/RemoveActor/RemoveActor2D`) |
| W3 | `webassembly/src/atoms/domain/bond_manager.cpp` | `updateBondGroupRenderer` 내부 렌더 갱신 호출 | `webassembly/src/atoms/application/ports/bond_render_port.h` + `webassembly/src/render/application/render_gateway.h` | 위임 + 치환 |
| W4 | `webassembly/src/atoms/domain/bond_manager.cpp` | `#include "../atoms_template.h"` 의존 | `webassembly/src/atoms/application/ports/bond_render_port.h` | include 역전(치환) |
| W4 | `webassembly/src/atoms/domain/bond_manager.cpp` | `#include "../../vtk_viewer.h"` 의존 | `webassembly/src/render/application/render_gateway.h` | include 역전(치환) |
| W4 | `webassembly/src/atoms/infrastructure/batch_update_system.cpp` | `#include "../../vtk_viewer.h"` 의존 | `webassembly/src/render/application/render_gateway.h` | include 역전(치환) |
| W4 | `webassembly/src/atoms/infrastructure/bz_plot_layer.cpp` | `#include "../../vtk_viewer.h"` 의존 | `webassembly/src/render/application/render_gateway.h` | include 역전(치환) |
| W4 | `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp` | `#include "../../vtk_viewer.h"` 의존 | `webassembly/src/render/application/render_gateway.h` | include 역전(치환) |
| W4 | `webassembly/src/atoms/infrastructure/vtk_renderer.h` | `#include "../../vtk_viewer.h"` 의존 | `webassembly/src/render/application/render_gateway.h` | include 역전(치환) |

## 12. Phase 1 실행 순서(작업팩 기준)

### P1-1. 경계 규칙 확정 (W1)
1. `docs/refactoring/phase1_boundary_rules.md` 작성
2. `scripts/check_phase1_boundaries.sh` 작성
3. 로컬에서 규칙 위반 기준선 기록(현재 위반 목록 저장)

### P1-2. ModelTree 조회 분리 (W2)
1. `StructureReadModel` DTO/인터페이스 파일 추가
2. `model_tree.cpp`의 원자/결합 데이터 준비 로직을 `StructureReadModel`로 이동
3. `model_tree.cpp`에서 `atoms::domain` 전역 `using` 제거

### P1-3. RenderGateway 도입/치환 (W3)
1. `IRenderGateway` 인터페이스 + `VtkRenderGateway` 구현 추가
2. `batch_update_system.cpp`, `bz_plot_layer.cpp`에 우선 적용
3. `charge_density_renderer.cpp`, `vtk_renderer.cpp`, `bond_manager.cpp` 순차 적용

### P1-4. include 역전 정리 (W4)
1. domain/infrastructure에서 상위 include 제거
2. 필요한 기능은 `atoms/application/ports/*`와 `render/application/*` 인터페이스로 대체
3. 컴파일 오류를 통해 누락 의존을 역추적하고 포트 최소 API로 보완

### P1-5. 회귀 점검 및 닫기 (W5)
1. 빌드 성공 확인
2. 구조 import(XSF/CHGCAR), Model Tree 토글, charge density 표시 스모크 점검
3. 경계 검사 스크립트 통과 확인 후 Phase 1 완료 처리

## 13. Phase 1 수용 기준(정량)

- `model_tree.cpp` 내 아래 패턴 0건
```bash
rg -n 'using atoms::domain::(createdAtoms|createdBonds|surroundingAtoms|surroundingBonds|bondGroups)' webassembly/src/model_tree.cpp
```

- `atoms/domain`, `atoms/infrastructure` 내 아래 패턴 0건
```bash
rg -n 'VtkViewer::Instance\(' webassembly/src/atoms/domain webassembly/src/atoms/infrastructure
```

- `atoms/domain`, `atoms/infrastructure` 내 상위 모듈 include 위반 0건
```bash
rg -n '#include ".*\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"' webassembly/src/atoms/domain webassembly/src/atoms/infrastructure
```

- 스모크 테스트 체크리스트 100% 통과
