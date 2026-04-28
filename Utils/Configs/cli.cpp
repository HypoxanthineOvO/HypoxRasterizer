#include "cli.hpp"

#include <sstream>
#include <stdexcept>

namespace {

std::vector<std::string> splitCommaList(const std::string& value) {
    std::vector<std::string> items;
    std::stringstream ss(value);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            items.push_back(item);
        }
    }
    return items;
}

Vec2i parseResolution(const std::string& value) {
    size_t xpos = value.find('x');
    if (xpos == std::string::npos || xpos == 0 || xpos == value.size() - 1) {
        throw std::runtime_error("Resolution must use WIDTHxHEIGHT format");
    }
    int width = std::stoi(value.substr(0, xpos));
    int height = std::stoi(value.substr(xpos + 1));
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Resolution must be positive");
    }
    return Vec2i(width, height);
}

std::string requireValue(int& index, int argc, const char* const* argv, const std::string& option) {
    if (index + 1 >= argc) {
        throw std::runtime_error(option + " requires a value");
    }
    ++index;
    return argv[index];
}

}  // namespace

CliOptions parseCliOptions(int argc, const char* const* argv) {
    CliOptions options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config") {
            options.config_path = requireValue(i, argc, argv, arg);
        } else if (arg == "--output-root") {
            options.output_root = requireValue(i, argc, argv, arg);
        } else if (arg == "--buffers") {
            options.buffers = splitCommaList(requireValue(i, argc, argv, arg));
        } else if (arg == "--resolution") {
            options.resolution_override = parseResolution(requireValue(i, argc, argv, arg));
            options.has_resolution_override = true;
        } else if (arg == "--debug") {
            options.debug = true;
        } else if (arg == "--verbose") {
            options.verbose = true;
        } else if (!arg.empty() && arg[0] == '-') {
            throw std::runtime_error("Unknown CLI option: " + arg);
        } else if (options.config_path.empty()) {
            options.config_path = arg;
        } else {
            throw std::runtime_error("Unexpected positional argument: " + arg);
        }
    }
    return options;
}
