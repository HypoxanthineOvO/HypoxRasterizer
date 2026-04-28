#include "output_manager.hpp"

#include <algorithm>
#include <utility>

namespace {

bool hasBuffer(const std::vector<std::string>& buffers, const std::string& value) {
    return buffers.empty() || std::find(buffers.begin(), buffers.end(), value) != buffers.end();
}

void copyIfExists(const std::filesystem::path& from, const std::filesystem::path& to) {
    if (std::filesystem::exists(from)) {
        std::filesystem::create_directories(to.parent_path());
        std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
    }
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

}  // namespace

OutputManager::OutputManager(std::string root, std::string scene_name)
    : output_root(std::move(root)), scene_name(std::move(scene_name)) {}

std::filesystem::path OutputManager::sceneRoot() const {
    return output_root / scene_name;
}

std::filesystem::path OutputManager::gbufferDir() const {
    return sceneRoot() / "gbuffer";
}

std::filesystem::path OutputManager::shadowDir() const {
    return sceneRoot() / "shadow";
}

std::filesystem::path OutputManager::colorPath() const {
    return sceneRoot() / "color.png";
}

void OutputManager::prepare(const std::vector<std::string>& buffers) const {
    std::filesystem::create_directories(sceneRoot());
    if (hasBuffer(buffers, "gbuffer")) {
        std::filesystem::remove_all(gbufferDir());
        std::filesystem::create_directories(gbufferDir());
    }
    if (hasBuffer(buffers, "shadow")) {
        std::filesystem::remove_all(shadowDir());
        std::filesystem::create_directories(shadowDir());
    }
}

void OutputManager::publishLegacyRendererOutputs(const std::filesystem::path& working_dir) const {
    publishLegacyRendererOutputs({}, working_dir);
}

void OutputManager::publishLegacyRendererOutputs(
    const std::vector<std::string>& buffers,
    const std::filesystem::path& working_dir
) const {
    std::filesystem::create_directories(sceneRoot());
    if (hasBuffer(buffers, "color")) {
        copyIfExists(working_dir / "color.png", colorPath());
    }
    if (hasBuffer(buffers, "gbuffer")) {
        copyIfExists(working_dir / "depth.png", gbufferDir() / "depth.png");
        copyIfExists(working_dir / "normal.png", gbufferDir() / "normal.png");
    }

    if (hasBuffer(buffers, "shadow")) {
        for (const auto& entry : std::filesystem::directory_iterator(working_dir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const std::string name = entry.path().filename().string();
            if (startsWith(name, "PointlightShadowMap") ||
                startsWith(name, "DirectionalLightShadowMap") ||
                startsWith(name, "ArealightShadowMap")) {
                copyIfExists(entry.path(), shadowDir() / name);
            }
        }
    }
}
