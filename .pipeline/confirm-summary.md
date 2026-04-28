# Confirm Summary: Validation Scene Suite Append Plan

## Project

- Project: HypoxRasterizer
- Stack: C++17, xmake, Eigen, stb, nlohmann_json, tinyobjloader, Python with uv
- Preset: TDD
- Plan mode: interactive
- Plan shape: append mode
- Existing completed prompts: 6
- New prompts: 6
- Total prompts after append: 12

## New Milestones

1. `06-validation-dataset-runner.md`: validation manifest, tier model, and batch runner foundation.
2. `07-fast-standard-validation-scenes.md`: fast corner cases and internal standard scenes including texture and Dragon/Bunny quick scenes.
3. `08-external-gi-scene-ingestion.md`: external GI scene source manifest, license/hash policy, uv fetch tooling, proxy support.
4. `09-output-baseline-stats.md`: `.f32`/PNG stats, hash, tolerance baselines, comparator.
5. `10-uv-2d-visualization-tools.md`: uv-managed 2D visualization for VPL overlays, shadow annotations, and G-Buffer/depth heatmaps.
6. `11-heavy-scene-validation-docs.md`: heavy scene hooks, performance budgets, final validation documentation.

## Test Points

- Manifest schema validation, tier/tag filtering, missing asset diagnostics.
- Fast suite end-to-end rendering with color, G-Buffer, depth, and shadow outputs.
- Standard scenes covering texture assets and Dragon/Bunny-level geometry.
- External source dry-run, license/source/hash checks, proxy-aware downloads.
- `.f32` parser, PNG/stat collectors, baseline comparator, strict/hash and tolerance modes.
- 2D visualization unit tests and annotated output generation.
- Heavy suite enumeration, missing external asset skip behavior, reduced-resolution heavy render.

## Generated Files

- `.pipeline/prompts/06-validation-dataset-runner.md`
- `.pipeline/prompts/07-fast-standard-validation-scenes.md`
- `.pipeline/prompts/08-external-gi-scene-ingestion.md`
- `.pipeline/prompts/09-output-baseline-stats.md`
- `.pipeline/prompts/10-uv-2d-visualization-tools.md`
- `.pipeline/prompts/11-heavy-scene-validation-docs.md`
- `.plan-state/discover.yaml`
- `.plan-state/decompose.yaml`
- `.plan-state/generate.yaml`
- `.pipeline/architecture.md`
- `.pipeline/state.yaml`

## Decisions

- Use one shared uv Python project at `tools/python/pyproject.toml`.
- External resource downloads are allowed.
- Proxy ports 7890, 7891, and 7892 are approved for download tooling.
- First external batch prioritizes auditable small/mid scenes; Bistro/San Miguel-style scenes start optional/heavy but remain future validation targets.
- First visualization milestone is 2D only.
- Baselines combine file/header, numeric stats, hashes, and tolerance comparisons.

## Awaiting Confirmation

Interactive planning requires explicit confirmation before execution. Use
`/hypo-workflow:start` or say `确认，开始执行` to run M7.
