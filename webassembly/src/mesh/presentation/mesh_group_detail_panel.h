#pragma once

// Standard library
#include <cstdint>

class MeshGroup;

/**
 * @brief Mesh-group detail presenter panel.
 */
class MeshGroupDetail {
public:
    /**
     * @brief Compatibility accessor resolved via runtime-owned panel instance.
     * @note Phase 11 runtime shim. Remove this wrapper in Phase 12.
     */
    static MeshGroupDetail& Instance();

    MeshGroupDetail();
    ~MeshGroupDetail();

    void Render(int32_t meshId);

    static int32_t GetSelectedMeshGroupId() { return Instance().m_SelectedMeshGroupId; }

    void SetUiPointSize(float size) { m_UiPointSize = size; }
    void SetUiGroupColor(const double* color);

    void SetHasPointSizeChanged(bool hasChanged) { m_HasPointSizeChanged = hasChanged; }
    void SetHasGroupColorChanged(bool hasChanged) { m_HasGroupColorChanged = hasChanged; }

private:
    int32_t m_SelectedMeshGroupId = -1;
    int32_t m_DeleteMeshGroupId = -1;

    float m_UiPointSize { 0.0f };
    float m_UiGroupColor[3] { 0.0f, 0.0f, 0.0f };

    bool m_HasPointSizeChanged = false;
    bool m_HasGroupColorChanged = false;

    void renderTableRow(const char* item, int32_t value);
    void renderTableRow(const char* item, const char* value = nullptr);
    void renderTableRow(const char* item, double value);

    void renderTableRowPointSize(MeshGroup* group);
    void renderTableRowGroupColor(MeshGroup* group);
};
