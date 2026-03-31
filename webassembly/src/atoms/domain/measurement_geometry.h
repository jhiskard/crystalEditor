#pragma once

#include <array>
#include <vector>

namespace atoms::domain {

struct MeasurementDihedralGeometry {
    double dihedralDeg = 0.0;
    std::array<double, 3> textPos {};
    std::array<std::array<double, 3>, 4> helperPlaneQuadA {};
    std::array<std::array<double, 3>, 4> helperPlaneQuadB {};
    std::vector<std::array<double, 3>> helperArcPoints;
};

double calcDistance(const float lhs[3], const float rhs[3]);
bool computeAngleMeasurementGeometry(
    const float point1[3],
    const float point2[3],
    const float point3[3],
    double arcRadiusScale,
    double& angleDeg,
    std::array<double, 3>& arcStart,
    std::array<double, 3>& arcEnd,
    std::array<double, 3>& arcCenter,
    std::array<double, 3>& textPos);
bool computeDihedralMeasurementGeometry(
    const float point1[3],
    const float point2[3],
    const float point3[3],
    const float point4[3],
    MeasurementDihedralGeometry& geometry);

} // namespace atoms::domain
