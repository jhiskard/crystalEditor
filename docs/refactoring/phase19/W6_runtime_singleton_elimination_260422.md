# Phase 19 / W6 — Runtime Singleton 제거

작성일: `2026-04-22 (KST)`
최종 업데이트: `2026-04-27 (KST, W5.10 반영)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 5.2 권고 + v2 DoD(핵심 runtime singleton 경로 제거) + W5.10 심볼 제로화 결과
우선순위: **Low (필수 봉인 항목)**
예상 소요: 2 ~ 3일
선행 의존: W5
후속 작업: W7, W8, W9

---

## 1. 배경

Phase 19 종료 조건에는 runtime 핵심 경로의 singleton 봉인이 포함된다. W5.10에서 `webassembly/src` 기준 `::Instance()`/`Instance()` 심볼은 0으로 정리되었다. 따라서 W6는 기존의 "대상 3종 `Instance()` 제거" 단계에서, **runtime 핵심 대상(`VtkViewer`, `MeshManager`, `FontManager`)의 singleton 진입점 0 유지 + 재유입 차단 게이트 정합화** 단계로 업데이트한다.

## 2. 현재 상태

| 항목 | 기준선 |
|---|---:|
| `DECLARE_SINGLETON` 사용(정의 포함) | 1 (`macro/singleton_macro.h` 정의 1건) |
| `DECLARE_SINGLETON` 클래스 선언 사용(매크로 정의 제외) | 0 |
| `::Instance()` 호출(전체) | 0 |
| `Instance()` 심볼(`webassembly/src`) | 0 |
| `::Shared()` 호출/정의(전체) | 43 |
| 대상 클래스 직접 정적 singleton 진입(`VtkViewer/MeshManager/FontManager`) | 0 |
| 핵심 대상 | `VtkViewer`, `MeshManager`, `FontManager` |

## 3. 설계

### 3.1 전환 원칙

- 소유: `VtkViewer`/`FontManager`는 `WorkbenchRuntime` 조립 경로에서 소유
- 접근: 생성자 기반 정적 조립(static local composition root) + 참조 전달
- 금지: 대상 3개 클래스의 정적 singleton 진입(`Class::Instance()`/동등 정적 진입점)

### 3.2 매크로 처리

`macro/singleton_macro.h`는 즉시 삭제 대신 **호환용 보존**을 기본으로 하되, `webassembly/src`의 production class 선언에서 사용 0을 강제한다.

### 3.3 게이트 정합화

- `check_phase19_singleton_zero.ps1`는 대상 3종 검사에서 **전역 `Instance()` 심볼 0 검사**로 정합화한다.
- `DECLARE_SINGLETON`는 production 소스에서 0, 매크로 정의 파일만 허용한다.
- `Shared()` 기반 정적 접근자는 W6에서 인벤토리로 추적하고, 후속 WBS에서 제거 대상으로 관리한다.

## 4. 실행 순서

1. W5.10 완료 기준선 동결(`::Instance()` 0, `Instance()` 심볼 0, `::Shared()` 인벤토리 작성).
2. `VtkViewer`/`MeshManager`/`FontManager`의 정적 singleton 진입점 부재를 재검증.
3. `WorkbenchRuntime`/repository/presentation 경계에서 runtime 소유 경로를 점검.
4. `check_phase19_singleton_zero.ps1`를 전역 `Instance()` 심볼 0 기준으로 정합화.
5. `build-wasm:release` 및 회귀 게이트 재실행.
6. W6 로그(`singleton_call_sites_phase19_latest.md`, `dependency_gate_report_W6.md`)를 W5.10 반영 상태로 갱신.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/render/presentation/viewer_window.{h,cpp}` | 수정 | singleton 제거 |
| `webassembly/src/render/infrastructure/vtk_render_gateway.cpp` | 수정 | runtime viewer 경유 |
| `webassembly/src/mesh/domain/mesh_repository_core.{h,cpp}` | 수정 | singleton 제거 |
| `webassembly/src/mesh/domain/mesh_repository.cpp` | 수정 | manager 조립 경계 전환 |
| `webassembly/src/shell/presentation/font/font_registry.{h,cpp}` | 수정 | singleton 제거 |
| `webassembly/src/shell/runtime/workbench_runtime.{h,cpp}` | 수정 | 소유/조립 확정 |
| `webassembly/src/macro/singleton_macro.h` | 유지 | production 사용 0 |
| `scripts/refactoring/check_phase19_singleton_zero.ps1` | **신규/갱신** | 전역 `Instance()` 심볼 0 게이트 |
| `package.json` | 수정 | `check:phase19:singleton-zero` 추가 |
| `docs/refactoring/phase19/logs/singleton_call_sites_phase19_latest.md` | **신규** | 인벤토리 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W6.md` | **신규** | 완료 리포트 |
| `docs/refactoring/phase19/logs/singleton_decomposition_progress_W5_10.md` | **참조** | W5.10 인수인계 기준선 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `DECLARE_SINGLETON` 클래스 선언 사용(매크로 정의 제외) | grep | 0 |
| `::Instance()` 호출(`webassembly/src`) | rg | 0 |
| `Instance()` 심볼(`webassembly/src`) | rg | 0 |
| `VtkViewer`/`MeshManager`/`FontManager` 정적 singleton 진입점 | rg | 0 |
| `check_phase19_singleton_zero.ps1` | 스크립트 실행 | PASS |
| 초기화 순서 회귀(Font/Render/Mesh) | 수동/e2e | PASS |
| 빌드 | `build-wasm:release` | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| FontManager 초기화 타이밍 문제 | 폰트 렌더 오류 | ImGui context 생성 직후 생성 순서 고정 |
| 정적 접근자(`Shared`) 잔존 확대 | 구조적 singleton 재유입 | 인벤토리 기준선 고정 + 후속 WBS 제거 계획 연동 |
| 호출부 누락 | 런타임 실패 | 인벤토리 기반 전수 치환 후 grep 재검증 |
| W7과 app.cpp 변경 충돌 | 병합 비용 증가 | W6 선완료 후 W7 진행 또는 파일 소유 분리 |

## 8. 산출물

- singleton 제거 코드 변경
- `check_phase19_singleton_zero.ps1`
- `singleton_call_sites_phase19_latest.md`
- `dependency_gate_report_W6.md`
- W5.10 인수인계 기준선 반영 내역

## 9. Git 커밋 템플릿

```
refactor(runtime): phase19/W6 — seal runtime singleton entrypoints after W5.10 global zeroization

- Reconcile W6 scope with W5.10 global Instance-zero state
- Keep runtime ownership for VtkViewer/MeshManager/FontManager without singleton entrypoints
- Update singleton-zero gate to global Instance() symbol checks

metrics:
  DECLARE_SINGLETON production class uses: 0
  Instance() symbols in webassembly/src: 0
  target runtime singleton entrypoints: 0

verif:
  check:phase19:singleton-zero: PASS
```

---

*W6 완료 시 runtime 핵심 대상(`VtkViewer`, `MeshManager`, `FontManager`)의 singleton 진입점은 0으로 봉인되고, `Instance()` 재유입은 게이트로 차단된다.*
