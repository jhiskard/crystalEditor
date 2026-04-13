#pragma once

class FileLoader;

namespace shell::presentation {

/**
 * @brief ImGui presenter for structure/XSF import popups.
 */
class ImportPopupPresenter {
public:
    void Render(FileLoader& fileLoader);
};

} // namespace shell::presentation

