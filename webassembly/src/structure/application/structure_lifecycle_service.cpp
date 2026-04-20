#include "../../workspace/legacy/legacy_atoms_runtime.h"
#include "../domain/structure_repository.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

void AtomsTemplate::RegisterStructure(int32_t id, const std::string& name) {
    if (id < 0) {
        return;
    }
    StructureEntry entry;
    entry.id = id;
    entry.name = name;
    entry.visible = true;
    m_Structures[id] = entry;
    m_StructureBondsVisible[id] = true;
    m_CurrentStructureId = id;
}


void AtomsTemplate::RemoveStructure(int32_t id) {
    auto& repository = structure::domain::GetStructureRepository();
    auto& createdAtoms = repository.CreatedAtoms();
    auto& surroundingAtoms = repository.SurroundingAtoms();
    auto& atomGroups = repository.AtomGroups();
    auto& createdBonds = repository.CreatedBonds();
    auto& surroundingBonds = repository.SurroundingBonds();
    auto& bondGroups = repository.BondGroups();
    auto& cell = repository.Cell();

    auto it = m_Structures.find(id);
    if (it == m_Structures.end()) {
        return;
    }
    removeDihedralMeasurementsByStructure(id);
    removeAngleMeasurementsByStructure(id);
    removeDistanceMeasurementsByStructure(id);
    removeCenterMeasurementsByStructure(id);
    m_Structures.erase(it);
    m_StructureBondsVisible.erase(id);
    auto cellIt = m_UnitCells.find(id);
    if (cellIt != m_UnitCells.end()) {
        if (m_vtkRenderer) {
            m_vtkRenderer->clearUnitCell(id);
        }
        m_UnitCells.erase(cellIt);
    }
    if (m_CurrentStructureId == id) {
        m_CurrentStructureId = -1;
        atoms::domain::setCellVisible(false);
        cell.hasCell = false;
    }

    if (m_ChargeDensityStructureId == id) {
        ClearChargeDensity();
    }

    std::vector<atoms::domain::AtomInfo> oldCreated = createdAtoms;
    std::vector<atoms::domain::AtomInfo> oldSurrounding = surroundingAtoms;

    std::unordered_set<uint32_t> removedAtomIds;
    for (const auto& atom : oldCreated) {
        if (atom.structureId == id) {
            removedAtomIds.insert(atom.id);
        }
    }
    for (const auto& atom : oldSurrounding) {
        if (atom.structureId == id) {
            removedAtomIds.insert(atom.id);
        }
    }
    for (uint32_t atomId : removedAtomIds) {
        m_AtomVisibilityById.erase(atomId);
        m_AtomLabelVisibilityById.erase(atomId);
    }

    createdAtoms.erase(std::remove_if(createdAtoms.begin(), createdAtoms.end(), [id](const atoms::domain::AtomInfo& atom) {
        return atom.structureId == id;
    }), createdAtoms.end());
    surroundingAtoms.erase(std::remove_if(surroundingAtoms.begin(), surroundingAtoms.end(), [id](const atoms::domain::AtomInfo& atom) {
        return atom.structureId == id;
    }), surroundingAtoms.end());

    std::unordered_map<uint32_t, int> newCreatedIndex;
    std::unordered_map<uint32_t, int> newSurroundingIndex;
    for (size_t i = 0; i < createdAtoms.size(); ++i) {
        newCreatedIndex[createdAtoms[i].id] = static_cast<int>(i);
    }
    for (size_t i = 0; i < surroundingAtoms.size(); ++i) {
        newSurroundingIndex[surroundingAtoms[i].id] = static_cast<int>(i);
    }

    for (auto itGroup = atomGroups.begin(); itGroup != atomGroups.end(); ) {
        auto& group = itGroup->second;
        std::vector<vtkSmartPointer<vtkTransform>> transforms;
        std::vector<atoms::domain::Color4f> colors;
        std::vector<uint32_t> atomIds;
        std::vector<atoms::domain::AtomType> atomTypes;
        std::vector<size_t> originalIndices;

        for (size_t i = 0; i < group.atomIds.size(); ++i) {
            uint32_t atomId = group.atomIds[i];
            if (removedAtomIds.find(atomId) != removedAtomIds.end()) {
                continue;
            }
            transforms.push_back(group.transforms[i]);
            colors.push_back(group.colors[i]);
            atomIds.push_back(atomId);
            atomTypes.push_back(group.atomTypes[i]);
            auto itCreated = newCreatedIndex.find(atomId);
            if (itCreated != newCreatedIndex.end()) {
                originalIndices.push_back(static_cast<size_t>(itCreated->second));
            } else {
                auto itSur = newSurroundingIndex.find(atomId);
                if (itSur != newSurroundingIndex.end()) {
                    originalIndices.push_back(static_cast<size_t>(itSur->second));
                } else {
                    originalIndices.push_back(SIZE_MAX);
                }
            }
        }

        group.transforms = std::move(transforms);
        group.colors = std::move(colors);
        group.atomIds = std::move(atomIds);
        group.atomTypes = std::move(atomTypes);
        group.originalIndices = std::move(originalIndices);

        if (group.atomIds.empty()) {
            if (m_vtkRenderer) {
                m_vtkRenderer->clearAtomGroupVTK(itGroup->first);
            }
            itGroup = atomGroups.erase(itGroup);
        } else {
            ++itGroup;
        }
    }

    auto resolveAtomId = [&](int index, atoms::domain::BondType type) -> uint32_t {
        if (index >= 0) {
            if (type == atoms::domain::BondType::SURROUNDING) {
                if (index < static_cast<int>(oldSurrounding.size())) return oldSurrounding[index].id;
                if (index < static_cast<int>(oldCreated.size())) return oldCreated[index].id;
            } else {
                if (index < static_cast<int>(oldCreated.size())) return oldCreated[index].id;
                if (index < static_cast<int>(oldSurrounding.size())) return oldSurrounding[index].id;
            }
        } else {
            int s = -index - 1;
            if (s >= 0 && s < static_cast<int>(oldSurrounding.size())) return oldSurrounding[s].id;
        }
        return 0;
    };

    std::unordered_set<uint32_t> removedBondIds;
    for (const auto& bond : createdBonds) {
        if (bond.structureId == id) {
            removedBondIds.insert(bond.id);
        }
    }
    for (const auto& bond : surroundingBonds) {
        if (bond.structureId == id) {
            removedBondIds.insert(bond.id);
        }
    }
    for (uint32_t bondId : removedBondIds) {
        m_BondVisibilityById.erase(bondId);
        m_BondLabelVisibilityById.erase(bondId);
    }

    createdBonds.erase(std::remove_if(createdBonds.begin(), createdBonds.end(), [id](const atoms::domain::BondInfo& bond) {
        return bond.structureId == id;
    }), createdBonds.end());
    surroundingBonds.erase(std::remove_if(surroundingBonds.begin(), surroundingBonds.end(), [id](const atoms::domain::BondInfo& bond) {
        return bond.structureId == id;
    }), surroundingBonds.end());

    auto updateBondIndex = [&](atoms::domain::BondInfo& bond) {
        uint32_t atom1Id = bond.atom1Id != 0 ? bond.atom1Id : resolveAtomId(bond.atom1Index, bond.bondType);
        uint32_t atom2Id = bond.atom2Id != 0 ? bond.atom2Id : resolveAtomId(bond.atom2Index, bond.bondType);
        int newIdx1 = -1;
        int newIdx2 = -1;
        if (bond.bondType == atoms::domain::BondType::SURROUNDING) {
            auto it1 = newSurroundingIndex.find(atom1Id);
            auto it2 = newSurroundingIndex.find(atom2Id);
            if (it1 != newSurroundingIndex.end()) newIdx1 = it1->second;
            if (it2 != newSurroundingIndex.end()) newIdx2 = it2->second;
            if (newIdx1 == -1) {
                auto it1c = newCreatedIndex.find(atom1Id);
                if (it1c != newCreatedIndex.end()) newIdx1 = it1c->second;
            }
            if (newIdx2 == -1) {
                auto it2c = newCreatedIndex.find(atom2Id);
                if (it2c != newCreatedIndex.end()) newIdx2 = it2c->second;
            }
        } else {
            auto it1 = newCreatedIndex.find(atom1Id);
            auto it2 = newCreatedIndex.find(atom2Id);
            if (it1 != newCreatedIndex.end()) newIdx1 = it1->second;
            if (it2 != newCreatedIndex.end()) newIdx2 = it2->second;
            if (newIdx1 == -1) {
                auto it1s = newSurroundingIndex.find(atom1Id);
                if (it1s != newSurroundingIndex.end()) newIdx1 = -(it1s->second + 1);
            }
            if (newIdx2 == -1) {
                auto it2s = newSurroundingIndex.find(atom2Id);
                if (it2s != newSurroundingIndex.end()) newIdx2 = -(it2s->second + 1);
            }
        }
        bond.atom1Id = atom1Id;
        bond.atom2Id = atom2Id;
        bond.atom1Index = newIdx1;
        bond.atom2Index = newIdx2;
    };

    for (auto& bond : createdBonds) {
        updateBondIndex(bond);
    }
    for (auto& bond : surroundingBonds) {
        updateBondIndex(bond);
    }

    for (auto& [key, group] : bondGroups) {
        std::vector<vtkSmartPointer<vtkTransform>> transforms1;
        std::vector<vtkSmartPointer<vtkTransform>> transforms2;
        std::vector<atoms::domain::Color4f> colors1;
        std::vector<atoms::domain::Color4f> colors2;
        std::vector<uint32_t> bondIds;
        for (size_t i = 0; i < group.bondIds.size(); ++i) {
            uint32_t bondId = group.bondIds[i];
            if (removedBondIds.find(bondId) != removedBondIds.end()) {
                continue;
            }
            transforms1.push_back(group.transforms1[i]);
            transforms2.push_back(group.transforms2[i]);
            colors1.push_back(group.colors1[i]);
            colors2.push_back(group.colors2[i]);
            bondIds.push_back(bondId);
        }
        group.transforms1 = std::move(transforms1);
        group.transforms2 = std::move(transforms2);
        group.colors1 = std::move(colors1);
        group.colors2 = std::move(colors2);
        group.bondIds = std::move(bondIds);
    }

    refreshRenderedGroups();
}


void AtomsTemplate::RemoveUnassignedData() {
    auto& repository = structure::domain::GetStructureRepository();
    auto& createdAtoms = repository.CreatedAtoms();
    auto& surroundingAtoms = repository.SurroundingAtoms();
    auto& atomGroups = repository.AtomGroups();
    auto& createdBonds = repository.CreatedBonds();
    auto& surroundingBonds = repository.SurroundingBonds();
    auto& bondGroups = repository.BondGroups();
    auto& cell = repository.Cell();

    std::vector<atoms::domain::AtomInfo> oldCreated = createdAtoms;
    std::vector<atoms::domain::AtomInfo> oldSurrounding = surroundingAtoms;

    std::unordered_set<uint32_t> removedAtomIds;
    for (const auto& atom : oldCreated) {
        if (atom.structureId < 0) {
            removedAtomIds.insert(atom.id);
        }
    }
    for (const auto& atom : oldSurrounding) {
        if (atom.structureId < 0) {
            removedAtomIds.insert(atom.id);
        }
    }
    for (uint32_t atomId : removedAtomIds) {
        m_AtomVisibilityById.erase(atomId);
        m_AtomLabelVisibilityById.erase(atomId);
    }

    if (removedAtomIds.empty()) {
        if (m_vtkRenderer) {
            m_vtkRenderer->clearUnitCell(-1);
        }
        if (m_CurrentStructureId < 0) {
            atoms::domain::setCellVisible(false);
            cell.hasCell = false;
        }
        return;
    }

    createdAtoms.erase(std::remove_if(createdAtoms.begin(), createdAtoms.end(), [](const atoms::domain::AtomInfo& atom) {
        return atom.structureId < 0;
    }), createdAtoms.end());
    surroundingAtoms.erase(std::remove_if(surroundingAtoms.begin(), surroundingAtoms.end(), [](const atoms::domain::AtomInfo& atom) {
        return atom.structureId < 0;
    }), surroundingAtoms.end());

    std::unordered_map<uint32_t, int> newCreatedIndex;
    std::unordered_map<uint32_t, int> newSurroundingIndex;
    for (size_t i = 0; i < createdAtoms.size(); ++i) {
        newCreatedIndex[createdAtoms[i].id] = static_cast<int>(i);
    }
    for (size_t i = 0; i < surroundingAtoms.size(); ++i) {
        newSurroundingIndex[surroundingAtoms[i].id] = static_cast<int>(i);
    }

    for (auto itGroup = atomGroups.begin(); itGroup != atomGroups.end(); ) {
        auto& group = itGroup->second;
        std::vector<vtkSmartPointer<vtkTransform>> transforms;
        std::vector<atoms::domain::Color4f> colors;
        std::vector<uint32_t> atomIds;
        std::vector<atoms::domain::AtomType> atomTypes;
        std::vector<size_t> originalIndices;

        for (size_t i = 0; i < group.atomIds.size(); ++i) {
            uint32_t atomId = group.atomIds[i];
            if (removedAtomIds.find(atomId) != removedAtomIds.end()) {
                continue;
            }
            transforms.push_back(group.transforms[i]);
            colors.push_back(group.colors[i]);
            atomIds.push_back(atomId);
            atomTypes.push_back(group.atomTypes[i]);
            auto itCreated = newCreatedIndex.find(atomId);
            if (itCreated != newCreatedIndex.end()) {
                originalIndices.push_back(static_cast<size_t>(itCreated->second));
            } else {
                auto itSur = newSurroundingIndex.find(atomId);
                if (itSur != newSurroundingIndex.end()) {
                    originalIndices.push_back(static_cast<size_t>(itSur->second));
                } else {
                    originalIndices.push_back(SIZE_MAX);
                }
            }
        }

        group.transforms = std::move(transforms);
        group.colors = std::move(colors);
        group.atomIds = std::move(atomIds);
        group.atomTypes = std::move(atomTypes);
        group.originalIndices = std::move(originalIndices);

        if (group.atomIds.empty()) {
            if (m_vtkRenderer) {
                m_vtkRenderer->clearAtomGroupVTK(itGroup->first);
            }
            itGroup = atomGroups.erase(itGroup);
        } else {
            ++itGroup;
        }
    }

    auto resolveAtomId = [&](int index, atoms::domain::BondType type) -> uint32_t {
        if (index >= 0) {
            if (type == atoms::domain::BondType::SURROUNDING) {
                if (index < static_cast<int>(oldSurrounding.size())) return oldSurrounding[index].id;
                if (index < static_cast<int>(oldCreated.size())) return oldCreated[index].id;
            } else {
                if (index < static_cast<int>(oldCreated.size())) return oldCreated[index].id;
                if (index < static_cast<int>(oldSurrounding.size())) return oldSurrounding[index].id;
            }
        } else {
            int s = -index - 1;
            if (s >= 0 && s < static_cast<int>(oldSurrounding.size())) return oldSurrounding[s].id;
        }
        return 0;
    };

    std::unordered_set<uint32_t> removedBondIds;
    for (const auto& bond : createdBonds) {
        if (bond.structureId < 0) {
            removedBondIds.insert(bond.id);
        }
    }
    for (const auto& bond : surroundingBonds) {
        if (bond.structureId < 0) {
            removedBondIds.insert(bond.id);
        }
    }
    for (uint32_t bondId : removedBondIds) {
        m_BondVisibilityById.erase(bondId);
        m_BondLabelVisibilityById.erase(bondId);
    }

    createdBonds.erase(std::remove_if(createdBonds.begin(), createdBonds.end(), [](const atoms::domain::BondInfo& bond) {
        return bond.structureId < 0;
    }), createdBonds.end());
    surroundingBonds.erase(std::remove_if(surroundingBonds.begin(), surroundingBonds.end(), [](const atoms::domain::BondInfo& bond) {
        return bond.structureId < 0;
    }), surroundingBonds.end());

    auto updateBondIndex = [&](atoms::domain::BondInfo& bond) {
        uint32_t atom1Id = bond.atom1Id != 0 ? bond.atom1Id : resolveAtomId(bond.atom1Index, bond.bondType);
        uint32_t atom2Id = bond.atom2Id != 0 ? bond.atom2Id : resolveAtomId(bond.atom2Index, bond.bondType);
        int newIdx1 = -1;
        int newIdx2 = -1;
        if (bond.bondType == atoms::domain::BondType::SURROUNDING) {
            auto it1 = newSurroundingIndex.find(atom1Id);
            auto it2 = newSurroundingIndex.find(atom2Id);
            if (it1 != newSurroundingIndex.end()) newIdx1 = it1->second;
            if (it2 != newSurroundingIndex.end()) newIdx2 = it2->second;
            if (newIdx1 == -1) {
                auto it1c = newCreatedIndex.find(atom1Id);
                if (it1c != newCreatedIndex.end()) newIdx1 = it1c->second;
            }
            if (newIdx2 == -1) {
                auto it2c = newCreatedIndex.find(atom2Id);
                if (it2c != newCreatedIndex.end()) newIdx2 = it2c->second;
            }
        } else {
            auto it1 = newCreatedIndex.find(atom1Id);
            auto it2 = newCreatedIndex.find(atom2Id);
            if (it1 != newCreatedIndex.end()) newIdx1 = it1->second;
            if (it2 != newCreatedIndex.end()) newIdx2 = it2->second;
            if (newIdx1 == -1) {
                auto it1s = newSurroundingIndex.find(atom1Id);
                if (it1s != newSurroundingIndex.end()) newIdx1 = -(it1s->second + 1);
            }
            if (newIdx2 == -1) {
                auto it2s = newSurroundingIndex.find(atom2Id);
                if (it2s != newSurroundingIndex.end()) newIdx2 = -(it2s->second + 1);
            }
        }
        bond.atom1Id = atom1Id;
        bond.atom2Id = atom2Id;
        bond.atom1Index = newIdx1;
        bond.atom2Index = newIdx2;
    };

    for (auto& bond : createdBonds) {
        updateBondIndex(bond);
    }
    for (auto& bond : surroundingBonds) {
        updateBondIndex(bond);
    }

    for (auto& [key, group] : bondGroups) {
        std::vector<vtkSmartPointer<vtkTransform>> transforms1;
        std::vector<vtkSmartPointer<vtkTransform>> transforms2;
        std::vector<atoms::domain::Color4f> colors1;
        std::vector<atoms::domain::Color4f> colors2;
        std::vector<uint32_t> bondIds;
        for (size_t i = 0; i < group.bondIds.size(); ++i) {
            uint32_t bondId = group.bondIds[i];
            if (removedBondIds.find(bondId) != removedBondIds.end()) {
                continue;
            }
            transforms1.push_back(group.transforms1[i]);
            transforms2.push_back(group.transforms2[i]);
            colors1.push_back(group.colors1[i]);
            colors2.push_back(group.colors2[i]);
            bondIds.push_back(bondId);
        }
        group.transforms1 = std::move(transforms1);
        group.transforms2 = std::move(transforms2);
        group.colors1 = std::move(colors1);
        group.colors2 = std::move(colors2);
        group.bondIds = std::move(bondIds);
    }

    if (m_vtkRenderer) {
        m_vtkRenderer->clearUnitCell(-1);
    }
    if (m_CurrentStructureId < 0) {
        atoms::domain::setCellVisible(false);
        cell.hasCell = false;
    }
    if (m_ChargeDensityStructureId < 0 && HasChargeDensity()) {
        ClearChargeDensity();
    }

    refreshRenderedGroups();
}





