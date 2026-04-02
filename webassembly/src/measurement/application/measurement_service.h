/**
 * @file measurement_service.h
 * @brief Measurement feature application service facade.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

class AtomsTemplate;

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
 * @brief Measurement list row model.
 */
struct MeasurementListItem {
    uint32_t id = 0;
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

/**
 * @brief Measurement use-case facade extracted from AtomsTemplate.
 * @details During Phase 8 this service delegates to compatibility APIs so call-sites
 *          can migrate without behavior changes.
 */
class MeasurementService {
public:
    /**
     * @brief Creates service bound to the atoms compatibility facade.
     */
    explicit MeasurementService(AtomsTemplate* atomsTemplate);

    MeasurementMode GetMode() const;
    bool IsModeActive() const;
    void EnterMode(MeasurementMode mode);
    void ExitMode();

    std::vector<MeasurementListItem> GetMeasurementsForStructure(int32_t structureId) const;
    void SetMeasurementVisible(uint32_t measurementId, bool visible);
    void RemoveMeasurement(uint32_t measurementId);
    void RemoveMeasurementsByStructure(int32_t structureId);

    std::vector<DistanceMeasurementListItem> GetDistanceMeasurementsForStructure(int32_t structureId) const;
    void SetDistanceMeasurementVisible(uint32_t measurementId, bool visible);
    void RemoveDistanceMeasurement(uint32_t measurementId);

private:
    AtomsTemplate* m_atomsTemplate = nullptr;
};

} // namespace application
} // namespace measurement

