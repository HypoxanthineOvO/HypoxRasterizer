# External Validation Scenes

External sources are tracked in `validation/external_sources.yaml`. Large files
stay under `assets/external/` and are ignored by git.

## Source Tiers

- `bitterli_cornell_box_pbrt_v4`: enabled standard candidate, public-domain,
  PBRT v4 zip. This is the first small GI source hook.
- `bitterli_veach_ajar_pbrt_v4`: enabled standard candidate, public-domain,
  PBRT v4 zip. This is a medium GI/material-lighting source hook.
- `pbrt_v4_bmw_m6_metadata`: disabled heavy metadata hook. Conversion and sparse
  fetching are intentionally deferred.
- `orca_bistro_heavy_metadata`: disabled heavy metadata hook for Bistro under
  CC-BY-4.0.

## Fetcher

Use the uv-managed tools project:

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_assets.fetch_scenes --manifest validation/external_sources.yaml --dry-run
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_assets.fetch_scenes --manifest validation/external_sources.yaml --dry-run --include-disabled --proxy-port 7890
PYTHONPATH=tools/python uv run --project tools/python python -m hypo_assets.fetch_scenes --manifest validation/external_sources.yaml --source bitterli_cornell_box_pbrt_v4 --proxy-port 7890
```

Approved proxy ports are `7890`, `7891`, and `7892`. The fetcher does not require
a proxy, but it validates requested proxy ports against the manifest before use.

## Hash Policy

Sources may use a recorded 64-character SHA-256 or a named hash policy:

- `record_on_first_download`: download first, print the SHA-256 to record later.
- `upstream_git_commit`: pin by upstream commit/tag before enabling download.
- `upstream_release`: pin by upstream release metadata before enabling download.

If a source has `sha256: pending`, it is not considered fully locked yet and
must not become default runnable validation without recording a hash or a more
specific upstream revision.

## Current Network Note

On 2026-04-28 12:50（上海时间）, direct download from Bitterli responded but was
very slow in this environment. The fetcher dry-run and proxy-aware planning are
therefore the reliable M9 contract; actual asset download can be retried with
`--proxy-port 7890`, `7891`, or `7892`.
