/**
 * @file structure_service_port.h
 * @brief Port interface for structure service compatibility backends.
 */
#pragma once

#include "structure_service_types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace structure {
namespace application {

/**
 * @brief Port abstraction used by StructureService.
 */
class StructureServicePort {
public:
    virtual ~StructureServicePort() = default;

    virtual int GetStructureCount() const = 0;
    virtual int32_t GetCurrentStructureId() const = 0;
    virtual void SetCurrentStructureId(int32_t structureId) = 0;

    virtual std::vector<StructureEntryView> GetStructures() const = 0;
    virtual bool IsStructureVisible(int32_t structureId) const = 0;
    virtual void SetStructureVisible(int32_t structureId, bool visible) = 0;

    virtual void RegisterStructure(int32_t structureId, const std::string& name) = 0;
    virtual void RemoveStructure(int32_t structureId) = 0;
    virtual void RemoveUnassignedData() = 0;
    virtual size_t GetAtomCountForStructure(int32_t structureId) const = 0;

    virtual bool IsBoundaryAtomsEnabled() const = 0;
    virtual void SetBoundaryAtomsEnabled(bool enabled) = 0;
};

} // namespace application
} // namespace structure
