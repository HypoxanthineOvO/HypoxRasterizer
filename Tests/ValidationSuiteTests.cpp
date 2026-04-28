#include "TestUtils.hpp"

#include "scene_config.hpp"
#include "validation_manifest.hpp"

#include <filesystem>
#include <set>
#include <string>
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

ValidationManifest loadRepoManifest() {
    return loadValidationManifest(findProjectRoot() / "validation/scenes.yaml");
}

const ValidationScene& findScene(const ValidationManifest& manifest, const std::string& id) {
    for (const ValidationScene& scene : manifest.scenes) {
        if (scene.id == id) {
            return scene;
        }
    }
    throw test::Failure("missing validation scene: " + id);
}

bool hasTag(const ValidationScene& scene, const std::string& tag) {
    return std::find(scene.tags.begin(), scene.tags.end(), tag) != scene.tags.end();
}

void requireConfigLoads(const ValidationScene& scene) {
    const std::filesystem::path config_path = findProjectRoot() / scene.config;
    test::require(std::filesystem::exists(config_path), "config should exist for " + scene.id);
    SceneConfig config = loadSceneConfig(config_path.string());
    test::require(!config.name.empty(), "scene config should have a name for " + scene.id);
}

SceneConfig loadValidationConfig(const ValidationScene& scene) {
    const std::filesystem::path config_path = findProjectRoot() / scene.config;
    test::require(std::filesystem::exists(config_path), "config should exist for " + scene.id);
    return loadSceneConfig(config_path.string());
}

void testFastCornerCasesAreRegistered() {
    ValidationManifest manifest = loadRepoManifest();
    const std::vector<std::string> required = {
        "fast_empty_scene",
        "fast_offscreen_triangle",
        "fast_multi_light_shadow",
        "fast_missing_asset_error"
    };

    for (const std::string& id : required) {
        const ValidationScene& scene = findScene(manifest, id);
        test::require(scene.tier == ValidationTier::Fast, id + " should be fast tier");
        test::require(scene.config.rfind("configs/validation/fast/", 0) == 0, id + " should live under fast configs");
        requireConfigLoads(scene);
    }
}

void testDefaultFastSelectionSkipsExpectedFailures() {
    ValidationManifest manifest = loadRepoManifest();
    ValidationSelection selection;
    selection.default_only = true;
    selection.tiers = {ValidationTier::Fast};
    std::vector<ValidationScene> scenes = selectValidationScenes(manifest, selection);

    bool saw_empty = false;
    bool saw_missing_asset = false;
    for (const ValidationScene& scene : scenes) {
        if (scene.id == "fast_empty_scene") saw_empty = true;
        if (scene.id == "fast_missing_asset_error") saw_missing_asset = true;
    }
    test::require(saw_empty, "default fast suite should include empty scene");
    test::require(!saw_missing_asset, "expected-failure scene should not run by default");
}

void testStandardScenesCoverTextureAndMediumMeshes() {
    ValidationManifest manifest = loadRepoManifest();
    const ValidationScene& texture = findScene(manifest, "standard_money_texture");
    const ValidationScene& bunny = findScene(manifest, "standard_bunny_mesh");
    const ValidationScene& dragon = findScene(manifest, "standard_dragon_quick");

    test::require(texture.tier == ValidationTier::Standard, "texture scene should be standard");
    test::require(hasTag(texture, "texture"), "texture scene should have texture tag");
    test::require(hasTag(bunny, "bunny"), "bunny scene should have bunny tag");
    test::require(hasTag(dragon, "dragon"), "dragon scene should have dragon tag");
    test::require(hasTag(dragon, "quick"), "dragon quick scene should be tagged quick");

    requireConfigLoads(texture);
    requireConfigLoads(bunny);
    requireConfigLoads(dragon);

    SceneConfig texture_config = loadValidationConfig(texture);
    test::require(!texture_config.materials.empty(), "texture scene should define materials");
    test::require(
        texture_config.materials[0].texture_file == "./assets/Money/2.png",
        "texture scene should preserve texture file path"
    );
}

void testMissingAssetCornerCaseIsDiagnosed() {
    ValidationManifest manifest = loadRepoManifest();
    std::vector<ValidationReferenceIssue> issues = validateValidationSceneReferences(
        manifest,
        findProjectRoot()
    );

    bool saw_missing_asset_issue = false;
    for (const ValidationReferenceIssue& issue : issues) {
        if (issue.scene_id == "fast_missing_asset_error" && issue.kind == "missing_asset") {
            saw_missing_asset_issue = true;
        }
    }
    test::require(saw_missing_asset_issue, "missing asset corner case should be diagnosed");
}

void testStandardSelectionHasRequiredCoverageTags() {
    ValidationManifest manifest = loadRepoManifest();
    ValidationSelection selection;
    selection.default_only = false;
    selection.tiers = {ValidationTier::Standard};
    std::vector<ValidationScene> scenes = selectValidationScenes(manifest, selection);

    std::set<std::string> tags;
    for (const ValidationScene& scene : scenes) {
        tags.insert(scene.tags.begin(), scene.tags.end());
    }
    test::require(tags.count("texture") > 0, "standard suite should include texture coverage");
    test::require(tags.count("cornell") > 0, "standard suite should include Cornell coverage");
    test::require(tags.count("dragon") > 0 || tags.count("bunny") > 0, "standard suite should include medium mesh coverage");
}

void testIndirectVPLValidationSceneIsRegistered() {
    ValidationManifest manifest = loadRepoManifest();
    const ValidationScene& scene = findScene(manifest, "standard_cornell_indirect_vpl");

    test::require(scene.tier == ValidationTier::Standard, "IndirectVPL scene should be standard");
    test::require(hasTag(scene, "indirect_vpl"), "IndirectVPL scene should have indirect_vpl tag");
    test::require(hasTag(scene, "cornell"), "IndirectVPL scene should have cornell tag");
    SceneConfig config = loadValidationConfig(scene);
    test::require(!config.lights.empty(), "IndirectVPL scene should define lights");
    test::require(config.lights[0].indirect_vpl.enabled, "IndirectVPL scene should enable indirectVPL");
    test::require(config.lights[0].indirect_vpl.samples >= 16, "IndirectVPL scene should use enough samples");
}

}  // namespace

int main() {
    test::Runner runner;
    runner.run("Fast corner cases are registered", testFastCornerCasesAreRegistered);
    runner.run("Default fast selection skips expected failures", testDefaultFastSelectionSkipsExpectedFailures);
    runner.run("Standard scenes cover texture and medium meshes", testStandardScenesCoverTextureAndMediumMeshes);
    runner.run("Missing asset corner case is diagnosed", testMissingAssetCornerCaseIsDiagnosed);
    runner.run("Standard selection has required coverage tags", testStandardSelectionHasRequiredCoverageTags);
    runner.run("IndirectVPL validation scene is registered", testIndirectVPLValidationSceneIsRegistered);
    return runner.finish();
}
