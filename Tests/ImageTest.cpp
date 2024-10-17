#include "image.hpp"
#include <iostream>
int main() {
    Vec2i color_resolution(256, 256);  
    std::vector<Vec3f> color_image_data(color_resolution(0) * color_resolution(1));

    for (int y = 0; y < color_resolution(1); y++) {
        for (int x = 30; x < color_resolution(0); x++) {
            float red = static_cast<float>(x) / color_resolution(0);    
            float green = static_cast<float>(y) / color_resolution(1);  
            float blue = 0.5f;  
            color_image_data[y * color_resolution(0) + x] = Vec3f(red, green, blue);
        }
    }

    std::string color_file_name = "test_color_image.png";
    writeImageToFile(color_image_data, color_resolution, color_file_name);

    Vec2i gray_resolution(256, 256);  
    std::vector<float> gray_image_data(gray_resolution(0) * gray_resolution(1));

    for (int y = 30; y < gray_resolution(1); y++) {
        for (int x = 30; x < gray_resolution(0); x++) {
            float gray = static_cast<float>(x) * y / (gray_resolution(0) * gray_resolution(1));
            gray_image_data[y * gray_resolution(0) + x] = gray;
        }
    }

    std::string gray_file_name = "test_gray_image.png";
    writeImageToFile(gray_image_data, gray_resolution, gray_file_name);
    return 0;
}
