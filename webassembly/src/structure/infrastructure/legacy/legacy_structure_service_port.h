/**
 * @file legacy_structure_service_port.h
 * @brief Legacy structure adapter implementing StructureServicePort.
 * @note Temporary compatibility path (remove in Phase18-W6, Track: P18-W3-STRUCTURE-PORT).
 */
#pragma once

#include "../../application/structure_service_port.h"

namespace structure {
namespace infrastructure {
namespace legacy {

/**
 * @brief Adapts structure service port calls to legacy AtomsTemplate facade.
 */
class LegacyStructureServicePort final : public application::StructureServicePort {
public:
    int GetStructureCount() const override;
    int32_t GetCurrentStructureId() const override;
    void SetCurrentStructureId(int32_t structureId) override;

    std::vector<application::StructureEntryView> GetStructures() const override;
    bool IsStructureVisible(int32_t structureId) const override;
    void SetStructureVisible(int32_t structureId, bool visible) override;

    void RegisterStructure(int32_t structureId, const std::string& name) override;
    void RemoveStructure(int32_t structureId) override;
    void RemoveUnassignedData() override;
    size_t GetAtomCountForStructure(int32_t structureId) const override;

    bool IsBoundaryAtomsEnabled() const override;
    void SetBoundaryAtomsEnabled(bool enabled) override;
};

} // namespace legacy
} // namespace infrastructure
} // namespace structure
