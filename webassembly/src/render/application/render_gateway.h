#pragma once

#include <cstdint>
#include <vector>

#include <vtkSmartPointer.h>

class vtkActor;
class vtkActor2D;
class vtkCamera;
class vtkVolume;

namespace render {
namespace application {

class RenderGateway {
public:
    virtual ~RenderGateway() = default;

    virtual void RequestRender() = 0;
    virtual void FitViewToVisibleProps() = 0;

    virtual void AddActor(vtkSmartPointer<vtkActor> actor, bool resetCamera = true) = 0;
    virtual void RemoveActor(vtkSmartPointer<vtkActor> actor) = 0;

    virtual void AddActor2D(
        vtkSmartPointer<vtkActor2D> actor2D,
        bool resetCamera = true,
        bool measurementOverlay = false) = 0;
    virtual void RemoveActor2D(vtkSmartPointer<vtkActor2D> actor2D) = 0;

    virtual void AddVolume(vtkSmartPointer<vtkVolume> volume, bool resetCamera = true) = 0;
    virtual void RemoveVolume(vtkSmartPointer<vtkVolume> volume) = 0;

    virtual bool CaptureActorImage(
        vtkActor* actor,
        std::vector<uint8_t>& rgbaPixels,
        int& width,
        int& height,
        bool isolateActor = true,
        const double* viewDirection = nullptr,
        const double* viewUpHint = nullptr,
        const double* backgroundColorRgb = nullptr) = 0;

    virtual vtkCamera* GetActiveCamera() = 0;
};

RenderGateway& GetRenderGateway();

} // namespace application
} // namespace render
