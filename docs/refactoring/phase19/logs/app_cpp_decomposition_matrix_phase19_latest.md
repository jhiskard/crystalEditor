# app.cpp Decomposition Matrix (Phase 19 Latest)

- Captured at: `2026-04-22 (KST)`
- File: `webassembly/src/app.cpp`

## Baseline vs Current

| Metric | Baseline | Current |
|---|---:|---:|
| LOC | 1,430 | 1,281 |
| `App::` method definitions | 34 (plan baseline) | 40 (code snapshot) |

## Ownership Targets for W7

| Responsibility | Target file(s) |
|---|---|
| Main menu render | `shell/presentation/main_menu.*` |
| Dock/layout handling | `shell/presentation/window_layout.*` |
| Window render dispatch | `shell/presentation/window_registry.*` |
| Popup rendering | `shell/presentation/popup_presenter.*` |
| Font/style controls | `shell/presentation/font/*`, `shell/presentation/theme/*` |

## W0 Note

- W0 only freezes decomposition targets.
- Actual `app.cpp <= 400` is W7 scope.

