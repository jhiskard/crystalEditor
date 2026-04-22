# Phase 19 / W1 — `friend class` 1건 해소

작성일: `2026-04-21 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_claude2_260421.md`
검증보고서 근거: 5.1 R3 (`friend\s+class` 1건 잔존), 5.2 High 권고
우선순위: **High**
예상 소요: 0.5 ~ 1일
선행 의존: W0 (기준선 고정)
후속 작업: W3 (io/infrastructure/legacy 제거 시 FileLoader 내부 구조 확정에 본 작업의 public API 재정리가 선행되어야 함)

---

## 1. 배경

독립 검증보고서 5.1 R3은 `io/application/import_entry_service.h:76`의 단 한 건 `friend class shell::presentation::ImportPopupPresenter;` 선언을 보고한다. Phase 14의 DoD에는 "`webassembly/src` 내 `friend class` 0"이 포함되어 있었으나, `check_phase14_state_ownership_alias_zero.ps1` 게이트는 이 선언을 감지하지 못했다. 본 작업은 Phase 14 DoD 지연 달성과 Phase 19 전체 friend-class 0 상태 봉인을 동시에 수행한다.

## 2. 현재 상태 (2026-04-21 측정)

| 항목 | 값 |
|---|---|
| `webassembly/src` 전체 `friend\s+class` 발생 건수 | 1 |
| 선언 위치 | `io/application/import_entry_service.h:76` |
| 수혜 클래스 | `shell::presentation::ImportPopupPresenter` |
| `ImportPopupPresenter`가 접근하는 `FileLoader` private 멤버 (추정) | ~8 |

### 2.1 `FileLoader` private 멤버 영향 범위

현재 `FileLoader`(=`import_entry_service.h`의 실체 클래스)의 private 멤버 중 `ImportPopupPresenter`가 접근한다고 추정되는 항목:

- `m_ShowStructureReplacePopup`
- `m_ReplaceSceneOnNextStructureImport`
- `m_DeferredStructureFileName`
- `m_ShowStructureImportErrorPopup`
- `m_StructureImportErrorTitle`
- `m_StructureImportErrorMessage`
- `m_ShowXsfGridCellWarningPopup`
- `m_XsfGridCellWarningText`

이 멤버군은 공통적으로 **"import 파이프라인이 popup presenter에게 노출하는 UI 일시 상태"** 이며, 팝업 본문 렌더에 필요하다.

## 3. 해소 설계 (옵션 A 채택)

### 3.1 선택지 비교

| 옵션 | 장점 | 단점 |
|---|---|---|
| **A. public API + 값 객체** | 계약이 명시적, 테스트 더블 작성 가능 | `FileLoader`의 public 표면이 늘어남 |
| B. setter 콜백 주입 | presenter가 값만 받음 | 호출 순서/생명주기 복잡도 상승 |
| C. allowlist 유지 | 코드 변경 최소 | DoD 지연 달성 실패, 구조적 봉인 불가 |

→ **옵션 A**를 채택한다. `ImportPopupPresenter`는 `FileLoader`의 private 멤버에 직접 접근하지 않고, `FileLoader`가 제공하는 **`ImportPopupState` 값 객체**만 읽는다.

### 3.2 신규 값 객체 정의

```cpp
// webassembly/src/io/application/import_popup_state.h (신규)
namespace wb::io::application {

struct ImportPopupState {
    // structure replace
    bool showStructureReplacePopup = false;
    std::string deferredStructureFileName;

    // structure import error
    bool showStructureImportErrorPopup = false;
    std::string structureImportErrorTitle;
    std::string structureImportErrorMessage;

    // XSF grid cell warning
    bool showXsfGridCellWarningPopup = false;
    std::string xsfGridCellWarningText;

    // generic
    bool replaceSceneOnNextStructureImport = false;
};

} // namespace wb::io::application
```

### 3.3 `FileLoader` public API 확장

```cpp
// 기존: class FileLoader { ... private: friend class ... ; m_Show...; }
// 변경 후:
public:
    const ImportPopupState& getImportPopupState() const noexcept;
    void acknowledgeStructureReplacePopup(bool shouldReplace); // Yes/No 처리
    void dismissStructureImportErrorPopup();
    void dismissXsfGridCellWarningPopup();
private:
    ImportPopupState m_ImportPopupState;
    // friend class 선언 **제거**
```

### 3.4 `ImportPopupPresenter` 재배선

1. `ImportPopupPresenter`는 `FileLoader*` 대신 `FileLoader&`를 생성자로 받고, 내부에서 `m_FileLoader.getImportPopupState()`를 호출.
2. 팝업 내부의 Yes/No/Close 버튼은 `FileLoader`의 명시 메서드(`acknowledgeStructureReplacePopup(true/false)`, `dismissStructureImportErrorPopup()`, `dismissXsfGridCellWarningPopup()`)를 호출.
3. `FileLoader*` 멤버가 `std::reference_wrapper<FileLoader>` 또는 raw 참조로 전환됨.

## 4. 실행 순서

1. **접근 인벤토리 작성**: `shell/presentation/import_popup_presenter.cpp`에서 `m_FileLoader->m_Show...`, `m_FileLoader->m_Deferred...` 등 private 접근 전수 grep → 리스트 확정.
   산출물: `docs/refactoring/phase19/logs/friend_class_access_inventory_phase19_latest.md`
2. **값 객체 도입**: `import_popup_state.h` 신규, `ImportPopupState` 초기값 포함.
3. **`FileLoader` 내부 멤버를 `ImportPopupState`로 집약**: 개별 `m_Show*`, `m_*Text` 멤버를 `m_ImportPopupState`로 이동.
4. **public API 추가**: `getImportPopupState()`, `acknowledgeStructureReplacePopup(bool)`, `dismissStructureImportErrorPopup()`, `dismissXsfGridCellWarningPopup()` 등 이관 대상 동작별 명시 메서드.
5. **Presenter 재배선**: `ImportPopupPresenter`의 내부 접근을 값 객체 read + 명시 API call로 전환.
6. **`friend class` 선언 제거**: `import_entry_service.h:76` 삭제.
7. **게이트 스크립트 추가**: `scripts/refactoring/check_phase19_friend_class_zero.ps1`
   - 동작: `webassembly/src` 하위 `friend\s+class` grep 결과 0 확인
   - package.json: `"check:phase19:friend-class-zero": "powershell -File scripts/refactoring/check_phase19_friend_class_zero.ps1"`
8. **기존 게이트 재실행**: `check:phase14:state-ownership-alias-zero` PASS 확인.
9. **회귀 테스트**:
   - `tests/e2e/workbench-smoke.spec.ts` — Import 시나리오 (XYZ, XSF, CHGCAR)
   - `tests/e2e/workbench-layout-panel-regression.spec.ts` — structure replace popup 동작
   - 수동: structure replace / import error / XSF grid warning 팝업 각 1회 발현 확인

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/io/application/import_popup_state.h` | **신규** | 값 객체 정의 |
| `webassembly/src/io/application/import_entry_service.h` | 수정 | `friend class` 제거, public API 확장, 멤버 집약 |
| `webassembly/src/io/application/import_entry_service.cpp` | 수정 | 멤버 이름 변경에 따른 접근 수정 |
| `webassembly/src/shell/presentation/import_popup_presenter.h` | 수정 | `FileLoader*`→`FileLoader&`, 멤버 접근 경로 변경 |
| `webassembly/src/shell/presentation/import_popup_presenter.cpp` | 수정 | 값 객체 read, 명시 API call로 전환 |
| `scripts/refactoring/check_phase19_friend_class_zero.ps1` | **신규** | 게이트 스크립트 |
| `package.json` | 수정 | `check:phase19:friend-class-zero` 엔트리 추가 |
| `docs/refactoring/phase19/logs/friend_class_access_inventory_phase19_latest.md` | **신규** | 접근 인벤토리 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W1.md` | **신규** | W1 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `webassembly/src` 전체 `friend\s+class` | `grep -rn "friend\s\+class" webassembly/src` | 0 |
| `check:phase14:state-ownership-alias-zero` | npm script | PASS |
| `check:phase19:friend-class-zero` | npm script | PASS |
| `import_popup_state.h` 존재 | 파일 체크 | 존재 |
| `ImportPopupPresenter`의 `FileLoader` private 접근 | 정적 분석 | 0 |
| smoke/e2e 회귀 | Playwright | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| Presenter가 다중 popup 상태를 동시 조작하는 로직에서 순서 의존성 발견 | 팝업 오동작 | 각 팝업별 독립 명시 API로 분리(이 계획서의 4.3 설계) |
| `ImportPopupState` 복사 비용 | 경미 | `const ImportPopupState&` 반환으로 복사 회피 |
| CI에서 임포트 팝업 자동화 누락 | 회귀 감지 실패 | 수동 체크리스트 + `phase19/logs/manual_ui_regression_W1_latest.md` 작성 |

## 8. 산출물

- `webassembly/src/io/application/import_popup_state.h` (신규)
- 수정된 `FileLoader` 2파일 + `ImportPopupPresenter` 2파일
- `scripts/refactoring/check_phase19_friend_class_zero.ps1`
- `docs/refactoring/phase19/logs/friend_class_access_inventory_phase19_latest.md`
- `docs/refactoring/phase19/logs/dependency_gate_report_W1.md`
- (선택) `docs/refactoring/phase19/logs/manual_ui_regression_W1_latest.md`

## 9. Git 커밋 템플릿

```
refactor(io): phase19/W1 — remove friend class from FileLoader

- Introduce ImportPopupState value object
- Expose explicit public API for popup ack/dismiss
- ImportPopupPresenter reads via FileLoader::getImportPopupState()
- Drop friend class declaration (import_entry_service.h:76)

metrics:
  friend\s+class in webassembly/src: 1 -> 0
  FileLoader public methods: +4
  private UI state members: 8 -> 1 (aggregated into ImportPopupState)

verif:
  check:phase14:state-ownership-alias-zero: PASS
  check:phase19:friend-class-zero: PASS
  e2e workbench-smoke: PASS
  e2e workbench-layout-panel-regression: PASS
```

---

*W1은 Phase 14의 미달성 DoD(friend class 0) 지연 달성을 겸한다. 완료 시점에 검증보고서 5.1 R3가 "해소됨"으로 갱신될 수 있다.*
