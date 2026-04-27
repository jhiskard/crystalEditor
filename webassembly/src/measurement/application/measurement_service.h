/**
 * @file measurement_service.h
 * @brief Measurement feature application service facade.
 */
#pragma once

#include "measurement_service_types.h"
#include "../../render/application/picked_atom_info.h"

#include <cstdint>
#include <vector>

class vtkActor;
class vtkRenderer;

namespace measurement {
namespace application {

/**
 * @brief Measurement use-case facade extracted from AtomsTemplate.
 * @details Application layer delegates to legacy runtime in W5.2 thin-shim mode.
 */
class MeasurementService {
public:
    /**
     * @brief Creates measurement service facade.
     */
    MeasurementService();

    MeasurementMode GetMode() const;
    bool IsModeActive() const;
    bool IsDragSelectionEnabled() const;
    void EnterMode(MeasurementMode mode);
    void ExitMode();

    render::application::PickedAtomInfo ResolvePickedAtomInfo(
        vtkActor* actor,
        const double pickPos[3]) const;
    void HandlePickerClick(const render::application::PickedAtomInfo& pickedAtomInfo);
    void HandleEmptyClick();
    void HandleDragSelectionInScreenRect(
        int x0,
        int y0,
        int x1,
        int y1,
        vtkRenderer* renderer,
        int viewportHeight,
        bool additive);
    void RenderModeOverlay();

    std::vector<MeasurementListItem> GetMeasurementsForStructure(int32_t structureId) const;
    void SetMeasurementVisible(uint32_t measurementId, bool visible);
    void RemoveMeasurement(uint32_t measurementId);
    void RemoveMeasurementsByStructure(int32_t structureId);

    std::vector<DistanceMeasurementListItem> GetDistanceMeasurementsForStructure(int32_t structureId) const;
    void SetDistanceMeasurementVisible(uint32_t measurementId, bool visible);
    void RemoveDistanceMeasurement(uint32_t measurementId);
};

} // namespace application
} // namespace measurement