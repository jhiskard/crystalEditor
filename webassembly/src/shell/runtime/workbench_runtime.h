#pragma once

#include <cstdint>
#include <string>

class App;
class AtomsTemplate;
class FileLoader;
class FontManager;
class MeshDetail;
class MeshGroupDetail;
class MeshManager;
class ModelTree;
class TestWindow;
class VtkViewer;

/**
 * @brief Runtime composition root facade for legacy singleton graph.
 * @details Phase 7에서는 singleton 제거 대신 접근 경계를 runtime으로 제한한다.
 *          main/app/wasm binding 계층은 이 타입을 통해서만 런타임 객체에 접근한다.
 */
class WorkbenchRuntime {
public:
    /**
     * @brief Returns the global runtime facade instance.
     */
    static WorkbenchRuntime& Instance();

    /**
     * @brief Initializes singleton-backed services that can be warmed up before ImGui context creation.
     * @details Services that touch ImGui state (for example fonts/icons) must be initialized
     *          after `ImGui::CreateContext()` in the bootstrap flow.
     */
    void PrimeLegacySingletons();

    /**
     * @brief Returns app shell controller.
     */
    App& AppController();

    /**
     * @brief Returns font manager service.
     */
    FontManager& FontRegistry();

    /**
     * @brief Returns viewer facade.
     */
    VtkViewer& Viewer();

    /**
     * @brief Returns atoms feature facade.
     */
    AtomsTemplate& AtomsTemplateFacade();

    /**
     * @brief Returns model tree panel facade.
     */
    ModelTree& ModelTreePanel();

    /**
     * @brief Returns mesh detail panel facade.
     */
    MeshDetail& MeshDetailPanel();

    /**
     * @brief Returns mesh group detail panel facade.
     */
    MeshGroupDetail& MeshGroupDetailPanel();

    /**
     * @brief Returns mesh repository/service facade.
     */
    MeshManager& MeshRepository();

    /**
     * @brief Returns import/file-loading service facade.
     */
    FileLoader& FileLoaderService();

    /**
     * @brief Returns test/debug panel facade.
     */
    TestWindow& TestWindowPanel();

    /**
     * @brief Renders one app frame through the shell controller.
     */
    void RenderAppFrame();

    /**
     * @brief Returns device pixel ratio used by ImGui scale bootstrap.
     */
    float DevicePixelRatio() const;

    /**
     * @brief Runtime wrapper for app persistence APIs exposed to wasm bindings.
     */
    void InitIdbfs();
    void SaveImGuiIniFile();
    void LoadImGuiIniFile();
    void ShowProgressPopup(bool show);
    void SetProgressPopupText(const std::string& title, const std::string& text);

    /**
     * @brief Runtime wrapper for import APIs exposed to wasm bindings.
     */
    void LoadArrayBuffer(const std::string& fileName, bool deleteFile);
    void LoadChgcarFile(const std::string& fileName);
    void HandleXSFGridFile(const std::string& fileName);
    void HandleStructureFile(const std::string& fileName);
    void WriteChunk(const std::string& fileName, int32_t offset, uintptr_t data, int32_t length);
    void CloseFile(const std::string& fileName);
    void ProcessFileInBackground(const std::string& fileName, bool deleteFile);

    /**
     * @brief Runtime wrapper for import UI entrypoints used by app menu.
     */
    void RequestOpenStructureImport();
    void RenderXsfGridImportPopups();

    /**
     * @brief Runtime query/command wrappers exposed to wasm bindings.
     */
    int GetStructureCount();
    int GetCurrentStructureId();
    bool IsStructureVisible(int id);
    void SetStructureVisible(int id, bool visible);
    int GetMeshCount();
    bool HasChargeDensity();
    void PrintMeshTree();

private:
    WorkbenchRuntime() = default;
};

/**
 * @brief Returns mutable global runtime facade.
 */
WorkbenchRuntime& GetWorkbenchRuntime();

/**
 * @brief Returns const global runtime facade.
 */
const WorkbenchRuntime& GetWorkbenchRuntimeConst();
