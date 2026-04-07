#pragma once

#include "../application/render_gateway.h"

class VtkViewer;

namespace render {
namespace infrastructure {

/**
 * @brief VTK adapter implementing the application-level render port contract.
 * @details This type is render-internal infrastructure and must not be
 *          referenced directly from non-render modules.
 */
class VtkRenderGateway final : public application::RenderGateway {
public:
    void RequestRender() override;
    void FitViewToVisibleProps() override;
    void ResetView() override;

    void AddActor(vtkSmartPointer<vtkActor> actor, bool resetCamera = true) override;
    void AddMeasurementOverlayActor(
        vtkSmartPointer<vtkActor> actor,
        bool resetCamera = true) override;
    void RemoveActor(vtkSmartPointer<vtkActor> actor) override;
    void RemoveMeasurementOverlayActor(vtkSmartPointer<vtkActor> actor) override;

    void AddActor2D(
        vtkSmartPointer<vtkActor2D> actor2D,
        bool resetCamera = true,
        bool measurementOverlay = false) override;
    void RemoveActor2D(vtkSmartPointer<vtkActor2D> actor2D) override;

    void AddVolume(vtkSmartPointer<vtkVolume> volume, bool resetCamera = true) override;
    void RemoveVolume(vtkSmartPointer<vtkVolume> volume) override;
    void SetProjectionMode(ProjectionMode mode) override;
    void AlignCameraToCellAxis(int axisIndex) override;
    void AlignCameraToIcellAxis(int axisIndex) override;
    void SetArrowRotateStepDeg(float stepDeg) override;
    float GetArrowRotateStepDeg() const override;
    bool IsPerformanceOverlayEnabled() const override;
    void SetPerformanceOverlayEnabled(bool enabled) override;

    bool CaptureActorImage(
        vtkActor* actor,
        std::vector<uint8_t>& rgbaPixels,
        int& width,
        int& height,
        bool isolateActor = true,
        const double* viewDirection = nullptr,
        const double* viewUpHint = nullptr,
        const double* backgroundColorRgb = nullptr) override;

    vtkCamera* GetActiveCamera() override;
    bool IsInteractionLodActive() const override;
    void BeginInteractionLod() override;
    void EndInteractionLod() override;
};

/**
 * @brief Returns legacy viewer facade through render infrastructure boundary.
 * @details Runtime/shell code should access the viewer via this adapter helper
 *          instead of calling `VtkViewer::Instance()` directly.
 */
VtkViewer& GetLegacyViewerFacade();

} // namespace infrastructure
} // namespace render
