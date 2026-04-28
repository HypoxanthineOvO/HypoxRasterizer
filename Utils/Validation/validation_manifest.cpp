#include "validation_manifest.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

std::string trim(const std::string& value) {
    size_t begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }
    size_t end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(begin, end - begin);
}

std::string stripComment(const std::string& value) {
    bool in_quote = false;
    char quote = '\0';
    for (size_t i = 0; i < value.size(); ++i) {
        char ch = value[i];
        if ((ch == '"' || ch == '\'') && (i == 0 || value[i - 1] != '\\')) {
            if (!in_quote) {
                in_quote = true;
                quote = ch;
            } else if (quote == ch) {
                in_quote = false;
            }
        }
        if (ch == '#' && !in_quote) {
            return value.substr(0, i);
        }
    }
    return value;
}

std::string unquote(std::string value) {
    value = trim(value);
    if (value.size() >= 2) {
        char first = value.front();
        char last = value.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            return value.substr(1, value.size() - 2);
        }
    }
    return value;
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

std::pair<std::string, std::string> parseKeyValue(const std::string& line) {
    const size_t colon = line.find(':');
    if (colon == std::string::npos) {
        throw std::runtime_error("Expected key: value line: " + line);
    }
    return {trim(line.substr(0, colon)), trim(line.substr(colon + 1))};
}

std::vector<std::string> parseStringList(std::string value) {
    value = trim(value);
    if (value.empty()) {
        return {};
    }
    if (value.front() == '[' && value.back() == ']') {
        value = value.substr(1, value.size() - 2);
    }

    std::vector<std::string> result;
    std::stringstream stream(value);
    std::string item;
    while (std::getline(stream, item, ',')) {
        item = unquote(item);
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

std::vector<int> parseIntList(const std::string& value) {
    std::vector<int> result;
    for (const std::string& item : parseStringList(value)) {
        result.push_back(std::stoi(item));
    }
    return result;
}

bool parseBool(const std::string& value) {
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    if (lower == "true" || lower == "yes" || lower == "1") {
        return true;
    }
    if (lower == "false" || lower == "no" || lower == "0") {
        return false;
    }
    throw std::runtime_error("Expected boolean value, got: " + value);
}

bool containsString(const std::vector<std::string>& values, const std::string& needle) {
    return std::find(values.begin(), values.end(), needle) != values.end();
}

bool containsTier(const std::vector<ValidationTier>& values, ValidationTier tier) {
    return std::find(values.begin(), values.end(), tier) != values.end();
}

void validateSceneShape(const ValidationScene& scene) {
    if (scene.id.empty()) {
        throw std::runtime_error("Validation scene is missing id");
    }
    if (scene.config.empty()) {
        throw std::runtime_error("Validation scene '" + scene.id + "' is missing config");
    }
    if (scene.expected_outputs.empty()) {
        throw std::runtime_error("Validation scene '" + scene.id + "' is missing expected_outputs");
    }
}

}  // namespace

ValidationTier parseValidationTier(const std::string& value) {
    if (value == "fast") return ValidationTier::Fast;
    if (value == "standard") return ValidationTier::Standard;
    if (value == "heavy") return ValidationTier::Heavy;
    throw std::runtime_error("Unknown validation tier: " + value);
}

std::string validationTierName(ValidationTier tier) {
    switch (tier) {
        case ValidationTier::Fast:
            return "fast";
        case ValidationTier::Standard:
            return "standard";
        case ValidationTier::Heavy:
            return "heavy";
    }
    return "unknown";
}

ValidationManifest loadValidationManifest(const std::filesystem::path& manifest_path) {
    std::ifstream input(manifest_path);
    if (!input.is_open()) {
        throw std::runtime_error("Cannot open validation manifest: " + manifest_path.string());
    }

    ValidationManifest manifest;
    enum class Section {
        Root,
        Proxy,
        Scenes
    };
    Section section = Section::Root;
    ValidationScene* current_scene = nullptr;

    std::string raw_line;
    while (std::getline(input, raw_line)) {
        std::string line = trim(stripComment(raw_line));
        if (line.empty()) {
            continue;
        }

        if (line == "proxy:") {
            section = Section::Proxy;
            current_scene = nullptr;
            continue;
        }
        if (line == "scenes:") {
            section = Section::Scenes;
            current_scene = nullptr;
            continue;
        }

        if (section == Section::Root) {
            auto [key, value] = parseKeyValue(line);
            if (key == "version") {
                manifest.version = std::stoi(value);
            }
            continue;
        }

        if (section == Section::Proxy) {
            auto [key, value] = parseKeyValue(line);
            if (key == "approved_ports") {
                manifest.proxy_approved_ports = parseIntList(value);
            }
            continue;
        }

        if (section == Section::Scenes) {
            if (startsWith(line, "- ")) {
                manifest.scenes.push_back(ValidationScene{});
                current_scene = &manifest.scenes.back();
                line = trim(line.substr(2));
                if (line.empty()) {
                    continue;
                }
            }
            if (current_scene == nullptr) {
                throw std::runtime_error("Scene field without active scene: " + line);
            }

            auto [key, value] = parseKeyValue(line);
            value = unquote(value);
            if (key == "id") {
                current_scene->id = value;
            } else if (key == "tier") {
                current_scene->tier = parseValidationTier(value);
            } else if (key == "config") {
                current_scene->config = value;
            } else if (key == "default") {
                current_scene->run_by_default = parseBool(value);
            } else if (key == "tags") {
                current_scene->tags = parseStringList(value);
            } else if (key == "assets") {
                current_scene->assets = parseStringList(value);
            } else if (key == "expected_outputs") {
                current_scene->expected_outputs = parseStringList(value);
            } else if (key == "runtime_budget_seconds") {
                current_scene->runtime_budget_seconds = std::stoi(value);
            } else if (key == "source_type") {
                current_scene->source_type = value;
            } else if (key == "license") {
                current_scene->license = value;
            }
        }
    }

    if (manifest.scenes.empty()) {
        throw std::runtime_error("Validation manifest has no scenes: " + manifest_path.string());
    }
    for (const ValidationScene& scene : manifest.scenes) {
        validateSceneShape(scene);
    }
    return manifest;
}

std::vector<ValidationScene> selectValidationScenes(
    const ValidationManifest& manifest,
    const ValidationSelection& selection
) {
    std::vector<ValidationScene> selected;
    for (const ValidationScene& scene : manifest.scenes) {
        if (selection.default_only && !scene.run_by_default) {
            continue;
        }
        if (selection.default_only && scene.tier == ValidationTier::Heavy) {
            continue;
        }
        if (!selection.tiers.empty() && !containsTier(selection.tiers, scene.tier)) {
            continue;
        }

        bool tags_match = true;
        for (const std::string& tag : selection.tags) {
            if (!containsString(scene.tags, tag)) {
                tags_match = false;
                break;
            }
        }
        if (!tags_match) {
            continue;
        }
        selected.push_back(scene);
    }
    return selected;
}

std::vector<ValidationReferenceIssue> validateValidationSceneReferences(
    const ValidationManifest& manifest,
    const std::filesystem::path& project_root
) {
    std::vector<ValidationReferenceIssue> issues;
    for (const ValidationScene& scene : manifest.scenes) {
        const std::filesystem::path config_path = project_root / scene.config;
        if (!std::filesystem::exists(config_path)) {
            issues.push_back({
                scene.id,
                "missing_config",
                scene.config,
                "Missing validation scene config: " + scene.config
            });
        }
        for (const std::string& asset : scene.assets) {
            const std::filesystem::path asset_path = project_root / asset;
            if (!std::filesystem::exists(asset_path)) {
                issues.push_back({
                    scene.id,
                    "missing_asset",
                    asset,
                    "Missing validation scene asset: " + asset
                });
            }
        }
    }
    return issues;
}
