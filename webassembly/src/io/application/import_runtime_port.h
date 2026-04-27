/**
 * @file import_runtime_port.h
 * @brief IO import compatibility port for legacy atoms runtime access.
 * @note Temporary compatibility path (remove in Phase18-W6 closeout, Track: P18-W4-IO-PORT).
 */
#pragma once

#include <cstdint>
#include <string>

namespace io::application {

/**
 * @brief Port contract for legacy runtime calls used by IO import application services.
 */
class ImportRuntimePort {
public:
    virtual ~ImportRuntimePort() = default;

    virtual bool HasStructures() const = 0;
    virtual bool HasUnitCell() const = 0;
    virtual bool HasChargeDensity() const = 0;

    virtual int32_t GetCurrentStructureId() const = 0;
    virtual int32_t GetChargeDensityStructureId() const = 0;
    virtual std::string GetLoadedFileName() const = 0;

    virtual void SetCurrentStructureId(int32_t structureId) = 0;
    virtual void SetChargeDensityStructureId(int32_t structureId) = 0;
    virtual void SetLoadedFileName(const std::string& fileName) = 0;
    virtual void RegisterStructure(int32_t structureId, const std::string& name) = 0;
    virtual void RemoveStructure(int32_t structureId) = 0;
    virtual void RemoveUnassignedData() = 0;
};

/**
 * @brief Returns default import runtime port bound to legacy runtime adapter.
 */
ImportRuntimePort& GetImportRuntimePort();

} // namespace io::application


