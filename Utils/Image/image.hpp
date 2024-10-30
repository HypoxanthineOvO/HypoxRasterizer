#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <vector>
#include <string>

#include "utils.hpp"

std::vector<Vec4f> readImageFromFile(const std::string& file_name);

void writeImageToFile(std::vector<Vec3f>& data, Vec2i resolution, const std::string& file_name);
void writeImageToFile(std::vector<float>& data, Vec2i resolution, const std::string& file_name);

#endif // IMAGE_HPP