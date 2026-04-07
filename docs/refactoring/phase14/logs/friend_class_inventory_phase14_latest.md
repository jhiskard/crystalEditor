# Phase 14 W0 Friend Class Inventory Baseline

- 측정 시각: `2026-04-06 (KST)`
- 범위: `webassembly/src/**/*.h`
- 기준 계획서: `docs/refactoring/phase14/refactoring_phase14_state_ownership_alias_zero_260406.md`

## 1) `friend class` 총량 기준선

총 선언 수: `4`

파일별 분포:

| 파일 | Count | 메모 |
|---|---:|---|
| `webassembly/src/app.h` | 1 | `friend class WorkbenchRuntime` |
| `webassembly/src/file_loader.h` | 1 | `friend class WorkbenchRuntime` |
| `webassembly/src/toolbar.h` | 1 | `friend class WorkbenchRuntime` |
| `webassembly/src/lcrs_tree.h` | 1 | legacy 구조 의존 |

## 2) Phase 14 목표

1. `friend class` 선언 전체 `4 -> 0`
2. `lcrs_tree.h`의 잔존 `friend` 선언 우선 제거
3. runtime 소유 객체(`app/file_loader/toolbar`)의 friend 의존은 대체 생성/소유 경로로 치환

## 3) W0 메모

1. Phase 13에서 runtime 소유권 전환 과정으로 `friend class WorkbenchRuntime`가 도입되었다.
2. Phase 14 W4에서 friend 제거 시 생성자 접근/소유권 경계를 함께 재설계해야 한다.

## 4) W4 적용 후 재측정 (`2026-04-07`, KST)

적용 범위:
- `webassembly/src/app.h`
- `webassembly/src/file_loader.h`
- `webassembly/src/toolbar.h`
- `webassembly/src/lcrs_tree.h`
- `webassembly/src/lcrs_tree.cpp`

재측정 결과:

| 항목 | W0 | W4 |
|---|---:|---:|
| `friend class` 선언 수 | 4 | 0 |
| `lcrs_tree.h` 내 `friend class` | 1 | 0 |

변경 메모:
1. `App/FileLoader/Toolbar`는 `friend class WorkbenchRuntime` 제거 후 생성자/소멸자를 public으로 전환.
2. `TreeNode`는 접근자/설정자(`SetParentMutable`, `SetLeftChildMutable`, `SetRightSiblingMutable`, `SetLeftSiblingMutable`) 추가 후 `LcrsTree` 조작 코드를 접근자 기반으로 치환.
