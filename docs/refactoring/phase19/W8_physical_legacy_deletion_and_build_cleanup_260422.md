# Phase 19 / W8 - 물리 잔존 legacy 정리 및 빌드 그래프 클린업

작성일: `2026-04-22 (KST)`
최종 업데이트: `2026-04-27 (KST, W7 이후 코드 상태 정합 반영)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: legacy 재유입 차단(봉인), v2 W8 정의
우선순위: **최우선**
예상 소요: 1 ~ 2일
선행 의존: W1~W7 완료
후속 작업: W9 (게이트 패키지), W10 (종료 검증/보고)

---

## 1. 배경

Phase 19의 W8은 "남은 legacy 경로의 물리 삭제 + 빌드 소스 그래프 정리" 단계다.

현재 브랜치에서는 W4/W5/W7을 거치며 legacy 디렉터리와 `legacy_viewer_facade`는 이미 정리되었다. 따라서 W8은 기존 계획의 대량 삭제 단계가 아니라, **실제 잔존물인 `legacy_atoms_runtime` 경로를 제거하고 참조를 비-legacy 경로로 치환하는 정밀 클린업 단계**로 재정의되어야 한다.

## 2. 착수 전 정합성 점검 (2026-04-27)

| 항목 | 점검값 | 해석 |
|---|---:|---|
| `webassembly/src/**/legacy` 디렉터리 수 | 0 | 기존 W8 목표 일부 선달성 |
| `render/application/legacy_viewer_facade.*` 존재 | 0 | 기존 W8 목표 일부 선달성 |
| `webassembly/src` 내 `legacy` 파일명 소스 수 | 2 | `workspace/runtime/legacy_atoms_runtime.{h,cpp}` 잔존 |
| `workspace/runtime/legacy_atoms_runtime.h` include 참조 수 | 31 | 경로 치환 필요 |
| `wb_*.cmake` 내 `legacy_atoms_runtime.cpp` 엔트리 수 | 1 | `wb_workspace.cmake` 정리 필요 |
| W1~W7 게이트 리포트 존재 (`dependency_gate_report_W1..W7`) | 전부 존재 | W8 착수 전제 충족 |

잔존 파일:

1. `webassembly/src/workspace/runtime/legacy_atoms_runtime.h`
2. `webassembly/src/workspace/runtime/legacy_atoms_runtime.cpp`

## 3. 업데이트된 W8 범위

### 3.1 In Scope

1. `legacy_atoms_runtime.{h,cpp}` 물리 삭제.
2. 동일 책임의 비-legacy 경로 파일로 치환 (예: `workspace_runtime_model_ref.{h,cpp}` 또는 동등 구조).
3. `webassembly/src` 내 include 경로 `workspace/runtime/legacy_atoms_runtime.h` 전량 치환.
4. `webassembly/cmake/modules/wb_workspace.cmake`의 legacy 소스 엔트리 정리.
5. 정리 후 인벤토리/게이트 로그 갱신.

### 3.2 Out of Scope (W8)

1. `workspace::legacy` 네임스페이스 전면 리네이밍은 W9/W10 게이트 정책과 충돌 가능성이 있으므로 본 단계의 필수 목표에서 제외.
2. 신규 메타게이트 추가/체이닝은 W9 범위.

## 4. 실행 순서

1. W1~W7 완료 상태 재확인 (`dependency_gate_report_W1..W7`).
2. `legacy_atoms_runtime` 대체 파일 설계/생성 (비-legacy 경로).
3. `webassembly/src` 전역 include 치환 (`legacy_atoms_runtime.h` -> 대체 헤더).
4. `legacy_atoms_runtime.{h,cpp}` 삭제.
5. `wb_workspace.cmake` 소스 엔트리 갱신.
6. 물리 정리 계측:
   - legacy 디렉터리 수
   - `legacy_atoms_runtime` 파일 존재 여부
   - include 경로 잔존 수
   - CMake 엔트리 잔존 수
7. 빌드 검증:
   - `build-wasm:debug`
   - `build-wasm:release`
8. 로그 갱신:
   - `docs/refactoring/phase19/logs/phase19_inventory_snapshot_latest.md`
   - `docs/refactoring/phase19/logs/dependency_gate_report_W8.md`

## 5. 파일 변경 명세 (업데이트)

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/workspace/runtime/legacy_atoms_runtime.h` | **삭제** | W8 핵심 잔존물 |
| `webassembly/src/workspace/runtime/legacy_atoms_runtime.cpp` | **삭제** | W8 핵심 잔존물 |
| `webassembly/src/workspace/runtime/*runtime_model_ref*.{h,cpp}` | 신규 또는 수정 | 대체 경로 (파일명은 구현 시 확정) |
| `webassembly/src/**` | 수정 | include 경로 치환 (31개 기준) |
| `webassembly/cmake/modules/wb_workspace.cmake` | 수정 | legacy 소스 엔트리 제거 |
| `docs/refactoring/phase19/logs/phase19_inventory_snapshot_latest.md` | 갱신 | W8 후 최신 수치 반영 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W8.md` | **신규** | W8 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `webassembly/src/**/legacy` 디렉터리 | find | 0 (유지) |
| `legacy_atoms_runtime.{h,cpp}` 존재 | find | 0 |
| `workspace/runtime/legacy_atoms_runtime.h` include 참조 | rg | 0 |
| `wb_workspace.cmake`의 `legacy_atoms_runtime.cpp` 엔트리 | rg | 0 |
| `build-wasm:debug` | 빌드 | PASS |
| `build-wasm:release` | 빌드 | PASS |

## 7. 리스크 및 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 대량 include 치환 중 경로 누락 | 컴파일 실패 | 치환 후 `rg` 재스캔 + 빌드 즉시 검증 |
| 대체 파일 책임 정의 불명확 | 런타임 결합도 증가 | 기존 함수 시그니처/소유권 유지 후 경로만 우선 치환 |
| W9 게이트 설계와 충돌 | 재작업 | W8에서는 물리 경로 삭제에 집중하고 심볼/메타게이트는 W9에서 봉인 |

## 8. 산출물

- `legacy_atoms_runtime` 물리 삭제 커밋
- 대체 경로 코드 및 include 치환
- 갱신된 `wb_workspace.cmake`
- `phase19_inventory_snapshot_latest.md`
- `dependency_gate_report_W8.md`

## 9. Git 커밋 템플릿

```text
refactor(cleanup): phase19/W8 - remove remaining legacy runtime path and clean build graph

- Delete workspace/runtime/legacy_atoms_runtime.{h,cpp}
- Replace includes with non-legacy runtime reference path
- Remove legacy runtime source entry from wb_workspace.cmake
- Refresh W8 inventory and gate report

metrics:
  legacy directories: 0 -> 0
  legacy runtime files: 2 -> 0
  legacy runtime include refs: 31 -> 0
  wb_workspace legacy entries: 1 -> 0

verif:
  build-wasm:debug: PASS
  build-wasm:release: PASS
```

---

*W8 완료 기준은 "남은 legacy 물리 경로를 0으로 만들고, 빌드 그래프에서 재유입 경로를 제거하는 것"이다.*
