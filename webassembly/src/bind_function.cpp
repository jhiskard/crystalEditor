#include "app.h"
#include "atoms/atoms_template.h"
#include "file_loader.h"
#include "mesh_manager.h"

// Emscripten
#include <emscripten/bind.h>

namespace {

int getStructureCount() {
    return static_cast<int>(AtomsTemplate::Instance().GetStructures().size());
}

int getCurrentStructureId() {
    return AtomsTemplate::Instance().GetCurrentStructureId();
}

bool isStructureVisible(int id) {
    return AtomsTemplate::Instance().IsStructureVisible(id);
}

void setStructureVisible(int id, bool visible) {
    AtomsTemplate::Instance().SetStructureVisible(id, visible);
}

int getMeshCount() {
    return static_cast<int>(MeshManager::Instance().GetMeshCount());
}

bool hasChargeDensity() {
    return AtomsTemplate::Instance().HasChargeDensity();
}

} // namespace

EMSCRIPTEN_BINDINGS(Constant) {
    emscripten::function("initIdbfs", &App::InitIdbfs);
    emscripten::function("saveImGuiIniFile", &App::SaveImGuiIniFile);
    emscripten::function("loadImGuiIniFile", &App::LoadImGuiIniFile);
    emscripten::function("loadArrayBuffer", &FileLoader::LoadArrayBuffer);
    // CHGCAR 파일 로더 바인딩
    emscripten::function("loadChgcarFile", &FileLoader::LoadChgcarFile);
    // XSF 그리드 파일 핸들러 바인딩
    emscripten::function("handleXSFGridFile", &FileLoader::HandleXSFGridFile);
    emscripten::function("handleStructureFile", &FileLoader::HandleStructureFile);
    emscripten::function("getStructureCount", &getStructureCount);
    emscripten::function("getCurrentStructureId", &getCurrentStructureId);
    emscripten::function("isStructureVisible", &isStructureVisible);
    emscripten::function("setStructureVisible", &setStructureVisible);
    emscripten::function("getMeshCount", &getMeshCount);
    emscripten::function("hasChargeDensity", &hasChargeDensity);
#ifdef DEBUG_BUILD
    emscripten::function("printMeshTree", &MeshManager::PrintMeshTree);
#endif
    emscripten::function("writeChunk", &FileLoader::WriteChunk);
    emscripten::function("closeFile", &FileLoader::CloseFile);
    emscripten::function("processFileInBackground", &FileLoader::ProcessFileInBackground);
    emscripten::function("showProgressPopup", &App::ShowProgressPopup);
    emscripten::function("setProgressPopupText", &App::SetProgressPopupText);
}
