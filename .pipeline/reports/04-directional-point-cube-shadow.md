# Report: M5 Directional/Point Lights and Cube Shadow Maps

## Summary

M5 made directional lights renderable through the existing CPU rasterizer path
and turned point-light cube shadow maps into structured six-face outputs.

## Step Status

| Step | Status | Evidence |
|---|---|---|
| write_tests | pass | Added `LightShadowTests` for point cube labels, directional normalization, V2 directional parsing, and shadow publication. |
| review_tests | pass | Tests cover the first required light/shadow contracts without pulling Cascade Shadow Maps into scope. |
| run_tests_red | pass | `LightShadowTests` failed before implementation because `DirectionalLight` and point cube face labels were missing. |
| implement | pass | Added `DirectionalLight`, directional legacy/V2 adapters, structured point cube shadow names, stale-output cleanup, and real `shadow_visibility` export. |
| run_tests_green | pass | Full regression passed, including point and directional smoke renders. |
| review_code | pass | Changes stay inside light/config/output/rasterizer boundaries; Cascade remains deferred. |

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
- `xmake run HypoxRasterizer --config configs/SmokeTest.v2.json --output-root build/test_outputs/m5_regression_point --buffers color,gbuffer,shadow --resolution 16x16`
  - Verified six point cube shadow face PNG files.
- `xmake run HypoxRasterizer --config configs/DirectionalSmoke.v2.json --output-root build/test_outputs/m5_regression_directional --buffers color,gbuffer,shadow --resolution 16x16`
  - Verified directional shadow PNG and float G-Buffer outputs.

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

- Directional shadows currently use one stable shadow map. Cascade Shadow Map is
  intentionally deferred.
- `ShadowMap` still owns a separate rasterization path. M5 clarified outputs and
  interfaces but did not fully deduplicate shadow rasterization with the main
  rasterizer.

## Next Action

Continue to M6: VPL Area Lighting, Demo Scenes, and Quality Pass.
