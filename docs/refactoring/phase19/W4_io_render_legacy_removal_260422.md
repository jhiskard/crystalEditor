# Phase 19 / W4 — IO/Render legacy 제거

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 5.1 R2 (`legacy_viewer_facade`), legacy import runtime bridge 잔존
우선순위: **High**
예상 소요: 1 ~ 2일
선행 의존: W3
후속 작업: W5

---

## 1. 배경

W3 완료 이후에도 IO/Render에는 legacy bridge 파일이 남는다.

- `io/infrastructure/legacy/legacy_import_runtime_port.*`
- `render/application/legacy_viewer_facade.*`

W4는 이 브리지들을 제거하고 `WorkbenchRuntime::viewer()` 및 도메인 저장소 직접 주입 경로로 치환한다.

## 2. 현재 상태

| 대상 | 역할 | 상태 |
|---|---|---|
| `legacy_import_runtime_port.*` | import 서비스에서 runtime/legacy 연결 | 제거 대상 |
| `legacy_viewer_facade.*` | render에서 viewer 접근 브리지 | 제거 대상 |

## 3. 설계

### 3.1 W4.1 IO 경로

- `ImportRuntimePort` 경유를 제거하고 `StructureRepository`, `DensityRepository` 직접 주입.
- `workbench_runtime.cpp` 조립 코드에서 `LegacyImportRuntimePort` 생성/주입 삭제.

### 3.2 W4.2 Render 경로

- `GetLegacyViewerFacade(...)` 호출 제거.
- `VtkRenderGateway`가 `VtkViewer&`를 직접 주입받도록 정리.

## 4. 실행 순서

1. `ImportRuntimePort` 표면 인벤토리 작성.
2. io import 서비스 시그니처를 repository 직접 주입으로 변경.
3. `io/infrastructure/legacy/` 디렉터리 삭제.
4. `render/application/legacy_viewer_facade.*` 호출자를 모두 `VtkViewer&` 직접 주입으로 전환.
5. `legacy_viewer_facade.*` 2파일 삭제.
6. `wb_io.cmake`, `wb_render.cmake`, `workbench_runtime.cpp` 갱신.
7. Import/Render 회귀 테스트 수행.
8. W4 완료 리포트 작성.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.h` | **삭제** | |
| `webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.cpp` | **삭제** | |
| `webassembly/src/io/infrastructure/legacy/` | **폴더 삭제** | |
| `webassembly/src/io/application/import_*_service.{h,cpp}` | 수정 | 직접 주입 |
| `webassembly/src/render/application/legacy_viewer_facade.h` | **삭제** | |
| `webassembly/src/render/application/legacy_viewer_facade.cpp` | **삭제** | |
| `webassembly/src/render/infrastructure/vtk_render_gateway.h` | 수정 | ctor 시그니처 |
| `webassembly/src/render/infrastructure/vtk_render_gateway.cpp` | 수정 | facade 호출 제거 |
| `webassembly/src/workspace/runtime/workbench_runtime.cpp` | 수정 | 조립 경로 변경 |
| `webassembly/cmake/modules/wb_io.cmake` | 수정 | 소스 정리 |
| `webassembly/cmake/modules/wb_render.cmake` | 수정 | 소스 정리 |
| `docs/refactoring/phase19/logs/import_runtime_port_surface_phase19_latest.md` | **신규** | 인벤토리 |
| `docs/refactoring/phase19/logs/port_retention_decisions_W4_latest.md` | **신규** | 판정 기록 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W4.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `io/infrastructure/legacy/` 존재 | find | 없음 |
| `legacy_viewer_facade.*` 존재 | find | 없음 |
| `GetLegacyViewerFacade` 심볼 | grep | 0 |
| `LegacyImportRuntimePort` 심볼 | grep | 0 |
| Import/Render 회귀 | e2e/수동 | PASS |
| 빌드 | `build-wasm:debug` | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| import 서비스 의존성 확장으로 생성자 연쇄 변경 | 빌드 실패 | W4에서 runtime 조립 파일 동시 갱신 |
| viewer 주입 변경 중 순환 의존 발생 | 구조 복잡도 상승 | 필요 시 render 인터페이스를 분리해 역의존 차단 |
| W5와 경로 충돌 | 병합 충돌 | W4 완료 후 W5 착수 순서 고정 |

## 8. 산출물

- 삭제 파일 4개 + 삭제 폴더 1개
- io/render 조립 갱신 코드
- `import_runtime_port_surface_phase19_latest.md`
- `port_retention_decisions_W4_latest.md`
- `dependency_gate_report_W4.md`

## 9. Git 커밋 템플릿

```
refactor(io,render): phase19/W4 — remove remaining io/render legacy bridges

- Remove io legacy_import_runtime_port and direct-inject repositories
- Remove render legacy_viewer_facade and inject VtkViewer directly
- Update runtime composition and cmake modules

metrics:
  io legacy bridge files: 2 -> 0
  render legacy facade files: 2 -> 0

verif:
  build-wasm:debug: PASS
  import/render regressions: PASS
```

---

*W4 완료 시 Phase 19의 legacy bridge는 `workspace/legacy` 중심으로 축소되며 W5 해체 준비가 완료된다.*
