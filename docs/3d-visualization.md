# Python 3D Visualization

The 3D visualization tools live in the uv-managed `tools/python` project. The
first layer is a headless scene data model that parses V2 scene configs and
renderer debug buffers without opening a GUI.

## Scene Data Model

`hypo_viz.scene3d.load_scene3d()` loads:

- scene name and config path
- camera resolution, position, target, FOV, and focal length
- materials by id
- objects with mesh path, material id, transform, OBJ vertex/face counts, local
  bounds, and transformed world bounds
- point, directional, and area light metadata
- direct VPL `.f32` buffers from an optional renderer output directory
- structured warnings for missing mesh assets

Example:

```sh
PYTHONPATH=tools/python uv run --project tools/python python - <<'PY'
from hypo_viz.scene3d import load_scene3d

scene = load_scene3d(
    "configs/AreaVPLSmoke.v2.json",
    output_root="build/highres_validation_20260428/standard/area_vpl_smoke",
)
print(scene.name, len(scene.objects), len(scene.lights), len(scene.vpl_buffers))
PY
```

## Mesh Modes

The default `metadata_only` mode streams OBJ files and records counts/bounds
without storing vertex arrays. `bounds_only` is an explicit alias for heavy
validation scenes such as Dragon quick. `full` keeps vertices and should only be
used by tools that actually need them.

## VPL Buffers

Direct VPL buffers are read from:

```text
<scene-output>/shadow/light_<index>_direct_vpls.f32
```

The first three channels are interpreted as sample position. Channels 4-6 are
interpreted as sample normal when present, and channels 7-9 as flux when present.

IndirectVPL buffers planned for the next renderer milestone will use separate
names such as `light_<index>_indirect_vpls.f32`.

## Static 3D Preview

`hypo_viz.render_3d_static` generates headless PNG previews using the current
software projection backend. It draws object bounds, camera direction/frustum,
light glyphs, area light rectangles, and direct VPL sample points when a scene
output directory is provided.

Example:

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-viz-3d-static \
  --config configs/AreaVPLSmoke.v2.json \
  --scene-output-root build/highres_validation_20260428/standard/area_vpl_smoke \
  --output build/test_outputs/3d_viz_static/area_overview.png \
  --report-json build/test_outputs/3d_viz_static/area_overview.json
```

For heavy scenes, use `--bounds-only` and optionally `--max-faces`:

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-viz-3d-static \
  --config configs/validation/standard/dragon_quick.v2.json \
  --bounds-only \
  --output build/test_outputs/3d_viz_static/dragon_bounds.png
```

## Interactive Viewer State

`hypo_viz.viewer3d` provides the experimental interactive surface. The first
portable backend writes a self-contained HTML inspection shell plus reproducible
viewer state. A requested GUI backend can fall back to static PNG in headless
environments.

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-viz-3d \
  --config configs/AreaVPLSmoke.v2.json \
  --scene-output-root build/highres_validation_20260428/standard/area_vpl_smoke \
  --backend html \
  --output build/test_outputs/3d_viz_viewer/area_viewer.html \
  --report-json build/test_outputs/3d_viz_viewer/area_viewer.json
```

For CI/headless checks:

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-viz-3d \
  --config configs/AreaVPLSmoke.v2.json \
  --backend pyvista \
  --headless \
  --output build/test_outputs/3d_viz_viewer/area_fallback.png \
  --report-json build/test_outputs/3d_viz_viewer/area_fallback.json
```

## Combined 2D/3D Reports

`hypo_viz.report3d` links renderer outputs back to visualization artifacts:

- `gbuffer/world_position.f32` point-cloud preview
- 2D heatmaps, VPL overlays, and shadow contact sheets
- static 3D scene overview
- shadow face labels and cube directions
- Markdown and JSON report files

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-viz-3d-report \
  --config configs/AreaVPLSmoke.v2.json \
  --scene-output-root build/highres_validation_20260428/standard/area_vpl_smoke \
  --output-dir build/test_outputs/3d_viz_report/area_vpl
```
