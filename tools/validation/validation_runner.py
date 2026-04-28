#!/usr/bin/env python3
"""Lightweight validation scene runner foundation.

This script intentionally uses only the Python standard library in M7. Later
milestones will move richer validation and visualization code into the shared
uv-managed tools project.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import shlex
import subprocess
from dataclasses import dataclass, field


@dataclass
class Scene:
    scene_id: str = ""
    tier: str = "fast"
    config: str = ""
    run_by_default: bool = False
    tags: list[str] = field(default_factory=list)
    assets: list[str] = field(default_factory=list)
    expected_outputs: list[str] = field(default_factory=list)
    source_type: str = "internal"
    license: str = ""


@dataclass
class RunResult:
    scene_id: str
    status: str
    message: str = ""


def _strip_comment(line: str) -> str:
    in_quote = False
    quote = ""
    for idx, char in enumerate(line):
        if char in {"'", '"'}:
            if not in_quote:
                in_quote = True
                quote = char
            elif quote == char:
                in_quote = False
        if char == "#" and not in_quote:
            return line[:idx]
    return line


def _value(raw: str) -> str:
    value = raw.strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in {"'", '"'}:
        return value[1:-1]
    return value


def _list(raw: str) -> list[str]:
    value = raw.strip()
    if value.startswith("[") and value.endswith("]"):
        value = value[1:-1]
    if not value:
        return []
    return [_value(item) for item in value.split(",") if _value(item)]


def _bool(raw: str) -> bool:
    return raw.strip().lower() in {"true", "yes", "1"}


def load_scenes(path: pathlib.Path) -> list[Scene]:
    scenes: list[Scene] = []
    section = ""
    current: Scene | None = None
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = _strip_comment(raw).strip()
        if not line:
            continue
        if line == "scenes:":
            section = "scenes"
            continue
        if section != "scenes":
            continue
        if line.startswith("- "):
            current = Scene()
            scenes.append(current)
            line = line[2:].strip()
            if not line:
                continue
        if current is None or ":" not in line:
            continue
        key, raw_value = [part.strip() for part in line.split(":", 1)]
        if key == "id":
            current.scene_id = _value(raw_value)
        elif key == "tier":
            current.tier = _value(raw_value)
        elif key == "config":
            current.config = _value(raw_value)
        elif key == "default":
            current.run_by_default = _bool(raw_value)
        elif key == "tags":
            current.tags = _list(raw_value)
        elif key == "assets":
            current.assets = _list(raw_value)
        elif key == "expected_outputs":
            current.expected_outputs = _list(raw_value)
        elif key == "source_type":
            current.source_type = _value(raw_value)
        elif key == "license":
            current.license = _value(raw_value)
    return scenes


def select_scenes(scenes: list[Scene], args: argparse.Namespace) -> list[Scene]:
    default_only = args.default or not (args.tier or args.tag or args.all)
    result: list[Scene] = []
    for scene in scenes:
        if default_only and not scene.run_by_default:
            continue
        if default_only and scene.tier == "heavy" and not args.include_heavy:
            continue
        if args.tier and scene.tier != args.tier:
            continue
        if args.tag and args.tag not in scene.tags:
            continue
        result.append(scene)
    return result


def render_command(scene: Scene, args: argparse.Namespace) -> list[str]:
    command = [
        args.renderer,
        "--config",
        scene.config,
        "--output-root",
        str(pathlib.Path(args.output_root) / scene.tier),
        "--buffers",
        ",".join(scene.expected_outputs or ["color", "gbuffer", "shadow"]),
    ]
    if args.resolution:
        command.extend(["--resolution", args.resolution])
    return command


def scene_output_name(scene: Scene) -> str:
    try:
        data = json.loads(pathlib.Path(scene.config).read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return pathlib.Path(scene.config).stem.replace(".v2", "")

    name = data.get("scene", {}).get("name")
    if isinstance(name, str) and name:
        return name
    return pathlib.Path(scene.config).stem.replace(".v2", "")


def verify_outputs(scene: Scene, args: argparse.Namespace) -> list[str]:
    scene_dir = pathlib.Path(args.output_root) / scene.tier / scene_output_name(scene)
    missing: list[str] = []
    for output in scene.expected_outputs:
        if output == "color":
            if not (scene_dir / "color.png").is_file():
                missing.append("color.png")
        elif output == "gbuffer":
            gbuffer_dir = scene_dir / "gbuffer"
            if not gbuffer_dir.is_dir() or not any(gbuffer_dir.iterdir()):
                missing.append("gbuffer/")
        elif output == "shadow":
            shadow_dir = scene_dir / "shadow"
            if not shadow_dir.is_dir() or not any(shadow_dir.iterdir()):
                missing.append("shadow/")
    return missing


def missing_references(scene: Scene) -> list[str]:
    missing: list[str] = []
    if not pathlib.Path(scene.config).exists():
        missing.append(scene.config)
    for asset in scene.assets:
        if not pathlib.Path(asset).exists():
            missing.append(asset)
    return missing


def should_skip_missing(scene: Scene) -> bool:
    return scene.tier == "heavy" or scene.source_type == "external" or not scene.run_by_default


def run_scene(scene: Scene, args: argparse.Namespace) -> RunResult:
    command = render_command(scene, args)
    print(f"{scene.scene_id}: {' '.join(shlex.quote(part) for part in command)}", flush=True)
    if not args.render:
        return RunResult(scene.scene_id, "dry-run")

    missing = missing_references(scene)
    if missing:
        message = "missing references: " + ", ".join(missing)
        if should_skip_missing(scene):
            return RunResult(scene.scene_id, "skipped", message)
        return RunResult(scene.scene_id, "failed", message)

    completed = subprocess.run(command, check=False)
    if completed.returncode != 0:
        return RunResult(scene.scene_id, "failed", f"renderer exit code {completed.returncode}")

    missing = verify_outputs(scene, args)
    if missing:
        return RunResult(scene.scene_id, "failed", "missing outputs: " + ", ".join(missing))
    return RunResult(scene.scene_id, "passed")


def main() -> int:
    parser = argparse.ArgumentParser(description="Enumerate or run validation scenes.")
    parser.add_argument("--manifest", default="validation/scenes.yaml")
    parser.add_argument("--renderer", default="./HypoxRasterizer")
    parser.add_argument("--output-root", default="build/validation_outputs")
    parser.add_argument("--tier", choices=["fast", "standard", "heavy"])
    parser.add_argument("--tag")
    parser.add_argument("--resolution")
    parser.add_argument("--default", action="store_true")
    parser.add_argument("--all", action="store_true")
    parser.add_argument("--include-heavy", action="store_true")
    parser.add_argument("--render", action="store_true")
    args = parser.parse_args()

    manifest = pathlib.Path(args.manifest)
    scenes = select_scenes(load_scenes(manifest), args)
    results: list[RunResult] = []
    for scene in scenes:
        result = run_scene(scene, args)
        results.append(result)
        if args.render:
            suffix = f" ({result.message})" if result.message else ""
            print(f"[{result.status.upper()}] {result.scene_id}{suffix}", flush=True)

    passed = sum(1 for result in results if result.status == "passed")
    failed = sum(1 for result in results if result.status == "failed")
    skipped = sum(1 for result in results if result.status == "skipped")
    dry_run = sum(1 for result in results if result.status == "dry-run")
    print(f"summary: selected={len(results)} passed={passed} failed={failed} skipped={skipped} dry_run={dry_run}")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
