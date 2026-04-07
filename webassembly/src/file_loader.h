#pragma once

#include "io/application/import_apply_service.h"
#include "io/application/import_workflow_service.h"
#include "io/application/parser_worker_service.h"

// Standard library
#include <cstdio>
#include <string>
#include <thread>
#include <unordered_map>

// VTK
#include <vtkSmartPointer.h>

class vtkObject;
class vtkCallbackCommand;
class Mesh;
class WorkbenchRuntime;


/**
 * @brief Import/file-loading coordinator owned by runtime composition root.
 * @details Wasm bindings and shell actions must access this service only via
 *          `WorkbenchRuntime` wrappers to keep import entrypoints centralized.
 */
class FileLoader {
public:
    FileLoader();
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

private:
    FileLoader(const FileLoader&) = delete;
    FileLoader& operator=(const FileLoader&) = delete;
    FileLoader(FileLoader&&) = delete;
    FileLoader& operator=(FileLoader&&) = delete;

    static std::unordered_map<std::string, FILE*> s_FileMap;
    vtkSmartPointer<vtkCallbackCommand> m_ProgressCallback;

    std::thread m_LoadingThread;
    io::application::ParserWorkerService m_ParserWorkerService;
    io::application::ImportApplyService m_ImportApplyService;
    io::application::ImportWorkflowService m_ImportWorkflowService;

    bool m_ShowStructureReplacePopup = false;
    bool m_ReplaceSceneOnNextStructureImport = false;
    std::string m_DeferredStructureFileName;
    bool m_ShowStructureImportErrorPopup = false;
    std::string m_StructureImportErrorTitle;
    std::string m_StructureImportErrorMessage;
    bool m_ShowXsfGridCellWarningPopup = false;
    std::string m_XsfGridCellWarningText;

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
