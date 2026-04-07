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
#include "../../toolbar.h"
#include "../../render/infrastructure/vtk_render_gateway.h"
#include "../../structure/application/structure_service.h"
#include "../../measurement/application/measurement_service.h"
#include "../../density/application/density_service.h"
#include "../../workspace/application/workspace_query_service.h"
#include "../../workspace/application/workspace_command_service.h"
#include "../application/shell_state_query_service.h"
#include "../application/shell_state_command_service.h"
#include "../application/workbench_controller.h"

namespace {
ModelTree& runtimeModelTreePanel() {
    static ModelTree panel;
    return panel;
}

MeshDetail& runtimeMeshDetailPanel() {
    static MeshDetail panel;
    return panel;
}

MeshGroupDetail& runtimeMeshGroupDetailPanel() {
    static MeshGroupDetail panel;
    return panel;
}

TestWindow& runtimeTestWindowPanel() {
    static TestWindow panel;
    return panel;
}
} // namespace

WorkbenchRuntime& WorkbenchRuntime::Instance() {
    static WorkbenchRuntime runtime;
    return runtime;
}

void WorkbenchRuntime::PrimeLegacySingletons() {
    (void)AppController();
}

App& WorkbenchRuntime::AppController() {
    static App appController;
    return appController;
}

FontManager& WorkbenchRuntime::FontRegistry() {
    return FontManager::Instance();
}

Toolbar& WorkbenchRuntime::ToolbarPanel() {
    static Toolbar toolbarPanel;
    return toolbarPanel;
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
    return runtimeModelTreePanel();
}

MeshDetail& WorkbenchRuntime::MeshDetailPanel() {
    return runtimeMeshDetailPanel();
}

MeshGroupDetail& WorkbenchRuntime::MeshGroupDetailPanel() {
    return runtimeMeshGroupDetailPanel();
}

MeshManager& WorkbenchRuntime::MeshRepository() {
    return MeshManager::Instance();
}

FileLoader& WorkbenchRuntime::FileLoaderService() {
    static FileLoader fileLoaderService;
    return fileLoaderService;
}

workspace::application::WorkspaceQueryService& WorkbenchRuntime::WorkspaceQuery() {
    return workspace::application::GetWorkspaceQueryService();
}

workspace::application::WorkspaceCommandService& WorkbenchRuntime::WorkspaceCommand() {
    return workspace::application::GetWorkspaceCommandService();
}

shell::application::ShellStateQueryService& WorkbenchRuntime::ShellStateQuery() {
    return shell::application::GetShellStateQueryService();
}

shell::application::ShellStateCommandService& WorkbenchRuntime::ShellStateCommand() {
    return shell::application::GetShellStateCommandService();
}

shell::application::WorkbenchController& WorkbenchRuntime::ShellController() {
    return shell::application::GetWorkbenchController();
}

TestWindow& WorkbenchRuntime::TestWindowPanel() {
    return runtimeTestWindowPanel();
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

void WorkbenchRuntime::SetProgress(float progress) {
    AppController().SetProgress(progress);
}

void WorkbenchRuntime::ShowProgressPopup(bool show) {
    App::ShowProgressPopup(show);
}

void WorkbenchRuntime::SetProgressPopupText(const std::string& title, const std::string& text) {
    App::SetProgressPopupText(title, text);
}

void WorkbenchRuntime::LoadArrayBuffer(const std::string& fileName, bool deleteFile) {
    FileLoaderService().LoadArrayBuffer(fileName, deleteFile);
}

void WorkbenchRuntime::LoadChgcarFile(const std::string& fileName) {
    FileLoaderService().LoadChgcarFile(fileName);
}

void WorkbenchRuntime::HandleXSFGridFile(const std::string& fileName) {
    FileLoaderService().HandleXSFGridFile(fileName);
}

void WorkbenchRuntime::HandleStructureFile(const std::string& fileName) {
    FileLoaderService().HandleStructureFile(fileName);
}

void WorkbenchRuntime::WriteChunk(
    const std::string& fileName,
    int32_t offset,
    uintptr_t data,
    int32_t length) {
    FileLoaderService().WriteChunk(fileName, offset, data, length);
}

void WorkbenchRuntime::CloseFile(const std::string& fileName) {
    FileLoaderService().CloseFile(fileName);
}

void WorkbenchRuntime::ProcessFileInBackground(const std::string& fileName, bool deleteFile) {
    FileLoaderService().ProcessFileInBackground(fileName, deleteFile);
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
