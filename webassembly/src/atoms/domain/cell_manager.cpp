#include "cell_manager.h"
#include "cell_transform.h"
#include "../../config/log_config.h"
#include "../legacy/atoms_template_facade.h"
#include "atom_manager.h"
#include "../../structure/domain/structure_repository.h"

CellInfo& GetCellInfo() {
    return structure::domain::GetStructureRepository().Cell();
}

std::vector<vtkSmartPointer<vtkActor>>& GetCellEdgeActors() {
    return structure::domain::GetStructureRepository().CellEdgeActors();
}

bool& GetCellVisible() {
    return structure::domain::GetStructureRepository().CellVisible();
}

namespace atoms::domain {

void setCellVisible(bool visible) {
    cellVisible = visible;
}

bool isCellVisible() {
    return cellVisible;
}

bool hasUnitCell() {
    return cellInfo.hasCell;
}

void setCellMatrix(const float matrix[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cellInfo.matrix[i][j] = matrix[i][j];
        }
    }
    calculateInverseMatrix(cellInfo.matrix, cellInfo.invmatrix);
}

void getCellMatrix(float out[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            out[i][j] = cellInfo.matrix[i][j];
        }
    }
}

void getCellInverse(float out[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            out[i][j] = cellInfo.invmatrix[i][j];
        }
    }
}

void setCellModified(bool modified) {
    cellInfo.modified = modified;
}

bool isCellModified() {
    return cellInfo.modified;
}

void applyCellChanges(::AtomsTemplate* parent) {
    if (!parent) {
        SPDLOG_ERROR("applyCellChanges called with null parent");
        return;
    }

    SPDLOG_DEBUG("Recalculating inverse matrix and fractional coordinates");
    calculateInverseMatrix(cellInfo.matrix, cellInfo.invmatrix);

    parent->clearUnitCell();
    parent->createUnitCell(cellInfo.matrix);

    for (auto& atom : createdAtoms) {
        cartesianToFractional(atom.position,      atom.fracPosition,     cellInfo.invmatrix);
        cartesianToFractional(atom.tempPosition,  atom.tempFracPosition, cellInfo.invmatrix);
    }

    for (auto& atom : surroundingAtoms) {
        cartesianToFractional(atom.position,      atom.fracPosition,     cellInfo.invmatrix);
        cartesianToFractional(atom.tempPosition,  atom.tempFracPosition, cellInfo.invmatrix);
    }

    if (parent->isSurroundingsVisible()) {
        SPDLOG_DEBUG("Regenerating surrounding atoms due to cell change");
        parent->hideSurroundingAtoms();
        parent->createSurroundingAtoms();
    }

    SPDLOG_INFO("Cell matrix update completed within batch");
}
} // namespace atoms::domain
