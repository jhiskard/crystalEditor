#pragma once

class WorkspaceRuntimeModel;

namespace atoms::domain {

class SurroundingAtomManager {
public:
    explicit SurroundingAtomManager(::WorkspaceRuntimeModel* parent);

    void createSurroundingAtoms();
    void hideSurroundingAtoms();
    bool isVisible() const;
    void setVisible(bool visible);

private:
    ::WorkspaceRuntimeModel* m_parent = nullptr;
};

} // namespace atoms::domain


