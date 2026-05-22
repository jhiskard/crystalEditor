#include "../../app.h"

#include "density/application/density_service.h"
#include "mesh/application/mesh_query_service.h"
#include "mesh/domain/mesh_entity.h"
#include "mesh/presentation/mesh_detail_panel.h"
#include "mesh/presentation/mesh_group_detail_panel.h"
#include "mesh/presentation/model_tree_panel.h"
#include "render/presentation/viewer_window.h"
#include "shell/application/shell_state_command_service.h"
#include "shell/application/shell_state_query_service.h"
#include "shell/presentation/atoms/atoms_window_presenter.h"
#include "shell/presentation/font/font_registry.h"
#include "shell/presentation/window_layout.h"
#include "shell/presentation/window_registry.h"
#include "shell/runtime/workbench_runtime.h"
#include "structure/application/structure_lifecycle_service.h"

#include <imgui.h>

#include <fstream>
#include <sstream>

namespace {

const char* kViewerWindowName = ICON_FA6_DISPLAY "  Viewer";
const char* kModelTreeWindowName = ICON_FA6_FOLDER_TREE "  Model Tree";
const char* kPeriodicTableWindowName = "Periodic Table";
const char* kCrystalTemplatesWindowName = "Crystal Templates";
const char* kBrillouinZonePlotWindowName = "Brillouin Zone Plot";
const char* kCreatedAtomsWindowName = "Created Atoms";
const char* kBondsManagementWindowName = "Bonds Management";
const char* kCellInformationWindowName = "Cell Information";
const char* kChargeDensityViewerWindowName = "Charge Density Viewer";
const char* kSliceViewerWindowName = "2D Slice Viewer";

bool ParseVisibilityValue(const std::string& value, bool& outValue) {
    if (value == "1" || value == "true" || value == "TRUE") {
        outValue = true;
        return true;
    }
    if (value == "0" || value == "false" || value == "FALSE") {
        outValue = false;
        return true;
    }
    return false;
}

}  // namespace

namespace shell::presentation {

const char* ViewerWindowName() {
    return kViewerWindowName;
}

const char* ModelTreeWindowName() {
    return kModelTreeWindowName;
}

const char* PeriodicTableWindowName() {
    return kPeriodicTableWindowName;
}

const char* CrystalTemplatesWindowName() {
    return kCrystalTemplatesWindowName;
}

const char* BrillouinZonePlotWindowName() {
    return kBrillouinZonePlotWindowName;
}

const char* CreatedAtomsWindowName() {
    return kCreatedAtomsWindowName;
}

const char* BondsManagementWindowName() {
    return kBondsManagementWindowName;
}

const char* CellInformationWindowName() {
    return kCellInformationWindowName;
}

const char* ChargeDensityViewerWindowName() {
    return kChargeDensityViewerWindowName;
}

const char* SliceViewerWindowName() {
    return kSliceViewerWindowName;
}

}  // namespace shell::presentation

void App::syncShellStateFromStore() {
    const shell::domain::ShellUiState& shellState = GetWorkbenchRuntime().ShellStateQuery().State();

    m_bFullDockSpace = shellState.fullDockSpace;
#ifdef SHOW_IMGUI_DEMO
    m_bShowDemoWindow = shellState.showDemoWindow;
#endif
#ifdef SHOW_FONT_ICONS
    m_bShowFontIcons = shellState.showFontIcons;
#endif
    m_bShowVtkViewer = shellState.showVtkViewer;
    m_bShowModelTree = shellState.showModelTree;
    m_bShowTestWindow = shellState.showTestWindow;
    m_bShowMeshDetail = shellState.showMeshDetail;
    m_bShowPeriodicTableWindow = shellState.showPeriodicTableWindow;
    m_bShowCrystalTemplatesWindow = shellState.showCrystalTemplatesWindow;
    m_bShowBrillouinZonePlotWindow = shellState.showBrillouinZonePlotWindow;
    m_bShowCreatedAtomsWindow = shellState.showCreatedAtomsWindow;
    m_bShowBondsManagementWindow = shellState.showBondsManagementWindow;
    m_bShowCellInformationWindow = shellState.showCellInformationWindow;
    m_bShowChargeDensityViewerWindow = shellState.showChargeDensityViewerWindow;
    m_bShowSliceViewerWindow = shellState.showSliceViewerWindow;
    m_RequestModelTreeFocus = shellState.requestModelTreeFocus;
    m_PendingFocusPassesRemaining = shellState.pendingFocusPassesRemaining;
    m_ResetWindowGeometryPassesRemaining = shellState.resetWindowGeometryPassesRemaining;
    m_ShouldApplyInitialLayout = shellState.shouldApplyInitialLayout;

    switch (shellState.pendingFocusTarget) {
    case shell::domain::ShellFocusTarget::None:
        m_PendingFocusTarget = FocusTarget::None;
        break;
    case shell::domain::ShellFocusTarget::ModelTree:
        m_PendingFocusTarget = FocusTarget::ModelTree;
        break;
    case shell::domain::ShellFocusTarget::PeriodicTable:
        m_PendingFocusTarget = FocusTarget::PeriodicTable;
        break;
    case shell::domain::ShellFocusTarget::CrystalTemplates:
        m_PendingFocusTarget = FocusTarget::CrystalTemplates;
        break;
    case shell::domain::ShellFocusTarget::BrillouinZonePlot:
        m_PendingFocusTarget = FocusTarget::BrillouinZonePlot;
        break;
    case shell::domain::ShellFocusTarget::CreatedAtoms:
        m_PendingFocusTarget = FocusTarget::CreatedAtoms;
        break;
    case shell::domain::ShellFocusTarget::BondsManagement:
        m_PendingFocusTarget = FocusTarget::BondsManagement;
        break;
    case shell::domain::ShellFocusTarget::CellInformation:
        m_PendingFocusTarget = FocusTarget::CellInformation;
        break;
    case shell::domain::ShellFocusTarget::ChargeDensityViewer:
        m_PendingFocusTarget = FocusTarget::ChargeDensityViewer;
        break;
    case shell::domain::ShellFocusTarget::SliceViewer:
        m_PendingFocusTarget = FocusTarget::SliceViewer;
        break;
    }

    switch (shellState.pendingLayoutPreset) {
    case shell::domain::ShellLayoutPreset::None:
        m_PendingLayoutPreset = LayoutPreset::None;
        break;
    case shell::domain::ShellLayoutPreset::DefaultFloating:
        m_PendingLayoutPreset = LayoutPreset::DefaultFloating;
        break;
    case shell::domain::ShellLayoutPreset::DockRight:
        m_PendingLayoutPreset = LayoutPreset::DockRight;
        break;
    case shell::domain::ShellLayoutPreset::DockBottom:
        m_PendingLayoutPreset = LayoutPreset::DockBottom;
        break;
    case shell::domain::ShellLayoutPreset::ResetDocking:
        m_PendingLayoutPreset = LayoutPreset::ResetDocking;
        break;
    }
}

void App::syncShellStateToStore() {
    shell::domain::ShellUiState& shellState = GetWorkbenchRuntime().ShellStateCommand().MutableState();

    shellState.fullDockSpace = m_bFullDockSpace;
#ifdef SHOW_IMGUI_DEMO
    shellState.showDemoWindow = m_bShowDemoWindow;
#endif
#ifdef SHOW_FONT_ICONS
    shellState.showFontIcons = m_bShowFontIcons;
#endif
    shellState.showVtkViewer = m_bShowVtkViewer;
    shellState.showModelTree = m_bShowModelTree;
    shellState.showTestWindow = m_bShowTestWindow;
    shellState.showMeshDetail = m_bShowMeshDetail;
    shellState.showPeriodicTableWindow = m_bShowPeriodicTableWindow;
    shellState.showCrystalTemplatesWindow = m_bShowCrystalTemplatesWindow;
    shellState.showBrillouinZonePlotWindow = m_bShowBrillouinZonePlotWindow;
    shellState.showCreatedAtomsWindow = m_bShowCreatedAtomsWindow;
    shellState.showBondsManagementWindow = m_bShowBondsManagementWindow;
    shellState.showCellInformationWindow = m_bShowCellInformationWindow;
    shellState.showChargeDensityViewerWindow = m_bShowChargeDensityViewerWindow;
    shellState.showSliceViewerWindow = m_bShowSliceViewerWindow;
    shellState.requestModelTreeFocus = m_RequestModelTreeFocus;
    shellState.pendingFocusPassesRemaining = m_PendingFocusPassesRemaining;
    shellState.resetWindowGeometryPassesRemaining = m_ResetWindowGeometryPassesRemaining;
    shellState.shouldApplyInitialLayout = m_ShouldApplyInitialLayout;

    switch (m_PendingFocusTarget) {
    case FocusTarget::None:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::None;
        break;
    case FocusTarget::ModelTree:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::ModelTree;
        break;
    case FocusTarget::PeriodicTable:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::PeriodicTable;
        break;
    case FocusTarget::CrystalTemplates:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::CrystalTemplates;
        break;
    case FocusTarget::BrillouinZonePlot:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::BrillouinZonePlot;
        break;
    case FocusTarget::CreatedAtoms:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::CreatedAtoms;
        break;
    case FocusTarget::BondsManagement:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::BondsManagement;
        break;
    case FocusTarget::CellInformation:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::CellInformation;
        break;
    case FocusTarget::ChargeDensityViewer:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::ChargeDensityViewer;
        break;
    case FocusTarget::SliceViewer:
        shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::SliceViewer;
        break;
    }

    // NOTE:
    // pendingLayoutPreset is treated as a one-shot command from ShellStateCommandService.
    // Do not mirror App's local cache back into the store here, otherwise menu-triggered
    // preset requests can be overwritten in the same frame before they are consumed.
}

void App::SaveImGuiIniFile() {
    GetWorkbenchRuntime().AppController().syncShellStateFromStore();

    size_t dataSize = 0;
    const char* data = ImGui::SaveIniSettingsToMemory(&dataSize);

    std::ofstream iniFile(
        GetWorkbenchRuntime().AppController().IMGUI_SETTING_FILE_PATH,
        std::ios::out | std::ios::trunc);
    if (!iniFile) {
        SPDLOG_ERROR(
            "Failed to open ImGui setting file for writing: {}",
            GetWorkbenchRuntime().AppController().IMGUI_SETTING_FILE_PATH);
        return;
    }
    iniFile.write(data, dataSize);
    iniFile.close();

    std::ofstream windowFile(
        GetWorkbenchRuntime().AppController().WINDOW_SETTING_FILE_PATH,
        std::ios::out | std::ios::trunc);
    if (!windowFile) {
        SPDLOG_ERROR(
            "Failed to open window visibility file for writing: {}",
            GetWorkbenchRuntime().AppController().WINDOW_SETTING_FILE_PATH);
        return;
    }

    const bool anyBuilderWindow =
        GetWorkbenchRuntime().AppController().m_bShowPeriodicTableWindow ||
        GetWorkbenchRuntime().AppController().m_bShowCrystalTemplatesWindow ||
        GetWorkbenchRuntime().AppController().m_bShowBrillouinZonePlotWindow;
    const bool anyEditorWindow =
        GetWorkbenchRuntime().AppController().m_bShowCreatedAtomsWindow ||
        GetWorkbenchRuntime().AppController().m_bShowBondsManagementWindow ||
        GetWorkbenchRuntime().AppController().m_bShowCellInformationWindow;
    const bool anyDataWindow =
        GetWorkbenchRuntime().AppController().m_bShowChargeDensityViewerWindow ||
        GetWorkbenchRuntime().AppController().m_bShowSliceViewerWindow;

    // Legacy group keys for backward compatibility.
    windowFile << "viewer=" << (GetWorkbenchRuntime().AppController().m_bShowVtkViewer ? 1 : 0)
               << "\n";
    windowFile << "model_tree=" << (GetWorkbenchRuntime().AppController().m_bShowModelTree ? 1 : 0)
               << "\n";
    windowFile << "crystal_builder=" << (anyBuilderWindow ? 1 : 0) << "\n";
    windowFile << "crystal_editor=" << (anyEditorWindow ? 1 : 0) << "\n";
    windowFile << "advanced_view=" << (anyDataWindow ? 1 : 0) << "\n";

    // Split-window keys.
    windowFile << "periodic_table_window="
               << (GetWorkbenchRuntime().AppController().m_bShowPeriodicTableWindow ? 1 : 0) << "\n";
    windowFile << "crystal_templates_window="
               << (GetWorkbenchRuntime().AppController().m_bShowCrystalTemplatesWindow ? 1 : 0) << "\n";
    windowFile << "brillouin_zone_plot_window="
               << (GetWorkbenchRuntime().AppController().m_bShowBrillouinZonePlotWindow ? 1 : 0) << "\n";
    windowFile << "created_atoms_window="
               << (GetWorkbenchRuntime().AppController().m_bShowCreatedAtomsWindow ? 1 : 0) << "\n";
    windowFile << "bonds_management_window="
               << (GetWorkbenchRuntime().AppController().m_bShowBondsManagementWindow ? 1 : 0) << "\n";
    windowFile << "cell_information_window="
               << (GetWorkbenchRuntime().AppController().m_bShowCellInformationWindow ? 1 : 0) << "\n";
    windowFile << "charge_density_viewer_window="
               << (GetWorkbenchRuntime().AppController().m_bShowChargeDensityViewerWindow ? 1 : 0) << "\n";
    windowFile << "slice_viewer_window="
               << (GetWorkbenchRuntime().AppController().m_bShowSliceViewerWindow ? 1 : 0) << "\n";
    windowFile.close();
}

void App::LoadImGuiIniFile() {
    std::ifstream iniFile(GetWorkbenchRuntime().AppController().IMGUI_SETTING_FILE_PATH, std::ifstream::in);
    if (!iniFile) {
        SPDLOG_INFO("ImGui setting file not found. Using default layout.");
    } else {
        std::ostringstream ss;
        ss << iniFile.rdbuf();
        iniFile.close();
        const std::string fileContents = ss.str();
        ImGui::LoadIniSettingsFromMemory(fileContents.c_str(), fileContents.size());
    }

    std::ifstream windowFile(GetWorkbenchRuntime().AppController().WINDOW_SETTING_FILE_PATH, std::ifstream::in);
    if (!windowFile) {
        SPDLOG_INFO("Window visibility file not found. Using default window visibility.");
        GetWorkbenchRuntime().AppController().m_ShouldApplyInitialLayout = true;
        GetWorkbenchRuntime().AppController().m_bShowVtkViewer = true;
        GetWorkbenchRuntime().AppController().m_bShowModelTree = false;
        GetWorkbenchRuntime().AppController().m_bShowPeriodicTableWindow = false;
        GetWorkbenchRuntime().AppController().m_bShowCrystalTemplatesWindow = false;
        GetWorkbenchRuntime().AppController().m_bShowBrillouinZonePlotWindow = false;
        GetWorkbenchRuntime().AppController().m_bShowCreatedAtomsWindow = false;
        GetWorkbenchRuntime().AppController().m_bShowBondsManagementWindow = false;
        GetWorkbenchRuntime().AppController().m_bShowCellInformationWindow = false;
        GetWorkbenchRuntime().AppController().m_bShowChargeDensityViewerWindow = false;
        GetWorkbenchRuntime().AppController().m_bShowSliceViewerWindow = false;
        GetWorkbenchRuntime().AppController().syncShellStateToStore();
        return;
    }
    GetWorkbenchRuntime().AppController().m_ShouldApplyInitialLayout = false;

    std::string line;
    while (std::getline(windowFile, line)) {
        const std::size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) {
            continue;
        }

        const std::string key = line.substr(0, delimiterPos);
        const std::string value = line.substr(delimiterPos + 1);
        bool parsedValue = false;
        if (!ParseVisibilityValue(value, parsedValue)) {
            continue;
        }

        if (key == "viewer") {
            GetWorkbenchRuntime().AppController().m_bShowVtkViewer = parsedValue;
        } else if (key == "model_tree") {
            GetWorkbenchRuntime().AppController().m_bShowModelTree = parsedValue;
        } else if (key == "crystal_builder") {
            GetWorkbenchRuntime().AppController().m_bShowPeriodicTableWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowCrystalTemplatesWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowBrillouinZonePlotWindow = parsedValue;
        } else if (key == "crystal_editor") {
            GetWorkbenchRuntime().AppController().m_bShowCreatedAtomsWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowBondsManagementWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowCellInformationWindow = parsedValue;
        } else if (key == "advanced_view") {
            GetWorkbenchRuntime().AppController().m_bShowChargeDensityViewerWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowSliceViewerWindow = parsedValue;
        } else if (key == "periodic_table_window") {
            GetWorkbenchRuntime().AppController().m_bShowPeriodicTableWindow = parsedValue;
        } else if (key == "crystal_templates_window") {
            GetWorkbenchRuntime().AppController().m_bShowCrystalTemplatesWindow = parsedValue;
        } else if (key == "brillouin_zone_plot_window") {
            GetWorkbenchRuntime().AppController().m_bShowBrillouinZonePlotWindow = parsedValue;
        } else if (key == "created_atoms_window") {
            GetWorkbenchRuntime().AppController().m_bShowCreatedAtomsWindow = parsedValue;
        } else if (key == "bonds_management_window") {
            GetWorkbenchRuntime().AppController().m_bShowBondsManagementWindow = parsedValue;
        } else if (key == "cell_information_window") {
            GetWorkbenchRuntime().AppController().m_bShowCellInformationWindow = parsedValue;
        } else if (key == "charge_density_viewer_window") {
            GetWorkbenchRuntime().AppController().m_bShowChargeDensityViewerWindow = parsedValue;
        } else if (key == "slice_viewer_window") {
            GetWorkbenchRuntime().AppController().m_bShowSliceViewerWindow = parsedValue;
        }
    }

    if (!GetWorkbenchRuntime().AppController().m_bShowModelTree) {
        GetWorkbenchRuntime().AppController().m_RequestModelTreeFocus = false;
    }

    GetWorkbenchRuntime().AppController().syncShellStateToStore();
}

void App::InitImGuiWindows() {
    // Keep panel/materialization warm-up in the runtime composition root path.
    (void)GetWorkbenchRuntime().Viewer();
    (void)GetWorkbenchRuntime().TestWindowPanel();
    (void)GetWorkbenchRuntime().ModelTreePanel();
    (void)GetWorkbenchRuntime().MeshDetailPanel();
    (void)GetWorkbenchRuntime().MeshGroupDetailPanel();
    (void)GetWorkbenchRuntime().AtomsWindowFeature();
}

void App::renderImGuiWindows() {
    syncShellStateFromStore();
    shell::presentation::atoms::AtomsWindowPresenter& atomsWindowPresenter =
        GetWorkbenchRuntime().AtomsWindowFeature();

    if (m_bShowAboutPopup) {
        renderAboutPopup();
    }

    {
        shell::domain::ShellUiState& shellState = GetWorkbenchRuntime().ShellStateCommand().MutableState();
        atomsWindowPresenter.ApplyPendingPanelRequests(shellState);
    }

    const bool applyResetLayoutThisFrame = m_ResetWindowGeometryPassesRemaining > 0;
    bool hasResetLayout = false;
    shell::presentation::ResetWindowLayout resetLayout {};
    if (applyResetLayoutThisFrame) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport != nullptr) {
            resetLayout = shell::presentation::BuildResetWindowLayout(viewport);
            hasResetLayout = true;

            GetWorkbenchRuntime().Viewer().RequestForcedWindowLayout(
                resetLayout.viewerPos,
                resetLayout.viewerSize);

            atomsWindowPresenter.RequestForcedBuilderWindowLayout(
                resetLayout.crystalBuilderPos,
                resetLayout.panelSize);
            atomsWindowPresenter.RequestForcedEditorWindowLayout(
                resetLayout.crystalEditorPos,
                resetLayout.panelSize);
            atomsWindowPresenter.RequestForcedAdvancedWindowLayout(
                resetLayout.advancedViewPos,
                resetLayout.panelSize);
        }
    }

    if (m_bShowVtkViewer) {
        GetWorkbenchRuntime().Viewer().Render();
    }

    const bool requestModelTreeFocus = m_RequestModelTreeFocus;

    if (m_bShowModelTree) {
        if (hasResetLayout) {
            ImGui::SetNextWindowDockID(0, ImGuiCond_Always);
            ImGui::SetNextWindowPos(resetLayout.modelTreePos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(resetLayout.panelSize, ImGuiCond_Always);
        }
        if (requestModelTreeFocus) {
            ImGui::SetNextWindowFocus();
        }
        GetWorkbenchRuntime().ModelTreePanel().Render(&m_bShowModelTree);
    }

    if (m_bShowPeriodicTableWindow) {
        atomsWindowPresenter.RenderPeriodicTableWindow(&m_bShowPeriodicTableWindow);
    }
    if (m_bShowCrystalTemplatesWindow) {
        atomsWindowPresenter.RenderCrystalTemplatesWindow(&m_bShowCrystalTemplatesWindow);
    }
    if (m_bShowBrillouinZonePlotWindow) {
        GetWorkbenchRuntime().StructureLifecycleFeature().RenderBrillouinZonePlotWindow(
            &m_bShowBrillouinZonePlotWindow);
    }
    if (m_bShowCreatedAtomsWindow) {
        atomsWindowPresenter.RenderCreatedAtomsWindow(&m_bShowCreatedAtomsWindow);
    }
    if (m_bShowBondsManagementWindow) {
        atomsWindowPresenter.RenderBondsManagementWindow(&m_bShowBondsManagementWindow);
    }
    if (m_bShowCellInformationWindow) {
        atomsWindowPresenter.RenderCellInformationWindow(&m_bShowCellInformationWindow);
    }

    density::application::DensityService& densityService = GetWorkbenchRuntime().DensityFeature();
    if (m_bShowChargeDensityViewerWindow) {
        densityService.RenderChargeDensityViewerWindow(&m_bShowChargeDensityViewerWindow);
    }
    if (m_bShowSliceViewerWindow) {
        densityService.RenderSliceViewerWindow(&m_bShowSliceViewerWindow);
    }

    const int32_t selectedMeshId = GetWorkbenchRuntime().ModelTreePanel().GetSelectedMeshId();
    if (selectedMeshId != -1 && m_bShowMeshDetail) {
        GetWorkbenchRuntime().MeshDetailPanel().Render(selectedMeshId, &m_bShowMeshDetail);
    }
    const Mesh* mesh = mesh::application::GetMeshQueryService().FindMeshById(selectedMeshId);
    if (mesh != nullptr && mesh->GetMeshGroupCount() > 0) {
        GetWorkbenchRuntime().MeshGroupDetailPanel().Render(selectedMeshId);
    }

#ifdef SHOW_IMGUI_DEMO
    if (m_bShowDemoWindow) {
        ImGui::ShowDemoWindow(&m_bShowDemoWindow);
    }
#endif
#ifdef SHOW_FONT_ICONS
    if (m_bShowFontIcons) {
        GetWorkbenchRuntime().FontRegistry().Render();
    }
#endif

    if (m_bShowBgColorPopup) {
        GetWorkbenchRuntime().Viewer().RenderBgColorPopup(&m_bShowBgColorPopup);
    }
    if (m_bShowProgressPopup) {
        renderProgressPopup();
    }
    GetWorkbenchRuntime().RenderXsfGridImportPopups();

    if (hasResetLayout) {
        ImGui::MarkIniSettingsDirty();
        --m_ResetWindowGeometryPassesRemaining;
    }

    if (requestModelTreeFocus && m_bShowModelTree) {
        ImGui::SetWindowFocus(shell::presentation::ModelTreeWindowName());
        m_RequestModelTreeFocus = false;
    }

    if (m_PendingFocusTarget != FocusTarget::None && m_PendingFocusPassesRemaining > 0) {
        const char* targetWindowName = nullptr;
        switch (m_PendingFocusTarget) {
        case FocusTarget::ModelTree:
            if (m_bShowModelTree) {
                targetWindowName = shell::presentation::ModelTreeWindowName();
            }
            break;
        case FocusTarget::PeriodicTable:
            if (m_bShowPeriodicTableWindow) {
                targetWindowName = shell::presentation::PeriodicTableWindowName();
            }
            break;
        case FocusTarget::CrystalTemplates:
            if (m_bShowCrystalTemplatesWindow) {
                targetWindowName = shell::presentation::CrystalTemplatesWindowName();
            }
            break;
        case FocusTarget::BrillouinZonePlot:
            if (m_bShowBrillouinZonePlotWindow) {
                targetWindowName = shell::presentation::BrillouinZonePlotWindowName();
            }
            break;
        case FocusTarget::CreatedAtoms:
            if (m_bShowCreatedAtomsWindow) {
                targetWindowName = shell::presentation::CreatedAtomsWindowName();
            }
            break;
        case FocusTarget::BondsManagement:
            if (m_bShowBondsManagementWindow) {
                targetWindowName = shell::presentation::BondsManagementWindowName();
            }
            break;
        case FocusTarget::CellInformation:
            if (m_bShowCellInformationWindow) {
                targetWindowName = shell::presentation::CellInformationWindowName();
            }
            break;
        case FocusTarget::ChargeDensityViewer:
            if (m_bShowChargeDensityViewerWindow) {
                targetWindowName = shell::presentation::ChargeDensityViewerWindowName();
            }
            break;
        case FocusTarget::SliceViewer:
            if (m_bShowSliceViewerWindow) {
                targetWindowName = shell::presentation::SliceViewerWindowName();
            }
            break;
        case FocusTarget::None:
            break;
        }

        if (targetWindowName != nullptr) {
            ImGui::SetWindowFocus(targetWindowName);
            --m_PendingFocusPassesRemaining;
        } else {
            m_PendingFocusPassesRemaining = 0;
        }

        if (m_PendingFocusPassesRemaining <= 0) {
            m_PendingFocusTarget = FocusTarget::None;
        }
    }

    syncShellStateToStore();
}
