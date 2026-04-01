# Phase 8 AtomsTemplate 기준선 인벤토리

기록일: `2026-04-01 (KST)`  
목적: Phase 8 착수 시점의 구조/결합도 기준선 고정

## 1) 파일 규모

| 파일 | 라인 수 |
|---|---:|
| `webassembly/src/atoms/atoms_template.cpp` | 5161 |
| `webassembly/src/atoms/atoms_template.h` | 1285 |
| `webassembly/src/vtk_viewer.cpp` | 1995 |
| `webassembly/src/file_loader.cpp` | 443 |
| `webassembly/src/app.cpp` | 1073 |
| `webassembly/src/platform/wasm/workbench_bindings.cpp` | 95 |
| `webassembly/src/shell/runtime/workbench_runtime.cpp` | 130 |

## 2) `AtomsTemplate` 결합도

| 항목 (`webassembly/src/atoms/atoms_template.cpp`) | 카운트 |
|---|---:|
| 전체 `::Instance()` 호출 | 103 |
| `VtkViewer::Instance()` | 75 |
| `MeshManager::Instance()` | 16 |
| `ModelTree::Instance()` | 2 |
| `MeshDetail::Instance()` | 3 |
| `AtomsTemplate::Instance()` | 5 |

## 3) 헤더 구조 지표

| 항목 (`webassembly/src/atoms/atoms_template.h`) | 카운트 |
|---|---:|
| `friend class` | 9 |
| `DECLARE_SINGLETON` | 1 |

## 4) 분해 타깃 디렉터리 존재 여부

| 경로 | 존재 여부 |
|---|---|
| `webassembly/src/structure` | False |
| `webassembly/src/measurement` | False |
| `webassembly/src/density` | False |

## 5) legacy alias 노출 압력 (`webassembly/src/atoms` 트리)

| 토큰 | 카운트 |
|---|---:|
| `createdAtoms` | 117 |
| `createdBonds` | 47 |
| `cellInfo` | 250 |

## 6) atoms 하위 모듈 볼륨

| 경로 | 파일 수 | 라인 수 |
|---|---:|---:|
| `webassembly/src/atoms/application` | 5 | 778 |
| `webassembly/src/atoms/domain` | 26 | 5770 |
| `webassembly/src/atoms/infrastructure` | 16 | 5353 |
| `webassembly/src/atoms/presentation` | 4 | 817 |
| `webassembly/src/atoms/ui` | 17 | 5095 |

## 7) 비고

- Phase 8에서는 위 기준선을 기반으로 `AtomsTemplate`를 compatibility facade 수준으로 축소한다.
- Phase 7 회귀 수정(폰트/아이콘 초기화 순서, VASP->XSF grid bootstrap)은 Phase 8 전 구간에서 보호 규칙으로 유지한다.
