#pragma once

#include "atom_manager.h"
#include "bond_manager.h"
#include "cell_manager.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace atoms {
namespace domain {

using StructureId = int32_t;
constexpr StructureId kUnassignedStructureId = -1;

struct StructureState {
    std::vector<AtomInfo> createdAtoms;
    std::vector<AtomInfo> surroundingAtoms;
    std::map<std::string, AtomGroupInfo> atomGroups;

    std::vector<BondInfo> createdBonds;
    std::vector<BondInfo> surroundingBonds;
    std::map<std::string, BondGroupInfo> bondGroups;

    CellInfo cellInfo {};
    bool cellVisible = false;
    bool surroundingsVisible = false;
};

struct AtomLocation {
    StructureId structureId = kUnassignedStructureId;
    bool surrounding = false;
    size_t index = 0;
};

struct BondLocation {
    StructureId structureId = kUnassignedStructureId;
    bool surrounding = false;
    size_t index = 0;
};

class StructureStateStore {
public:
    static StructureStateStore& Instance();

    StructureState& GetOrCreate(StructureId structureId);
    const StructureState* Find(StructureId structureId) const;
    bool Remove(StructureId structureId);
    void RemoveUnassigned();
    void Clear();

    const std::unordered_map<StructureId, StructureState>& All() const { return m_states; }

    // Legacy compatibility accessors (Phase 2 migration bridge)
    std::vector<AtomInfo>& CreatedAtoms();
    std::vector<AtomInfo>& SurroundingAtoms();
    std::map<std::string, AtomGroupInfo>& AtomGroups();

    std::vector<BondInfo>& CreatedBonds();
    std::vector<BondInfo>& SurroundingBonds();
    std::map<std::string, BondGroupInfo>& BondGroups();

    CellInfo& Cell();
    bool& CellVisible();
    bool& SurroundingsVisible();
    std::vector<vtkSmartPointer<vtkActor>>& CellEdgeActors();

    uint32_t GenerateAtomId();
    uint32_t GenerateBondId();
    void ResetBondIdCounter(uint32_t startId = 1);

    AtomInfo* FindAtomById(uint32_t atomId);
    BondInfo* FindBondById(uint32_t bondId);
    void RebuildIndexes();

private:
    StructureStateStore();

    StructureState& LegacyState();
    const StructureState& LegacyState() const;

private:
    std::unordered_map<StructureId, StructureState> m_states;
    uint32_t m_nextAtomId = 1;
    uint32_t m_nextBondId = 1;
    std::unordered_map<uint32_t, AtomLocation> m_atomIndex;
    std::unordered_map<uint32_t, BondLocation> m_bondIndex;
    std::vector<vtkSmartPointer<vtkActor>> m_cellEdgeActors;
};

} // namespace domain
} // namespace atoms
