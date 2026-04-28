# M11 uv-managed 2D Visualization Tools

Completed at: 2026-04-28 13:10（上海时间）

## Summary

M11 added static 2D visualization tools for validation outputs. The tools can
render `.f32` heatmaps, annotate VPL sample positions on shadow maps, create
shadow contact sheets, and batch-process a scene output directory.

## Changes

- Added `pillow` to the uv-managed Python tools project.
- Added `hypo_viz.render_2d`.
- Added `tools/python/tests/test_visualization.py`.
- Added CLI script entry `hypo-viz`.
- Added `docs/visualization.md`.
- Updated README with validation/baseline/visualization commands.
- Generated example PNGs under:
  - `build/test_outputs/viz_area_vpl/`
  - `build/test_outputs/viz_fast_shadow/`

## Verification

- `PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests` - 13 passed
- `python3 -m py_compile tools/python/hypo_viz/render_2d.py`
- `PYTHONPATH=tools/python uv run --project tools/python python -m hypo_viz.render_2d scene --root build/validation_outputs_m8_standard/standard/area_vpl_smoke --output-dir build/test_outputs/viz_area_vpl`
- `PYTHONPATH=tools/python uv run --project tools/python python -m hypo_viz.render_2d shadow-sheet --shadow-dir build/validation_outputs_m8/fast/fast_multi_light_shadow/shadow --output build/test_outputs/viz_fast_shadow/point_directional_contact_sheet.png`

## Notes

- Missing optional shadow/VPL inputs return warnings and do not fail scene batch
  processing.
- 3D or interactive visualization remains deferred.
