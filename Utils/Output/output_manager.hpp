#ifndef OUTPUT_MANAGER_HPP_
#define OUTPUT_MANAGER_HPP_

#include <filesystem>
#include <string>
#include <vector>

class OutputManager {
    std::filesystem::path output_root;
    std::string scene_name;

public:
    OutputManager(std::string root, std::string scene_name);

    std::filesystem::path sceneRoot() const;
    std::filesystem::path gbufferDir() const;
    std::filesystem::path shadowDir() const;
    std::filesystem::path colorPath() const;

    void prepare(const std::vector<std::string>& buffers) const;
    void publishLegacyRendererOutputs(const std::filesystem::path& working_dir = std::filesystem::current_path()) const;
    void publishLegacyRendererOutputs(
        const std::vector<std::string>& buffers,
        const std::filesystem::path& working_dir = std::filesystem::current_path()
    ) const;
};

#endif  // OUTPUT_MANAGER_HPP_
