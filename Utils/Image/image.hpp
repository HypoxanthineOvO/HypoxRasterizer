#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <vector>
#include <string>

#include "utils.hpp"

struct ImageData {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<Vec4f> pixels;
};

ImageData readImageDataFromFile(const std::string& file_name);
std::vector<Vec4f> readImageFromFile(const std::string& file_name);

void writeImageToFile(std::vector<Vec3f>& data, Vec2i resolution, const std::string& file_name);
void writeImageToFile(std::vector<float>& data, Vec2i resolution, const std::string& file_name);

#endif // IMAGE_HPP
