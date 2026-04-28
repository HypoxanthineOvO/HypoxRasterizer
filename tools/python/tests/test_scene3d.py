import struct
import tempfile
import unittest
from pathlib import Path

from hypo_viz.scene3d import (
    load_scene3d,
    load_vpl_sample_buffer,
    resolve_repo_root,
)


class Scene3DTests(unittest.TestCase):
    def write_f32(self, path: Path, name: str, width: int, height: int, channels: int, values: list[float]) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("wb") as handle:
            handle.write(b"HYPORASTER_F32_V1\n")
            handle.write(name.encode("utf-8") + b"\n")
            handle.write(f"{width} {height} {channels}\n".encode("ascii"))
            handle.write(struct.pack("<" + "f" * len(values), *values))

    def test_area_vpl_scene_loads_objects_camera_lights(self) -> None:
        scene = load_scene3d("configs/AreaVPLSmoke.v2.json")

        self.assertEqual(scene.name, "area_vpl_smoke")
        self.assertEqual(len(scene.objects), 7)
        self.assertEqual(scene.camera.resolution, [64, 64])
        self.assertEqual(scene.camera.position, [0.0, 1.0, 3.0])
        self.assertEqual(len(scene.lights), 1)
        self.assertEqual(scene.lights[0].type, "area")
        self.assertEqual(scene.lights[0].vpl_samples, 64)
        self.assertEqual(scene.lights[0].size, [0.9, 0.6])
        self.assertTrue(scene.lights[0].shadow_enabled)
        self.assertEqual(scene.objects[0].material_id, "white")
        self.assertTrue(scene.objects[0].mesh.exists)
        self.assertGreater(scene.objects[0].mesh.vertex_count, 0)

    def test_bunny_mesh_metadata_and_world_bounds(self) -> None:
        scene = load_scene3d("configs/validation/standard/bunny_mesh.v2.json")

        bunny = scene.objects[0]
        self.assertEqual(bunny.name, "bunny")
        self.assertGreater(bunny.mesh.vertex_count, 1000)
        self.assertGreater(bunny.mesh.face_count, 1000)
        self.assertEqual(bunny.translation, [0.1, 0.0, -0.02])
        self.assertEqual(bunny.scale, [2.0, 2.0, 2.0])
        self.assertLess(bunny.world_bounds.minimum[0], bunny.world_bounds.maximum[0])
        self.assertLess(bunny.world_bounds.minimum[1], bunny.world_bounds.maximum[1])
        self.assertLess(bunny.world_bounds.minimum[2], bunny.world_bounds.maximum[2])

    def test_dragon_bounds_only_mode_avoids_vertex_storage(self) -> None:
        scene = load_scene3d("configs/validation/standard/dragon_quick.v2.json", mesh_mode="bounds_only")

        dragon = scene.objects[0]
        self.assertEqual(dragon.name, "dragon")
        self.assertGreater(dragon.mesh.vertex_count, 100000)
        self.assertGreater(dragon.mesh.face_count, 100000)
        self.assertEqual(dragon.mesh.mode, "bounds_only")
        self.assertEqual(dragon.mesh.vertices, [])
        self.assertFalse(scene.warnings)

    def test_vpl_buffer_loads_3d_samples(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_scene3d_vpl_"))
        vpl = root / "shadow" / "light_0_direct_vpls.f32"
        self.write_f32(
            vpl,
            "light_0_direct_vpls",
            2,
            1,
            6,
            [0, 1, 2, 0, 1, 0, 3, 4, 5, 0, 0, 1],
        )

        samples = load_vpl_sample_buffer(vpl)

        self.assertEqual(len(samples), 2)
        self.assertEqual(samples[0].position, [0.0, 1.0, 2.0])
        self.assertEqual(samples[0].normal, [0.0, 1.0, 0.0])
        self.assertEqual(samples[1].position, [3.0, 4.0, 5.0])

    def test_missing_mesh_records_warning(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_scene3d_missing_"))
        config = root / "missing.v2.json"
        config.write_text(
            """
{
  "scene": {"name": "missing_mesh"},
  "camera": {"resolution": [8, 8], "position": [0, 0, 1], "target": [0, 0, 0], "fov": 45},
  "materials": [{"id": "mat", "type": "phong", "baseColor": [1, 1, 1]}],
  "objects": [{"name": "bad", "mesh": "./assets/nope.obj", "material": "mat"}],
  "lights": []
}
""".strip(),
            encoding="utf-8",
        )

        scene = load_scene3d(config, repo_root=resolve_repo_root(Path.cwd()))

        self.assertEqual(scene.objects[0].mesh.exists, False)
        self.assertTrue(any("missing mesh" in warning for warning in scene.warnings))


if __name__ == "__main__":
    unittest.main()
