# Report: M3 G-Buffer, Depth, and Float Output

## Summary

M3 introduced the first real float G-Buffer path and fixed depth preview
normalization edge cases.

## Step Status

| Step | Status | Evidence |
|---|---|---|
| write_tests | pass | Added `GBufferTests` for sample storage, metadata, depth preview, and float writer. |
| review_tests | pass | Tests cover the core M3 data contract without overfitting to rasterizer internals. |
| run_tests_red | pass | `GBufferTests` failed before implementation because the buffer writer interface was missing. |
| implement | pass | Added `GBuffer`, `FloatBufferWriter`, view-space interpolation preservation, and `Rasterizer::ExportGBuffer`. |
| run_tests_green | pass | GBuffer tests pass and V2 smoke render emits `.f32` G-Buffer outputs. |
| review_code | pass | Structure is isolated enough for M4/M5 to extend material and shadow semantics. |

## Verification

- `xmake build -y HypoxRasterizer`
- `xmake build -y GBufferTests`
- `xmake run GBufferTests`
  - Result: 4 passed, 0 failed
- `xmake run HypoxRasterizer --config configs/SmokeTest.v2.json --output-root build/test_outputs/m3_render --buffers color,gbuffer,shadow --resolution 16x16`
  - Generated G-Buffer float files including `world_position.f32`, `view_position.f32`, `linear_depth.f32`, `world_normal.f32`, `view_normal.f32`, `albedo.f32`, `material_id.f32`, `roughness.f32`, `metallic.f32`, and `shadow_visibility.f32`
  - Generated `linear_depth_preview.png`
  - Float header verified as `HYPORASTER_F32_V1`

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

- `material_id`, `roughness`, `metallic`, and `shadow_visibility` are currently
  structural placeholders or defaults. M4 and M5 are expected to connect them to
  real material and shadow semantics.
- The `.f32` format is simple by design and leaves room for later fixed-point
  quantization metadata and writers.

## Next Action

Continue to M4: Material System and Shading Correctness.
