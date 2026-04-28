# Report: M2 Scene Schema, CLI, and Output Contract

## Summary

M2 added the new external scene/CLI/output contract while preserving the legacy
renderer path.

## Step Status

| Step | Status | Evidence |
|---|---|---|
| write_tests | pass | Added `SceneCliOutputTests` and `configs/SmokeTest.v2.json`. |
| review_tests | pass | Tests cover CLI parsing, invalid resolution, V2 schema load/error, output layout, and buffer selection. |
| run_tests_red | pass | `SceneCliOutputTests` failed before implementation because `cli.hpp` was missing. |
| implement | pass | Added CLI parser, V2 scene loader, OutputManager, temporary V2 adapter, README and schema docs. |
| run_tests_green | pass | Baseline and M2 tests pass; V2 smoke render publishes the new output layout. |
| review_code | pass | Scope matches M2; temporary adapter is explicitly documented and isolated. |

## Verification

- `xmake build -y HypoxRasterizer`
- `xmake build -y BaselineTests`
- `xmake build -y SceneCliOutputTests`
- `xmake run BaselineTests`
  - Result: 6 passed, 0 failed
- `xmake run SceneCliOutputTests`
  - Result: 6 passed, 0 failed
- `xmake run HypoxRasterizer --config configs/SmokeTest.v2.json --output-root build/test_outputs/m2_render --buffers color,gbuffer,shadow --resolution 16x16`
  - Generated: `build/test_outputs/m2_render/smoke_v2/color.png`
  - Generated: `build/test_outputs/m2_render/smoke_v2/gbuffer/depth.png`
  - Generated: `build/test_outputs/m2_render/smoke_v2/gbuffer/normal.png`
  - Generated: point light shadow outputs under `build/test_outputs/m2_render/smoke_v2/shadow/`
  - Verified resolution override in `build/cli_overrides/SmokeTest.v2.legacy.json`

## Evaluation

- `tests_pass`: pass
- `no_regressions`: pass
- `matches_plan`: pass
- `code_quality`: pass
- `diff_score`: 1
- `code_quality`: 4
- `test_coverage`: 2
- `complexity`: 3
- `architecture_drift`: 3
- `overall`: 2

## Notes

- The V2-to-legacy render adapter is temporary. Later milestones should move the
  renderer to consume `SceneConfig` directly.
- `--buffers` now affects legacy output publishing and should remain part of the
  output contract.

## Next Action

Continue to M3: G-Buffer, Depth, and Float Output.
