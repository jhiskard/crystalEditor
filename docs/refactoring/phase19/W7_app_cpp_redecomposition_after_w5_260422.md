# Phase 19 / W7 - `app.cpp` 재분해 (W5 이후 실행)

작성일: `2026-04-22 (KST)`
최종 업데이트: `2026-04-27 (KST, 실행 완료 반영)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 5.1 R4 (`app.cpp` 과밀), 5.2 Medium 권고
우선순위: **Medium**
선행 의존: W5 (**필수**), W6 (권장)
후속 작업: W8, W9

---

## 1. 배경

`app.cpp`는 Phase 11 이후에도 UI/레이아웃/팝업/스타일 책임이 집중된 상태로 유지되어 있었다. W7의 목적은 `app.cpp`를 엔트리/오케스트레이션 셸로 축소하고, UI 책임을 `shell/presentation` 하위 모듈로 분리하는 것이다.

## 2. 착수 전 정합성 점검 (2026-04-27)

| 항목 | 점검값 | 비고 |
|---|---:|---|
| `webassembly/src/app.cpp` LOC | 1,278 | W0 기준선(1,430) 대비 일부 감소 상태였으나 여전히 과밀 |
| `app.cpp` 내부 `App::` 메서드 정의 수 | 40 | UI/상태/팝업/스타일 책임 혼재 |
| W7 대상 파일 존재 | 0/12 | `main_menu/window_layout/window_registry/popup/font/theme` 파일군 미생성 |
| W7 크기 게이트 스크립트 | 없음 | `check_phase19_app_cpp_size.ps1` 신규 필요 |

## 3. 업데이트된 W7 실행 계획

### 3.1 파일 분해 단위

- `shell/presentation/main_menu.{h,cpp}`
- `shell/presentation/window_layout.{h,cpp}`
- `shell/presentation/window_registry.{h,cpp}`
- `shell/presentation/popup_presenter.{h,cpp}`
- `shell/presentation/font/font_scale_controller.{h,cpp}`
- `shell/presentation/theme/color_style_controller.{h,cpp}`

### 3.2 책임 분리 원칙

- `app.cpp`: 생성/종료, 렌더 루프 진입, 공용 유틸리티(static tooltip/text metrics)만 유지
- `main_menu/window_layout/window_registry`: DockSpace, 메뉴, 레이아웃 적용, 윈도우 렌더 분리
- `popup/font/theme`: 팝업/폰트 스케일/색상 스타일 책임 분리

### 3.3 검증 게이트

- `check:phase19:app-cpp-size` 추가 (`app.cpp <= 400`)
- `build-wasm:release`
- `test:cpp`
- `test:smoke`

## 4. 실행 결과 (W7)

1. 계획서 기준 12개 presentation 분해 파일 생성 및 구현 이관 완료.
2. `app.cpp`를 경량 엔트리 셸로 축소.
3. `wb_shell` 빌드 타깃에 분해 파일 등록 및 include 경로 정렬.
4. `check_phase19_app_cpp_size.ps1` 및 `npm` 스크립트 추가.

## 5. 파일 변경 명세 (실행 반영)

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/shell/presentation/main_menu.h` | 신규 | W7 분해 |
| `webassembly/src/shell/presentation/main_menu.cpp` | 신규 | DockSpace/메뉴 렌더 |
| `webassembly/src/shell/presentation/window_layout.h` | 신규 | 레이아웃 구조체/헬퍼 선언 |
| `webassembly/src/shell/presentation/window_layout.cpp` | 신규 | 레이아웃 계산/`RequestLayout1` |
| `webassembly/src/shell/presentation/window_registry.h` | 신규 | 윈도우명 accessor 선언 |
| `webassembly/src/shell/presentation/window_registry.cpp` | 신규 | 윈도우 렌더/상태 동기화/ini 입출력 |
| `webassembly/src/shell/presentation/popup_presenter.h` | 신규 | W7 분해 |
| `webassembly/src/shell/presentation/popup_presenter.cpp` | 신규 | About/Progress popup 및 관련 helper |
| `webassembly/src/shell/presentation/font/font_scale_controller.h` | 신규 | W7 분해 |
| `webassembly/src/shell/presentation/font/font_scale_controller.cpp` | 신규 | 폰트 스케일 책임 |
| `webassembly/src/shell/presentation/theme/color_style_controller.h` | 신규 | W7 분해 |
| `webassembly/src/shell/presentation/theme/color_style_controller.cpp` | 신규 | 색상 스타일 책임 |
| `webassembly/src/app.cpp` | 수정 | 엔트리/오케스트레이션 중심으로 축소 |
| `webassembly/cmake/modules/wb_shell.cmake` | 수정 | 분해 파일 등록 + include 경로 추가 |
| `scripts/refactoring/check_phase19_app_cpp_size.ps1` | 신규 | LOC 게이트 |
| `package.json` | 수정 | `check:phase19:app-cpp-size` 추가 |
| `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md` | 갱신 | W7 결과 반영 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W7.md` | 신규 | W7 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 | 결과 |
|---|---|---|---|
| `app.cpp` LOC | line count | <= 400 | **73 (PASS)** |
| 분해 파일(12개) 존재 | 파일 체크 | 전부 존재 | **PASS** |
| `check_phase19_app_cpp_size.ps1` | 스크립트 실행 | PASS | **PASS** |
| WASM 릴리즈 빌드 | `build-wasm:release` | PASS | **PASS** |
| C++ 테스트 | `test:cpp` | PASS | **PASS** |
| E2E 스모크 | `test:smoke` | PASS | **PASS (2 passed)** |

## 7. 리스크 및 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 분해 후 include 경로 누락 | 컴파일 실패 | `wb_shell` include 경로 명시 + 빌드 재검증 |
| 메뉴/레이아웃 동작 회귀 | UI 동작 이상 | `layout-panel` 회귀 스모크 테스트로 확인 |
| 책임 분리 중 심볼 누락 | 링크 실패 | 빌드 전수 재검증 및 파일 단위 소유 분리 |

## 8. 산출물

- W7 분해 코드 12파일
- 축소된 `app.cpp`
- `check_phase19_app_cpp_size.ps1`
- `dependency_gate_report_W7.md`
- 갱신된 `app_cpp_decomposition_matrix_phase19_latest.md`

## 9. Git 커밋 템플릿

```text
refactor(shell): phase19/W7 - redecompose app.cpp after W5/W6 baseline

- Extract dock/menu/layout/window/popup/font/theme responsibilities from app.cpp
- Keep app.cpp as lightweight entry/orchestration shell
- Add app.cpp size gate and wire W7 modules into wb_shell

metrics:
  app.cpp LOC: 1278 -> 73
  extracted presentation files: +12

verif:
  check:phase19:app-cpp-size: PASS
  build-wasm:release: PASS
  test:cpp: PASS
  test:smoke: PASS
```

---

*W7 완료 기준에서 `app.cpp`는 경량 오케스트레이터로 축소되었고, UI 책임은 presentation 계층으로 재배치되었다.*
