# Phase 19 / W5 — `AtomsTemplate` 완전 분해

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 5.1 R1 (대형 단일 파일), 5.2 핵심 권고
우선순위: **최고 작업량**
예상 소요: 7 ~ 10일
선행 의존: W3, W4
후속 작업: W6, W7, W8

---

## 1. 배경

`workspace/legacy/atoms_template_facade.cpp`는 Phase 19에서 반드시 해체해야 할 최대 리스크다. W5의 목표는 `AtomsTemplate` 책임을 기능별 모듈로 이관하고, `LegacyAtomsRuntime` 경로를 제거하는 것이다.

## 2. 현재 상태

| 항목 | 기준선 |
|---|---:|
| `atoms_template_facade.h` LOC | 1,485 |
| `atoms_template_facade.cpp` LOC | 5,808 |
| `LegacyAtomsRuntime()` 호출 파일 수 | 14 |
| `workspace/legacy/` 참조 파일 수 | 32 |

## 3. 운영 규칙 (v2 필수)

1. `W5.1~W5.9`는 **병렬 금지, 순차 실행**.
2. 각 단계에서 **Thin-shim 3단계** 허용:
   - 1차: 신규 서비스 이관
   - 2차: 호출자 전환
   - 3차: shim 제거
3. 각 서브 단계 종료 시 회귀 테스트 + 게이트 증빙 필수.
4. 단계 종료마다 `atoms_template_call_graph_phase19_latest.md` 갱신.

## 4. 실행 순서

### W5.1 Atom/Bond/Cell

- 대상: 선택/hover/visibility/cell 관련 메서드
- 귀속: `structure/application/structure_interaction_service`

### W5.2 Measurement

- 대상: measurement click 처리/모드/overlay
- 귀속: `measurement/application/measurement_service` + presentation 컴포넌트

### W5.3 Density

- 대상: charge density 로드/표시/파이프라인 제어
- 귀속: `density/application` + `density/presentation`

### W5.4 Structure lifecycle/BZ

- 대상: structure 추가/교체/삭제, BZ 계산
- 귀속: `structure/application/structure_lifecycle_service`

### W5.5 UI Render Window

- 대상: `AtomsTemplate` 내 ImGui window 렌더 함수
- 귀속: `shell/presentation/atoms/*` 또는 대응 presenter

### W5.6 XSF/CHGCAR bridge

- 대상: XSF/CHGCAR 브리지 함수
- 귀속: `io/application/import_*_service`

### W5.7 `workspace/legacy` 4파일 삭제

- `atoms_template_facade.{h,cpp}`
- `legacy_atoms_runtime.{h,cpp}`

### W5.8 CMake 정리

- `wb_workspace.cmake`에서 legacy 소스 완전 제거

### W5.9 심볼 제로화 (`AtomsTemplate`, `LegacyAtomsRuntime`)

- 대상: `webassembly/src/**`의 타입명/전방 선언/include/alias 잔존 참조
- 실행: 코드 기준 심볼 인벤토리 동결(`docs/**` 제외 후 `rg` 기준선 확정)
- 실행: 잔존 호출자/헤더를 중립 명칭 기반 경로로 치환 후 shim 제거
- 검증: 재검색으로 `AtomsTemplate`, `LegacyAtomsRuntime` 심볼 0 확인
- 산출: `atoms_template_decomposition_progress_W5_9.md` + call graph 최신화

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/workspace/legacy/atoms_template_facade.h` | **삭제** | W5.7 |
| `webassembly/src/workspace/legacy/atoms_template_facade.cpp` | **삭제** | W5.7 |
| `webassembly/src/workspace/legacy/legacy_atoms_runtime.h` | **삭제** | W5.7 |
| `webassembly/src/workspace/legacy/legacy_atoms_runtime.cpp` | **삭제** | W5.7 |
| `webassembly/src/structure/application/**` | 수정/신규 | W5.1, W5.4 |
| `webassembly/src/measurement/application/**` | 수정/신규 | W5.2 |
| `webassembly/src/density/application/**` | 수정/신규 | W5.3 |
| `webassembly/src/density/presentation/**` | 신규/수정 | W5.3 |
| `webassembly/src/shell/presentation/**` | 신규/수정 | W5.5 |
| `webassembly/src/io/application/**` | 수정 | W5.6 |
| `webassembly/cmake/modules/wb_workspace.cmake` | 수정 | W5.8 |
| `webassembly/src/**` | 수정/치환 | W5.9 (잔존 심볼 제거) |
| `docs/refactoring/phase19/logs/atoms_template_call_graph_phase19_latest.md` | **신규/갱신** | 단계별 갱신 |
| `docs/refactoring/phase19/logs/w5_state_ownership_matrix_phase19_latest.md` | **신규** | 상태 소유권 기록 |
| `docs/refactoring/phase19/logs/atoms_template_decomposition_progress_W5_9.md` | **신규** | W5.9 증빙 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W5.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `AtomsTemplate` 심볼 | grep | 0 |
| `LegacyAtomsRuntime` 심볼 | grep | 0 |
| `webassembly/src/workspace/legacy/` 존재 | find | 없음 |
| W5.1~W5.9 순차 증빙 | 로그 | 전 단계 기록 완료 |
| `atoms_template_call_graph_phase19_latest.md` | 문서 체크 | 최신 반영 |
| `atoms_template_decomposition_progress_W5_9.md` | 문서 체크 | 작성 완료 |
| 회귀 테스트 | 단위/e2e/수동 | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 메서드 이관 누락 | 런타임 결손 | W0 소유권 매트릭스 기반으로 "미이관 0건" 확인 |
| 중간 단계 회귀 대량 발생 | 일정 지연 | 서브 단계 단위 게이트/회귀 강제 |
| shim 잔존 방치 | 기술부채 재유입 | 각 shim에 제거 목표 WBS + 추적 ID 부여 |

## 8. 산출물

- `workspace/legacy` 삭제 결과
- 기능별 신규/수정 서비스 및 presenter
- 단계별 call graph/소유권 로그
- W5.9 심볼 제로화 증빙 로그
- `dependency_gate_report_W5.md`

## 9. Git 커밋 템플릿

```
refactor(workspace): phase19/W5 — fully decompose AtomsTemplate and remove legacy runtime

- Execute W5.1~W5.9 sequential migration with thin-shim strategy
- Delete workspace/legacy files (atoms_template_facade + legacy_atoms_runtime)
- Update workspace cmake sources
- Execute W5.9 symbol zeroization for AtomsTemplate and LegacyAtomsRuntime
- Refresh call graph and ownership matrix logs

metrics:
  AtomsTemplate references: N -> 0
  LegacyAtomsRuntime caller files: 14 -> 0
  workspace/legacy directory: present -> removed

verif:
  stage regressions (W5.1~W5.9): PASS
  build/test gates: PASS
```

---

*W5 완료는 Phase 19의 핵심 분기점이며, 이후 W6~W10은 봉인과 종료 절차에 집중한다.*
