# M10 Output Statistics, Hashes, and Tolerance Baselines

Completed at: 2026-04-28 13:04（上海时间）

## Summary

M10 added stats-only output baselines for validation scenes. The Python tooling
can parse Hypox `.f32` buffers, inspect PNG headers, collect hashes and numeric
stats, and compare current outputs against baselines with tolerance or strict
hash mode.

## Changes

- Added `hypo_validation.collect_stats`.
- Added `hypo_validation.compare_baseline`.
- Added `tools/python/tests/test_output_baselines.py`.
- Added baseline JSON files:
  - `validation/baselines/fast_multi_light_shadow.json`
  - `validation/baselines/standard_area_vpl_smoke.json`
- Added `docs/output-baselines.md`.
- Added baseline path metadata to `validation/scenes.yaml`.
- Added uv scripts `hypo-collect-stats` and `hypo-compare-baseline`.

## Verification

- `PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests` - 9 passed
- `PYTHONPATH=tools/python uv run --project tools/python python -m hypo_validation.compare_baseline --baseline validation/baselines/fast_multi_light_shadow.json --current-root build/validation_outputs_m8/fast/fast_multi_light_shadow --strict-hash`
- `PYTHONPATH=tools/python uv run --project tools/python python -m hypo_validation.compare_baseline --baseline validation/baselines/standard_area_vpl_smoke.json --current-root build/validation_outputs_m8_standard/standard/area_vpl_smoke`
- `python3 -m py_compile tools/python/hypo_validation/collect_stats.py tools/python/hypo_validation/compare_baseline.py`
- `xmake run ValidationSceneTests` - 5 passed
- `xmake run ValidationSuiteTests` - 5 passed
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --default --tier fast`

## Notes

- Baseline updates are explicit; compare failures never overwrite baselines.
- The standard VPL baseline includes `shadow/light_0_direct_vpls.f32`.
