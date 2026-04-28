# Planned Architecture Evolution

This file records the planning-time target architecture. The initialization
baseline remains in `.pipeline/architecture/`.

## Planning Context

- Project: HypoxRasterizer
- Stack: C++17, xmake, Eigen, stb, nlohmann_json, tinyobjloader
- Preset: TDD
- Plan shape: append mode over an initialized existing project
- Milestones: 6

## Target Boundaries

The current renderer concentrates too much responsibility in the rasterizer,
light, material, and image output paths. The planned architecture should split
the work into clearer contracts:

- Build/test layer: repeatable xmake targets and smoke render checks.
- Scene layer: new JSON schema, validation, resource paths, and CLI overrides.
- Output layer: `OutputManager` or equivalent module owning
  `outputs/<scene-name>/{color.png,gbuffer/,shadow/}`.
- Buffer layer: framebuffer/GBuffer channel definitions, float dumps, PNG
  previews, metadata for future quantization.
- Material layer: material parameters, texture resources, samplers, and shading
  models separated from rasterization loops.
- Light/shadow layer: light sampling, shadow resources, visibility queries, and
  structured shadow debug output.
- Demo/quality layer: representative scenes and documented rendering outputs.

## Milestone Architecture Intent

### M1 Build/Test Baseline

Add executable regression coverage before major refactors. This milestone should
not redesign rendering behavior; it should make later changes measurable.

### M2 Scene/CLI/Output Contract

Introduce the long-term scene and command surface. New schema design is allowed;
legacy configs are migration references, not hard constraints.

### M3 G-Buffer and Float Output

Create a first-class buffer system with explicit coordinate spaces and float
data output. PNG files are previews, not the authoritative data format. The
channel metadata should leave room for fixed-point quantization.

### M4 Material System

Make the existing Blinn-Phong/Phong path correct first, then add a clean data
path for PBR metallic-roughness and common texture channels.

### M5 Directional/Point Shadows

Treat Directional and Point lights as first-class. Point lights must expose a
structured cube shadow map instead of ad hoc six-file output. Cascade shadow maps
are deliberately deferred.

### M6 VPL and Quality Pass

Use VPL sampling as a visible capability, especially for area lights and scenes
that stress many samples and cube shadow behavior. Close the first planning cycle
with representative demos and documented limitations.

## Deferred Decisions

- Cascade Shadow Map: evaluate after M5/M6 once directional shadow quality is
  visible.
- Fixed-point quantization: M3 must reserve metadata and writer boundaries, but
  exact bit width and error metrics are a future design step.
- GPU acceleration: out of scope for this plan; current work evolves the CPU
  rasterizer.

## Append Plan: Validation Scene Suite

The next planning cycle appends six milestones after M6. It focuses on
validation data rather than new renderer features.

### M7 Validation Dataset Manifest and Runner

Create a scene manifest, tiered suite model, and batch runner foundation.
Validation scenes will be grouped as fast, standard, and heavy. The manifest
will hold expected outputs, assets, tags, runtime budget, source, and license.

ADDED

- `Utils/Validation` with a small validation manifest parser for the project
  YAML subset.
- `ValidationSceneTests` covering manifest parsing, tier/tag selection, default
  heavy exclusion, missing reference diagnostics, and repository manifest
  contracts.
- `validation/scenes.yaml` with fast/standard/heavy seed entries and approved
  proxy ports 7890, 7891, and 7892.
- `tools/validation/validation_runner.py` as a standard-library dry-run runner
  for enumerating selected scenes and printing renderer commands.
- `configs/validation/{fast,standard,heavy}/` directory structure and
  `assets/external/` placeholder policy.
- `docs/validation-scenes.md` documenting tiers, manifest fields, and runner
  usage.

CHANGED

- `xmake.lua` now builds `ValidationSceneTests` and includes
  `Utils/Validation`.

REASON

- Later fast/standard/heavy scene work needs a stable manifest and selection
  contract before adding many configs or external assets.

IMPACT

- M8 can add scene data without designing the suite format again.
- M9-M11 can reuse the manifest fields for external sources, baselines, and
  visualization.

### M8 Fast and Standard Scenes

Add hand-written fast corner cases and high-quality internal standard scenes.
Standard validation must include texture scenes and Dragon/Bunny-level geometry
in a quick configuration, not just simple triangle smoke tests.

ADDED

- `ValidationSuiteTests` covering required fast scene registration, default
  expected-failure exclusion, standard texture/Bunny/Dragon coverage, missing
  asset diagnostics, and V2 texture path preservation.
- Fast V2 configs under `configs/validation/fast/` for empty scene, offscreen
  geometry, multi-light shadow, and missing-asset error handling.
- Standard V2 configs under `configs/validation/standard/` for Money texture,
  Bunny mesh, and Dragon quick validation.
- Manifest entries for the new fast and standard scenes with tags, assets,
  expected outputs, runtime budgets, and default-selection behavior.

CHANGED

- `SceneMaterialConfig` now preserves a V2 material texture path.
- The V2-to-legacy adapter in `main.cpp` maps texture materials to legacy
  `TextureMat` with `TextureFilePath`.
- `tools/validation/validation_runner.py` now reports per-scene status,
  aggregate pass/fail/dry-run counts, and verifies expected output groups after
  render mode.
- `docs/validation-scenes.md` documents the current internal scene coverage and
  M8 runner commands.

REASON

- The validation suite needed representative internal data before external GI
  ingestion. Texture and medium-mesh scenes must be executable at standard
  budgets, while known-bad assets remain explicit diagnostics instead of
  default failures.

IMPACT

- Daily default validation now covers more than triangle smoke tests.
- M9 can add external scenes without changing the manifest shape.
- M10 can consume the same scene/output contract for statistical baselines and
  tolerance comparisons.

### M9 External GI Scene Ingestion

Add auditable external source metadata and uv-managed download tooling. Candidate
sources include Bitterli Rendering Resources, PBRT v4 scenes, and
McGuire/NVIDIA ORCA scenes. Large external assets stay out of git and begin as
optional/heavy hooks when conversion is not yet stable.

ADDED

- `tools/python/pyproject.toml` as the shared uv-managed Python tools project.
- `hypo_assets.external_sources` for manifest parsing, required metadata
  validation, and download planning.
- `hypo_assets.fetch_scenes` CLI for dry-run/download planning, approved proxy
  ports, skip-existing behavior, and hash verification/recording policy.
- Placeholder `hypo_validation` and `hypo_viz` Python modules for M10/M11.
- `validation/external_sources.yaml` with enabled Bitterli standard candidates
  and disabled PBRT/ORCA heavy metadata hooks.
- `docs/external-scenes.md` documenting source tiers, proxy usage, hash policy,
  and the current slow-source note.
- Python unit tests for source validation, dry-run metadata, proxy ports, and
  runner skip behavior.

CHANGED

- `tools/validation/validation_runner.py` now parses source/license metadata,
  preflights config/assets in render mode, and skips missing optional external
  or heavy scenes instead of failing the selected suite.

REASON

- External GI scenes need auditability before they can influence validation:
  source, license, target, conversion, and hash policy are now explicit.
- Heavy external scenes should be discoverable without making default validation
  brittle when large assets are not downloaded.

IMPACT

- M10 can build output baselines over the same internal/external manifest model.
- M11 can use the uv Python project without creating another environment.
- Actual Bitterli downloads are supported, but first-run hash recording remains
  pending because direct download was very slow in this environment.

### M10 Output Baselines

Add statistical baselines for `.f32` and PNG outputs. Baselines should combine
file/header checks, numeric stats, hashes, and tolerance comparisons rather than
relying on visual inspection.

ADDED

- `hypo_validation.collect_stats` for `.f32` header/payload parsing, PNG IHDR
  shape parsing, SHA-256 collection, and numeric stats.
- `hypo_validation.compare_baseline` for tolerance-based stats comparison and
  optional strict hash comparison.
- Python unit tests covering malformed `.f32` headers, PNG shape/hash stats,
  missing files, shape mismatch, numeric drift, strict hash mismatch, and output
  tree roundtrip.
- Stats-only baseline JSON files under `validation/baselines/` for
  `fast_multi_light_shadow` and `standard_area_vpl_smoke`.
- `docs/output-baselines.md` documenting explicit collect/compare/update flow.

CHANGED

- `tools/python/pyproject.toml` now exposes `hypo-collect-stats` and
  `hypo-compare-baseline` scripts.
- `validation/scenes.yaml` records baseline paths for the fast multi-light and
  standard VPL scenes.

REASON

- Validation needs machine-checkable output health without committing bulky
  golden images or float buffers.
- Tolerance stats are the default because strict byte hash comparison is too
  brittle for future renderer changes unless explicitly requested.

IMPACT

- M11 visualization can read the same `.f32` files and baseline metadata.
- M12 can include baseline comparison in final/heavy validation reporting.

### M11 2D Visualization

Add uv-managed Python 2D visualization tools for heatmaps, contact sheets, VPL
sample overlays, and shadow face annotations. 3D interactive visualization is
deferred.

ADDED

- `hypo_viz.render_2d` with `.f32` heatmap rendering, VPL sample overlays,
  shadow contact sheets, scene batch visualization, and a CLI.
- `pillow` as the only M11 visualization dependency in the uv project.
- Python unit tests for heatmap output, VPL overlay annotation, point cube
  contact sheets, missing-input warnings, and stable output naming.
- Example outputs under `build/test_outputs/viz_area_vpl/` and
  `build/test_outputs/viz_fast_shadow/`.
- `docs/visualization.md` with scene and individual command examples.

CHANGED

- `tools/python/pyproject.toml` now exposes the `hypo-viz` command.
- README now points to validation, baseline, external-source, and visualization
  docs.

REASON

- The validation suite now produces enough buffers that visual debugging needs
  repeatable static images, especially for VPL sample locations and shadow map
  faces.

IMPACT

- M12 can include generated visualization artifacts in final validation docs.
- 3D/interactive visualization remains deferred; M11 is intentionally static
  and batch-friendly.

### M12 Heavy Validation and Documentation

Add heavy scene hooks, runtime budgets, reduced-resolution validation for at
least one internal heavy/medium-heavy scene, and final documentation describing
scene purpose, outputs, source/license, and default/release/benchmark tiers.

ADDED

- `configs/validation/heavy/dragon_lowres_benchmark.v2.json` as an internal
  heavy Dragon benchmark that renders at 24x24.
- `heavy_dragon_lowres_benchmark` and `heavy_orca_bistro_hook` manifest entries.
- `validation/perf/heavy_dragon_lowres_benchmark.json` with runtime, command,
  output root, resolution, and budget metadata.
- Python tests for heavy manifest entries, heavy pass/skip runner behavior,
  perf metadata, and documented commands.

CHANGED

- `docs/validation-scenes.md` now documents fast/standard/heavy command tiers,
  heavy scene purposes, and performance metadata.
- README now includes the heavy runner command alongside validation/baseline/viz
  commands.

REASON

- Heavy scenes must be executable and documented without slowing down daily
  validation or making missing external assets look like renderer failures.

IMPACT

- The validation stack now has daily, release, and benchmark/manual entry
  points.
- Future work can replace external hooks with converted Bistro/Sponza/San Miguel
  configs without changing runner semantics.

## Execution Notes

### M1 Build/Test Baseline

ADDED

- `BaselineTests` xmake target.
- Lightweight test harness under `Tests/TestUtils.hpp`.
- Baseline automated tests covering triangle interpolation, camera basis,
  image output, config parsing/error handling, and OBJ loading.
- `configs/SmokeTest.json` as a low-resolution render smoke scene.
- Vendored header-only `stb` and `nlohmann` headers under `third_party/include`
  to avoid online package-fetch instability for those libraries.

CHANGED

- `xmake.lua` now treats `stb` and `nlohmann_json` as vendored header-only
  includes while leaving Eigen, OpenMP, and tinyobjloader under xmake package
  management.
- README now records build, baseline test, and smoke render commands.

REASON

- M1 required a reliable, repeatable baseline before larger renderer refactors.
- Online xmake package fetches for header-only dependencies failed repeatedly in
  this environment, so vendoring these small headers makes the baseline command
  reproducible.

IMPACT

- Later milestones can rely on `xmake run BaselineTests` and
  `xmake run HypoxRasterizer configs/SmokeTest.json` as the minimum safety net.
- The broader rendering architecture is unchanged.

### M2 Scene/CLI/Output Contract

ADDED

- `CliOptions` and `parseCliOptions` for config path, output root, buffer
  selection, resolution override, and debug/verbose flags.
- `SceneConfig` V2 loader for the new long-term scene schema.
- `OutputManager` for `outputs/<scene-name>/{color.png,gbuffer/,shadow/}`.
- `configs/SmokeTest.v2.json` as the first V2 scene fixture.
- `docs/scene-config-v2.md` with schema and CLI migration notes.
- `SceneCliOutputTests` covering CLI parsing, schema validation, output layout,
  and buffer selection.

CHANGED

- `main.cpp` now accepts named CLI options while preserving positional config
  compatibility.
- The renderer still uses the legacy `Rasterizer` path internally, but `main.cpp`
  can materialize a temporary legacy config from the V2 smoke schema and publish
  legacy outputs to the new output layout.
- README now points users to the new output-root command shape and V2 schema doc.

REASON

- M2 needed a simple, explicit external contract before M3 rewrites buffer
  internals.
- The temporary V2-to-legacy adapter lets the new schema be exercised before the
  rendering core directly consumes the new scene model.

IMPACT

- M3 can build real G-Buffer writers behind `OutputManager`.
- Later milestones should remove the temporary adapter once the renderer consumes
  `SceneConfig` directly.
- Buffer selection is now part of the command contract and must be preserved by
  future output code.

### M3 G-Buffer and Float Output

ADDED

- `GBuffer` and `GBufferSample` with explicit world/view position, linear depth,
  world/view normal, albedo, material id, roughness, metallic, and shadow
  visibility channels.
- `FloatBufferWriter` with a stable `HYPORASTER_F32_V1` header and raw float
  payload.
- `GBufferTests` covering channel metadata, sample storage, uniform depth
  preview, and float writer output.
- `Rasterizer::ExportGBuffer` to write `.f32` channel files and
  `linear_depth_preview.png` under `outputs/<scene>/gbuffer/`.

CHANGED

- Vertex processing now preserves view-space triangles so fragment processing can
  interpolate view-space position separately from clip/screen position.
- Legacy depth and shadow preview normalization now avoid division by zero when
  all values are equal.
- `main.cpp` exports G-Buffer data when the selected buffers include `gbuffer`.

REASON

- M3 required float buffer data as the authoritative output, with PNG only as a
  preview/debug representation.
- View-space data needed to be preserved before projection to avoid using NDC as
  view position.

IMPACT

- M4 can write real material parameters into `albedo`, `roughness`, `metallic`,
  and `material_id` instead of defaults.
- M5 can replace the placeholder `shadow_visibility` with real shadow results.
- Current material and shadow channels are structurally present but semantically
  partial until those later milestones.

### M4 Material System

ADDED

- `MaterialSample` as the common material sampling result.
- `PBRMaterial` with base color, roughness, and metallic parameters.
- `MaterialTests` covering color material defaults, PBR parameters, texture
  dimensions, and V2 material field parsing.
- Roughness and metallic fields in both legacy `MaterialConfig` and V2
  `SceneMaterialConfig`.

CHANGED

- `TextureMaterial` now uses the actual loaded image width and height instead of
  assuming 1024 by 1024.
- `ImageData` exposes image dimensions while preserving the old
  `readImageFromFile` API.
- `Rasterizer::ExportGBuffer` now samples material data so albedo, roughness,
  and metallic are written from the material layer.
- The temporary V2 render adapter carries roughness and metallic into the legacy
  config used by the current rasterizer.

REASON

- M4 needed a stable material parameter path before lighting and shadow work
  builds on top of G-Buffer outputs.

IMPACT

- Future BRDF work can extend `MaterialSample` without changing G-Buffer channel
  names.
- Full PBR shading is not complete; this milestone establishes the parameter and
  sampling path.

### M5 Directional/Point Shadows

ADDED

- `DirectionalLight` with normalized direction, direct-light sampling proxy, and
  a single stable shadow map.
- Explicit point-light cube face labels: `+X`, `-X`, `+Y`, `-Y`, `+Z`, `-Z`.
- Structured point cube shadow output names:
  `PointlightShadowMap_<light-index>_face_<axis>.png`.
- Directional shadow output names:
  `DirectionalLightShadowMap_<light-index>.png`.
- `configs/DirectionalSmoke.v2.json` as a runnable directional-light smoke
  fixture.
- `LightShadowTests` covering cube face labels, directional normalization, V2
  directional parsing, and shadow output publication.

CHANGED

- The legacy config path now recognizes `DirectionalLight`.
- The temporary V2 render adapter now supports both `point` and `directional`
  light records.
- `main.cpp` cleans stale legacy render outputs before each run so shadow maps
  from old scenes cannot leak into `outputs/<scene>/shadow/`.
- `Rasterizer::ExportGBuffer` writes real `shadow_visibility` values computed
  during fragment shading instead of a constant placeholder.

REASON

- M5 required Directional and Point lights to be first-class before VPL area
  demos build on them.
- Point cube shadow files needed deterministic face metadata and names rather
  than ad hoc direction strings.

IMPACT

- Cascade Shadow Map is still deferred; current directional support uses one
  shadow map and keeps the later cascade decision isolated.
- `ShadowMap` still owns its own rasterization path. M5 made the boundary
  clearer but did not fully deduplicate it with the main rasterizer loop.

### M6 VPL and Quality Pass

ADDED

- Configurable `AreaLight` VPL sampling with deterministic grid mode and
  reproducible stochastic jitter via `vpl.seed`.
- V2 area-light fields: `normal`, `size`, and `vpl.{samples,seed,stochastic}`.
- Legacy adapter support for V2 area lights.
- `Rasterizer::ExportLightDebugData` for VPL debug float output under
  `outputs/<scene>/shadow/light_<index>_direct_vpls.f32`.
- `configs/AreaVPLSmoke.v2.json` as a Cornell-box-style area-light demo using
  existing small assets.
- `VPLAreaTests` covering sample count, energy conservation, seed
  reproducibility, and V2 schema parsing.

CHANGED

- `shadow_visibility` G-Buffer metadata now describes actual direct-light
  visibility after shadow queries.
- README and `docs/scene-config-v2.md` now document point, directional, and area
  demo commands plus shadow/VPL debug output semantics.

REASON

- M6 needed VPL area lighting to be visible and inspectable, not just hard-coded
  into the light constructor.
- VPL debug data is written as float buffers to keep it consistent with the
  project’s authoritative output format.

IMPACT

- Area lights now expose many direct lighting samples while keeping the
  implementation simple enough for the CPU rasterizer.
- Cascade Shadow Maps remain deferred to a later planning cycle. The current
  demos make single-map directional quality visible enough for that decision.

## Append Plan: Python 3D Visualization

The next planning cycle appends four milestones after M12. It focuses on
extending the uv-managed Python visualization stack from static 2D outputs to
3D scene inspection.

### M13 Python 3D Scene Data Model

Build a reusable Python scene graph from V2 configs, OBJ mesh metadata,
transforms, camera, lights, and VPL/debug `.f32` buffers.

### M14 Static 3D Preview and Headless Screenshots

Generate static 3D PNG previews showing mesh/bounds, camera frustum, light
glyphs, area light rectangles, and VPL points. Headless/static output is the
first target.

### M15 Interactive 3D Viewer

Add an optional interactive viewer with layer toggles for meshes, bounds,
camera, lights, VPL points, and G-Buffer-derived point clouds. GUI unavailability
must be handled without breaking tests.

### M16 G-Buffer, Shadow, and VPL 3D Linkage

Connect renderer outputs to 3D views: `world_position.f32` point clouds, shadow
cube face directions, light/frustum metadata, VPL 2D/3D linkage, and final
visualization reports.

PLANNED STACK

- `trimesh` for mesh loading and geometry metadata.
- `PyVista` for primary 3D plotting, glyphs, static screenshots, and optional
  interactive viewer.
- `Open3D` as a fallback/adjacent option for point clouds, camera debugging, and
  headless workflows.

## Append Plan: IndirectVPL Indirect Lighting

This planning cycle now also appends four renderer milestones after M16. The
goal is to add inspectable one-bounce indirect lighting through IndirectVPLs,
while keeping the existing direct VPL and shadow outputs stable.

### M17 IndirectVPL Design and Data Contract

Define scene schema fields, C++ sample structures, metadata, and float-buffer
output names for indirect VPLs. Direct area VPLs remain separate from indirect
first-bounce samples.

### M18 IndirectVPL First-Bounce Sampler

Sample light emission from area, point, and directional lights; intersect scene
geometry; and create first-hit VPLs with position, normal, flux, material, and
source-light metadata.

### M19 IndirectVPL Shading and Visibility Integration

Gather indirect lighting from IndirectVPL samples in the main shading path, with
explicit config switches for direct-only, indirect enabled, visibility enabled,
and shadow/visibility debug modes.

### M20 IndirectVPL Validation and Visualization

Add Cornell, textured, and Dragon quick comparisons, then connect indirect VPL
buffers to the Python 2D/3D visualization and reporting pipeline.

PLANNED DEFAULT

- Start with one-bounce first-hit sampling.
- Keep IndirectVPL disabled by default so existing baselines do not drift.
- Defer RSM, multi-bounce, and MIS until the first-bounce data path is
  measurable and visualizable.

## M13 Execution Notes

ADDED

- `tools/python/hypo_viz/scene3d.py` with a headless Python scene data model for
  V2 configs, OBJ metadata, transformed bounds, camera/light records, and direct
  VPL `.f32` buffers.
- `tools/python/tests/test_scene3d.py` covering AreaVPL, Bunny metadata, Dragon
  bounds-only mode, VPL buffer parsing, and missing mesh warnings.
- `docs/3d-visualization.md` documenting the initial scene data model.

REASON

- Static and interactive 3D visualization need a stable, renderer-independent
  data model before adding screenshot or viewer code.

## M14 Execution Notes

ADDED

- `tools/python/hypo_viz/render_3d_static.py` with a headless `pillow_software`
  projection backend.
- Static preview support for object bounds, camera/frustum hints, point,
  directional, and area light glyphs, area rectangles, and direct VPL samples.
- CLI entry point `hypo-viz-3d-static`.
- Unit tests covering AreaVPL output, Dragon bounds-only output, missing mesh
  warnings, and unavailable backend errors.

REASON

- A software projection backend keeps static 3D diagnostics usable in CI and
  headless environments before optional PyVista/Open3D viewer work begins.

## M15 Execution Notes

ADDED

- `tools/python/hypo_viz/viewer3d.py` with reproducible viewer state, a
  self-contained HTML inspection backend, and explicit headless fallback to the
  M14 static preview path.
- CLI entry point `hypo-viz-3d`.
- Unit tests for viewer state generation, HTML output, static fallback, and
  unavailable interactive backend errors.

REASON

- The interactive viewer should be useful for manual inspection without making
  CI or server environments depend on a GUI. The HTML/state backend preserves
  the intended layer controls while keeping the heavier PyVista path optional.

## M16 Execution Notes

ADDED

- `tools/python/hypo_viz/report3d.py` combining 2D visualization outputs,
  static 3D previews, world-position point-cloud previews, and shadow face
  metadata into Markdown/JSON reports.
- CLI entry point `hypo-viz-3d-report`.
- Unit tests for point-cloud conversion, point-cloud preview images, cube shadow
  face metadata, and full AreaVPL report generation.

REASON

- Renderer outputs need one command that ties together G-Buffer, shadow maps,
  VPL samples, and 3D scene context before IndirectVPL adds more debug buffers.

## M17 Execution Notes

ADDED

- `IndirectVPLConfig` shared by V2 and legacy config paths.
- V2 `lights[].indirectVPL` parsing and V2-to-legacy adapter support.
- Legacy `Lights[].IndirectVPL` parsing.
- Richer `IndirectVPL` sample data with position, normal, flux, radius, bounce,
  source light, and flags.
- `vpl_debug.hpp` helpers for `light_<index>_indirect_vpls.f32` float buffers.
- `IndirectVPLTests` covering schema parsing and float output contract.

REASON

- The indirect-light implementation needs a stable data and output contract
  before adding geometry sampling or shading behavior.

## M18 Execution Notes

ADDED

- `indirect_vpl_sampler.hpp/.cpp` with ray/triangle first-hit intersection and
  deterministic first-bounce sample generation.
- Tests for ray hits, deterministic point-light samples, area/directional light
  smoke sampling, and empty geometry.

REASON

- The first implementation favors an inspectable CPU ray path over a BVH or RSM
  path. This is enough to validate one-bounce sample generation before shading
  integration.

## M19 Execution Notes

ADDED

- Renderer integration for `LightConfig::indirect_vpl`: first-bounce samples are
  generated during scene initialization and stored on the light.
- IndirectVPL gather in `FragmentShading`, with visibility and clamp switches.
- `gbuffer/indirect_vpl_radiance.f32` debug output.
- `shadow/light_<index>_indirect_vpls.f32` export through
  `ExportLightDebugData`.

REASON

- M19 connects the data and sampler contracts to visible renderer behavior while
  keeping IndirectVPL disabled by default.

## M20 Execution Notes

ADDED

- IndirectVPL validation configs for Cornell, Money texture, and Dragon quick.
- Manifest entries tagged `indirect_vpl`.
- Python 2D overlay support for `light_*_indirect_vpls.f32`.
- 3D report metadata for direct vs indirect VPL buffer counts.
- Final report and Cornell direct-only vs IndirectVPL comparison artifacts.

REASON

- IndirectVPL needs validation data and visual artifacts, not just renderer
  code. The Cornell comparison now exercises sample output, G-Buffer debug
  channels, cube shadows, and 2D/3D visualization in one path.
