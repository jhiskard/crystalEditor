#include "app.h"
#include "shell/runtime/workbench_runtime.h"
#include "shell/application/workbench_controller.h"
#include "shell/application/shell_state_query_service.h"
#include "shell/application/shell_state_command_service.h"
#include "shell/presentation/font/font_registry.h"
#include "render/presentation/viewer_window.h"
#include "mesh/presentation/model_tree_panel.h"
#include "mesh/presentation/mesh_detail_panel.h"
#include "mesh/application/mesh_query_service.h"
#include "mesh/presentation/mesh_group_detail_panel.h"
#include "mesh/domain/mesh_entity.h"
#include "density/application/density_service.h"
#include "structure/application/structure_lifecycle_service.h"

// Legacy atoms facade
#include "workspace/legacy/legacy_atoms_runtime.h"

// ImGui
#include <imgui.h>
#include <imgui_internal.h>

// Standard library
#include <fstream>

// Emscripten
#include <emscripten/emscripten.h>

// Third-party libraries
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace {
const char* kViewerWindowName = ICON_FA6_DISPLAY"  Viewer";
const char* kModelTreeWindowName = ICON_FA6_FOLDER_TREE"  Model Tree";
const char* kPeriodicTableWindowName = "Periodic Table";
const char* kCrystalTemplatesWindowName = "Crystal Templates";
const char* kBrillouinZonePlotWindowName = "Brillouin Zone Plot";
const char* kCreatedAtomsWindowName = "Created Atoms";
const char* kBondsManagementWindowName = "Bonds Management";
const char* kCellInformationWindowName = "Cell Information";
const char* kChargeDensityViewerWindowName = "Charge Density Viewer";
const char* kSliceViewerWindowName = "2D Slice Viewer";

bool parseVisibilityValue(const std::string& value, bool& outValue) {
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

float toUiScale(App::FontSizePreset preset) {
    switch (preset) {
    case App::FontSizePreset::Small:
        return 1.0f;
    case App::FontSizePreset::Medium:
        return 1.2f;
    case App::FontSizePreset::Large:
        return 1.5f;
    }
    return 1.0f;
}

App::FontSizePreset fontSizePresetFromInt(int value) {
    switch (value) {
    case 1:
        return App::FontSizePreset::Medium;
    case 2:
        return App::FontSizePreset::Large;
    default:
        return App::FontSizePreset::Small;
    }
}

struct ResetWindowLayout {
    ImVec2 viewerPos;
    ImVec2 viewerSize;
    ImVec2 modelTreePos;
    ImVec2 advancedViewPos;
    ImVec2 crystalBuilderPos;
    ImVec2 crystalEditorPos;
    ImVec2 panelSize;
};

ResetWindowLayout buildResetWindowLayout(const ImGuiViewport* viewport) {
    const ImVec2 workPos = viewport->WorkPos;
    const ImVec2 workSize = viewport->WorkSize;

    const ImVec2 viewerSize(workSize.x * 0.5f, workSize.y * 0.5f);
    const ImVec2 viewerPos(
        workPos.x + (workSize.x - viewerSize.x) * 0.5f,
        workPos.y + (workSize.y - viewerSize.y) * 0.5f
    );

    const ImVec2 panelSize(workSize.x * 0.3f, workSize.y * 0.5f);
    const float inset5x = workSize.x * 0.05f;
    const float inset5y = workSize.y * 0.05f;
    const float inset10x = workSize.x * 0.10f;
    const float inset10y = workSize.y * 0.10f;

    ResetWindowLayout layout {};
    layout.viewerPos = viewerPos;
    layout.viewerSize = viewerSize;
    layout.panelSize = panelSize;
    layout.modelTreePos = ImVec2(
        workPos.x + workSize.x - panelSize.x - inset5x,
        workPos.y + inset5y
    );
    layout.advancedViewPos = ImVec2(
        workPos.x + workSize.x - panelSize.x - inset10x,
        workPos.y + inset10y
    );
    layout.crystalBuilderPos = ImVec2(workPos.x + inset5x, workPos.y + inset5y);
    layout.crystalEditorPos = ImVec2(workPos.x + inset10x, workPos.y + inset10y);
    return layout;
}
}


App::App() {
    initLogger();
}

App::~App() {
    shutdownLogger();
}

void App::initLogger(bool useConsole, bool newFile) {
    if (useConsole) {
        auto console = spdlog::stdout_color_mt("console");
        spdlog::set_default_logger(console);
    }
    else {
        auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/vtk-workbench.log", newFile);
        spdlog::set_default_logger(file_logger);
    }
#ifdef DEBUG_BUILD
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
#else
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%^%l%$] %v");
#endif
    SPDLOG_DEBUG("Logger initialized");
}

void App::shutdownLogger() {
    spdlog::shutdown();
    SPDLOG_DEBUG("Logger shutdown");
}

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

void App::InitIdbfs() {
    EM_ASM({
        const path = UTF8ToString($0);
        VtkModule.FS.mkdir(path);
        VtkModule.FS.mount(IDBFS, {}, path); 
        console.log(`IDBFS mounted on "${path}"`);
    }, GetWorkbenchRuntime().AppController().IDBFS_MOUNT_PATH);
}

void App::SaveImGuiIniFile() {
    GetWorkbenchRuntime().AppController().syncShellStateFromStore();

    // Get ImGui setting data
    size_t dataSize = 0;
    const char* data = ImGui::SaveIniSettingsToMemory(&dataSize);

    std::ofstream iniFile(GetWorkbenchRuntime().AppController().IMGUI_SETTING_FILE_PATH, std::ios::out | std::ios::trunc);
    if (!iniFile) {
        SPDLOG_ERROR("Failed to open ImGui setting file for writing: {}",
            GetWorkbenchRuntime().AppController().IMGUI_SETTING_FILE_PATH);
        return;
    }
    iniFile.write(data, dataSize);
    iniFile.close();

    std::ofstream windowFile(GetWorkbenchRuntime().AppController().WINDOW_SETTING_FILE_PATH, std::ios::out | std::ios::trunc);
    if (!windowFile) {
        SPDLOG_ERROR("Failed to open window visibility file for writing: {}",
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
    windowFile << "viewer=" << (GetWorkbenchRuntime().AppController().m_bShowVtkViewer ? 1 : 0) << "\n";
    windowFile << "model_tree=" << (GetWorkbenchRuntime().AppController().m_bShowModelTree ? 1 : 0) << "\n";
    windowFile << "crystal_builder=" << (anyBuilderWindow ? 1 : 0) << "\n";
    windowFile << "crystal_editor=" << (anyEditorWindow ? 1 : 0) << "\n";
    windowFile << "advanced_view=" << (anyDataWindow ? 1 : 0) << "\n";

    // Split-window keys.
    windowFile << "periodic_table_window=" << (GetWorkbenchRuntime().AppController().m_bShowPeriodicTableWindow ? 1 : 0) << "\n";
    windowFile << "crystal_templates_window=" << (GetWorkbenchRuntime().AppController().m_bShowCrystalTemplatesWindow ? 1 : 0) << "\n";
    windowFile << "brillouin_zone_plot_window=" << (GetWorkbenchRuntime().AppController().m_bShowBrillouinZonePlotWindow ? 1 : 0) << "\n";
    windowFile << "created_atoms_window=" << (GetWorkbenchRuntime().AppController().m_bShowCreatedAtomsWindow ? 1 : 0) << "\n";
    windowFile << "bonds_management_window=" << (GetWorkbenchRuntime().AppController().m_bShowBondsManagementWindow ? 1 : 0) << "\n";
    windowFile << "cell_information_window=" << (GetWorkbenchRuntime().AppController().m_bShowCellInformationWindow ? 1 : 0) << "\n";
    windowFile << "charge_density_viewer_window=" << (GetWorkbenchRuntime().AppController().m_bShowChargeDensityViewerWindow ? 1 : 0) << "\n";
    windowFile << "slice_viewer_window=" << (GetWorkbenchRuntime().AppController().m_bShowSliceViewerWindow ? 1 : 0) << "\n";
    windowFile.close();
}

void App::LoadImGuiIniFile() {
    std::ifstream iniFile(GetWorkbenchRuntime().AppController().IMGUI_SETTING_FILE_PATH, std::ifstream::in);
    if (!iniFile) {
        SPDLOG_INFO("ImGui setting file not found. Using default layout.");
    }
    else {
        std::ostringstream ss;
        ss << iniFile.rdbuf();
        iniFile.close();
        std::string fileContents = ss.str();

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
        if (!parseVisibilityValue(value, parsedValue)) {
            continue;
        }

        if (key == "viewer") {
            GetWorkbenchRuntime().AppController().m_bShowVtkViewer = parsedValue;
        }
        else if (key == "model_tree") {
            GetWorkbenchRuntime().AppController().m_bShowModelTree = parsedValue;
        }
        else if (key == "crystal_builder") {
            GetWorkbenchRuntime().AppController().m_bShowPeriodicTableWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowCrystalTemplatesWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowBrillouinZonePlotWindow = parsedValue;
        }
        else if (key == "crystal_editor") {
            GetWorkbenchRuntime().AppController().m_bShowCreatedAtomsWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowBondsManagementWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowCellInformationWindow = parsedValue;
        }
        else if (key == "advanced_view") {
            GetWorkbenchRuntime().AppController().m_bShowChargeDensityViewerWindow = parsedValue;
            GetWorkbenchRuntime().AppController().m_bShowSliceViewerWindow = parsedValue;
        }
        else if (key == "periodic_table_window") {
            GetWorkbenchRuntime().AppController().m_bShowPeriodicTableWindow = parsedValue;
        }
        else if (key == "crystal_templates_window") {
            GetWorkbenchRuntime().AppController().m_bShowCrystalTemplatesWindow = parsedValue;
        }
        else if (key == "brillouin_zone_plot_window") {
            GetWorkbenchRuntime().AppController().m_bShowBrillouinZonePlotWindow = parsedValue;
        }
        else if (key == "created_atoms_window") {
            GetWorkbenchRuntime().AppController().m_bShowCreatedAtomsWindow = parsedValue;
        }
        else if (key == "bonds_management_window") {
            GetWorkbenchRuntime().AppController().m_bShowBondsManagementWindow = parsedValue;
        }
        else if (key == "cell_information_window") {
            GetWorkbenchRuntime().AppController().m_bShowCellInformationWindow = parsedValue;
        }
        else if (key == "charge_density_viewer_window") {
            GetWorkbenchRuntime().AppController().m_bShowChargeDensityViewerWindow = parsedValue;
        }
        else if (key == "slice_viewer_window") {
            GetWorkbenchRuntime().AppController().m_bShowSliceViewerWindow = parsedValue;
        }
    }

    if (!GetWorkbenchRuntime().AppController().m_bShowModelTree) {
        GetWorkbenchRuntime().AppController().m_RequestModelTreeFocus = false;
    }

    GetWorkbenchRuntime().AppController().syncShellStateToStore();
}

void App::SetColorStyle(ColorStyle style) {
    GetWorkbenchRuntime().AppController().setColorStyle(style);
}

ColorStyle App::GetColorStyle() {
    return GetWorkbenchRuntime().AppController().getColorStyle();
}

void App::SetFontSizePreset(FontSizePreset preset) {
    GetWorkbenchRuntime().AppController().setFontSizePreset(preset);
}

App::FontSizePreset App::GetFontSizePreset() {
    return GetWorkbenchRuntime().AppController().getFontSizePreset();
}

float App::UiScale() {
    return GetWorkbenchRuntime().AppController().m_kUiScale;
}

void App::setColorStyle(ColorStyle style) {
    m_ColorStyle = style;

    switch (style) {
    case ColorStyle::Dark:
        ImGui::StyleColorsDark();
        break;
    case ColorStyle::Light:
        ImGui::StyleColorsLight();
        break;
    case ColorStyle::Classic:
        ImGui::StyleColorsClassic();
        break;
    }

    // Save viewer style on local storage
    EM_ASM({
        const styleKey = UTF8ToString($0);
        const style = $1;
        localStorage.setItem(styleKey, style);
    }, IMGUI_STYLE_KEY, static_cast<int>(m_ColorStyle));
}

void App::setFontSizePreset(FontSizePreset preset) {
    m_FontSizePreset = preset;
    m_kUiScale = toUiScale(preset);
    applyFontScale();

    // Save font size preset on local storage.
    EM_ASM({
        const fontSizeKey = UTF8ToString($0);
        const fontSizePreset = $1;
        localStorage.setItem(fontSizeKey, fontSizePreset);
    }, IMGUI_FONT_SIZE_KEY, static_cast<int>(m_FontSizePreset));
}

void App::applyFontScale() {
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = m_kUiScale;
}

void App::SetDetailedStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Sizes
    style.WindowPadding = ImVec2(6.0f, 6.0f);
    style.FramePadding = ImVec2(5.0f, 3.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 8.0f;
    style.ScrollbarSize = 9.0f;
    style.GrabMinSize = 8.0f;

    // Borders
    style.FrameBorderSize = 0.5f;
    style.TabBorderSize = 1.0f;

    // Rounding
    style.WindowRounding = 0.0f;
    style.FrameRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;

    // Tables
    style.CellPadding = ImVec2(6.0f, 3.0f);

    // Colors
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    style.Colors[ImGuiCol_Button].w = 0.2f;
}

void App::SetInitColorStyle() {
    int colorStyle = EM_ASM_INT({
        const styleKey = UTF8ToString($0);
        const style = localStorage.getItem(styleKey);
        if (style) {
            return style;
        }
        else {         // If not found in local storage, set default style.
            return 0;  // 0 is dark style.
        }
    }, IMGUI_STYLE_KEY);

    ColorStyle initColorStyle = static_cast<ColorStyle>(colorStyle);
    setColorStyle(initColorStyle);
}

void App::SetInitFontSize() {
    int fontSizePreset = EM_ASM_INT({
        const fontSizeKey = UTF8ToString($0);
        const fontSize = localStorage.getItem(fontSizeKey);
        if (fontSize) {
            const parsed = parseInt(fontSize, 10);
            return Number.isNaN(parsed) ? 0 : parsed;
        }
        else {
            return 0;  // 0 is small size.
        }
    }, IMGUI_FONT_SIZE_KEY);

    setFontSizePreset(fontSizePresetFromInt(fontSizePreset));
}

void App::Render() {
    renderDockSpaceAndMenu();
    renderImGuiWindows();
}

void App::renderDockSpaceAndMenu() {
    syncShellStateFromStore();

    static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
    FontManager& fontManager = GetWorkbenchRuntime().FontRegistry();

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (m_bFullDockSpace) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspaceFlags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar();

    if (m_bFullDockSpace)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    ImGuiID dockspace_id = ImGui::GetID("DockSpace");


    auto dockLeftPanelWindows = [&](ImGuiID dockNodeId) {
        ImGui::DockBuilderDockWindow(kModelTreeWindowName, dockNodeId);
        ImGui::DockBuilderDockWindow(kCreatedAtomsWindowName, dockNodeId);
        ImGui::DockBuilderDockWindow(kBondsManagementWindowName, dockNodeId);
        ImGui::DockBuilderDockWindow(kCellInformationWindowName, dockNodeId);
        ImGui::DockBuilderDockWindow(kPeriodicTableWindowName, dockNodeId);
        ImGui::DockBuilderDockWindow(kCrystalTemplatesWindowName, dockNodeId);
        ImGui::DockBuilderDockWindow(kBrillouinZonePlotWindowName, dockNodeId);
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

        // Keep App mirror and shell store in sync within the same frame.
        syncShellStateFromStore();
    };

    if (m_ShouldApplyInitialLayout && m_PendingLayoutPreset == LayoutPreset::None) {
        m_PendingLayoutPreset = LayoutPreset::DefaultFloating;
        m_ShouldApplyInitialLayout = false;
    }

    if (m_PendingLayoutPreset != LayoutPreset::None) {
        // Any explicit layout request consumes the one-time bootstrap layout signal.
        m_ShouldApplyInitialLayout = false;
        GetWorkbenchRuntime().ShellStateCommand().MutableState().shouldApplyInitialLayout = false;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            // 레이아웃 전환 시 기존 메뉴 기반 포커스 요청은 초기화한다.
            m_PendingFocusTarget = FocusTarget::None;
            m_PendingFocusPassesRemaining = 0;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | dockspaceFlags);
            ImGui::DockBuilderSetNodePos(dockspace_id, viewport->WorkPos);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

            if (m_PendingLayoutPreset == LayoutPreset::DefaultFloating) {
                // Layout 1: Viewer 중심 + 좌측 30% 도킹 패널(기본은 숨김)
                ImGuiID dock_main_id = dockspace_id;
                ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(
                    dock_main_id, ImGuiDir_Left, 0.30f, nullptr, &dock_main_id);
                ImGui::DockBuilderDockWindow(kViewerWindowName, dock_main_id);
                dockLeftPanelWindows(dock_left_id);
                if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_left_id)) {
                    node->SelectedTabId = ImGui::GetID(kModelTreeWindowName);
                }

                applyLayoutVisibilityPreset(LayoutPreset::DefaultFloating);
            }
            else if (m_PendingLayoutPreset == LayoutPreset::DockRight) {
                // Layout 2:
                // - Left 30%: Model Tree + Builder/Editor windows (tab stack)
                // - Right 15% (top): Charge Density Viewer
                // - Right 15% (bottom): 2D Slice Viewer
                // - Center: Viewer
                applyLayoutVisibilityPreset(LayoutPreset::DockRight);

                ImGuiID dock_main_id = dockspace_id;
                ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(
                    dock_main_id, ImGuiDir_Left, 0.30f, nullptr, &dock_main_id);
                // Remaining width is 70%. To allocate 15% of total width on the right:
                // ratio_on_remaining = 15 / 70.
                ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(
                    dock_main_id, ImGuiDir_Right, 15.0f / 70.0f, nullptr, &dock_main_id);
                ImGui::DockBuilderDockWindow(kViewerWindowName, dock_main_id);

                dockLeftPanelWindows(dock_left_id);
                if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_left_id)) {
                    node->SelectedTabId = ImGui::GetID(kModelTreeWindowName);
                }

                // Dock Charge Density Viewer first on the right.
                ImGui::DockBuilderDockWindow(kChargeDensityViewerWindowName, dock_right_id);
                // Then split right area into top/bottom and dock Plane(2D Slice Viewer) to bottom.
                ImGuiID dock_right_bottom_id = ImGui::DockBuilderSplitNode(
                    dock_right_id, ImGuiDir_Down, 0.50f, nullptr, &dock_right_id);
                ImGui::DockBuilderDockWindow(kSliceViewerWindowName, dock_right_bottom_id);
                if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_right_id)) {
                    node->SelectedTabId = ImGui::GetID(kChargeDensityViewerWindowName);
                }
            }
            else if (m_PendingLayoutPreset == LayoutPreset::DockBottom) {
                // Layout 3: Viewer 60% 위, 좌측 패널 창 40% 아래
                applyLayoutVisibilityPreset(LayoutPreset::DockBottom);

                ImGuiID dock_main_id = dockspace_id;
                ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(
                    dock_main_id, ImGuiDir_Down, 0.40f, nullptr, &dock_main_id);
                ImGui::DockBuilderDockWindow(kViewerWindowName, dock_main_id);
                dockLeftPanelWindows(dock_bottom_id);
                if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_bottom_id)) {
                    node->SelectedTabId = ImGui::GetID(kModelTreeWindowName);
                }
            }
            else if (m_PendingLayoutPreset == LayoutPreset::ResetDocking) {
                // Reset: 모든 창 도킹 상태 초기화 + 창 기본 배치 적용.
                // DockBuilderRemoveNode/AddNode 이후 추가 도킹을 수행하지 않으면
                // 기존 창은 도킹 해제되어 floating 상태로 복원된다.
                applyLayoutVisibilityPreset(LayoutPreset::ResetDocking);
                m_ResetWindowGeometryPassesRemaining = 2;
            }
            ImGui::DockBuilderFinish(dockspace_id);
            ImGui::MarkIniSettingsDirty();
        }

        m_PendingLayoutPreset = LayoutPreset::None;
        // Consume one-shot layout request in the shell store after applying once.
        GetWorkbenchRuntime().ShellStateCommand().MutableState().pendingLayoutPreset =
            shell::domain::ShellLayoutPreset::None;
    }

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
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
                openBuilderPanel(
                    shell::application::BuilderPanelAction::BravaisLatticeTemplates);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("  Measurement")) {
            const bool isDistanceMode = controller.IsMeasurementModeActive(
                measurement::application::MeasurementMode::Distance);
            const bool isAngleMode = controller.IsMeasurementModeActive(
                measurement::application::MeasurementMode::Angle);
            const bool isDihedralMode = controller.IsMeasurementModeActive(
                measurement::application::MeasurementMode::Dihedral);
            const bool isGeometricCenterMode = controller.IsMeasurementModeActive(
                measurement::application::MeasurementMode::GeometricCenter);
            const bool isCenterOfMassMode = controller.IsMeasurementModeActive(
                measurement::application::MeasurementMode::CenterOfMass);
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
                controller.EnterMeasurementMode(
                    measurement::application::MeasurementMode::GeometricCenter);
            }
            if (ImGui::MenuItem("Center of Mass", nullptr, isCenterOfMassMode)) {
                controller.EnterMeasurementMode(
                    measurement::application::MeasurementMode::CenterOfMass);
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

        bool openSettings = ImGui::BeginMenu(ICON_FA6_GEAR"  Settings");
        if (openSettings) {
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
            if (ImGui::MenuItem(ICON_FA6_EXPAND"  Full Screen")) {
                EM_ASM({
                    const canvas = document.getElementById("canvas");
                    canvas.requestFullscreen();
                });
            }
            ImGui::EndMenu();
        }
        bool openWindows = ImGui::BeginMenu(ICON_FA6_WINDOW_RESTORE"  Windows");
        if (openWindows) {
            ImGui::MenuItem(ICON_FA6_DISPLAY"  Viewer", nullptr, &m_bShowVtkViewer);
            ImGui::MenuItem(ICON_FA6_FOLDER_TREE"  Model Tree", nullptr, &m_bShowModelTree);
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
        //#ifdef SHOW_IMGUI_DEMO
        //    ImGui::MenuItem("Show Demo Window", nullptr, &m_bShowDemoWindow);
        //#endif
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
        AddTooltip("Layout 2", "Left 30%: Model Tree/tool windows, Right 15%: Charge Density(top)+Plane(bottom), Center: Viewer.");
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

void App::InitImGuiWindows() {
    // Keep panel/materialization warm-up in the runtime composition root path.
    (void)GetWorkbenchRuntime().Viewer();
    (void)GetWorkbenchRuntime().TestWindowPanel();
    (void)GetWorkbenchRuntime().ModelTreePanel();
    (void)GetWorkbenchRuntime().MeshDetailPanel();
    (void)GetWorkbenchRuntime().MeshGroupDetailPanel();
    (void)workspace::legacy::LegacyAtomsRuntime();
}

void App::renderAboutPopup() {
    ImGui::OpenPopup("About Crystal Viewer");
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 280), ImGuiCond_Appearing);
    
    if (ImGui::BeginPopupModal("About Crystal Viewer", &m_bShowAboutPopup, 
                               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        
        ImGui::Dummy(ImVec2(0, 25));
        
        float windowWidth = ImGui::GetWindowSize().x;
        
        // 타이틀 (큰 글씨 - 스케일 사용)
        const char* title = ICON_FA6_ATOM "  Crystal Viewer";
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        float originalScale = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale = 1.8f;
        ImGui::PushFont(ImGui::GetFont());
        
        float titleWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
        ImGui::Text("%s", title);
        
        ImGui::GetFont()->Scale = originalScale;
        ImGui::PopFont();
        ImGui::PopStyleColor();
        
        // 버전 정보
        ImGui::Dummy(ImVec2(0, 8));
        const char* version = "Version 1.0";
        float versionWidth = ImGui::CalcTextSize(version).x;
        ImGui::SetCursorPosX((windowWidth - versionWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", version);
        
        // 구분선
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 20));
        
        // 저작권 정보
        const char* copyright = "Copyright (c) 2025 by KISTI";
        float copyrightWidth = ImGui::CalcTextSize(copyright).x;
        ImGui::SetCursorPosX((windowWidth - copyrightWidth) * 0.5f);
        ImGui::Text("%s", copyright);
        
        // 기관명
        ImGui::Dummy(ImVec2(0, 5));
        const char* subtitle = "Korea Institute of Science and Technology Information";
        float subtitleWidth = ImGui::CalcTextSize(subtitle).x;
        ImGui::SetCursorPosX((windowWidth - subtitleWidth) * 0.5f);
        ImGui::TextDisabled("%s", subtitle);
        
        // 닫기 버튼
        ImGui::Dummy(ImVec2(0, 25));
        
        float buttonWidth = 120.0f;
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
        if (ImGui::Button("Close", ImVec2(buttonWidth, 32))) {
            m_bShowAboutPopup = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void App::renderImGuiWindows() {
    syncShellStateFromStore();

    // About 팝업 렌더링
    if (m_bShowAboutPopup) {
        renderAboutPopup();
    }

    {
        shell::domain::ShellUiState& shellState = GetWorkbenchRuntime().ShellStateCommand().MutableState();
        AtomsTemplate& atomsTemplate = workspace::legacy::LegacyAtomsRuntime();

        if (shellState.hasPendingEditorRequest) {
            atomsTemplate.RequestEditorSection(shellState.pendingEditorRequest);
            shellState.hasPendingEditorRequest = false;
        }
        if (shellState.hasPendingBuilderRequest) {
            atomsTemplate.RequestBuilderSection(shellState.pendingBuilderRequest);
            shellState.hasPendingBuilderRequest = false;
        }
        if (shellState.hasPendingDataRequest) {
            atomsTemplate.RequestDataMenu(shellState.pendingDataRequest);
            shellState.hasPendingDataRequest = false;
        }
    }

    const bool applyResetLayoutThisFrame = m_ResetWindowGeometryPassesRemaining > 0;
    bool hasResetLayout = false;
    ResetWindowLayout resetLayout {};
    if (applyResetLayoutThisFrame) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport != nullptr) {
            resetLayout = buildResetWindowLayout(viewport);
            hasResetLayout = true;

            GetWorkbenchRuntime().Viewer().RequestForcedWindowLayout(resetLayout.viewerPos, resetLayout.viewerSize);

            AtomsTemplate& atomsTemplate = workspace::legacy::LegacyAtomsRuntime();
            atomsTemplate.RequestForcedBuilderWindowLayout(resetLayout.crystalBuilderPos, resetLayout.panelSize);
            atomsTemplate.RequestForcedEditorWindowLayout(resetLayout.crystalEditorPos, resetLayout.panelSize);
            atomsTemplate.RequestForcedAdvancedWindowLayout(resetLayout.advancedViewPos, resetLayout.panelSize);
        }
    }

    if (m_bShowVtkViewer) {
        GetWorkbenchRuntime().Viewer().Render();
    }
    // if (m_bShowVtkViewer2) {
    //     GetWorkbenchRuntime().Viewer().Render();
    // }

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

    AtomsTemplate& atomsTemplate = workspace::legacy::LegacyAtomsRuntime();
    if (m_bShowPeriodicTableWindow) {
        atomsTemplate.RenderPeriodicTableWindow(&m_bShowPeriodicTableWindow);
    }
    if (m_bShowCrystalTemplatesWindow) {
        atomsTemplate.RenderCrystalTemplatesWindow(&m_bShowCrystalTemplatesWindow);
    }
    if (m_bShowBrillouinZonePlotWindow) {
        GetWorkbenchRuntime().StructureLifecycleFeature().RenderBrillouinZonePlotWindow(
            &m_bShowBrillouinZonePlotWindow);
    }
    if (m_bShowCreatedAtomsWindow) {
        atomsTemplate.RenderCreatedAtomsWindow(&m_bShowCreatedAtomsWindow);
    }
    if (m_bShowBondsManagementWindow) {
        atomsTemplate.RenderBondsManagementWindow(&m_bShowBondsManagementWindow);
    }
    if (m_bShowCellInformationWindow) {
        atomsTemplate.RenderCellInformationWindow(&m_bShowCellInformationWindow);
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
    if (mesh != nullptr) {
        if (mesh->GetMeshGroupCount() > 0) {
            GetWorkbenchRuntime().MeshGroupDetailPanel().Render(selectedMeshId);
        }
    }
    // if (m_bShowTestWindow) {
    //     GetWorkbenchRuntime().TestWindowPanel().Render(&m_bShowTestWindow);
    // }
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
        ImGui::SetWindowFocus(kModelTreeWindowName);
        m_RequestModelTreeFocus = false;
    }

    if (m_PendingFocusTarget != FocusTarget::None && m_PendingFocusPassesRemaining > 0) {
        const char* targetWindowName = nullptr;
        switch (m_PendingFocusTarget) {
        case FocusTarget::ModelTree:
            if (m_bShowModelTree) {
                targetWindowName = kModelTreeWindowName;
            }
            break;
        case FocusTarget::PeriodicTable:
            if (m_bShowPeriodicTableWindow) {
                targetWindowName = kPeriodicTableWindowName;
            }
            break;
        case FocusTarget::CrystalTemplates:
            if (m_bShowCrystalTemplatesWindow) {
                targetWindowName = kCrystalTemplatesWindowName;
            }
            break;
        case FocusTarget::BrillouinZonePlot:
            if (m_bShowBrillouinZonePlotWindow) {
                targetWindowName = kBrillouinZonePlotWindowName;
            }
            break;
        case FocusTarget::CreatedAtoms:
            if (m_bShowCreatedAtomsWindow) {
                targetWindowName = kCreatedAtomsWindowName;
            }
            break;
        case FocusTarget::BondsManagement:
            if (m_bShowBondsManagementWindow) {
                targetWindowName = kBondsManagementWindowName;
            }
            break;
        case FocusTarget::CellInformation:
            if (m_bShowCellInformationWindow) {
                targetWindowName = kCellInformationWindowName;
            }
            break;
        case FocusTarget::ChargeDensityViewer:
            if (m_bShowChargeDensityViewerWindow) {
                targetWindowName = kChargeDensityViewerWindowName;
            }
            break;
        case FocusTarget::SliceViewer:
            if (m_bShowSliceViewerWindow) {
                targetWindowName = kSliceViewerWindowName;
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

double App::DevicePixelRatio() {
    return emscripten_get_device_pixel_ratio();
}

float App::TextBaseWidth() {
    return ImGui::CalcTextSize("A").x;
}

float App::TextBaseHeight() {
    return ImGui::GetTextLineHeight();
}

float App::TextBaseHeightWithSpacing() {
    return ImGui::GetTextLineHeightWithSpacing();
}

float App::TitleBarHeight() {
    return ImGui::GetFrameHeight();
}

void App::AddTooltip(const char* label, const char* desc) {
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(label);
        if (desc) {
            ImGui::Separator();
            ImGui::TextDisabled("%s", desc);            
        }
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void App::renderProgressPopup() {
    // Force the popup to be open
    ImGui::OpenPopup(m_PopupTitle.c_str());
    
    // Center the popup
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    // Create a modal popup that blocks inputs to other windows
    if (ImGui::BeginPopupModal(m_PopupTitle.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("%s", m_PopupText.c_str());
        
        char progressText[32];
        snprintf(progressText, sizeof(progressText), "%.1f%%", m_Progress * 100.0f);
        
        // Progress bar with percentage text
        ImGui::ProgressBar(m_Progress, ImVec2(-1, 0), progressText);
        
        // Only end the popup if it's showing - don't close it here
        ImGui::EndPopup();
    }
}

void App::showProgressPopup(bool show) {
    m_bShowProgressPopup = show;
    GetWorkbenchRuntime().Viewer().SetRenderPaused(show);
    if (show) {
        m_Progress = 0.0f;  // Reset progress when showing the popup
    }
    else {
        // Set default values when hiding the popup
        m_PopupTitle = "Loading...";
        m_PopupText = "Loading...";
    }
}

void App::ShowProgressPopup(bool show) {
    GetWorkbenchRuntime().AppController().showProgressPopup(show);
}

void App::SetProgressPopupText(const std::string& title, const std::string& text) {
    GetWorkbenchRuntime().AppController().setProgressPopupText(title, text);
}

void App::ShowCrystalBuilderWindow(bool show) {
    App& app = GetWorkbenchRuntime().AppController();
    app.syncShellStateFromStore();
    app.m_ShouldApplyInitialLayout = false;
    app.m_bShowPeriodicTableWindow = show;
    app.syncShellStateToStore();
}

void App::ShowCrystalEditorWindow(bool show) {
    App& app = GetWorkbenchRuntime().AppController();
    app.syncShellStateFromStore();
    app.m_ShouldApplyInitialLayout = false;
    app.m_bShowCreatedAtomsWindow = show;
    app.syncShellStateToStore();
}

void App::ShowAdvancedViewWindow(bool show) {
    App& app = GetWorkbenchRuntime().AppController();
    app.syncShellStateFromStore();
    app.m_ShouldApplyInitialLayout = false;
    app.m_bShowChargeDensityViewerWindow = show;
    app.syncShellStateToStore();
}

void App::ShowSliceViewerWindow(bool show) {
    App& app = GetWorkbenchRuntime().AppController();
    app.syncShellStateFromStore();
    app.m_ShouldApplyInitialLayout = false;
    app.m_bShowSliceViewerWindow = show;
    app.syncShellStateToStore();
}

void App::ShowAtomsTemplateWindow(bool show) {
    ShowCrystalBuilderWindow(show);
}

void App::RequestLayout1() {
    shell::domain::ShellUiState& shellState = GetWorkbenchRuntime().ShellStateCommand().MutableState();
    shellState.pendingLayoutPreset = shell::domain::ShellLayoutPreset::DefaultFloating;
    shellState.requestModelTreeFocus = false;
    shellState.pendingFocusTarget = shell::domain::ShellFocusTarget::None;
    shellState.pendingFocusPassesRemaining = 0;
}

void App::setProgressPopupText(const std::string& title, const std::string& text) {
    m_PopupTitle = title;
    m_PopupText = text;
}
