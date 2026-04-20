# Phase18 W4~W6 실행 보고서 (IO/Runtime 경계 치환 + Singleton 제거)

작성일시: `2026-04-20 (KST)`
기준 문서: `docs/refactoring/phase18/refactoring_phase18_phase17_limit_closure_execution_plan_260416.md`

## 1. 실행 범위

1. W4: IO/Runtime 경계 치환
2. W5: Shell/App/Presentation 최종 치환 잔여 정리 + 미사용 요소 정리
3. W6: `AtomsTemplate` singleton 제거 및 런타임 소유 객체 전환

## 2. 코드 변경 요약

### 2.1 W4

1. `io/application` 계층의 legacy 직접 의존 제거
2. Import 포트 계약 도입
   - `io/application/import_runtime_port.h`
3. Legacy adapter 구현
   - `io/infrastructure/legacy/legacy_import_runtime_port.*`
4. 기존 import 서비스는 포트 호출로만 legacy 런타임에 접근

### 2.2 W5

1. 빈 폴더/불필요 `.gitkeep` 정리 완료
2. 미사용 헤더 후보 3건은 즉시 삭제 대신 allowlist로 보존 근거 문서화
   - `docs/refactoring/phase18/logs/unused_file_allowlist_phase18_latest.md`

### 2.3 W6

1. `DECLARE_SINGLETON(AtomsTemplate)` 제거
2. `workspace::legacy::LegacyAtomsRuntime()` 접근자 기반으로 런타임 소유 객체 전환
   - `workspace/legacy/legacy_atoms_runtime.h/.cpp`
3. 전역 `AtomsTemplate::Instance()` 호출 제거

## 3. 완료 기준 점검

| 완료 기준 | 결과 |
|---|---|
| IO + Runtime 경로 `AtomsTemplate::Instance()` 0 | 달성 |
| `workspace/legacy/atoms_template_facade.h` direct include 0 | 달성 |
| `app.cpp`의 `AtomsTemplate::Instance()` 0 | 달성 |
| singleton 선언 0 | 달성 |
| 빈 폴더/`.gitkeep` 정리 항목 A 완료 | 달성 |
| 미참조 헤더 후보 3개 처리 | 달성(allowlist 문서화) |

## 4. 검증 결과

1. `npm run build-wasm:release`: PASS
2. `npm run type-check`: PASS
3. `npm run test:cpp`: PASS
4. `npm run test:smoke`: PASS
5. `check_phase17_target_graph_final.ps1`: PASS
6. `check_phase17_r6_atoms_legacy_zero.ps1`: PASS
7. `check_phase17_root_allowlist.ps1`: PASS

## 5. 산출물

1. `docs/refactoring/phase18/logs/w4_w6_execution_snapshot_latest.md`
2. `docs/refactoring/phase18/logs/unused_file_allowlist_phase18_latest.md`
3. `docs/refactoring/phase18/logs/comment_policy_fix_report_phase18_latest.md`
4. `docs/refactoring/phase18/logs/comment_policy_violation_list_phase18_latest.md`
