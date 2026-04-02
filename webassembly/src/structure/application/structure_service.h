/**
 * @file structure_service.h
 * @brief Structure feature application service facade.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

class AtomsTemplate;

namespace structure {
namespace application {

/**
 * @brief Read model entry for structure list UI.
 */
struct StructureEntryView {
    int32_t id = -1;
    std::string name;
    bool visible = true;
};

/**
 * @brief Structure use-case facade extracted from AtomsTemplate.
 * @details The implementation delegates to `AtomsTemplate` during Phase 8 to
 *          preserve behavior while call-sites migrate to module-level APIs.
 */
class StructureService {
public:
    /**
     * @brief Creates service bound to the atoms compatibility facade.
     */
    explicit StructureService(AtomsTemplate* atomsTemplate);

    /**
     * @brief Returns number of registered structures.
     */
    int GetStructureCount() const;

    /**
     * @brief Returns active structure id.
     */
    int32_t GetCurrentStructureId() const;

    /**
     * @brief Sets active structure id.
     */
    void SetCurrentStructureId(int32_t structureId);

    /**
     * @brief Returns structure list read model.
     */
    std::vector<StructureEntryView> GetStructures() const;

    /**
     * @brief Queries structure visibility by id.
     */
    bool IsStructureVisible(int32_t structureId) const;

    /**
     * @brief Updates structure visibility by id.
     */
    void SetStructureVisible(int32_t structureId, bool visible);

    /**
     * @brief Registers a structure entry.
     */
    void RegisterStructure(int32_t structureId, const std::string& name);

    /**
     * @brief Removes a structure entry and related state.
     */
    void RemoveStructure(int32_t structureId);

    /**
     * @brief Removes legacy unassigned structure payloads.
     */
    void RemoveUnassignedData();

    /**
     * @brief Returns atom count for the target structure.
     */
    size_t GetAtomCountForStructure(int32_t structureId) const;

private:
    AtomsTemplate* m_atomsTemplate = nullptr;
};

} // namespace application
} // namespace structure

