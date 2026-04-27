#pragma once

#include <cstdint>
#include <string>

class App;
class FileLoader;
class FontManager;
class MeshDetail;
class MeshGroupDetail;
class ModelTree;
class TestWindow;
class Toolbar;
class VtkViewer;

namespace structure {
namespace application {
class StructureInteractionService;
class StructureLifecycleService;
class StructureService;
} // namespace application
} // namespace structure

namespace measurement {
namespace application {
class MeasurementService;
} // namespace application
} // namespace measurement

namespace density {
namespace application {
class DensityService;
} // namespace application
} // namespace density

namespace mesh {
namespace domain {
class MeshRepository;
} // namespace domain
} // namespace mesh

namespace workspace {
namespace application {
class WorkspaceQueryService;
class WorkspaceCommandService;
} // namespace application
} // namespace workspace

namespace shell {
namespace application {
class ShellStateQueryService;
class ShellStateCommandService;
class WorkbenchController;
} // namespace application
namespace domain {
enum class ShellLayoutPreset;
enum class ShellWindowId;
} // namespace domain
} // namespace shell

namespace workbench {
namespace panel {
enum class EditorRequest;
enum class BuilderRequest;
enum class DataRequest;
} // namespace panel
} // namespace workbench

/**
 * @brief Runtime composition root facade for legacy singleton graph.
 * @details main/app/wasm binding 계층은 이 타입을 통해서만 런타임 객체에 접근한다.
 *          Phase 13에서는 App/Toolbar 진입점을 runtime-owned instance로 전환했다.
 */
class WorkbenchRuntime {
public:
    /**
     * @brief Returns the global runtime facade instance.
     */
    static WorkbenchRuntime& Instance();

    /**
     * @brief Initializes singleton-backed services used during runtime bootstrap.
     * @details Services that touch ImGui state or OpenGL resources (for example fonts/icons/textures)
     *          must be initialized after `ImGui::CreateContext()` and after a valid GL context exists.
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
     * @brief Returns toolbar facade owned behind runtime boundary.
     * @note Runtime owns the concrete toolbar presenter instance.
     */
    Toolbar& ToolbarPanel();

    /**
     * @brief Returns viewer facade.
     */
    VtkViewer& Viewer();

    /**
     * @brief Returns structure feature service facade.
     */
    structure::application::StructureService& StructureFeature();

    /**
     * @brief Returns structure interaction service facade.
     */
    structure::application::StructureInteractionService& StructureInteractionFeature();

    /**
     * @brief Returns structure lifecycle/BZ feature service facade.
     */
    structure::application::StructureLifecycleService& StructureLifecycleFeature();

    /**
     * @brief Returns measurement feature service facade.
     */
    measurement::application::MeasurementService& MeasurementFeature();

    /**
     * @brief Returns density feature service facade.
     */
    density::application::DensityService& DensityFeature();

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
    mesh::domain::MeshRepository& MeshRepository();

    /**
     * @brief Returns import/file-loading service facade.
     * @note Runtime owns this service instance and mediates all wasm import entrypoints.
     */
    FileLoader& FileLoaderService();

    /**
     * @brief Returns workspace context query facade.
     */
    workspace::application::WorkspaceQueryService& WorkspaceQuery();

    /**
     * @brief Returns workspace context command facade.
     */
    workspace::application::WorkspaceCommandService& WorkspaceCommand();

    /**
     * @brief Returns shell state query facade.
     */
    shell::application::ShellStateQueryService& ShellStateQuery();

    /**
     * @brief Returns shell state command facade.
     */
    shell::application::ShellStateCommandService& ShellStateCommand();

    /**
     * @brief Returns shell workbench controller.
     */
    shell::application::WorkbenchController& ShellController();

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
    void SetProgress(float progress);
    void ShowProgressPopup(bool show);
    void SetProgressPopupText(const std::string& title, const std::string& text);

    /**
     * @brief Runtime wrapper for import APIs exposed to wasm bindings.
     * @details JS/wasm bindings must not call `FileLoader` static entrypoints directly.
     *          Route every import action through this facade to preserve runtime ownership.
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

    /**
     * @brief Runtime wrapper for shell layout/panel test hooks.
     * @details These hooks are used only by wasm test seam (`__VTK_WORKBENCH_TEST__`)
     *          to validate layout/reset and menu-panel synchronization contracts.
     */
    void RequestLayoutPreset(shell::domain::ShellLayoutPreset preset);
    void OpenEditorPanel(workbench::panel::EditorRequest request);
    void OpenBuilderPanel(workbench::panel::BuilderRequest request);
    void OpenDataPanel(workbench::panel::DataRequest request);
    bool IsShellWindowVisible(shell::domain::ShellWindowId windowId);
    int GetPendingLayoutPreset();
    bool HasPendingEditorRequest();
    bool HasPendingBuilderRequest();
    bool HasPendingDataRequest();

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
