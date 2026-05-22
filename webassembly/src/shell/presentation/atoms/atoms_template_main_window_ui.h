#pragma once

#include <memory>

// 전역 WorkspaceRuntimeModel로 고정 (네임스페이스 불일치 방지)
class WorkspaceRuntimeModel;

namespace atoms::ui {

class AtomEditorUI;
class BondUI;
class CellInfoUI;
class BZPlotUI;
class PeriodicTableUI;
class BravaisLatticeUI;

/**
 * @brief WorkspaceRuntimeModel의 최상위 ImGui 창/레이아웃을 전담하는 UI 파사드.
 *
 * - ImGui::Begin/End, CollapsingHeader 등 레이아웃 관리
 * - 하위 UI 모듈(AtomEditorUI/BondUI/...) 호출
 *
 * NOTE:
 * - parent 타입을 ::WorkspaceRuntimeModel* 로 고정하여
 *   (atoms::WorkspaceRuntimeModel* vs ::WorkspaceRuntimeModel*) 네임스페이스 불일치로 인한 빌드 블로커를 제거.
 */
class WorkspaceRuntimeModelMainWindowUI {
public:
    explicit WorkspaceRuntimeModelMainWindowUI(::WorkspaceRuntimeModel* parent);
    ~WorkspaceRuntimeModelMainWindowUI();

    WorkspaceRuntimeModelMainWindowUI(const WorkspaceRuntimeModelMainWindowUI&) = delete;
    WorkspaceRuntimeModelMainWindowUI& operator=(const WorkspaceRuntimeModelMainWindowUI&) = delete;
    WorkspaceRuntimeModelMainWindowUI(WorkspaceRuntimeModelMainWindowUI&&) = delete;
    WorkspaceRuntimeModelMainWindowUI& operator=(WorkspaceRuntimeModelMainWindowUI&&) = delete;

    /// 기존 WorkspaceRuntimeModel::Render(...) 역할을 대체
    void render(bool* openWindow = nullptr);

private:
    ::WorkspaceRuntimeModel* m_parent = nullptr;

    std::unique_ptr<AtomEditorUI>     m_atomEditorUI;
    std::unique_ptr<BondUI>           m_bondUI;
    std::unique_ptr<CellInfoUI>       m_cellInfoUI;
    std::unique_ptr<BZPlotUI>         m_bzPlotUI;
    std::unique_ptr<PeriodicTableUI>  m_periodicTableUI;
    std::unique_ptr<BravaisLatticeUI> m_bravaisLatticeUI;
};

} // namespace atoms::ui


