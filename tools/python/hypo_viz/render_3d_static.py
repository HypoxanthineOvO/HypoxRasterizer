from __future__ import annotations

import argparse
import json
import math
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

from PIL import Image, ImageDraw

from hypo_viz.scene3d import Bounds3D, Scene3D, Vector3, load_scene3d


SUPPORTED_BACKENDS = {"pillow_software"}


class Static3DBackendError(RuntimeError):
    pass


@dataclass(frozen=True)
class Static3DRenderResult:
    scene: str
    output: str
    backend: str
    object_count: int
    light_count: int
    vpl_sample_count: int
    warnings: list[str]


def _sub(a: Vector3, b: Vector3) -> Vector3:
    return [a[i] - b[i] for i in range(3)]


def _length(v: Vector3) -> float:
    return math.sqrt(sum(item * item for item in v))


def _normalize(v: Vector3) -> Vector3:
    length = _length(v)
    if length <= 1e-8:
        return [0.0, 0.0, 0.0]
    return [item / length for item in v]


def _collect_points(scene: Scene3D) -> list[Vector3]:
    points: list[Vector3] = [scene.camera.position, scene.camera.target]
    for obj in scene.objects:
        points.extend(obj.world_bounds.corners)
    for light in scene.lights:
        if light.position is not None:
            points.append(light.position)
    for buffer in scene.vpl_buffers:
        points.extend(sample.position for sample in buffer.samples)
    return points or [[0.0, 0.0, 0.0]]


def _scene_bounds(points: Iterable[Vector3]) -> Bounds3D:
    point_list = list(points)
    if not point_list:
        point_list = [[0.0, 0.0, 0.0]]
    minimum = [min(point[axis] for point in point_list) for axis in range(3)]
    maximum = [max(point[axis] for point in point_list) for axis in range(3)]
    for axis in range(3):
        if minimum[axis] == maximum[axis]:
            minimum[axis] -= 0.5
            maximum[axis] += 0.5
    return Bounds3D(minimum, maximum)


class _Projector:
    def __init__(self, bounds: Bounds3D, width: int, height: int) -> None:
        self.bounds = bounds
        self.width = width
        self.height = height
        span = max(bounds.size[0] + bounds.size[2] * 0.65, bounds.size[1] + bounds.size[2] * 0.35, 1e-6)
        self.scale = min(width, height) * 0.72 / span
        self.center = bounds.center

    def project(self, point: Vector3) -> tuple[int, int]:
        rel = _sub(point, self.center)
        x = rel[0] - rel[2] * 0.55
        y = rel[1] + rel[2] * 0.32
        px = int(round(self.width * 0.5 + x * self.scale))
        py = int(round(self.height * 0.56 - y * self.scale))
        return px, py


BOX_EDGES = [
    (0, 1),
    (0, 2),
    (0, 4),
    (3, 1),
    (3, 2),
    (3, 7),
    (5, 1),
    (5, 4),
    (5, 7),
    (6, 2),
    (6, 4),
    (6, 7),
]


def _draw_line(draw: ImageDraw.ImageDraw, projector: _Projector, a: Vector3, b: Vector3, fill, width: int = 1) -> None:
    draw.line((*projector.project(a), *projector.project(b)), fill=fill, width=width)


def _draw_bounds(draw: ImageDraw.ImageDraw, projector: _Projector, bounds: Bounds3D, color: tuple[int, int, int]) -> None:
    corners = bounds.corners
    for a, b in BOX_EDGES:
        _draw_line(draw, projector, corners[a], corners[b], color, width=2)


def _draw_camera(draw: ImageDraw.ImageDraw, projector: _Projector, scene: Scene3D) -> None:
    camera = scene.camera
    cam = camera.position
    target = camera.target
    _draw_line(draw, projector, cam, target, (255, 255, 255), width=2)
    cx, cy = projector.project(cam)
    draw.rectangle((cx - 4, cy - 4, cx + 4, cy + 4), outline=(255, 255, 255), fill=(32, 32, 32))
    draw.text((cx + 6, cy - 8), "camera", fill=(255, 255, 255))

    direction = _normalize(_sub(target, cam))
    if direction == [0.0, 0.0, 0.0]:
        return
    distance = max(_length(_sub(target, cam)) * 0.2, 0.2)
    for offset in ([-distance, -distance, distance], [distance, -distance, distance], [-distance, distance, distance], [distance, distance, distance]):
        corner = [target[i] + offset[i] * 0.25 for i in range(3)]
        _draw_line(draw, projector, cam, corner, (120, 120, 120))


def _draw_lights(draw: ImageDraw.ImageDraw, projector: _Projector, scene: Scene3D) -> None:
    for index, light in enumerate(scene.lights):
        color = (255, 220, 80) if light.type == "point" else (120, 220, 255) if light.type == "directional" else (255, 150, 60)
        origin = light.position if light.position is not None else scene.camera.target
        x, y = projector.project(origin)
        draw.ellipse((x - 5, y - 5, x + 5, y + 5), outline=color, fill=color)
        draw.text((x + 7, y - 8), f"{index}:{light.type}", fill=color)
        direction = light.direction or light.normal
        if direction:
            end = [origin[i] + direction[i] * 0.35 for i in range(3)]
            _draw_line(draw, projector, origin, end, color, width=2)
        if light.type == "area" and light.size:
            sx, sy = light.size[0] * 0.5, light.size[1] * 0.5
            rect = [
                [origin[0] - sx, origin[1], origin[2] - sy],
                [origin[0] + sx, origin[1], origin[2] - sy],
                [origin[0] + sx, origin[1], origin[2] + sy],
                [origin[0] - sx, origin[1], origin[2] + sy],
            ]
            for a, b in [(0, 1), (1, 2), (2, 3), (3, 0)]:
                _draw_line(draw, projector, rect[a], rect[b], color, width=2)


def _draw_vpls(draw: ImageDraw.ImageDraw, projector: _Projector, scene: Scene3D) -> None:
    palette = [(255, 64, 32), (64, 255, 180), (180, 120, 255), (255, 255, 80)]
    for buffer_index, buffer in enumerate(scene.vpl_buffers):
        color = palette[buffer_index % len(palette)]
        for sample in buffer.samples:
            x, y = projector.project(sample.position)
            draw.ellipse((x - 2, y - 2, x + 2, y + 2), fill=color)


def _render_with_pillow(scene: Scene3D, output: Path, image_size: tuple[int, int]) -> None:
    width, height = image_size
    image = Image.new("RGB", (width, height), (18, 20, 24))
    draw = ImageDraw.Draw(image)
    projector = _Projector(_scene_bounds(_collect_points(scene)), width, height)

    for axis, color in [([1.0, 0.0, 0.0], (180, 70, 70)), ([0.0, 1.0, 0.0], (70, 180, 90)), ([0.0, 0.0, 1.0], (80, 120, 220))]:
        _draw_line(draw, projector, [0.0, 0.0, 0.0], axis, color)

    for obj in scene.objects:
        color = (120, 170, 255) if obj.mesh.exists else (255, 80, 120)
        _draw_bounds(draw, projector, obj.world_bounds, color)
        cx, cy = projector.project(obj.world_bounds.center)
        draw.text((cx + 3, cy + 3), obj.name[:18], fill=color)

    _draw_lights(draw, projector, scene)
    _draw_vpls(draw, projector, scene)
    _draw_camera(draw, projector, scene)
    draw.text((8, 8), scene.name, fill=(235, 235, 235))

    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)


def render_static_preview(
    config: Path | str,
    output: Path | str,
    *,
    scene_output_root: Path | str | None = None,
    report_json: Path | str | None = None,
    bounds_only: bool = False,
    max_faces: int | None = None,
    backend: str = "pillow_software",
    image_size: tuple[int, int] = (960, 720),
) -> Static3DRenderResult:
    if backend not in SUPPORTED_BACKENDS:
        raise Static3DBackendError(f"static 3D backend is unavailable: {backend}")

    mesh_mode = "bounds_only" if bounds_only else "metadata_only"
    scene = load_scene3d(config, output_root=scene_output_root, mesh_mode=mesh_mode)
    warnings = list(scene.warnings)
    if max_faces is not None:
        for obj in scene.objects:
            if obj.mesh.face_count > max_faces:
                warnings.append(f"{obj.name}: {obj.mesh.face_count} faces exceeds max_faces={max_faces}; drawing bounds")

    output_path = Path(output)
    _render_with_pillow(scene, output_path, image_size)
    result = Static3DRenderResult(
        scene=scene.name,
        output=str(output_path),
        backend=backend,
        object_count=len(scene.objects),
        light_count=len(scene.lights),
        vpl_sample_count=sum(len(buffer.samples) for buffer in scene.vpl_buffers),
        warnings=warnings,
    )
    if report_json is not None:
        report_path = Path(report_json)
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(asdict(result), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return result


def _parse_size(raw: str) -> tuple[int, int]:
    if "x" not in raw:
        raise argparse.ArgumentTypeError("size must be WIDTHxHEIGHT")
    width, height = raw.lower().split("x", 1)
    return int(width), int(height)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Render a headless static 3D scene preview.")
    parser.add_argument("--config", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--scene-output-root")
    parser.add_argument("--report-json")
    parser.add_argument("--bounds-only", action="store_true")
    parser.add_argument("--max-faces", type=int)
    parser.add_argument("--backend", default="pillow_software")
    parser.add_argument("--size", type=_parse_size, default=(960, 720))
    args = parser.parse_args(argv)

    result = render_static_preview(
        args.config,
        args.output,
        scene_output_root=args.scene_output_root,
        report_json=args.report_json,
        bounds_only=args.bounds_only,
        max_faces=args.max_faces,
        backend=args.backend,
        image_size=args.size,
    )
    for warning in result.warnings:
        print(f"warning: {warning}")
    print(json.dumps(asdict(result), sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
