// render/infrastructure/atoms/batch_update_system.cpp
#include "batch_update_system.h"
#include "../../../workspace/runtime/legacy_atoms_runtime.h"
#include "../../application/render_gateway.h"
#include <spdlog/spdlog.h>

namespace atoms {
namespace infrastructure {

// ============================================================================
// BatchUpdateSystem ����
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
        // 1. ���� �׷� ������Ʈ (���� �ý��� - ���� �Լ� ȣ��)
        for (const std::string& symbol : pendingAtomGroups) {
            ::updateUnifiedAtomGroupVTK(symbol);  // ���� �Լ� ȣ��
            SPDLOG_DEBUG("Updated unified atom group: {}", symbol);
        }
        
        // 2. ���� �׷� ������Ʈ (���� �Լ� ȣ��)
        for (const std::string& bondKey : pendingBondGroups) {
            if (parent) {
                parent->updateBondGroupVTK(bondKey);
            }
            SPDLOG_DEBUG("Updated bond group: {}", bondKey);
        }
        
        // 3. ��ġ ���� ����
        batchMode = false;
        pendingAtomGroups.clear();
        pendingBondGroups.clear();
        
        // 4. ���� ������ ȣ��
        render::application::GetRenderGateway().RequestRender();
        
        // 5. ���� ����
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
        
        // ������ ���·� �����ϱ� ���� ������ ����
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
        // ���� ������ ��ġ�� ������ �ϰ��� ��η� ó��
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
        // ���� ������ ��ġ�� ������ �ϰ��� ��η� ó��
        BatchGuard guard(this);
        pendingBondGroups.insert(bondKey);
        SPDLOG_DEBUG("Scheduled bond group update (single-item batch): {}", bondKey);
    }
}

void BatchUpdateSystem::updatePerformanceStats(float duration) {
    // WorkspaceRuntimeModel�� updatePerformanceStats ȣ��
    // parent�� ���� ��� ������Ʈ�� ����
    parent->UpdateBatchPerformanceStats(duration,
                                        pendingAtomGroups.size(),
                                        pendingBondGroups.size());
}

// ============================================================================
// RAII BatchGuard ����
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







