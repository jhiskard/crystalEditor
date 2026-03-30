#include "parser_worker_service.h"

#include "../../app.h"
#include "../../atoms/atoms_template.h"
#include "../../config/log_config.h"

// Emscripten
#include <emscripten/threading.h>

// Standard library
#include <fstream>
#include <thread>
#include <utility>

namespace io::application {

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

void ParserWorkerService::DispatchToMainThread(ParserResultCallback callback, ParserWorkerResult* result) {
    if (callback == nullptr) {
        delete result;
        return;
    }

    emscripten_async_run_in_main_runtime_thread(
        EM_FUNC_SIG_VIP,
        callback,
        0,
        result);
}

void ParserWorkerService::ProcessStructureFile(
    const std::string& fileName,
    ParserResultCallback callback) const {
    std::thread([fileName, callback]() {
        const std::string filePath = "/" + fileName;
        if (ContainsDatagrid3d(filePath)) {
            SPDLOG_INFO("Detected DATAGRID_3D in structure file: {}", fileName);

            atoms::infrastructure::FileIOManager loader(&AtomsTemplate::Instance());
            loader.SetProgressCallback([](float progress) {
                App::Instance().SetProgress(progress);
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
        atoms::infrastructure::FileIOManager loader(&AtomsTemplate::Instance());
        loader.SetProgressCallback([](float progress) {
            App::Instance().SetProgress(progress);
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
    std::thread([fileName, callback]() {
        SPDLOG_INFO("Starting XSF file processing (background): {}", fileName);
        const std::string filePath = "/" + fileName;

        atoms::infrastructure::FileIOManager loader(&AtomsTemplate::Instance());
        loader.SetProgressCallback([](float progress) {
            App::Instance().SetProgress(progress);
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
    std::thread([fileName, callback]() {
        SPDLOG_INFO("Starting XSF grid file processing (background): {}", fileName);
        const std::string filePath = "/" + fileName;

        atoms::infrastructure::FileIOManager loader(&AtomsTemplate::Instance());
        loader.SetProgressCallback([](float progress) {
            App::Instance().SetProgress(progress);
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
    std::thread([fileName, callback]() {
        SPDLOG_INFO("Starting CHGCAR file processing (background): {}", fileName);
        const std::string filePath = "/" + fileName;

        auto parseResult = atoms::infrastructure::ChgcarParser::parse(
            filePath,
            [](float progress) {
                App::Instance().SetProgress(progress);
            });

        auto* result = new ParserWorkerResult();
        result->kind = ParsedImportKind::Chgcar;
        result->filePath = filePath;
        result->chgcarResult = std::move(parseResult);
        DispatchToMainThread(callback, result);
    }).detach();
}

} // namespace io::application
