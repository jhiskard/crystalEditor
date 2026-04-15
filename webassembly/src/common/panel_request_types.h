/**
 * @file panel_request_types.h
 * @brief Shared panel request enums used across shell/feature boundaries.
 */
#pragma once

namespace workbench {
namespace panel {

/**
 * @brief Logical editor-panel requests from menu/toolbar actions.
 */
enum class EditorRequest {
    Atoms = 0,
    Bonds,
    Cell
};

/**
 * @brief Logical builder-panel requests from menu/toolbar actions.
 */
enum class BuilderRequest {
    AddAtoms = 0,
    BravaisLatticeTemplates,
    BrillouinZone
};

/**
 * @brief Logical data-panel requests from menu/toolbar actions.
 */
enum class DataRequest {
    Isosurface = 0,
    Surface,
    Volumetric,
    Plane
};

} // namespace panel
} // namespace workbench

