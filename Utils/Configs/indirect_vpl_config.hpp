#ifndef INDIRECT_VPL_CONFIG_HPP_
#define INDIRECT_VPL_CONFIG_HPP_

#include <cstdint>
#include <vector>

struct IndirectVPLConfig {
    bool enabled = false;
    int samples = 0;
    uint32_t seed = 0;
    int bounces = 1;
    bool visibility = true;
    float clamp = 10.0f;
    float radius = 1.0f;
    std::vector<int> source_light_indices;
};

#endif  // INDIRECT_VPL_CONFIG_HPP_
