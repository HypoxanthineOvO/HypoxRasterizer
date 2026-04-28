# Report: M1 Build, Test, and Baseline Recovery

## Summary

M1 restored a repeatable baseline for build, tests, and smoke rendering.

## Step Status

| Step | Status | Evidence |
|---|---|---|
| write_tests | pass | Added `Tests/BaselineTests.cpp`, `Tests/TestUtils.hpp`, and `configs/SmokeTest.json`. |
| review_tests | pass | Coverage matches M1 expectations for geometry, camera, image IO, config parsing, config failure, OBJ load, and smoke render setup. |
| run_tests_red | pass | `xmake run BaselineTests` failed before implementation because the target did not exist. |
| implement | pass | Added `BaselineTests` target, vendored header-only deps, README commands, and path-stable tests. |
| run_tests_green | pass | Build, baseline tests, and smoke render passed. |
| review_code | pass | Scope matches M1; architecture drift is limited to test/build baseline and vendored header-only dependencies. |

## Verification

- `xmake build -y HypoxRasterizer`
- `xmake build -y BaselineTests`
- `xmake run BaselineTests`
  - Result: 6 passed, 0 failed
- `xmake run HypoxRasterizer configs/SmokeTest.json`
  - Generated: `color.png`, `depth.png`, `normal.png`
  - Generated: six `PointlightShadowMap...png` files

## Evaluation

- `tests_pass`: pass
- `no_regressions`: pass
- `matches_plan`: pass
- `code_quality`: pass
- `diff_score`: 1
- `code_quality`: 4
- `test_coverage`: 2
- `complexity`: 2
- `architecture_drift`: 2
- `overall`: 2

## Notes

- `stb` and `nlohmann_json` are now vendored under `third_party/include` because online xmake package fetches failed repeatedly in this environment.
- The main renderer architecture was not refactored in this milestone.

## Next Action

Continue to M2: Scene Schema, CLI, and Output Contract.
