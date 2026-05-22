// render/infrastructure/atoms/batch_update_system.cpp
#include "batch_update_system.h"
#include "../../../workspace/runtime/workspace_runtime_model_ref.h"
#include "../../application/render_gateway.h"
#include <spdlog/spdlog.h>

namespace atoms {
namespace infrastructure {

// ============================================================================
// BatchUpdateSystem 占쏙옙占쏙옙
// ============================================================================

BatchUpdateSystem::BatchUpdateSystem(WorkspaceRuntimeModel* parent) 
    : parent(parent)
    , batchMode(false) {
    SPDLOG_DEBUG("BatchUpdateSystem initialized");
}

void BatchUpdateSystem::beginBatch() {
    if (batchMode) {
        SPDLOG_WARN("beginBatch() called while already in batch mode - ignoring");
        return;
    }
    
    batchMode = true;
    pendingAtomGroups.clear();
    pendingBondGroups.clear();
    batchStartTime = std::chrono::high_resolution_clock::now();
    
    SPDLOG_DEBUG("Batch mode started (atoms + bonds only)");
}

void BatchUpdateSystem::endBatch() {
    if (!batchMode) {
        SPDLOG_WARN("endBatch() called while not in batch mode - ignoring");
        return;
    }
    
    SPDLOG_DEBUG("Ending batch mode - processing {} atom groups, {} bond groups", 
                pendingAtomGroups.size(), pendingBondGroups.size());
    
    auto batchStart = std::chrono::high_resolution_clock::now();
    
    try {
        // 1. 占쏙옙占쏙옙 占쌓뤄옙 占쏙옙占쏙옙占쏙옙트 (占쏙옙占쏙옙 占시쏙옙占쏙옙 - 占쏙옙占쏙옙 占쌉쇽옙 호占쏙옙)
        for (const std::string& symbol : pendingAtomGroups) {
            ::updateUnifiedAtomGroupVTK(symbol);  // 占쏙옙占쏙옙 占쌉쇽옙 호占쏙옙
            SPDLOG_DEBUG("Updated unified atom group: {}", symbol);
        }
        
        // 2. 占쏙옙占쏙옙 占쌓뤄옙 占쏙옙占쏙옙占쏙옙트 (占쏙옙占쏙옙 占쌉쇽옙 호占쏙옙)
        for (const std::string& bondKey : pendingBondGroups) {
            if (parent) {
                parent->updateBondGroupVTK(bondKey);
            }
            SPDLOG_DEBUG("Updated bond group: {}", bondKey);
        }
        
        // 3. 占쏙옙치 占쏙옙占쏙옙 占쏙옙占쏙옙
        batchMode = false;
        pendingAtomGroups.clear();
        pendingBondGroups.clear();
        
        // 4. 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 호占쏙옙
        render::application::GetRenderGateway().RequestRender();
        
        // 5. 占쏙옙占쏙옙 占쏙옙占쏙옙
        auto batchEnd = std::chrono::high_resolution_clock::now();
        float duration = std::chrono::duration<float, std::milli>(batchEnd - batchStart).count();
        
        SPDLOG_INFO("Batch update completed - {} atom groups, {} bond groups in {:.2f}ms",
                   pendingAtomGroups.size(), pendingBondGroups.size(), duration);
        
        updatePerformanceStats(duration);
        
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Error during batch update: {}", e.what());
        forceBatchEnd();
        throw;
    }
}

void BatchUpdateSystem::forceBatchEnd() {
    if (batchMode) {
        SPDLOG_WARN("Force ending batch mode due to error or abnormal condition");
        batchMode = false;
        pendingAtomGroups.clear();
        pendingBondGroups.clear();
        
        // 占쏙옙占쏙옙占쏙옙 占쏙옙占승뤄옙 占쏙옙占쏙옙占싹깍옙 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
        try {
            render::application::GetRenderGateway().RequestRender();
        } catch (const std::exception& e) {
            SPDLOG_ERROR("Failed to render during force batch end: {}", e.what());
        }
    }
}

void BatchUpdateSystem::scheduleAtomGroupUpdate(const std::string& symbol) {
    if (batchMode) {
        pendingAtomGroups.insert(symbol);
        SPDLOG_DEBUG("Scheduled unified atom group update: {}", symbol);
    } else {
        // 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙치占쏙옙 占쏙옙占쏙옙占쏙옙 占싹곤옙占쏙옙 占쏙옙管占?처占쏙옙
        BatchGuard guard(this);
        pendingAtomGroups.insert(symbol);
        SPDLOG_DEBUG("Scheduled unified atom group update (single-item batch): {}", symbol);
    }
}

void BatchUpdateSystem::scheduleBondGroupUpdate(const std::string& bondKey) {
    if (batchMode) {
        pendingBondGroups.insert(bondKey);
        SPDLOG_DEBUG("Scheduled bond group update: {}", bondKey);
    } else {
        // 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙치占쏙옙 占쏙옙占쏙옙占쏙옙 占싹곤옙占쏙옙 占쏙옙管占?처占쏙옙
        BatchGuard guard(this);
        pendingBondGroups.insert(bondKey);
        SPDLOG_DEBUG("Scheduled bond group update (single-item batch): {}", bondKey);
    }
}

void BatchUpdateSystem::updatePerformanceStats(float duration) {
    // WorkspaceRuntimeModel占쏙옙 updatePerformanceStats 호占쏙옙
    // parent占쏙옙 占쏙옙占쏙옙 占쏙옙占?占쏙옙占쏙옙占쏙옙트占쏙옙 占쏙옙占쏙옙
    parent->UpdateBatchPerformanceStats(duration,
                                        pendingAtomGroups.size(),
                                        pendingBondGroups.size());
}

// ============================================================================
// RAII BatchGuard 占쏙옙占쏙옙
// ============================================================================

BatchUpdateSystem::BatchGuard::BatchGuard(BatchUpdateSystem* sys) 
    : system(sys) {
    wasActivated = !sys->isBatchMode();
    if (wasActivated) {
        sys->beginBatch();
        SPDLOG_DEBUG("BatchGuard: Started new batch");
    } else {
        SPDLOG_DEBUG("BatchGuard: Using existing batch");
    }
}

BatchUpdateSystem::BatchGuard::~BatchGuard() {
    if (wasActivated && system) {
        try {
            system->endBatch();
            SPDLOG_DEBUG("BatchGuard: Ended batch successfully");
        } catch (const std::exception& e) {
            SPDLOG_ERROR("Exception in BatchGuard destructor: {}", e.what());
            try {
                system->forceBatchEnd();
            } catch (...) {
                SPDLOG_CRITICAL("Failed to force end batch in BatchGuard destructor");
            }
        }
    }
}

} // namespace infrastructure
} // namespace atoms







