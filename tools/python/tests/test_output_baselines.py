import base64
import math
import struct
import tempfile
import unittest
from pathlib import Path

from hypo_validation.collect_stats import (
    FloatBufferFormatError,
    collect_output_tree,
    collect_path_stats,
    read_f32_buffer,
    write_baseline,
)
from hypo_validation.compare_baseline import compare_baselines


PNG_1X1_RGBA = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAFgwJ/lGZK9QAAAABJRU5ErkJggg=="
)


class OutputBaselineTests(unittest.TestCase):
    def write_f32(self, path: Path, name: str, width: int, height: int, channels: int, values: list[float]) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("wb") as handle:
            handle.write(b"HYPORASTER_F32_V1\n")
            handle.write(name.encode("utf-8") + b"\n")
            handle.write(f"{width} {height} {channels}\n".encode("ascii"))
            handle.write(struct.pack("<" + "f" * len(values), *values))

    def test_f32_reader_stats_and_malformed_header(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_baseline_f32_"))
        good = root / "linear_depth.f32"
        self.write_f32(good, "linear_depth", 2, 1, 1, [0.0, 2.0])

        buffer = read_f32_buffer(good)
        self.assertEqual(buffer.name, "linear_depth")
        self.assertEqual(buffer.shape, [2, 1, 1])
        self.assertEqual(buffer.values, [0.0, 2.0])

        stats = collect_path_stats(good, root)
        self.assertEqual(stats["kind"], "f32")
        self.assertEqual(stats["shape"], [2, 1, 1])
        self.assertAlmostEqual(stats["mean"], 1.0)
        self.assertAlmostEqual(stats["nonzero_ratio"], 0.5)
        self.assertAlmostEqual(stats["finite_ratio"], 1.0)

        bad = root / "bad.f32"
        bad.write_bytes(b"NOT_HYPO\n")
        with self.assertRaises(FloatBufferFormatError):
            read_f32_buffer(bad)

    def test_png_stats_collect_hash_and_shape(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_baseline_png_"))
        image = root / "shadow" / "PointlightShadowMap_0_face_pos_x.png"
        image.parent.mkdir(parents=True, exist_ok=True)
        image.write_bytes(PNG_1X1_RGBA)

        stats = collect_path_stats(image, root)
        self.assertEqual(stats["kind"], "png")
        self.assertEqual(stats["shape"], [1, 1, 4])
        self.assertEqual(len(stats["sha256"]), 64)

    def test_compare_detects_missing_shape_drift_and_hash(self) -> None:
        baseline = {
            "files": {
                "gbuffer/linear_depth.f32": {
                    "kind": "f32",
                    "shape": [2, 1, 1],
                    "sha256": "a" * 64,
                    "mean": 1.0,
                    "std": 0.5,
                    "min": 0.0,
                    "max": 2.0,
                    "finite_ratio": 1.0,
                    "nonzero_ratio": 0.5,
                },
                "shadow/map.png": {
                    "kind": "png",
                    "shape": [1, 1, 4],
                    "sha256": "b" * 64,
                },
            }
        }
        current = {
            "files": {
                "gbuffer/linear_depth.f32": {
                    "kind": "f32",
                    "shape": [4, 1, 1],
                    "sha256": "c" * 64,
                    "mean": 4.0,
                    "std": 0.5,
                    "min": 0.0,
                    "max": 8.0,
                    "finite_ratio": 1.0,
                    "nonzero_ratio": 0.5,
                }
            }
        }

        diffs = compare_baselines(baseline, current, strict_hash=True, abs_tol=0.01)
        kinds = {diff.kind for diff in diffs}
        self.assertIn("missing_file", kinds)
        self.assertIn("shape_mismatch", kinds)
        self.assertIn("hash_mismatch", kinds)
        self.assertIn("numeric_drift", kinds)

    def test_output_tree_roundtrip_includes_gbuffer_shadow_and_vpl(self) -> None:
        root = Path(tempfile.mkdtemp(prefix="hypo_baseline_tree_"))
        scene = root / "fast_multi_light_shadow"
        self.write_f32(scene / "gbuffer" / "linear_depth.f32", "linear_depth", 2, 1, 1, [1.0, math.inf])
        self.write_f32(scene / "shadow" / "light_0_direct_vpls.f32", "direct_vpls", 2, 1, 3, [0, 1, 2, 3, 4, 5])
        shadow_png = scene / "shadow" / "PointlightShadowMap_0_face_pos_x.png"
        shadow_png.parent.mkdir(parents=True, exist_ok=True)
        shadow_png.write_bytes(PNG_1X1_RGBA)

        baseline = collect_output_tree(scene, scene_id="fast_multi_light_shadow")
        self.assertIn("gbuffer/linear_depth.f32", baseline["files"])
        self.assertIn("shadow/PointlightShadowMap_0_face_pos_x.png", baseline["files"])
        self.assertIn("shadow/light_0_direct_vpls.f32", baseline["files"])
        self.assertLess(baseline["files"]["gbuffer/linear_depth.f32"]["finite_ratio"], 1.0)

        baseline_path = root / "baseline.json"
        write_baseline(baseline, baseline_path)
        self.assertEqual(compare_baselines(baseline, baseline), [])


if __name__ == "__main__":
    unittest.main()
