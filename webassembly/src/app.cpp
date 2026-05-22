#include "app.h"

#include "shell/runtime/workbench_runtime.h"

#include <imgui.h>

#include <emscripten/emscripten.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

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
    } else {
        auto fileLogger = spdlog::basic_logger_mt("file_logger", "logs/vtk-workbench.log", newFile);
        spdlog::set_default_logger(fileLogger);
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

void App::InitIdbfs() {
    EM_ASM({
        const path = UTF8ToString($0);
        VtkModule.FS.mkdir(path);
        VtkModule.FS.mount(IDBFS, {}, path);
        console.log(`IDBFS mounted on "${path}"`);
    }, GetWorkbenchRuntime().AppController().IDBFS_MOUNT_PATH);
}

void App::Render() {
    renderDockSpaceAndMenu();
    renderImGuiWindows();
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
