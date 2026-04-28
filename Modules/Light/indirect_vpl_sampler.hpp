#ifndef INDIRECT_VPL_SAMPLER_HPP_
#define INDIRECT_VPL_SAMPLER_HPP_

#include "configs.hpp"
#include "light.hpp"
#include "object.hpp"

#include <memory>
#include <vector>

struct RayHit {
    bool hit = false;
    Vec3f position = Vec3f::Zero();
    Vec3f normal = Vec3f(0, 1, 0);
    float t = 0.0f;
    int object_index = -1;
    int triangle_index = -1;
};

RayHit intersectTriangleFirstHit(const Vec3f& origin, const Vec3f& direction, const Triangle& triangle);

RayHit intersectSceneFirstHit(
    const Vec3f& origin,
    const Vec3f& direction,
    const std::vector<std::shared_ptr<Object>>& objects
);

std::vector<IndirectVPL> sampleFirstBounceIndirectVPLs(
    const Light& light,
    const std::vector<std::shared_ptr<Object>>& objects,
    const IndirectVPLConfig& config,
    int source_light_index
);

Vec3f evaluateIndirectVPLContribution(
    const Vec3f& position,
    const Vec3f& normal,
    const Vec3f& albedo,
    const IndirectVPL& sample,
    float clamp
);

#endif  // INDIRECT_VPL_SAMPLER_HPP_
