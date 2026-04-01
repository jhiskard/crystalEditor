/**
 * @file measurement_read_model.h
 * @brief Measurement presentation read-model primitives.
 */
#pragma once

#include <cstdint>
#include <string>

namespace measurement {
namespace presentation {

/**
 * @brief Flat list item model for measurement UI tables.
 */
struct MeasurementRowModel {
    uint32_t id = 0;
    std::string label;
    bool visible = true;
};

} // namespace presentation
} // namespace measurement

