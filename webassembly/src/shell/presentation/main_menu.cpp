#include "../../app.h"

#include "icon/FontAwesome6.h"
#include "shell/application/shell_state_command_service.h"
#include "shell/application/workbench_controller.h"
#include "shell/presentation/main_menu.h"
#include "shell/presentation/window_registry.h"
#include "shell/runtime/workbench_runtime.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <emscripten/emscripten.h>

void App::renderDockSpaceAndMenu() {
    syncShellStateFromStore();

    static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (m_bFullDockSpace) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspaceFlags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) {
        windowFlags |= ImGuiWindowFlags_NoBackground;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar();

    if (m_bFullDockSpace) {
        ImGui::PopStyleVar(2);
    }

    ImGuiIO& io = ImGui::GetIO();
    const ImGuiID dockspaceId = ImGui::GetID("DockSpace");

    auto dockLeftPanelWindows = [&](ImGuiID dockNodeId) {
        ImGui::DockBuilderDockWindow(shell::presentation::ModelTreeWindowName(), dockNodeId);
        ImGui::DockBuilderDockWindow(shell::presentation::CreatedAtomsWindowName(), dockNodeId);
        ImGui::DockBuilderDockWindow(shell::presentation::BondsManagementWindowName(), dockNodeId);
        ImGui::DockBuilderDockWindow(shell::presentation::CellInformationWindowName(), dockNodeId);
        ImGui::DockBuilderDockWindow(shell::presentation::PeriodicTableWindowName(), dockNodeId);
        ImGui::DockBuilderDockWindow(shell::presentation::CrystalTemplatesWindowName(), dockNodeId);
        ImGui::DockBuilderDockWindow(shell::presentation::BrillouinZonePlotWindowName(), dockNodeId);
    };

    auto applyLayoutVisibilityPreset = [&](LayoutPreset preset) {
        shell::application::ShellStateCommandService& command = GetWorkbenchRuntime().ShellStateCommand();

        auto setVisible = [&](shell::domain::ShellWindowId windowId, bool visible) {
            command.SetWindowVisible(windowId, visible);
        };

        auto setDefaultsToHidden = [&]() {
            setVisible(shell::domain::ShellWindowId::Viewer, true);
            setVisible(shell::domain::ShellWindowId::ModelTree, false);
            setVisible(shell::domain::ShellWindowId::PeriodicTable, false);
            setVisible(shell::domain::ShellWindowId::CrystalTemplates, false);
            setVisible(shell::domain::ShellWindowId::BrillouinZonePlot, false);
            setVisible(shell::domain::ShellWindowId::CreatedAtoms, false);
            setVisible(shell::domain::ShellWindowId::BondsManagement, false);
            setVisible(shell::domain::ShellWindowId::CellInformation, false);
            setVisible(shell::domain::ShellWindowId::ChargeDensityViewer, false);
            setVisible(shell::domain::ShellWindowId::SliceViewer, false);
        };

        shell::domain::ShellUiState& shellState = command.MutableState();
        shellState.requestModelTreeFocus = false;
        shellState.shouldApplyInitialLayout = false;
        command.RequestFocus(shell::domain::ShellFocusTarget::None, 0);

        switch (preset) {
        case LayoutPreset::DefaultFloating:
            setDefaultsToHidden();
            break;
        case LayoutPreset::DockRight:
            setDefaultsToHidden();
            setVisible(shell::domain::ShellWindowId::ModelTree, true);
            setVisible(shell::domain::ShellWindowId::ChargeDensityViewer, true);
            setVisible(shell::domain::ShellWindowId::SliceViewer, true);
            shellState.requestModelTreeFocus = true;
            command.RequestFocus(shell::domain::ShellFocusTarget::ModelTree, 2);
            break;
        case LayoutPreset::DockBottom:
            setDefaultsToHidden();
            setVisible(shell::domain::ShellWindowId::ModelTree, true);
            shellState.requestModelTreeFocus = true;
            command.RequestFocus(shell::domain::ShellFocusTarget::ModelTree, 2);
            break;
        case LayoutPreset::ResetDocking:
            setVisible(shell::domain::ShellWindowId::Viewer, true);
            setVisible(shell::domain::ShellWindowId::ModelTree, true);
            setVisible(shell::domain::ShellWindowId::PeriodicTable, true);
            setVisible(shell::domain::ShellWindowId::CrystalTemplates, true);
            setVisible(shell::domain::ShellWindowId::BrillouinZonePlot, true);
            setVisible(shell::domain::ShellWindowId::CreatedAtoms, true);
            setVisible(shell::domain::ShellWindowId::BondsManagement, true);
            setVisible(shell::domain::ShellWindowId::CellInformation, true);
            setVisible(shell::domain::ShellWindowId::ChargeDensityViewer, true);
            setVisible(shell::domain::ShellWindowId::SliceViewer, false);
            break;
        case LayoutPreset::None:
            break;
        }

        syncShellStateFromStore();
    };

    if (m_ShouldApplyInitialLayout && m_PendingLayoutPreset == LayoutPreset::None) {
        m_PendingLayoutPreset = LayoutPreset::DefaultFloating;
        m_ShouldApplyInitialLayout = false;
    }

    if (m_PendingLayoutPreset != LayoutPreset::None) {
        m_ShouldApplyInitialLayout = false;
        GetWorkbenchRuntime().ShellStateCommand().MutableState().shouldApplyInitialLayout = false;

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            m_PendingFocusTarget = FocusTarget::None;
            m_PendingFocusPassesRemaining = 0;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::DockBuilderRemoveNode(dockspaceId);
            ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace | dockspaceFlags);
            ImGui::DockBuilderSetNodePos(dockspaceId, viewport->WorkPos);
            ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

            if (m_PendingLayoutPreset == LayoutPreset::DefaultFloating) {
                ImGuiID dockMainId = dockspaceId;
                ImGuiID dockLeftId = ImGui::DockBuilderSplitNode(
                    dockMainId,
                    ImGuiDir_Left,
                    0.30f,
                    nullptr,
                    &dockMainId);
                ImGui::DockBuilderDockWindow(shell::presentation::ViewerWindowName(), dockMainId);
                dockLeftPanelWindows(dockLeftId);
                if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockLeftId)) {
                    node->SelectedTabId = ImGui::GetID(shell::presentation::ModelTreeWindowName());
                }

                applyLayoutVisibilityPreset(LayoutPreset::DefaultFloating);
            } else if (m_PendingLayoutPreset == LayoutPreset::DockRight) {
                applyLayoutVisibilityPreset(LayoutPreset::DockRight);

                ImGuiID dockMainId = dockspaceId;
                ImGuiID dockLeftId = ImGui::DockBuilderSplitNode(
                    dockMainId,
                    ImGuiDir_Left,
                    0.30f,
                    nullptr,
                    &dockMainId);
                ImGuiID dockRightId = ImGui::DockBuilderSplitNode(
                    dockMainId,
                    ImGuiDir_Right,
                    15.0f / 70.0f,
                    nullptr,
                    &dockMainId);
                ImGui::DockBuilderDockWindow(shell::presentation::ViewerWindowName(), dockMainId);

                dockLeftPanelWindows(dockLeftId);
                if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockLeftId)) {
                    node->SelectedTabId = ImGui::GetID(shell::presentation::ModelTreeWindowName());
                }

                ImGui::DockBuilderDockWindow(shell::presentation::ChargeDensityViewerWindowName(), dockRightId);
                ImGuiID dockRightBottomId = ImGui::DockBuilderSplitNode(
                    dockRightId,
                    ImGuiDir_Down,
                    0.50f,
                    nullptr,
                    &dockRightId);
                ImGui::DockBuilderDockWindow(shell::presentation::SliceViewerWindowName(), dockRightBottomId);
                if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockRightId)) {
                    node->SelectedTabId = ImGui::GetID(shell::presentation::ChargeDensityViewerWindowName());
                }
            } else if (m_PendingLayoutPreset == LayoutPreset::DockBottom) {
                applyLayoutVisibilityPreset(LayoutPreset::DockBottom);

                ImGuiID dockMainId = dockspaceId;
                ImGuiID dockBottomId = ImGui::DockBuilderSplitNode(
                    dockMainId,
                    ImGuiDir_Down,
                    0.40f,
                    nullptr,
                    &dockMainId);
                ImGui::DockBuilderDockWindow(shell::presentation::ViewerWindowName(), dockMainId);
                dockLeftPanelWindows(dockBottomId);
                if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockBottomId)) {
                    node->SelectedTabId = ImGui::GetID(shell::presentation::ModelTreeWindowName());
                }
            } else if (m_PendingLayoutPreset == LayoutPreset::ResetDocking) {
                applyLayoutVisibilityPreset(LayoutPreset::ResetDocking);
                m_ResetWindowGeometryPassesRemaining = 2;
            }

            ImGui::DockBuilderFinish(dockspaceId);
            ImGui::MarkIniSettingsDirty();
        }

        m_PendingLayoutPreset = LayoutPreset::None;
        GetWorkbenchRuntime().ShellStateCommand().MutableState().pendingLayoutPreset =
            shell::domain::ShellLayoutPreset::None;
    }

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
    }

    if (ImGui::BeginMenuBar()) {
        shell::application::WorkbenchController& controller = GetWorkbenchRuntime().ShellController();

        auto openEditorPanel = [&](shell::application::EditorPanelAction action) {
            controller.OpenEditorPanel(action);
            syncShellStateFromStore();
        };
        auto openBuilderPanel = [&](shell::application::BuilderPanelAction action) {
            controller.OpenBuilderPanel(action);
            syncShellStateFromStore();
        };
        auto openDataPanel = [&](shell::application::DataPanelAction action) {
            controller.OpenDataPanel(action);
            syncShellStateFromStore();
        };

        if (ImGui::BeginMenu("  Crystal Viewer")) {
            if (ImGui::MenuItem(ICON_FA6_CIRCLE_INFO "  About")) {
                m_bShowAboutPopup = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("  File")) {
            if (ImGui::MenuItem("Open Structure File")) {
                controller.RequestOpenStructureImport();
            }
            AddTooltip("Open Structure File", "Import XSF, XSF(Grid), vasp CHGCAR");

            ImGui::BeginDisabled();
            ImGui::MenuItem("Open Recent");
            ImGui::EndDisabled();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("  Edit")) {
            if (ImGui::MenuItem("Atoms")) {
                openEditorPanel(shell::application::EditorPanelAction::Atoms);
            }
            if (ImGui::MenuItem("Bonds")) {
                openEditorPanel(shell::application::EditorPanelAction::Bonds);
            }
            if (ImGui::MenuItem("Cell")) {
                openEditorPanel(shell::application::EditorPanelAction::Cell);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("  Build")) {
            if (ImGui::MenuItem("Add atoms")) {
                openBuilderPanel(shell::application::BuilderPanelAction::AddAtoms);
            }
            if (ImGui::MenuItem("Bravais Lattice Templates")) {
                openBuilderPanel(shell::application::BuilderPanelAction::BravaisLatticeTemplates);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("  Measurement")) {
            const bool isDistanceMode =
                controller.IsMeasurementModeActive(measurement::application::MeasurementMode::Distance);
            const bool isAngleMode =
                controller.IsMeasurementModeActive(measurement::application::MeasurementMode::Angle);
            const bool isDihedralMode =
                controller.IsMeasurementModeActive(measurement::application::MeasurementMode::Dihedral);
            const bool isGeometricCenterMode =
                controller.IsMeasurementModeActive(measurement::application::MeasurementMode::GeometricCenter);
            const bool isCenterOfMassMode =
                controller.IsMeasurementModeActive(measurement::application::MeasurementMode::CenterOfMass);

            if (ImGui::MenuItem("Distance", nullptr, isDistanceMode)) {
                controller.EnterMeasurementMode(measurement::application::MeasurementMode::Distance);
            }
            if (ImGui::MenuItem("Angle", nullptr, isAngleMode)) {
                controller.EnterMeasurementMode(measurement::application::MeasurementMode::Angle);
            }
            if (ImGui::MenuItem("Dihedral", nullptr, isDihedralMode)) {
                controller.EnterMeasurementMode(measurement::application::MeasurementMode::Dihedral);
            }
            if (ImGui::MenuItem("Geometric Center", nullptr, isGeometricCenterMode)) {
                controller.EnterMeasurementMode(measurement::application::MeasurementMode::GeometricCenter);
            }
            if (ImGui::MenuItem("Center of Mass", nullptr, isCenterOfMassMode)) {
                controller.EnterMeasurementMode(measurement::application::MeasurementMode::CenterOfMass);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("  Data")) {
            if (ImGui::MenuItem("Isosurface")) {
                openDataPanel(shell::application::DataPanelAction::Isosurface);
            }
            if (ImGui::MenuItem("Surface")) {
                openDataPanel(shell::application::DataPanelAction::Surface);
            }
            if (ImGui::MenuItem("Volumetric")) {
                openDataPanel(shell::application::DataPanelAction::Volumetric);
            }
            if (ImGui::MenuItem("Plane")) {
                openDataPanel(shell::application::DataPanelAction::Plane);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("  Utilities")) {
            if (ImGui::MenuItem("Brillouin Zone")) {
                openBuilderPanel(shell::application::BuilderPanelAction::BrillouinZone);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA6_GEAR "  Settings")) {
            bool nodeInfoEnabled = controller.IsNodeInfoEnabled();
            bool viewerFpsOverlayEnabled = controller.IsViewerFpsOverlayEnabled();

            if (ImGui::MenuItem("Node Tooltip", nullptr, &nodeInfoEnabled)) {
                controller.SetNodeInfoEnabled(nodeInfoEnabled);
            }
            if (ImGui::MenuItem("Viewer FPS Overlay", nullptr, &viewerFpsOverlayEnabled)) {
                controller.SetViewerFpsOverlayEnabled(viewerFpsOverlayEnabled);
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Style")) {
                if (ImGui::MenuItem("Dark", nullptr, GetColorStyle() == ColorStyle::Dark)) {
                    SetColorStyle(ColorStyle::Dark);
                }
                if (ImGui::MenuItem("Light", nullptr, GetColorStyle() == ColorStyle::Light)) {
                    SetColorStyle(ColorStyle::Light);
                }
                if (ImGui::MenuItem("Classic", nullptr, GetColorStyle() == ColorStyle::Classic)) {
                    SetColorStyle(ColorStyle::Classic);
                }
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Background Color", nullptr, &m_bShowBgColorPopup);
            if (ImGui::BeginMenu("Font Size")) {
                if (ImGui::MenuItem("small", nullptr, GetFontSizePreset() == FontSizePreset::Small)) {
                    SetFontSizePreset(FontSizePreset::Small);
                }
                if (ImGui::MenuItem("medium", nullptr, GetFontSizePreset() == FontSizePreset::Medium)) {
                    SetFontSizePreset(FontSizePreset::Medium);
                }
                if (ImGui::MenuItem("large", nullptr, GetFontSizePreset() == FontSizePreset::Large)) {
                    SetFontSizePreset(FontSizePreset::Large);
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem(ICON_FA6_EXPAND "  Full Screen")) {
                EM_ASM({
                    const canvas = document.getElementById("canvas");
                    canvas.requestFullscreen();
                });
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA6_WINDOW_RESTORE "  Windows")) {
            ImGui::MenuItem(ICON_FA6_DISPLAY "  Viewer", nullptr, &m_bShowVtkViewer);
            ImGui::MenuItem(ICON_FA6_FOLDER_TREE "  Model Tree", nullptr, &m_bShowModelTree);
            ImGui::Separator();
            ImGui::MenuItem("Created Atoms", nullptr, &m_bShowCreatedAtomsWindow);
            ImGui::MenuItem("Bonds Management", nullptr, &m_bShowBondsManagementWindow);
            ImGui::MenuItem("Cell Information", nullptr, &m_bShowCellInformationWindow);
            ImGui::Separator();
            ImGui::MenuItem("Periodic Table", nullptr, &m_bShowPeriodicTableWindow);
            ImGui::MenuItem("Crystal Templates", nullptr, &m_bShowCrystalTemplatesWindow);
            ImGui::MenuItem("Brillouin Zone Plot", nullptr, &m_bShowBrillouinZonePlotWindow);
            ImGui::Separator();
            ImGui::MenuItem("Charge Density Viewer", nullptr, &m_bShowChargeDensityViewerWindow);
            ImGui::MenuItem("2D Slice Viewer", nullptr, &m_bShowSliceViewerWindow);
#ifdef DEBUG_BUILD
            ImGui::MenuItem("Full Dockspace", nullptr, &m_bFullDockSpace);
#endif
#ifdef SHOW_FONT_ICONS
            ImGui::MenuItem("Show Font Icons", nullptr, &m_bShowFontIcons);
#endif
            ImGui::EndMenu();
        }

        ImGui::Separator();
        if (ImGui::SmallButton("Layout 1")) {
            controller.RequestLayoutPreset(shell::domain::ShellLayoutPreset::DefaultFloating);
        }
        AddTooltip("Layout 1", "Viewer window uses the default floating placement.");
        ImGui::SameLine();
        if (ImGui::SmallButton("Layout 2")) {
            controller.RequestLayoutPreset(shell::domain::ShellLayoutPreset::DockRight);
        }
        AddTooltip(
            "Layout 2",
            "Left 30%: Model Tree/tool windows, Right 15%: Charge Density(top)+Plane(bottom), Center: Viewer.");
        ImGui::SameLine();
        if (ImGui::SmallButton("Layout 3")) {
            controller.RequestLayoutPreset(shell::domain::ShellLayoutPreset::DockBottom);
        }
        AddTooltip("Layout 3", "Dock Viewer (60%) top -- Model Tree and split tool windows (40%) bottom.");
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset")) {
            controller.RequestLayoutPreset(shell::domain::ShellLayoutPreset::ResetDocking);
        }
        AddTooltip("Reset", "Reset docking and restore default window geometry.");
        ImGui::EndMenuBar();
    }

    syncShellStateToStore();
    ImGui::End();
}
