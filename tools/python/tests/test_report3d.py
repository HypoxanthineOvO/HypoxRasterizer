import base64
import json
import struct
import tempfile
import unittest
from pathlib import Path

from PIL import Image

from hypo_viz.report3d import (
    generate_scene_3d_report,
    load_world_position_points,
    shadow_face_metadata,
    write_point_cloud_preview,
)


PNG_1X1_RGBA = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAFgwJ/lGZK9QAAAABJRU5ErkJggg=="
)


class Report3DTests(unittest.TestCase):
    def write_f32(self, path: Path, name: str, width: int, height: int, channels: int, values: list[float]) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("wb") as handle:
            handle.write(b"HYPORASTER_F32_V1\n")
            handle.write(name.encode("utf-8") + b"\n")
            handle.write(f"{width} {height} {channels}\n".encode("ascii"))
            handle.write(struct.pack("<" + "f" * len(values), *values))

    def test_world_position_f32_converts_to_downsampled_points(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_report3d_points_"))
        source = root / "gbuffer" / "world_position.f32"
        values: list[float] = []
        for y in range(4):
            for x in range(4):
                values.extend([float(x), float(y), float(x + y)])
        self.write_f32(source, "world_position", 4, 4, 3, values)

        points = load_world_position_points(source, stride=2)

        self.assertEqual(len(points), 4)
        self.assertEqual(points[0], [0.0, 0.0, 0.0])
        self.assertEqual(points[-1], [2.0, 2.0, 4.0])

    def test_point_cloud_preview_writes_png(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_report3d_preview_"))
        output = root / "world_position_preview.png"

        count = write_point_cloud_preview([[0, 0, 0], [1, 1, 1], [2, 0, 1]], output, image_size=(128, 96))

        self.assertEqual(count, 3)
        with Image.open(output) as image:
            self.assertEqual(image.size, (128, 96))

    def test_shadow_face_metadata_labels_cube_faces(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_report3d_shadow_"))
        shadow = root / "shadow"
        shadow.mkdir(parents=True, exist_ok=True)
        for face in ["pos_x", "neg_x", "pos_y", "neg_y", "pos_z", "neg_z"]:
            Image.new("RGB", (2, 2), (16, 16, 16)).save(shadow / f"PointlightShadowMap_0_face_{face}.png")

        metadata = shadow_face_metadata(shadow)

        self.assertEqual(len(metadata), 6)
        by_face = {item["face"]: item for item in metadata}
        self.assertEqual(by_face["pos_x"]["direction"], [1, 0, 0])
        self.assertEqual(by_face["neg_z"]["direction"], [0, 0, -1])

    def test_generate_scene_report_links_2d_3d_and_shadow_outputs(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_report3d_scene_"))
        scene_output = root / "area_vpl_smoke"
        scene_output.mkdir(parents=True, exist_ok=True)
        Image.new("RGB", (16, 16), (20, 20, 20)).save(scene_output / "color.png")
        self.write_f32(scene_output / "gbuffer" / "world_position.f32", "world_position", 2, 2, 3, [0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0])
        self.write_f32(scene_output / "shadow" / "light_0_direct_vpls.f32", "light_0_direct_vpls", 2, 1, 6, [0, 1, 0, 0, -1, 0, 1, 1, 0, 0, -1, 0])
        self.write_f32(
            scene_output / "shadow" / "light_0_indirect_vpls.f32",
            "light_0_indirect_vpls",
            1,
            1,
            13,
            [0, 0, 0, 0, 1, 0, 1, 1, 1, 0.2, 1, 0, 0],
        )
        Image.new("RGB", (4, 4), (32, 32, 32)).save(scene_output / "shadow" / "ArealightShadowMap_0.png")
        output_dir = root / "report"

        result = generate_scene_3d_report(
            "configs/AreaVPLSmoke.v2.json",
            scene_output,
            output_dir,
            point_stride=1,
        )

        self.assertTrue((output_dir / "report.md").exists())
        self.assertTrue((output_dir / "report.json").exists())
        self.assertTrue((output_dir / "scene3d.png").exists())
        self.assertTrue((output_dir / "world_position_point_cloud.png").exists())
        self.assertGreater(result["point_cloud"]["points"], 0)
        self.assertEqual(result["vpl_buffers"]["indirect"], 1)
        self.assertEqual(result["static_3d"]["scene"], "area_vpl_smoke")
        metadata = json.loads((output_dir / "report.json").read_text(encoding="utf-8"))
        self.assertEqual(metadata["scene"], "area_vpl_smoke")


if __name__ == "__main__":
    unittest.main()
