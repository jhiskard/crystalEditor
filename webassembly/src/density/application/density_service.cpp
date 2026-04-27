#include "density_service.h"

#include "../../density/presentation/charge_density_ui.h"
#include "../../workspace/runtime/legacy_atoms_runtime.h"

namespace density {
namespace application {

DensityService::DensityService() = default;

bool DensityService::HasChargeDensity() const {
    return workspace::legacy::WorkspaceRuntimeModelRef().HasChargeDensity();
}

bool DensityService::IsChargeDensityVisible() const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsChargeDensityVisible();
}

void DensityService::SetChargeDensityVisible(bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetChargeDensityVisible(visible);
}

bool DensityService::IsSimpleViewActive() const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsChargeDensitySimpleViewActive();
}

void DensityService::SyncChargeDensityViewTypeState() {
    workspace::legacy::WorkspaceRuntimeModelRef().SyncChargeDensityViewTypeState();
}

int32_t DensityService::GetChargeDensityStructureId() const {
    return workspace::legacy::WorkspaceRuntimeModelRef().GetChargeDensityStructureId();
}

void DensityService::SetChargeDensityStructureId(int32_t structureId) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetChargeDensityStructureId(structureId);
}

bool DensityService::IsAdvancedGridVisible(int32_t meshId, bool volumeMode) const {
    return workspace::legacy::WorkspaceRuntimeModelRef().IsChargeDensityAdvancedGridVisible(meshId, volumeMode);
}

void DensityService::SetAdvancedGridVisible(int32_t meshId, bool volumeMode, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetChargeDensityAdvancedGridVisible(meshId, volumeMode, visible);
}

void DensityService::SetAllAdvancedGridVisible(int32_t structureId, bool volumeMode, bool visible) {
    workspace::legacy::WorkspaceRuntimeModelRef().SetAllChargeDensityAdvancedGridVisible(structureId, volumeMode, visible);
}

void DensityService::ApplyAdvancedGridVisibilityForStructure(int32_t structureId) {
    workspace::legacy::WorkspaceRuntimeModelRef().ApplyChargeDensityAdvancedGridVisibilityForStructure(structureId);
}

std::vector<SimpleGridEntry> DensityService::GetSimpleGridEntries() const {
    std::vector<SimpleGridEntry> output;
    const atoms::ui::ChargeDensityUI* ui = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
    if (ui == nullptr) {
        return output;
    }

    const auto source = ui->getSimpleGridEntries();
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({ item.name, item.visible });
    }
    return output;
}

std::vector<SliceGridEntry> DensityService::GetSliceGridEntries() const {
    std::vector<SliceGridEntry> output;
    const atoms::ui::ChargeDensityUI* ui = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
    if (ui == nullptr) {
        return output;
    }

    const auto source = ui->getSliceGridEntries();
    output.reserve(source.size());
    for (const auto& item : source) {
        output.push_back({ item.name, item.visible });
    }
    return output;
}

bool DensityService::IsSliceVisible() const {
    const atoms::ui::ChargeDensityUI* ui = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
    return ui != nullptr && ui->isSliceVisible();
}

void DensityService::SetAllSimpleGridVisible(bool visible) {
    if (atoms::ui::ChargeDensityUI* ui = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI()) {
        ui->setAllSimpleGridVisible(visible);
    }
}

void DensityService::SetAllSliceGridVisible(bool visible) {
    if (atoms::ui::ChargeDensityUI* ui = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI()) {
        ui->setAllSliceGridVisible(visible);
    }
}

bool DensityService::SetSimpleGridVisible(const std::string& gridName, bool visible) {
    atoms::ui::ChargeDensityUI* ui = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
    return ui != nullptr && ui->setSimpleGridVisible(gridName, visible);
}

bool DensityService::SetSliceGridVisible(const std::string& gridName, bool visible) {
    atoms::ui::ChargeDensityUI* ui = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
    return ui != nullptr && ui->setSliceGridVisible(gridName, visible);
}

bool DensityService::SelectSimpleGridByName(const std::string& gridName) {
    atoms::ui::ChargeDensityUI* ui = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
    return ui != nullptr && ui->selectSimpleGridByName(gridName);
}

bool DensityService::SelectSliceGridByName(const std::string& gridName) {
    atoms::ui::ChargeDensityUI* ui = workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
    return ui != nullptr && ui->selectSliceGridByName(gridName);
}

void DensityService::RenderChargeDensityViewerWindow(bool* openWindow) {
    workspace::legacy::WorkspaceRuntimeModelRef().RenderChargeDensityViewerWindow(openWindow);
}

void DensityService::RenderSliceViewerWindow(bool* openWindow) {
    workspace::legacy::WorkspaceRuntimeModelRef().RenderSliceViewerWindow(openWindow);
}

atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() {
    return workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
}

const atoms::ui::ChargeDensityUI* DensityService::ChargeDensityUi() const {
    return workspace::legacy::WorkspaceRuntimeModelRef().chargeDensityUI();
}

} // namespace application
} // namespace density