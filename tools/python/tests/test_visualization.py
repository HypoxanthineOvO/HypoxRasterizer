import tempfile
import unittest
from pathlib import Path

from PIL import Image

from hypo_validation.collect_stats import read_f32_buffer
from hypo_viz.render_2d import (
    create_shadow_contact_sheet,
    overlay_vpl_samples,
    render_scene_visuals,
    render_f32_heatmap,
    visualization_output_path,
)


class VisualizationTests(unittest.TestCase):
    def write_f32(self, path: Path, name: str, width: int, height: int, channels: int, values: list[float]) -> None:
        import struct

        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("wb") as handle:
            handle.write(b"HYPORASTER_F32_V1\n")
            handle.write(name.encode("utf-8") + b"\n")
            handle.write(f"{width} {height} {channels}\n".encode("ascii"))
            handle.write(struct.pack("<" + "f" * len(values), *values))

    def test_heatmap_generation_from_f32(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_viz_heatmap_"))
        source = root / "gbuffer" / "linear_depth.f32"
        output = root / "viz" / "linear_depth_heatmap.png"
        self.write_f32(source, "linear_depth", 2, 2, 1, [0.0, 0.25, 0.75, 1.0])

        render_f32_heatmap(source, output)

        with Image.open(output) as image:
            self.assertEqual(image.size, (2, 2))
            self.assertGreater(len(image.convert("RGB").getcolors(maxcolors=16) or []), 1)
        self.assertEqual(read_f32_buffer(source).shape, [2, 2, 1])

    def test_vpl_overlay_generates_annotated_png(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_viz_vpl_"))
        base = root / "shadow" / "ArealightShadowMap_0.png"
        vpl = root / "shadow" / "light_0_direct_vpls.f32"
        output = root / "viz" / "light_0_direct_vpls_annotated.png"
        base.parent.mkdir(parents=True, exist_ok=True)
        Image.new("RGB", (32, 32), (12, 12, 12)).save(base)
        self.write_f32(vpl, "light_0_direct_vpls", 4, 1, 3, [0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0])

        warnings = overlay_vpl_samples(base, vpl, output)

        self.assertEqual(warnings, [])
        with Image.open(output) as opened:
            image = opened.convert("RGB")
            self.assertEqual(image.size, (32, 32))
            colors = {color for _, color in image.getcolors(maxcolors=32 * 32)}
            self.assertIn((255, 64, 32), colors)

    def test_shadow_contact_sheet_for_cube_faces(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_viz_shadow_"))
        shadow_dir = root / "shadow"
        shadow_dir.mkdir(parents=True, exist_ok=True)
        for index, face in enumerate(["pos_x", "neg_x", "pos_y", "neg_y", "pos_z", "neg_z"]):
            Image.new("RGB", (8, 8), (index * 30, 16, 64)).save(
                shadow_dir / f"PointlightShadowMap_0_face_{face}.png"
            )
        output = root / "viz" / "point_cube_contact_sheet.png"

        warnings = create_shadow_contact_sheet(shadow_dir, output)

        self.assertEqual(warnings, [])
        with Image.open(output) as image:
            self.assertEqual(image.size, (24, 32))

    def test_missing_optional_inputs_warn_and_stable_output_name(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_viz_missing_"))
        output = root / "viz" / "missing.png"

        warnings = create_shadow_contact_sheet(root / "missing_shadow", output)

        self.assertTrue(warnings)
        self.assertFalse(output.exists())
        self.assertEqual(
            visualization_output_path(root, "shadow/light_0_direct_vpls.f32", "overlay").as_posix(),
            (root / "viz" / "shadow_light_0_direct_vpls_overlay.png").as_posix(),
        )

    def test_scene_visuals_include_indirect_vpl_overlay(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_viz_indirect_vpl_"))
        shadow = root / "shadow"
        shadow.mkdir(parents=True, exist_ok=True)
        Image.new("RGB", (32, 32), (12, 12, 12)).save(shadow / "PointlightShadowMap_0_face_pos_y.png")
        self.write_f32(
            shadow / "light_0_indirect_vpls.f32",
            "light_0_indirect_vpls",
            2,
            1,
            13,
            [
                0, 0, 0, 0, 1, 0, 1, 1, 1, 0.2, 1, 0, 0,
                1, 1, 0, 0, 1, 0, 1, 0, 0, 0.2, 1, 0, 0,
            ],
        )
        output_dir = root / "viz"

        warnings = render_scene_visuals(root, output_dir)

        self.assertEqual(warnings, [])
        self.assertTrue((output_dir / "light_0_indirect_vpls_annotated.png").exists())
        self.assertTrue((output_dir / "shadow_light_0_indirect_vpls_heatmap.png").exists())


if __name__ == "__main__":
    unittest.main()
