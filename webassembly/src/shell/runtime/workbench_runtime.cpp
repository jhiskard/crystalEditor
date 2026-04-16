#include "workbench_runtime.h"

#include "../../app.h"
#include "../../workspace/legacy/atoms_template_facade.h"
#include "../../io/application/import_entry_service.h"
#include "../../platform/browser/browser_file_dialog_adapter.h"
#include "../../platform/worker/emscripten_worker_port.h"
#include "../../platform/worker/runtime_progress_port.h"
#include "../presentation/font/font_registry.h"
#include "../../mesh/presentation/mesh_detail_panel.h"
#include "../../mesh/presentation/mesh_group_detail_panel.h"
#include "../../mesh/domain/mesh_repository.h"
#include "../../mesh/presentation/model_tree_panel.h"
#include "../presentation/debug/test_window_panel.h"
#include "../presentation/toolbar/viewer_toolbar_presenter.h"
#include "../../render/application/legacy_viewer_facade.h"
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
    return render::application::GetLegacyViewerFacade();
}

structure::application::StructureService& WorkbenchRuntime::StructureFeature() {
    return AtomsTemplate::Instance().structureService();
}

measurement::application::MeasurementService& WorkbenchRuntime::MeasurementFeature() {
    return AtomsTemplate::Instance().measurementService();
}

density::application::DensityService& WorkbenchRuntime::DensityFeature() {
    return AtomsTemplate::Instance().densityService();
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

mesh::domain::MeshRepository& WorkbenchRuntime::MeshRepository() {
    return mesh::domain::GetMeshRepository();
}

FileLoader& WorkbenchRuntime::FileLoaderService() {
    static platform::browser::BrowserFileDialogAdapter fileDialogAdapter;
    static platform::worker::EmscriptenWorkerPort workerPort;
    static platform::worker::RuntimeProgressPort progressPort;
    static FileLoader fileLoaderService(fileDialogAdapter, workerPort, progressPort);
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
    MeshRepository().PrintMeshTree();
#endif
}

WorkbenchRuntime& GetWorkbenchRuntime() {
    return WorkbenchRuntime::Instance();
}

const WorkbenchRuntime& GetWorkbenchRuntimeConst() {
    return WorkbenchRuntime::Instance();
}



