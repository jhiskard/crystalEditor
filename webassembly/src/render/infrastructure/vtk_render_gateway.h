#pragma once

#include "../application/render_gateway.h"

namespace render {
namespace infrastructure {

class VtkRenderGateway final : public application::RenderGateway {
public:
    void RequestRender() override;
    void FitViewToVisibleProps() override;

    void AddActor(vtkSmartPointer<vtkActor> actor, bool resetCamera = true) override;
    void RemoveActor(vtkSmartPointer<vtkActor> actor) override;

    void AddActor2D(
        vtkSmartPointer<vtkActor2D> actor2D,
        bool resetCamera = true,
        bool measurementOverlay = false) override;
    void RemoveActor2D(vtkSmartPointer<vtkActor2D> actor2D) override;

    void AddVolume(vtkSmartPointer<vtkVolume> volume, bool resetCamera = true) override;
    void RemoveVolume(vtkSmartPointer<vtkVolume> volume) override;

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
};

} // namespace infrastructure
} // namespace render
