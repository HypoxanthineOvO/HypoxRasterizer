# Report: M7 Validation Dataset Manifest and Runner Foundation

## Summary

M7 established the validation scene suite contract: a manifest format, tiered
scene selection, reference diagnostics, a repository seed manifest, and a
lightweight dry-run runner.

## Step Status

| Step | Status | Evidence |
|---|---|---|
| write_tests | pass | Added `ValidationSceneTests` for manifest parsing, proxy ports, tier/tag selection, default heavy exclusion, missing references, and repository manifest contract. |
| review_tests | pass | Tests cover the M7 manifest and runner-foundation expectations. |
| run_tests_red | pass | `ValidationSceneTests` failed before implementation because `validation_manifest.hpp` was missing. |
| implement | pass | Added `Utils/Validation`, `validation/scenes.yaml`, validation config directories, external asset placeholder, docs, and dry-run runner. |
| run_tests_green | pass | Full regression and runner dry-runs passed. |
| review_code | pass | Adjusted runner selection semantics so explicit `--tier heavy` can enumerate heavy scenes while default still excludes them. |

## Verification

- `xmake build -y HypoxRasterizer`
  - Result: pass
- `xmake run BaselineTests`
  - Result: 6 passed, 0 failed
- `xmake run SceneCliOutputTests`
  - Result: 6 passed, 0 failed
- `xmake run GBufferTests`
  - Result: 4 passed, 0 failed
- `xmake run MaterialTests`
  - Result: 4 passed, 0 failed
- `xmake run LightShadowTests`
  - Result: 4 passed, 0 failed
- `xmake run VPLAreaTests`
  - Result: 3 passed, 0 failed
- `xmake run ValidationSceneTests`
  - Result: 5 passed, 0 failed
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml`
  - Result: selected 3 default non-heavy scenes
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --all --include-heavy`
  - Result: selected 4 scenes including the heavy placeholder

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

- The M7 parser intentionally supports a small project-owned YAML subset. M9-M11
  can move richer tooling into the shared uv Python project.
- Heavy scenes are enumerable but excluded by default.

## Next Action

Continue to M8: Fast Corner Cases and Standard Internal Scenes.
