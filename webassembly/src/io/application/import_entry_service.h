#pragma once

#include "import_apply_service.h"
#include "file_dialog_port.h"
#include "import_popup_state.h"
#include "progress_port.h"
#include "import_workflow_service.h"
#include "parser_worker_service.h"
#include "worker_port.h"
#include "../platform/memfs_chunk_stream.h"

// Standard library
#include <memory>
#include <string>
#include <thread>

// VTK
#include <vtkSmartPointer.h>

class vtkObject;
class vtkCallbackCommand;
class Mesh;
class WorkbenchRuntime;

namespace shell::presentation {
class ImportPopupPresenter;
} // namespace shell::presentation


/**
 * @brief Import/file-loading coordinator owned by runtime composition root.
 * @details Wasm bindings and shell actions must access this service only via
 *          `WorkbenchRuntime` wrappers to keep import entrypoints centralized.
 */
class FileLoader {
public:
    FileLoader(
        io::application::FileDialogPort& fileDialogPort,
        io::application::WorkerPort& workerPort,
        io::application::ProgressPort& progressPort);
    ~FileLoader();

    /**
     * @brief Requests mesh file picker from browser platform adapter.
     */
    void OpenFileBrowser(bool useMainThread = true);
    void OpenFileStreamingBrowser(bool useMainThread = true);
    /**
     * @brief Opens guarded structure import flow (replace-scene confirmation included).
     */
    void RequestOpenStructureImport();
    void OpenStructureFileBrowser();
    void OpenXSFFileBrowser();
    void OpenXSFGridFileBrowser();

    /**
     * @brief Loads a MemFS file into runtime import pipeline.
     * @param fileName MemFS path.
     * @param deleteFile When true, remove temporary MemFS file after processing.
     */
    void LoadArrayBuffer(const std::string& fileName, bool deleteFile);
    void WriteChunk(const std::string& fileName, int32_t offset, uintptr_t data, int32_t length);
    void CloseFile(const std::string& fileName);
    void HandleXSFGridFile(const std::string& fileName);
    void HandleStructureFile(const std::string& fileName);
    void ProcessFileInBackground(const std::string& fileName, bool deleteFile);

    void handleXSFFile(const std::string& fileName);
    void handleXSFGridFile(const std::string& fileName);
    void handleStructureFile(const std::string& fileName);
    void RenderXsfGridImportPopups();

    void OpenChgcarFileBrowser();
    void LoadChgcarFile(const std::string& filename);

    /**
     * @brief Returns immutable popup state for shell presentation layer.
     */
    const io::application::ImportPopupState& getImportPopupState() const noexcept;

    /**
     * @brief Opens structure import browser with replace-scene decision.
     */
    void acknowledgeStructureReplacePopup(bool shouldReplace);

    /**
     * @brief Closes structure import error popup and clears message state.
     */
    void dismissStructureImportErrorPopup();

    /**
     * @brief Closes XSF grid warning popup and clears warning message.
     */
    void dismissXsfGridCellWarningPopup();

private:
    FileLoader(const FileLoader&) = delete;
    FileLoader& operator=(const FileLoader&) = delete;
    FileLoader(FileLoader&&) = delete;
    FileLoader& operator=(FileLoader&&) = delete;

    vtkSmartPointer<vtkCallbackCommand> m_ProgressCallback;

    std::thread m_LoadingThread;
    io::application::ParserWorkerService m_ParserWorkerService;
    io::application::ImportApplyService m_ImportApplyService;
    io::application::ImportWorkflowService m_ImportWorkflowService;
    io::application::FileDialogPort& m_FileDialogPort;
    io::application::WorkerPort& m_WorkerPort;
    io::application::ProgressPort& m_ProgressPort;
    io::platform::MemfsChunkStream m_MemfsChunkStream;
    std::unique_ptr<shell::presentation::ImportPopupPresenter> m_ImportPopupPresenter;
    io::application::ImportPopupState m_ImportPopupState;

    Mesh* readVtkFile(const std::string& fileName);
    Mesh* readVtuFile(const std::string& fileName);
    Mesh* readUnvFile(const std::string& fileName);

    static void progressCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
    void processFileInBackground(const std::string& fileName, bool deleteFile);
    void processStructureFileInBackground(const std::string& fileName);
    void processXSFFileInBackground(const std::string& fileName);
    void processXSFGridFileInBackground(const std::string& fileName);
    void processChgcarFileInBackground(const std::string& fileName);
    bool hasSceneDataForStructureImport() const;

    static void DisplayNewMesh(int dummy, const void* mesh);
    static void OnParserWorkerResult(int dummy, const void* data);
    void handleParserWorkerResult(io::application::ParserWorkerResult& result);
    void clearReplaceSceneImportTransaction();
    void finalizeImportOnSuccess(int32_t importedStructureId);
    void rollbackImportOnFailure(int32_t importedStructureId);
    void cleanupImportedStructure(int32_t importedStructureId);
    void showStructureImportErrorPopup(const std::string& title, const std::string& message);
};
