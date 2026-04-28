# M9 External GI Scene Source Ingestion

Completed at: 2026-04-28 12:56（上海时间）

## Summary

M9 introduced an auditable external scene ingestion layer. External sources now
have explicit URL, license, target, format, hash policy, conversion hook, tier,
and enabled/disabled status. The Python tools live under one uv-managed project
at `tools/python/`.

## Changes

- Added `tools/python/pyproject.toml`, `uv.lock`, and package placeholders for
  `hypo_assets`, `hypo_validation`, and `hypo_viz`.
- Added `hypo_assets.external_sources` for manifest parsing, validation, and
  download planning.
- Added `hypo_assets.fetch_scenes` for dry-run/download planning, approved
  proxy ports, skip-existing behavior, and SHA-256 verification/recording.
- Added `validation/external_sources.yaml` with enabled Bitterli standard
  candidates and disabled PBRT/ORCA heavy metadata hooks.
- Updated `tools/validation/validation_runner.py` so missing optional external
  or heavy scenes are skipped cleanly in render mode.
- Added `docs/external-scenes.md` and updated `assets/external/README.md`.

## Verification

- `PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests` - 5 passed
- `PYTHONPATH=tools/python uv run --project tools/python python -m hypo_assets.fetch_scenes --manifest validation/external_sources.yaml --dry-run`
- `PYTHONPATH=tools/python uv run --project tools/python python -m hypo_assets.fetch_scenes --manifest validation/external_sources.yaml --dry-run --include-disabled --proxy-port 7890`
- `python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --tier heavy --render`
- `xmake run ValidationSceneTests` - 5 passed
- `xmake run ValidationSuiteTests` - 5 passed
- `python3 -m py_compile tools/validation/validation_runner.py tools/python/hypo_assets/external_sources.py tools/python/hypo_assets/fetch_scenes.py`

## Notes

- Direct Bitterli download was reachable but too slow in this environment and
  timed out before completion. No partial external zip is kept in the repo.
- The fetcher supports retries by rerunning with `--proxy-port 7890`, `7891`, or
  `7892`; first successful download should record the printed SHA-256.
