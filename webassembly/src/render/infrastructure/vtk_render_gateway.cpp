#include "vtk_render_gateway.h"

#include "../../vtk_viewer.h"

namespace {
VtkViewer& viewerFacade() {
    return VtkViewer::Instance();
}
} // namespace

namespace render {
namespace infrastructure {

void VtkRenderGateway::RequestRender() {
    viewerFacade().RequestRender();
}

void VtkRenderGateway::FitViewToVisibleProps() {
    viewerFacade().FitViewToVisibleProps();
}

void VtkRenderGateway::ResetView() {
    viewerFacade().ResetView();
}

void VtkRenderGateway::AddActor(vtkSmartPointer<vtkActor> actor, bool resetCamera) {
    viewerFacade().AddActor(actor, resetCamera);
}

void VtkRenderGateway::AddMeasurementOverlayActor(
    vtkSmartPointer<vtkActor> actor,
    bool resetCamera) {
    viewerFacade().AddMeasurementOverlayActor(actor, resetCamera);
}

void VtkRenderGateway::RemoveActor(vtkSmartPointer<vtkActor> actor) {
    viewerFacade().RemoveActor(actor);
}

void VtkRenderGateway::RemoveMeasurementOverlayActor(vtkSmartPointer<vtkActor> actor) {
    viewerFacade().RemoveMeasurementOverlayActor(actor);
}

void VtkRenderGateway::AddActor2D(
    vtkSmartPointer<vtkActor2D> actor2D,
    bool resetCamera,
    bool measurementOverlay) {
    viewerFacade().AddActor2D(actor2D, resetCamera, measurementOverlay);
}

void VtkRenderGateway::RemoveActor2D(vtkSmartPointer<vtkActor2D> actor2D) {
    viewerFacade().RemoveActor2D(actor2D);
}

void VtkRenderGateway::AddVolume(vtkSmartPointer<vtkVolume> volume, bool resetCamera) {
    viewerFacade().AddVolume(volume, resetCamera);
}

void VtkRenderGateway::RemoveVolume(vtkSmartPointer<vtkVolume> volume) {
    viewerFacade().RemoveVolume(volume);
}

void VtkRenderGateway::SetProjectionMode(ProjectionMode mode) {
    viewerFacade().SetProjectionMode(mode);
}

void VtkRenderGateway::AlignCameraToCellAxis(int axisIndex) {
    viewerFacade().AlignCameraToCellAxis(axisIndex);
}

void VtkRenderGateway::AlignCameraToIcellAxis(int axisIndex) {
    viewerFacade().AlignCameraToIcellAxis(axisIndex);
}

void VtkRenderGateway::SetArrowRotateStepDeg(float stepDeg) {
    viewerFacade().SetArrowRotateStepDeg(stepDeg);
}

float VtkRenderGateway::GetArrowRotateStepDeg() const {
    return viewerFacade().GetArrowRotateStepDeg();
}

bool VtkRenderGateway::IsPerformanceOverlayEnabled() const {
    return viewerFacade().IsPerformanceOverlayEnabled();
}

void VtkRenderGateway::SetPerformanceOverlayEnabled(bool enabled) {
    viewerFacade().SetPerformanceOverlayEnabled(enabled);
}

bool VtkRenderGateway::CaptureActorImage(
    vtkActor* actor,
    std::vector<uint8_t>& rgbaPixels,
    int& width,
    int& height,
    bool isolateActor,
    const double* viewDirection,
    const double* viewUpHint,
    const double* backgroundColorRgb) {
    return viewerFacade().CaptureActorImage(
        actor,
        rgbaPixels,
        width,
        height,
        isolateActor,
        viewDirection,
        viewUpHint,
        backgroundColorRgb);
}

vtkCamera* VtkRenderGateway::GetActiveCamera() {
    return viewerFacade().GetActiveCamera();
}

bool VtkRenderGateway::IsInteractionLodActive() const {
    return viewerFacade().IsInteractionLodActive();
}

void VtkRenderGateway::BeginInteractionLod() {
    viewerFacade().BeginInteractionLod();
}

void VtkRenderGateway::EndInteractionLod() {
    viewerFacade().EndInteractionLod();
}

VtkViewer& GetLegacyViewerFacade() {
    return viewerFacade();
}

} // namespace infrastructure
} // namespace render

namespace render {
namespace application {

RenderGateway& GetRenderGateway() {
    static infrastructure::VtkRenderGateway gateway;
    return gateway;
}

} // namespace application
} // namespace render
