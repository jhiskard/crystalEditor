/**
 * @file structure_service.h
 * @brief Structure feature application service facade.
 */
#pragma once

#include "structure_service_types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace structure {
namespace application {

/**
 * @brief Structure use-case facade extracted from WorkspaceRuntimeModel.
 * @details 서비스 레이어는 포트 계약만 의존하며, legacy 호환 경로는
 *          infrastructure adapter에서만 관리한다.
 */
class StructureService {
public:
    /**
     * @brief Creates structure service facade.
     */
    StructureService();

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

    /**
     * @brief Returns current boundary-atom visibility.
     */
    bool IsBoundaryAtomsEnabled() const;

    /**
     * @brief Updates boundary-atom visibility.
     */
    void SetBoundaryAtomsEnabled(bool enabled);

};

} // namespace application
} // namespace structure



