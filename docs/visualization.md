# 2D Visualization Tools

M11 adds uv-managed static 2D visualization helpers under `hypo_viz`.

## Scene Batch

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_viz.render_2d scene \
  --root build/validation_outputs_m8_standard/standard/area_vpl_smoke \
  --output-dir build/test_outputs/viz_area_vpl
```

This generates:

- heatmaps for `.f32` G-Buffer and shadow debug buffers
- `shadow_contact_sheet.png`
- `light_0_direct_vpls_annotated.png` when `light_0_direct_vpls.f32` and a
  shadow map are present

## Individual Commands

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_viz.render_2d heatmap \
  --input build/validation_outputs_m8/fast/fast_multi_light_shadow/gbuffer/linear_depth.f32 \
  --output build/test_outputs/viz/linear_depth_heatmap.png

PYTHONPATH=tools/python uv run --project tools/python python -m hypo_viz.render_2d vpl-overlay \
  --base build/validation_outputs_m8_standard/standard/area_vpl_smoke/shadow/ArealightShadowMap_0.png \
  --vpl build/validation_outputs_m8_standard/standard/area_vpl_smoke/shadow/light_0_direct_vpls.f32 \
  --output build/test_outputs/viz/light_0_direct_vpls_annotated.png

PYTHONPATH=tools/python uv run --project tools/python python -m hypo_viz.render_2d shadow-sheet \
  --shadow-dir build/validation_outputs_m8/fast/fast_multi_light_shadow/shadow \
  --output build/test_outputs/viz/point_directional_contact_sheet.png
```

Missing optional shadow or VPL inputs emit warnings and do not fail the entire
scene batch.
