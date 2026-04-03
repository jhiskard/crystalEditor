/**
 * @file import_workflow_service.h
 * @brief Transaction workflow service for structure import lifecycle.
 */
#pragma once

#include "import_orchestrator.h"

#include <cstdint>

namespace io::application {

/**
 * @brief Coordinates replace-scene import transaction boundaries.
 * @details This service owns transaction snapshot/rollback state so UI-facing
 *          facades can delegate workflow control without directly mutating
 *          cross-feature scene context.
 */
class ImportWorkflowService {
public:
    /**
     * @brief Returns whether scene data exists before structure import.
     */
    bool HasSceneDataForStructureImport() const;

    /**
     * @brief Starts a replace-scene transaction by capturing snapshot state.
     */
    void BeginReplaceSceneImportTransaction();

    /**
     * @brief Returns whether replace-scene transaction is active.
     */
    bool IsReplaceSceneImportTransactionActive() const { return m_ReplaceSceneImportTransactionActive; }

    /**
     * @brief Finalizes transaction on import success.
     */
    void FinalizeImportOnSuccess(int32_t importedStructureId);

    /**
     * @brief Rolls back transaction on import failure.
     */
    void RollbackImportOnFailure(int32_t importedStructureId);

    /**
     * @brief Removes partially imported structure when transaction is not active.
     */
    void CleanupImportedStructure(int32_t importedStructureId) const;

    /**
     * @brief Clears replace-scene transaction state.
     */
    void ClearReplaceSceneImportTransaction();

private:
    ImportOrchestrator m_ImportOrchestrator;
    ReplaceSceneImportSnapshot m_ReplaceSceneImportSnapshot;
    bool m_ReplaceSceneImportTransactionActive = false;
};

} // namespace io::application

