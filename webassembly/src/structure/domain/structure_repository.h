/**
 * @file structure_repository.h
 * @brief Structure state repository facade for Phase 8 modularization.
 */
#pragma once

#include "../../structure/domain/atoms/structure_state_store.h"

namespace structure {
namespace domain {

/**
 * @brief Repository facade that centralizes access to structure state storage.
 * @details This class wraps the legacy `atoms::domain::StructureStateStore`
 *          so callers can migrate to the `structure` module without changing runtime behavior.
 */
class StructureRepository {
public:
    /**
     * @brief Returns singleton repository facade instance.
     */
    static StructureRepository& Instance();

    atoms::domain::StructureState& GetOrCreate(atoms::domain::StructureId structureId);
    const atoms::domain::StructureState* Find(atoms::domain::StructureId structureId) const;
    bool Remove(atoms::domain::StructureId structureId);
    void RemoveUnassigned();
    void Clear();

    const std::unordered_map<atoms::domain::StructureId, atoms::domain::StructureState>& All() const;

    std::vector<atoms::domain::AtomInfo>& CreatedAtoms();
    std::vector<atoms::domain::AtomInfo>& SurroundingAtoms();
    std::map<std::string, atoms::domain::AtomGroupInfo>& AtomGroups();

    std::vector<atoms::domain::BondInfo>& CreatedBonds();
    std::vector<atoms::domain::BondInfo>& SurroundingBonds();
    std::map<std::string, atoms::domain::BondGroupInfo>& BondGroups();

    CellInfo& Cell();
    bool& CellVisible();
    bool& SurroundingsVisible();
    std::vector<vtkSmartPointer<vtkActor>>& CellEdgeActors();

    uint32_t GenerateAtomId();
    uint32_t GenerateBondId();
    void ResetBondIdCounter(uint32_t startId = 1);

    atoms::domain::AtomInfo* FindAtomById(uint32_t atomId);
    atoms::domain::BondInfo* FindBondById(uint32_t bondId);
    void RebuildIndexes();

private:
    StructureRepository() = default;
};

/**
 * @brief Convenience accessor for structure repository facade.
 */
StructureRepository& GetStructureRepository();

} // namespace domain
} // namespace structure

