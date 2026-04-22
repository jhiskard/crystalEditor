# Phase 19 / W3 — Structure/Measurement/Density 포트 내재화

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 5.1 R1/R2, 5.2 권고(legacy 경유 경로 제거)
우선순위: **High**
예상 소요: 2 ~ 3일
선행 의존: W1, W2
후속 작업: W4, W5

---

## 1. 배경

Phase 10~12에서 도입된 `legacy_*_service_port`는 격리에는 유효했지만, Phase 19 목표(물리 삭제)에서는 제거 대상이다. W3의 목적은 feature 서비스가 legacy adapter를 경유하지 않고 도메인 Repository/Store를 직접 주입받는 구조로 전환하는 것이다.

## 2. 현재 상태

| Feature | 대상 파일 | 상태 |
|---|---|---|
| Structure | `structure/infrastructure/legacy/legacy_structure_service_port.{h,cpp}` | 제거 대상 |
| Measurement | `measurement/infrastructure/legacy/legacy_measurement_service_port.{h,cpp}` | 제거 대상 |
| Density | `density/infrastructure/legacy/legacy_density_service_port.{h,cpp}` | 제거 대상 |

공통 이슈:

- adapter 내부 `LegacyAtomsRuntime()` 경유
- 포트 유지/삭제 기준 문서화 미흡

## 3. 설계 원칙

### 3.1 포트 유지/삭제 기준 (v2 4.1 승계)

| 판정 | 기준 |
|---|---|
| 유지 | 테스트 더블 주입 수요 또는 복수 구현체 계획이 실제 존재 |
| 삭제 | legacy 구현체가 유일하고 대체 구현 계획 없음 |
| 기본값 | 삭제(YAGNI), 유지 시 근거를 문서화 |

결정은 다음 문서에 기록한다.

- `docs/refactoring/phase19/logs/service_test_strategy_phase19_latest.md`
- `docs/refactoring/phase19/logs/port_retention_decisions_W3_latest.md`

### 3.2 목표 구조

- `*_service.cpp`는 `*_repository`, `*_store`, `render_gateway` 등 현행 모듈 계약에 직접 의존
- `infrastructure/legacy/` 디렉터리 3개 제거

## 4. 실행 순서

1. W3.1 Structure: 포트 표면 인벤토리 후 `structure_service` 직접 주입 구조로 전환.
2. W3.2 Measurement: W2에서 확정한 `PickedAtomInfo` 경계를 유지한 채 포트 제거.
3. W3.3 Density: density 서비스의 포트 경유 경로를 저장소 직접 호출로 전환.
4. feature별 포트 유지/삭제 판정을 로그에 기록.
5. `legacy_*_service_port.{h,cpp}` 6개 파일 + 3개 legacy 폴더 삭제.
6. `wb_structure.cmake`, `wb_measurement.cmake`, `wb_density.cmake` 갱신.
7. W3 완료 리포트 작성.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/structure/infrastructure/legacy/**` | **삭제** | 포트/어댑터 제거 |
| `webassembly/src/measurement/infrastructure/legacy/**` | **삭제** | 포트/어댑터 제거 |
| `webassembly/src/density/infrastructure/legacy/**` | **삭제** | 포트/어댑터 제거 |
| `webassembly/src/structure/application/structure_service.{h,cpp}` | 수정 | 직접 주입 |
| `webassembly/src/measurement/application/measurement_service.{h,cpp}` | 수정 | 직접 주입 |
| `webassembly/src/density/application/density_service.{h,cpp}` | 수정 | 직접 주입 |
| `webassembly/src/workspace/runtime/workbench_runtime.cpp` | 수정 | 조립 경로 갱신 |
| `webassembly/cmake/modules/wb_structure.cmake` | 수정 | legacy 엔트리 제거 |
| `webassembly/cmake/modules/wb_measurement.cmake` | 수정 | legacy 엔트리 제거 |
| `webassembly/cmake/modules/wb_density.cmake` | 수정 | legacy 엔트리 제거 |
| `docs/refactoring/phase19/logs/service_test_strategy_phase19_latest.md` | **신규/갱신** | 포트 유지 근거 |
| `docs/refactoring/phase19/logs/port_retention_decisions_W3_latest.md` | **신규** | 판정 기록 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W3.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| 3개 feature의 `infrastructure/legacy/` 존재 | find | 없음 |
| `Legacy*ServicePort` 심볼 참조 | grep | 0 |
| 포트 유지/삭제 판정 문서 | 로그 체크 | 기록 완료 |
| feature별 회귀 테스트 | 단위/e2e | PASS |
| 빌드 | `build-wasm:debug` | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 포트 제거 후 테스트 주입 경로 상실 | 테스트 취약 | 유지 필요 포트는 근거 문서화 후 한정 유지 |
| 도메인 메서드 노출 부족 | 전환 지연 | W3 초반에 갭 분석 후 domain API 선확장 |
| W5 이전 과도기 브리지 잔존 | 구조 혼선 | 임시 shim 주석에 제거 목표(W5)와 추적 ID 표기 |

## 8. 산출물

- 삭제 파일 6개/삭제 폴더 3개
- service 직접 주입으로 바뀐 feature 서비스
- `service_test_strategy_phase19_latest.md`
- `port_retention_decisions_W3_latest.md`
- `dependency_gate_report_W3.md`

## 9. Git 커밋 템플릿

```
refactor(feature): phase19/W3 — internalize legacy service ports

- Remove legacy service adapters for structure/measurement/density
- Rewire feature services to repository/store direct injection
- Update runtime composition and cmake module sources
- Record port retention decisions and test strategy

metrics:
  legacy service port files: 6 -> 0
  feature legacy directories: 3 -> 0

verif:
  build-wasm:debug: PASS
  feature regressions: PASS
```

---

*W3 완료 시 feature 서비스는 legacy adapter 계층 없이 자체 도메인으로 귀속된다.*
