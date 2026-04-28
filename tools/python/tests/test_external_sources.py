import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

from hypo_assets.external_sources import (
    ExternalSourceManifestError,
    load_external_sources,
    plan_downloads,
)
from hypo_assets.fetch_scenes import proxy_url_for_port


class ExternalSourceTests(unittest.TestCase):
    def write_manifest(self, text: str) -> Path:
        root = Path(tempfile.mkdtemp(prefix="hypo_external_sources_"))
        path = root / "external_sources.yaml"
        path.write_text(textwrap.dedent(text), encoding="utf-8")
        return path

    def test_manifest_requires_license_url_and_hash_policy(self) -> None:
        manifest = self.write_manifest(
            """
            version: 1
            proxy:
              approved_ports: [7890, 7891, 7892]
            sources:
              - id: missing_license
                url: https://example.invalid/scene.zip
                tier: standard
                target: assets/external/example/scene.zip
                format: zip
                sha256: pending
                hash_policy: record_on_first_download
                enabled: true
            """
        )

        with self.assertRaises(ExternalSourceManifestError):
            load_external_sources(manifest)

    def test_plan_downloads_exposes_auditable_metadata(self) -> None:
        manifest = self.write_manifest(
            """
            version: 1
            proxy:
              approved_ports: [7890, 7891, 7892]
            sources:
              - id: bitterli_cornell_box
                url: https://example.invalid/cornell-box.zip
                license: public-domain
                tier: standard
                target: assets/external/bitterli/cornell-box.zip
                format: zip
                sha256: pending
                hash_policy: record_on_first_download
                conversion: pbrt_v4_metadata
                enabled: true
            """
        )

        sources = load_external_sources(manifest)
        plans = plan_downloads(sources, include_disabled=False)

        self.assertEqual(len(plans), 1)
        self.assertEqual(plans[0].source_id, "bitterli_cornell_box")
        self.assertEqual(plans[0].license, "public-domain")
        self.assertEqual(plans[0].target, Path("assets/external/bitterli/cornell-box.zip"))
        self.assertEqual(plans[0].hash_policy, "record_on_first_download")

    def test_fetcher_dry_run_reports_proxy_and_hash_policy(self) -> None:
        manifest = self.write_manifest(
            """
            version: 1
            proxy:
              approved_ports: [7890, 7891, 7892]
            sources:
              - id: bitterli_cornell_box
                url: https://example.invalid/cornell-box.zip
                license: public-domain
                tier: standard
                target: assets/external/bitterli/cornell-box.zip
                format: zip
                sha256: pending
                hash_policy: record_on_first_download
                conversion: pbrt_v4_metadata
                enabled: true
            """
        )

        completed = subprocess.run(
            [
                sys.executable,
                "-m",
                "hypo_assets.fetch_scenes",
                "--manifest",
                str(manifest),
                "--dry-run",
                "--proxy-port",
                "7891",
            ],
            check=True,
            text=True,
            capture_output=True,
        )

        self.assertIn("bitterli_cornell_box", completed.stdout)
        self.assertIn("license=public-domain", completed.stdout)
        self.assertIn("hash_policy=record_on_first_download", completed.stdout)
        self.assertIn("proxy=http://127.0.0.1:7891", completed.stdout)

    def test_proxy_port_must_be_approved(self) -> None:
        self.assertEqual(proxy_url_for_port(7890, [7890, 7891, 7892]), "http://127.0.0.1:7890")
        with self.assertRaises(ValueError):
            proxy_url_for_port(8888, [7890, 7891, 7892])

    def test_runner_skips_missing_optional_external_heavy_scene(self) -> None:
        manifest = self.write_manifest(
            """
            version: 1
            proxy:
              approved_ports: [7890, 7891, 7892]
            scenes:
              - id: heavy_missing_external
                tier: heavy
                config: configs/validation/heavy/missing_external.v2.json
                default: false
                tags: [heavy, external, optional]
                assets: [assets/external/missing/scene.obj]
                expected_outputs: [color]
                runtime_budget_seconds: 180
                source_type: external
                license: pending
            """
        )

        completed = subprocess.run(
            [
                sys.executable,
                "tools/validation/validation_runner.py",
                "--manifest",
                str(manifest),
                "--tier",
                "heavy",
                "--render",
                "--renderer",
                "./missing_renderer",
            ],
            cwd=Path(__file__).resolve().parents[3],
            text=True,
            capture_output=True,
        )

        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        self.assertIn("[SKIPPED] heavy_missing_external", completed.stdout)
        self.assertIn("summary: selected=1 passed=0 failed=0 skipped=1", completed.stdout)


if __name__ == "__main__":
    unittest.main()
