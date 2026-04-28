#ifndef VPL_DEBUG_HPP_
#define VPL_DEBUG_HPP_

#include "float_buffer_writer.hpp"
#include "light.hpp"

#include <string>
#include <vector>

inline std::string indirectVPLBufferName(size_t light_index) {
    return "light_" + std::to_string(light_index) + "_indirect_vpls";
}

inline FloatBuffer makeIndirectVPLFloatBuffer(size_t light_index, const std::vector<IndirectVPL>& samples) {
    FloatBuffer buffer;
    buffer.name = indirectVPLBufferName(light_index);
    buffer.width = static_cast<int>(samples.size());
    buffer.height = 1;
    buffer.channels = 13;
    buffer.data.reserve(samples.size() * static_cast<size_t>(buffer.channels));
    for (const IndirectVPL& sample : samples) {
        std::vector<float> record = sample.toFloatRecord();
        buffer.data.insert(buffer.data.end(), record.begin(), record.end());
    }
    return buffer;
}

#endif  // VPL_DEBUG_HPP_
