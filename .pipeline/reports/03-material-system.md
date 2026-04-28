# Report: M4 Material System and Shading Correctness

## Summary

M4 extended the material layer with a common sampling contract, PBR parameters,
real texture dimensions, and material values in G-Buffer export.

## Step Status

| Step | Status | Evidence |
|---|---|---|
| write_tests | pass | Added `MaterialTests` for material sampling, PBR params, texture dimensions, and V2 material fields. |
| review_tests | pass | Tests cover the expected material contracts without requiring full PBR BRDF implementation. |
| run_tests_red | pass | `MaterialTests` failed before implementation because `MaterialSample` and `PBRMaterial` were missing. |
| implement | pass | Added material sample path, PBR material, texture dimension handling, roughness/metallic config parsing, and G-Buffer export. |
| run_tests_green | pass | All regression suites pass and V2 smoke render preserves material parameters. |
| review_code | pass | Changes stay in material/config/buffer boundaries and keep full PBR shading deferred. |

## Verification

- `xmake run BaselineTests`
  - Result: 6 passed, 0 failed
- `xmake run SceneCliOutputTests`
  - Result: 6 passed, 0 failed
- `xmake run GBufferTests`
  - Result: 4 passed, 0 failed
- `xmake run MaterialTests`
  - Result: 4 passed, 0 failed
- `xmake run HypoxRasterizer --config configs/SmokeTest.v2.json --output-root build/test_outputs/m4_render --buffers color,gbuffer,shadow --resolution 16x16`
  - Verified `roughness.f32` and `metallic.f32` are emitted.
  - Verified temporary legacy config contains `Roughness` and `Metallic`.

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

- PBR is currently a parameter/data path, not a full physically based shading
  implementation.
- Texture loading now stores actual image dimensions and clamps UV lookup.

## Next Action

Continue to M5: Directional/Point Lights and Cube Shadow Maps.
