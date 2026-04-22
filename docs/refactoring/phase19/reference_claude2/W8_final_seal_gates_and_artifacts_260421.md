# Phase 19 / W8 — 최종 봉인 + 게이트/산출물

작성일: `2026-04-21 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_claude2_260421.md`
검증보고서 근거: 전체 리스크 + DoD 재측정
우선순위: **필수** (Phase 19 종료 게이트)
예상 소요: 1 ~ 2일
선행 의존: W1, W2, W3, W4, W5, W6, W7 **전부 완료**
후속 작업: W9 (릴리스)

---

## 1. 목적

W8은 Phase 19의 실질 작업(W1~W7)이 모두 완료된 후, 그 결과를 **게이트로 봉인**하고 **검증 가능한 산출물**로 기록한다. 본 단계가 완료되면 독립 검증보고서가 식별한 모든 리스크(R1~R4) 및 권고(High/Medium/Low) 5건이 "해소됨"으로 갱신될 수 있다.

## 2. 산출물 구성

### 2.1 신규 게이트 스크립트 (7종)

모두 `scripts/refactoring/` 아래에 PowerShell로 작성하고, 기존 phase 게이트와 동일한 형식(종료 코드 0 = PASS, 1 = FAIL)을 유지한다.

| 스크립트 | 검증 내용 |
|---|---|
| `check_phase19_legacy_directory_zero.ps1` | `webassembly/src/**/legacy/**` 경로 (파일/폴더) 발생 0 |
| `check_phase19_atoms_template_zero.ps1` | `AtomsTemplate` 심볼 grep 결과 0 (테스트, 문서 제외) |
| `check_phase19_legacy_runtime_zero.ps1` | `LegacyAtomsRuntime` 심볼 grep 결과 0 |
| `check_phase19_singleton_zero.ps1` | `DECLARE_SINGLETON\s*\(` (정의 외) + `::Instance()` 전량 0 |
| `check_phase19_friend_class_zero.ps1` | `friend\s+class` 전체 0 |
| `check_phase19_app_cpp_size.ps1` | `webassembly/src/app.cpp` LOC ≤ 400 |
| `check_phase19_final_closure.ps1` | 위 6개 + 기존 `check_phase7/12/13/14/15/17/18` 게이트 전부 PASS 메타 게이트 |

### 2.2 package.json 엔트리

```jsonc
{
  "scripts": {
    "check:phase19:legacy-zero":        "powershell -File scripts/refactoring/check_phase19_legacy_directory_zero.ps1",
    "check:phase19:atoms-template-zero":"powershell -File scripts/refactoring/check_phase19_atoms_template_zero.ps1",
    "check:phase19:legacy-runtime-zero":"powershell -File scripts/refactoring/check_phase19_legacy_runtime_zero.ps1",
    "check:phase19:singleton-zero":     "powershell -File scripts/refactoring/check_phase19_singleton_zero.ps1",
    "check:phase19:friend-class-zero":  "powershell -File scripts/refactoring/check_phase19_friend_class_zero.ps1",
    "check:phase19:app-cpp-size":       "powershell -File scripts/refactoring/check_phase19_app_cpp_size.ps1",
    "check:phase19:final-closure":      "powershell -File scripts/refactoring/check_phase19_final_closure.ps1"
  }
}
```

### 2.3 최종 로그 패키지

`docs/refactoring/phase19/logs/`에 다음 로그를 최종 확정:

- `phase19_inventory_snapshot_latest.md` — Phase 19 완료 시점 스냅샷
- `atoms_template_ownership_matrix_phase19_latest.md` — 186 메서드 이관 결과표 (W0에서 시작, W5에서 실측 업데이트)
- `app_cpp_decomposition_matrix_phase19_latest.md` — 34 메서드 이관 결과표 (W0→W7)
- `legacy_runtime_caller_inventory_phase19_latest.md` — 14파일 → 0 이행 이력
- `singleton_call_sites_phase19_latest.md` — W6 결과
- `friend_class_access_inventory_phase19_latest.md` — W1 결과
- `measurement_vtk_token_inventory_phase19_latest.md` — W2 결과
- `port_retention_decisions_W3_latest.md` — W3 결과
- `port_retention_decisions_W4_latest.md` — W4 결과
- `dependency_gate_report_W{1..8}.md` — 서브 단계별 완료 리포트
- `manual_ui_regression_phase19_latest.md` — 수동 UI 회귀 증빙 (Phase 18 체크리스트 기준)
- `e2e_summary_phase19_latest.md` — 전 e2e 실행 결과 요약

### 2.4 종료 보고서 2종

#### 2.4.1 `docs/refactoring/refactoring_result_report_phase19_final_closure_260XXX.md`

Phase 19 단독 종료 보고서. 포함 내용:

1. 실행 개요
2. 정량 지표 Before/After 표 (본 계획서 5.1의 DoD 표 전량)
3. 검증보고서 5장 리스크/권고 해소 현황
4. 검증보고서 4.2 DoD 11개 항목 재측정 결과 (전량 달성 확인)
5. 서브 단계별 요약 + 게이트 결과
6. 수동/자동 회귀 결과 집계
7. 잔여 과제 없음 확인

#### 2.4.2 `docs/refactoring/refactoring_result_report_phase0_19_comprehensive_260XXX.md`

Phase 0~19 전체 프로그램 종료 보고서. 포함 내용:

1. 20개 Phase 일정 및 완료 이력
2. 모든 DoD 최종 달성 상태
3. 아키텍처 변경 요약 (Before/After diagram)
4. 핵심 지표(LOC, 모듈 수, 의존성, 게이트 수) 추이
5. 감사 보고서 발행 단계 게이트 결과
6. 다음 단계 제언 (Phase 20 이후가 필요하다면 그 조건)

## 3. 실행 순서

1. **W1~W7 전부 완료 확인**:
   - 각 `dependency_gate_report_W{N}.md`가 존재하고 PASS
   - 기존 Phase 7~18 게이트 모두 재실행 PASS
2. **신규 게이트 스크립트 7종 작성** (각 1파일, ~20~40 LOC 예상):
   - 일관된 에러 포맷 (grep 결과 있으면 `FAIL: found X occurrences at <file:line>`)
3. **`package.json`에 엔트리 추가**
4. **게이트 로컬 실행 PASS 확인**:
   ```
   npm run check:phase19:legacy-zero
   npm run check:phase19:atoms-template-zero
   npm run check:phase19:legacy-runtime-zero
   npm run check:phase19:singleton-zero
   npm run check:phase19:friend-class-zero
   npm run check:phase19:app-cpp-size
   npm run check:phase19:final-closure
   ```
5. **로그 패키지 확정**:
   - 각 로그의 "latest" 파일이 최종 값으로 업데이트되었는지 확인
6. **종료 보고서 2종 작성**:
   - Phase 19 단독 보고서
   - Phase 0~19 종합 보고서
7. **검증보고서 역참조 업데이트**:
   - `refactoring_verification_report_independent_260420.md`는 외부 독립 검증 결과물이므로 수정하지 않는다.
   - 대신 `docs/refactoring/refactoring_result_report_phase19_final_closure_260XXX.md`에 "검증보고서 5.1 R1 — 해소됨: ..., R2 — 해소됨: ..., R3 — 해소됨: ..., R4 — 해소됨: ..., 권고 High — 이행 완료, ..." 섹션을 포함
8. **최종 수동 회귀**: Phase 18의 체크리스트를 그대로 수행하고 결과를 `manual_ui_regression_phase19_latest.md`에 기록
9. **Docker 빌드 회귀**: `npm run build:docker` 로컬 PASS
10. **Git 태그**: W9에서 `tag: refactor-phase19-closed` 부착

## 4. 게이트 스크립트 구현 예

### 4.1 `check_phase19_legacy_directory_zero.ps1`

```powershell
$ErrorActionPreference = 'Stop'
$root = Join-Path $PSScriptRoot '..\..\webassembly\src'
$legacyDirs = Get-ChildItem -Path $root -Directory -Recurse |
    Where-Object { $_.Name -eq 'legacy' }
if ($legacyDirs.Count -gt 0) {
    Write-Host "FAIL: found $($legacyDirs.Count) legacy directory(ies)"
    $legacyDirs | ForEach-Object { Write-Host "  - $($_.FullName)" }
    exit 1
}
$legacyFiles = Get-ChildItem -Path $root -File -Recurse -Include '*legacy*.h','*legacy*.cpp'
if ($legacyFiles.Count -gt 0) {
    Write-Host "FAIL: found $($legacyFiles.Count) legacy file(s)"
    $legacyFiles | ForEach-Object { Write-Host "  - $($_.FullName)" }
    exit 1
}
Write-Host 'PASS: no legacy directories or files found'
exit 0
```

### 4.2 `check_phase19_atoms_template_zero.ps1`

```powershell
$ErrorActionPreference = 'Stop'
$root = Join-Path $PSScriptRoot '..\..\webassembly\src'
$matches = Get-ChildItem -Path $root -File -Recurse -Include '*.h','*.cpp','*.cc' |
    Select-String -Pattern 'AtomsTemplate' -SimpleMatch
if ($matches.Count -gt 0) {
    Write-Host "FAIL: found $($matches.Count) AtomsTemplate reference(s)"
    $matches | Select-Object -First 10 | ForEach-Object { Write-Host "  - $($_.Path):$($_.LineNumber): $($_.Line.Trim())" }
    exit 1
}
Write-Host 'PASS: no AtomsTemplate references'
exit 0
```

### 4.3 `check_phase19_final_closure.ps1` (메타 게이트)

```powershell
$scripts = @(
  'scripts/refactoring/check_phase7_composition_root.ps1',
  'scripts/refactoring/check_phase12_facade_seal.ps1',
  'scripts/refactoring/check_phase13_runtime_hard_seal.ps1',
  'scripts/refactoring/check_phase14_state_ownership_alias_zero.ps1',
  'scripts/refactoring/check_phase15_render_platform_isolation.ps1',
  'scripts/refactoring/check_phase17_root.ps1',
  'scripts/refactoring/check_phase18_limit_closure.ps1',
  'scripts/refactoring/check_phase19_legacy_directory_zero.ps1',
  'scripts/refactoring/check_phase19_atoms_template_zero.ps1',
  'scripts/refactoring/check_phase19_legacy_runtime_zero.ps1',
  'scripts/refactoring/check_phase19_singleton_zero.ps1',
  'scripts/refactoring/check_phase19_friend_class_zero.ps1',
  'scripts/refactoring/check_phase19_app_cpp_size.ps1'
)
$fail = $false
foreach ($s in $scripts) {
    & powershell -File $s
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL: $s exited $LASTEXITCODE"
        $fail = $true
    }
}
if ($fail) { exit 1 }
Write-Host 'PASS: all Phase 7/12/13/14/15/17/18/19 gates PASS'
exit 0
```

※ 실제 스크립트 파일명은 기존 각 Phase 게이트 파일명과 정확히 매칭되어야 한다. W0에서 현 파일명을 캡처.

## 5. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| 신규 게이트 스크립트 7종 존재 | 파일 체크 | 있음 |
| `package.json` 엔트리 7종 | 검색 | 있음 |
| 7종 게이트 로컬 실행 | PASS | 전량 PASS |
| 기존 Phase 7~18 게이트 재실행 | PASS | 전량 PASS |
| 종료 보고서 2종 존재 | 파일 체크 | 있음 |
| 검증보고서 5장 리스크/권고 해소 문서화 | 보고서 내용 확인 | 전량 "해소됨" |
| DoD 11개 재측정 | 보고서 내용 확인 | 전량 "달성" |
| 수동 UI 회귀 증빙 | `manual_ui_regression_phase19_latest.md` | 전 시나리오 PASS |
| Docker 빌드 | `npm run build:docker` | PASS |

## 6. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 게이트 스크립트에서 문자열 매칭 오판(주석/문서 내 `AtomsTemplate` 언급 포함) | False positive | grep 대상을 `*.h`, `*.cpp`, `*.cc`로 한정하고 `docs/` 폴더 제외 |
| 기존 게이트 스크립트 파일명 불일치 | 메타 게이트 실패 | W0에서 `ls scripts/refactoring/check_phase*.ps1`로 목록 캡처 후 스크립트 하드코딩 |
| 종합 보고서 작성 과정에서 과거 Phase 수치 재계산 필요 | 공수 증가 | Phase별 `refactoring_phase*_*.md` 보고서에서 최종 수치를 직접 인용, 재산출 회피 |
| CI에서 PowerShell 7이 아닌 Windows PowerShell 5 환경 | 문법 불일치 | 스크립트 상단 `#requires -Version 5.1` 명시 및 호환 문법만 사용 |
| 독립 검증보고서가 외부 파일이라 갱신 금지 | 리스크 상태 업데이트 위치 모호 | Phase 19 단독 종료 보고서에 "검증보고서 5장 맵핑" 섹션으로 정리 (보고서 자체를 수정하지 않음) |

## 7. 산출물

- `scripts/refactoring/check_phase19_*.ps1` 7개
- `package.json` 수정
- `docs/refactoring/phase19/logs/*` 로그 10+종 최종화
- `docs/refactoring/refactoring_result_report_phase19_final_closure_260XXX.md`
- `docs/refactoring/refactoring_result_report_phase0_19_comprehensive_260XXX.md`
- `docs/refactoring/phase19/logs/dependency_gate_report_W8.md`

## 8. Git 커밋 템플릿

```
chore(gates): phase19/W8 — add closure gates & reports

- Add 7 new Phase 19 gate scripts
- Wire npm run check:phase19:* entries
- Write final closure reports (phase19 + phase0-19 comprehensive)
- Capture manual UI regression & e2e summary logs

metrics:
  new gate scripts: 7
  phase19 gates passing: 7/7
  phase 7-18 gates passing: all
  DoD items (verification report 4.2): 11/11 achieved
  risks (verification report 5.1): 4/4 resolved
  recommendations (verification report 5.2): 5/5 fulfilled
```

---

*W8 완료 시점에 Phase 19는 실질적으로 종료된다. 이후 W9 (릴리스 및 태그)는 절차적 단계다.*
