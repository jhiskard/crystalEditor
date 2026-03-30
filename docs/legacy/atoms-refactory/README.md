# Legacy Refactory 문서 정리 계획 (Phase 0)

## 1. 목적
- `webassembly/src` 내 리팩토링 계획/메모 문서(`refactory*.md`)를 소스 코드와 분리해 관리한다.
- Phase 1부터 코드 탐색 노이즈를 줄이고, 문서 이력은 보존한다.

## 2. 이동 대상(확정)
- `webassembly/src/refactory.md`
- `webassembly/src/refactory3.md`
- `webassembly/src/refactory4.md`
- `webassembly/src/refactory5.md`
- `webassembly/src/refactory_atom_manager.md`
- `webassembly/src/refactory_details.md`
- `webassembly/src/refactory_details2.md`

## 3. 보존 정책
- 파일 삭제 금지, `git mv`로 경로만 변경
- 원본 내용 수정은 최소화(경로 링크 정리만 허용)
- 추후 사용 중단 시에도 `docs/legacy/` 하위 보관 유지

## 4. 실행 시점
- Phase 0: 계획 확정만 수행
- 실제 이동: Phase 1 착수 직전 또는 Phase 1 초기 커밋에서 실행

## 5. 권장 실행 커맨드(Phase 1 시점)
```powershell
New-Item -ItemType Directory -Force -Path docs/legacy/atoms-refactory | Out-Null
git mv webassembly/src/refactory*.md docs/legacy/atoms-refactory/
```
