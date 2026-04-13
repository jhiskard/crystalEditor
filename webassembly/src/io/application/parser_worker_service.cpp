#include "parser_worker_service.h"

#include "../../config/log_config.h"
#include "../../shell/runtime/workbench_runtime.h"

// Standard library
#include <fstream>
#include <thread>
#include <utility>

namespace io::application {

ParserWorkerService::ParserWorkerService(WorkerPort& workerPort, ProgressPort& progressPort)
    : m_WorkerPort(workerPort), m_ProgressPort(progressPort) {}

bool ParserWorkerService::ContainsDatagrid3d(const std::string& filePath) {
    std::ifstream fin(filePath);
    if (!fin) {
        return false;
    }
    std::string line;
    while (std::getline(fin, line)) {
        if (line.find("DATAGRID_3D") != std::string::npos) {
            return true;
        }
    }
    return false;
}

void ParserWorkerService::DispatchToMainThread(ParserResultCallback callback, ParserWorkerResult* result) const {
    if (callback == nullptr) {
        delete result;
        return;
    }

    m_WorkerPort.DispatchToMainThreadAsync(callback, result);
}

void ParserWorkerService::ProcessStructureFile(
    const std::string& fileName,
    ParserResultCallback callback) const {
    std::thread([this, fileName, callback]() {
        const std::string filePath = "/" + fileName;
        if (ContainsDatagrid3d(filePath)) {
            SPDLOG_INFO("Detected DATAGRID_3D in structure file: {}", fileName);

            atoms::infrastructure::FileIOManager loader(&GetWorkbenchRuntime().AtomsTemplateFacade());
            loader.SetProgressCallback([this](float progress) {
                m_ProgressPort.ReportProgress(progress);
            });

            auto parseResult = loader.load3DGridXSFFile(filePath);
            auto* result = new ParserWorkerResult();
            result->kind = ParsedImportKind::XsfGrid;
            result->filePath = filePath;
            result->xsfGridResult = std::move(parseResult);
            DispatchToMainThread(callback, result);
            return;
        }

        SPDLOG_INFO("Starting XSF structure file processing (background): {}", fileName);
        atoms::infrastructure::FileIOManager loader(&GetWorkbenchRuntime().AtomsTemplateFacade());
        loader.SetProgressCallback([this](float progress) {
            m_ProgressPort.ReportProgress(progress);
        });

        auto parseResult = loader.loadXSFFile(filePath);
        auto* result = new ParserWorkerResult();
        result->kind = ParsedImportKind::XsfStructure;
        result->filePath = filePath;
        result->xsfResult = std::move(parseResult);
        DispatchToMainThread(callback, result);
    }).detach();
}

void ParserWorkerService::ProcessXsfFile(
    const std::string& fileName,
    ParserResultCallback callback) const {
    std::thread([this, fileName, callback]() {
        SPDLOG_INFO("Starting XSF file processing (background): {}", fileName);
        const std::string filePath = "/" + fileName;

        atoms::infrastructure::FileIOManager loader(&GetWorkbenchRuntime().AtomsTemplateFacade());
        loader.SetProgressCallback([this](float progress) {
            m_ProgressPort.ReportProgress(progress);
        });

        auto parseResult = loader.loadXSFFile(filePath);
        auto* result = new ParserWorkerResult();
        result->kind = ParsedImportKind::XsfStructure;
        result->filePath = filePath;
        result->xsfResult = std::move(parseResult);
        DispatchToMainThread(callback, result);
    }).detach();
}

void ParserWorkerService::ProcessXsfGridFile(
    const std::string& fileName,
    ParserResultCallback callback) const {
    std::thread([this, fileName, callback]() {
        SPDLOG_INFO("Starting XSF grid file processing (background): {}", fileName);
        const std::string filePath = "/" + fileName;

        atoms::infrastructure::FileIOManager loader(&GetWorkbenchRuntime().AtomsTemplateFacade());
        loader.SetProgressCallback([this](float progress) {
            m_ProgressPort.ReportProgress(progress);
        });

        auto parseResult = loader.load3DGridXSFFile(filePath);
        auto* result = new ParserWorkerResult();
        result->kind = ParsedImportKind::XsfGrid;
        result->filePath = filePath;
        result->xsfGridResult = std::move(parseResult);
        DispatchToMainThread(callback, result);
    }).detach();
}

void ParserWorkerService::ProcessChgcarFile(
    const std::string& fileName,
    ParserResultCallback callback) const {
    std::thread([this, fileName, callback]() {
        SPDLOG_INFO("Starting CHGCAR file processing (background): {}", fileName);
        const std::string filePath = "/" + fileName;

        auto parseResult = atoms::infrastructure::ChgcarParser::parse(
            filePath,
            [this](float progress) {
                m_ProgressPort.ReportProgress(progress);
            });

        auto* result = new ParserWorkerResult();
        result->kind = ParsedImportKind::Chgcar;
        result->filePath = filePath;
        result->chgcarResult = std::move(parseResult);
        DispatchToMainThread(callback, result);
    }).detach();
}

} // namespace io::application
