# M15 Interactive 3D Viewer

完成时间：2026-04-28 14:13（上海时间）

## Summary

Implemented the experimental 3D viewer surface as reproducible viewer state plus
a portable HTML inspection shell. Headless requests for an unavailable GUI
backend fall back to the static 3D preview path when requested.

## Changes

- Added `hypo_viz.viewer3d`.
- Added CLI script `hypo-viz-3d`.
- Added tests for viewer state, HTML output, static fallback, and unavailable
  backend errors.
- Updated `docs/3d-visualization.md` and architecture notes.

## Generated Examples

- `build/test_outputs/3d_viz_viewer/area_viewer.html`
- `build/test_outputs/3d_viz_viewer/area_viewer.json`
- `build/test_outputs/3d_viz_viewer/area_fallback.png`
- `build/test_outputs/3d_viz_viewer/area_fallback.json`

## Validation

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m unittest tools/python/tests/test_viewer3d.py
PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests
```

Result: `30` Python tests passed.
