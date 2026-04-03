/**
 * @file shell_state_store.h
 * @brief Central shell UI state store for panel visibility and layout requests.
 */
#pragma once

namespace shell {
namespace domain {

/**
 * @brief Identifiers for shell-managed panel visibility flags.
 */
enum class ShellWindowId {
    Viewer = 0,
    ModelTree,
    TestWindow,
    MeshDetail,
    PeriodicTable,
    CrystalTemplates,
    BrillouinZonePlot,
    CreatedAtoms,
    BondsManagement,
    CellInformation,
    ChargeDensityViewer,
    SliceViewer
};

/**
 * @brief Dock/layout presets requested from shell UI actions.
 */
enum class ShellLayoutPreset {
    None = 0,
    DefaultFloating = 1,
    DockRight = 2,
    DockBottom = 3,
    ResetDocking = 4
};

/**
 * @brief Focus targets requested from shell UI actions.
 */
enum class ShellFocusTarget {
    None = 0,
    ModelTree,
    PeriodicTable,
    CrystalTemplates,
    BrillouinZonePlot,
    CreatedAtoms,
    BondsManagement,
    CellInformation,
    ChargeDensityViewer,
    SliceViewer
};

/**
 * @brief Mutable shell state snapshot owned by the shell domain store.
 * @details This state contains only shell-level UI context (visibility/focus/layout)
 *          and intentionally excludes feature payload data.
 */
struct ShellUiState {
    bool showVtkViewer = true;
    bool showModelTree = false;
    bool showTestWindow = false;
    bool showMeshDetail = false;

    bool showPeriodicTableWindow = false;
    bool showCrystalTemplatesWindow = false;
    bool showBrillouinZonePlotWindow = false;
    bool showCreatedAtomsWindow = false;
    bool showBondsManagementWindow = false;
    bool showCellInformationWindow = false;
    bool showChargeDensityViewerWindow = false;
    bool showSliceViewerWindow = false;

    bool fullDockSpace = true;
    bool showFontIcons = true;
    bool showDemoWindow = false;

    bool requestModelTreeFocus = false;
    bool shouldApplyInitialLayout = false;
    ShellLayoutPreset pendingLayoutPreset = ShellLayoutPreset::None;
    ShellFocusTarget pendingFocusTarget = ShellFocusTarget::None;
    int pendingFocusPassesRemaining = 0;
    int resetWindowGeometryPassesRemaining = 0;
};

/**
 * @brief Single-owner store for shell UI state.
 * @details Phase 11 starts migrating shell/panel mutable state out of `App`
 *          into this store. Legacy mirror fields in `App` remain temporarily.
 */
class ShellStateStore {
public:
    /**
     * @brief Returns global shell state store instance.
     */
    static ShellStateStore& Instance();

    /**
     * @brief Resets shell state to default values.
     */
    void Reset();

    /**
     * @brief Returns immutable shell state snapshot.
     */
    const ShellUiState& State() const { return m_State; }

    /**
     * @brief Returns mutable shell state snapshot.
     */
    ShellUiState& MutableState() { return m_State; }

private:
    ShellStateStore() = default;

    ShellUiState m_State {};
};

/**
 * @brief Convenience accessor for shell state store.
 */
ShellStateStore& GetShellStateStore();

} // namespace domain
} // namespace shell
