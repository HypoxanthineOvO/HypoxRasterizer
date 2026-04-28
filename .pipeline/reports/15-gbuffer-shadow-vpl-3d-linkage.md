# M16 G-Buffer, Shadow, and VPL 3D Linkage

完成时间：2026-04-28 14:23（上海时间）

## Summary

Implemented a combined 2D/3D visualization report command that links renderer
outputs back to scene context.

## Changes

- Added `hypo_viz.report3d`.
- Added CLI script `hypo-viz-3d-report`.
- Added world-position point-cloud loading and preview PNG output.
- Added shadow face metadata with point cube face directions.
- Reused M11 2D visualization and M14 static 3D preview in one report command.
- Updated docs and architecture notes.

## Generated Example

- `build/test_outputs/3d_viz_report/area_vpl/report.md`
- `build/test_outputs/3d_viz_report/area_vpl/report.json`
- `build/test_outputs/3d_viz_report/area_vpl/scene3d.png`
- `build/test_outputs/3d_viz_report/area_vpl/world_position_point_cloud.png`

## Validation

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m unittest tools/python/tests/test_report3d.py
PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests
```

Result: `34` Python tests passed.
