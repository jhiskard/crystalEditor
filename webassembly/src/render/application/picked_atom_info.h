#pragma once

#include <array>
#include <cstdint>

namespace render::application {

/**
 * @brief Value-object pick result transferred out of render interaction.
 */
struct PickedAtomInfo {
    bool hit = false;
    uint32_t atomId = 0;
    int32_t structureId = -1;
    std::array<double, 3> pickPosition { 0.0, 0.0, 0.0 };
    std::array<double, 3> atomPosition { 0.0, 0.0, 0.0 };
};

} // namespace render::application

