# Phase 19 / W6 — Runtime Singleton 제거

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 5.2 권고 + v2 DoD(`DECLARE_SINGLETON`/`::Instance()` 0)
우선순위: **Low (필수 봉인 항목)**
예상 소요: 2 ~ 3일
선행 의존: W5
후속 작업: W7, W8, W9

---

## 1. 배경

Phase 19 종료 조건에는 singleton 잔존 0이 포함된다. 특히 `VtkViewer`, `MeshManager`, `FontManager`의 매크로 기반 전역 접근은 composition root 원칙과 충돌한다. W6의 목적은 전역 접근을 모두 주입 기반으로 전환하는 것이다.

## 2. 현재 상태

| 항목 | 기준선 |
|---|---:|
| `DECLARE_SINGLETON` 사용(정의 포함) | 4 |
| `::Instance()` 호출 | 48 |
| 핵심 대상 | `VtkViewer`, `MeshManager`, `FontManager` |

## 3. 설계

### 3.1 전환 원칙

- 소유: `WorkbenchRuntime` 단일 소유
- 접근: 생성자 주입/참조 전달
- 금지: static singleton accessor

### 3.2 매크로 처리

`macro/singleton_macro.h`는 삭제를 기본으로 하고, 부득이하면 deprecated 상태로 봉인하되 사용 0을 강제한다.

## 4. 실행 순서

1. singleton 호출자 인벤토리 작성.
2. `VtkViewer`의 singleton 매크로 제거 및 호출부 참조 주입 전환.
3. `MeshManager` singleton 제거 및 mesh 서비스 생성자 주입 전환.
4. `FontManager` singleton 제거 및 runtime 조립 순서 확정.
5. `singleton_macro.h` 삭제 또는 deprecated 봉인.
6. `check_phase19_singleton_zero.ps1` 추가.
7. W6 완료 로그 작성.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/render/presentation/viewer_window.{h,cpp}` | 수정 | singleton 제거 |
| `webassembly/src/mesh/domain/mesh_repository_core.{h,cpp}` | 수정 | singleton 제거 |
| `webassembly/src/mesh/application/mesh_*_service.{h,cpp}` | 수정 | 주입 기반 전환 |
| `webassembly/src/shell/presentation/font/font_registry.{h,cpp}` | 수정 | singleton 제거 |
| `webassembly/src/workspace/runtime/workbench_runtime.{h,cpp}` | 수정 | 소유/조립 확정 |
| `webassembly/src/macro/singleton_macro.h` | 삭제/수정 | 최종 정책 반영 |
| `scripts/refactoring/check_phase19_singleton_zero.ps1` | **신규** | 게이트 |
| `package.json` | 수정 | `check:phase19:singleton-zero` 추가 |
| `docs/refactoring/phase19/logs/singleton_call_sites_phase19_latest.md` | **신규** | 인벤토리 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W6.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `DECLARE_SINGLETON` 사용 | grep | 0 |
| `::Instance()` 호출 | grep | 0 |
| `check_phase19_singleton_zero.ps1` | 스크립트 실행 | PASS |
| 초기화 순서 회귀(Font/Render/Mesh) | 수동/e2e | PASS |
| 빌드 | `build-wasm:debug` | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| FontManager 초기화 타이밍 문제 | 폰트 렌더 오류 | ImGui context 생성 직후 생성 순서 고정 |
| 호출부 누락 | 런타임 실패 | 인벤토리 기반 전수 치환 후 grep 재검증 |
| W7과 app.cpp 변경 충돌 | 병합 비용 증가 | W6 선완료 후 W7 진행 또는 파일 소유 분리 |

## 8. 산출물

- singleton 제거 코드 변경
- `check_phase19_singleton_zero.ps1`
- `singleton_call_sites_phase19_latest.md`
- `dependency_gate_report_W6.md`

## 9. Git 커밋 템플릿

```
refactor(runtime): phase19/W6 — eliminate remaining singleton accessors

- Remove singleton macros from VtkViewer/MeshManager/FontManager
- Replace Instance() calls with runtime-owned dependency injection
- Add singleton-zero gate script

metrics:
  DECLARE_SINGLETON uses: 4 -> 0
  ::Instance() calls: 48 -> 0

verif:
  check:phase19:singleton-zero: PASS
```

---

*W6 완료 시 전역 singleton 접근 경로는 코드베이스에서 소멸한다.*
