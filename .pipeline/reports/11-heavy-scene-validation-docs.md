# M12 Heavy Scene Validation and Documentation Pass

Completed at: 2026-04-28 13:19（上海时间）

## Summary

M12 finalized the validation scene suite into daily, release, and benchmark
layers. Heavy validation now has an internal Dragon low-resolution benchmark
that actually renders, plus an external Bistro hook that skips cleanly when
assets are not downloaded.

## Changes

- Added `configs/validation/heavy/dragon_lowres_benchmark.v2.json`.
- Added `heavy_dragon_lowres_benchmark` and `heavy_orca_bistro_hook` to
  `validation/scenes.yaml`.
- Added `validation/perf/heavy_dragon_lowres_benchmark.json`.
- Added `tools/python/tests/test_heavy_validation_docs.py`.
- Updated `docs/validation-scenes.md` with daily/release/benchmark commands,
  heavy scene descriptions, and performance metadata.
- Updated README with the heavy runner command.

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
- `PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests` - 17 passed
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --default --tier fast`
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --tier standard`
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --tier heavy --render --output-root build/test_outputs/final_heavy`
- `PYTHONPATH=tools/python uv run --project tools/python python -m hypo_validation.compare_baseline --baseline validation/baselines/fast_multi_light_shadow.json --current-root build/validation_outputs_m8/fast/fast_multi_light_shadow`
- `PYTHONPATH=tools/python uv run --project tools/python python -m hypo_validation.compare_baseline --baseline validation/baselines/standard_area_vpl_smoke.json --current-root build/validation_outputs_m8_standard/standard/area_vpl_smoke`

## Heavy Result

- `heavy_dragon_lowres_benchmark`: passed, generated color, G-Buffer, and point
  cube shadow outputs under `build/test_outputs/final_heavy/`.
- `heavy_orca_bistro_hook`: skipped because external Bistro assets are not
  downloaded.

Latest recorded heavy Dragon runtime: `3.616037436` seconds at 24x24.

## Next Work

- Replace temporary V2-to-legacy render adapter with direct `SceneConfig`
  consumption.
- Convert external PBRT/ORCA scenes into runnable V2 configs.
- Improve BRDF and shadow quality, including evaluating Cascade Shadow Maps.
- Add richer baseline policies once external scene hashes are recorded.
