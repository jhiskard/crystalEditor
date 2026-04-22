# Phase 19 / W1 — `friend class` 1건 해소

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 5.1 R3 (`friend\s+class` 1건 잔존), 5.2 High 권고
우선순위: **High**
예상 소요: 0.5 ~ 1일
선행 의존: W0 (기준선 고정)
후속 작업: W3 (io 서비스 내재화), W9 (`check_phase19_friend_class_zero.ps1` 메타게이트 편입)

---

## 1. 배경

독립평가 리스크 중 즉시 해소 가능한 High 항목은 `friend class` 잔존 1건이다. 대상은 `io/application/import_entry_service.h`의 `friend class shell::presentation::ImportPopupPresenter;` 선언이며, 이는 캡슐화 위반이자 상태 소유권 규칙 위반 신호다.

## 2. 현재 상태

| 항목 | 값 |
|---|---|
| `webassembly/src` 전체 `friend\s+class` 발생 건수 | 1 |
| 선언 위치 | `io/application/import_entry_service.h:76` |
| 수혜 클래스 | `shell::presentation::ImportPopupPresenter` |
| private 멤버 직접 접근 추정 수 | 약 8개 |

### 2.1 영향 멤버군

- `m_ShowStructureReplacePopup`
- `m_ReplaceSceneOnNextStructureImport`
- `m_DeferredStructureFileName`
- `m_ShowStructureImportErrorPopup`
- `m_StructureImportErrorTitle`
- `m_StructureImportErrorMessage`
- `m_ShowXsfGridCellWarningPopup`
- `m_XsfGridCellWarningText`

## 3. 해소 설계 (옵션 A)

### 3.1 선택지 비교

| 옵션 | 장점 | 단점 |
|---|---|---|
| **A. 값 객체 + 명시 API** | 캡슐화 복원, 테스트 용이 | public API 증가 |
| B. 콜백 주입 | presenter 단순화 가능 | 수명/순서 복잡 |
| C. 현상 유지 | 코드 변경 최소 | DoD 미달 지속 |

W1은 **옵션 A**를 채택한다.

### 3.2 신규 값 객체

`io/application/import_popup_state.h`에 `ImportPopupState`를 추가하고, presenter는 해당 값 객체만 읽도록 변경한다.

### 3.3 `FileLoader` API

- `const ImportPopupState& getImportPopupState() const noexcept`
- `void acknowledgeStructureReplacePopup(bool shouldReplace)`
- `void dismissStructureImportErrorPopup()`
- `void dismissXsfGridCellWarningPopup()`

`friend class` 선언은 삭제한다.

## 4. 실행 순서

1. `ImportPopupPresenter`의 `FileLoader` private 접근 지점 인벤토리 작성.
2. `ImportPopupState` 값 객체 추가.
3. `FileLoader` 내부 popup 상태 멤버를 `ImportPopupState`로 집약.
4. presenter 호출 경로를 값 객체 read + 명시 메서드 호출로 전환.
5. `friend class` 선언 삭제.
6. 신규 게이트 스크립트 `check_phase19_friend_class_zero.ps1` 추가.
7. W1 완료 로그 `dependency_gate_report_W1.md` 작성.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/io/application/import_popup_state.h` | **신규** | 값 객체 |
| `webassembly/src/io/application/import_entry_service.h` | 수정 | `friend class` 제거, API 확장 |
| `webassembly/src/io/application/import_entry_service.cpp` | 수정 | 상태 접근 갱신 |
| `webassembly/src/shell/presentation/import_popup_presenter.h` | 수정 | 참조 방식 변경 |
| `webassembly/src/shell/presentation/import_popup_presenter.cpp` | 수정 | private 접근 제거 |
| `scripts/refactoring/check_phase19_friend_class_zero.ps1` | **신규** | 게이트 |
| `package.json` | 수정 | `check:phase19:friend-class-zero` 추가 |
| `docs/refactoring/phase19/logs/friend_class_access_inventory_phase19_latest.md` | **신규** | 접근 인벤토리 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W1.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `friend\s+class` in `webassembly/src` | grep | 0 |
| `ImportPopupPresenter`의 private 멤버 직접 접근 | 정적 검색 | 0 |
| `check_phase19_friend_class_zero.ps1` | 스크립트 실행 | PASS |
| Import popup 시나리오 회귀 | e2e/수동 | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| popup 상태 전환 시점 누락 | 팝업 오동작 | `acknowledge/dismiss` API로 상태전환 통로 단일화 |
| 값 객체 필드 누락 | UI 정보 불일치 | W1 인벤토리에서 기존 필드 전수 맵핑 |
| W3에서 io 시그니처 재변경 | 재작업 | W1에서 public API 계약을 먼저 안정화 |

## 8. 산출물

- `ImportPopupState` 신규 헤더
- `friend class` 제거된 io/shell 수정본
- `check_phase19_friend_class_zero.ps1`
- `friend_class_access_inventory_phase19_latest.md`
- `dependency_gate_report_W1.md`

## 9. Git 커밋 템플릿

```
refactor(io): phase19/W1 — remove friend class from FileLoader

- Introduce ImportPopupState value object
- Expose explicit popup state APIs on FileLoader
- Remove private member access from ImportPopupPresenter
- Drop friend class declaration

metrics:
  friend\s+class in webassembly/src: 1 -> 0
  FileLoader popup state members: 8 -> 1 aggregate object

verif:
  check:phase19:friend-class-zero: PASS
```

---

*W1 완료 시 독립평가 R3의 핵심 신호(`friend class`)가 즉시 0으로 봉인된다.*
