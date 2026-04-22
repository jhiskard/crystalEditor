# Phase 19 / W8 — 물리 삭제 및 빌드 정리

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: legacy 완전 소거(최우선), v2 W8 정의
우선순위: **최우선**
예상 소요: 1 ~ 2일
선행 의존: W1~W7 완료
후속 작업: W9 (게이트 패키지), W10 (종료 검증)

---

## 1. 배경

W1~W7에서 책임 이관이 완료되어도 물리 파일/폴더가 남으면 legacy 재유입 위험이 남는다. W8은 남은 legacy 경로를 전수 삭제하고 CMake 소스 그래프에서 legacy 엔트리를 완전히 제거하는 단계다.

## 2. 삭제 대상

### 2.1 legacy 디렉터리

1. `webassembly/src/workspace/legacy`
2. `webassembly/src/structure/infrastructure/legacy`
3. `webassembly/src/measurement/infrastructure/legacy`
4. `webassembly/src/density/infrastructure/legacy`
5. `webassembly/src/io/infrastructure/legacy`

### 2.2 디렉터리 외 legacy 파일

- `webassembly/src/render/application/legacy_viewer_facade.h`
- `webassembly/src/render/application/legacy_viewer_facade.cpp`

## 3. 설계

W8은 "기능 이관 완료 확인 후 삭제" 원칙을 적용한다.

- 삭제 전: 호출자 0 확인
- 삭제 후: 빌드 그래프 무결성 확인
- 금지: dead code를 남긴 채 경로명만 변경하는 우회

## 4. 실행 순서

1. W1~W7 완료 리포트(`dependency_gate_report_W1~W7`) 확인.
2. legacy 디렉터리/파일 잔존 목록 재스캔.
3. 잔존 파일 삭제.
4. `webassembly/cmake/modules/wb_*.cmake`에서 legacy 소스 엔트리 전량 제거.
5. `find legacy dir == 0`, `legacy token == 0` 확인 로그 생성.
6. `build-wasm:debug` 및 `build-wasm:release` 빌드 검증.
7. W8 완료 리포트 작성.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/**/legacy/**` | **삭제** | 잔존분 전량 |
| `webassembly/src/render/application/legacy_viewer_facade.h` | **삭제** | 잔존 시 |
| `webassembly/src/render/application/legacy_viewer_facade.cpp` | **삭제** | 잔존 시 |
| `webassembly/cmake/modules/wb_*.cmake` | 수정 | legacy 소스/링크 제거 |
| `docs/refactoring/phase19/logs/phase19_inventory_snapshot_latest.md` | 갱신 | 삭제 후 스냅샷 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W8.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `webassembly/src` 하위 legacy 디렉터리 | find | 0 |
| `legacy_viewer_facade` 파일 존재 | find | 0 |
| `wb_*.cmake`의 legacy 엔트리 | grep | 0 |
| `legacy` 토큰(코드 경로 기준) | grep | 0 (허용 목록 제외) |
| `build-wasm:debug` | 빌드 | PASS |
| `build-wasm:release` | 빌드 | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 삭제 순서 오류로 링크 에러 발생 | 빌드 실패 | 호출자 0 확인 후 삭제, cmake 동시 갱신 |
| 문서/테스트의 legacy 문자열에 false positive | 검증 혼선 | 코드 대상/문서 대상 검사 패턴 분리 |
| W9 이전에 임시 스크립트 부재 | 확인 누락 | W8에서 최소 잔존 스캔 로그를 의무화 |

## 8. 산출물

- legacy 경로 삭제 커밋
- 갱신된 `wb_*.cmake`
- `phase19_inventory_snapshot_latest.md`(삭제 후)
- `dependency_gate_report_W8.md`

## 9. Git 커밋 템플릿

```
refactor(cleanup): phase19/W8 — physically remove remaining legacy paths

- Delete all legacy directories/files under webassembly/src
- Remove legacy source entries from wb_*.cmake modules
- Refresh phase19 inventory snapshot after deletion

metrics:
  legacy directories: 5 -> 0
  out-of-dir legacy files: 2 -> 0
  legacy cmake source entries: N -> 0

verif:
  build-wasm:debug: PASS
  build-wasm:release: PASS
```

---

*W8 완료 시 Phase 19의 "물리 삭제" 축이 닫히며, 이후 W9~W10은 봉인과 종료 절차에 집중한다.*
