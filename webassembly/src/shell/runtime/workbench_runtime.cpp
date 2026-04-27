#include "workbench_runtime.h"

#include "../../app.h"
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
#include "../../render/presentation/viewer_window.h"
#include "../../structure/application/structure_interaction_service.h"
#include "../../structure/application/structure_lifecycle_service.h"
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

structure::application::StructureService& runtimeStructureFeature() {
    static structure::application::StructureService service;
    return service;
}

structure::application::StructureInteractionService& runtimeStructureInteractionFeature() {
    static structure::application::StructureInteractionService service;
    return service;
}

structure::application::StructureLifecycleService& runtimeStructureLifecycleFeature() {
    static structure::application::StructureLifecycleService service;
    return service;
}

measurement::application::MeasurementService& runtimeMeasurementFeature() {
    static measurement::application::MeasurementService service;
    return service;
}

density::application::DensityService& runtimeDensityFeature() {
    static density::application::DensityService service;
    return service;
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
    return VtkViewer::Instance();
}

structure::application::StructureService& WorkbenchRuntime::StructureFeature() {
    return runtimeStructureFeature();
}

structure::application::StructureInteractionService& WorkbenchRuntime::StructureInteractionFeature() {
    return runtimeStructureInteractionFeature();
}

structure::application::StructureLifecycleService& WorkbenchRuntime::StructureLifecycleFeature() {
    return runtimeStructureLifecycleFeature();
}

measurement::application::MeasurementService& WorkbenchRuntime::MeasurementFeature() {
    return runtimeMeasurementFeature();
}

density::application::DensityService& WorkbenchRuntime::DensityFeature() {
    return runtimeDensityFeature();
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

void WorkbenchRuntime::RequestLayoutPreset(shell::domain::ShellLayoutPreset preset) {
    ShellController().RequestLayoutPreset(preset);
}

void WorkbenchRuntime::OpenEditorPanel(workbench::panel::EditorRequest request) {
    ShellController().OpenEditorPanel(static_cast<shell::application::EditorPanelAction>(request));
}

void WorkbenchRuntime::OpenBuilderPanel(workbench::panel::BuilderRequest request) {
    ShellController().OpenBuilderPanel(static_cast<shell::application::BuilderPanelAction>(request));
}

void WorkbenchRuntime::OpenDataPanel(workbench::panel::DataRequest request) {
    ShellController().OpenDataPanel(static_cast<shell::application::DataPanelAction>(request));
}

bool WorkbenchRuntime::IsShellWindowVisible(shell::domain::ShellWindowId windowId) {
    return ShellStateQuery().IsWindowVisible(windowId);
}

int WorkbenchRuntime::GetPendingLayoutPreset() {
    return static_cast<int>(ShellStateQuery().State().pendingLayoutPreset);
}

bool WorkbenchRuntime::HasPendingEditorRequest() {
    return ShellStateQuery().State().hasPendingEditorRequest;
}

bool WorkbenchRuntime::HasPendingBuilderRequest() {
    return ShellStateQuery().State().hasPendingBuilderRequest;
}

bool WorkbenchRuntime::HasPendingDataRequest() {
    return ShellStateQuery().State().hasPendingDataRequest;
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
