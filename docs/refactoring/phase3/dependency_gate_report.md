# Phase 3 Dependency Gate Report

기준 시점: `2026-03-30 (KST)`  
기준 문서: `refactoring_phase3_import_pipeline_separation_260330.md`  
경계 검사 로그: `docs/refactoring/phase3/logs/dependency_check_phase3_latest.txt`  
빌드 로그: `docs/refactoring/phase3/logs/build_phase3_gate_latest.txt`

## 1. 정량 결과 (P3 게이트)
| 항목 | Baseline | Target | Current | 결과 |
|---|---:|---:|---:|---|
| `P3.file_loader_lines` | 1,449 | 1,014 이하(30%+ 감소) | 443 | PASS |
| `P3.file_loader_em_asm` | 다수 | 0 | 0 | PASS |
| `P3.import_callback_entrypoints` | 3 | 1~2 | 1 | PASS |
| `P3.parser_worker_delegate_calls` | N/A | 4 | 4 | PASS |
| `P3.browser_adapter_delegate_calls` | N/A | 6 이상 | 6 | PASS |
| `P3.import_apply_delegate_calls` | N/A | 1 | 1 | PASS |
| `P3.import_orchestrator_delegate_calls` | N/A | 4 이상 | 4 | PASS |
| `P3.required_files_exist` | N/A | 0 | 0 | PASS |

## 2. 검증 명령
```powershell
powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase3_import_pipeline.ps1
```

## 3. 빌드 결과
- 명령: `cmd /c "..\emsdk\emsdk_env.bat && npm.cmd run build-wasm:release"`
- 결과: `PASS`
- 산출물
  - `public/wasm/VTK-Workbench.js`
  - `public/wasm/VTK-Workbench.wasm`
  - `public/wasm/VTK-Workbench.data`

## 4. 스모크 근거
- 정적 앵커 체크: PASS
  - 단일 import 콜백 진입점: `FileLoader::OnParserWorkerResult`
  - BrowserAdapter 위임: `BrowserFilePicker::Open*`
  - ParserWorker 위임: `m_ParserWorkerService.Process*`
  - ImportApply 위임: `m_ImportApplyService.Apply`
  - Orchestrator 위임: `m_ImportOrchestrator.*`
- 런타임 수동 스모크(UI 실조작): **PASS**
  - 실행 일시: `2026-03-30 (KST)`
  - 실행 결과: 사용자 판단 `이상 없음`

## 5. 요약
- Phase 3의 구조 분리 목표(W1~W5)는 코드/정량 게이트 기준으로 충족됐다.
- WASM 릴리즈 빌드도 성공해 컴파일/링크 경로는 검증됐다.
- 수동 런타임 스모크까지 PASS되어 Phase 3 종료 판정 기준을 충족했다.
