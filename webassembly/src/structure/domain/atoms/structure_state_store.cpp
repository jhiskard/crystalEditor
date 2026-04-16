#include "structure_state_store.h"

namespace atoms {
namespace domain {

StructureStateStore& StructureStateStore::Instance() {
    static StructureStateStore store;
    return store;
}

StructureStateStore::StructureStateStore() {
    m_states.emplace(kUnassignedStructureId, StructureState {});
}

StructureState& StructureStateStore::LegacyState() {
    return GetOrCreate(kUnassignedStructureId);
}

const StructureState& StructureStateStore::LegacyState() const {
    auto it = m_states.find(kUnassignedStructureId);
    if (it == m_states.end()) {
        // const context fallback: constructor always initializes this key.
        // This line is defensive and should never be hit.
        static const StructureState emptyState {};
        return emptyState;
    }
    return it->second;
}

StructureState& StructureStateStore::GetOrCreate(StructureId structureId) {
    return m_states[structureId];
}

const StructureState* StructureStateStore::Find(StructureId structureId) const {
    auto it = m_states.find(structureId);
    return (it == m_states.end()) ? nullptr : &it->second;
}

bool StructureStateStore::Remove(StructureId structureId) {
    if (structureId == kUnassignedStructureId) {
        RemoveUnassigned();
        return true;
    }

    auto it = m_states.find(structureId);
    if (it == m_states.end()) {
        return false;
    }

    m_states.erase(it);
    RebuildIndexes();
    return true;
}

void StructureStateStore::RemoveUnassigned() {
    m_states[kUnassignedStructureId] = StructureState {};
    m_cellEdgeActors.clear();
    RebuildIndexes();
}

void StructureStateStore::Clear() {
    m_states.clear();
    m_states.emplace(kUnassignedStructureId, StructureState {});
    m_nextAtomId = 1;
    m_nextBondId = 1;
    m_cellEdgeActors.clear();
    RebuildIndexes();
}

std::vector<AtomInfo>& StructureStateStore::CreatedAtoms() {
    return LegacyState().createdAtoms;
}

std::vector<AtomInfo>& StructureStateStore::SurroundingAtoms() {
    return LegacyState().surroundingAtoms;
}

std::map<std::string, AtomGroupInfo>& StructureStateStore::AtomGroups() {
    return LegacyState().atomGroups;
}

std::vector<BondInfo>& StructureStateStore::CreatedBonds() {
    return LegacyState().createdBonds;
}

std::vector<BondInfo>& StructureStateStore::SurroundingBonds() {
    return LegacyState().surroundingBonds;
}

std::map<std::string, BondGroupInfo>& StructureStateStore::BondGroups() {
    return LegacyState().bondGroups;
}

CellInfo& StructureStateStore::Cell() {
    return LegacyState().cellInfo;
}

bool& StructureStateStore::CellVisible() {
    return LegacyState().cellVisible;
}

bool& StructureStateStore::SurroundingsVisible() {
    return LegacyState().surroundingsVisible;
}

std::vector<vtkSmartPointer<vtkActor>>& StructureStateStore::CellEdgeActors() {
    return m_cellEdgeActors;
}

uint32_t StructureStateStore::GenerateAtomId() {
    return m_nextAtomId++;
}

uint32_t StructureStateStore::GenerateBondId() {
    return m_nextBondId++;
}

void StructureStateStore::ResetBondIdCounter(uint32_t startId) {
    m_nextBondId = startId;
}

AtomInfo* StructureStateStore::FindAtomById(uint32_t atomId) {
    auto it = m_atomIndex.find(atomId);
    if (it == m_atomIndex.end()) {
        return nullptr;
    }

    const AtomLocation& loc = it->second;
    auto stateIt = m_states.find(loc.structureId);
    if (stateIt == m_states.end()) {
        return nullptr;
    }

    auto& vec = loc.surrounding ? stateIt->second.surroundingAtoms : stateIt->second.createdAtoms;
    if (loc.index >= vec.size()) {
        return nullptr;
    }
    return &vec[loc.index];
}

BondInfo* StructureStateStore::FindBondById(uint32_t bondId) {
    auto it = m_bondIndex.find(bondId);
    if (it == m_bondIndex.end()) {
        return nullptr;
    }

    const BondLocation& loc = it->second;
    auto stateIt = m_states.find(loc.structureId);
    if (stateIt == m_states.end()) {
        return nullptr;
    }

    auto& vec = loc.surrounding ? stateIt->second.surroundingBonds : stateIt->second.createdBonds;
    if (loc.index >= vec.size()) {
        return nullptr;
    }
    return &vec[loc.index];
}

void StructureStateStore::RebuildIndexes() {
    m_atomIndex.clear();
    m_bondIndex.clear();

    for (auto& [structureId, state] : m_states) {
        for (size_t i = 0; i < state.createdAtoms.size(); ++i) {
            const AtomInfo& atom = state.createdAtoms[i];
            if (atom.id != 0) {
                m_atomIndex[atom.id] = AtomLocation { structureId, false, i };
            }
        }

        for (size_t i = 0; i < state.surroundingAtoms.size(); ++i) {
            const AtomInfo& atom = state.surroundingAtoms[i];
            if (atom.id != 0) {
                m_atomIndex[atom.id] = AtomLocation { structureId, true, i };
            }
        }

        for (size_t i = 0; i < state.createdBonds.size(); ++i) {
            const BondInfo& bond = state.createdBonds[i];
            if (bond.id != 0) {
                m_bondIndex[bond.id] = BondLocation { structureId, false, i };
            }
        }

        for (size_t i = 0; i < state.surroundingBonds.size(); ++i) {
            const BondInfo& bond = state.surroundingBonds[i];
            if (bond.id != 0) {
                m_bondIndex[bond.id] = BondLocation { structureId, true, i };
            }
        }
    }
}

} // namespace domain
} // namespace atoms
