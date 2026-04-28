#include "float_buffer_writer.hpp"

#include <fstream>
#include <stdexcept>

void writeFloatBuffer(const std::filesystem::path& output_path, const FloatBuffer& buffer) {
    if (buffer.width <= 0 || buffer.height <= 0 || buffer.channels <= 0) {
        throw std::runtime_error("FloatBuffer dimensions must be positive");
    }
    const size_t expected_size = static_cast<size_t>(buffer.width) *
                                 static_cast<size_t>(buffer.height) *
                                 static_cast<size_t>(buffer.channels);
    if (buffer.data.size() != expected_size) {
        throw std::runtime_error("FloatBuffer payload size does not match dimensions");
    }

    std::filesystem::create_directories(output_path.parent_path());
    std::ofstream output(output_path, std::ios::binary);
    if (!output.is_open()) {
        throw std::runtime_error("Cannot open float buffer output: " + output_path.string());
    }

    output << "HYPORASTER_F32_V1\n";
    output << buffer.name << "\n";
    output << buffer.width << " " << buffer.height << " " << buffer.channels << "\n";
    output.write(
        reinterpret_cast<const char*>(buffer.data.data()),
        static_cast<std::streamsize>(buffer.data.size() * sizeof(float))
    );
}
