/**
 * @file density_read_model.h
 * @brief Density presentation read-model primitives.
 */
#pragma once

#include <cstdint>
#include <string>

namespace density {
namespace presentation {

/**
 * @brief Grid visibility row model for density tree/panel rendering.
 */
struct GridVisibilityRow {
    int32_t meshId = -1;
    std::string name;
    bool visible = false;
    bool volumeMode = false;
};

} // namespace presentation
} // namespace density

