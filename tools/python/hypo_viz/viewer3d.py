from __future__ import annotations

import argparse
import json
import os
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

from hypo_viz.render_3d_static import render_static_preview
from hypo_viz.scene3d import load_scene3d


DEFAULT_LAYERS = {
    "mesh": True,
    "bounds": True,
    "camera": True,
    "lights": True,
    "vpl": True,
    "world_position": False,
}


class ViewerUnavailableError(RuntimeError):
    pass


@dataclass(frozen=True)
class ViewerState:
    scene: str
    config: str
    scene_output_root: str | None
    layers: dict[str, bool]
    object_count: int
    light_count: int
    vpl_sample_count: int
    object_names: list[str]
    light_names: list[str]
    world_position_path: str | None
    point_size: int
    vpl_color: str
    mesh_opacity: float
    bounds_only: bool
    warnings: list[str]


def _merged_layers(layers: dict[str, bool] | None) -> dict[str, bool]:
    result = dict(DEFAULT_LAYERS)
    if layers:
        for key, value in layers.items():
            if key in result:
                result[key] = bool(value)
    return result


def build_viewer_state(
    config: Path | str,
    *,
    scene_output_root: Path | str | None = None,
    layers: dict[str, bool] | None = None,
    point_size: int = 4,
    vpl_color: str = "#ff4020",
    mesh_opacity: float = 0.45,
    bounds_only: bool = False,
) -> ViewerState:
    scene = load_scene3d(
        config,
        output_root=scene_output_root,
        mesh_mode="bounds_only" if bounds_only else "metadata_only",
    )
    root = Path(scene_output_root) if scene_output_root is not None else None
    world_position = root / "gbuffer" / "world_position.f32" if root is not None else None
    world_position_path = str(world_position) if world_position is not None and world_position.exists() else None
    state_layers = _merged_layers(layers)
    if world_position_path is None:
        state_layers["world_position"] = False
    return ViewerState(
        scene=scene.name,
        config=str(config),
        scene_output_root=str(scene_output_root) if scene_output_root is not None else None,
        layers=state_layers,
        object_count=len(scene.objects),
        light_count=len(scene.lights),
        vpl_sample_count=sum(len(buffer.samples) for buffer in scene.vpl_buffers),
        object_names=[obj.name for obj in scene.objects],
        light_names=[light.name for light in scene.lights],
        world_position_path=world_position_path,
        point_size=point_size,
        vpl_color=vpl_color,
        mesh_opacity=mesh_opacity,
        bounds_only=bounds_only,
        warnings=list(scene.warnings),
    )


def write_viewer_state(state: ViewerState, output: Path | str) -> None:
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(asdict(state), indent=2, sort_keys=True) + "\n", encoding="utf-8")


def write_html_viewer(state: ViewerState, output: Path | str) -> None:
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    state_json = json.dumps(asdict(state), indent=2, sort_keys=True)
    html = f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>HypoxRasterizer 3D Viewer - {state.scene}</title>
  <style>
    body {{ margin: 0; background: #16181d; color: #e6e6e6; font-family: sans-serif; }}
    aside {{ position: fixed; inset: 0 auto 0 0; width: 280px; padding: 16px; background: #20242b; box-sizing: border-box; }}
    main {{ margin-left: 280px; padding: 18px; }}
    label {{ display: block; margin: 8px 0; }}
    canvas {{ width: 100%; max-width: 960px; height: 540px; background: #101217; border: 1px solid #343a44; }}
    pre {{ white-space: pre-wrap; font-size: 12px; }}
  </style>
</head>
<body>
<aside>
  <h1>{state.scene}</h1>
  <label><input id="toggle-mesh" type="checkbox" checked> mesh</label>
  <label><input id="toggle-bounds" type="checkbox" checked> bounds</label>
  <label><input id="toggle-camera" type="checkbox" checked> camera</label>
  <label><input id="toggle-lights" type="checkbox" checked> lights</label>
  <label><input id="toggle-vpl" type="checkbox" checked> vpl</label>
  <label><input id="toggle-world-position" type="checkbox"> world_position</label>
  <pre id="summary"></pre>
</aside>
<main>
  <canvas id="viewport" width="960" height="540"></canvas>
</main>
<script>
const viewerState = {state_json};
document.getElementById("summary").textContent = JSON.stringify(viewerState, null, 2);
const canvas = document.getElementById("viewport");
const ctx = canvas.getContext("2d");
function draw() {{
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.fillStyle = "#101217";
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  ctx.fillStyle = "#e6e6e6";
  ctx.font = "18px sans-serif";
  ctx.fillText(viewerState.scene, 24, 34);
  const rows = [
    ["objects", viewerState.object_count],
    ["lights", viewerState.light_count],
    ["vpl samples", viewerState.vpl_sample_count],
    ["world position", viewerState.world_position_path || "none"]
  ];
  ctx.font = "15px sans-serif";
  rows.forEach((row, index) => ctx.fillText(row[0] + ": " + row[1], 24, 72 + index * 26));
  ctx.strokeStyle = viewerState.vpl_color;
  ctx.lineWidth = 3;
  ctx.strokeRect(300, 120, 360, 240);
  ctx.fillText("Open the JSON state for exact geometry; this HTML is a lightweight inspection shell.", 300, 390);
}}
document.querySelectorAll("input[type=checkbox]").forEach((input) => input.addEventListener("change", draw));
draw();
</script>
</body>
</html>
"""
    output.write_text(html, encoding="utf-8")


def _gui_available(headless: bool) -> bool:
    if headless:
        return False
    return bool(os.environ.get("DISPLAY") or os.environ.get("WAYLAND_DISPLAY") or os.name == "nt")


def launch_viewer(
    config: Path | str,
    *,
    scene_output_root: Path | str | None = None,
    output: Path | str | None = None,
    report_json: Path | str | None = None,
    backend: str = "html",
    fallback_static: bool = True,
    headless: bool = False,
    bounds_only: bool = False,
    layers: dict[str, bool] | None = None,
    point_size: int = 4,
    vpl_color: str = "#ff4020",
    mesh_opacity: float = 0.45,
) -> dict[str, Any]:
    if backend == "html":
        if output is None:
            raise ValueError("html backend requires --output")
        state = build_viewer_state(
            config,
            scene_output_root=scene_output_root,
            layers=layers,
            point_size=point_size,
            vpl_color=vpl_color,
            mesh_opacity=mesh_opacity,
            bounds_only=bounds_only,
        )
        write_html_viewer(state, output)
        result = {"mode": "html", "output": str(output), **asdict(state)}
    elif backend == "state":
        state = build_viewer_state(
            config,
            scene_output_root=scene_output_root,
            layers=layers,
            point_size=point_size,
            vpl_color=vpl_color,
            mesh_opacity=mesh_opacity,
            bounds_only=bounds_only,
        )
        if output is not None:
            write_viewer_state(state, output)
        result = {"mode": "state", **asdict(state)}
    elif backend == "pyvista":
        if not _gui_available(headless):
            if not fallback_static:
                raise ViewerUnavailableError("pyvista interactive viewer is unavailable in this headless environment")
            if output is None:
                raise ValueError("static fallback requires --output")
            static = render_static_preview(
                config,
                output,
                scene_output_root=scene_output_root,
                bounds_only=bounds_only,
            )
            result = {
                "mode": "static_fallback",
                "reason": "pyvista interactive viewer is unavailable in this headless environment",
                **asdict(static),
            }
        else:
            raise ViewerUnavailableError("pyvista backend is not installed in this lightweight viewer build")
    else:
        raise ViewerUnavailableError(f"unsupported viewer backend: {backend}")

    if report_json is not None:
        report_path = Path(report_json)
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return result


def _parse_layers(raw: list[str] | None) -> dict[str, bool]:
    if not raw:
        return {}
    result: dict[str, bool] = {}
    for item in raw:
        if item.startswith("no-"):
            result[item[3:]] = False
        else:
            result[item] = True
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Launch or describe the experimental 3D viewer.")
    parser.add_argument("--config", required=True)
    parser.add_argument("--scene-output-root")
    parser.add_argument("--output")
    parser.add_argument("--report-json")
    parser.add_argument("--backend", choices=["html", "state", "pyvista"], default="html")
    parser.add_argument("--no-fallback-static", action="store_true")
    parser.add_argument("--headless", action="store_true")
    parser.add_argument("--bounds-only", action="store_true")
    parser.add_argument("--layer", action="append", help="Enable layer, or disable with no-LAYER")
    parser.add_argument("--point-size", type=int, default=4)
    parser.add_argument("--vpl-color", default="#ff4020")
    parser.add_argument("--mesh-opacity", type=float, default=0.45)
    args = parser.parse_args(argv)

    result = launch_viewer(
        args.config,
        scene_output_root=args.scene_output_root,
        output=args.output,
        report_json=args.report_json,
        backend=args.backend,
        fallback_static=not args.no_fallback_static,
        headless=args.headless,
        bounds_only=args.bounds_only,
        layers=_parse_layers(args.layer),
        point_size=args.point_size,
        vpl_color=args.vpl_color,
        mesh_opacity=args.mesh_opacity,
    )
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
