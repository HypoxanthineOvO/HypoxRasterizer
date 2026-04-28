#include "TestUtils.hpp"

#include "validation_manifest.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace {

std::filesystem::path findProjectRoot() {
    std::filesystem::path current = std::filesystem::current_path();
    for (int depth = 0; depth < 8; ++depth) {
        if (std::filesystem::exists(current / "xmake.lua") &&
            std::filesystem::exists(current / "configs") &&
            std::filesystem::exists(current / "assets")) {
            return current;
        }
        if (!current.has_parent_path() || current == current.parent_path()) {
            break;
        }
        current = current.parent_path();
    }
    throw test::Failure("unable to locate project root from current working directory");
}

std::filesystem::path writeManifestFixture() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/validation_manifest";
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path manifest_path = output_dir / "scenes.yaml";
    std::ofstream out(manifest_path);
    out << R"(version: 1
proxy:
  approved_ports: [7890, 7891, 7892]
scenes:
  - id: fast_smoke
    tier: fast
    config: configs/SmokeTest.v2.json
    default: true
    tags: [smoke, point, gbuffer, shadow]
    assets: [assets/Triangle/scene1.obj]
    expected_outputs: [color, gbuffer, shadow]
    runtime_budget_seconds: 5
    source_type: internal
    license: project
  - id: standard_texture
    tier: standard
    config: configs/AreaVPLSmoke.v2.json
    default: true
    tags: [standard, area, vpl]
    assets: [assets/CornellBox/floor.obj, assets/CornellBox/short_box.obj]
    expected_outputs: [color, gbuffer, shadow]
    runtime_budget_seconds: 15
    source_type: internal
    license: project
  - id: heavy_optional
    tier: heavy
    config: configs/validation/heavy/missing.v2.json
    default: false
    tags: [heavy, external]
    assets: [assets/external/missing.obj]
    expected_outputs: [color]
    runtime_budget_seconds: 120
    source_type: external
    license: pending
)";
    return manifest_path;
}

void testManifestLoadsScenesAndProxyPorts() {
    ValidationManifest manifest = loadValidationManifest(writeManifestFixture());

    test::require(manifest.version == 1, "manifest version should load");
    test::require(manifest.proxy_approved_ports.size() == 3, "proxy ports should load");
    test::require(manifest.proxy_approved_ports[0] == 7890, "first proxy port");
    test::require(manifest.scenes.size() == 3, "three scenes should load");

    const ValidationScene& scene = manifest.scenes[0];
    test::require(scene.id == "fast_smoke", "scene id should load");
    test::require(scene.tier == ValidationTier::Fast, "scene tier should load");
    test::require(scene.run_by_default, "default flag should load");
    test::require(scene.tags.size() == 4, "tags should load");
    test::require(scene.expected_outputs.size() == 3, "expected outputs should load");
}

void testDefaultSelectionExcludesHeavy() {
    ValidationManifest manifest = loadValidationManifest(writeManifestFixture());
    ValidationSelection selection;
    selection.default_only = true;
    std::vector<ValidationScene> scenes = selectValidationScenes(manifest, selection);

    test::require(scenes.size() == 2, "default selection should include fast and standard only");
    for (const ValidationScene& scene : scenes) {
        test::require(scene.tier != ValidationTier::Heavy, "heavy scenes should not run by default");
    }
}

void testTierAndTagSelection() {
    ValidationManifest manifest = loadValidationManifest(writeManifestFixture());
    ValidationSelection selection;
    selection.default_only = false;
    selection.tiers = {ValidationTier::Standard};
    selection.tags = {"vpl"};
    std::vector<ValidationScene> scenes = selectValidationScenes(manifest, selection);

    test::require(scenes.size() == 1, "standard vpl selection should find one scene");
    test::require(scenes[0].id == "standard_texture", "selected scene id");
}

void testReferenceValidationReportsMissingAssets() {
    ValidationManifest manifest = loadValidationManifest(writeManifestFixture());
    std::vector<ValidationReferenceIssue> issues = validateValidationSceneReferences(
        manifest,
        findProjectRoot()
    );

    bool saw_missing_heavy_config = false;
    bool saw_missing_heavy_asset = false;
    for (const ValidationReferenceIssue& issue : issues) {
        if (issue.scene_id == "heavy_optional" && issue.kind == "missing_config") {
            saw_missing_heavy_config = true;
        }
        if (issue.scene_id == "heavy_optional" && issue.kind == "missing_asset") {
            saw_missing_heavy_asset = true;
        }
    }
    test::require(saw_missing_heavy_config, "missing heavy config should be reported");
    test::require(saw_missing_heavy_asset, "missing heavy asset should be reported");
}

void testRepositoryManifestDryRunContract() {
    const std::filesystem::path manifest_path = findProjectRoot() / "validation/scenes.yaml";
    ValidationManifest manifest = loadValidationManifest(manifest_path);
    ValidationSelection selection;
    selection.default_only = true;
    std::vector<ValidationScene> scenes = selectValidationScenes(manifest, selection);

    test::require(!scenes.empty(), "repository manifest should have default scenes");
    test::require(manifest.proxy_approved_ports.size() == 3, "repository manifest should preserve approved proxy ports");
    for (const ValidationScene& scene : scenes) {
        test::require(scene.tier != ValidationTier::Heavy, "repository default selection should exclude heavy");
        test::require(!scene.config.empty(), "selected scene should have config path");
    }
}

}  // namespace

int main() {
    test::Runner runner;
    runner.run("Validation manifest loads scenes and proxy ports", testManifestLoadsScenesAndProxyPorts);
    runner.run("Default selection excludes heavy", testDefaultSelectionExcludesHeavy);
    runner.run("Tier and tag selection", testTierAndTagSelection);
    runner.run("Reference validation reports missing assets", testReferenceValidationReportsMissingAssets);
    runner.run("Repository manifest dry-run contract", testRepositoryManifestDryRunContract);
    return runner.finish();
}
