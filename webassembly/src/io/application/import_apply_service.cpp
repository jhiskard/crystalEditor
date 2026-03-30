#include "import_apply_service.h"

#include "../../atoms/atoms_template.h"
#include "../../atoms/ui/charge_density_ui.h"
#include "../../config/log_config.h"
#include "../../mesh_manager.h"

// Standard library
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

// VTK
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkResampleToImage.h>
#include <vtkStructuredGrid.h>

namespace io::application {

namespace {

std::string ExtractFileName(const std::string& filePath, const char* fallback) {
    std::string fileNameOnly = filePath;
    size_t lastSlash = fileNameOnly.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        fileNameOnly = fileNameOnly.substr(lastSlash + 1);
    }
    if (fileNameOnly.empty()) {
        fileNameOnly = fallback;
    }
    return fileNameOnly;
}

bool IsAxisAlignedGrid(const float vectors[3][3]) {
    const float eps = 1e-6f;
    return std::fabs(vectors[0][1]) < eps &&
           std::fabs(vectors[0][2]) < eps &&
           std::fabs(vectors[1][0]) < eps &&
           std::fabs(vectors[1][2]) < eps &&
           std::fabs(vectors[2][0]) < eps &&
           std::fabs(vectors[2][1]) < eps;
}

vtkSmartPointer<vtkStructuredGrid> CreateStructuredGrid(
    const atoms::infrastructure::FileIOManager::Grid3DResult& grid) {
    vtkSmartPointer<vtkStructuredGrid> structuredGrid = vtkSmartPointer<vtkStructuredGrid>::New();
    structuredGrid->SetDimensions(grid.dims[0], grid.dims[1], grid.dims[2]);

    const size_t total = grid.values.size();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(static_cast<vtkIdType>(total));

    const float denomX = (grid.dims[0] > 1) ? static_cast<float>(grid.dims[0] - 1) : 1.0f;
    const float denomY = (grid.dims[1] > 1) ? static_cast<float>(grid.dims[1] - 1) : 1.0f;
    const float denomZ = (grid.dims[2] > 1) ? static_cast<float>(grid.dims[2] - 1) : 1.0f;

    size_t idx = 0;
    for (int k = 0; k < grid.dims[2]; ++k) {
        const float tz = static_cast<float>(k) / denomZ;
        for (int j = 0; j < grid.dims[1]; ++j) {
            const float ty = static_cast<float>(j) / denomY;
            for (int i = 0; i < grid.dims[0]; ++i) {
                const float tx = static_cast<float>(i) / denomX;
                const float x = grid.origin[0]
                    + tx * grid.vectors[0][0]
                    + ty * grid.vectors[1][0]
                    + tz * grid.vectors[2][0];
                const float y = grid.origin[1]
                    + tx * grid.vectors[0][1]
                    + ty * grid.vectors[1][1]
                    + tz * grid.vectors[2][1];
                const float z = grid.origin[2]
                    + tx * grid.vectors[0][2]
                    + ty * grid.vectors[1][2]
                    + tz * grid.vectors[2][2];
                points->SetPoint(static_cast<vtkIdType>(idx), x, y, z);
                ++idx;
            }
        }
    }

    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetName(grid.label.c_str());
    scalars->SetNumberOfComponents(1);
    scalars->SetNumberOfTuples(static_cast<vtkIdType>(total));
    for (size_t i = 0; i < total; ++i) {
        scalars->SetValue(static_cast<vtkIdType>(i), grid.values[i]);
    }

    structuredGrid->SetPoints(points);
    structuredGrid->GetPointData()->SetScalars(scalars);
    return structuredGrid;
}

vtkSmartPointer<vtkImageData> CreateImageData(
    const atoms::infrastructure::FileIOManager::Grid3DResult& grid) {
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetDimensions(grid.dims[0], grid.dims[1], grid.dims[2]);
    image->SetOrigin(grid.origin[0], grid.origin[1], grid.origin[2]);

    double spacing[3] = { 1.0, 1.0, 1.0 };
    if (grid.dims[0] > 1) {
        spacing[0] = std::fabs(grid.vectors[0][0]) / static_cast<double>(grid.dims[0] - 1);
    }
    if (grid.dims[1] > 1) {
        spacing[1] = std::fabs(grid.vectors[1][1]) / static_cast<double>(grid.dims[1] - 1);
    }
    if (grid.dims[2] > 1) {
        spacing[2] = std::fabs(grid.vectors[2][2]) / static_cast<double>(grid.dims[2] - 1);
    }
    image->SetSpacing(spacing);

    const size_t total = grid.values.size();
    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetName(grid.label.c_str());
    scalars->SetNumberOfComponents(1);
    scalars->SetNumberOfTuples(static_cast<vtkIdType>(total));
    for (size_t i = 0; i < total; ++i) {
        scalars->SetValue(static_cast<vtkIdType>(i), grid.values[i]);
    }
    image->GetPointData()->SetScalars(scalars);
    return image;
}

vtkSmartPointer<vtkImageData> ResampleStructuredGridToImage(
    vtkSmartPointer<vtkStructuredGrid> grid, const int dims[3]) {
    if (!grid) {
        return nullptr;
    }
    vtkNew<vtkResampleToImage> resample;
    resample->SetInputDataObject(grid);
    resample->SetSamplingDimensions(dims[0], dims[1], dims[2]);
    double bounds[6];
    grid->GetBounds(bounds);
    resample->SetSamplingBounds(bounds);
    resample->Update();
    return resample->GetOutput();
}

} // namespace

ImportApplyResult ImportApplyService::Apply(ParserWorkerResult& parsedResult) const {
    switch (parsedResult.kind) {
        case ParsedImportKind::XsfStructure:
            return ApplyXsf(parsedResult);
        case ParsedImportKind::XsfGrid:
            return ApplyXsfGrid(parsedResult);
        case ParsedImportKind::Chgcar:
            return ApplyChgcar(parsedResult);
    }

    ImportApplyResult result;
    result.errorMessage = "Unknown parsed import kind.";
    return result;
}

ImportApplyResult ImportApplyService::ApplyXsf(ParserWorkerResult& parsedResult) const {
    ImportApplyResult result;
    auto& parseResult = parsedResult.xsfResult;
    if (!parseResult.success) {
        result.errorMessage = parseResult.errorMessage;
        return result;
    }

    const std::string fileNameOnly = ExtractFileName(parsedResult.filePath, "XSF");

    int32_t structureId = -1;
    bool applySuccess = false;
    std::string failureReason;
    try {
        structureId = MeshManager::Instance().RegisterXsfStructure(fileNameOnly);
        if (structureId < 0) {
            throw std::runtime_error("Failed to register XSF structure");
        }
        AtomsTemplate::Instance().SetCurrentStructureId(structureId);
        AtomsTemplate::Instance().RegisterStructure(structureId, fileNameOnly);
        AtomsTemplate::Instance().SetLoadedFileName(fileNameOnly);
        applySuccess = AtomsTemplate::Instance().LoadXSFParsedData(parseResult);
    }
    catch (const std::exception& e) {
        SPDLOG_ERROR("Error applying XSF data: {}", e.what());
        failureReason = e.what();
        applySuccess = false;
    }

    result.importedStructureId = structureId;
    result.success = applySuccess;
    if (!applySuccess) {
        if (failureReason.empty()) {
            failureReason = "Failed to apply parsed XSF data.";
        }
        result.errorMessage = failureReason;
    }
    return result;
}

ImportApplyResult ImportApplyService::ApplyXsfGrid(ParserWorkerResult& parsedResult) const {
    ImportApplyResult result;
    auto& parseResult = parsedResult.xsfGridResult;
    if (!parseResult.success) {
        result.errorMessage = parseResult.errorMessage;
        return result;
    }

    const std::string fileNameOnly = ExtractFileName(parsedResult.filePath, "XSF Grid");
    int32_t structureId = -1;
    bool applySuccess = true;
    std::string failureReason;

    try {
        structureId = MeshManager::Instance().RegisterXsfStructure(fileNameOnly);
        if (structureId < 0) {
            throw std::runtime_error("Failed to register XSF grid structure");
        }

        AtomsTemplate::Instance().SetCurrentStructureId(structureId);
        AtomsTemplate::Instance().RegisterStructure(structureId, fileNameOnly);
        AtomsTemplate::Instance().SetLoadedFileName(fileNameOnly);

        MeshManager& meshManager = MeshManager::Instance();
        std::vector<atoms::ui::ChargeDensityUI::GridDataEntry> gridEntries;
        int gridIndex = 1;
        for (auto& gridSet : parseResult.grids) {
            auto& gridHigh = gridSet.high;
            auto& gridMedium = gridSet.medium;
            auto& gridLow = gridSet.low;
            atoms::infrastructure::FileIOManager::Grid3DResult* simpleSource = &gridHigh;
            if (simpleSource->values.empty() && gridSet.hasMedium) {
                simpleSource = &gridMedium;
            }
            if (simpleSource->values.empty() && gridSet.hasLow) {
                simpleSource = &gridLow;
            }
            if (gridHigh.label.empty()) {
                std::string label = "noname_";
                if (gridIndex < 10) {
                    label += "0";
                }
                label += std::to_string(gridIndex);
                gridHigh.label = label;
                gridMedium.label = label;
                gridLow.label = label;
            }

            vtkSmartPointer<vtkDataSet> highDataSet;
            vtkSmartPointer<vtkDataSet> mediumDataSet;
            vtkSmartPointer<vtkDataSet> lowDataSet;
            vtkSmartPointer<vtkDataSet> highVolumeDataSet;
            vtkSmartPointer<vtkDataSet> mediumVolumeDataSet;
            vtkSmartPointer<vtkDataSet> lowVolumeDataSet;
            if (IsAxisAlignedGrid(gridHigh.vectors)) {
                highDataSet = CreateImageData(gridHigh);
                if (gridSet.hasMedium && !gridMedium.values.empty()) {
                    mediumDataSet = CreateImageData(gridMedium);
                }
                if (gridSet.hasLow && !gridLow.values.empty()) {
                    lowDataSet = CreateImageData(gridLow);
                }
                highVolumeDataSet = highDataSet;
                mediumVolumeDataSet = mediumDataSet;
                lowVolumeDataSet = lowDataSet;
            } else {
                vtkSmartPointer<vtkStructuredGrid> highStructured = CreateStructuredGrid(gridHigh);
                highDataSet = highStructured;
                if (gridSet.hasMedium && !gridMedium.values.empty()) {
                    vtkSmartPointer<vtkStructuredGrid> mediumStructured = CreateStructuredGrid(gridMedium);
                    mediumDataSet = mediumStructured;
                    mediumVolumeDataSet = ResampleStructuredGridToImage(mediumStructured, gridMedium.dims);
                }
                if (gridSet.hasLow && !gridLow.values.empty()) {
                    vtkSmartPointer<vtkStructuredGrid> lowStructured = CreateStructuredGrid(gridLow);
                    lowDataSet = lowStructured;
                    lowVolumeDataSet = ResampleStructuredGridToImage(lowStructured, gridLow.dims);
                }
                highVolumeDataSet = ResampleStructuredGridToImage(highStructured, gridHigh.dims);
            }

            Mesh* mesh = meshManager.InsertMesh(gridHigh.label.c_str(), nullptr, nullptr, highDataSet, structureId);
            if (mesh == nullptr) {
                failureReason = "Failed to create mesh from DATAGRID_3D data.";
                applySuccess = false;
                break;
            }
            mesh->SetVolumeQualityDataSets(highDataSet, mediumDataSet, lowDataSet,
                highVolumeDataSet, mediumVolumeDataSet, lowVolumeDataSet);
            mesh->SetVolumeMeshVisibility(true);

            if (simpleSource && !simpleSource->values.empty()) {
                atoms::ui::ChargeDensityUI::GridDataEntry entry;
                entry.name = simpleSource->label;
                entry.dims = { simpleSource->dims[0], simpleSource->dims[1], simpleSource->dims[2] };
                entry.origin[0] = simpleSource->origin[0];
                entry.origin[1] = simpleSource->origin[1];
                entry.origin[2] = simpleSource->origin[2];
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        entry.vectors[i][j] = simpleSource->vectors[i][j];
                    }
                }
                entry.values = std::move(simpleSource->values);
                gridEntries.push_back(std::move(entry));
            }
            ++gridIndex;
        }

        if (!gridEntries.empty()) {
            if (auto* cdUI = AtomsTemplate::Instance().chargeDensityUI()) {
                cdUI->setGridDataEntries(std::move(gridEntries));
            }

            AtomsTemplate::Instance().SetAllChargeDensityAdvancedGridVisible(
                structureId, false, false);
            AtomsTemplate::Instance().SetAllChargeDensityAdvancedGridVisible(
                structureId, true, false);
        }

        if (applySuccess && !parseResult.atoms.empty()) {
            const bool renderCell = parseResult.hasCellVectors && parseResult.cellVectorsConsistent;
            applySuccess = ApplyXsfGridAtomsPayload(parseResult, structureId, fileNameOnly, renderCell);
            if (!applySuccess && failureReason.empty()) {
                failureReason = "Failed to apply atomic data from XSF grid.";
            }
            if (applySuccess && parseResult.hasCellVectors && !parseResult.cellVectorsConsistent) {
                result.showXsfGridCellWarning = true;
                result.xsfGridCellWarningText =
                    "Cell vectors differ across DATAGRID_3D blocks. Cell rendering was skipped.";
            }
        }
    }
    catch (const std::exception& e) {
        SPDLOG_ERROR("Error applying XSF grid data: {}", e.what());
        failureReason = e.what();
        applySuccess = false;
    }

    result.importedStructureId = structureId;
    result.success = applySuccess;
    if (!applySuccess) {
        if (failureReason.empty()) {
            failureReason = "Failed to apply parsed XSF grid data.";
        }
        result.errorMessage = failureReason;
    }
    return result;
}

ImportApplyResult ImportApplyService::ApplyChgcar(ParserWorkerResult& parsedResult) const {
    ImportApplyResult result;
    auto& parseResult = parsedResult.chgcarResult;
    if (!parseResult.success) {
        result.errorMessage = parseResult.errorMessage;
        return result;
    }

    const std::string fileNameOnly = ExtractFileName(parsedResult.filePath, "CHGCAR");

    int32_t structureId = -1;
    bool applySuccess = false;
    std::string failureReason;
    try {
        structureId = MeshManager::Instance().RegisterXsfStructure(fileNameOnly);
        if (structureId < 0) {
            throw std::runtime_error("Failed to register CHGCAR structure");
        }
        AtomsTemplate::Instance().SetCurrentStructureId(structureId);
        AtomsTemplate::Instance().RegisterStructure(structureId, fileNameOnly);

        applySuccess = AtomsTemplate::Instance().LoadChgcarParsedData(parseResult);
        if (applySuccess) {
            AtomsTemplate::Instance().SetLoadedFileName(fileNameOnly);
            SPDLOG_INFO("CHGCAR file loaded successfully: {}", fileNameOnly);
        } else {
            failureReason = "Failed to apply parsed CHGCAR data.";
        }
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Error applying CHGCAR data: {}", e.what());
        failureReason = e.what();
        applySuccess = false;
    }

    result.importedStructureId = structureId;
    result.success = applySuccess;
    if (!applySuccess) {
        if (failureReason.empty()) {
            failureReason = "Failed to import CHGCAR data.";
        }
        result.errorMessage = failureReason;
    }
    return result;
}

bool ImportApplyService::ApplyXsfGridAtomsPayload(
    atoms::infrastructure::FileIOManager::Grid3DParseResult& parsedResult,
    int32_t structureId,
    const std::string& structureName,
    bool renderCell) const {
    if (parsedResult.atoms.empty()) {
        return true;
    }

    if (structureId < 0) {
        structureId = MeshManager::Instance().RegisterXsfStructure(structureName);
    }
    if (structureId < 0) {
        SPDLOG_ERROR("Failed to register XSF grid structure for atoms payload");
        return false;
    }

    AtomsTemplate::Instance().SetCurrentStructureId(structureId);
    AtomsTemplate::Instance().RegisterStructure(structureId, structureName);

    atoms::infrastructure::FileIOManager::ParseResult parse;
    parse.success = true;
    if (parsedResult.hasCellVectors) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                parse.cellVectors[i][j] = parsedResult.cellVectors[i][j];
            }
        }
    }
    parse.atoms = std::move(parsedResult.atoms);

    try {
        return AtomsTemplate::Instance().LoadXSFParsedData(parse, renderCell);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Error applying XSF grid atoms: {}", e.what());
        return false;
    }
}

} // namespace io::application
