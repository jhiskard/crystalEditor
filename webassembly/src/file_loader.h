#pragma once

#include "macro/singleton_macro.h"
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


class FileLoader {
    DECLARE_SINGLETON(FileLoader)

public:
    void OpenFileBrowser(bool useMainThread = true);
    void OpenFileStreamingBrowser(bool useMainThread = true);
    void RequestOpenStructureImport();
    void OpenStructureFileBrowser();
    void OpenXSFFileBrowser();
    void OpenXSFGridFileBrowser();

    // fileName: MemFS path
    // deleteFile: remove MemFS file from this function
    static void LoadArrayBuffer(const std::string& fileName, bool deleteFile);
    static void WriteChunk(const std::string& fileName, int32_t offset, uintptr_t data, int32_t length);
    static void CloseFile(const std::string& fileName);
    static void HandleXSFGridFile(const std::string& fileName);
    static void HandleStructureFile(const std::string& fileName);
    static void ProcessFileInBackground(const std::string& fileName, bool deleteFile);

    void handleXSFFile(const std::string& fileName);
    void handleXSFGridFile(const std::string& fileName);
    void handleStructureFile(const std::string& fileName);
    void RenderXsfGridImportPopups();

    void OpenChgcarFileBrowser();
    static void LoadChgcarFile(const std::string& filename);

private:
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
