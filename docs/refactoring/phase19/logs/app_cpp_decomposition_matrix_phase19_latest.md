# app.cpp Decomposition Matrix (Phase 19 Latest)

- Captured at: `2026-04-27 (KST)`
- Stage: `W7 app.cpp redecomposition`
- File: `webassembly/src/app.cpp`

## Baseline vs Current

| Metric | Baseline (W0) | W7 Input (2026-04-27) | Current (After W7) |
|---|---:|---:|---:|
| LOC | 1,430 | 1,278 | 73 |
| `App::` method definitions in `app.cpp` | 34 (plan baseline) | 40 | 10 |
| `App::` method definitions in `webassembly/src` total | 34 (plan baseline) | 40 | 40 |

## W7 Ownership Result

| Responsibility | Owner file(s) | Status |
|---|---|---|
| Main menu + DockSpace | `shell/presentation/main_menu.*` | DONE |
| Layout preset / reset geometry | `shell/presentation/window_layout.*` | DONE |
| Window rendering + shell state sync | `shell/presentation/window_registry.*` | DONE |
| About/Progress popup | `shell/presentation/popup_presenter.*` | DONE |
| Font scale controls | `shell/presentation/font/font_scale_controller.*` | DONE |
| Color style controls | `shell/presentation/theme/color_style_controller.*` | DONE |

## W7 Gate Snapshot

1. `npm run check:phase19:app-cpp-size` => `PASS` (`73 <= 400`)
2. `npm run build-wasm:release` => `PASS`
3. `npm run test:cpp` => `PASS`
4. `npm run test:smoke` => `PASS` (`2 passed`)

## Notes

- `app.cpp`는 엔트리/오케스트레이션/공용 툴팁 유틸리티 중심으로 유지.
- 분해 대상 책임은 `shell/presentation` 계층으로 이관 완료.
