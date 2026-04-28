import json
import subprocess
import sys
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[3]


class HeavyValidationDocsTests(unittest.TestCase):
    def test_heavy_manifest_has_internal_benchmark_and_external_hooks(self) -> None:
        manifest = (PROJECT_ROOT / "validation/scenes.yaml").read_text(encoding="utf-8")

        self.assertIn("id: heavy_dragon_lowres_benchmark", manifest)
        self.assertIn("config: configs/validation/heavy/dragon_lowres_benchmark.v2.json", manifest)
        self.assertIn("tier: heavy", manifest)
        self.assertIn("source_type: internal", manifest)
        self.assertIn("id: heavy_orca_bistro_hook", manifest)
        self.assertIn("source_type: external", manifest)

    def test_heavy_runner_renders_internal_and_skips_missing_external_hooks(self) -> None:
        completed = subprocess.run(
            [
                sys.executable,
                "tools/validation/validation_runner.py",
                "--manifest",
                "validation/scenes.yaml",
                "--tier",
                "heavy",
                "--render",
                "--output-root",
                "build/test_outputs/m12_heavy_runner",
            ],
            cwd=PROJECT_ROOT,
            text=True,
            capture_output=True,
        )

        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        self.assertIn("[PASSED] heavy_dragon_lowres_benchmark", completed.stdout)
        self.assertIn("[SKIPPED] heavy_orca_bistro_hook", completed.stdout)
        self.assertIn("failed=0", completed.stdout)

    def test_heavy_perf_metadata_is_recorded(self) -> None:
        perf_path = PROJECT_ROOT / "validation/perf/heavy_dragon_lowres_benchmark.json"
        data = json.loads(perf_path.read_text(encoding="utf-8"))

        self.assertEqual(data["scene_id"], "heavy_dragon_lowres_benchmark")
        self.assertEqual(data["tier"], "heavy")
        self.assertEqual(data["resolution"], [24, 24])
        self.assertGreater(data["runtime_seconds"], 0.0)
        self.assertIn("--tier heavy", data["runner_command"])

    def test_docs_record_core_runner_commands(self) -> None:
        docs = (PROJECT_ROOT / "docs/validation-scenes.md").read_text(encoding="utf-8")

        self.assertIn("python3 tools/validation/validation_runner.py --default --tier fast --render", docs)
        self.assertIn("python3 tools/validation/validation_runner.py --tier standard --render", docs)
        self.assertIn("python3 tools/validation/validation_runner.py --tier heavy --render", docs)
        self.assertIn("heavy_dragon_lowres_benchmark", docs)
        self.assertIn("heavy_orca_bistro_hook", docs)


if __name__ == "__main__":
    unittest.main()
