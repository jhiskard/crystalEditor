#include "workbench_runtime.h"

#include "../../app.h"
#include "../../atoms/atoms_template.h"
#include "../../file_loader.h"
#include "../../font_manager.h"
#include "../../mesh_detail.h"
#include "../../mesh_group_detail.h"
#include "../../mesh_manager.h"
#include "../../model_tree.h"
#include "../../test_window.h"
#include "../../render/infrastructure/vtk_render_gateway.h"
#include "../../structure/application/structure_service.h"
#include "../../measurement/application/measurement_service.h"
#include "../../density/application/density_service.h"

WorkbenchRuntime& WorkbenchRuntime::Instance() {
    static WorkbenchRuntime runtime;
    return runtime;
}

void WorkbenchRuntime::PrimeLegacySingletons() {
    (void)AppController();
    (void)MeshRepository();
}

App& WorkbenchRuntime::AppController() {
    return App::Instance();
}

FontManager& WorkbenchRuntime::FontRegistry() {
    return FontManager::Instance();
}

VtkViewer& WorkbenchRuntime::Viewer() {
    return render::infrastructure::GetLegacyViewerFacade();
}

AtomsTemplate& WorkbenchRuntime::AtomsTemplateFacade() {
    return AtomsTemplate::Instance();
}

structure::application::StructureService& WorkbenchRuntime::StructureFeature() {
    return AtomsTemplateFacade().structureService();
}

measurement::application::MeasurementService& WorkbenchRuntime::MeasurementFeature() {
    return AtomsTemplateFacade().measurementService();
}

density::application::DensityService& WorkbenchRuntime::DensityFeature() {
    return AtomsTemplateFacade().densityService();
}

ModelTree& WorkbenchRuntime::ModelTreePanel() {
    return ModelTree::Instance();
}

MeshDetail& WorkbenchRuntime::MeshDetailPanel() {
    return MeshDetail::Instance();
}

MeshGroupDetail& WorkbenchRuntime::MeshGroupDetailPanel() {
    return MeshGroupDetail::Instance();
}

MeshManager& WorkbenchRuntime::MeshRepository() {
    return MeshManager::Instance();
}

FileLoader& WorkbenchRuntime::FileLoaderService() {
    return FileLoader::Instance();
}

TestWindow& WorkbenchRuntime::TestWindowPanel() {
    return TestWindow::Instance();
}

void WorkbenchRuntime::RenderAppFrame() {
    AppController().Render();
}

float WorkbenchRuntime::DevicePixelRatio() const {
    return static_cast<float>(App::DevicePixelRatio());
}

void WorkbenchRuntime::InitIdbfs() {
    App::InitIdbfs();
}

void WorkbenchRuntime::SaveImGuiIniFile() {
    App::SaveImGuiIniFile();
}

void WorkbenchRuntime::LoadImGuiIniFile() {
    App::LoadImGuiIniFile();
}

void WorkbenchRuntime::ShowProgressPopup(bool show) {
    App::ShowProgressPopup(show);
}

void WorkbenchRuntime::SetProgressPopupText(const std::string& title, const std::string& text) {
    App::SetProgressPopupText(title, text);
}

void WorkbenchRuntime::LoadArrayBuffer(const std::string& fileName, bool deleteFile) {
    FileLoader::LoadArrayBuffer(fileName, deleteFile);
}

void WorkbenchRuntime::LoadChgcarFile(const std::string& fileName) {
    FileLoader::LoadChgcarFile(fileName);
}

void WorkbenchRuntime::HandleXSFGridFile(const std::string& fileName) {
    FileLoader::HandleXSFGridFile(fileName);
}

void WorkbenchRuntime::HandleStructureFile(const std::string& fileName) {
    FileLoader::HandleStructureFile(fileName);
}

void WorkbenchRuntime::WriteChunk(
    const std::string& fileName,
    int32_t offset,
    uintptr_t data,
    int32_t length) {
    FileLoader::WriteChunk(fileName, offset, data, length);
}

void WorkbenchRuntime::CloseFile(const std::string& fileName) {
    FileLoader::CloseFile(fileName);
}

void WorkbenchRuntime::ProcessFileInBackground(const std::string& fileName, bool deleteFile) {
    FileLoader::ProcessFileInBackground(fileName, deleteFile);
}

void WorkbenchRuntime::RequestOpenStructureImport() {
    FileLoaderService().RequestOpenStructureImport();
}

void WorkbenchRuntime::RenderXsfGridImportPopups() {
    FileLoaderService().RenderXsfGridImportPopups();
}

int WorkbenchRuntime::GetStructureCount() {
    return StructureFeature().GetStructureCount();
}

int WorkbenchRuntime::GetCurrentStructureId() {
    return StructureFeature().GetCurrentStructureId();
}

bool WorkbenchRuntime::IsStructureVisible(int id) {
    return StructureFeature().IsStructureVisible(id);
}

void WorkbenchRuntime::SetStructureVisible(int id, bool visible) {
    StructureFeature().SetStructureVisible(id, visible);
}

int WorkbenchRuntime::GetMeshCount() {
    return static_cast<int>(MeshRepository().GetMeshCount());
}

bool WorkbenchRuntime::HasChargeDensity() {
    return DensityFeature().HasChargeDensity();
}

void WorkbenchRuntime::PrintMeshTree() {
#ifdef DEBUG_BUILD
    MeshManager::PrintMeshTree();
#endif
}

WorkbenchRuntime& GetWorkbenchRuntime() {
    return WorkbenchRuntime::Instance();
}

const WorkbenchRuntime& GetWorkbenchRuntimeConst() {
    return WorkbenchRuntime::Instance();
}
