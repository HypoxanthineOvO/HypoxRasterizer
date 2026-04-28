# Validation Scenes

The validation suite is organized through `validation/scenes.yaml`.

## Tiers

- `fast`: default daily checks and corner cases. These should stay cheap.
- `standard`: representative validation scenes, including texture scenes and
  medium mesh scenes such as Bunny or quick Dragon variants.
- `heavy`: optional benchmark scenes and external GI assets. These must not run
  by default.

## Manifest Fields

Each scene entry records:

- `id`: stable scene identifier.
- `tier`: `fast`, `standard`, or `heavy`.
- `config`: renderer config path.
- `default`: whether default selection includes the scene.
- `tags`: capabilities covered by the scene.
- `assets`: required asset paths.
- `expected_outputs`: expected output groups such as `color`, `gbuffer`, and
  `shadow`.
- `runtime_budget_seconds`: rough runtime budget for low-resolution validation.
- `source_type` and `license`: source and license tracking.

Proxy ports `7890`, `7891`, and `7892` are approved for later external scene
fetch tooling. M7 only records this contract; downloading starts in M9.

## Current Internal Scene Coverage

Fast scenes are hand-written corner cases and should stay cheap:

- `fast_empty_scene`: no objects/lights, verifies empty color and G-Buffer export.
- `fast_offscreen_triangle`: geometry outside the view, verifies blank geometry
  handling without treating it as a failure.
- `fast_multi_light_shadow`: point plus directional light, verifies mixed shadow
  output including point cube faces.
- `fast_missing_asset_error`: expected-failure fixture for missing asset
  diagnostics; it is not selected by default.
- `fast_point_smoke`: legacy point-light smoke scene kept in the default fast
  suite.

Standard internal scenes exercise representative assets:

- `standard_directional_smoke`: directional shadow smoke scene.
- `standard_area_vpl_smoke`: CornellBox-style area light and VPL output path.
- `standard_money_texture`: textured Money mesh using `assets/Money/2.png`.
- `standard_bunny_mesh`: medium Bunny mesh with point cube shadow output.
- `standard_dragon_quick`: low-resolution Stanford Dragon quick validation.

Heavy scenes are explicit benchmark/release checks and are never selected by
daily default:

- `heavy_dragon_lowres_benchmark`: internal Stanford Dragon at 24x24, verifies a
  medium-heavy mesh can still produce color, G-Buffer, and point cube shadow
  output within the heavy budget.
- `heavy_orca_bistro_hook`: external NVIDIA ORCA Bistro hook. Missing assets are
  skipped with a clear warning; fetch/source metadata lives in
  `validation/external_sources.yaml`.

## Runner

The lightweight runner can dry-run selected scenes, execute renders, and report
per-scene pass/fail status plus a summary. Render mode also checks that each
scene's expected output groups exist under
`<output-root>/<tier>/<scene-name>/`.

```sh
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml
python3 tools/validation/validation_runner.py --tier standard --tag vpl
python3 tools/validation/validation_runner.py --tier heavy
python3 tools/validation/validation_runner.py --all --include-heavy
```

Add `--render` to execute the printed renderer commands. Heavy scenes are not
selected by default unless `--include-heavy` is provided.

Core validation commands:

```sh
python3 tools/validation/validation_runner.py --default --tier fast --render
python3 tools/validation/validation_runner.py --tier standard --render
python3 tools/validation/validation_runner.py --tier standard --tag dragon --render
python3 tools/validation/validation_runner.py --tier heavy --render
```

Recommended tiers:

- Daily: `python3 tools/validation/validation_runner.py --default --tier fast --render`
- Release: `python3 tools/validation/validation_runner.py --tier standard --render`
- Benchmark/manual: `python3 tools/validation/validation_runner.py --tier heavy --render`

The latest heavy Dragon low-resolution measurement is recorded in
`validation/perf/heavy_dragon_lowres_benchmark.json`.
