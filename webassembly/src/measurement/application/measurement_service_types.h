/**
 * @file measurement_service_types.h
 * @brief Shared DTO types for measurement application services.
 */
#pragma once

#include <cstdint>
#include <string>

namespace measurement {
namespace application {

/**
 * @brief Measurement mode used by the measurement module API.
 */
enum class MeasurementMode {
    None = 0,
    Distance = 1,
    Angle = 2,
    Dihedral = 3,
    GeometricCenter = 4,
    CenterOfMass = 5
};

/**
 * @brief Measurement entry type used by measurement list/group views.
 */
enum class MeasurementType {
    Distance = 0,
    Angle = 1,
    Dihedral = 2,
    GeometricCenter = 3,
    CenterOfMass = 4
};

/**
 * @brief Measurement list row model.
 */
struct MeasurementListItem {
    uint32_t id = 0;
    MeasurementType type = MeasurementType::Distance;
    std::string displayName;
    bool visible = true;
};

/**
 * @brief Distance-only list row model.
 */
struct DistanceMeasurementListItem {
    uint32_t id = 0;
    std::string displayName;
    bool visible = true;
};

} // namespace application
} // namespace measurement
