#include "../presentation/viewer_window.h"

#include "../../atoms/domain/cell_manager.h"
#include "../../config/log_config.h"

#include <vtkCamera.h>
#include <vtkRenderer.h>

#include <cmath>

namespace {
bool AlignCameraToAxisImpl(vtkRenderer* renderer, const double axis[3], const double upCandidate[3]) {
    if (!renderer) {
        return false;
    }

    double dir[3] = { axis[0], axis[1], axis[2] };
    double dirNorm = std::sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);

    if (dirNorm < 1e-6) {
        return false;
    }
    dir[0] /= dirNorm;
    dir[1] /= dirNorm;
    dir[2] /= dirNorm;

    double up[3] = { upCandidate[0], upCandidate[1], upCandidate[2] };
    double dot = up[0] * dir[0] + up[1] * dir[1] + up[2] * dir[2];

    up[0] -= dot * dir[0];
    up[1] -= dot * dir[1];
    up[2] -= dot * dir[2];
    double upNorm = std::sqrt(up[0] * up[0] + up[1] * up[1] + up[2] * up[2]);

    if (upNorm < 1e-6) {

        up[0] = 0.0;
        up[1] = 1.0;
        up[2] = 0.0;
        dot = up[0] * dir[0] + up[1] * dir[1] + up[2] * dir[2];
        up[0] -= dot * dir[0];
        up[1] -= dot * dir[1];
        up[2] -= dot * dir[2];
        upNorm = std::sqrt(up[0] * up[0] + up[1] * up[1] + up[2] * up[2]);
        if (upNorm < 1e-6) {
            up[0] = 1.0;
            up[1] = 0.0;
            up[2] = 0.0;
            dot = up[0] * dir[0] + up[1] * dir[1] + up[2] * dir[2];
            up[0] -= dot * dir[0];
            up[1] -= dot * dir[1];
            up[2] -= dot * dir[2];
            upNorm = std::sqrt(up[0] * up[0] + up[1] * up[1] + up[2] * up[2]);
            if (upNorm < 1e-6) {
                return false;
            }
        }
    }

    up[0] /= upNorm;
    up[1] /= upNorm;
    up[2] /= upNorm;

    vtkCamera* camera = renderer->GetActiveCamera();
    if (!camera) {
        SPDLOG_WARN("  -> No active camera!");

        return false;
    }

    double focalPoint[3];
    camera->GetFocalPoint(focalPoint);

    double cameraPos[3];
    camera->GetPosition(cameraPos);

    double dx = cameraPos[0] - focalPoint[0];
    double dy = cameraPos[1] - focalPoint[1];
    double dz = cameraPos[2] - focalPoint[2];
    double dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist < 1e-6) {
        dist = 1.0;
    }

    double newPos[3] = {
        focalPoint[0] + dir[0] * dist,
        focalPoint[1] + dir[1] * dist,
        focalPoint[2] + dir[2] * dist
    };


    camera->SetPosition(newPos);
    camera->SetViewUp(up);
    camera->OrthogonalizeViewUp();
    renderer->ResetCameraClippingRange();

    // ✅ 설정 후 확인
    double finalPos[3], finalUp[3];
    camera->GetPosition(finalPos);
    camera->GetViewUp(finalUp);
        
    return true;
}
} // namespace

void VtkViewer::AlignCameraToCellAxis(int axisIndex) {
    if (!m_Renderer || axisIndex < 0 || axisIndex > 2) {
        return;
    }
    if (!atoms::domain::hasUnitCell()) {
        return;
    }

    float cell[3][3];
    atoms::domain::getCellMatrix(cell);
    // ✅ 디버그 로그 추가
    SPDLOG_INFO("AlignCameraToCellAxis({}) - Cell matrix:", axisIndex);
    SPDLOG_INFO("  a1 (cell[0]): [{:.4f}, {:.4f}, {:.4f}]", cell[0][0], cell[0][1], cell[0][2]);
    SPDLOG_INFO("  a2 (cell[1]): [{:.4f}, {:.4f}, {:.4f}]", cell[1][0], cell[1][1], cell[1][2]);
    SPDLOG_INFO("  a3 (cell[2]): [{:.4f}, {:.4f}, {:.4f}]", cell[2][0], cell[2][1], cell[2][2]);

    double axis[3] = {
        static_cast<double>(cell[axisIndex][0]),
        static_cast<double>(cell[axisIndex][1]),
        static_cast<double>(cell[axisIndex][2])
    };
    SPDLOG_INFO("  Selected axis[{}]: [{:.4f}, {:.4f}, {:.4f}]", axisIndex, axis[0], axis[1], axis[2]);
    // axis 벡터 길이 체크
    double axisNorm = std::sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
    SPDLOG_INFO("  Axis norm: {:.6f}", axisNorm);

    
    int upIndex = (axisIndex == 0) ? 2 : (axisIndex == 1) ? 0 : 1;
    double up[3] = {
        static_cast<double>(cell[upIndex][0]),
        static_cast<double>(cell[upIndex][1]),
        static_cast<double>(cell[upIndex][2])
    };

    if (AlignCameraToAxisImpl(m_Renderer, axis, up)) {
        setCameraDirection(CameraDirection::NOT_ALIGNED);
        RequestRender();
    }
}

void VtkViewer::AlignCameraToIcellAxis(int axisIndex) {
    if (!m_Renderer || axisIndex < 0 || axisIndex > 2) {
        return;
    }
    if (!atoms::domain::hasUnitCell()) {
        return;
    }

    float invmatrix[3][3];
    atoms::domain::getCellInverse(invmatrix);

    double icell[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            icell[i][j] = static_cast<double>(invmatrix[j][i]);
        }
    }

    double axis[3] = {
        icell[axisIndex][0],
        icell[axisIndex][1],
        icell[axisIndex][2]
    };

    int upIndex = (axisIndex == 0) ? 2 : (axisIndex == 1) ? 0 : 1;
    double up[3] = {
        icell[upIndex][0],
        icell[upIndex][1],
        icell[upIndex][2]
    };

    if (AlignCameraToAxisImpl(m_Renderer, axis, up)) {
        setCameraDirection(CameraDirection::NOT_ALIGNED);
        RequestRender();
    }
}

