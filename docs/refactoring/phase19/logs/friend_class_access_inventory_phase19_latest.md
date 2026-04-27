# Friend-Class Access Inventory (W1)

- Date: `2026-04-22 (KST)`
- Target: `FileLoader` ↔ `ImportPopupPresenter`
- Previous declaration: `friend class shell::presentation::ImportPopupPresenter;`

## Previous private state accessed by presenter

- structure replace popup state
- deferred structure file name
- structure import error title/message
- XSF grid warning text

## W1 refactor result

- Introduced `io::application::ImportPopupState`
- Added explicit `FileLoader` popup APIs:
  - `getImportPopupState()`
  - `acknowledgeStructureReplacePopup(bool)`
  - `dismissStructureImportErrorPopup()`
  - `dismissXsfGridCellWarningPopup()`
- Removed `friend class` declaration

## Verification

- `friend class` occurrences under `webassembly/src`: `0`

