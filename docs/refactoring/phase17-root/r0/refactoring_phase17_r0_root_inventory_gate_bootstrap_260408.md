# Phase 17-R0 수행 보고서 - 루트 인벤토리 고정 및 게이트 도입

작성일: `2026-04-08`  
기준 계획서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`

## 1. 수행 목표

R0 목표:
1. 루트 코드 파일 인벤토리 고정
2. 루트 allowlist 검사 스크립트 도입
3. 루트 ownership 검사 스크립트 도입
4. 누락 디렉터리 scaffold 생성

## 2. 수행 내용

### 2.1 정적 게이트 스크립트 추가

추가 파일:
- `scripts/refactoring/check_phase17_root_allowlist.ps1`
- `scripts/refactoring/check_phase17_root_ownership_map.ps1`

게이트 핵심:
- 루트 코드 파일이 `2026-04-08` 스냅샷 baseline 범위를 벗어나면 FAIL
- facade 필수 파일(`main.cpp`, `app.cpp`, `app.h`) 누락 시 FAIL
- 현재 루트 코드 파일이 ownership map에 없으면 FAIL
- `atoms_template_bravais_lattice.cpp`, `atoms_template_periodic_table.cpp`는 설명 자료(`documentation_material_excluded`)로 분류

### 2.2 디렉터리 scaffold 생성

생성 경로:
- `webassembly/src/platform/worker/.gitkeep`
- `webassembly/src/platform/persistence/.gitkeep`
- `webassembly/src/shell/presentation/.gitkeep`
- `webassembly/src/workspace/presentation/.gitkeep`
- `webassembly/src/render/presentation/.gitkeep`
- `webassembly/src/structure/infrastructure/.gitkeep`
- `webassembly/src/io/platform/.gitkeep`

### 2.3 인벤토리 고정 문서

작성 파일:
- `docs/refactoring/phase17-root/r0/logs/root_inventory_phase17_r0_latest.md`

스냅샷 값:
- `webassembly/src` 루트 코드 파일: `40`
- `webassembly/src/atoms` 루트 코드 파일: `2`
- 총합: `42`

## 3. 검증

실행 명령:
1. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_allowlist.ps1`
2. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_ownership_map.ps1`

결과:
- 두 게이트 모두 PASS
- 로그 파일 저장:
  - `docs/refactoring/phase17-root/r0/logs/check_phase17_root_allowlist_latest.txt`
  - `docs/refactoring/phase17-root/r0/logs/check_phase17_root_ownership_map_latest.txt`

## 4. R1 착수 조건 점검

R1(`Facade 축소`) 착수 전 조건:
1. 인벤토리 baseline 고정 여부: PASS
2. allowlist/ownership gate 동작 여부: PASS
3. 디렉터리 scaffold 준비 여부: PASS

판정:
- `GO` (R1 진행 가능)
