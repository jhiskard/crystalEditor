# Phase 19 / W0 — 기준선 고정 및 인벤토리 동결

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 독립평가 5.1/5.2 전 항목 + v2 계획서 2장(기준선)
우선순위: **최우선**
예상 소요: 0.5 ~ 1일
선행 의존: 없음
후속 작업: W1~W10 전체

---

## 1. 배경

Phase 19는 "격리 유지"가 아니라 "실질 이관 + 물리 삭제"를 목표로 한다. 이 목표를 추적 가능하게 만들려면 시작 시점의 정량 기준선이 먼저 고정되어야 한다. W0는 이후 모든 WBS의 성과를 비교하는 단일 기준점을 만드는 단계다.

## 2. 현재 상태 (기준선 값)

| 항목 | 기준선 값 |
|---|---:|
| `webassembly/src/**/legacy/` 디렉터리 수 | 5 |
| legacy 디렉터리 내부 파일 수 | 12 |
| 디렉터리 외 legacy 파일 수 | 2 |
| 해체 대상 LOC 합계 | 7,872 |
| `LegacyAtomsRuntime()` 호출 파일 수 | 14 |
| `workspace/legacy/` 문자열 참조 파일 수 | 32 |
| `friend class` 선언 수 | 1 |
| `DECLARE_SINGLETON` 사용 수 (정의 포함) | 4 |
| `::Instance()` 호출 수 | 48 |
| `webassembly/src/app.cpp` LOC | 1,430 |

## 3. 동결 범위

### 3.1 코드 인벤토리

- legacy 경로/파일 목록
- legacy 관련 심볼(`AtomsTemplate`, `LegacyAtomsRuntime`, `legacy_*_port`, `legacy_viewer_facade`)
- singleton 관련 심볼(`DECLARE_SINGLETON`, `::Instance()`)
- `app.cpp` 메서드/LOC 분해 매트릭스

### 3.2 산출물 동결 파일

- `docs/refactoring/phase19/logs/phase19_inventory_snapshot_latest.md`
- `docs/refactoring/phase19/logs/legacy_runtime_caller_inventory_phase19_latest.md`
- `docs/refactoring/phase19/logs/atoms_template_ownership_matrix_phase19_latest.md`
- `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md`
- `docs/refactoring/phase19/logs/dependency_gate_report_W0.md`

## 4. 실행 순서

1. `webassembly/src` 하위 legacy 디렉터리/파일 전수 스캔 후 목록화.
2. `AtomsTemplate`, `LegacyAtomsRuntime`, `legacy_*_service_port`, `legacy_viewer_facade` 호출자 전수 스캔.
3. `DECLARE_SINGLETON`, `::Instance()` 사용처 전수 스캔.
4. `app.cpp`의 함수 목록과 책임 분해표(유지/이관) 작성.
5. 위 4개 기준선을 로그 문서로 확정하고 수치를 `dependency_gate_report_W0.md`에 고정.
6. 이후 WBS에서 기준선을 갱신하지 않고 "Before(W0) / After(WN)" 방식으로만 비교 기록.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `docs/refactoring/phase19/logs/phase19_inventory_snapshot_latest.md` | **신규** | legacy 디렉터리/파일/LOC 기준선 |
| `docs/refactoring/phase19/logs/legacy_runtime_caller_inventory_phase19_latest.md` | **신규** | `LegacyAtomsRuntime()` 호출자 목록 |
| `docs/refactoring/phase19/logs/atoms_template_ownership_matrix_phase19_latest.md` | **신규** | W5 책임 귀속 매트릭스 |
| `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md` | **신규** | W7 이관 매트릭스 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W0.md` | **신규** | W0 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| 기준선 로그 4종 존재 | 파일 체크 | 전부 존재 |
| 기준선 수치 고정 여부 | `dependency_gate_report_W0.md` | 표로 명시 완료 |
| `phase19_inventory_snapshot_latest.md`의 legacy 디렉터리 수 | 수동 확인 | 5 |
| `legacy_runtime_caller_inventory_phase19_latest.md`의 호출 파일 수 | 수동 확인 | 14 |
| `app_cpp_decomposition_matrix_phase19_latest.md`의 총 LOC | 수동 확인 | 1,430 |
| W1 착수 기준선 참조 링크 | W1 문서 | W0 산출물 링크 명시 |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 기준선 측정 중 코드가 동시 변경됨 | 기준값 왜곡 | W0 측정 시점 커밋 SHA를 로그 상단에 고정 |
| grep 기준 불일치로 WBS별 수치 비교 불가 | 진행률 추적 실패 | W0에서 사용한 명령/패턴을 로그 하단에 고정 |
| W5/W7 매트릭스가 과소 작성됨 | 후속 작업 누락 | 함수 단위 ID를 부여해 "미분류 0건" 확인 |

## 8. 산출물

- 기준선 로그 4종
- `dependency_gate_report_W0.md`
- W1~W10에서 재사용할 측정 명령 패턴

## 9. Git 커밋 템플릿

```
docs(refactor): phase19/W0 — lock baseline inventory and metrics

- Freeze legacy directory/file/LOC baseline
- Capture LegacyAtomsRuntime callers and AtomsTemplate ownership matrix
- Capture app.cpp decomposition matrix baseline
- Publish dependency_gate_report_W0.md

metrics:
  legacy dirs: 5
  legacy files (in-dir): 12
  legacy files (out-of-dir): 2
  target LOC: 7872
  LegacyAtomsRuntime caller files: 14
  friend class: 1
  DECLARE_SINGLETON uses: 4
  ::Instance() calls: 48
  app.cpp LOC: 1430
```

---

*W0는 Phase 19의 측정 기준점을 확정하는 단계다. 이후 모든 WBS 완료 보고는 W0 기준선 대비 변화량으로만 판정한다.*
