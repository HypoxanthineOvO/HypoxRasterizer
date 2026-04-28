from __future__ import annotations

import argparse
import json
import math
from datetime import datetime
from pathlib import Path
from typing import Any
from zoneinfo import ZoneInfo

from PIL import Image, ImageDraw

from hypo_validation.collect_stats import read_f32_buffer
from hypo_viz.render_2d import render_scene_visuals
from hypo_viz.render_3d_static import render_static_preview


FACE_DIRECTIONS = {
    "pos_x": [1, 0, 0],
    "neg_x": [-1, 0, 0],
    "pos_y": [0, 1, 0],
    "neg_y": [0, -1, 0],
    "pos_z": [0, 0, 1],
    "neg_z": [0, 0, -1],
}


def load_world_position_points(path: Path | str, stride: int = 4, max_points: int | None = None) -> list[list[float]]:
    buffer = read_f32_buffer(path)
    if buffer.channels < 3:
        raise ValueError(f"{path}: world_position buffer needs at least 3 channels")
    stride = max(1, int(stride))
    points: list[list[float]] = []
    for y in range(0, buffer.height, stride):
        for x in range(0, buffer.width, stride):
            offset = (y * buffer.width + x) * buffer.channels
            point = [
                float(buffer.values[offset]),
                float(buffer.values[offset + 1]),
                float(buffer.values[offset + 2]),
            ]
            if all(math.isfinite(value) for value in point):
                points.append(point)
                if max_points is not None and len(points) >= max_points:
                    return points
    return points


def _range(values: list[float]) -> tuple[float, float]:
    if not values:
        return 0.0, 1.0
    min_value = min(values)
    max_value = max(values)
    if min_value == max_value:
        return min_value - 0.5, max_value + 0.5
    return min_value, max_value


def write_point_cloud_preview(
    points: list[list[float]],
    output: Path | str,
    image_size: tuple[int, int] = (960, 720),
) -> int:
    output = Path(output)
    width, height = image_size
    image = Image.new("RGB", (width, height), (14, 16, 20))
    draw = ImageDraw.Draw(image)
    if points:
        xs = [point[0] for point in points]
        ys = [point[1] for point in points]
        zs = [point[2] for point in points]
        min_x, max_x = _range(xs)
        min_z, max_z = _range(zs)
        min_y, max_y = _range(ys)
        for x, y, z in points:
            px = int((x - min_x) / (max_x - min_x) * (width - 1))
            py = int((1.0 - (z - min_z) / (max_z - min_z)) * (height - 1))
            t = (y - min_y) / (max_y - min_y)
            color = (int(255 * t), 180, int(255 * (1.0 - t)))
            draw.ellipse((px - 1, py - 1, px + 1, py + 1), fill=color)
    draw.text((8, 8), f"world_position points: {len(points)}", fill=(240, 240, 240))
    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)
    return len(points)


def _face_label(path: Path) -> str | None:
    for face in FACE_DIRECTIONS:
        if f"face_{face}" in path.stem:
            return face
    return None


def shadow_face_metadata(shadow_dir: Path | str) -> list[dict[str, Any]]:
    shadow_dir = Path(shadow_dir)
    if not shadow_dir.is_dir():
        return []
    entries: list[dict[str, Any]] = []
    for path in sorted(shadow_dir.glob("*.png")):
        face = _face_label(path)
        entries.append(
            {
                "path": path.as_posix(),
                "name": path.stem,
                "face": face or "single",
                "direction": FACE_DIRECTIONS.get(face),
            }
        )
    return entries


def vpl_buffer_counts(shadow_dir: Path | str) -> dict[str, int]:
    shadow_dir = Path(shadow_dir)
    if not shadow_dir.is_dir():
        return {"direct": 0, "indirect": 0}
    return {
        "direct": len(list(shadow_dir.glob("light_*_direct_vpls.f32"))),
        "indirect": len(list(shadow_dir.glob("light_*_indirect_vpls.f32"))),
    }


def _write_markdown_report(result: dict[str, Any], output: Path) -> None:
    lines = [
        f"# 3D Visualization Report - {result['scene']}",
        "",
        f"Generated: {result['generated_at']}",
        "",
        "## Artifacts",
        "",
        f"- Static 3D: `{result['artifacts']['static_3d_png']}`",
        f"- Static 3D JSON: `{result['artifacts']['static_3d_json']}`",
        f"- Point cloud preview: `{result['artifacts']['point_cloud_png']}`",
        f"- 2D visualization directory: `{result['artifacts']['viz2d_dir']}`",
        "",
        "## Counts",
        "",
        f"- World-position points: `{result['point_cloud']['points']}`",
        f"- Shadow entries: `{len(result['shadow_faces'])}`",
        f"- Warnings: `{len(result['warnings'])}`",
    ]
    if result["warnings"]:
        lines.extend(["", "## Warnings", ""])
        lines.extend(f"- {warning}" for warning in result["warnings"])
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def generate_scene_3d_report(
    config: Path | str,
    scene_output_root: Path | str,
    output_dir: Path | str,
    *,
    point_stride: int = 8,
    max_points: int | None = 20000,
) -> dict[str, Any]:
    scene_output_root = Path(scene_output_root)
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    warnings: list[str] = []
    viz2d_dir = output_dir / "viz2d"
    warnings.extend(render_scene_visuals(scene_output_root, viz2d_dir))

    static_png = output_dir / "scene3d.png"
    static_json = output_dir / "scene3d.json"
    static_result = render_static_preview(
        config,
        static_png,
        scene_output_root=scene_output_root,
        report_json=static_json,
    )
    warnings.extend(static_result.warnings)

    world_position = scene_output_root / "gbuffer" / "world_position.f32"
    point_cloud_png = output_dir / "world_position_point_cloud.png"
    point_count = 0
    if world_position.exists():
        points = load_world_position_points(world_position, stride=point_stride, max_points=max_points)
        point_count = write_point_cloud_preview(points, point_cloud_png)
    else:
        warnings.append(f"missing world_position buffer: {world_position}")

    result: dict[str, Any] = {
        "scene": static_result.scene,
        "generated_at": datetime.now(ZoneInfo("Asia/Shanghai")).strftime("%Y-%m-%d %H:%M（上海时间）"),
        "scene_output_root": scene_output_root.as_posix(),
        "static_3d": {
            "scene": static_result.scene,
            "backend": static_result.backend,
            "object_count": static_result.object_count,
            "light_count": static_result.light_count,
            "vpl_sample_count": static_result.vpl_sample_count,
        },
        "point_cloud": {
            "source": world_position.as_posix(),
            "points": point_count,
            "stride": point_stride,
        },
        "shadow_faces": shadow_face_metadata(scene_output_root / "shadow"),
        "vpl_buffers": vpl_buffer_counts(scene_output_root / "shadow"),
        "artifacts": {
            "static_3d_png": static_png.as_posix(),
            "static_3d_json": static_json.as_posix(),
            "point_cloud_png": point_cloud_png.as_posix(),
            "viz2d_dir": viz2d_dir.as_posix(),
        },
        "warnings": warnings,
    }
    (output_dir / "report.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    _write_markdown_report(result, output_dir / "report.md")
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Generate a combined 2D/3D visualization report.")
    parser.add_argument("--config", required=True)
    parser.add_argument("--scene-output-root", required=True)
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--point-stride", type=int, default=8)
    parser.add_argument("--max-points", type=int, default=20000)
    args = parser.parse_args(argv)

    result = generate_scene_3d_report(
        args.config,
        args.scene_output_root,
        args.output_dir,
        point_stride=args.point_stride,
        max_points=args.max_points,
    )
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
