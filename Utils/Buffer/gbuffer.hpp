#ifndef GBUFFER_HPP_
#define GBUFFER_HPP_

#include "cores.hpp"
#include "float_buffer_writer.hpp"
#include <string>
#include <vector>

struct BufferChannelInfo {
    std::string name;
    int components = 1;
    std::string semantic;
};

struct GBufferSample {
    Vec3f world_position = Vec3f::Zero();
    Vec3f view_position = Vec3f::Zero();
    float linear_depth = 0.0f;
    Vec3f world_normal = Vec3f::Zero();
    Vec3f view_normal = Vec3f::Zero();
    Vec3f albedo = Vec3f::Zero();
    int material_id = 0;
    float roughness = 0.5f;
    float metallic = 0.0f;
    float shadow_visibility = 0.0f;
};

class GBuffer {
    Vec2i resolution;
    std::vector<GBufferSample> samples;

    int index(int x, int y) const;

public:
    explicit GBuffer(Vec2i resolution);

    Vec2i getResolution() const { return resolution; }
    int getWidth() const { return resolution.x(); }
    int getHeight() const { return resolution.y(); }

    void writePixel(int x, int y, const GBufferSample& sample);
    GBufferSample sample(int x, int y) const;

    std::vector<float> normalizedLinearDepthPreview() const;
    std::vector<FloatBuffer> floatBuffers() const;

    static std::vector<BufferChannelInfo> channelMetadata();
};

#endif  // GBUFFER_HPP_
