# Phase 14 W0 Legacy Alias Inventory Baseline

- 측정 시각: `2026-04-06 (KST)`
- 범위: `webassembly/src/**/*.cpp`, `webassembly/src/**/*.h`
- 기준 계획서: `docs/refactoring/phase14/refactoring_phase14_state_ownership_alias_zero_260406.md`

## 1) legacy alias 총량 기준선

검색 패턴:
- `atoms::domain::createdAtoms`
- `atoms::domain::createdBonds`
- `atoms::domain::cellInfo`
- `using atoms::domain::createdAtoms|createdBonds|cellInfo`

총 참조 수: `14`

## 2) 파일별 분포

| 파일 | Count |
|---|---:|
| `webassembly/src/atoms/atoms_template.cpp` | 7 |
| `webassembly/src/atoms/application/structure_lifecycle_service.cpp` | 2 |
| `webassembly/src/atoms/infrastructure/file_io_manager.cpp` | 2 |
| `webassembly/src/atoms/ui/atom_editor_ui.cpp` | 2 |
| `webassembly/src/atoms/ui/bravais_lattice_ui.cpp` | 1 |

## 3) 경계 노출 점검

1. `atoms/*` 외부 legacy alias 노출: `0`
2. Phase 14 목표:
   - 외부 노출 `0` 유지
   - 내부 참조 `14 -> 0`

## 4) W0 메모

1. Phase 13 완료 시점 대비 alias 참조 제거 작업은 아직 미착수 상태다.
2. W1~W3에서 계층 순서(`domain -> application -> ui`)로 단계별 제거를 진행한다.

## 5) W1~W2 적용 후 재측정 (`2026-04-07`, KST)

적용 범위:
- `webassembly/src/atoms/domain/*` (점검/재확인)
- `webassembly/src/structure/domain/*` (점검/재확인)
- `webassembly/src/atoms/application/structure_lifecycle_service.cpp`
- `webassembly/src/structure/application/*` (점검/재확인)
- `webassembly/src/measurement/application/*` (점검/재확인)
- `webassembly/src/density/application/*` (점검/재확인)

재측정 결과:

| 항목 | W0 | W2 |
|---|---:|---:|
| legacy alias 전체 참조 수 | 14 | 12 |
| `atoms/domain + structure/domain` 범위 참조 수 | 0 | 0 |
| application 범위 참조 수 (`atoms/structure/measurement/density`) | 2 | 0 |
| `atoms/*` 외부 노출 | 0 | 0 |

잔여 참조 분포 (`12`):
1. `webassembly/src/atoms/atoms_template.cpp`: 7
2. `webassembly/src/atoms/infrastructure/file_io_manager.cpp`: 2
3. `webassembly/src/atoms/ui/atom_editor_ui.cpp`: 2
4. `webassembly/src/atoms/ui/bravais_lattice_ui.cpp`: 1

검증 메모:
1. `npm run test:cpp` PASS (`2026-04-07`, KST)
2. W3 대상으로 `atoms/ui`, `atoms_template`, `infrastructure` 잔여 alias 제거를 진행한다.

## 6) W3~W5 적용 후 재측정 (`2026-04-07`, KST)

적용 범위:
- `webassembly/src/atoms/ui/atom_editor_ui.cpp`
- `webassembly/src/atoms/ui/bravais_lattice_ui.cpp`
- `webassembly/src/atoms/infrastructure/file_io_manager.cpp`
- `webassembly/src/atoms/atoms_template.cpp`
- `scripts/refactoring/check_phase14_state_ownership_alias_zero.ps1`

재측정 결과:

| 항목 | W2 | W5 |
|---|---:|---:|
| legacy alias 전체 참조 수 | 12 | 0 |
| `atoms/domain + structure/domain` 범위 참조 수 | 0 | 0 |
| application 범위 참조 수 (`atoms/structure/measurement/density`) | 0 | 0 |
| `atoms/*` 외부 노출 | 0 | 0 |

게이트/검증 메모:
1. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase14_state_ownership_alias_zero.ps1`: PASS
2. 로그: `docs/refactoring/phase14/logs/check_phase14_state_ownership_alias_zero_latest.txt`
3. `npm run build-wasm:release`: PASS
4. `npm run test:cpp`: PASS
