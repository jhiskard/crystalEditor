# Port Retention Decisions — W4

- Date: `2026-04-22 (KST)`

## Decisions

| Area | Target | Decision | Reason |
|---|---|---|---|
| IO runtime bridge | `legacy_import_runtime_port.*` | Remove legacy location | eliminate `io/infrastructure/legacy` path |
| IO runtime contract | `import_runtime_port.h` | Retain contract (temporary) | parser/apply/workflow layers still share this interface |
| Render viewer bridge | `legacy_viewer_facade.*` | Remove | runtime now accesses `VtkViewer::Instance()` directly |

## Verification

- `legacy_import_runtime_port` refs: `0`
- `legacy_viewer_facade`/`GetLegacyViewerFacade` refs: `0`

