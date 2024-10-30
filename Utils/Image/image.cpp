
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "image.hpp"

inline float clamp01(float v) {
    if (v > 1) v = 1;
    else if (v < 0) v = 0;
    return v;
}

static inline uint8_t gammaCorrection(float radiance) {
	return static_cast<uint8_t>(ceil(255 * pow(clamp01(radiance), 1 / 2.2)));
}

std::vector<Vec3f> readImageFromFile(const std::string& file_name) {
    int width, height, channels;
    unsigned char* data = stbi_load(file_name.c_str(), &width, &height, &channels, 3);
    if (!data) {
        throw std::runtime_error("Failed to load image: " + file_name);
    }
    std::vector<Vec3f> image_data(width * height);
    for (int i = 0; i < width * height; i++) {
        image_data[i] = Vec3f(data[3 * i] / 255.0f, data[3 * i + 1] / 255.0f, data[3 * i + 2] / 255.0f);
    }
    stbi_image_free(data);
    return image_data;
}

void writeImageToFile(std::vector<Vec3f>& data, Vec2i resolution, const std::string& file_name) {
	std::vector<uint8_t> rgb_data(resolution.x() * resolution.y() * 3);
	for (int i = 0; i < data.size(); i++) {
		rgb_data[3 * i] = gammaCorrection(data[i].x());
		rgb_data[3 * i + 1] = gammaCorrection(data[i].y());
		rgb_data[3 * i + 2] = gammaCorrection(data[i].z());
	}
	stbi_flip_vertically_on_write(true);
	stbi_write_png(file_name.c_str(), resolution.x(), resolution.y(), 3, rgb_data.data(), 0);
}

void writeImageToFile(std::vector<float>& data, Vec2i resolution, const std::string& file_name) {
    // Construct Image data
    std::vector<Vec3f> image_data(resolution(0) * resolution(1));
    for (int i = 0; i < resolution(0) * resolution(1); i++) {
        image_data[i] = Vec3f(data[i], data[i], data[i]);
    }
    writeImageToFile(image_data, resolution, file_name);
}