# Phase 3 세부 작업계획서: Import 파이프라인 분리

작성일: `2026-03-30 (KST)`  
최종 업데이트: `2026-03-31 (Phase 1 양식 재정리)`  
대상 범위: `webassembly/src/file_loader.cpp`, `webassembly/src/io/*`, `webassembly/src/atoms/infrastructure/*parser*`  
진행 상태: `Phase 종료 판정 완료(근거: docs/refactoring/phase3/dependency_gate_report.md, docs/refactoring/phase3/go_no_go_phase4.md)`

## 0. Phase 2 종료 반영사항

### 0.1 진입 판정
- 판정: **GO**
- 근거 문서:
  - `docs/refactoring/phase2/dependency_gate_report.md`
  - `docs/refactoring/phase2/go_no_go_phase3.md`

### 0.2 착수 기준선(Phase 2 종료 시점)
| 항목 | 기준값 | 출처 |
|---|---:|---|
| `file_loader.cpp` 라인 수 | 1,449 | 계획 기준선 |
| import 콜백 진입점 수 | 3 | 계획 기준선 |
| Browser/Parser/Apply/Transaction 로직 결합 | 단일 파일 집중 | 착수 시점 코드 |

### 0.3 Phase 3 범위 확정
- Phase 3 핵심:
  - Browser adapter 분리
  - Parser worker 분리
  - Apply service 분리
  - Import transaction(orchestrator) 분리
- 후속 Phase 이관:
  - UI/렌더 대형 함수 분해는 Phase 4
  - 빌드 시스템 모듈화는 Phase 5

## 1. Phase 3 목표

### 목표
- `FileLoader`의 과도한 책임을 `io/platform`, `io/application`, `io/infrastructure` 계층으로 분리한다.

### 도달 결과물
1. BrowserAdapter, ParserWorkerService, ImportApplyService, ImportOrchestrator 분리
2. import 처리 흐름을 오케스트레이션 중심으로 단순화
3. 파서/적용/트랜잭션 경계 명확화

### 비목표(Phase 3에서 하지 않음)
- 대규모 UI 분해(`atoms_template.cpp`, `model_tree.cpp`) (Phase 4)
- 빌드 타깃 모듈화(CMake 분할) (Phase 5)

## 2. 현재 문제 요약(착수 근거)
1. `file_loader.cpp`가 파일 선택, 파싱, 적용, 트랜잭션, UI 알림까지 모두 담당했다.
2. 예외/롤백 흐름이 산발적으로 분산되어 실패 경로 추적이 어려웠다.
3. 브라우저(EM_ASM) 의존과 도메인 적용 로직이 한 파일에 결합되어 테스트/확장이 어려웠다.

## 3. 작업 원칙
1. 동작 보존 우선(behavior-preserving)
2. 기존 진입 API는 유지하고 내부 구현만 점진 위임
3. 브라우저/파서/적용/트랜잭션 경계 분명히 분리
4. WBS 단위 커밋과 근거 로그를 동반

## 4. 작업 단위(WBS)

## W0. 착수/브랜치 셋업
### 작업
- `refactor/phase3-import-pipeline` 브랜치 준비
- 산출 디렉터리 준비(`docs/refactoring/phase3`, `docs/refactoring/phase3/logs`)

### 완료 기준
- 브랜치 및 산출 구조 준비 완료

## W1. BrowserAdapter 분리
### 작업
- 브라우저 파일 선택(EM_ASM/JS bridge) 경로를 `browser_file_picker`로 추출
- `FileLoader`는 adapter 호출만 수행하도록 정리

### 영향 파일(예상)
- `webassembly/src/file_loader.cpp`
- `webassembly/src/io/platform/browser_file_picker.{h,cpp}`

### 완료 기준
- 브라우저 접근 코드가 `file_loader.cpp`에서 분리됨

## W2. ParserWorkerService 분리
### 작업
- XSF/GRID/CHGCAR 백그라운드 파싱 경로를 service로 추출
- parse result 콜백 진입점을 축소

### 영향 파일(예상)
- `webassembly/src/file_loader.cpp`
- `webassembly/src/io/application/parser_worker_service.{h,cpp}`

### 완료 기준
- `FileLoader`의 파싱 직접 구현 경로 제거

## W3. ImportApplyService 통합
### 작업
- `ApplyXSFParseResult`, `ApplyXSFGridParseResult`, `ApplyChgcarParseResult` 적용 경로를 service로 통합

### 영향 파일(예상)
- `webassembly/src/file_loader.cpp`
- `webassembly/src/io/application/import_apply_service.{h,cpp}`

### 완료 기준
- 적용 로직이 `ImportApplyService` 경유로 일원화

## W4. ImportOrchestrator 추출
### 작업
- begin/finalize/rollback 트랜잭션 흐름을 orchestrator로 추출
- scene 상태 점검/컨텍스트 질의 helper 분리

### 영향 파일(예상)
- `webassembly/src/file_loader.cpp`
- `webassembly/src/io/application/import_orchestrator.{h,cpp}`
- `webassembly/src/io/application/import_context_query.{h,cpp}`

### 완료 기준
- 실패/롤백 경로가 orchestrator 중심으로 정렬됨

## W5. 통합/호환 정리
### 작업
- 임시 호환 경로 정리
- import entrypoint 개수 정리 및 delegate 호출 정합성 확인

### 완료 기준
- 파이프라인 위임 구조가 단일 경로로 정리됨

## W6. Gate 검사/문서화
### 작업
- 정적 검사 스크립트 실행 및 로그 기록
- gate report / go-no-go 문서 작성

### 완료 기준
- Phase 4 착수 판정 가능 상태

## 5. 상세 일정(권장)
- Day 1: W0~W1
- Day 2: W2
- Day 3: W3
- Day 4: W4
- Day 5: W5~W6

예상 공수: 4~6 MD

## 6. 파일별 변경 계획

### 신규 파일
- `webassembly/src/io/platform/browser_file_picker.{h,cpp}`
- `webassembly/src/io/application/parser_worker_service.{h,cpp}`
- `webassembly/src/io/application/import_apply_service.{h,cpp}`
- `webassembly/src/io/application/import_orchestrator.{h,cpp}`
- `webassembly/src/io/application/import_context_query.{h,cpp}`

### 주요 수정 파일
- `webassembly/src/file_loader.cpp`
- `webassembly/src/atoms/infrastructure/file_io_manager.cpp` (파서 분리 시)
- `webassembly/src/atoms/infrastructure/chgcar_parser.cpp` (io 경계 정리 시)
- `CMakeLists.txt`

## 7. 커밋/동기화 전략
1. 커밋 1: W0 셋업
2. 커밋 2: W1 BrowserAdapter
3. 커밋 3: W2 ParserWorkerService
4. 커밋 4: W3 ImportApplyService
5. 커밋 5: W4 ImportOrchestrator/Context
6. 커밋 6: W5 통합/호환
7. 커밋 7: W6 Gate 문서

## 8. 리스크 및 대응
1. 리스크: import 실패 경로 회귀
   - 대응: orchestrator 단일 롤백 경로 + 실패 시나리오 점검
2. 리스크: 파서 결과 적용 누락
   - 대응: apply 서비스 entrypoint 통합 및 delegate count 점검
3. 리스크: 브라우저 의존 분리 중 이벤트 누락
   - 대응: browser adapter API 고정 + smoke 시나리오 검증

## 9. Phase 3 완료(DoD)
- `FileLoader`의 브라우저/파서/적용/트랜잭션 책임 분리 완료
- import callback 진입점 축소 및 delegate 경로 확립
- gate report + go/no-go 문서 작성 완료

## 10. 착수 체크리스트
- [x] Phase 2 종료 `GO` 확인
- [x] W0 착수/브랜치 셋업
- [x] W1 BrowserAdapter 분리
- [x] W2 ParserWorkerService 분리
- [x] W3 ImportApplyService 통합
- [x] W4 ImportOrchestrator 추출
- [x] W5 통합/호환 정리
- [x] W6 Gate 문서화

## 11. Phase 3 클래스/함수 이동 매핑표
> 표기  
> - 이동: 기존 함수/로직을 신규 파일로 이동  
> - 위임: 기존 진입점은 유지하고 내부 구현을 신규 서비스로 위임

| WBS | 기존 위치 | 클래스/함수(현행) | 신규 모듈(목표 위치) | 방식 | 비고 |
|---|---|---|---|---|---|
| W1 | `webassembly/src/file_loader.cpp` | `OpenFileBrowser`, `OpenStructureFileBrowser`, `OpenXSFGridFileBrowser`, `OpenChgcarFileBrowser` | `webassembly/src/io/platform/browser_file_picker.cpp` | 이동 | EM_ASM/브라우저 연동 분리 |
| W2 | `webassembly/src/file_loader.cpp` | `processXSFFileInBackground`, `processXSFGridFileInBackground`, `processChgcarFileInBackground` | `webassembly/src/io/application/parser_worker_service.cpp` | 이동 | 파싱 워커 분리 |
| W3 | `webassembly/src/file_loader.cpp` | `ApplyXSFParseResult` | `webassembly/src/io/application/import_apply_service.cpp` | 이동 | 구조 적용 경로 통합 |
| W3 | `webassembly/src/file_loader.cpp` | `ApplyXSFGridParseResult` | `webassembly/src/io/application/import_apply_service.cpp` | 이동 | GRID 적용 통합 |
| W3 | `webassembly/src/file_loader.cpp` | `ApplyChgcarParseResult` | `webassembly/src/io/application/import_apply_service.cpp` | 이동 | CHGCAR 적용 통합 |
| W4 | `webassembly/src/file_loader.cpp` | `beginReplaceSceneImportTransaction`, `finalizeReplaceSceneImportSuccess`, `rollbackFailedStructureImport` | `webassembly/src/io/application/import_orchestrator.cpp` | 이동 | 트랜잭션 책임 분리 |
| W4 | `webassembly/src/file_loader.cpp` | `collectRootMeshIds`, `hasSceneDataForStructureImport` | `webassembly/src/io/application/import_context_query.cpp` | 이동 | 컨텍스트 질의 분리 |
| W5 | `webassembly/src/file_loader.cpp` | import 진입 API 내부 흐름 | `BrowserFilePicker`, `ParserWorkerService`, `ImportApplyService`, `ImportOrchestrator` | 위임 | 파일로더 orchestrator 역할로 축소 |

## 12. Phase 3 실행 순서(작업자 기준)
1. 브라우저 연동 경로를 먼저 분리(W1)
2. 파서 워커를 분리해 CPU 비용 경로를 안정화(W2)
3. 적용 경로를 통합해 도메인 반영 책임 정리(W3)
4. 트랜잭션/롤백을 단일 오케스트레이터로 고정(W4)
5. 호환 정리 후 gate 문서로 마감(W5~W6)

## 13. Phase 3 수용 기준(정량)
| 항목 | Baseline | Target | 검증 방식 |
|---|---:|---:|---|
| `file_loader.cpp` 라인 수 | 1,449 | <= 1,014 | `check_phase3_import_pipeline.ps1` |
| `EM_ASM`/`EMSCRIPTEN` in `file_loader.cpp` | 다수 | 0 | 정적 grep |
| import callback entrypoint | 3 | 1~2 | 정적 grep |
| parser delegate call 수 | N/A | 4 | gate script |
| apply delegate call 수 | N/A | 1 | gate script |
