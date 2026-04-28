#ifndef FLOAT_BUFFER_WRITER_HPP_
#define FLOAT_BUFFER_WRITER_HPP_

#include <filesystem>
#include <string>
#include <vector>

struct FloatBuffer {
    std::string name;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<float> data;
};

void writeFloatBuffer(const std::filesystem::path& output_path, const FloatBuffer& buffer);

#endif  // FLOAT_BUFFER_WRITER_HPP_
