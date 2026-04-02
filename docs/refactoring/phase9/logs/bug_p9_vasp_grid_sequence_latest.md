# P9-BUG-01 재현 로그 (W0 Restart Baseline)

등록일: `2026-04-02 (KST)`  
이슈 ID: `P9-BUG-01`  
연계 문서: `docs/refactoring/phase8/go_no_go_phase9.md`

## 1. 증상
- `XSF(Grid) -> VASP -> XSF(Grid) -> VASP` 순서로 파일을 로드하면,
  이전 grid 데이터가 완전히 제거되지 않고 남아 있는 현상이 발생한다.

## 2. 재현 절차 (W0 고정)
1. 페이지 새로고침 후 `XSF(Grid 포함)` 파일을 로드한다.
2. `VASP` 파일을 로드한다.
3. 다시 `XSF(Grid 포함)` 파일을 로드한다.
4. 다시 `VASP` 파일을 로드한다.
5. Model Tree 및 Viewer에서 grid/isovalue 잔존 여부를 확인한다.

## 3. 기대 결과 / 실제 결과
- 기대 결과:
  - VASP 로드 시 기존 XSF grid 데이터가 완전히 정리된다.
  - 이후 VASP 기준 구조/표시 상태만 남는다.
- 실제 결과:
  - 특정 순서에서 기존 grid 표시/상태가 남아 후속 표시와 중첩된다.

## 4. 현재 상태
- 상태: `Open`
- 우선순위: `Medium`
- 비고:
  - 기존 수정 시도 2건 롤백됨.
  - Phase 9 W4에서 원인 귀속 및 수정 반영.

## 5. Phase 9 처리 규칙
1. W0에서 재현 절차와 관찰 포인트를 고정한다.
2. W3~W4에서 clear/apply/rollback 및 actor detach 흐름을 점검한다.
3. W6 종료 문서(`dependency_gate_report.md`, `go_no_go_phase10.md`)에 해결/이관 상태를 명시한다.

## 6. W3 점검 메모 (2026-04-02 KST)
- W3 범위에서 `render` 경계 전환(포트/어댑터 경유)을 우선 적용했다.
- 현재 상태:
  - `P9-BUG-01`: `Open`
  - 원인 가설 후보: import apply/rollback 순서 또는 workspace/grid 컨텍스트 clear 누락
- W4에서 확인할 사항:
  1. VASP 로딩 직전/직후 grid actor detach 누락 여부
  2. import 실패/재시도 경로에서 rollback 이후 상태 잔존 여부
  3. 구조 전환 후 density context reset 순서

## 7. W4 원인 귀속 및 코드 수정 (2026-04-02 KST)
- 원인 귀속:
  - `ChargeDensityUI`가 VASP(CHGCAR) 로드 경로에서 `m_gridDataEntries`를 비우지 않아,
    이전 XSF grid 메타데이터가 잔존했다.
  - 이 상태에서 UI/렌더 상태 갱신 시 이전 grid 엔트리가 재사용되어 잔존 표시가 재발할 수 있었다.
- 수정 적용:
  - `webassembly/src/atoms/ui/charge_density_ui.cpp`
    1. `loadFile()`에서 비-grid 로드 시작 시 `m_gridDataEntries.clear()` 및 인덱스 초기화
    2. `clear()`에서 `m_gridDataEntries.clear()` 및 인덱스 초기화
    3. `loadFromParseResultInternal(..., loadedFromGrid=false)` 경로에서 `m_gridDataEntries.clear()` 및 인덱스 초기화
- 판정:
  - 상태: `Open` (코드 수정 반영 완료, W6 수동 재현 시나리오로 최종 확인 예정)
