import json
import struct
import tempfile
import unittest
from pathlib import Path

from PIL import Image

from hypo_viz.render_3d_static import Static3DBackendError, render_static_preview


class Static3DPreviewTests(unittest.TestCase):
    def write_f32(self, path: Path, name: str, width: int, height: int, channels: int, values: list[float]) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("wb") as handle:
            handle.write(b"HYPORASTER_F32_V1\n")
            handle.write(name.encode("utf-8") + b"\n")
            handle.write(f"{width} {height} {channels}\n".encode("ascii"))
            handle.write(struct.pack("<" + "f" * len(values), *values))

    def test_area_vpl_scene_generates_static_overview_png_and_report(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_static3d_area_"))
        scene_output = root / "area_vpl_smoke"
        self.write_f32(
            scene_output / "shadow" / "light_0_direct_vpls.f32",
            "light_0_direct_vpls",
            3,
            1,
            6,
            [-0.3, 1.8, 0.0, 0, -1, 0, 0.0, 1.8, 0.0, 0, -1, 0, 0.3, 1.8, 0.0, 0, -1, 0],
        )
        output = root / "area_overview.png"
        report = root / "area_overview.json"

        result = render_static_preview(
            "configs/AreaVPLSmoke.v2.json",
            output,
            scene_output_root=scene_output,
            report_json=report,
            image_size=(480, 360),
        )

        self.assertTrue(output.exists())
        self.assertTrue(report.exists())
        self.assertEqual(result.backend, "pillow_software")
        self.assertEqual(result.object_count, 7)
        self.assertEqual(result.light_count, 1)
        self.assertEqual(result.vpl_sample_count, 3)
        with Image.open(output) as image:
            self.assertEqual(image.size, (480, 360))
            self.assertGreater(len(image.convert("RGB").getcolors(maxcolors=480 * 360) or []), 4)
        metadata = json.loads(report.read_text(encoding="utf-8"))
        self.assertEqual(metadata["scene"], "area_vpl_smoke")
        self.assertEqual(metadata["vpl_sample_count"], 3)

    def test_dragon_bounds_only_static_preview(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_static3d_dragon_"))
        output = root / "dragon_bounds.png"

        result = render_static_preview(
            "configs/validation/standard/dragon_quick.v2.json",
            output,
            bounds_only=True,
            image_size=(320, 240),
        )

        self.assertTrue(output.exists())
        self.assertEqual(result.object_count, 1)
        self.assertEqual(result.backend, "pillow_software")
        with Image.open(output) as image:
            self.assertEqual(image.size, (320, 240))

    def test_missing_mesh_warns_but_still_writes_preview(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_static3d_missing_"))
        config = root / "missing.v2.json"
        config.write_text(
            """
{
  "scene": {"name": "missing_mesh"},
  "camera": {"resolution": [8, 8], "position": [0, 0, 2], "target": [0, 0, 0], "fov": 45},
  "materials": [],
  "objects": [{"name": "bad", "mesh": "./assets/nope.obj", "material": "mat"}],
  "lights": [{"name": "point", "type": "point", "position": [1, 2, 3]}]
}
""".strip(),
            encoding="utf-8",
        )
        output = root / "missing.png"

        result = render_static_preview(config, output, image_size=(160, 120))

        self.assertTrue(output.exists())
        self.assertTrue(any("missing mesh" in warning for warning in result.warnings))

    def test_unavailable_backend_errors_without_partial_file(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_static3d_backend_"))
        output = root / "should_not_exist.png"

        with self.assertRaises(Static3DBackendError):
            render_static_preview("configs/AreaVPLSmoke.v2.json", output, backend="not-installed")

        self.assertFalse(output.exists())


if __name__ == "__main__":
    unittest.main()
