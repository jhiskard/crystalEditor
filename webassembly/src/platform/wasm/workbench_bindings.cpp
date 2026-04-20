#include "../../shell/runtime/workbench_runtime.h"
#include "../../shell/domain/shell_state_store.h"
#include "../../common/panel_request_types.h"

#include <emscripten/bind.h>

namespace {

/**
 * @brief Mounts IDBFS through runtime facade.
 */
void initIdbfs() {
    GetWorkbenchRuntime().InitIdbfs();
}

/**
 * @brief Saves ImGui ini state through runtime facade.
 */
void saveImGuiIniFile() {
    GetWorkbenchRuntime().SaveImGuiIniFile();
}

/**
 * @brief Loads ImGui ini state through runtime facade.
 */
void loadImGuiIniFile() {
    GetWorkbenchRuntime().LoadImGuiIniFile();
}

void loadArrayBuffer(const std::string& fileName, bool deleteFile) {
    // Runtime-mediated import contract (Phase 13):
    // binding -> WorkbenchRuntime -> FileLoaderService(instance-owned)
    GetWorkbenchRuntime().LoadArrayBuffer(fileName, deleteFile);
}

void loadChgcarFile(const std::string& fileName) {
    GetWorkbenchRuntime().LoadChgcarFile(fileName);
}

void handleXSFGridFile(const std::string& fileName) {
    GetWorkbenchRuntime().HandleXSFGridFile(fileName);
}

void handleStructureFile(const std::string& fileName) {
    GetWorkbenchRuntime().HandleStructureFile(fileName);
}

int getStructureCount() {
    return GetWorkbenchRuntime().GetStructureCount();
}

int getCurrentStructureId() {
    return GetWorkbenchRuntime().GetCurrentStructureId();
}

bool isStructureVisible(int id) {
    return GetWorkbenchRuntime().IsStructureVisible(id);
}

void setStructureVisible(int id, bool visible) {
    GetWorkbenchRuntime().SetStructureVisible(id, visible);
}

int getMeshCount() {
    return GetWorkbenchRuntime().GetMeshCount();
}

bool hasChargeDensity() {
    return GetWorkbenchRuntime().HasChargeDensity();
}

shell::domain::ShellLayoutPreset toShellLayoutPreset(int preset) {
    using shell::domain::ShellLayoutPreset;
    switch (preset) {
    case 1:
        return ShellLayoutPreset::DefaultFloating;
    case 2:
        return ShellLayoutPreset::DockRight;
    case 3:
        return ShellLayoutPreset::DockBottom;
    case 4:
        return ShellLayoutPreset::ResetDocking;
    case 0:
    default:
        return ShellLayoutPreset::None;
    }
}

workbench::panel::EditorRequest toEditorRequest(int request) {
    using workbench::panel::EditorRequest;
    switch (request) {
    case 1:
        return EditorRequest::Bonds;
    case 2:
        return EditorRequest::Cell;
    case 0:
    default:
        return EditorRequest::Atoms;
    }
}

workbench::panel::BuilderRequest toBuilderRequest(int request) {
    using workbench::panel::BuilderRequest;
    switch (request) {
    case 1:
        return BuilderRequest::BravaisLatticeTemplates;
    case 2:
        return BuilderRequest::BrillouinZone;
    case 0:
    default:
        return BuilderRequest::AddAtoms;
    }
}

workbench::panel::DataRequest toDataRequest(int request) {
    using workbench::panel::DataRequest;
    switch (request) {
    case 1:
        return DataRequest::Surface;
    case 2:
        return DataRequest::Volumetric;
    case 3:
        return DataRequest::Plane;
    case 0:
    default:
        return DataRequest::Isosurface;
    }
}

shell::domain::ShellWindowId toShellWindowId(int windowId) {
    using shell::domain::ShellWindowId;
    switch (windowId) {
    case 1:
        return ShellWindowId::ModelTree;
    case 2:
        return ShellWindowId::TestWindow;
    case 3:
        return ShellWindowId::MeshDetail;
    case 4:
        return ShellWindowId::PeriodicTable;
    case 5:
        return ShellWindowId::CrystalTemplates;
    case 6:
        return ShellWindowId::BrillouinZonePlot;
    case 7:
        return ShellWindowId::CreatedAtoms;
    case 8:
        return ShellWindowId::BondsManagement;
    case 9:
        return ShellWindowId::CellInformation;
    case 10:
        return ShellWindowId::ChargeDensityViewer;
    case 11:
        return ShellWindowId::SliceViewer;
    case 0:
    default:
        return ShellWindowId::Viewer;
    }
}

void requestLayoutPreset(int preset) {
    GetWorkbenchRuntime().RequestLayoutPreset(toShellLayoutPreset(preset));
}

void openEditorPanel(int request) {
    GetWorkbenchRuntime().OpenEditorPanel(toEditorRequest(request));
}

void openBuilderPanel(int request) {
    GetWorkbenchRuntime().OpenBuilderPanel(toBuilderRequest(request));
}

void openDataPanel(int request) {
    GetWorkbenchRuntime().OpenDataPanel(toDataRequest(request));
}

bool isShellWindowVisible(int windowId) {
    return GetWorkbenchRuntime().IsShellWindowVisible(toShellWindowId(windowId));
}

int getPendingLayoutPreset() {
    return GetWorkbenchRuntime().GetPendingLayoutPreset();
}

bool hasPendingEditorRequest() {
    return GetWorkbenchRuntime().HasPendingEditorRequest();
}

bool hasPendingBuilderRequest() {
    return GetWorkbenchRuntime().HasPendingBuilderRequest();
}

bool hasPendingDataRequest() {
    return GetWorkbenchRuntime().HasPendingDataRequest();
}

void writeChunk(const std::string& fileName, int32_t offset, uintptr_t data, int32_t length) {
    GetWorkbenchRuntime().WriteChunk(fileName, offset, data, length);
}

void closeFile(const std::string& fileName) {
    GetWorkbenchRuntime().CloseFile(fileName);
}

void processFileInBackground(const std::string& fileName, bool deleteFile) {
    GetWorkbenchRuntime().ProcessFileInBackground(fileName, deleteFile);
}

void showProgressPopup(bool show) {
    GetWorkbenchRuntime().ShowProgressPopup(show);
}

void setProgressPopupText(const std::string& title, const std::string& text) {
    GetWorkbenchRuntime().SetProgressPopupText(title, text);
}

#ifdef DEBUG_BUILD
void printMeshTree() {
    GetWorkbenchRuntime().PrintMeshTree();
}
#endif

}  // namespace

EMSCRIPTEN_BINDINGS(Constant) {
    emscripten::function("initIdbfs", &initIdbfs);
    emscripten::function("saveImGuiIniFile", &saveImGuiIniFile);
    emscripten::function("loadImGuiIniFile", &loadImGuiIniFile);
    emscripten::function("loadArrayBuffer", &loadArrayBuffer);
    emscripten::function("loadChgcarFile", &loadChgcarFile);
    emscripten::function("handleXSFGridFile", &handleXSFGridFile);
    emscripten::function("handleStructureFile", &handleStructureFile);
    emscripten::function("getStructureCount", &getStructureCount);
    emscripten::function("getCurrentStructureId", &getCurrentStructureId);
    emscripten::function("isStructureVisible", &isStructureVisible);
    emscripten::function("setStructureVisible", &setStructureVisible);
    emscripten::function("getMeshCount", &getMeshCount);
    emscripten::function("hasChargeDensity", &hasChargeDensity);
    emscripten::function("requestLayoutPreset", &requestLayoutPreset);
    emscripten::function("openEditorPanel", &openEditorPanel);
    emscripten::function("openBuilderPanel", &openBuilderPanel);
    emscripten::function("openDataPanel", &openDataPanel);
    emscripten::function("isShellWindowVisible", &isShellWindowVisible);
    emscripten::function("getPendingLayoutPreset", &getPendingLayoutPreset);
    emscripten::function("hasPendingEditorRequest", &hasPendingEditorRequest);
    emscripten::function("hasPendingBuilderRequest", &hasPendingBuilderRequest);
    emscripten::function("hasPendingDataRequest", &hasPendingDataRequest);
#ifdef DEBUG_BUILD
    emscripten::function("printMeshTree", &printMeshTree);
#endif
    emscripten::function("writeChunk", &writeChunk);
    emscripten::function("closeFile", &closeFile);
    emscripten::function("processFileInBackground", &processFileInBackground);
    emscripten::function("showProgressPopup", &showProgressPopup);
    emscripten::function("setProgressPopupText", &setProgressPopupText);
}

