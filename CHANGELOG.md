# Changelog

## v0.1.0 - 2026-04-28

首个 HypoxRasterizer validation/research release。

### Features

- Added V2 scene config loading, CLI options, and structured output directories.
- Added float G-Buffer, depth previews, and `.f32` debug buffer exports.
- Added material parameter channels for albedo, roughness, and metallic data.
- Added directional lights, point cube shadow maps, and area-light direct VPL sampling.
- Added validation scene suites for fast, standard, and heavy tiers.
- Added output baseline statistics, PNG hashes, and tolerance comparison tooling.
- Added uv-managed Python tools for 2D heatmaps, VPL overlays, shadow sheets, and 3D scene reports.
- Added first-bounce IndirectVPL sampling, renderer gather integration, debug buffers, and validation scenes.

### Validation

- Release regression passed on 2026-04-28 16:03 (Shanghai time).
- C++ targets passed: `BaselineTests`, `SceneCliOutputTests`, `GBufferTests`, `MaterialTests`, `LightShadowTests`, `VPLAreaTests`, `ValidationSceneTests`, `ValidationSuiteTests`, and `IndirectVPLTests`.
- Python tool tests passed: 35 unittest cases.
- IndirectVPL standard validation runner passed 2 render scenes.

### Notes

- Current IndirectVPL support is one-bounce first-hit sampling, not full global illumination.
- Heavy external scenes are represented by metadata/fetch hooks; large downloaded assets remain outside git.
