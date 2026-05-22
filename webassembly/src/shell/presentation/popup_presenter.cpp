#include "../../app.h"

#include "icon/FontAwesome6.h"
#include "render/presentation/viewer_window.h"
#include "shell/presentation/popup_presenter.h"
#include "shell/runtime/workbench_runtime.h"

#include <imgui.h>

#include <cstdio>

void App::renderAboutPopup() {
    ImGui::OpenPopup("About Crystal Viewer");

    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 280), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(
            "About Crystal Viewer",
            &m_bShowAboutPopup,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Dummy(ImVec2(0, 25));

        const float windowWidth = ImGui::GetWindowSize().x;

        const char* title = ICON_FA6_ATOM "  Crystal Viewer";

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        const float originalScale = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale = 1.8f;
        ImGui::PushFont(ImGui::GetFont());

        const float titleWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
        ImGui::Text("%s", title);

        ImGui::GetFont()->Scale = originalScale;
        ImGui::PopFont();
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0, 8));
        const char* version = "Version 1.0";
        const float versionWidth = ImGui::CalcTextSize(version).x;
        ImGui::SetCursorPosX((windowWidth - versionWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", version);

        ImGui::Dummy(ImVec2(0, 20));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 20));

        const char* copyright = "Copyright (c) 2025 by KISTI";
        const float copyrightWidth = ImGui::CalcTextSize(copyright).x;
        ImGui::SetCursorPosX((windowWidth - copyrightWidth) * 0.5f);
        ImGui::Text("%s", copyright);

        ImGui::Dummy(ImVec2(0, 5));
        const char* subtitle = "Korea Institute of Science and Technology Information";
        const float subtitleWidth = ImGui::CalcTextSize(subtitle).x;
        ImGui::SetCursorPosX((windowWidth - subtitleWidth) * 0.5f);
        ImGui::TextDisabled("%s", subtitle);

        ImGui::Dummy(ImVec2(0, 25));

        const float buttonWidth = 120.0f;
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
        if (ImGui::Button("Close", ImVec2(buttonWidth, 32))) {
            m_bShowAboutPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void App::renderProgressPopup() {
    ImGui::OpenPopup(m_PopupTitle.c_str());

    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(
            m_PopupTitle.c_str(),
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("%s", m_PopupText.c_str());

        char progressText[32];
        std::snprintf(progressText, sizeof(progressText), "%.1f%%", m_Progress * 100.0f);
        ImGui::ProgressBar(m_Progress, ImVec2(-1, 0), progressText);
        ImGui::EndPopup();
    }
}

void App::showProgressPopup(bool show) {
    m_bShowProgressPopup = show;
    GetWorkbenchRuntime().Viewer().SetRenderPaused(show);
    if (show) {
        m_Progress = 0.0f;
    } else {
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

void App::ShowWorkspaceRuntimeModelWindow(bool show) {
    ShowCrystalBuilderWindow(show);
}

void App::setProgressPopupText(const std::string& title, const std::string& text) {
    m_PopupTitle = title;
    m_PopupText = text;
}
