#ifndef VALIDATION_MANIFEST_HPP_
#define VALIDATION_MANIFEST_HPP_

#include <filesystem>
#include <string>
#include <vector>

enum class ValidationTier {
    Fast,
    Standard,
    Heavy
};

struct ValidationScene {
    std::string id;
    ValidationTier tier = ValidationTier::Fast;
    std::string config;
    bool run_by_default = false;
    std::vector<std::string> tags;
    std::vector<std::string> assets;
    std::vector<std::string> expected_outputs;
    int runtime_budget_seconds = 0;
    std::string source_type;
    std::string license;
};

struct ValidationManifest {
    int version = 1;
    std::vector<int> proxy_approved_ports;
    std::vector<ValidationScene> scenes;
};

struct ValidationSelection {
    bool default_only = true;
    std::vector<ValidationTier> tiers;
    std::vector<std::string> tags;
};

struct ValidationReferenceIssue {
    std::string scene_id;
    std::string kind;
    std::string path;
    std::string message;
};

ValidationTier parseValidationTier(const std::string& value);
std::string validationTierName(ValidationTier tier);

ValidationManifest loadValidationManifest(const std::filesystem::path& manifest_path);
std::vector<ValidationScene> selectValidationScenes(
    const ValidationManifest& manifest,
    const ValidationSelection& selection
);
std::vector<ValidationReferenceIssue> validateValidationSceneReferences(
    const ValidationManifest& manifest,
    const std::filesystem::path& project_root
);

#endif  // VALIDATION_MANIFEST_HPP_
