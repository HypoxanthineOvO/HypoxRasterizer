# M8 Fast Corner Cases and Standard Internal Scenes

Completed at: 2026-04-28 12:38（上海时间）

## Summary

M8 added executable fast corner cases and representative internal standard
scenes. The suite now covers empty/offscreen cases, missing asset diagnostics,
mixed point/directional shadows, texture loading, Bunny, Dragon quick, and the
existing Cornell/VPL smoke path.

## Changes

- Added `Tests/ValidationSuiteTests.cpp` and registered `ValidationSuiteTests`.
- Added fast V2 configs under `configs/validation/fast/`.
- Added standard V2 configs under `configs/validation/standard/`.
- Updated `validation/scenes.yaml` with tags, assets, expected outputs, runtime
  budgets, and default-selection behavior.
- Preserved V2 texture paths in `SceneMaterialConfig`.
- Mapped V2 texture materials to legacy `TextureMat` in the renderer adapter.
- Extended `tools/validation/validation_runner.py` with render pass/fail
  summaries and expected-output group checks.
- Updated `docs/validation-scenes.md` and `.pipeline/architecture.md`.

## Verification

- `xmake build -y HypoxRasterizer`
- `xmake run BaselineTests` - 6 passed
- `xmake run SceneCliOutputTests` - 6 passed
- `xmake run GBufferTests` - 4 passed
- `xmake run MaterialTests` - 4 passed
- `xmake run LightShadowTests` - 4 passed
- `xmake run VPLAreaTests` - 3 passed
- `xmake run ValidationSceneTests` - 5 passed
- `xmake run ValidationSuiteTests` - 5 passed
- `python3 -m py_compile tools/validation/validation_runner.py`
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml`
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --tier standard`
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --default --tier fast --output-root build/validation_outputs_m8 --render`
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --tier standard --output-root build/validation_outputs_m8_standard --render`

## Notes

- `fast_missing_asset_error` is intentionally excluded from default selection.
- Dragon quick rendered successfully with the full Stanford Dragon mesh at
  16x16 resolution, keeping it usable as standard validation rather than heavy
  only.
