# M13 Python 3D Scene Data Model

完成时间：2026-04-28 13:56（上海时间）

## Summary

Implemented a headless Python 3D scene data model under `tools/python`. It loads
V2 scene configs, OBJ mesh metadata, transforms, camera/lights, and direct VPL
`.f32` buffers without requiring GUI support.

## Changes

- Added `hypo_viz.scene3d`.
- Added tests for AreaVPL, Bunny, Dragon bounds-only metadata, VPL buffers, and
  missing mesh warnings.
- Added `docs/3d-visualization.md`.
- Updated `.pipeline/architecture.md` with M13 notes.

## Validation

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m unittest tools/python/tests/test_scene3d.py
PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests
```

Result: `22` Python tests passed.

## Notes

The mesh loader is intentionally metadata-first and streams OBJ files. Heavy
scenes can use `bounds_only` without storing vertices, which keeps the later
static/interactive visualization work from paying unnecessary memory cost.
