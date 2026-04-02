#pragma once

#include <cstdint>
#include <vector>

#include <vtkSmartPointer.h>

class vtkActor;
class vtkActor2D;
class vtkCamera;
class vtkVolume;
enum class ProjectionMode;

namespace render {
namespace application {

/**
 * @brief Render port contract consumed by non-render feature modules.
 * @details Non-render layers must use this interface instead of calling
 *          `VtkViewer::Instance()` directly.
 */
class RenderGateway {
public:
    virtual ~RenderGateway() = default;

    virtual void RequestRender() = 0;
    virtual void FitViewToVisibleProps() = 0;
    virtual void ResetView() = 0;

    virtual void AddActor(vtkSmartPointer<vtkActor> actor, bool resetCamera = true) = 0;
    virtual void AddMeasurementOverlayActor(
        vtkSmartPointer<vtkActor> actor,
        bool resetCamera = true) = 0;
    virtual void RemoveActor(vtkSmartPointer<vtkActor> actor) = 0;
    virtual void RemoveMeasurementOverlayActor(vtkSmartPointer<vtkActor> actor) = 0;

    virtual void AddActor2D(
        vtkSmartPointer<vtkActor2D> actor2D,
        bool resetCamera = true,
        bool measurementOverlay = false) = 0;
    virtual void RemoveActor2D(vtkSmartPointer<vtkActor2D> actor2D) = 0;

    virtual void AddVolume(vtkSmartPointer<vtkVolume> volume, bool resetCamera = true) = 0;
    virtual void RemoveVolume(vtkSmartPointer<vtkVolume> volume) = 0;
    virtual void SetProjectionMode(ProjectionMode mode) = 0;
    virtual void AlignCameraToCellAxis(int axisIndex) = 0;
    virtual void AlignCameraToIcellAxis(int axisIndex) = 0;
    virtual void SetArrowRotateStepDeg(float stepDeg) = 0;
    virtual float GetArrowRotateStepDeg() const = 0;

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

    /**
     * @brief Returns whether interaction-time LOD is active.
     */
    virtual bool IsInteractionLodActive() const = 0;

    /**
     * @brief Activates interaction-time LOD rendering.
     */
    virtual void BeginInteractionLod() = 0;

    /**
     * @brief Restores normal rendering after interaction-time LOD.
     */
    virtual void EndInteractionLod() = 0;
};

RenderGateway& GetRenderGateway();

} // namespace application
} // namespace render
