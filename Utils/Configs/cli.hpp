#ifndef CLI_HPP_
#define CLI_HPP_

#include "cores.hpp"
#include <string>
#include <vector>

struct CliOptions {
    std::string config_path;
    std::string output_root = "outputs";
    std::vector<std::string> buffers;
    bool has_resolution_override = false;
    Vec2i resolution_override = Vec2i(0, 0);
    bool debug = false;
    bool verbose = false;
};

CliOptions parseCliOptions(int argc, const char* const* argv);

#endif  // CLI_HPP_
