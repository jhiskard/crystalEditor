#pragma once

#include "../../../enum/toolbar_enums.h"
#include "../../../enum/viewer_enums.h"
#include "../../../render/infrastructure/resources/texture_resource.h"

// Standard library
#include <cstdint>
#include <unordered_map>

struct ImVec2;
class WorkbenchRuntime;


/**
 * @brief Viewer toolbar presenter.
 * @note Phase 13: runtime owns this presenter instance; direct singleton entrypoint is removed.
 */
class Toolbar {
public:
    Toolbar();
    ~Toolbar();

    //void Render(const ImVec2& windowPos, const ImVec2& windowSize);
    void Render(const ImVec2& viewerContentSize);
    // ✅ 새로 추가: Charge Density 애니메이션 컨트롤
    // Getters
    MeshDisplayMode GetMeshDisplayMode() const { return m_MeshDisplayMode; }
    ProjectionMode GetProjectionMode() const { return m_ProjectionMode; }

private:
    Toolbar(const Toolbar&) = delete;
    Toolbar& operator=(const Toolbar&) = delete;
    Toolbar(Toolbar&&) = delete;
    Toolbar& operator=(Toolbar&&) = delete;

    Anchor m_Anchor { Anchor::TOP };
    int32_t m_Padding;

    MeshDisplayMode m_MeshDisplayMode { MeshDisplayMode::WIRESHADED };
    std::unordered_map<MeshDisplayMode, TextureUPtr> m_MeshDisplayIconMap;

    ProjectionMode m_ProjectionMode { ProjectionMode::PERSPECTIVE };
    std::unordered_map<ProjectionMode, TextureUPtr> m_ProjectionIconMap;

    void init();
    void renderBoundaryAtomsQuickButton();
    void renderMeshDisplayModeButtons();
    void renderProjectionModeButtons();
    void renderResetViewButton();
    void renderCellAlignButtons();
    void renderChargeDensityControls();
    void renderArrowRotationStepInput();

};
