# Phase 17-R1 수행 보고서 - Facade 계약 고정

작성일: `2026-04-08`  
기준 계획서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`

## 1. 수행 목표

R1 목표:
1. `main.cpp` feature 직접 접근 금지 상태 점검
2. `app.cpp` 루트 include 의존 목록 축소 계획 확정
3. facade 계약 정적 게이트 도입

## 2. 수행 내용

### 2.1 facade 계약 게이트 추가

추가 스크립트:
- `scripts/refactoring/check_phase17_facade_contract.ps1`

검사 항목:
1. `main.cpp` 로컬 include allowlist 일치
2. `app.h`의 root legacy include 0
3. `app.cpp` root legacy include baseline(8) 초과 금지
4. `app.cpp` allowlist 외 root include 0
5. `main/app`의 legacy singleton `::Instance()` 호출 0
6. `main.cpp`의 feature facade 직접 접근 0
7. `main.cpp`, `app.h`의 금지 타입 토큰 0

### 2.2 facade 의존 인벤토리 확정

작성 파일:
- `docs/refactoring/phase17-root/r1/logs/facade_dependency_inventory_phase17_r1_latest.md`

핵심 값:
- `app.cpp` root legacy include: `8` (R1 baseline)
- baseline 외 신규 root include: `0`

### 2.3 R1 검증 로그

실행 명령:
- `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_facade_contract.ps1`

결과:
- 전 항목 PASS
- 로그:
  - `docs/refactoring/phase17-root/r1/logs/check_phase17_facade_contract_latest.txt`

## 3. 완료 기준 점검

1. facade 계약 위반 0: `PASS`
2. facade 복잡도 역증가 차단 게이트 확보: `PASS`
3. 의존 목록 축소 계획 확정: `PASS`

## 4. 판정

- `GO` (R2 착수 가능)
