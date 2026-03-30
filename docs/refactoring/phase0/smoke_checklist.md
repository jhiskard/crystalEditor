# Phase 0 Smoke Checklist

## 1. 목적
- Phase 1 진입 전 회귀 감시 기준을 고정한다.
- Phase 0에서는 코드 변경 없이 시나리오/앵커/판정 양식을 확정한다.

## 2. 시나리오 체크리스트(기준 정의)
| ID | 시나리오 | 함수 앵커 | 기대 결과 |
|---|---|---|---|
| S1 | Structure import (XSF) | `FileLoader::OpenStructureFileBrowser`, `FileLoader::ApplyXSFParseResult` | import 성공, 구조 트리 반영 |
| S2 | XSF Grid import | `FileLoader::OpenXSFGridFileBrowser`, `FileLoader::ApplyXSFGridParseResult` | Grid 데이터 반영, UI 오류 없음 |
| S3 | CHGCAR import | `FileLoader::OpenChgcarFileBrowser`, `FileLoader::ApplyChgcarParseResult` | 밀도 데이터 로드/표시 가능 |
| S4 | Model Tree 렌더/토글 | `ModelTree::Render`, `ModelTree::renderXsfStructureTable` | 트리 렌더/확장/클릭 동작 |
| S5 | Structure/Atom/Bond visibility | `AtomsTemplate::SetStructureVisible`, `SetAtomVisibleById`, `SetBondVisibleById`, `SetBondsVisible` | eye 토글 동작/렌더 반영 |
| S6 | Unit Cell visibility | `AtomsTemplate::SetUnitCellVisible` | 셀 표시 토글 반영 |
| S7 | Measurement | `AtomsTemplate::EnterMeasurementMode`, `ExitMeasurementMode`, `RemoveMeasurementsByStructure` | 진입/종료/삭제 정상 |
| S8 | Charge Density UI | `AtomsTemplate::RenderChargeDensityViewerWindow`, `RenderSliceViewerWindow` | 창 표시/슬라이스 UI 렌더 |

## 3. 실행 결과 기록

### 실행 #1 (2026-03-27, 정적 앵커 점검)
- 실행 방식: `rg` 기반 함수 앵커 존재 검증
- 결과: 8/8 `PASS`

| ID | 결과 | 근거 |
|---|---|---|
| S1 | PASS | `webassembly/src/file_loader.cpp` 내 앵커 2건 확인 |
| S2 | PASS | `webassembly/src/file_loader.cpp` 내 앵커 2건 확인 |
| S3 | PASS | `webassembly/src/file_loader.cpp` 내 앵커 2건 확인 |
| S4 | PASS | `webassembly/src/model_tree.cpp` 내 앵커 2건 확인 |
| S5 | PASS | `webassembly/src/atoms/atoms_template.cpp` 내 앵커 6건 확인 |
| S6 | PASS | `webassembly/src/atoms/atoms_template.cpp` 내 앵커 1건 확인 |
| S7 | PASS | `webassembly/src/atoms/atoms_template.cpp` 내 앵커 3건 확인 |
| S8 | PASS | `webassembly/src/atoms/atoms_template.cpp` 내 앵커 2건 확인 |

### 실행 #2 (수동 런타임 스모크)
- 실행일시: `2026-03-27` (사용자 확인)
- 상태: `COMPLETED`
- 결과: 사용자 확인 기준 `PASS` (수동 런타임 스모크 수행 완료)
- 비고:
  - 본 문서의 수동 런타임 결과는 사용자 실행/확인값을 기록
  - 상세 스크린샷/세부 로그는 필요 시 Phase 1 문서에 추가 첨부

## 4. 결과 입력 양식(재사용)
| 실행일시 | 시나리오 ID | 결과(PASS/FAIL/BLOCKED) | 증상/로그 | 비고 |
|---|---|---|---|---|
| YYYY-MM-DD HH:mm | Sx | PASS/FAIL/BLOCKED | 재현 절차 + 스크린샷/로그 | 담당자 |
