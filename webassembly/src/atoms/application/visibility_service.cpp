#include "../atoms_template.h"

#include "../../vtk_viewer.h"
#include "../ui/charge_density_ui.h"

void AtomsTemplate::SetStructureVisible(bool visible) {
    m_StructureVisible = visible;

    for (auto& [id, entry] : m_Structures) {
        entry.visible = visible;
    }
    for (auto& [id, cellEntry] : m_UnitCells) {
        (void)id;
        cellEntry.visible = visible;
    }
    for (const auto& [id, entry] : m_Structures) {
        (void)entry;
        m_StructureBondsVisible[id] = visible;
    }

    if (m_vtkRenderer) {
        m_vtkRenderer->setUnitCellVisible(visible);
    }
    atoms::domain::setCellVisible(visible);

    for (auto& [symbol, vis] : m_AtomGroupVisibility) {
        vis = visible;
    }
    m_BondsVisible = visible;
    if (m_chargeDensityUI && m_chargeDensityUI->hasData()) {
        m_chargeDensityUI->setIsosurfaceVisible(visible);
    }
    if (m_chargeDensityUI && m_ChargeDensityStructureId >= 0) {
        m_chargeDensityUI->setStructureVisible(visible);
    }

    refreshRenderedGroups();
}



void AtomsTemplate::SetAtomVisibleById(uint32_t atomId, bool visible) {
    if (atomId == 0) {
        return;
    }
    m_AtomVisibilityById[atomId] = visible;
    refreshRenderedGroups();
    VtkViewer::Instance().RequestRender();
}


void AtomsTemplate::SetAtomLabelVisibleById(uint32_t atomId, bool visible) {
    if (atomId == 0) {
        return;
    }
    auto it = m_AtomLabelVisibilityById.find(atomId);
    if (it != m_AtomLabelVisibilityById.end() && it->second == visible) {
        return;
    }
    m_AtomLabelVisibilityById[atomId] = visible;
    refreshLabelActors();
    VtkViewer::Instance().RequestRender();
}


void AtomsTemplate::SetBondVisibleById(uint32_t bondId, bool visible) {
    if (bondId == 0) {
        return;
    }
    m_BondVisibilityById[bondId] = visible;
    refreshRenderedGroups();
    VtkViewer::Instance().RequestRender();
}


void AtomsTemplate::SetBondLabelVisibleById(uint32_t bondId, bool visible) {
    if (bondId == 0) {
        return;
    }
    auto it = m_BondLabelVisibilityById.find(bondId);
    if (it != m_BondLabelVisibilityById.end() && it->second == visible) {
        return;
    }
    m_BondLabelVisibilityById[bondId] = visible;
    refreshLabelActors();
    VtkViewer::Instance().RequestRender();
}


void AtomsTemplate::SetStructureVisible(int32_t id, bool visible) {
    if (id < 0) {
        SetStructureVisible(visible);
        return;
    }
    auto it = m_Structures.find(id);
    if (it == m_Structures.end()) {
        return;
    }
    it->second.visible = visible;
    auto cellIt = m_UnitCells.find(id);
    if (cellIt != m_UnitCells.end() && m_vtkRenderer) {
        bool effectiveVisible = visible && cellIt->second.visible;
        m_vtkRenderer->setUnitCellVisible(id, effectiveVisible);
    }
    if (id == m_CurrentStructureId) {
        bool cellVisible = false;
        if (cellIt != m_UnitCells.end()) {
            cellVisible = visible && cellIt->second.visible;
        }
        atoms::domain::setCellVisible(cellVisible);
    }
    if (m_chargeDensityUI && m_ChargeDensityStructureId == id) {
        m_chargeDensityUI->setStructureVisible(visible);
    }
    if (visible) {
        ApplyChargeDensityAdvancedGridVisibilityForStructure(id);
    }
    refreshRenderedGroups();
}


void AtomsTemplate::SetUnitCellVisible(int32_t id, bool visible) {
    if (id < 0) {
        setCellVisible(visible);
        return;
    }
    auto it = m_UnitCells.find(id);
    if (it == m_UnitCells.end()) {
        return;
    }
    it->second.visible = visible;
    if (m_vtkRenderer) {
        bool effectiveVisible = visible && IsStructureVisible(id);
        m_vtkRenderer->setUnitCellVisible(id, effectiveVisible);
    }
    if (id == m_CurrentStructureId) {
        bool effectiveVisible = visible && IsStructureVisible(id);
        atoms::domain::setCellVisible(effectiveVisible);
    }
    VtkViewer::Instance().RequestRender();
}

