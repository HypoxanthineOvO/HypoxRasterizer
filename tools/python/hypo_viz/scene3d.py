from __future__ import annotations

import json
import math
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Iterable

from hypo_validation.collect_stats import read_f32_buffer


Vector3 = list[float]


@dataclass(frozen=True)
class Bounds3D:
    minimum: Vector3
    maximum: Vector3

    @property
    def center(self) -> Vector3:
        return [(self.minimum[i] + self.maximum[i]) * 0.5 for i in range(3)]

    @property
    def size(self) -> Vector3:
        return [self.maximum[i] - self.minimum[i] for i in range(3)]

    @property
    def corners(self) -> list[Vector3]:
        return [
            [x, y, z]
            for x in (self.minimum[0], self.maximum[0])
            for y in (self.minimum[1], self.maximum[1])
            for z in (self.minimum[2], self.maximum[2])
        ]


@dataclass(frozen=True)
class MeshMetadata:
    path: Path
    exists: bool
    vertex_count: int = 0
    face_count: int = 0
    local_bounds: Bounds3D | None = None
    mode: str = "metadata_only"
    vertices: list[Vector3] = field(default_factory=list)


@dataclass(frozen=True)
class Object3D:
    name: str
    mesh: MeshMetadata
    material_id: str
    translation: Vector3
    rotation: Vector3
    scale: Vector3
    world_bounds: Bounds3D


@dataclass(frozen=True)
class Camera3D:
    resolution: list[int]
    position: Vector3
    target: Vector3
    fov: float
    focal_length: float | None = None


@dataclass(frozen=True)
class Light3D:
    name: str
    type: str
    position: Vector3 | None = None
    direction: Vector3 | None = None
    normal: Vector3 | None = None
    intensity: Vector3 = field(default_factory=lambda: [1.0, 1.0, 1.0])
    size: list[float] | None = None
    vpl_samples: int = 0
    vpl_seed: int | None = None
    vpl_stochastic: bool = False
    shadow_enabled: bool = True


@dataclass(frozen=True)
class VPLSample3D:
    position: Vector3
    normal: Vector3 | None = None
    flux: Vector3 | None = None
    source_light_index: int | None = None


@dataclass(frozen=True)
class VPLBuffer3D:
    path: Path
    name: str
    light_index: int | None
    samples: list[VPLSample3D]
    shape: list[int]


@dataclass(frozen=True)
class Scene3D:
    name: str
    config_path: Path
    repo_root: Path
    camera: Camera3D
    materials: dict[str, dict[str, Any]]
    objects: list[Object3D]
    lights: list[Light3D]
    vpl_buffers: list[VPLBuffer3D] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)


def _as_float_list(value: Any, length: int, default: Iterable[float]) -> list[float]:
    if not isinstance(value, list):
        return [float(item) for item in default]
    result = [float(item) for item in value[:length]]
    while len(result) < length:
        result.append(float(list(default)[len(result)]))
    return result


def _as_int_list(value: Any, length: int, default: Iterable[int]) -> list[int]:
    if not isinstance(value, list):
        return [int(item) for item in default]
    result = [int(item) for item in value[:length]]
    default_list = list(default)
    while len(result) < length:
        result.append(int(default_list[len(result)]))
    return result


def resolve_repo_root(start: Path | str) -> Path:
    path = Path(start).resolve()
    candidates = [path] if path.is_dir() else [path.parent]
    candidates.extend(candidates[0].parents)
    for candidate in candidates:
        if (candidate / "configs").is_dir() and (candidate / "assets").is_dir():
            return candidate
    return Path.cwd().resolve()


def resolve_scene_path(raw_path: str, config_path: Path, repo_root: Path) -> Path:
    path = Path(raw_path)
    if path.is_absolute():
        return path
    if raw_path.startswith("./assets/") or raw_path.startswith("assets/"):
        return (repo_root / raw_path.replace("./", "", 1)).resolve()
    config_relative = (config_path.parent / path).resolve()
    if config_relative.exists():
        return config_relative
    return (repo_root / path).resolve()


def _empty_bounds() -> Bounds3D:
    return Bounds3D([0.0, 0.0, 0.0], [0.0, 0.0, 0.0])


def load_obj_metadata(path: Path | str, mode: str = "metadata_only") -> MeshMetadata:
    path = Path(path)
    if not path.exists():
        return MeshMetadata(path=path, exists=False, mode=mode)

    vertex_count = 0
    face_count = 0
    minimum = [math.inf, math.inf, math.inf]
    maximum = [-math.inf, -math.inf, -math.inf]
    keep_vertices = mode == "full"
    vertices: list[Vector3] = []

    with path.open("r", encoding="utf-8", errors="ignore") as handle:
        for line in handle:
            if line.startswith("v "):
                parts = line.split()
                if len(parts) < 4:
                    continue
                try:
                    vertex = [float(parts[1]), float(parts[2]), float(parts[3])]
                except ValueError:
                    continue
                vertex_count += 1
                for axis in range(3):
                    minimum[axis] = min(minimum[axis], vertex[axis])
                    maximum[axis] = max(maximum[axis], vertex[axis])
                if keep_vertices:
                    vertices.append(vertex)
            elif line.startswith("f "):
                face_count += 1

    bounds = Bounds3D(minimum, maximum) if vertex_count else _empty_bounds()
    return MeshMetadata(
        path=path,
        exists=True,
        vertex_count=vertex_count,
        face_count=face_count,
        local_bounds=bounds,
        mode=mode,
        vertices=vertices,
    )


def _rotation_matrix_xyz(rotation_degrees: Vector3) -> list[list[float]]:
    rx, ry, rz = [math.radians(value) for value in rotation_degrees]
    sx, cx = math.sin(rx), math.cos(rx)
    sy, cy = math.sin(ry), math.cos(ry)
    sz, cz = math.sin(rz), math.cos(rz)
    mx = [[1, 0, 0], [0, cx, -sx], [0, sx, cx]]
    my = [[cy, 0, sy], [0, 1, 0], [-sy, 0, cy]]
    mz = [[cz, -sz, 0], [sz, cz, 0], [0, 0, 1]]
    return _matmul3(mz, _matmul3(my, mx))


def _matmul3(a: list[list[float]], b: list[list[float]]) -> list[list[float]]:
    return [
        [sum(a[row][k] * b[k][col] for k in range(3)) for col in range(3)]
        for row in range(3)
    ]


def _transform_point(point: Vector3, translation: Vector3, rotation: Vector3, scale: Vector3) -> Vector3:
    scaled = [point[i] * scale[i] for i in range(3)]
    matrix = _rotation_matrix_xyz(rotation)
    rotated = [sum(matrix[row][col] * scaled[col] for col in range(3)) for row in range(3)]
    return [rotated[i] + translation[i] for i in range(3)]


def transform_bounds(bounds: Bounds3D | None, translation: Vector3, rotation: Vector3, scale: Vector3) -> Bounds3D:
    if bounds is None:
        return _empty_bounds()
    corners = [_transform_point(corner, translation, rotation, scale) for corner in bounds.corners]
    return Bounds3D(
        [min(corner[axis] for corner in corners) for axis in range(3)],
        [max(corner[axis] for corner in corners) for axis in range(3)],
    )


def _parse_light(raw: dict[str, Any], index: int) -> Light3D:
    vpl = raw.get("vpl") if isinstance(raw.get("vpl"), dict) else {}
    shadow = raw.get("shadow") if isinstance(raw.get("shadow"), dict) else {}
    return Light3D(
        name=str(raw.get("name", f"light_{index}")),
        type=str(raw.get("type", "point")),
        position=_as_float_list(raw.get("position"), 3, [0.0, 0.0, 0.0]) if "position" in raw else None,
        direction=_as_float_list(raw.get("direction"), 3, [0.0, -1.0, 0.0]) if "direction" in raw else None,
        normal=_as_float_list(raw.get("normal"), 3, [0.0, -1.0, 0.0]) if "normal" in raw else None,
        intensity=_as_float_list(raw.get("intensity"), 3, [1.0, 1.0, 1.0]),
        size=_as_float_list(raw.get("size"), 2, [1.0, 1.0]) if "size" in raw else None,
        vpl_samples=int(vpl.get("samples", 0)),
        vpl_seed=int(vpl["seed"]) if "seed" in vpl else None,
        vpl_stochastic=bool(vpl.get("stochastic", False)),
        shadow_enabled=bool(shadow.get("enabled", True)),
    )


def _parse_light_index(path: Path) -> int | None:
    parts = path.stem.split("_")
    for index, part in enumerate(parts):
        if part == "light" and index + 1 < len(parts):
            try:
                return int(parts[index + 1])
            except ValueError:
                return None
    return None


def load_vpl_sample_buffer(path: Path | str) -> list[VPLSample3D]:
    buffer = read_f32_buffer(path)
    samples: list[VPLSample3D] = []
    count = buffer.width * buffer.height
    for index in range(count):
        offset = index * buffer.channels
        if buffer.channels < 3:
            continue
        position = [float(buffer.values[offset]), float(buffer.values[offset + 1]), float(buffer.values[offset + 2])]
        normal = None
        if buffer.channels >= 6:
            normal = [
                float(buffer.values[offset + 3]),
                float(buffer.values[offset + 4]),
                float(buffer.values[offset + 5]),
            ]
        flux = None
        if buffer.channels >= 9:
            flux = [
                float(buffer.values[offset + 6]),
                float(buffer.values[offset + 7]),
                float(buffer.values[offset + 8]),
            ]
        samples.append(VPLSample3D(position=position, normal=normal, flux=flux))
    return samples


def load_vpl_buffers(output_root: Path | str | None) -> list[VPLBuffer3D]:
    if output_root is None:
        return []
    root = Path(output_root)
    shadow_dir = root / "shadow"
    if not shadow_dir.is_dir():
        return []
    result: list[VPLBuffer3D] = []
    for path in sorted(shadow_dir.glob("light_*_*vpls.f32")):
        buffer = read_f32_buffer(path)
        result.append(
            VPLBuffer3D(
                path=path,
                name=buffer.name,
                light_index=_parse_light_index(path),
                samples=load_vpl_sample_buffer(path),
                shape=buffer.shape,
            )
        )
    return result


def load_scene3d(
    config_path: Path | str,
    repo_root: Path | str | None = None,
    output_root: Path | str | None = None,
    mesh_mode: str = "metadata_only",
) -> Scene3D:
    config_path = Path(config_path).resolve()
    repo = Path(repo_root).resolve() if repo_root is not None else resolve_repo_root(config_path)
    data = json.loads(config_path.read_text(encoding="utf-8"))
    warnings: list[str] = []

    camera_raw = data.get("camera", {})
    camera = Camera3D(
        resolution=_as_int_list(camera_raw.get("resolution"), 2, [512, 512]),
        position=_as_float_list(camera_raw.get("position"), 3, [0.0, 0.0, 1.0]),
        target=_as_float_list(camera_raw.get("target"), 3, [0.0, 0.0, 0.0]),
        fov=float(camera_raw.get("fov", 45.0)),
        focal_length=float(camera_raw["focalLength"]) if "focalLength" in camera_raw else None,
    )
    materials = {
        str(item.get("id", f"material_{index}")): dict(item)
        for index, item in enumerate(data.get("materials", []))
        if isinstance(item, dict)
    }

    objects: list[Object3D] = []
    for index, raw in enumerate(data.get("objects", [])):
        if not isinstance(raw, dict):
            continue
        mesh_path = resolve_scene_path(str(raw.get("mesh", "")), config_path, repo)
        mesh = load_obj_metadata(mesh_path, mode=mesh_mode)
        if not mesh.exists:
            warnings.append(f"missing mesh for object {raw.get('name', index)}: {mesh_path}")
        translation = _as_float_list(raw.get("translation"), 3, [0.0, 0.0, 0.0])
        rotation = _as_float_list(raw.get("rotation"), 3, [0.0, 0.0, 0.0])
        scale = _as_float_list(raw.get("scale"), 3, [1.0, 1.0, 1.0])
        objects.append(
            Object3D(
                name=str(raw.get("name", f"object_{index}")),
                mesh=mesh,
                material_id=str(raw.get("material", "")),
                translation=translation,
                rotation=rotation,
                scale=scale,
                world_bounds=transform_bounds(mesh.local_bounds, translation, rotation, scale),
            )
        )

    lights = [_parse_light(raw, index) for index, raw in enumerate(data.get("lights", [])) if isinstance(raw, dict)]
    scene_raw = data.get("scene", {}) if isinstance(data.get("scene"), dict) else {}
    return Scene3D(
        name=str(scene_raw.get("name", config_path.stem)),
        config_path=config_path,
        repo_root=repo,
        camera=camera,
        materials=materials,
        objects=objects,
        lights=lights,
        vpl_buffers=load_vpl_buffers(output_root),
        warnings=warnings,
    )
