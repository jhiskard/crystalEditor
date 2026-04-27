/**
 * @file measurement_repository.h
 * @brief Measurement domain repository placeholder for Phase 8 extraction.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace measurement {
namespace domain {

/**
 * @brief Stable domain row for measurement queries.
 * @details This type decouples presentation read models from legacy AtomsTemplate structs.
 */
struct MeasurementRecord {
    uint32_t id = 0;
    std::string displayName;
    bool visible = true;
};

/**
 * @brief Read-only repository interface for measurement snapshots.
 */
class MeasurementRepository {
public:
    virtual ~MeasurementRepository() = default;
    virtual std::vector<MeasurementRecord> ListByStructure(int32_t structureId) const = 0;
};

} // namespace domain
} // namespace measurement



