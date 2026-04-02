#include "vtk_render_gateway.h"

#include "../../vtk_viewer.h"

namespace render {
namespace infrastructure {

void VtkRenderGateway::RequestRender() {
    VtkViewer::Instance().RequestRender();
}

void VtkRenderGateway::FitViewToVisibleProps() {
    VtkViewer::Instance().FitViewToVisibleProps();
}

void VtkRenderGateway::ResetView() {
    VtkViewer::Instance().ResetView();
}

void VtkRenderGateway::AddActor(vtkSmartPointer<vtkActor> actor, bool resetCamera) {
    VtkViewer::Instance().AddActor(actor, resetCamera);
}

void VtkRenderGateway::AddMeasurementOverlayActor(
    vtkSmartPointer<vtkActor> actor,
    bool resetCamera) {
    VtkViewer::Instance().AddMeasurementOverlayActor(actor, resetCamera);
}

void VtkRenderGateway::RemoveActor(vtkSmartPointer<vtkActor> actor) {
    VtkViewer::Instance().RemoveActor(actor);
}

void VtkRenderGateway::RemoveMeasurementOverlayActor(vtkSmartPointer<vtkActor> actor) {
    VtkViewer::Instance().RemoveMeasurementOverlayActor(actor);
}

void VtkRenderGateway::AddActor2D(
    vtkSmartPointer<vtkActor2D> actor2D,
    bool resetCamera,
    bool measurementOverlay) {
    VtkViewer::Instance().AddActor2D(actor2D, resetCamera, measurementOverlay);
}

void VtkRenderGateway::RemoveActor2D(vtkSmartPointer<vtkActor2D> actor2D) {
    VtkViewer::Instance().RemoveActor2D(actor2D);
}

void VtkRenderGateway::AddVolume(vtkSmartPointer<vtkVolume> volume, bool resetCamera) {
    VtkViewer::Instance().AddVolume(volume, resetCamera);
}

void VtkRenderGateway::RemoveVolume(vtkSmartPointer<vtkVolume> volume) {
    VtkViewer::Instance().RemoveVolume(volume);
}

void VtkRenderGateway::SetProjectionMode(ProjectionMode mode) {
    VtkViewer::Instance().SetProjectionMode(mode);
}

void VtkRenderGateway::AlignCameraToCellAxis(int axisIndex) {
    VtkViewer::Instance().AlignCameraToCellAxis(axisIndex);
}

void VtkRenderGateway::AlignCameraToIcellAxis(int axisIndex) {
    VtkViewer::Instance().AlignCameraToIcellAxis(axisIndex);
}

void VtkRenderGateway::SetArrowRotateStepDeg(float stepDeg) {
    VtkViewer::Instance().SetArrowRotateStepDeg(stepDeg);
}

float VtkRenderGateway::GetArrowRotateStepDeg() const {
    return VtkViewer::Instance().GetArrowRotateStepDeg();
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
    return VtkViewer::Instance().CaptureActorImage(
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
    return VtkViewer::Instance().GetActiveCamera();
}

VtkViewer& GetLegacyViewerFacade() {
    return VtkViewer::Instance();
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
