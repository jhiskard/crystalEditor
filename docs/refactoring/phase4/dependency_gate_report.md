# Phase 4 Dependency Gate Report

Date: `2026-03-30 (KST)`  
Plan: `docs/refactoring/phase4/refactoring_phase4_ui_renderer_split_260330.md`  
Branch: `refactor/phase4-ui-render-split`

Evidence logs:
- Static split gate: `docs/refactoring/phase4/logs/dependency_check_phase4_latest.txt`
- Boundary gate: `docs/refactoring/phase4/logs/boundary_check_phase4_latest.txt`
- Build gate: `docs/refactoring/phase4/logs/build_phase4_gate_latest.txt`
- Manual smoke: `docs/refactoring/phase4/logs/manual_smoke_phase4_latest.md`

## 1. Hard Gate Results (Must Pass)
| Item | Current | Target | Result |
|---|---:|---:|---|
| `P4.required_files_exist` | 0 missing | 0 | PASS |
| `P4.atoms_moved_defs_removed_from_origin` | 0 | 0 | PASS |
| `P4.atoms_moved_defs_present_in_new_files` | 20 | 20 | PASS |
| `P4.charge_moved_defs_removed_from_origin` | 0 | 0 | PASS |
| `P4.charge_moved_defs_present_in_new_files` | 6 | 6 | PASS |
| `P4.model_tree_moved_defs_removed_from_origin` | 0 | 0 | PASS |
| `P4.model_tree_moved_defs_present_in_new_files` | 5 | 5 | PASS |
| `P4.viewer_moved_defs_removed_from_origin` | 0 | 0 | PASS |
| `P4.viewer_moved_defs_present_in_new_files` | 3 | 3 | PASS |
| `P4.cmake_wiring_entries` | 12 | 12 | PASS |
| `R1~R6 boundary checks` | 0 violations | 0 | PASS |

## 2. Advisory Metrics (Recommended)
| Item | Baseline | Target | Current | Result |
|---|---:|---:|---:|---|
| `atoms_template.cpp` lines | 6,130 | <= 4,291 | 5,913 | FAIL (Advisory) |
| `model_tree.cpp` lines | 1,782 | <= 1,336 | 42 | PASS (Advisory) |
| `charge_density_ui.cpp` lines | 2,260 | <= 1,695 | 1,764 | FAIL (Advisory) |
| `vtk_viewer.cpp` lines | 2,449 | <= 1,959 | 2,190 | FAIL (Advisory) |

## 3. Deferred Runtime Gates (By Current Directive)
- Release build verification: `SKIPPED`
  - Reason: current execution directive excludes compile validation in this run.
- Manual smoke verification: `PENDING`
  - Reason: this run is code/doc/static-gate only.

## 4. Summary
- Phase 4 hard static gates are `PASS`.
- Runtime gates (build/manual smoke) are not executed in this run by directive.
- Phase close judgment is now possible with explicit deferred items documented.
