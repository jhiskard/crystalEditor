# Service Test Strategy (Phase 19)

- Date: `2026-04-22 (KST)`
- Decision scope: W3 feature service internalization

## Port Retention Criteria

1. Keep only when verified test-double channel is required.
2. Remove when sole purpose is legacy adapter indirection.

## W3 Decision

- `StructureServicePort`: remove legacy adapter path, keep no extra adapter.
- `MeasurementServicePort`: remove legacy adapter path, keep no extra adapter.
- `DensityServicePort`: remove legacy adapter path, keep no extra adapter.

## Rationale

- Current codebase had single concrete implementation paths only.
- Additional indirection did not provide active multi-implementation value.

