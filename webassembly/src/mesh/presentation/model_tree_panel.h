#pragma once

// ImGui
#include <imgui.h>

// Standard library
#include <cstdint>

class TreeNode;

/**
 * @brief Model tree presenter panel.
 */
class ModelTree {
public:
    /**
     * @brief Compatibility accessor resolved via runtime-owned panel instance.
     * @note Phase 11 runtime shim. Remove this wrapper in Phase 12.
     */
    static ModelTree& Instance();

    ModelTree();
    ~ModelTree();

    void Render(bool* openWindow = nullptr);

    static int32_t GetSelectedMeshId() { return Instance().m_SelectedMeshId; }

private:
    int32_t m_DeleteMeshId = -1;
    int32_t m_SelectedMeshId = -1;

    void renderMeshTree(TreeNode* node);
    void renderMeshTable(ImGuiTableFlags tableFlags);
    void renderDeleteConfirmPopup();
    void renderClearMeasurementsConfirmPopup();

    void renderXsfStructureTable(ImGuiTableFlags tableFlags);

    int32_t m_PendingDeleteMeshId = -1;
    bool m_ShowDeleteConfirmPopup = false;
    int32_t m_PendingClearMeasurementsStructureId = -1;
    bool m_ShowClearMeasurementsConfirmPopup = false;
};
