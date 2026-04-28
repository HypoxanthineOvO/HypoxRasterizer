from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from .collect_stats import collect_output_tree


NUMERIC_KEYS = ["min", "max", "mean", "std", "finite_ratio", "nonzero_ratio"]


@dataclass(frozen=True)
class BaselineDifference:
    kind: str
    path: str
    message: str


def _as_float(value: Any) -> float | None:
    if value is None:
        return None
    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def _outside_tolerance(expected: float, actual: float, abs_tol: float, rel_tol: float) -> bool:
    allowed = abs_tol + rel_tol * abs(expected)
    return abs(expected - actual) > allowed


def compare_baselines(
    baseline: dict[str, Any],
    current: dict[str, Any],
    *,
    strict_hash: bool = False,
    abs_tol: float = 1e-4,
    rel_tol: float = 1e-4,
) -> list[BaselineDifference]:
    diffs: list[BaselineDifference] = []
    baseline_files = baseline.get("files", {})
    current_files = current.get("files", {})

    for path, expected in sorted(baseline_files.items()):
        actual = current_files.get(path)
        if actual is None:
            diffs.append(BaselineDifference("missing_file", path, "file is missing from current output"))
            continue

        if expected.get("kind") != actual.get("kind"):
            diffs.append(
                BaselineDifference(
                    "kind_mismatch",
                    path,
                    f"expected {expected.get('kind')} got {actual.get('kind')}",
                )
            )
            continue

        if expected.get("shape") != actual.get("shape"):
            diffs.append(
                BaselineDifference(
                    "shape_mismatch",
                    path,
                    f"expected shape {expected.get('shape')} got {actual.get('shape')}",
                )
            )

        if strict_hash and expected.get("sha256") != actual.get("sha256"):
            diffs.append(BaselineDifference("hash_mismatch", path, "sha256 differs"))

        if expected.get("kind") != "f32":
            continue
        for key in NUMERIC_KEYS:
            expected_value = _as_float(expected.get(key))
            actual_value = _as_float(actual.get(key))
            if expected_value is None or actual_value is None:
                if expected_value != actual_value:
                    diffs.append(
                        BaselineDifference("numeric_drift", path, f"{key}: expected {expected_value} got {actual_value}")
                    )
                continue
            if _outside_tolerance(expected_value, actual_value, abs_tol, rel_tol):
                diffs.append(
                    BaselineDifference(
                        "numeric_drift",
                        path,
                        f"{key}: expected {expected_value} got {actual_value}",
                    )
                )
                break

    for path in sorted(set(current_files) - set(baseline_files)):
        diffs.append(BaselineDifference("extra_file", path, "file is not present in baseline"))

    return diffs


def _load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Compare validation output stats to a baseline.")
    parser.add_argument("--baseline", required=True, help="baseline JSON file")
    parser.add_argument("--current", help="current stats JSON file")
    parser.add_argument("--current-root", help="collect current stats from this scene output directory")
    parser.add_argument("--strict-hash", action="store_true")
    parser.add_argument("--abs-tol", type=float, default=1e-4)
    parser.add_argument("--rel-tol", type=float, default=1e-4)
    args = parser.parse_args(argv)

    baseline = _load_json(Path(args.baseline))
    if args.current:
        current = _load_json(Path(args.current))
    elif args.current_root:
        current = collect_output_tree(Path(args.current_root), scene_id=baseline.get("scene_id", ""))
    else:
        parser.error("one of --current or --current-root is required")

    diffs = compare_baselines(
        baseline,
        current,
        strict_hash=args.strict_hash,
        abs_tol=args.abs_tol,
        rel_tol=args.rel_tol,
    )
    for diff in diffs:
        print(f"{diff.kind}: {diff.path}: {diff.message}")
    print(f"summary: differences={len(diffs)}")
    return 1 if diffs else 0


if __name__ == "__main__":
    raise SystemExit(main())
