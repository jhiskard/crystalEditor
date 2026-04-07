#pragma once

class VtkViewer;

namespace render {
namespace application {

/**
 * @brief Returns the legacy viewer facade for runtime/shell compatibility.
 * @details Non-render modules should include this application-level bridge
 *          instead of depending on render infrastructure headers.
 */
VtkViewer& GetLegacyViewerFacade();

} // namespace application
} // namespace render
