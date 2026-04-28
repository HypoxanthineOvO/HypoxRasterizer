import json
import struct
import tempfile
import unittest
from pathlib import Path

from PIL import Image

from hypo_viz.viewer3d import (
    ViewerUnavailableError,
    build_viewer_state,
    launch_viewer,
    write_html_viewer,
)


class Viewer3DTests(unittest.TestCase):
    def write_f32(self, path: Path, name: str, width: int, height: int, channels: int, values: list[float]) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("wb") as handle:
            handle.write(b"HYPORASTER_F32_V1\n")
            handle.write(name.encode("utf-8") + b"\n")
            handle.write(f"{width} {height} {channels}\n".encode("ascii"))
            handle.write(struct.pack("<" + "f" * len(values), *values))

    def test_area_vpl_viewer_state_contains_layers_and_counts(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_viewer3d_state_"))
        scene_output = root / "area_vpl_smoke"
        self.write_f32(
            scene_output / "shadow" / "light_0_direct_vpls.f32",
            "light_0_direct_vpls",
            2,
            1,
            6,
            [0, 1, 0, 0, -1, 0, 1, 1, 0, 0, -1, 0],
        )

        state = build_viewer_state(
            "configs/AreaVPLSmoke.v2.json",
            scene_output_root=scene_output,
            layers={"mesh": True, "bounds": True, "lights": True, "vpl": True},
            point_size=5,
            mesh_opacity=0.35,
        )

        self.assertEqual(state.scene, "area_vpl_smoke")
        self.assertEqual(state.object_count, 7)
        self.assertEqual(state.light_count, 1)
        self.assertEqual(state.vpl_sample_count, 2)
        self.assertTrue(state.layers["mesh"])
        self.assertTrue(state.layers["vpl"])
        self.assertEqual(state.point_size, 5)
        self.assertAlmostEqual(state.mesh_opacity, 0.35)

    def test_html_viewer_writes_reproducible_state(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_viewer3d_html_"))
        output = root / "viewer.html"
        state = build_viewer_state("configs/validation/standard/bunny_mesh.v2.json", bounds_only=True)

        write_html_viewer(state, output)

        text = output.read_text(encoding="utf-8")
        self.assertIn("standard_bunny_mesh", text)
        self.assertIn("viewerState", text)
        self.assertIn("toggle-bounds", text)

    def test_headless_pyvista_request_falls_back_to_static_png(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_viewer3d_fallback_"))
        output = root / "fallback.png"
        report = root / "fallback.json"

        result = launch_viewer(
            "configs/AreaVPLSmoke.v2.json",
            output=output,
            report_json=report,
            backend="pyvista",
            fallback_static=True,
            headless=True,
        )

        self.assertEqual(result["mode"], "static_fallback")
        self.assertTrue(output.exists())
        self.assertTrue(report.exists())
        with Image.open(output) as image:
            self.assertGreater(image.size[0], 0)
        metadata = json.loads(report.read_text(encoding="utf-8"))
        self.assertEqual(metadata["mode"], "static_fallback")

    def test_headless_without_fallback_reports_unavailable(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_viewer3d_unavailable_"))
        output = root / "none.png"

        with self.assertRaises(ViewerUnavailableError):
            launch_viewer(
                "configs/AreaVPLSmoke.v2.json",
                output=output,
                backend="pyvista",
                fallback_static=False,
                headless=True,
            )

        self.assertFalse(output.exists())


if __name__ == "__main__":
    unittest.main()
