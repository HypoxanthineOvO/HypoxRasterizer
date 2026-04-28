# HypoxRasterizer

Current release: v0.1.0

## Introduction
Rasterizer Simulator of ShanghaiTech VLSI Lab.

## Build

```sh
xmake
```

## Test

```sh
xmake build BaselineTests
xmake run BaselineTests
xmake run LightShadowTests
xmake run VPLAreaTests
```

## Smoke Render

```sh
xmake run HypoxRasterizer --config configs/SmokeTest.json --output-root outputs
xmake run HypoxRasterizer --config configs/DirectionalSmoke.v2.json --output-root outputs --buffers color,gbuffer,shadow
xmake run HypoxRasterizer --config configs/AreaVPLSmoke.v2.json --output-root outputs --buffers color,gbuffer,shadow
```

The smoke scene is intentionally low resolution and is used as a quick pipeline
check. It still leaves legacy root-level PNG outputs in place, and also publishes
the current renderer outputs to `outputs/<scene-name>/`.

Shadow outputs include point cube shadow faces, directional shadow maps, and VPL
debug `.f32` files for area lights.

## Scene Config V2

`configs/SmokeTest.v2.json` demonstrates the new scene schema. See
`docs/scene-config-v2.md` for the field layout, CLI examples, and migration
notes.

## Validation And Visualization

```sh
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --default --tier fast
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_validation.compare_baseline --baseline validation/baselines/fast_multi_light_shadow.json --current-root build/validation_outputs_m8/fast/fast_multi_light_shadow
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_viz.render_2d scene --root build/validation_outputs_m8_standard/standard/area_vpl_smoke --output-dir build/test_outputs/viz_area_vpl
python3 tools/validation/validation_runner.py --tier heavy --render
```

See `docs/validation-scenes.md`, `docs/external-scenes.md`,
`docs/output-baselines.md`, `docs/visualization.md`, and
`docs/3d-visualization.md` for the full validation toolchain.

## IndirectVPL

The renderer includes an experimental first-bounce IndirectVPL path. It emits
debug samples to `shadow/light_<index>_indirect_vpls.f32` and writes gathered
indirect radiance to `gbuffer/indirect_vpl_radiance.f32` when enabled by a V2
scene config.
