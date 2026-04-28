from __future__ import annotations

import argparse
import hashlib
import json
import math
import statistics
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Any


F32_MAGIC = "HYPORASTER_F32_V1"
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


class FloatBufferFormatError(ValueError):
    pass


@dataclass(frozen=True)
class FloatBufferData:
    name: str
    width: int
    height: int
    channels: int
    values: list[float]

    @property
    def shape(self) -> list[int]:
        return [self.width, self.height, self.channels]


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _read_required_line(handle, label: str) -> str:
    line = handle.readline()
    if not line:
        raise FloatBufferFormatError(f"missing {label}")
    try:
        return line.decode("utf-8").rstrip("\n")
    except UnicodeDecodeError as exc:
        raise FloatBufferFormatError(f"invalid utf-8 in {label}") from exc


def read_f32_buffer(path: Path | str) -> FloatBufferData:
    path = Path(path)
    with path.open("rb") as handle:
        magic = _read_required_line(handle, "magic")
        if magic != F32_MAGIC:
            raise FloatBufferFormatError(f"{path}: expected {F32_MAGIC}, got {magic!r}")

        name = _read_required_line(handle, "name")
        dims = _read_required_line(handle, "dimensions").split()
        if len(dims) != 3:
            raise FloatBufferFormatError(f"{path}: expected width height channels")
        try:
            width, height, channels = [int(item) for item in dims]
        except ValueError as exc:
            raise FloatBufferFormatError(f"{path}: dimensions must be integers") from exc
        if width <= 0 or height <= 0 or channels <= 0:
            raise FloatBufferFormatError(f"{path}: dimensions must be positive")

        payload = handle.read()

    count = width * height * channels
    expected_bytes = count * 4
    if len(payload) != expected_bytes:
        raise FloatBufferFormatError(
            f"{path}: expected {expected_bytes} payload bytes, got {len(payload)}"
        )
    values = list(struct.unpack("<" + "f" * count, payload))
    return FloatBufferData(name=name, width=width, height=height, channels=channels, values=values)


def _numeric_stats(values: list[float]) -> dict[str, Any]:
    total = len(values)
    finite = [value for value in values if math.isfinite(value)]
    finite_count = len(finite)
    nonzero_count = sum(1 for value in values if value != 0.0)

    if finite:
        mean = statistics.fmean(finite)
        std = statistics.pstdev(finite) if len(finite) > 1 else 0.0
        min_value: float | None = min(finite)
        max_value: float | None = max(finite)
    else:
        mean = None
        std = None
        min_value = None
        max_value = None

    return {
        "min": min_value,
        "max": max_value,
        "mean": mean,
        "std": std,
        "nonzero_ratio": nonzero_count / total if total else 0.0,
        "finite_ratio": finite_count / total if total else 0.0,
    }


def _png_shape(path: Path) -> list[int]:
    data = path.read_bytes()
    if len(data) < 33 or not data.startswith(PNG_SIGNATURE):
        raise ValueError(f"{path}: not a supported PNG file")
    chunk_type = data[12:16]
    if chunk_type != b"IHDR":
        raise ValueError(f"{path}: missing IHDR chunk")
    width, height = struct.unpack(">II", data[16:24])
    color_type = data[25]
    channels_by_color_type = {
        0: 1,
        2: 3,
        3: 1,
        4: 2,
        6: 4,
    }
    channels = channels_by_color_type.get(color_type, 0)
    if channels == 0:
        raise ValueError(f"{path}: unsupported PNG color type {color_type}")
    return [width, height, channels]


def collect_path_stats(path: Path | str, root: Path | str | None = None) -> dict[str, Any]:
    path = Path(path)
    root_path = Path(root) if root is not None else path.parent
    rel = path.relative_to(root_path).as_posix() if path.is_relative_to(root_path) else path.name

    common: dict[str, Any] = {
        "path": rel,
        "sha256": sha256_file(path),
        "bytes": path.stat().st_size,
    }
    if path.suffix == ".f32":
        buffer = read_f32_buffer(path)
        return {
            **common,
            "kind": "f32",
            "name": buffer.name,
            "shape": buffer.shape,
            **_numeric_stats(buffer.values),
        }
    if path.suffix == ".png":
        return {
            **common,
            "kind": "png",
            "shape": _png_shape(path),
        }
    raise ValueError(f"unsupported output file type: {path}")


def collect_output_tree(root: Path | str, scene_id: str = "") -> dict[str, Any]:
    root = Path(root)
    files: dict[str, Any] = {}
    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.suffix not in {".f32", ".png"}:
            continue
        stats = collect_path_stats(path, root)
        files[stats["path"]] = stats
    return {
        "version": 1,
        "scene_id": scene_id or root.name,
        "root": str(root),
        "files": files,
    }


def write_baseline(baseline: dict[str, Any], path: Path | str) -> None:
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(baseline, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Collect validation output statistics.")
    parser.add_argument("--root", required=True, help="scene output directory")
    parser.add_argument("--scene-id", default="")
    parser.add_argument("--output", help="baseline JSON output path")
    args = parser.parse_args(argv)

    baseline = collect_output_tree(Path(args.root), scene_id=args.scene_id)
    if args.output:
        write_baseline(baseline, Path(args.output))
    else:
        print(json.dumps(baseline, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
