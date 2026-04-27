/**
 * @file structure_lifecycle_service.h
 * @brief Structure lifecycle and BZ feature application service facade.
 */
#pragma once

#include <cstdint>
#include <string>

namespace structure {
namespace application {

/**
 * @brief Lifecycle/BZ use-case facade extracted from WorkspaceRuntimeModel.
 * @details Application layer delegates to legacy runtime in W5.4 thin-shim mode.
 */
class StructureLifecycleService {
public:
    /**
     * @brief Creates structure lifecycle service facade.
     */
    StructureLifecycleService();

    void RegisterStructure(int32_t structureId, const std::string& name);
    void RemoveStructure(int32_t structureId);
    void RemoveUnassignedData();

    void RenderBrillouinZonePlotWindow(bool* openWindow = nullptr);
    bool EnterBZPlotMode(
        const std::string& path,
        int npoints,
        bool showVectors,
        bool showLabels,
        std::string& outErrorMessage);
    void ExitBZPlotMode();
    bool IsBZPlotMode() const;
};

} // namespace application
} // namespace structure
