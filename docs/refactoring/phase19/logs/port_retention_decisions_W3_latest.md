# Port Retention Decisions — W3

- Date: `2026-04-22 (KST)`
- Status: `Approved`

## Decisions

| Area | Legacy adapter | Decision | Reason |
|---|---|---|---|
| Structure | `legacy_structure_service_port.*` | Remove | single adapter, no active alternate impl |
| Measurement | `legacy_measurement_service_port.*` | Remove | single adapter, no active alternate impl |
| Density | `legacy_density_service_port.*` | Remove | single adapter, no active alternate impl |

## Outcome

- All 3 feature `infrastructure/legacy` directories removed.
- Service implementations now call runtime path directly.

