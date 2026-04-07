#include "legacy_viewer_facade.h"

#include "../infrastructure/vtk_render_gateway.h"

namespace render {
namespace application {

VtkViewer& GetLegacyViewerFacade() {
    return infrastructure::GetLegacyViewerFacade();
}

} // namespace application
} // namespace render
