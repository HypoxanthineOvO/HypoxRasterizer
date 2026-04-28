# M20 IndirectVPL Validation and Visualization

完成时间：2026-04-28 14:55（上海时间）

## Summary

Added IndirectVPL validation scenes, updated Python visualization to recognize
indirect VPL buffers, rendered the Cornell comparison, and generated the final
validation report.

## Changes

- Added standard Cornell and Money IndirectVPL validation configs.
- Added heavy Dragon IndirectVPL quick config.
- Registered IndirectVPL scenes in `validation/scenes.yaml`.
- Updated 2D visualization to annotate `light_*_indirect_vpls.f32`.
- Updated 3D report JSON to count direct and indirect VPL buffers.
- Added final report `reports/indirect-vpl-validation-2026-04-28.md`.

## Generated Artifacts

- `build/test_outputs/indirect_vpl_validation/standard_cornell_indirect_vpl/`
- `build/test_outputs/indirect_vpl_validation/standard_cornell_direct_only/`
- `build/test_outputs/indirect_vpl_runner/standard/`
- `build/test_outputs/indirect_vpl_report/cornell/`

## Validation

```sh
xmake run ValidationSuiteTests
PYTHONPATH=tools/python uv run --project tools/python python -m unittest tools/python/tests/test_visualization.py tools/python/tests/test_report3d.py
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --renderer ./HypoxRasterizer --tier standard --tag indirect_vpl --render --resolution 48x48 --output-root build/test_outputs/indirect_vpl_runner
```

Result: IndirectVPL validation runner passed `2/2` standard scenes.

Final regression also passed:

- `xmake run IndirectVPLTests`
- `xmake run ValidationSuiteTests`
- `xmake run VPLAreaTests`
- `xmake run LightShadowTests`
- `xmake run BaselineTests`
- `xmake run SceneCliOutputTests`
- `xmake run GBufferTests`
- `xmake run MaterialTests`
- `xmake run ValidationSceneTests`
- `PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests`
