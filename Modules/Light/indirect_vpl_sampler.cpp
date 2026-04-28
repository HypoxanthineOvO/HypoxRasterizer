#include "indirect_vpl_sampler.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

constexpr float kRayEpsilon = 1e-5f;

Vec3f normalizedOrDown(const Vec3f& direction) {
    if (direction.norm() < kRayEpsilon) {
        return Vec3f(0, -1, 0);
    }
    return direction.normalized();
}

Vec3f deterministicDirection(int index, int count) {
    if (count <= 1 || index == 0) {
        return Vec3f(0, -1, 0);
    }
    const float angle = static_cast<float>(index) * 2.39996323f;
    const float radius = 0.12f * std::sqrt(static_cast<float>(index) / static_cast<float>(std::max(1, count - 1)));
    Vec3f direction(
        std::cos(angle) * radius,
        -1.0f,
        std::sin(angle) * radius
    );
    return direction.normalized();
}

Vec3f triangleNormal(const Triangle& triangle) {
    Vec3f p0 = triangle.getVertex(0).position;
    Vec3f p1 = triangle.getVertex(1).position;
    Vec3f p2 = triangle.getVertex(2).position;
    Vec3f normal = (p1 - p0).cross(p2 - p0);
    if (normal.norm() < kRayEpsilon) {
        return Vec3f(0, 1, 0);
    }
    return normal.normalized();
}

}  // namespace

RayHit intersectTriangleFirstHit(const Vec3f& origin, const Vec3f& direction, const Triangle& triangle) {
    const Vec3f dir = normalizedOrDown(direction);
    const Vec3f v0 = triangle.getVertex(0).position;
    const Vec3f v1 = triangle.getVertex(1).position;
    const Vec3f v2 = triangle.getVertex(2).position;
    const Vec3f edge1 = v1 - v0;
    const Vec3f edge2 = v2 - v0;
    const Vec3f h = dir.cross(edge2);
    const float a = edge1.dot(h);
    if (std::abs(a) < kRayEpsilon) {
        return RayHit();
    }
    const float f = 1.0f / a;
    const Vec3f s = origin - v0;
    const float u = f * s.dot(h);
    if (u < 0.0f || u > 1.0f) {
        return RayHit();
    }
    const Vec3f q = s.cross(edge1);
    const float v = f * dir.dot(q);
    if (v < 0.0f || u + v > 1.0f) {
        return RayHit();
    }
    const float t = f * edge2.dot(q);
    if (t <= kRayEpsilon) {
        return RayHit();
    }

    RayHit hit;
    hit.hit = true;
    hit.t = t;
    hit.position = origin + dir * t;
    hit.normal = triangleNormal(triangle);
    if (hit.normal.dot(-dir) < 0.0f) {
        hit.normal = -hit.normal;
    }
    return hit;
}

RayHit intersectSceneFirstHit(
    const Vec3f& origin,
    const Vec3f& direction,
    const std::vector<std::shared_ptr<Object>>& objects
) {
    RayHit best;
    best.t = std::numeric_limits<float>::max();
    for (size_t object_index = 0; object_index < objects.size(); ++object_index) {
        std::vector<Triangle> triangles = objects[object_index]->getTriangles();
        for (size_t triangle_index = 0; triangle_index < triangles.size(); ++triangle_index) {
            RayHit hit = intersectTriangleFirstHit(origin, direction, triangles[triangle_index]);
            if (hit.hit && hit.t < best.t) {
                best = hit;
                best.object_index = static_cast<int>(object_index);
                best.triangle_index = static_cast<int>(triangle_index);
            }
        }
    }
    if (best.object_index < 0) {
        return RayHit();
    }
    return best;
}

std::vector<IndirectVPL> sampleFirstBounceIndirectVPLs(
    const Light& light,
    const std::vector<std::shared_ptr<Object>>& objects,
    const IndirectVPLConfig& config,
    int source_light_index
) {
    std::vector<IndirectVPL> result;
    if (!config.enabled || config.samples <= 0 || objects.empty()) {
        return result;
    }

    std::vector<DirectVPL> direct_vpls = light.getDirectVPLs();
    if (direct_vpls.empty()) {
        return result;
    }

    result.reserve(static_cast<size_t>(config.samples));
    for (int sample_index = 0; sample_index < config.samples; ++sample_index) {
        const DirectVPL& source = direct_vpls[static_cast<size_t>(sample_index) % direct_vpls.size()];
        const Vec3f direction = deterministicDirection(sample_index, config.samples);
        RayHit hit = intersectSceneFirstHit(source.position, direction, objects);
        if (!hit.hit) {
            continue;
        }

        const Vec3f offset_position = hit.position + hit.normal * kRayEpsilon;
        Vec3f flux = source.intensity.cwiseAbs();
        if (direct_vpls.size() < static_cast<size_t>(config.samples)) {
            flux /= static_cast<float>(std::max(1, config.samples / static_cast<int>(direct_vpls.size())));
        }
        result.push_back(IndirectVPL(
            offset_position,
            hit.normal,
            flux,
            config.radius,
            1,
            source_light_index,
            0
        ));
    }
    return result;
}

Vec3f evaluateIndirectVPLContribution(
    const Vec3f& position,
    const Vec3f& normal,
    const Vec3f& albedo,
    const IndirectVPL& sample,
    float clamp
) {
    Vec3f to_vpl = sample.position - position;
    const float distance_squared = std::max(to_vpl.squaredNorm(), 1e-4f);
    Vec3f direction = to_vpl.normalized();
    const float surface_cos = std::max(0.0f, normal.normalized().dot(direction));
    const float vpl_cos = std::max(0.0f, sample.normal.normalized().dot(-direction));
    Vec3f contribution = sample.flux.cwiseProduct(albedo) *
                         (surface_cos * vpl_cos) /
                         (1.0f + distance_squared);
    const float safe_clamp = std::max(0.0f, clamp);
    if (safe_clamp > 0.0f) {
        contribution = contribution.cwiseMin(Vec3f(safe_clamp, safe_clamp, safe_clamp));
    }
    return contribution;
}
