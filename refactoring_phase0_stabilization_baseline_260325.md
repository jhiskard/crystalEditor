# Phase 0 세부 작업계획서: 안정화/기준선 수립

작성일: 2026-03-25  
대상 범위: `webassembly/src` + 빌드/검증 스크립트 + 리팩토링 문서 체계

## 1. Phase 0 목표

## 목표
- 이후 Phase(1~6)의 변경이 회귀를 일으켰는지 판단할 수 있도록 **기능/구조/품질 기준선(Baseline)**을 수치와 체크리스트로 고정한다.
- 리팩토링 시작 전, 빌드 및 핵심 사용 시나리오를 재현 가능한 절차로 문서화한다.

## 핵심 결과물
1. 기준선 스냅샷 문서(정량 지표 + 함수 앵커 + 실행 커맨드)
2. 수동 스모크 체크리스트(구조 import/가시성/측정/CHGCAR)
3. 의존 위반 탐지 규칙 초안 및 실행 스크립트
4. 레거시 문서(`refactory*.md`) 정리 계획

## 비목표(Phase 0에서 하지 않음)
- 모듈 경계 수정/코드 이동(Phase 1부터)
- 전역 상태 통합/저장소 전환(Phase 2)
- `FileLoader` 분해(Phase 3)

## 2. 현재 기준선 스냅샷(2026-03-25 측정)

## 정량 지표
- `webassembly/src` 파일 수: **130**
- `AtomsTemplate::Instance()` 호출 수: **72**
- `VtkViewer::Instance()` 호출 수: **216**
- `atoms` 상위 include 패턴 위반 수(탐지식 기준): **15**
- 대형 파일
  - `webassembly/src/atoms/atoms_template.cpp`: 6,876 lines
  - `webassembly/src/vtk_viewer.cpp`: 2,690 lines
  - `webassembly/src/model_tree.cpp`: 1,956 lines
  - `webassembly/src/file_loader.cpp`: 1,448 lines
- 소스 디렉터리 내 레거시 계획 문서: `webassembly/src/refactory*.md` **7개**
- 테스트 현황: 실질 단위테스트 부재(확인 파일: `webassembly/src/test_window.cpp/h`)

## 기준선 측정 커맨드(재실행용)
```bash
rg --files webassembly/src | wc -l
rg -n 'AtomsTemplate::Instance\(' webassembly/src | wc -l
rg -n 'VtkViewer::Instance\(' webassembly/src | wc -l
rg -n '#include ".*\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"|#include ".*\.\./\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"' webassembly/src/atoms | wc -l
wc -l webassembly/src/atoms/atoms_template.cpp webassembly/src/vtk_viewer.cpp webassembly/src/model_tree.cpp webassembly/src/file_loader.cpp
rg --files webassembly/src | rg 'refactory.*\.md'
```

## 3. 작업 원칙
- **코드 비변경 원칙**: Phase 0에서는 운영 코드 동작을 바꾸지 않는다.
- **재현 가능성 우선**: 모든 기준선은 커맨드/경로/확인 기준을 명시한다.
- **후속 Phase 연결성**: Phase 1 착수에 바로 사용할 산출물 형식으로 작성한다.

## 4. 작업 단위(WBS)

## W0-1. 기준선 참조점 고정(Tag/Manifest)

### 작업
1. 기준선 커밋 해시 기록 (`git rev-parse HEAD`)
2. 기준선 메타 파일 작성
- 제안: `docs/refactoring/phase0/baseline_manifest_20260325.md`
- 포함: 브랜치, 커밋, 측정 일시, 측정자, 측정 커맨드

### 완료 기준
- Phase 1 이후 비교 가능한 기준선 식별자(해시/날짜) 확보

## W0-2. 빌드 기준선 고정

### 작업
1. WASM 빌드 절차 표준화
- 기준 스크립트: `build-wasm.sh`
2. 빌드 성공/실패 판단 기준 문서화
- 필수 산출물: `public/wasm/VTK-Workbench.{js,wasm,data}`
3. 빌드 로그 저장 경로 정의
- 제안: `docs/refactoring/phase0/logs/build_YYYYMMDD_HHMM.txt`

### 완료 기준
- 동일 절차로 빌드 성공 여부를 반복 확인 가능

## W0-3. 기능 스모크 시나리오 체크리스트 정의

### 작업
1. 핵심 사용자 플로우 정의
- Structure import(XSF)
- Grid/CHGCAR import
- Model Tree 렌더/토글
- Atom/Bond/UnitCell visibility 토글
- Measurement 진입/종료/삭제
- Charge density viewer/slice viewer 표시
2. 각 시나리오별 함수 앵커 기록(회귀 분석 기준점)
3. 수동 점검 결과 입력 양식 정의(Pass/Fail/비고)

### 완료 기준
- 제안 파일: `docs/refactoring/phase0/smoke_checklist.md` 작성 완료

## W0-4. 아키텍처/의존 기준선 리포트 작성

### 작업
1. 의존 경계 위반 패턴 정의
- direct singleton 호출, 상위 include, 전역 컨테이너 직접 참조
2. 현재 위반 목록(파일/라인) 스냅샷 저장
3. Phase 1 검증 커맨드와 동일한 패턴으로 기준선 보고서 작성

### 완료 기준
- 제안 파일: `docs/refactoring/phase0/dependency_baseline_report.md` 작성 완료

## W0-5. 레거시 문서 정리 계획 수립

### 작업
1. `webassembly/src/refactory*.md` 목록 확정
2. 이동 대상/보존 정책 정의
- 대상 경로 제안: `docs/legacy/atoms-refactory/`
3. 이동 시점 정의
- Phase 0: 계획만 확정
- 실제 이동: Phase 1 착수 직전 또는 초기에 실행

### 완료 기준
- 문서 정리 정책 및 실행 순서 확정

## W0-6. Phase 1 진입 게이트 정의

### 작업
1. Go/No-Go 조건 확정
- 기준선 문서 100% 작성
- 빌드 기준선 1회 이상 성공
- 스모크 체크리스트 1회 실행 완료
2. 리스크 등록부 작성
- 우선 리스크: 렌더 회귀, import 회귀, 측정 기능 회귀

### 완료 기준
- 제안 파일: `docs/refactoring/phase0/go_no_go_phase1.md`

## 5. 함수 앵커 기반 스모크 매핑표 (Phase 0)

| 시나리오 | 기준 함수(현행 앵커) | 파일 | 검증 포인트 |
|---|---|---|---|
| Structure 파일 선택/로드 | `FileLoader::OpenStructureFileBrowser`, `FileLoader::ApplyXSFParseResult` | `webassembly/src/file_loader.cpp` | import 성공, 구조 트리 반영 |
| XSF Grid 로드 | `FileLoader::OpenXSFGridFileBrowser`, `FileLoader::ApplyXSFGridParseResult` | `webassembly/src/file_loader.cpp` | Grid 데이터 반영, UI 오류 없음 |
| CHGCAR 로드 | `FileLoader::OpenChgcarFileBrowser`, `FileLoader::ApplyChgcarParseResult` | `webassembly/src/file_loader.cpp` | 밀도 데이터 로드/표시 가능 |
| Model Tree 렌더 | `ModelTree::Render`, `ModelTree::renderXsfStructureTable` | `webassembly/src/model_tree.cpp` | 트리 렌더/확장/클릭 동작 |
| Structure/Atom/Bond 가시성 | `AtomsTemplate::SetStructureVisible`, `SetAtomVisibleById`, `SetBondVisibleById`, `SetBondsVisible` | `webassembly/src/atoms/atoms_template.cpp` | eye 토글 동작/렌더 반영 |
| Unit Cell 가시성 | `AtomsTemplate::SetUnitCellVisible(int32_t,bool)` | `webassembly/src/atoms/atoms_template.cpp` | 셀 표시 토글 반영 |
| 측정 기능 | `AtomsTemplate::EnterMeasurementMode`, `ExitMeasurementMode`, `RemoveMeasurementsByStructure` | `webassembly/src/atoms/atoms_template.cpp` | 진입/종료/삭제 정상 |
| Charge Density UI | `AtomsTemplate::RenderChargeDensityViewerWindow`, `RenderSliceViewerWindow` | `webassembly/src/atoms/atoms_template.cpp` | 창 표시/슬라이스 UI 렌더 |

## 6. 상세 일정(권장)

- Day 1 오전
  - W0-1, W0-2 완료
- Day 1 오후
  - W0-3(체크리스트 초안 + 1회 점검)
- Day 2 오전
  - W0-4(의존 기준선 리포트)
- Day 2 오후
  - W0-5, W0-6 완료 및 Phase 1 진입 승인

총 예상 공수: **1.5~2 MD** (중난도, 구현보다 운영정의 중심)

## 7. 산출물 목록(최종)

- 본 문서: `refactoring_phase0_stabilization_baseline_260325.md`
- `docs/refactoring/phase0/baseline_manifest_20260325.md`
- `docs/refactoring/phase0/smoke_checklist.md`
- `docs/refactoring/phase0/dependency_baseline_report.md`
- `docs/refactoring/phase0/go_no_go_phase1.md`
- (선택) `scripts/refactoring/collect_phase0_baseline.sh`

## 8. Phase 0 완료(DoD)

- 기준선 메타(커밋/브랜치/측정일시/명령) 기록 완료
- 빌드 기준선 절차 문서화 및 1회 이상 성공 로그 확보
- 스모크 시나리오 체크리스트 작성 + 1회 실행 결과 기록
- 의존 기준선 리포트(위반 패턴 + 파일/라인) 생성
- Phase 1 진입 Go/No-Go 문서 승인 상태

## 9. 착수 체크리스트

- [ ] 기준선 manifest 템플릿 생성
- [ ] 빌드 로그 저장 경로 생성
- [ ] 스모크 시나리오 항목 확정(최소 8개)
- [ ] 의존 패턴 검사식 확정
- [ ] Phase 1 진입 게이트 확정
