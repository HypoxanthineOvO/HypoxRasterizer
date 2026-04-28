# Output Baselines

M10 baselines are stats-only JSON files under `validation/baselines/`. They do
not store golden PNG or `.f32` payloads.

## Collect

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_validation.collect_stats \
  --root build/validation_outputs_m8/fast/fast_multi_light_shadow \
  --scene-id fast_multi_light_shadow \
  --output validation/baselines/fast_multi_light_shadow.json
```

The collector records:

- `.f32`: `HYPORASTER_F32_V1` header fields, shape, SHA-256, min/max/mean/std,
  finite ratio, and nonzero ratio.
- `.png`: PNG IHDR shape and SHA-256.

## Compare

Default comparison uses tolerance-based numeric stats for `.f32` files:

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_validation.compare_baseline \
  --baseline validation/baselines/fast_multi_light_shadow.json \
  --current-root build/validation_outputs_m8/fast/fast_multi_light_shadow
```

Strict hash comparison is available when byte-identical output is expected:

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_validation.compare_baseline \
  --baseline validation/baselines/fast_multi_light_shadow.json \
  --current-root build/validation_outputs_m8/fast/fast_multi_light_shadow \
  --strict-hash
```

## Current Baselines

- `validation/baselines/fast_multi_light_shadow.json`: fast G-Buffer plus point
  cube and directional shadow maps.
- `validation/baselines/standard_area_vpl_smoke.json`: Cornell/VPL standard
  scene, including `shadow/light_0_direct_vpls.f32`.

Baseline updates are always explicit. A failed comparison must not overwrite the
baseline automatically.
