# Phase 19 / W7 — `app.cpp` 재분해 (W5 이후 실행)

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 5.1 R4 (`app.cpp` 과대), 5.2 Medium 권고
우선순위: **Medium**
예상 소요: 3 ~ 4일
선행 의존: W5 (**필수**), W6 (권장)
후속 작업: W8, W9

---

## 1. 배경

`app.cpp`는 기준선 1,430 LOC로 Phase 11 분해 목표가 미완료 상태다. v2 계획은 W7을 W5 이후로 이동시켜, legacy 해체가 끝난 안정된 런타임에서 UI/레이아웃 책임을 분리하도록 정의했다.

## 2. 목표 상태

| 항목 | 기준선 | 목표 |
|---|---:|---:|
| `webassembly/src/app.cpp` LOC | 1,430 | ≤ 400 |
| 메뉴/레이아웃 전담 파일 | 없음 | 실파일 생성 |
| `app.cpp` 책임 | UI 혼재 | 프레임 오케스트레이션/수명관리 중심 |

## 3. 설계

### 3.1 신규 파일 (v2 지정)

- `shell/presentation/main_menu.{h,cpp}`
- `shell/presentation/window_layout.{h,cpp}`
- `shell/presentation/window_registry.{h,cpp}`
- `shell/presentation/popup_presenter.{h,cpp}`
- `shell/presentation/font/font_scale_controller.{h,cpp}`
- `shell/presentation/theme/color_style_controller.{h,cpp}`

### 3.2 책임 분리 원칙

- `app.cpp`: 루프/초기화/종료/동기화
- shell presentation: ImGui 렌더, 메뉴, 레이아웃, 팝업, 스타일/폰트

## 4. 실행 순서

1. W0의 `app_cpp_decomposition_matrix`를 기준으로 함수별 이관 대상 확정.
2. `main_menu` 추출.
3. `window_layout` 추출(Layout1~3/Reset 포함).
4. `window_registry` 추출(창 등록/렌더 디스패치).
5. `popup_presenter` 추출(진행률/공지 팝업).
6. `font_scale_controller`, `color_style_controller` 추출.
7. `app.cpp`를 오케스트레이션 중심으로 축소.
8. `check_phase19_app_cpp_size.ps1` 추가 및 실행.
9. 레이아웃 회귀(e2e + 수동) 수행.
10. W7 완료 리포트 작성.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/shell/presentation/main_menu.h` | **신규** | |
| `webassembly/src/shell/presentation/main_menu.cpp` | **신규** | |
| `webassembly/src/shell/presentation/window_layout.h` | **신규** | |
| `webassembly/src/shell/presentation/window_layout.cpp` | **신규** | |
| `webassembly/src/shell/presentation/window_registry.h` | **신규** | |
| `webassembly/src/shell/presentation/window_registry.cpp` | **신규** | |
| `webassembly/src/shell/presentation/popup_presenter.h` | **신규** | |
| `webassembly/src/shell/presentation/popup_presenter.cpp` | **신규** | |
| `webassembly/src/shell/presentation/font/font_scale_controller.h` | **신규** | |
| `webassembly/src/shell/presentation/font/font_scale_controller.cpp` | **신규** | |
| `webassembly/src/shell/presentation/theme/color_style_controller.h` | **신규** | |
| `webassembly/src/shell/presentation/theme/color_style_controller.cpp` | **신규** | |
| `webassembly/src/app.cpp` | 수정 | 대폭 축소 |
| `webassembly/src/app.h` | 수정 | 멤버 재구성 |
| `webassembly/src/workspace/runtime/workbench_runtime.{h,cpp}` | 수정 | 조립/주입 |
| `webassembly/cmake/modules/wb_shell.cmake` | 수정 | 신규 소스 등록 |
| `scripts/refactoring/check_phase19_app_cpp_size.ps1` | **신규** | LOC 게이트 |
| `package.json` | 수정 | `check:phase19:app-cpp-size` 추가 |
| `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md` | 갱신 | W7 결과 반영 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W7.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `app.cpp` LOC | wc -l | ≤ 400 |
| `main_menu.cpp` 존재 | 파일 체크 | 있음 |
| `window_layout.cpp` 존재 | 파일 체크 | 있음 |
| `window_registry.cpp` 존재 | 파일 체크 | 있음 |
| `check_phase19_app_cpp_size.ps1` | 스크립트 실행 | PASS |
| 레이아웃/패널 회귀 | e2e/수동 | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| DockSpace ID 변경 | 사용자 레이아웃 깨짐 | 기존 ID 문자열 유지 |
| static 진입점 호환성 저하 | JS/WASM 연동 이슈 | 필요한 static 진입점은 얇은 delegator로 유지 |
| W6과 폰트 처리 중첩 | 회귀 가능성 | W6 완료 기준의 `FontManager` 주입 방식 재사용 |

## 8. 산출물

- shell presentation 신규 파일 세트
- 축소된 `app.cpp`
- `check_phase19_app_cpp_size.ps1`
- `dependency_gate_report_W7.md`

## 9. Git 커밋 템플릿

```
refactor(shell): phase19/W7 — redecompose app.cpp after W5 completion

- Extract menu/layout/window/popup/font/theme responsibilities from app.cpp
- Keep app.cpp as orchestration/lifecycle shell
- Add app.cpp size gate script

metrics:
  app.cpp LOC: 1430 -> <=400
  extracted presentation files: +12

verif:
  check:phase19:app-cpp-size: PASS
  layout/panel regression: PASS
```

---

*W7 완료 시 `app.cpp`는 대형 UI 호스트가 아니라 경량 엔트리/오케스트레이터로 정착한다.*
