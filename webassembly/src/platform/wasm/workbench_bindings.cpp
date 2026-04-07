#include "../../shell/runtime/workbench_runtime.h"

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
#ifdef DEBUG_BUILD
    emscripten::function("printMeshTree", &printMeshTree);
#endif
    emscripten::function("writeChunk", &writeChunk);
    emscripten::function("closeFile", &closeFile);
    emscripten::function("processFileInBackground", &processFileInBackground);
    emscripten::function("showProgressPopup", &showProgressPopup);
    emscripten::function("setProgressPopupText", &setProgressPopupText);
}

