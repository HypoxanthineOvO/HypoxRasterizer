# M14 Static 3D Preview and Headless Screenshots

完成时间：2026-04-28 14:05（上海时间）

## Summary

Implemented headless static 3D preview rendering for validation scenes. The
first backend is a lightweight Pillow software projection that works without GUI
or GPU dependencies.

## Changes

- Added `hypo_viz.render_3d_static`.
- Added CLI script `hypo-viz-3d-static`.
- Added tests for AreaVPL static preview, Dragon bounds-only preview, missing
  mesh warnings, and unavailable backend errors.
- Updated `docs/3d-visualization.md` and architecture notes.

## Generated Examples

- `build/test_outputs/3d_viz_static/area_overview.png`
- `build/test_outputs/3d_viz_static/area_overview.json`
- `build/test_outputs/3d_viz_static/dragon_bounds.png`
- `build/test_outputs/3d_viz_static/dragon_bounds.json`

## Validation

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m unittest tools/python/tests/test_render_3d_static.py
PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests
```

Result: `26` Python tests passed.
