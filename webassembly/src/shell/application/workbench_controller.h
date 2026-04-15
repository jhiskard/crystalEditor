/**
 * @file workbench_controller.h
 * @brief Shell command controller that routes menu/toolbar actions.
 */
#pragma once

#include "../../common/panel_request_types.h"
#include "../../enum/viewer_enums.h"
#include "../../measurement/application/measurement_service.h"
#include "../domain/shell_state_store.h"

namespace shell {
namespace application {

using EditorPanelAction = workbench::panel::EditorRequest;
using BuilderPanelAction = workbench::panel::BuilderRequest;
using DataPanelAction = workbench::panel::DataRequest;

/**
 * @brief Shell-level controller for menu/toolbar command dispatch.
 * @details This controller centralizes shell action routing so `App`/`Toolbar`
 *          can focus on rendering while command side-effects stay in one place.
 */
class WorkbenchController {
public:
    /**
     * @brief Returns singleton controller instance.
     */
    static WorkbenchController& Instance();

    /**
     * @brief Opens structure import flow from shell menu actions.
     */
    void RequestOpenStructureImport();

    /**
     * @brief Routes editor menu actions and updates shell focus/visibility state.
     */
    void OpenEditorPanel(EditorPanelAction action);

    /**
     * @brief Routes builder menu actions and updates shell focus/visibility state.
     */
    void OpenBuilderPanel(BuilderPanelAction action);

    /**
     * @brief Routes data menu actions and updates shell focus/visibility state.
     */
    void OpenDataPanel(DataPanelAction action);

    /**
     * @brief Returns whether node tooltip info is enabled.
     */
    bool IsNodeInfoEnabled() const;

    /**
     * @brief Enables/disables node tooltip info.
     */
    void SetNodeInfoEnabled(bool enabled);

    /**
     * @brief Returns whether viewer FPS overlay is enabled.
     */
    bool IsViewerFpsOverlayEnabled() const;

    /**
     * @brief Enables/disables viewer FPS overlay.
     */
    void SetViewerFpsOverlayEnabled(bool enabled);

    /**
     * @brief Returns whether a measurement mode is currently active.
     */
    bool IsMeasurementModeActive(measurement::application::MeasurementMode mode) const;

    /**
     * @brief Enters the requested measurement mode.
     */
    void EnterMeasurementMode(measurement::application::MeasurementMode mode);

    /**
     * @brief Requests a shell layout preset transition.
     */
    void RequestLayoutPreset(domain::ShellLayoutPreset preset);

    /**
     * @brief Toggles boundary-atom visibility.
     */
    void ToggleBoundaryAtoms();

    /**
     * @brief Returns current boundary-atom visibility.
     */
    bool IsBoundaryAtomsEnabled() const;

    /**
     * @brief Sets mesh display mode for all visible meshes.
     */
    void SetMeshDisplayMode(MeshDisplayMode mode);

    /**
     * @brief Sets viewer projection mode.
     */
    void SetProjectionMode(ProjectionMode mode);

    /**
     * @brief Resets viewer camera to fit current view.
     */
    void ResetView();

    /**
     * @brief Aligns camera to active cell axis for toolbar quick action.
     */
    void AlignCameraToActiveCellAxis(int axisIndex);

    /**
     * @brief Returns current arrow-key camera rotation step.
     */
    int GetArrowRotateStepDeg() const;

    /**
     * @brief Sets arrow-key camera rotation step.
     * @param stepDeg Camera step in degrees. Valid range is [1, 180].
     */
    void SetArrowRotateStepDeg(int stepDeg);

private:
    WorkbenchController() = default;
};

/**
 * @brief Convenience accessor for shell workbench controller.
 */
WorkbenchController& GetWorkbenchController();

} // namespace application
} // namespace shell
