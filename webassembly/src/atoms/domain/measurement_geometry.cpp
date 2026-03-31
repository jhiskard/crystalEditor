#include "measurement_geometry.h"

#include <algorithm>
#include <cmath>

namespace atoms::domain {

namespace {

double vectorNorm(const std::array<double, 3>& vec) {
    return std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

std::array<double, 3> normalizeVector(const std::array<double, 3>& vec) {
    const double norm = vectorNorm(vec);
    if (norm <= 1e-12) {
        return { 0.0, 0.0, 0.0 };
    }
    return { vec[0] / norm, vec[1] / norm, vec[2] / norm };
}

std::array<double, 3> crossVector(
    const std::array<double, 3>& lhs,
    const std::array<double, 3>& rhs) {
    return {
        lhs[1] * rhs[2] - lhs[2] * rhs[1],
        lhs[2] * rhs[0] - lhs[0] * rhs[2],
        lhs[0] * rhs[1] - lhs[1] * rhs[0]
    };
}

double dotVector(const std::array<double, 3>& lhs, const std::array<double, 3>& rhs) {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}

std::array<double, 3> pickPerpendicularAxis(const std::array<double, 3>& vec) {
    if (std::fabs(vec[2]) < 0.8) {
        return { 0.0, 0.0, 1.0 };
    }
    return { 0.0, 1.0, 0.0 };
}

std::array<double, 3> toVec3(const float pos[3]) {
    return {
        static_cast<double>(pos[0]),
        static_cast<double>(pos[1]),
        static_cast<double>(pos[2])
    };
}

std::array<double, 3> subtractVector(
    const std::array<double, 3>& lhs,
    const std::array<double, 3>& rhs) {
    return { lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2] };
}

std::array<double, 3> addVector(
    const std::array<double, 3>& lhs,
    const std::array<double, 3>& rhs) {
    return { lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2] };
}

std::array<double, 3> scaleVector(const std::array<double, 3>& vec, double scale) {
    return { vec[0] * scale, vec[1] * scale, vec[2] * scale };
}

std::array<double, 3> projectVectorToPlane(
    const std::array<double, 3>& vec,
    const std::array<double, 3>& planeNormalUnit) {
    return subtractVector(vec, scaleVector(planeNormalUnit, dotVector(vec, planeNormalUnit)));
}

std::array<double, 3> rotateVectorAroundAxis(
    const std::array<double, 3>& vec,
    const std::array<double, 3>& axisUnit,
    double angleRad) {
    const double cosTheta = std::cos(angleRad);
    const double sinTheta = std::sin(angleRad);
    const auto term1 = scaleVector(vec, cosTheta);
    const auto term2 = scaleVector(crossVector(axisUnit, vec), sinTheta);
    const auto term3 = scaleVector(axisUnit, dotVector(axisUnit, vec) * (1.0 - cosTheta));
    return addVector(addVector(term1, term2), term3);
}

} // namespace

double calcDistance(const float lhs[3], const float rhs[3]) {
    const double dx = static_cast<double>(lhs[0]) - static_cast<double>(rhs[0]);
    const double dy = static_cast<double>(lhs[1]) - static_cast<double>(rhs[1]);
    const double dz = static_cast<double>(lhs[2]) - static_cast<double>(rhs[2]);
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool computeAngleMeasurementGeometry(
    const float point1[3],
    const float point2[3],
    const float point3[3],
    double arcRadiusScale,
    double& angleDeg,
    std::array<double, 3>& arcStart,
    std::array<double, 3>& arcEnd,
    std::array<double, 3>& arcCenter,
    std::array<double, 3>& textPos) {
    constexpr double kEps = 1e-8;
    constexpr double kRadToDeg = 180.0 / 3.14159265358979323846;

    const std::array<double, 3> center = {
        static_cast<double>(point2[0]),
        static_cast<double>(point2[1]),
        static_cast<double>(point2[2])
    };
    const std::array<double, 3> vec1 = {
        static_cast<double>(point1[0]) - center[0],
        static_cast<double>(point1[1]) - center[1],
        static_cast<double>(point1[2]) - center[2]
    };
    const std::array<double, 3> vec2 = {
        static_cast<double>(point3[0]) - center[0],
        static_cast<double>(point3[1]) - center[1],
        static_cast<double>(point3[2]) - center[2]
    };

    const double len1 = vectorNorm(vec1);
    const double len2 = vectorNorm(vec2);
    if (len1 <= kEps || len2 <= kEps) {
        return false;
    }

    const std::array<double, 3> dir1 = normalizeVector(vec1);
    const std::array<double, 3> dir2 = normalizeVector(vec2);

    const double cosine = std::clamp(dotVector(dir1, dir2), -1.0, 1.0);
    angleDeg = std::acos(cosine) * kRadToDeg;

    const double radius = std::max(
        0.05,
        std::min(len1, len2) * 0.35 * std::max(0.05, arcRadiusScale));
    arcCenter = center;
    arcStart = {
        center[0] + dir1[0] * radius,
        center[1] + dir1[1] * radius,
        center[2] + dir1[2] * radius
    };
    arcEnd = {
        center[0] + dir2[0] * radius,
        center[1] + dir2[1] * radius,
        center[2] + dir2[2] * radius
    };

    std::array<double, 3> bisector = {
        dir1[0] + dir2[0],
        dir1[1] + dir2[1],
        dir1[2] + dir2[2]
    };
    if (vectorNorm(bisector) <= kEps) {
        std::array<double, 3> normal = crossVector(dir1, dir2);
        if (vectorNorm(normal) <= kEps) {
            normal = crossVector(dir1, pickPerpendicularAxis(dir1));
        }
        normal = normalizeVector(normal);
        bisector = crossVector(normal, dir1);
    }
    bisector = normalizeVector(bisector);
    if (vectorNorm(bisector) <= kEps) {
        return false;
    }

    const double textRadius = radius * 1.18;
    textPos = {
        center[0] + bisector[0] * textRadius,
        center[1] + bisector[1] * textRadius,
        center[2] + bisector[2] * textRadius
    };
    return true;
}

bool computeDihedralMeasurementGeometry(
    const float point1[3],
    const float point2[3],
    const float point3[3],
    const float point4[3],
    MeasurementDihedralGeometry& geometry) {
    constexpr double kEps = 1e-8;
    constexpr double kPi = 3.14159265358979323846;
    constexpr double kRadToDeg = 180.0 / kPi;

    const auto p1 = toVec3(point1);
    const auto p2 = toVec3(point2);
    const auto p3 = toVec3(point3);
    const auto p4 = toVec3(point4);

    const auto b1 = subtractVector(p2, p1);
    const auto b2 = subtractVector(p3, p2);
    const auto b3 = subtractVector(p4, p3);

    const double b2Len = vectorNorm(b2);
    if (b2Len <= kEps) {
        return false;
    }

    const auto n12 = crossVector(b1, b2);
    const auto n23 = crossVector(b2, b3);
    const double n12Len = vectorNorm(n12);
    const double n23Len = vectorNorm(n23);
    if (n12Len <= kEps || n23Len <= kEps) {
        return false;
    }

    const auto axis23 = normalizeVector(b2);
    const double x = dotVector(n12, n23);
    const double y = b2Len * dotVector(b1, crossVector(b2, b3));
    const double angleRad = std::atan2(y, x);
    geometry.dihedralDeg = angleRad * kRadToDeg;

    const auto proj1 = projectVectorToPlane(b1, axis23);
    const auto proj3 = projectVectorToPlane(b3, axis23);
    const double proj1Len = vectorNorm(proj1);
    const double proj3Len = vectorNorm(proj3);
    if (proj1Len <= kEps || proj3Len <= kEps) {
        return false;
    }

    const auto startDir = normalizeVector(proj1);
    const auto endDir = normalizeVector(proj3);
    const auto orthoDir = normalizeVector(crossVector(axis23, startDir));
    if (vectorNorm(orthoDir) <= kEps) {
        return false;
    }
    const auto mid23 = scaleVector(addVector(p2, p3), 0.5);

    const double arcRadius = std::max(0.05, std::min(proj1Len, proj3Len) * 0.6);
    const double planeRadius = std::max(0.05, arcRadius * 1.0);
    const double halfLen = std::max(0.08, b2Len * 0.55);

    const auto axisOffset = scaleVector(axis23, halfLen);
    const auto planeOffset1 = scaleVector(startDir, planeRadius);
    const auto planeOffset3 = scaleVector(endDir, planeRadius);

    geometry.helperPlaneQuadA = {
        addVector(subtractVector(mid23, axisOffset), planeOffset1),
        addVector(addVector(mid23, axisOffset), planeOffset1),
        subtractVector(addVector(mid23, axisOffset), planeOffset1),
        subtractVector(subtractVector(mid23, axisOffset), planeOffset1)
    };
    geometry.helperPlaneQuadB = {
        addVector(subtractVector(mid23, axisOffset), planeOffset3),
        addVector(addVector(mid23, axisOffset), planeOffset3),
        subtractVector(addVector(mid23, axisOffset), planeOffset3),
        subtractVector(subtractVector(mid23, axisOffset), planeOffset3)
    };

    int arcResolution = 48;
    if (std::fabs(angleRad) < (kPi / 12.0)) {
        arcResolution = 24;
    }
    geometry.helperArcPoints.clear();
    geometry.helperArcPoints.reserve(static_cast<size_t>(arcResolution) + 1);
    for (int i = 0; i <= arcResolution; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(arcResolution);
        const double theta = angleRad * t;
        const auto dir = addVector(
            scaleVector(startDir, std::cos(theta)),
            scaleVector(orthoDir, std::sin(theta)));
        geometry.helperArcPoints.push_back(addVector(mid23, scaleVector(dir, arcRadius)));
    }

    const auto textDir = normalizeVector(rotateVectorAroundAxis(startDir, axis23, angleRad * 0.5));
    const double textRadius = arcRadius * 1.18;
    const double textAxisOffset = std::max(0.02, arcRadius * 0.12);
    geometry.textPos = addVector(
        addVector(mid23, scaleVector(textDir, textRadius)),
        scaleVector(axis23, textAxisOffset));

    return true;
}

} // namespace atoms::domain
