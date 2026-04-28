from __future__ import annotations

import argparse
import math
from pathlib import Path

from PIL import Image, ImageDraw

from hypo_validation.collect_stats import FloatBufferData, read_f32_buffer


VPL_COLOR = (255, 64, 32)


def visualization_output_path(scene_root: Path | str, relative_path: str, mode: str) -> Path:
    stem = Path(relative_path).with_suffix("").as_posix().replace("/", "_")
    return Path(scene_root) / "viz" / f"{stem}_{mode}.png"


def _finite_range(values: list[float]) -> tuple[float, float]:
    finite = [value for value in values if math.isfinite(value)]
    if not finite:
        return 0.0, 1.0
    min_value = min(finite)
    max_value = max(finite)
    if min_value == max_value:
        return min_value, min_value + 1.0
    return min_value, max_value


def _heat_color(value: float, min_value: float, max_value: float) -> tuple[int, int, int]:
    if not math.isfinite(value):
        return (255, 0, 255)
    t = max(0.0, min(1.0, (value - min_value) / (max_value - min_value)))
    r = int(255 * t)
    g = int(255 * (1.0 - abs(t - 0.5) * 2.0))
    b = int(255 * (1.0 - t))
    return (r, g, b)


def _channel_values(buffer: FloatBufferData, channel: int) -> list[float]:
    if channel < 0 or channel >= buffer.channels:
        raise ValueError(f"channel {channel} out of range for {buffer.channels} channels")
    values: list[float] = []
    for index in range(buffer.width * buffer.height):
        values.append(buffer.values[index * buffer.channels + channel])
    return values


def render_f32_heatmap(source: Path | str, output: Path | str, channel: int = 0) -> list[str]:
    source = Path(source)
    output = Path(output)
    buffer = read_f32_buffer(source)
    values = _channel_values(buffer, channel)
    min_value, max_value = _finite_range(values)

    image = Image.new("RGB", (buffer.width, buffer.height))
    pixels = image.load()
    for y in range(buffer.height):
        for x in range(buffer.width):
            pixels[x, y] = _heat_color(values[y * buffer.width + x], min_value, max_value)

    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)
    return []


def _sample_points(buffer: FloatBufferData) -> list[tuple[float, float]]:
    if buffer.channels < 2:
        return []
    count = buffer.width * buffer.height
    points: list[tuple[float, float]] = []
    for index in range(count):
        x = buffer.values[index * buffer.channels]
        y = buffer.values[index * buffer.channels + 1]
        if math.isfinite(x) and math.isfinite(y):
            points.append((x, y))
    return points


def _normalize_points(points: list[tuple[float, float]], width: int, height: int) -> list[tuple[int, int]]:
    if not points:
        return []
    xs = [point[0] for point in points]
    ys = [point[1] for point in points]
    if min(xs) >= 0.0 and max(xs) <= 1.0 and min(ys) >= 0.0 and max(ys) <= 1.0:
        min_x, max_x = 0.0, 1.0
        min_y, max_y = 0.0, 1.0
    else:
        min_x, max_x = _finite_range(xs)
        min_y, max_y = _finite_range(ys)
    result: list[tuple[int, int]] = []
    for x, y in points:
        px = int(round((x - min_x) / (max_x - min_x) * (width - 1)))
        py = int(round((1.0 - (y - min_y) / (max_y - min_y)) * (height - 1)))
        result.append((max(0, min(width - 1, px)), max(0, min(height - 1, py))))
    return result


def overlay_vpl_samples(base_png: Path | str, vpl_f32: Path | str, output: Path | str) -> list[str]:
    base_png = Path(base_png)
    vpl_f32 = Path(vpl_f32)
    output = Path(output)
    warnings: list[str] = []
    if not base_png.exists():
        return [f"missing base image: {base_png}"]
    if not vpl_f32.exists():
        return [f"missing VPL buffer: {vpl_f32}"]

    image = Image.open(base_png).convert("RGB")
    buffer = read_f32_buffer(vpl_f32)
    points = _normalize_points(_sample_points(buffer), image.width, image.height)
    if not points:
        warnings.append(f"no VPL points found in {vpl_f32}")

    draw = ImageDraw.Draw(image)
    for x, y in points:
        draw.ellipse((x - 2, y - 2, x + 2, y + 2), fill=VPL_COLOR)

    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)
    return warnings


def _label_for_shadow(path: Path) -> str:
    name = path.stem
    marker = "face_"
    if marker in name:
        return name[name.index(marker) + len(marker):]
    return name


def create_shadow_contact_sheet(shadow_dir: Path | str, output: Path | str) -> list[str]:
    shadow_dir = Path(shadow_dir)
    output = Path(output)
    if not shadow_dir.is_dir():
        return [f"missing shadow directory: {shadow_dir}"]

    images = sorted(shadow_dir.glob("*.png"))
    if not images:
        return [f"no shadow PNG files found in {shadow_dir}"]

    opened = [Image.open(path).convert("RGB") for path in images]
    tile_w, tile_h = opened[0].size
    label_h = max(8, tile_h // 4)
    cols = 3 if len(opened) >= 6 else max(1, math.ceil(math.sqrt(len(opened))))
    rows = math.ceil(len(opened) / cols)
    sheet = Image.new("RGB", (cols * tile_w, rows * (tile_h + label_h)), (0, 0, 0))
    draw = ImageDraw.Draw(sheet)

    for index, (path, image) in enumerate(zip(images, opened)):
        if image.size != (tile_w, tile_h):
            image = image.resize((tile_w, tile_h), Image.Resampling.BILINEAR)
        col = index % cols
        row = index // cols
        x = col * tile_w
        y = row * (tile_h + label_h)
        sheet.paste(image, (x, y))
        draw.rectangle((x, y + tile_h, x + tile_w, y + tile_h + label_h), fill=(16, 16, 16))
        draw.text((x + 1, y + tile_h), _label_for_shadow(path)[:8], fill=(255, 255, 255))

    output.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output)
    return []


def render_scene_visuals(scene_root: Path | str, output_dir: Path | str | None = None) -> list[str]:
    scene_root = Path(scene_root)
    viz_root = Path(output_dir) if output_dir else scene_root / "viz"
    warnings: list[str] = []
    for f32_path in sorted(scene_root.rglob("*.f32")):
        rel = f32_path.relative_to(scene_root).as_posix()
        warnings.extend(render_f32_heatmap(f32_path, viz_root / f"{Path(rel).with_suffix('').as_posix().replace('/', '_')}_heatmap.png"))

    shadow_dir = scene_root / "shadow"
    warnings.extend(create_shadow_contact_sheet(shadow_dir, viz_root / "shadow_contact_sheet.png"))

    for vpl_path in sorted(shadow_dir.glob("light_*_*vpls.f32")):
        candidates = sorted(shadow_dir.glob("*ShadowMap*.png"))
        if not candidates:
            warnings.append(f"missing shadow map base for {vpl_path}")
            continue
        warnings.extend(
            overlay_vpl_samples(
                candidates[0],
                vpl_path,
                viz_root / f"{vpl_path.with_suffix('').name}_annotated.png",
            )
        )
    return warnings


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Generate 2D visualization PNGs for validation outputs.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    heatmap = subparsers.add_parser("heatmap")
    heatmap.add_argument("--input", required=True)
    heatmap.add_argument("--output", required=True)
    heatmap.add_argument("--channel", type=int, default=0)

    overlay = subparsers.add_parser("vpl-overlay")
    overlay.add_argument("--base", required=True)
    overlay.add_argument("--vpl", required=True)
    overlay.add_argument("--output", required=True)

    sheet = subparsers.add_parser("shadow-sheet")
    sheet.add_argument("--shadow-dir", required=True)
    sheet.add_argument("--output", required=True)

    scene = subparsers.add_parser("scene")
    scene.add_argument("--root", required=True)
    scene.add_argument("--output-dir")

    args = parser.parse_args(argv)
    if args.command == "heatmap":
        warnings = render_f32_heatmap(args.input, args.output, args.channel)
    elif args.command == "vpl-overlay":
        warnings = overlay_vpl_samples(args.base, args.vpl, args.output)
    elif args.command == "shadow-sheet":
        warnings = create_shadow_contact_sheet(args.shadow_dir, args.output)
    else:
        warnings = render_scene_visuals(args.root, args.output_dir)

    for warning in warnings:
        print(f"warning: {warning}")
    print(f"summary: warnings={len(warnings)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
