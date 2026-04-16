#include "import_entry_service.h"

#include "../../common/string_utils.h"
#include "../../config/log_config.h"
#include "../../mesh/application/mesh_command_service.h"
#include "../../shell/runtime/workbench_runtime.h"
#include "../../shell/presentation/import/import_popup_presenter.h"
#include "../infrastructure/unv/unv_mesh_reader.h"
#include "../../render/application/render_gateway.h"

// Standard library
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <utility>

// VTK
#include <vtkAlgorithm.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkDataSet.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>

FileLoader::FileLoader(
    io::application::FileDialogPort& fileDialogPort,
    io::application::WorkerPort& workerPort,
    io::application::ProgressPort& progressPort)
    : m_ProgressCallback(vtkSmartPointer<vtkCallbackCommand>::New()),
      m_ParserWorkerService(workerPort, progressPort),
      m_FileDialogPort(fileDialogPort),
      m_WorkerPort(workerPort),
      m_ProgressPort(progressPort),
      m_ImportPopupPresenter(std::make_unique<shell::presentation::ImportPopupPresenter>()) {
    m_ProgressCallback->SetClientData(this);
    m_ProgressCallback->SetCallback(FileLoader::progressCallback);
}

FileLoader::~FileLoader() {
    if (m_LoadingThread.joinable()) {
        m_LoadingThread.join();
    }
    m_MemfsChunkStream.CloseAll();
}

void FileLoader::RequestOpenStructureImport() {
    // Clear stale one-shot state from previous canceled requests.
    m_ReplaceSceneOnNextStructureImport = false;
    m_DeferredStructureFileName.clear();

    if (!hasSceneDataForStructureImport()) {
        OpenStructureFileBrowser();
        return;
    }
    m_ShowStructureReplacePopup = true;
}

bool FileLoader::hasSceneDataForStructureImport() const {
    return m_ImportWorkflowService.HasSceneDataForStructureImport();
}

void FileLoader::clearReplaceSceneImportTransaction() {
    m_ReplaceSceneOnNextStructureImport = false;
    m_DeferredStructureFileName.clear();
    m_ImportWorkflowService.ClearReplaceSceneImportTransaction();
}

void FileLoader::finalizeImportOnSuccess(int32_t importedStructureId) {
    if (!m_ImportWorkflowService.IsReplaceSceneImportTransactionActive()) {
        return;
    }
    m_ImportWorkflowService.FinalizeImportOnSuccess(importedStructureId);
    clearReplaceSceneImportTransaction();
}

void FileLoader::rollbackImportOnFailure(int32_t importedStructureId) {
    if (!m_ImportWorkflowService.IsReplaceSceneImportTransactionActive()) {
        return;
    }
    m_ImportWorkflowService.RollbackImportOnFailure(importedStructureId);
    clearReplaceSceneImportTransaction();
}

void FileLoader::cleanupImportedStructure(int32_t importedStructureId) {
    m_ImportWorkflowService.CleanupImportedStructure(importedStructureId);
}

void FileLoader::showStructureImportErrorPopup(const std::string& title, const std::string& message) {
    m_StructureImportErrorTitle = title.empty() ? "Import Structure Failed" : title;
    if (message.empty()) {
        m_StructureImportErrorMessage = "The selected structure file could not be imported.";
    } else {
        m_StructureImportErrorMessage = message;
    }
    m_ShowStructureImportErrorPopup = true;
}

void FileLoader::OpenFileBrowser(bool useMainThread) {
    m_FileDialogPort.OpenMeshFileBrowser(useMainThread);
}

void FileLoader::OpenStructureFileBrowser() {
    m_FileDialogPort.OpenStructureFileBrowser();
}

void FileLoader::OpenXSFFileBrowser() {
    m_FileDialogPort.OpenXsfFileBrowser();
}

void FileLoader::OpenXSFGridFileBrowser() {
    m_FileDialogPort.OpenXsfGridFileBrowser();
}

void FileLoader::handleXSFFile(const std::string& fileName) {
    processXSFFileInBackground(fileName);
}

void FileLoader::HandleXSFGridFile(const std::string& fileName) {
    handleXSFGridFile(fileName);
}

void FileLoader::HandleStructureFile(const std::string& fileName) {
    handleStructureFile(fileName);
}

void FileLoader::handleXSFGridFile(const std::string& fileName) {
    processXSFGridFileInBackground(fileName);
}

void FileLoader::handleStructureFile(const std::string& fileName) {
    if (!m_ReplaceSceneOnNextStructureImport && hasSceneDataForStructureImport()) {
        m_DeferredStructureFileName = fileName;
        m_ShowStructureReplacePopup = true;
        GetWorkbenchRuntime().ShowProgressPopup(false);
        return;
    }

    if (m_ReplaceSceneOnNextStructureImport) {
        m_ImportWorkflowService.BeginReplaceSceneImportTransaction();
        m_ReplaceSceneOnNextStructureImport = false;
    }

    std::string extension;
    std::string fileLower = fileName;
    StringUtils::ToLower(fileLower);

    std::string::size_type dotIndex = fileName.rfind('.');
    if (dotIndex != std::string::npos && dotIndex + 1 < fileName.size()) {
        extension = fileName.substr(dotIndex + 1);
        StringUtils::ToLower(extension);
    }

    if (extension == "xsf") {
        processStructureFileInBackground(fileName);
        return;
    }

    if (extension == "vasp" || fileLower.find("chgcar") != std::string::npos) {
        processChgcarFileInBackground(fileName);
        return;
    }

    handleXSFFile(fileName);
}

void FileLoader::LoadArrayBuffer(const std::string& fileName, bool deleteFile) {
    std::string::size_type dotIndex = fileName.rfind('.');
    if (dotIndex == std::string::npos) {
        SPDLOG_ERROR("File name does not have an extension: {}", fileName);
        return;
    }

    std::string extension = fileName.substr(dotIndex + 1);
    if (extension.empty()) {
        SPDLOG_ERROR("File name does not have an extension: {}", fileName);
        return;
    }
    StringUtils::ToLower(extension);

    if (extension == "xsf") {
        try {
            SPDLOG_INFO("Processing XSF file: {}", fileName);
            handleXSFFile(fileName);
        }
        catch (const std::exception& e) {
            SPDLOG_ERROR("Error handling XSF file: {}: {}", fileName, e.what());
        }
        return;
    }

    Mesh* newMesh = nullptr;
    if (extension == "vtk") {
        newMesh = readVtkFile(fileName);
    }
    else if (extension == "vtu") {
        newMesh = readVtuFile(fileName);
    }
    else if (extension == "unv") {
        newMesh = readUnvFile(fileName);
    }

    CloseFile(fileName);
    if (deleteFile) {
        std::remove(fileName.c_str());
    }

    if (newMesh == nullptr) {
        SPDLOG_ERROR("Failed to load file: {}", fileName);
        return;
    }

    m_WorkerPort.DispatchToMainThreadSync(DisplayNewMesh, newMesh);
}

Mesh* FileLoader::readVtkFile(const std::string& fileName) {
    vtkNew<vtkGenericDataObjectReader> genericReader;
    genericReader->SetFileName(fileName.c_str());
    genericReader->AddObserver(vtkCommand::ProgressEvent, m_ProgressCallback);
    genericReader->Update();

    vtkSmartPointer<vtkDataSet> volumeDataSet = vtkDataSet::SafeDownCast(genericReader->GetOutput());
    if (!volumeDataSet) {
        SPDLOG_ERROR("Failed to load file: {}", fileName);
        return nullptr;
    }

    const std::string fileNameOnly = fileName.substr(0, fileName.rfind('.'));
    Mesh* newMesh = mesh::application::GetMeshCommandService().InsertMesh(
        fileNameOnly.c_str(), nullptr, nullptr, volumeDataSet);
    return newMesh;
}

Mesh* FileLoader::readVtuFile(const std::string& fileName) {
    vtkNew<vtkXMLUnstructuredGridReader> reader;
    reader->SetFileName(fileName.c_str());
    reader->AddObserver(vtkCommand::ProgressEvent, m_ProgressCallback);
    reader->Update();

    vtkSmartPointer<vtkUnstructuredGrid> volumeDataSet = reader->GetOutput();
    if (!volumeDataSet) {
        SPDLOG_ERROR("Failed to load file: {}", fileName);
        return nullptr;
    }

    const std::string fileNameOnly = fileName.substr(0, fileName.rfind('.'));
    Mesh* newMesh = mesh::application::GetMeshCommandService().InsertMesh(
        fileNameOnly.c_str(), nullptr, nullptr, volumeDataSet);
    return newMesh;
}

Mesh* FileLoader::readUnvFile(const std::string& fileName) {
    UnvReaderUPtr unvReader = UnvReader::New(fileName.c_str());
    if (!unvReader) {
        SPDLOG_ERROR("Failed to create UNV reader: {}", fileName);
        return nullptr;
    }

    unvReader->SetProgressCallback([this](float progress) {
        m_ProgressPort.ReportProgress(progress);
    });

    if (!unvReader->ReadUnvFile()) {
        SPDLOG_ERROR("Failed to read UNV file: {}", fileName);
        return nullptr;
    }

    const std::string fileNameOnly = fileName.substr(0, fileName.rfind('.'));
    Mesh* newMesh = mesh::application::GetMeshCommandService().InsertMesh(
        fileNameOnly.c_str(),
        unvReader->GetEdgeMesh(),
        unvReader->GetFaceMesh(),
        unvReader->GetVolumeMesh());

    if (unvReader->GetMeshGroupCount() > 0) {
        newMesh->SetMeshGroups(unvReader->MoveMeshGroups());
    }

    return newMesh;
}

void FileLoader::OpenFileStreamingBrowser(bool useMainThread) {
    m_FileDialogPort.OpenMeshFileStreamingBrowser(useMainThread);
}

void FileLoader::WriteChunk(const std::string& fileName, int32_t offset, uintptr_t data, int32_t length) {
    m_MemfsChunkStream.WriteChunk(fileName, offset, data, length);
}

void FileLoader::CloseFile(const std::string& fileName) {
    m_MemfsChunkStream.CloseFile(fileName);
}

void FileLoader::progressCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData) {
    vtkAlgorithm* reader = static_cast<vtkAlgorithm*>(caller);
    FileLoader* fileLoader = static_cast<FileLoader*>(clientData);

    double progress = reader->GetProgress();
    if (fileLoader != nullptr) {
        fileLoader->m_ProgressPort.ReportProgress(static_cast<float>(progress));
    }
    SPDLOG_INFO("Loading object - Progress: {:.2f}%", progress * 100.0);
}

void FileLoader::ProcessFileInBackground(const std::string& fileName, bool deleteFile) {
    processFileInBackground(fileName, deleteFile);
}

void FileLoader::processFileInBackground(const std::string& fileName, bool deleteFile) {
    if (m_LoadingThread.joinable()) {
        m_LoadingThread.join();
    }

    m_LoadingThread = std::thread([this, fileName, deleteFile]() {
        try {
            LoadArrayBuffer(fileName, deleteFile);
            GetWorkbenchRuntime().ShowProgressPopup(false);
        }
        catch (const std::exception& e) {
            SPDLOG_ERROR("Error loading file: {}", e.what());
            GetWorkbenchRuntime().ShowProgressPopup(false);
        }
    });
}

void FileLoader::processStructureFileInBackground(const std::string& fileName) {
    m_ParserWorkerService.ProcessStructureFile(fileName, OnParserWorkerResult);
}

void FileLoader::processXSFFileInBackground(const std::string& fileName) {
    m_ParserWorkerService.ProcessXsfFile(fileName, OnParserWorkerResult);
}

void FileLoader::processXSFGridFileInBackground(const std::string& fileName) {
    m_ParserWorkerService.ProcessXsfGridFile(fileName, OnParserWorkerResult);
}

void FileLoader::processChgcarFileInBackground(const std::string& fileName) {
    m_ParserWorkerService.ProcessChgcarFile(fileName, OnParserWorkerResult);
}

void FileLoader::DisplayNewMesh(int dummy, const void* mesh) {
    const Mesh* newMesh = static_cast<const Mesh*>(mesh);
    render::application::GetRenderGateway().AddActor(newMesh->GetVolumeMeshActor(), true);
}

void FileLoader::OnParserWorkerResult(int dummy, const void* data) {
    std::unique_ptr<io::application::ParserWorkerResult> result(
        static_cast<io::application::ParserWorkerResult*>(const_cast<void*>(data)));
    if (!result) {
        return;
    }

    GetWorkbenchRuntime().FileLoaderService().handleParserWorkerResult(*result);
}

void FileLoader::handleParserWorkerResult(io::application::ParserWorkerResult& result) {
    auto finishWithCleanup = [&result]() {
        if (!result.filePath.empty()) {
            std::remove(result.filePath.c_str());
            SPDLOG_INFO("Temporary import file deleted: {}", result.filePath);
        }
        GetWorkbenchRuntime().ShowProgressPopup(false);
    };

    io::application::ImportApplyResult applyResult = m_ImportApplyService.Apply(result);
    if (!applyResult.success) {
        SPDLOG_ERROR("Failed to apply import parse result: {}", applyResult.errorMessage);
        showStructureImportErrorPopup("Import Structure Failed", applyResult.errorMessage);
        if (m_ImportWorkflowService.IsReplaceSceneImportTransactionActive()) {
            rollbackImportOnFailure(applyResult.importedStructureId);
        } else {
            cleanupImportedStructure(applyResult.importedStructureId);
        }
        finishWithCleanup();
        return;
    }

    if (m_ImportWorkflowService.IsReplaceSceneImportTransactionActive()) {
        finalizeImportOnSuccess(applyResult.importedStructureId);
    }

    if (applyResult.showXsfGridCellWarning) {
        m_XsfGridCellWarningText = applyResult.xsfGridCellWarningText;
        m_ShowXsfGridCellWarningPopup = true;
    }

    render::application::GetRenderGateway().ResetView();
    finishWithCleanup();
}

void FileLoader::RenderXsfGridImportPopups() {
    if (m_ImportPopupPresenter) {
        m_ImportPopupPresenter->Render(*this);
    }
}

void FileLoader::OpenChgcarFileBrowser() {
    m_FileDialogPort.OpenChgcarFileBrowser();
}

void FileLoader::LoadChgcarFile(const std::string& filename) {
    SPDLOG_INFO("LoadChgcarFile called: {}", filename);
    processChgcarFileInBackground(filename);
}
