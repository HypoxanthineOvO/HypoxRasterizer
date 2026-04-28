# Report: M6 VPL Area Lighting, Demo Scenes, and Quality Pass

## Summary

M6 added configurable Area light VPL sampling, VPL debug float output, and a
Cornell-box-style Area VPL demo scene. The final regression pass covers point,
directional, and area-light rendering through the unified CLI/output contract.

## Step Status

| Step | Status | Evidence |
|---|---|---|
| write_tests | pass | Added `VPLAreaTests` for VPL sample count, energy conservation, stochastic seed reproducibility, and V2 area VPL fields. |
| review_tests | pass | Tests cover the VPL sampling and schema contracts without large renderer rewrites. |
| run_tests_red | pass | `VPLAreaTests` failed before implementation because configurable Area VPL APIs and fields were missing. |
| implement | pass | Added configurable AreaLight VPL sampling, V2/legacy area fields, VPL debug `.f32` output, AreaVPL smoke config, and selected output-directory cleanup. |
| run_tests_green | pass | Full regression passed with six test targets and point/directional/area smoke renders. |
| review_code | pass | Changes stay within light/config/output/rasterizer boundaries and keep Cascade Shadow Maps deferred. |

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
- `xmake run HypoxRasterizer --config configs/SmokeTest.v2.json --output-root build/test_outputs/final_point --buffers color,gbuffer,shadow --resolution 16x16`
  - Verified six point cube shadow faces.
- `xmake run HypoxRasterizer --config configs/DirectionalSmoke.v2.json --output-root build/test_outputs/final_directional --buffers color,gbuffer,shadow --resolution 16x16`
  - Verified directional shadow output.
- `xmake run HypoxRasterizer --config configs/AreaVPLSmoke.v2.json --output-root build/test_outputs/final_area --buffers color,gbuffer,shadow --resolution 24x24`
  - Verified area shadow output and `light_0_direct_vpls.f32`.

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

- VPL debug buffers use the shared float-buffer format and store
  `[position.xyz, intensity.rgb]` records.
- Area VPL sampling supports deterministic grid placement and reproducible
  stochastic jitter.
- Cascade Shadow Maps remain a next-cycle decision; the current directional
  implementation intentionally stays single-map.

## Next Action

Planning cycle complete. Next cycle should focus on direct `SceneConfig`
rendering, better BRDF/shadow quality, optional Cascade Shadow Maps, and larger
demo scenes once performance is measured.
