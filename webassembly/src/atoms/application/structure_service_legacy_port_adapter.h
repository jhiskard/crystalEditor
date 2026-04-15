/**
 * @file structure_service_legacy_port_adapter.h
 * @brief Legacy adapter that bridges StructureServicePort to AtomsTemplate.
 */
#pragma once

#include "../../structure/application/structure_service_port.h"

class AtomsTemplate;

namespace atoms {
namespace application {

/**
 * @brief Structure service port adapter backed by AtomsTemplate.
 */
class StructureServiceLegacyPortAdapter final : public structure::application::StructureServicePort {
public:
    explicit StructureServiceLegacyPortAdapter(AtomsTemplate* atomsTemplate);

    int GetStructureCount() const override;
    int32_t GetCurrentStructureId() const override;
    void SetCurrentStructureId(int32_t structureId) override;

    std::vector<structure::application::StructureEntryView> GetStructures() const override;
    bool IsStructureVisible(int32_t structureId) const override;
    void SetStructureVisible(int32_t structureId, bool visible) override;

    void RegisterStructure(int32_t structureId, const std::string& name) override;
    void RemoveStructure(int32_t structureId) override;
    void RemoveUnassignedData() override;
    size_t GetAtomCountForStructure(int32_t structureId) const override;
    bool IsBoundaryAtomsEnabled() const override;
    void SetBoundaryAtomsEnabled(bool enabled) override;

private:
    AtomsTemplate* m_atomsTemplate = nullptr;
};

} // namespace application
} // namespace atoms
