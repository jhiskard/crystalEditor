# Phase 3 세부 실행계획서: Import 파이프라인 분리

작성일: `2026-03-30 (KST)`  
전제: Phase 2 종료 판정 `GO`  
- 근거: `docs/refactoring/phase2/go_no_go_phase3.md`

## 0. 착수 전 결정사항(확정)
1. Phase 2까지 변경사항은 현재 시점에 Git 저장소에 동기화한다.
2. 이 시점 이후 동기화는 아래 운영 규칙을 따른다.
   - Phase별 작업 브랜치 분리
   - Gate(검사/빌드/스모크/Go-NoGo) 충족 후 `main` 머지
   - Phase 완료 머지 시 태그 부여

## 1. Git 운영 규칙(Phase 3부터 적용)
1. 브랜치 전략
   - 시작 브랜치: `main` (Phase 2 머지 기준 최신 상태)
   - 작업 브랜치: `refactor/phase3-import-pipeline`
   - 병렬 작업 시 하위 브랜치: `refactor/phase3-w{N}-{topic}`

2. 동기화 시점
   - WBS 완료 단위로 작업 브랜치에 수시 `push`
   - Phase 3 Gate PASS 직후 PR 생성 및 `main` 머지
   - 머지 직후 태그: `phase3-go-YYYYMMDD`

3. 병합 기준
   - `dependency_gate_report` PASS
   - WASM release build PASS
   - 수동 스모크 PASS
   - `go_no_go_phase4.md`에서 `GO`

## 2. Phase 3 목표
1. `file_loader.cpp`의 다중 책임(브라우저 연동/파싱 워커/적용/트랜잭션)을 서비스 단위로 분리한다.
2. `ApplyXSFParseResult`, `ApplyXSFGridParseResult`, `ApplyChgcarParseResult` 적용 경로를 공통 서비스로 일원화한다.
3. Import 실패 시 롤백 경로를 `ImportOrchestrator`로 집중시켜 복구 가능성을 높인다.

## 3. 완료 기준(DoD)
1. `FileLoader`는 오케스트레이션/UI 트리거 중심으로 축소되고, 파싱/적용/트랜잭션 로직은 별도 모듈로 이동
2. import 관련 핵심 함수 이동 후에도 기존 사용자 시나리오(구조 로드, GRID 로드, CHGCAR 로드, 가시성/트리 반영) 동작 보존
3. Phase 3 게이트 문서 작성 완료
   - `docs/refactoring/phase3/dependency_gate_report.md`
   - `docs/refactoring/phase3/go_no_go_phase4.md`

## 4. 작업 원칙
1. 동작 보존 우선(behavior-preserving)
2. 파일 분할은 어댑터/서비스 추출 중심으로 진행
3. 함수 시그니처 대규모 변경은 지양하고 위임 경로를 먼저 구축
4. 각 WBS 종료 시점마다 컴파일 가능 상태 유지

## 5. WBS

## W0. 착수/브랜치 셋업
### 작업
- `main` 최신 기준 확인 후 `refactor/phase3-import-pipeline` 브랜치 생성
- Phase 3 산출 문서 디렉터리 생성 (`docs/refactoring/phase3`, `docs/refactoring/phase3/logs`)

### 완료 기준
- 브랜치 생성 및 첫 동기화 완료

## W1. BrowserAdapter 분리
### 작업
- `FileLoader::OpenFileBrowser/OpenStructureFileBrowser/OpenXSFGridFileBrowser/OpenChgcarFileBrowser`를 `io/platform` 계층으로 추출
- `EM_ASM`/브라우저 파일 선택 콜백 경로를 adapter 경유로 통합

### 영향 파일(예상)
- `webassembly/src/file_loader.cpp`
- `webassembly/src/file_loader.h`
- `webassembly/src/io/platform/browser_file_picker.h/.cpp` (신규)

### 완료 기준
- 파일 선택 트리거 기능 동일 동작

## W2. ParserWorkerService 분리
### 작업
- `processXSFFileInBackground/processXSFGridFileInBackground/processChgcarFileInBackground`를 `io/application/parser_worker_service`로 이동
- 비동기 콜백 에러 처리/취소 경로 표준화

### 영향 파일(예상)
- `webassembly/src/file_loader.cpp`
- `webassembly/src/io/application/parser_worker_service.h/.cpp` (신규)

### 완료 기준
- 3종 파서 워커 경로가 동일 인터페이스로 호출됨

## W3. ImportApplyService 통합
### 작업
- `ApplyXSFParseResult`, `ApplyXSFGridParseResult`, `ApplyChgcarParseResult`를 공통 apply 서비스로 통합
- Mesh/Atoms 적용 경로를 전략화하여 중복 코드 제거

### 영향 파일(예상)
- `webassembly/src/file_loader.cpp`
- `webassembly/src/io/application/import_apply_service.h/.cpp` (신규)

### 완료 기준
- 결과 적용 함수 중복 제거 및 단일 진입점 확보

## W4. ImportOrchestrator 추출
### 작업
- `beginReplaceSceneImportTransaction/finalize*/rollback*` 로직을 오케스트레이터로 이동
- 실패 시 롤백 처리와 UI 알림 경로 일원화

### 영향 파일(예상)
- `webassembly/src/file_loader.cpp`
- `webassembly/src/io/application/import_orchestrator.h/.cpp` (신규)

### 완료 기준
- 성공/실패 트랜잭션 경로가 서비스 내부에서 닫힘

## W5. 통합/호환 정리
### 작업
- `FileLoader`는 orchestration 호출만 남기도록 축소
- 필요 시 기존 static callback 시그니처 호환 어댑터 유지

### 완료 기준
- 기존 UI/메뉴에서 호출 경로 변경 없이 동작

## W6. Gate 검사/문서화
### 작업
- Phase 3 전용 gate report 및 Go/No-Go 문서 작성
- 빌드 로그 및 스모크 기록 저장

### 완료 기준
- Phase 3 종료 판정 가능 상태

## 6. 정량 목표(권장)
| 항목 | Baseline | Target | 검증 방식 |
|---|---:|---:|---|
| `file_loader.cpp` 라인 수 | 약 1,400+ | 30% 이상 감소 | `wc -l` 또는 에디터 기준 집계 |
| import callback entrypoint 수 | 3 | 1~2 | 함수 시그니처 점검 |
| 브라우저 연동(`EM_ASM`)의 `file_loader.cpp` 직접 포함 | 다수 | 0 | `rg -n \"EM_ASM|EMSCRIPTEN\" webassembly/src/file_loader.cpp` |
| release build | PASS | PASS 유지 | `npm.cmd run build-wasm:release` |

## 7. 리스크 및 대응
1. 리스크: 비동기 callback 수명주기 오류  
   - 대응: callback 소유권/수명 정책을 service 단위로 문서화
2. 리스크: rollback 누락으로 부분 반영 상태 잔존  
   - 대응: orchestrator 단일 경로에서 성공/실패 종료 처리 강제
3. 리스크: UI 이벤트 경로 회귀  
   - 대응: 수동 스모크 체크리스트에 import 시나리오 우선 배치

## 8. 커밋/동기화 전략
1. 커밋 1: W0/W1 (브랜치 셋업 + BrowserAdapter)
2. 커밋 2: W2 (ParserWorkerService)
3. 커밋 3: W3 (ImportApplyService)
4. 커밋 4: W4 (ImportOrchestrator)
5. 커밋 5: W5 (통합/호환 정리)
6. 커밋 6: W6 (게이트 리포트 + Go/No-Go + 로그)

동기화 규칙:
- 각 커밋 묶음 종료 시 `push`
- Gate PASS 이후 PR/merge
- merge 후 `phase3-go-YYYYMMDD` 태그

## 9. 착수 체크리스트
- [x] Phase 2 `GO` 판정 확인
- [x] Phase 2 변경사항 동기화 계획 반영
- [x] Phase 3 Git 운영 규칙 확정
- [x] `refactor/phase3-import-pipeline` 브랜치 생성
- [x] W1 BrowserAdapter 분리
- [x] W2 ParserWorkerService 분리
- [x] W3 ImportApplyService 통합
- [x] W4 ImportOrchestrator 추출
- [x] W5 통합/호환 정리
- [x] W6 Gate 문서/로그 작성
- [x] 수동 런타임 스모크 최종 PASS 기록
