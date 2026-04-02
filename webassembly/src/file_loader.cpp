#include "file_loader.h"

#include "app.h"
#include "atoms/atoms_template.h"
#include "common/string_utils.h"
#include "config/log_config.h"
#include "io/platform/browser_file_picker.h"
#include "mesh_manager.h"
#include "unv_reader.h"
#include "render/application/render_gateway.h"

// Emscripten
#include <emscripten/threading.h>

// ImGui
#include <imgui.h>

// Standard library
#include <cfloat>
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

std::unordered_map<std::string, FILE*> FileLoader::s_FileMap;

FileLoader::FileLoader()
    : m_ProgressCallback(vtkSmartPointer<vtkCallbackCommand>::New()) {
    m_ProgressCallback->SetClientData(this);
    m_ProgressCallback->SetCallback(FileLoader::progressCallback);
}

FileLoader::~FileLoader() {
    if (m_LoadingThread.joinable()) {
        m_LoadingThread.join();
    }
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
    return m_ImportOrchestrator.HasSceneDataForStructureImport();
}

void FileLoader::clearReplaceSceneImportTransaction() {
    m_ReplaceSceneOnNextStructureImport = false;
    m_ReplaceSceneImportTransactionActive = false;
    m_DeferredStructureFileName.clear();
    m_ReplaceSceneImportSnapshot.Clear();
}

void FileLoader::finalizeImportOnSuccess(int32_t importedStructureId) {
    if (!m_ReplaceSceneImportTransactionActive) {
        return;
    }
    m_ImportOrchestrator.FinalizeReplaceSceneImportSuccess(
        m_ReplaceSceneImportSnapshot,
        importedStructureId);
    clearReplaceSceneImportTransaction();
}

void FileLoader::rollbackImportOnFailure(int32_t importedStructureId) {
    if (!m_ReplaceSceneImportTransactionActive) {
        return;
    }
    m_ImportOrchestrator.RollbackFailedStructureImport(
        m_ReplaceSceneImportSnapshot,
        importedStructureId);
    clearReplaceSceneImportTransaction();
}

void FileLoader::cleanupImportedStructure(int32_t importedStructureId) {
    if (importedStructureId < 0) {
        return;
    }

    MeshManager& meshManager = MeshManager::Instance();
    AtomsTemplate& atomsTemplate = AtomsTemplate::Instance();

    const Mesh* importedMesh = meshManager.GetMeshById(importedStructureId);
    if (importedMesh == nullptr) {
        return;
    }

    if (importedMesh->IsXsfStructure()) {
        atomsTemplate.RemoveStructure(importedStructureId);
        meshManager.DeleteXsfStructure(importedStructureId);
    } else {
        meshManager.DeleteMesh(importedStructureId);
    }
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
    io::platform::BrowserFilePicker::OpenMeshFileBrowser(useMainThread);
}

void FileLoader::OpenStructureFileBrowser() {
    io::platform::BrowserFilePicker::OpenStructureFileBrowser();
}

void FileLoader::OpenXSFFileBrowser() {
    io::platform::BrowserFilePicker::OpenXsfFileBrowser();
}

void FileLoader::OpenXSFGridFileBrowser() {
    io::platform::BrowserFilePicker::OpenXsfGridFileBrowser();
}

void FileLoader::handleXSFFile(const std::string& fileName) {
    processXSFFileInBackground(fileName);
}

void FileLoader::HandleXSFGridFile(const std::string& fileName) {
    Instance().handleXSFGridFile(fileName);
}

void FileLoader::HandleStructureFile(const std::string& fileName) {
    Instance().handleStructureFile(fileName);
}

void FileLoader::handleXSFGridFile(const std::string& fileName) {
    processXSFGridFileInBackground(fileName);
}

void FileLoader::handleStructureFile(const std::string& fileName) {
    if (!m_ReplaceSceneOnNextStructureImport && hasSceneDataForStructureImport()) {
        m_DeferredStructureFileName = fileName;
        m_ShowStructureReplacePopup = true;
        App::Instance().ShowProgressPopup(false);
        return;
    }

    if (m_ReplaceSceneOnNextStructureImport) {
        m_ReplaceSceneImportSnapshot =
            m_ImportOrchestrator.BeginReplaceSceneImportTransaction();
        m_ReplaceSceneImportTransactionActive = true;
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
            Instance().handleXSFFile(fileName);
        }
        catch (const std::exception& e) {
            SPDLOG_ERROR("Error handling XSF file: {}: {}", fileName, e.what());
        }
        return;
    }

    Mesh* newMesh = nullptr;
    if (extension == "vtk") {
        newMesh = Instance().readVtkFile(fileName);
    }
    else if (extension == "vtu") {
        newMesh = Instance().readVtuFile(fileName);
    }
    else if (extension == "unv") {
        newMesh = Instance().readUnvFile(fileName);
    }

    Instance().CloseFile(fileName);
    if (deleteFile) {
        std::remove(fileName.c_str());
    }

    if (newMesh == nullptr) {
        SPDLOG_ERROR("Failed to load file: {}", fileName);
        return;
    }

    emscripten_sync_run_in_main_runtime_thread(
        EM_FUNC_SIG_VIP,
        DisplayNewMesh,
        0,
        (void*)newMesh);
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
    Mesh* newMesh = MeshManager::Instance().InsertMesh(fileNameOnly.c_str(), nullptr, nullptr, volumeDataSet);
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
    Mesh* newMesh = MeshManager::Instance().InsertMesh(fileNameOnly.c_str(), nullptr, nullptr, volumeDataSet);
    return newMesh;
}

Mesh* FileLoader::readUnvFile(const std::string& fileName) {
    UnvReaderUPtr unvReader = UnvReader::New(fileName.c_str());
    if (!unvReader) {
        SPDLOG_ERROR("Failed to create UNV reader: {}", fileName);
        return nullptr;
    }

    if (!unvReader->ReadUnvFile()) {
        SPDLOG_ERROR("Failed to read UNV file: {}", fileName);
        return nullptr;
    }

    const std::string fileNameOnly = fileName.substr(0, fileName.rfind('.'));
    Mesh* newMesh = MeshManager::Instance().InsertMesh(
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
    io::platform::BrowserFilePicker::OpenMeshFileStreamingBrowser(useMainThread);
}

void FileLoader::WriteChunk(const std::string& fileName, int32_t offset, uintptr_t data, int32_t length) {
    FILE*& file = s_FileMap[fileName];

    if (!file) {
        file = fopen(fileName.c_str(), "wb+");
        if (!file) {
            SPDLOG_ERROR("Failed to open file: {}", fileName);
            return;
        }
    }

    if (fseek(file, offset, SEEK_SET) != 0) {
        SPDLOG_ERROR("Failed to seek file: {}", fileName);
        return;
    }

    size_t written = fwrite(reinterpret_cast<void*>(data), 1, length, file);
    if (written != static_cast<size_t>(length)) {
        SPDLOG_ERROR("Failed to write data to file: {}", fileName);
    }

    fflush(file);
}

void FileLoader::CloseFile(const std::string& fileName) {
    auto it = s_FileMap.find(fileName);
    if (it != s_FileMap.end()) {
        fclose(it->second);
        s_FileMap.erase(it);
    }
}

void FileLoader::progressCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData) {
    vtkAlgorithm* reader = static_cast<vtkAlgorithm*>(caller);
    FileLoader* fileLoader = static_cast<FileLoader*>(clientData);

    double progress = reader->GetProgress();
    App::Instance().SetProgress(static_cast<float>(progress));
    SPDLOG_INFO("Loading object - Progress: {:.2f}%", progress * 100.0);
}

void FileLoader::ProcessFileInBackground(const std::string& fileName, bool deleteFile) {
    FileLoader::Instance().processFileInBackground(fileName, deleteFile);
}

void FileLoader::processFileInBackground(const std::string& fileName, bool deleteFile) {
    if (m_LoadingThread.joinable()) {
        m_LoadingThread.join();
    }

    m_LoadingThread = std::thread([fileName, deleteFile]() {
        try {
            LoadArrayBuffer(fileName, deleteFile);
            App::Instance().ShowProgressPopup(false);
        }
        catch (const std::exception& e) {
            SPDLOG_ERROR("Error loading file: {}", e.what());
            App::Instance().ShowProgressPopup(false);
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

    FileLoader::Instance().handleParserWorkerResult(*result);
}

void FileLoader::handleParserWorkerResult(io::application::ParserWorkerResult& result) {
    auto finishWithCleanup = [&result]() {
        if (!result.filePath.empty()) {
            std::remove(result.filePath.c_str());
            SPDLOG_INFO("Temporary import file deleted: {}", result.filePath);
        }
        App::Instance().ShowProgressPopup(false);
    };

    io::application::ImportApplyResult applyResult = m_ImportApplyService.Apply(result);
    if (!applyResult.success) {
        SPDLOG_ERROR("Failed to apply import parse result: {}", applyResult.errorMessage);
        showStructureImportErrorPopup("Import Structure Failed", applyResult.errorMessage);
        if (m_ReplaceSceneImportTransactionActive) {
            rollbackImportOnFailure(applyResult.importedStructureId);
        } else {
            cleanupImportedStructure(applyResult.importedStructureId);
        }
        finishWithCleanup();
        return;
    }

    if (m_ReplaceSceneImportTransactionActive) {
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
    if (m_ShowStructureReplacePopup) {
        ImGui::OpenPopup("Clear Current Data and Import?");
        m_ShowStructureReplacePopup = false;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(520.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopupModal("Clear Current Data and Import?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("Viewer is not empty.");
        ImGui::TextWrapped("Do you want to clear all current data and continue importing?");

        if (ImGui::Button("Yes")) {
            if (!m_DeferredStructureFileName.empty()) {
                m_ReplaceSceneOnNextStructureImport = true;
                App::Instance().SetProgressPopupText(
                    "Loading structure file",
                    "File(" + m_DeferredStructureFileName + ") is loading, please wait...");
                App::Instance().ShowProgressPopup(true);
                const std::string deferredFileName = m_DeferredStructureFileName;
                m_DeferredStructureFileName.clear();
                handleStructureFile(deferredFileName);
            } else {
                m_ReplaceSceneOnNextStructureImport = true;
                OpenStructureFileBrowser();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            m_ReplaceSceneOnNextStructureImport = false;
            if (!m_DeferredStructureFileName.empty()) {
                const std::string memfsPath = "/" + m_DeferredStructureFileName;
                std::remove(memfsPath.c_str());
                m_DeferredStructureFileName.clear();
            }
            App::Instance().ShowProgressPopup(false);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (m_ShowStructureImportErrorPopup) {
        ImGui::OpenPopup("Import Structure Failed");
        m_ShowStructureImportErrorPopup = false;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(540.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopupModal("Import Structure Failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!m_StructureImportErrorTitle.empty()) {
            ImGui::TextUnformatted(m_StructureImportErrorTitle.c_str());
            ImGui::Separator();
        }
        if (m_StructureImportErrorMessage.empty()) {
            ImGui::TextWrapped("The selected structure file could not be imported.");
        } else {
            ImGui::TextWrapped("%s", m_StructureImportErrorMessage.c_str());
        }
        if (ImGui::Button("OK")) {
            m_StructureImportErrorTitle.clear();
            m_StructureImportErrorMessage.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (m_ShowXsfGridCellWarningPopup) {
        ImGui::OpenPopup("XSF Cell Warning");
        m_ShowXsfGridCellWarningPopup = false;
    }
    if (ImGui::BeginPopupModal("XSF Cell Warning", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (m_XsfGridCellWarningText.empty()) {
            ImGui::TextWrapped("Cell vectors differ across DATAGRID_3D blocks.");
        } else {
            ImGui::TextWrapped("%s", m_XsfGridCellWarningText.c_str());
        }
        if (ImGui::Button("OK")) {
            m_XsfGridCellWarningText.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void FileLoader::OpenChgcarFileBrowser() {
    io::platform::BrowserFilePicker::OpenChgcarFileBrowser();
}

void FileLoader::LoadChgcarFile(const std::string& filename) {
    SPDLOG_INFO("LoadChgcarFile called: {}", filename);
    Instance().processChgcarFileInBackground(filename);
}
