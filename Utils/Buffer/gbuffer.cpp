#include "gbuffer.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

void appendVec3(std::vector<float>& data, const Vec3f& value) {
    data.push_back(value.x());
    data.push_back(value.y());
    data.push_back(value.z());
}

FloatBuffer makeBuffer(const std::string& name, int width, int height, int channels, std::vector<float> data) {
    FloatBuffer buffer;
    buffer.name = name;
    buffer.width = width;
    buffer.height = height;
    buffer.channels = channels;
    buffer.data = std::move(data);
    return buffer;
}

}  // namespace

GBuffer::GBuffer(Vec2i resolution) : resolution(resolution) {
    if (resolution.x() <= 0 || resolution.y() <= 0) {
        throw std::runtime_error("GBuffer resolution must be positive");
    }
    samples.resize(static_cast<size_t>(resolution.x()) * static_cast<size_t>(resolution.y()));
}

int GBuffer::index(int x, int y) const {
    if (x < 0 || x >= resolution.x() || y < 0 || y >= resolution.y()) {
        throw std::runtime_error("GBuffer pixel index out of range");
    }
    return y * resolution.x() + x;
}

void GBuffer::writePixel(int x, int y, const GBufferSample& sample) {
    samples[index(x, y)] = sample;
}

GBufferSample GBuffer::sample(int x, int y) const {
    return samples[index(x, y)];
}

std::vector<float> GBuffer::normalizedLinearDepthPreview() const {
    float min_depth = std::numeric_limits<float>::infinity();
    float max_depth = -std::numeric_limits<float>::infinity();
    for (const auto& item : samples) {
        if (std::isfinite(item.linear_depth)) {
            min_depth = std::min(min_depth, item.linear_depth);
            max_depth = std::max(max_depth, item.linear_depth);
        }
    }

    std::vector<float> preview(samples.size(), 0.0f);
    if (!std::isfinite(min_depth) || !std::isfinite(max_depth) || std::fabs(max_depth - min_depth) < 1e-8f) {
        return preview;
    }

    for (size_t i = 0; i < samples.size(); ++i) {
        float depth = samples[i].linear_depth;
        preview[i] = std::isfinite(depth) ? (depth - min_depth) / (max_depth - min_depth) : 0.0f;
    }
    return preview;
}

std::vector<FloatBuffer> GBuffer::floatBuffers() const {
    const int width = resolution.x();
    const int height = resolution.y();
    std::vector<float> world_position;
    std::vector<float> view_position;
    std::vector<float> linear_depth;
    std::vector<float> world_normal;
    std::vector<float> view_normal;
    std::vector<float> albedo;
    std::vector<float> material_id;
    std::vector<float> roughness;
    std::vector<float> metallic;
    std::vector<float> shadow_visibility;

    world_position.reserve(samples.size() * 3);
    view_position.reserve(samples.size() * 3);
    linear_depth.reserve(samples.size());
    world_normal.reserve(samples.size() * 3);
    view_normal.reserve(samples.size() * 3);
    albedo.reserve(samples.size() * 3);
    material_id.reserve(samples.size());
    roughness.reserve(samples.size());
    metallic.reserve(samples.size());
    shadow_visibility.reserve(samples.size());

    for (const auto& item : samples) {
        appendVec3(world_position, item.world_position);
        appendVec3(view_position, item.view_position);
        linear_depth.push_back(item.linear_depth);
        appendVec3(world_normal, item.world_normal);
        appendVec3(view_normal, item.view_normal);
        appendVec3(albedo, item.albedo);
        material_id.push_back(static_cast<float>(item.material_id));
        roughness.push_back(item.roughness);
        metallic.push_back(item.metallic);
        shadow_visibility.push_back(item.shadow_visibility);
    }

    return {
        makeBuffer("world_position", width, height, 3, std::move(world_position)),
        makeBuffer("view_position", width, height, 3, std::move(view_position)),
        makeBuffer("linear_depth", width, height, 1, std::move(linear_depth)),
        makeBuffer("world_normal", width, height, 3, std::move(world_normal)),
        makeBuffer("view_normal", width, height, 3, std::move(view_normal)),
        makeBuffer("albedo", width, height, 3, std::move(albedo)),
        makeBuffer("material_id", width, height, 1, std::move(material_id)),
        makeBuffer("roughness", width, height, 1, std::move(roughness)),
        makeBuffer("metallic", width, height, 1, std::move(metallic)),
        makeBuffer("shadow_visibility", width, height, 1, std::move(shadow_visibility)),
    };
}

std::vector<BufferChannelInfo> GBuffer::channelMetadata() {
    return {
        {"world_position", 3, "World-space fragment position"},
        {"view_position", 3, "View-space fragment position"},
        {"linear_depth", 1, "Linear depth value"},
        {"world_normal", 3, "World-space normal"},
        {"view_normal", 3, "View-space normal"},
        {"albedo", 3, "Material base color"},
        {"material_id", 1, "Numeric material identifier"},
        {"roughness", 1, "Material roughness"},
        {"metallic", 1, "Material metallic"},
        {"shadow_visibility", 1, "Fraction of visible direct lights after shadow queries"},
    };
}
