# VTK Workbench Atoms 리팩토링 계획서 (2026-03-24)

## 1) 목적
현재 `webassembly/src/atoms` 영역은 레이어(Infra/Domain/UI/Orchestration) 의도는 있으나, 기능 추가(CHGCAR 처리, Model Tree 제어 확장) 이후 책임 경계가 흐려졌습니다.  
이 문서는 **현 코드 구조를 기준으로 유지보수성/변경 용이성/회귀 안정성**을 높이는 리팩토링 실행안을 제시합니다.

## 2) 범위
- 포함
  - `webassembly/src/atoms/**`
  - `webassembly/src/model_tree.*`
  - `webassembly/src/file_loader.*`
  - `webassembly/src/mesh*` (Charge Density 연동 지점)
- 제외
  - 렌더링 엔진(VTK) 교체
  - UI 프레임워크(ImGui) 교체
  - 파일 포맷 스펙 변경(CHGCAR/XSF 자체 규칙 변경)

## 3) 현 구조 스냅샷 (근거)

| 파일 | LOC(대략) | 관찰 포인트 |
|---|---:|---|
| `webassembly/src/atoms/atoms_template.cpp` | 6876 | 오케스트레이션을 넘어 도메인/상태/UI/렌더 제어까지 집중 |
| `webassembly/src/atoms/atoms_template.h` | 1403 | Friend 다수 + 측정/구조/전하밀도/뷰 상태가 한 클래스에 공존 |
| `webassembly/src/model_tree.cpp` | 1956 | 구조/원자/결합/측정/그리드 가시성 제어가 한 렌더 루프에 결합 |
| `webassembly/src/atoms/ui/charge_density_ui.cpp` | 2519 | UI, 렌더 제어, slice 캐시, OpenGL 리소스 수명 관리 동시 수행 |
| `webassembly/src/file_loader.cpp` | 1448 | 비동기 파싱 + 메인스레드 적용 + 롤백 트랜잭션 로직 집중 |
| `webassembly/src/atoms/infrastructure/file_io_manager.cpp` | 847 | 파서 + 구조 초기화(오케스트레이션 호출) 혼재 |
| `webassembly/src/atoms/domain/bond_manager.cpp` | 900 | 도메인 규칙 + 배치 업데이트/렌더 호출 결합 |
| `webassembly/src/atoms/domain/surrounding_atom_manager.cpp` | 389 | 주변원자 계산 + 부모 오케스트레이터 의존 |

추가로 `domain/*.h`에 `extern` 글로벌 상태가 존재합니다.
- `createdAtoms`, `surroundingAtoms`, `atomGroups`
- `createdBonds`, `surroundingBonds`, `bondGroups`
- `cellInfo`, `cellVisible`

## 4) 핵심 문제 진단

## 4.1 Orchestrator 비대화 (God Object)
`AtomsTemplate`가 아래 책임을 동시에 가집니다.
- 구조 레지스트리/선택 상태
- 원자/결합 변경 반영
- 측정(거리/각/이면각/중심) 생성/표시
- CHGCAR/XSF import 적용
- 전하밀도 simple/advanced 표시 상태
- UI 모달/패널 상태

결과: 변경 영향 범위 예측이 어렵고, 회귀 위험이 큽니다.

## 4.2 Domain 순수성 훼손
Domain 계층이 `AtomsTemplate*`를 통해 상위 계층 동작(렌더/배치 반영)에 의존합니다.  
또한 글로벌 컨테이너(`extern`) 기반이라 다중 구조/동시 import/테스트 격리에 불리합니다.

## 4.3 ModelTree가 2차 오케스트레이터 역할
`model_tree.cpp`는 표시용 UI를 넘어 가시성 정책의 실질 소유자처럼 동작합니다.  
특히 measurement/grid/charge density 제어가 `AtomsTemplate` 내부 정책과 중복/교차합니다.

## 4.4 Charge Density 소스 오브 트루스 분산
아래 3군데에 상태/정책이 흩어져 있습니다.
- `AtomsTemplate` (view type, suppression, structure 연결)
- `ChargeDensityUI` (grid별 가시성, slice 설정/캐시)
- `ModelTree` (구조 트리에서 직접 표시 토글)

결과: 동일 기능 변경 시 수정 지점이 많고 불일치 버그 가능성이 큽니다.

## 4.5 Import 파이프라인 응집도 부족
`FileLoader`와 `FileIOManager`가 파싱/적용/롤백을 서로 나눠 갖지만 경계가 불명확합니다.  
CHGCAR/XSF/XSF-DATAGRID 흐름이 유사한데 DTO/트랜잭션 모델이 통합되어 있지 않습니다.

## 4.6 테스트 기반 부족
실질적인 자동 회귀 테스트가 부족하여 리팩토링 시 안전망이 약합니다.

## 5) 목표 아키텍처

## 5.1 원칙
- 도메인 규칙은 **렌더/ImGui/VTK 비의존**
- UI는 상태를 소유하지 않고 **Presenter/UseCase 호출만 수행**
- 구조 단위 상태는 단일 저장소에서 관리
- import는 공통 트랜잭션 경계로 처리
- 대규모 일괄 변경(Big-bang) 금지, 단계적 치환

## 5.2 제안 컴포넌트

1. `StructureSessionStore` (신규)
- 구조별 상태 집합 소유
- Atom/Bond/Cell/Measurement/ChargeDensityVisibility를 구조 단위로 보관
- 기존 `extern` 글로벌 상태 대체

2. `AtomsApplicationService` (신규)
- 유스케이스 오케스트레이션 전담
- 예: `ImportStructure`, `SetStructureVisibility`, `UpdateMeasurementVisibility`, `ApplyChargeDensityViewType`

3. `ImportPipeline` (신규)
- 포맷별 파서 결과를 공통 DTO(`StructureImportPayload`)로 정규화
- `begin/apply/commit/rollback` 트랜잭션 패턴 통일

4. `ChargeDensityCoordinator` (신규)
- simple/advanced/grid/volume/slice 가시성 정책 단일화
- `ModelTree`와 `ChargeDensityUI`는 coordinator API만 호출

5. `ModelTreePresenter` (신규)
- 트리 렌더링에 필요한 ViewModel 구성
- 토글 이벤트를 ApplicationService로 전달

6. `AtomsTemplate` (축소)
- Composition Root + 하위 서비스 연결만 담당
- 외부 호환 API는 유지하되 내부 위임 중심으로 전환

## 5.3 의존성 규칙
- `domain` -> 누구도 참조하지 않음(순수)
- `application` -> `domain`만 참조
- `infrastructure` -> `application` 포트 구현
- `ui/presentation` -> `application` 호출
- `atoms_template` -> 위 계층 조립 및 호환 레이어

## 6) 단계별 리팩토링 실행안

## Phase 0. 안전망 확보 (1주)
- 현 동작 스냅샷 수집
  - 샘플 파일: XSF, XSF+DATAGRID_3D, CHGCAR
  - 검증 시나리오: 로드/표시 토글/측정 생성/구조 삭제
- 로그 기준선 정리(`SPDLOG` key event)
- 최소 통합 테스트 하니스(헤드리스 가능 범위) 구축

산출물
- `docs/refactor-baseline.md`
- import/visibility/measurement smoke test 스크립트

## Phase 1. 상태 저장소 분리 (1~2주)
- `StructureSessionStore` 도입
- `domain` 글로벌 `extern` 제거 경로 마련
  - 우선 읽기 전용 어댑터 -> 점진 치환
- 구조별 데이터 접근을 `structureId` 중심으로 강제

영향 파일(우선)
- `atoms/domain/atom_manager.*`
- `atoms/domain/bond_manager.*`
- `atoms/domain/cell_manager.*`
- `atoms/atoms_template.*`

완료 기준
- 신규 코드에서 글로벌 컨테이너 직접 접근 금지

## Phase 2. ImportPipeline 통합 (1~2주)
- `StructureImportPayload` 정의
- XSF/XSF-GRID/CHGCAR 적용 루틴 통합
- `FileLoader`의 rollback/finalize를 파이프라인 객체로 이동

영향 파일(우선)
- `file_loader.*`
- `atoms/infrastructure/file_io_manager.*`
- `atoms/infrastructure/chgcar_parser.*`
- `atoms/atoms_template.*`

완료 기준
- import 경로별 예외 처리/롤백 방식 일관화

## Phase 3. Charge Density 정책 단일화 (1~2주)
- `ChargeDensityCoordinator` 도입
- 가시성/뷰타입/구조연결 정책을 coordinator로 이동
- `ModelTree`, `ChargeDensityUI`, `AtomsTemplate` 중복 로직 제거

영향 파일(우선)
- `atoms/ui/charge_density_ui.*`
- `model_tree.*`
- `atoms/atoms_template.*`

완료 기준
- Charge density 상태 변경 API가 한 경로로 수렴

## Phase 4. ModelTree Presenter 분리 (1주)
- 트리 렌더링 데이터 ViewModel화
- 현재의 직접 객체 제어를 presenter 이벤트 디스패치로 전환

영향 파일
- `model_tree.*`
- `atoms/presentation/model_tree_presenter.*` (신규)

완료 기준
- `model_tree.cpp`에서 도메인/인프라 직접 의존 최소화

## Phase 5. AtomsTemplate 축소 및 정리 (1~2주)
- Measurement/StructureRegistry/Visibility를 서비스로 이관
- `friend class` 의존 축소
- 호환 API 유지 + deprecated 주석 부여

완료 기준
- `AtomsTemplate`는 facade/composition 역할 중심
- 신규 기능 추가 시 `atoms_template.cpp` 직접 수정 없이 확장 가능

## 7) 우선순위 백로그

P0 (반드시 선행)
1. `StructureSessionStore` 도입 및 글로벌 상태 접근 래핑
2. Import 공통 DTO와 트랜잭션 경계 정의
3. Charge Density 상태 변경 단일 API 정의

P1 (중요)
1. ModelTree Presenter 도입
2. Measurement 서비스 분리
3. `AtomsTemplate` public API 정리(읽기/쓰기/명령 분리)

P2 (개선)
1. 네이밍/파일 배치 정리
2. 성능 지표 수집(대용량 CHGCAR 로드 시간, 메모리)
3. 문서/개발 가이드 업데이트

## 8) 리스크 및 대응

1. 리스크: 기능 회귀(가시성 토글, 측정 표시 누락)
- 대응: Phase별 smoke test + 샘플 파일 고정

2. 리스크: 마이그레이션 중 이중 상태(기존/신규) 불일치
- 대응: 임시 동기화 어댑터 + 전환 완료 시 제거 체크리스트

3. 리스크: 파일 이동/분리로 인한 빌드 깨짐
- 대응: 단계별 컴파일 가능한 작은 PR 단위 진행

4. 리스크: 성능 저하
- 대응: import/render 주요 구간 타이밍 로그 비교

## 9) 완료 판정 지표 (Definition of Done)
- 동일 입력 파일(XSF/CHGCAR) 기준 시각 결과 기능 동등
- 구조/원자/결합/측정/그리드 가시성 토글 회귀 0건
- 신규 코드에서 domain 글로벌 상태 직접 접근 0건
- `AtomsTemplate` 책임 축소(핵심 상태 로직의 서비스 위임)
- 핵심 시나리오 자동 스모크 테스트 통과

## 10) 권장 디렉터리 재배치(점진)

```text
webassembly/src/atoms/
  application/
    atoms_application_service.h/.cpp
    import_pipeline.h/.cpp
    charge_density_coordinator.h/.cpp
  domain/
    model/
    services/
    repositories/
  infrastructure/
    parsers/
    vtk_adapters/
    mesh_adapters/
  presentation/
    model_tree_presenter.h/.cpp
  ui/
    ... (ImGui 뷰)
  atoms_template.h/.cpp   # facade + composition root
```

## 11) 실행 제안
- 빅뱅 리라이트가 아니라 **6~9주 점진 전환**을 권장합니다.
- 시작 순서는 `상태 저장소 분리 -> import 통합 -> charge density 단일화 -> model tree presenter -> orchestrator 축소`가 가장 안전합니다.
- 각 Phase 종료 시점마다 “기능 동등성 체크리스트”를 고정 수행해야 합니다.

