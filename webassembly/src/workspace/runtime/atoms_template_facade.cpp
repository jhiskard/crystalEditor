#include "atoms_template_facade.h"
#include "../../app.h"
#include "../../mesh/presentation/model_tree_panel.h"
#include "../../mesh/presentation/mesh_detail_panel.h"
#include "../../mesh/domain/mesh_repository.h"
#include "../../mesh/application/mesh_query_service.h"
#include "../../mesh/application/mesh_command_service.h"
#include "../../config/log_config.h" // spdlog ?ㅻ뜑 寃쎈줈 ?섏젙 - 濡쒓퉭 ?ㅼ젙 ?뚯씪 ?ъ슜
#include "../../structure/application/structure_service.h"
#include "../../measurement/application/measurement_service.h"
#include "../../density/application/density_service.h"
#include "../../render/application/picked_atom_info.h"
#include "../../render/application/render_gateway.h"

#include "../../render/infrastructure/atoms/bond_renderer.h"
#include "../../shell/presentation/atoms/atom_editor_ui.h"
#include "../../shell/presentation/atoms/bond_ui.h"
#include "../../shell/presentation/atoms/cell_info_ui.h"
#include "../../shell/presentation/atoms/bravais_lattice_ui.h"
#include "../../shell/presentation/atoms/periodic_table_ui.h"
#include "../../shell/presentation/atoms/bz_plot_ui.h"

// Standard library
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <unordered_set>

#include <vtkCoordinate.h>
#include <vtkCellPicker.h>
#include <vtkCellArray.h>
#include <vtkArcSource.h>
#include <vtkLine.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <vtkRegularPolygonSource.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkImageData.h>
#include <vtkStructuredGrid.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkResampleToImage.h>

#include "../../density/presentation/charge_density_ui.h"
#include "../../io/infrastructure/chgcar_parser.h"
#include "../../measurement/domain/measurement_geometry.h"
#include "../../structure/domain/structure_repository.h"


// ============================================================================
// (251212) Global variables
// ============================================================================

static int resolution = 20;

// ============================================================================
// (251212) Performance
// ============================================================================

struct RenderingPerformanceStats {
    int totalAtoms = 0;
    int totalGroups = 0;
    int totalBonds = 0;
    float lastUpdateTime = 0.0f;
    float averageUpdateTime = 0.0f;
    int updateCount = 0;
};

static RenderingPerformanceStats g_performanceStats;
static std::chrono::high_resolution_clock::time_point g_lastMeasureTime;


namespace {
constexpr float kMinMeasurementLineWidth = 2.0f;
constexpr float kMaxMeasurementLineWidth = 18.0f;
constexpr float kMinMeasurementLabelScale = 1.0f;
constexpr float kMaxMeasurementLabelScale = 3.0f;
constexpr float kMinAngleArcRadiusScale = 0.6f;
constexpr float kMaxAngleArcRadiusScale = 2.0f;
constexpr float kMinDihedralPlaneOpacity = 0.05f;
constexpr float kMaxDihedralPlaneOpacity = 0.60f;
constexpr int kMeasurementValueBaseFontSize = static_cast<int>(14 * 1.5f);

auto& createdAtoms = structure::domain::GetStructureRepository().CreatedAtoms();
auto& surroundingAtoms = structure::domain::GetStructureRepository().SurroundingAtoms();
auto& atomGroups = structure::domain::GetStructureRepository().AtomGroups();
auto& createdBonds = structure::domain::GetStructureRepository().CreatedBonds();
auto& surroundingBonds = structure::domain::GetStructureRepository().SurroundingBonds();
auto& bondGroups = structure::domain::GetStructureRepository().BondGroups();

float clampFloatRange(float value, float minValue, float maxValue) {
    return std::clamp(value, minValue, maxValue);
}

float snapToOneDecimal(float value) {
    return std::round(value * 10.0f) / 10.0f;
}

std::array<double, 3> toDoubleColor(const std::array<float, 3>& color) {
    return {
        static_cast<double>(color[0]),
        static_cast<double>(color[1]),
        static_cast<double>(color[2])
    };
}

void requestViewerRender() {
    render::application::GetRenderGateway().RequestRender();
}

int scaledFontSize(int baseFontSize, float scale) {
    const int scaled = static_cast<int>(std::lround(static_cast<double>(baseFontSize) * static_cast<double>(scale)));
    return std::clamp(scaled, 8, 144);
}

vtkTextProperty* textPropertyFromActor2D(vtkActor2D* actor) {
    vtkTextActor* textActor = vtkTextActor::SafeDownCast(actor);
    if (!textActor) {
        return nullptr;
    }
    return textActor->GetTextProperty();
}

void applyTextScaleToActor(vtkActor2D* actor, float scale, int baseFontSize) {
    vtkTextProperty* property = textPropertyFromActor2D(actor);
    if (!property) {
        return;
    }
    property->SetFontSize(scaledFontSize(baseFontSize, scale));
}

void applyLineStyleToActor(
    vtkActor* actor,
    const std::array<float, 3>& color,
    float lineWidth) {
    if (!actor) {
        return;
    }
    vtkProperty* property = actor->GetProperty();
    if (!property) {
        return;
    }
    property->SetColor(
        static_cast<double>(color[0]),
        static_cast<double>(color[1]),
        static_cast<double>(color[2]));
    property->SetLineWidth(static_cast<double>(lineWidth));
}

bool isAxisAlignedLattice(const float lattice[3][3]) {
    const float eps = 1e-6f;
    return std::fabs(lattice[0][1]) < eps &&
           std::fabs(lattice[0][2]) < eps &&
           std::fabs(lattice[1][0]) < eps &&
           std::fabs(lattice[1][2]) < eps &&
           std::fabs(lattice[2][0]) < eps &&
           std::fabs(lattice[2][1]) < eps;
}

int calcDownsampleDim(int dim, int factor) {
    if (dim <= 1) {
        return 1;
    }
    return (dim - 1) / factor + 1;
}

bool isSameGridShape(const atoms::infrastructure::ChgcarParser::ParseResult& lhs,
                     const atoms::infrastructure::ChgcarParser::ParseResult& rhs) {
    return lhs.gridShape[0] == rhs.gridShape[0] &&
           lhs.gridShape[1] == rhs.gridShape[1] &&
           lhs.gridShape[2] == rhs.gridShape[2];
}

atoms::infrastructure::ChgcarParser::ParseResult createDownsampledChgcarResult(
    const atoms::infrastructure::ChgcarParser::ParseResult& source,
    int factor) {
    atoms::infrastructure::ChgcarParser::ParseResult downsampled = source;
    if (factor <= 1 || source.density.empty()) {
        return downsampled;
    }

    const int srcNx = source.gridShape[0];
    const int srcNy = source.gridShape[1];
    const int srcNz = source.gridShape[2];
    if (srcNx <= 0 || srcNy <= 0 || srcNz <= 0) {
        return downsampled;
    }

    const size_t srcTotal = static_cast<size_t>(srcNx) * static_cast<size_t>(srcNy) * static_cast<size_t>(srcNz);
    if (source.density.size() < srcTotal) {
        return downsampled;
    }

    const int dstNx = calcDownsampleDim(srcNx, factor);
    const int dstNy = calcDownsampleDim(srcNy, factor);
    const int dstNz = calcDownsampleDim(srcNz, factor);
    if (dstNx == srcNx && dstNy == srcNy && dstNz == srcNz) {
        return downsampled;
    }

    downsampled.gridShape = { dstNx, dstNy, dstNz };
    downsampled.density.clear();
    downsampled.density.reserve(static_cast<size_t>(dstNx) * static_cast<size_t>(dstNy) * static_cast<size_t>(dstNz));

    bool hasFirstValue = false;
    float minValue = 0.0f;
    float maxValue = 0.0f;

    for (int ix = 0; ix < dstNx; ++ix) {
        const int srcIx = std::min(ix * factor, srcNx - 1);
        for (int iy = 0; iy < dstNy; ++iy) {
            const int srcIy = std::min(iy * factor, srcNy - 1);
            for (int iz = 0; iz < dstNz; ++iz) {
                const int srcIz = std::min(iz * factor, srcNz - 1);
                const int srcIndex = srcIz + srcIy * srcNz + srcIx * srcNy * srcNz;
                const float value = source.density[static_cast<size_t>(srcIndex)];
                downsampled.density.push_back(value);
                if (!hasFirstValue) {
                    minValue = value;
                    maxValue = value;
                    hasFirstValue = true;
                } else {
                    minValue = std::min(minValue, value);
                    maxValue = std::max(maxValue, value);
                }
            }
        }
    }

    downsampled.minValue = minValue;
    downsampled.maxValue = maxValue;

    downsampled.hasSpinPolarization = false;
    downsampled.spinDensity.clear();
    if (source.hasSpinPolarization && source.spinDensity.size() >= srcTotal) {
        downsampled.spinDensity.reserve(downsampled.density.size());
        for (int ix = 0; ix < dstNx; ++ix) {
            const int srcIx = std::min(ix * factor, srcNx - 1);
            for (int iy = 0; iy < dstNy; ++iy) {
                const int srcIy = std::min(iy * factor, srcNy - 1);
                for (int iz = 0; iz < dstNz; ++iz) {
                    const int srcIz = std::min(iz * factor, srcNz - 1);
                    const int srcIndex = srcIz + srcIy * srcNz + srcIx * srcNy * srcNz;
                    downsampled.spinDensity.push_back(source.spinDensity[static_cast<size_t>(srcIndex)]);
                }
            }
        }
        downsampled.hasSpinPolarization = true;
    }

    return downsampled;
}

vtkSmartPointer<vtkImageData> createChgcarImageData(
    const atoms::infrastructure::ChgcarParser::ParseResult& result) {
    const int nx = result.gridShape[0];
    const int ny = result.gridShape[1];
    const int nz = result.gridShape[2];

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetDimensions(nx, ny, nz);
    image->SetOrigin(0.0, 0.0, 0.0);

    double spacing[3] = { 1.0, 1.0, 1.0 };
    if (nx > 1) {
        spacing[0] = std::fabs(result.lattice[0][0]) / static_cast<double>(nx - 1);
    }
    if (ny > 1) {
        spacing[1] = std::fabs(result.lattice[1][1]) / static_cast<double>(ny - 1);
    }
    if (nz > 1) {
        spacing[2] = std::fabs(result.lattice[2][2]) / static_cast<double>(nz - 1);
    }
    image->SetSpacing(spacing);

    image->AllocateScalars(VTK_FLOAT, 1);
    float* ptr = static_cast<float*>(image->GetScalarPointer());

    for (int ix = 0; ix < nx; ++ix) {
        for (int iy = 0; iy < ny; ++iy) {
            for (int iz = 0; iz < nz; ++iz) {
                int vaspIdx = iz + iy * nz + ix * ny * nz;
                int vtkIdx = ix + iy * nx + iz * nx * ny;
                ptr[vtkIdx] = result.density[vaspIdx];
            }
        }
    }

    image->Modified();
    return image;
}

vtkSmartPointer<vtkStructuredGrid> createChgcarStructuredGrid(
    const atoms::infrastructure::ChgcarParser::ParseResult& result) {
    const int nx = result.gridShape[0];
    const int ny = result.gridShape[1];
    const int nz = result.gridShape[2];

    vtkSmartPointer<vtkStructuredGrid> grid = vtkSmartPointer<vtkStructuredGrid>::New();
    grid->SetDimensions(nx, ny, nz);

    const size_t total = static_cast<size_t>(nx) * static_cast<size_t>(ny) * static_cast<size_t>(nz);
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(static_cast<vtkIdType>(total));

    const float denomX = (nx > 1) ? static_cast<float>(nx - 1) : 1.0f;
    const float denomY = (ny > 1) ? static_cast<float>(ny - 1) : 1.0f;
    const float denomZ = (nz > 1) ? static_cast<float>(nz - 1) : 1.0f;

    size_t idx = 0;
    for (int iz = 0; iz < nz; ++iz) {
        const float tz = static_cast<float>(iz) / denomZ;
        for (int iy = 0; iy < ny; ++iy) {
            const float ty = static_cast<float>(iy) / denomY;
            for (int ix = 0; ix < nx; ++ix) {
                const float tx = static_cast<float>(ix) / denomX;
                const float x = tx * result.lattice[0][0] + ty * result.lattice[1][0] + tz * result.lattice[2][0];
                const float y = tx * result.lattice[0][1] + ty * result.lattice[1][1] + tz * result.lattice[2][1];
                const float z = tx * result.lattice[0][2] + ty * result.lattice[1][2] + tz * result.lattice[2][2];
                points->SetPoint(static_cast<vtkIdType>(idx), x, y, z);
                ++idx;
            }
        }
    }

    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetName("Isosurface");
    scalars->SetNumberOfComponents(1);
    scalars->SetNumberOfTuples(static_cast<vtkIdType>(total));

    for (int ix = 0; ix < nx; ++ix) {
        for (int iy = 0; iy < ny; ++iy) {
            for (int iz = 0; iz < nz; ++iz) {
                int vaspIdx = iz + iy * nz + ix * ny * nz;
                int vtkIdx = ix + iy * nx + iz * nx * ny;
                scalars->SetValue(static_cast<vtkIdType>(vtkIdx), result.density[vaspIdx]);
            }
        }
    }

    grid->SetPoints(points);
    grid->GetPointData()->SetScalars(scalars);
    return grid;
}

vtkSmartPointer<vtkImageData> resampleStructuredGridToImage(
    vtkSmartPointer<vtkStructuredGrid> grid, const int dims[3]) {
    if (!grid) {
        return nullptr;
    }
    vtkNew<vtkResampleToImage> resample;
    resample->SetInputDataObject(grid);
    resample->SetSamplingDimensions(dims[0], dims[1], dims[2]);
    double bounds[6];
    grid->GetBounds(bounds);
    resample->SetSamplingBounds(bounds);
    resample->Update();
    return resample->GetOutput();
}

bool createChgcarDataSets(const atoms::infrastructure::ChgcarParser::ParseResult& result,
                          vtkSmartPointer<vtkDataSet>& surfaceDataSet,
                          vtkSmartPointer<vtkDataSet>& volumeDataSet) {
    surfaceDataSet = nullptr;
    volumeDataSet = nullptr;
    if (result.density.empty()) {
        return false;
    }

    if (isAxisAlignedLattice(result.lattice)) {
        vtkSmartPointer<vtkImageData> image = createChgcarImageData(result);
        surfaceDataSet = image;
        volumeDataSet = image;
    } else {
        vtkSmartPointer<vtkStructuredGrid> structured = createChgcarStructuredGrid(result);
        surfaceDataSet = structured;
        volumeDataSet = resampleStructuredGridToImage(structured, result.gridShape.data());
        if (!volumeDataSet) {
            volumeDataSet = structured;
        }
    }
    return surfaceDataSet != nullptr;
}

const atoms::domain::AtomInfo* resolveBondAtom(
    const atoms::domain::BondInfo& bond, bool firstEndpoint) {
    const auto& created = createdAtoms;
    const auto& surrounding = surroundingAtoms;
    const int atomIndex = firstEndpoint ? bond.atom1Index : bond.atom2Index;
    const uint32_t atomId = firstEndpoint ? bond.atom1Id : bond.atom2Id;

    if (atomId != 0) {
        for (const auto& atom : created) {
            if (atom.id == atomId) {
                return &atom;
            }
        }
        for (const auto& atom : surrounding) {
            if (atom.id == atomId) {
                return &atom;
            }
        }
    }

    if (atomIndex >= 0) {
        if (bond.bondType == atoms::domain::BondType::SURROUNDING) {
            if (atomIndex < static_cast<int>(surrounding.size())) {
                return &surrounding[atomIndex];
            }
            if (atomIndex < static_cast<int>(created.size())) {
                return &created[atomIndex];
            }
        } else {
            if (atomIndex < static_cast<int>(created.size())) {
                return &created[atomIndex];
            }
            if (atomIndex < static_cast<int>(surrounding.size())) {
                return &surrounding[atomIndex];
            }
        }
    } else {
        const int surroundingIndex = -atomIndex - 1;
        if (surroundingIndex >= 0 && surroundingIndex < static_cast<int>(surrounding.size())) {
            return &surrounding[surroundingIndex];
        }
    }

    return nullptr;
}

double calcDistance(const float lhs[3], const float rhs[3]) {
    return atoms::domain::calcDistance(lhs, rhs);
}

vtkSmartPointer<vtkActor> createMeasurementLineActor(
    const float lhs[3], const float rhs[3]) {
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    const vtkIdType id0 = points->InsertNextPoint(lhs[0], lhs[1], lhs[2]);
    const vtkIdType id1 = points->InsertNextPoint(rhs[0], rhs[1], rhs[2]);

    vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, id0);
    line->GetPointIds()->SetId(1, id1);

    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(line);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetPickable(false);
    actor->GetProperty()->SetColor(0.1, 0.1, 0.1);
    actor->GetProperty()->SetLineWidth(2.0);
    return actor;
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
    return atoms::domain::computeAngleMeasurementGeometry(
        point1,
        point2,
        point3,
        arcRadiusScale,
        angleDeg,
        arcStart,
        arcEnd,
        arcCenter,
        textPos);
}

vtkSmartPointer<vtkActor> createSegmentedDistanceLineActor(
    const float lhs[3], const float rhs[3]) {
    constexpr int kSegmentCount = 20;
    constexpr double kVisibleRatio = 0.55;

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    const double vx = static_cast<double>(rhs[0]) - static_cast<double>(lhs[0]);
    const double vy = static_cast<double>(rhs[1]) - static_cast<double>(lhs[1]);
    const double vz = static_cast<double>(rhs[2]) - static_cast<double>(lhs[2]);

    for (int i = 0; i < kSegmentCount; ++i) {
        const double t0 = static_cast<double>(i) / static_cast<double>(kSegmentCount);
        const double t1 = std::min(1.0, (static_cast<double>(i) + kVisibleRatio) /
                                            static_cast<double>(kSegmentCount));
        if (t1 <= t0) {
            continue;
        }

        const double p0[3] = {
            static_cast<double>(lhs[0]) + vx * t0,
            static_cast<double>(lhs[1]) + vy * t0,
            static_cast<double>(lhs[2]) + vz * t0
        };
        const double p1[3] = {
            static_cast<double>(lhs[0]) + vx * t1,
            static_cast<double>(lhs[1]) + vy * t1,
            static_cast<double>(lhs[2]) + vz * t1
        };

        const vtkIdType id0 = points->InsertNextPoint(p0);
        const vtkIdType id1 = points->InsertNextPoint(p1);

        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, id0);
        line->GetPointIds()->SetId(1, id1);
        lines->InsertNextCell(line);
    }

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetPickable(false);
    actor->GetProperty()->SetColor(0.1, 0.1, 0.1);
    actor->GetProperty()->SetLineWidth(2.0);

    return actor;
}

vtkSmartPointer<vtkActor> createAngleArcActor(
    const std::array<double, 3>& start,
    const std::array<double, 3>& end,
    const std::array<double, 3>& center) {
    vtkSmartPointer<vtkArcSource> source = vtkSmartPointer<vtkArcSource>::New();
    source->SetPoint1(start[0], start[1], start[2]);
    source->SetPoint2(end[0], end[1], end[2]);
    source->SetCenter(center[0], center[1], center[2]);
    source->SetResolution(64);
    source->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(source->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetPickable(false);
    actor->GetProperty()->SetColor(0.1, 0.1, 0.1);
    actor->GetProperty()->SetLineWidth(2.0);
    return actor;
}

vtkSmartPointer<vtkActor2D> createMeasurementValueTextActor(
    const std::string& text,
    const std::array<double, 3>& worldPosition) {
    vtkSmartPointer<vtkTextActor> actor = vtkSmartPointer<vtkTextActor>::New();
    actor->SetInput(text.c_str());

    vtkTextProperty* property = actor->GetTextProperty();
    if (property) {
        property->SetFontSize(kMeasurementValueBaseFontSize);
        property->SetColor(0.0, 0.0, 0.0);
        property->SetBackgroundColor(1.0, 1.0, 1.0);
        property->SetBackgroundOpacity(0.7);
        property->SetFrame(true);
        property->SetFrameColor(0.0, 0.0, 0.0);
        property->SetJustificationToCentered();
        property->SetVerticalJustificationToCentered();
    }

    vtkCoordinate* coord = actor->GetPositionCoordinate();
    coord->SetCoordinateSystemToWorld();
    coord->SetValue(worldPosition[0], worldPosition[1], worldPosition[2]);
    actor->SetPickable(false);

    return actor;
}

vtkSmartPointer<vtkActor2D> createDistanceTextActor(const std::string& text, const float lhs[3], const float rhs[3]) {
    const std::array<double, 3> midpoint = {
        (static_cast<double>(lhs[0]) + static_cast<double>(rhs[0])) * 0.5,
        (static_cast<double>(lhs[1]) + static_cast<double>(rhs[1])) * 0.5,
        (static_cast<double>(lhs[2]) + static_cast<double>(rhs[2])) * 0.5
    };
    return createMeasurementValueTextActor(text, midpoint);
}

std::string formatDistanceText(double distance) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4) << distance;
    return oss.str();
}

std::string formatAngleText(double angleDeg) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4) << angleDeg << "°";
    return oss.str();
}

std::string formatDihedralText(double angleDeg) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4) << angleDeg << "°";
    return oss.str();
}

std::string formatCenterCoordinateText(
    WorkspaceRuntimeModel::MeasurementType type,
    const std::array<double, 3>& centerPos) {
    const char* prefix = "Center(geom.)";
    if (type == WorkspaceRuntimeModel::MeasurementType::CenterOfMass) {
        prefix = "Center(mass)";
    }

    std::ostringstream oss;
    oss << prefix << " " << std::fixed << std::setprecision(4)
        << "(" << centerPos[0] << ", " << centerPos[1] << ", " << centerPos[2] << ")";
    return oss.str();
}

using DihedralGeometry = atoms::domain::MeasurementDihedralGeometry;

bool computeDihedralMeasurementGeometryLocal(
    const float point1[3],
    const float point2[3],
    const float point3[3],
    const float point4[3],
    DihedralGeometry& geometry) {
    return atoms::domain::computeDihedralMeasurementGeometry(
        point1,
        point2,
        point3,
        point4,
        geometry);
}

vtkSmartPointer<vtkActor> createDihedralHelperPlaneActor(
    const std::array<std::array<double, 3>, 4>& quad) {
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(4);
    for (vtkIdType i = 0; i < 4; ++i) {
        points->SetPoint(i, quad[static_cast<size_t>(i)].data());
    }

    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType quadCell[4] = { 0, 1, 2, 3 };
    polys->InsertNextCell(4, quadCell);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polys);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetPickable(false);
    vtkProperty* property = actor->GetProperty();
    if (property) {
        property->SetColor(0.2, 0.2, 0.2);
        property->SetOpacity(0.2);
        property->SetRepresentationToSurface();
        property->SetEdgeVisibility(false);
    }
    return actor;
}

vtkSmartPointer<vtkActor> createDihedralHelperArcActor(
    const std::vector<std::array<double, 3>>& arcPoints) {
    if (arcPoints.size() < 2) {
        return nullptr;
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(static_cast<vtkIdType>(arcPoints.size()));
    for (vtkIdType i = 0; i < static_cast<vtkIdType>(arcPoints.size()); ++i) {
        points->SetPoint(i, arcPoints[static_cast<size_t>(i)].data());
    }

    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(static_cast<vtkIdType>(arcPoints.size()));
    for (vtkIdType i = 0; i < static_cast<vtkIdType>(arcPoints.size()); ++i) {
        polyLine->GetPointIds()->SetId(i, i);
    }

    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(polyLine);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetPickable(false);
    vtkProperty* property = actor->GetProperty();
    if (property) {
        property->SetColor(0.1, 0.1, 0.1);
        property->SetLineWidth(2.0);
    }
    return actor;
}

std::array<double, 3> measurementOrderColor(size_t order) {
    switch (order) {
    case 1:
        return { 1.0, 0.92, 0.16 }; // yellow
    case 2:
        return { 0.2, 0.95, 1.0 }; // cyan
    case 3:
        return { 1.0, 0.35, 0.85 }; // magenta
    default:
        return { 0.65, 1.0, 0.4 }; // lime
    }
}

std::array<double, 3> measurementStyleColor(size_t styleIndex) {
    constexpr size_t kPaletteSize = 4;
    return measurementOrderColor((styleIndex % kPaletteSize) + 1);
}

double renderedSelectionSphereRadius(const atoms::domain::AtomInfo& atom) {
    return std::max(0.01, static_cast<double>(atom.radius) * 0.5 + 0.01);
}

int plusTextFontSizeFromRadius(double radiusWorld) {
    const int raw = static_cast<int>(std::lround(radiusWorld * 36.0));
    return std::clamp(raw, 18, 72);
}

vtkSmartPointer<vtkActor2D> createPlusTextActor(
    const std::array<double, 3>& worldPosition,
    double radiusWorld,
    const std::array<double, 3>& color) {
    vtkSmartPointer<vtkTextActor> actor = vtkSmartPointer<vtkTextActor>::New();
    actor->SetInput("+");

    vtkTextProperty* property = actor->GetTextProperty();
    if (property) {
        property->SetFontSize(plusTextFontSizeFromRadius(std::max(0.01, radiusWorld)));
        property->SetBold(true);
        property->SetColor(color[0], color[1], color[2]);
        property->SetJustificationToCentered();
        property->SetVerticalJustificationToCentered();
        property->SetBackgroundOpacity(0.0);
        property->SetFrame(false);
        property->SetShadow(false);
    }

    vtkCoordinate* coord = actor->GetPositionCoordinate();
    coord->SetCoordinateSystemToWorld();
    coord->SetValue(worldPosition[0], worldPosition[1], worldPosition[2]);
    actor->SetPickable(false);
    return actor;
}

vtkSmartPointer<vtkActor2D> createCircleTextActor(
    const std::array<double, 3>& worldPosition,
    double radiusWorld,
    const std::array<double, 3>& color,
    double opacity) {
    vtkSmartPointer<vtkTextActor> actor = vtkSmartPointer<vtkTextActor>::New();
    actor->SetInput("●");

    vtkTextProperty* property = actor->GetTextProperty();
    if (property) {
        const int plusFontSize = plusTextFontSizeFromRadius(std::max(0.01, radiusWorld));
        property->SetFontSize(static_cast<int>(std::lround(static_cast<double>(plusFontSize) * 1.45)));
        property->SetBold(true);
        property->SetColor(color[0], color[1], color[2]);
        property->SetOpacity(opacity);
        property->SetJustificationToCentered();
        property->SetVerticalJustificationToCentered();
        property->SetBackgroundOpacity(0.0);
        property->SetFrame(false);
        property->SetShadow(false);
    }

    vtkCoordinate* coord = actor->GetPositionCoordinate();
    coord->SetCoordinateSystemToWorld();
    coord->SetValue(worldPosition[0], worldPosition[1], worldPosition[2]);
    actor->SetPickable(false);
    return actor;
}

vtkSmartPointer<vtkActor> createMeasurementPickShellActor(
    const atoms::domain::AtomInfo& atom,
    size_t order) {
    vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
    const double renderedRadius = renderedSelectionSphereRadius(atom);
    const double radius = renderedRadius;
    source->SetRadius(radius);
    source->SetThetaResolution(24);
    source->SetPhiResolution(24);
    source->SetCenter(atom.position[0], atom.position[1], atom.position[2]);
    source->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(source->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetPickable(false);
    vtkProperty* property = actor->GetProperty();
    if (property) {
        const auto color = measurementStyleColor(order > 0 ? (order - 1) : 0);
        property->SetColor(color[0], color[1], color[2]);
        property->SetRepresentationToWireframe();
        property->SetLineWidth(2.0);
        property->SetAmbient(1.0);
        property->SetDiffuse(0.0);
        property->SetSpecular(0.0);
    }
    return actor;
}

vtkSmartPointer<vtkActor> createCreatedAtomSelectionShellActor(const atoms::domain::AtomInfo& atom) {
    vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
    const double renderedRadius = renderedSelectionSphereRadius(atom);
    source->SetRadius(renderedRadius);
    source->SetThetaResolution(24);
    source->SetPhiResolution(24);
    source->SetCenter(atom.position[0], atom.position[1], atom.position[2]);
    source->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(source->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetPickable(false);
    vtkProperty* property = actor->GetProperty();
    if (property) {
        property->SetColor(1.0, 1.0, 0.0);
        property->SetRepresentationToWireframe();
        property->SetLineWidth(2.0);
        property->SetAmbient(1.0);
        property->SetDiffuse(0.0);
        property->SetSpecular(0.0);
    }
    return actor;
}

vtkSmartPointer<vtkActor2D> createMeasurementOrderTextActor(
    const atoms::domain::AtomInfo& atom,
    size_t order) {
    vtkSmartPointer<vtkTextActor> actor = vtkSmartPointer<vtkTextActor>::New();
    std::string text = "#" + std::to_string(order);
    actor->SetInput(text.c_str());

    vtkTextProperty* property = actor->GetTextProperty();
    if (property) {
        const auto color = measurementStyleColor(order > 0 ? (order - 1) : 0);
        property->SetFontSize(32);
        property->SetColor(color[0], color[1], color[2]);
        property->SetBackgroundColor(0.0, 0.0, 0.0);
        property->SetBackgroundOpacity(0.45);
        property->SetFrame(true);
        property->SetFrameColor(color[0], color[1], color[2]);
        property->SetJustificationToCentered();
        property->SetVerticalJustificationToCentered();
    }

    vtkCoordinate* coord = actor->GetPositionCoordinate();
    coord->SetCoordinateSystemToWorld();
    const double renderedRadius = renderedSelectionSphereRadius(atom);
    coord->SetValue(
        atom.position[0],
        atom.position[1],
        atom.position[2] + std::max(0.02, renderedRadius * 1.25));
    actor->SetPickable(false);
    return actor;
}
} // namespace

// ============================================================================
// (251212) ATOM RENDERING SYSTEM
// ============================================================================

// (251211) 濡쒖쭅 ?뚰듃??domain layer濡?遺꾨━??
// addAtomToGroup, initializeAtomGroup, removeAtomFromGroup

/*
 * Called by applyAtomChanges, createAtomSphere
 * m_vtkRenderer ?묎렐???꾪븳 wrapper
*/
void initializeAtomGroup(const std::string& symbol, float radius) {
    // ?꾨찓?? 洹몃９ ?곗씠???앹꽦/?낅뜲?댄듃
    atoms::domain::initializeAtomGroup(symbol, radius);

    // ?ㅼ??ㅽ듃?덉씠?? ?뚮뜑留??뚯씠?꾨씪??珥덇린?붾뒗 WorkspaceRuntimeModel媛 ?대떦
    workspace::legacy::WorkspaceRuntimeModelRef().initializeUnifiedAtomGroupVTK(symbol, radius);
}

/**
 * 諛곗튂?낅뜲?댄듃 ?쒖뒪?쒖뿉???ъ슜?? * m_vtkRenderer ?묎렐???꾪븳 wrapper
 */
void updateUnifiedAtomGroupVTK(const std::string& symbol) {
    workspace::legacy::WorkspaceRuntimeModelRef().updateUnifiedAtomGroupVTK(symbol);
}

/*
 * Called by applyAtomChanges, LoadXSFFile, 
 *           createSurroundingAtoms, hideSurroundingAtoms
 * ?쒓굅 濡쒖쭅? domain??遺꾨━
*/
void removeAtomFromGroup(const std::string& symbol, uint32_t atomId) {
    // ?꾨찓?? 洹몃９ ?곗씠????젣
    bool removed = atoms::domain::removeAtomFromGroup(symbol, atomId);

    // ?ㅼ??ㅽ듃?덉씠?? VTK ?낅뜲?댄듃??WorkspaceRuntimeModel媛 ?ㅼ?以꾨쭅
    if (removed) {
        if (auto* batch = workspace::legacy::WorkspaceRuntimeModelRef().batchSystem()) {
            batch->scheduleAtomGroupUpdate(symbol);
        }
    }
}

// ============================================================================
// Bonds
// ============================================================================

static float bondScalingFactor = 1.0f; // Default scaling factor
static float bondToleranceFactor = 0.10f;
static float bondThickness = 1.0f;      // 寃고빀 ?먭퍡 (湲곕낯媛?1.0)
static float bondOpacity = 1.0f;        // 寃고빀 ?щ챸??(湲곕낯媛?1.0)


// ============================================================================
//  (251215) BOND RENDERING SYSTEM
// ============================================================================

/**
 * @brief 寃고빀 洹몃９ 珥덇린??(?꾩뿭 ?⑥닔 ?섑띁)
 */
void initializeBondGroup(const std::string& key, float radius) {
    // ?꾨찓?? 洹몃９ ?곗씠???앹꽦/?낅뜲?댄듃
    atoms::domain::initializeBondGroup(key, radius);

    // ?ㅼ??ㅽ듃?덉씠?? ?뚮뜑留??뚯씠?꾨씪??珥덇린?붾뒗 WorkspaceRuntimeModel媛 ?대떦
    workspace::legacy::WorkspaceRuntimeModelRef().initializeBondGroupVTK(key, radius);
}

/**
 * @brief 紐⑤뱺 寃고빀 洹몃９ ?뺣━ (?꾩뿭 ?⑥닔 ?섑띁)
 */
void clearAllBondGroups() {
    SPDLOG_INFO("Clearing all 2-color bond groups...");
    
    try {
        // VTKRenderer瑜??듯븳 VTK ?뚯씠?꾨씪???뺣━
        workspace::legacy::WorkspaceRuntimeModelRef().clearAllBondGroupsVTK();
        
        // bondGroups ?곗씠??援ъ“ ?뺣━
        atoms::domain::clearAllBondGroups();
        
        SPDLOG_INFO("All 2-color bond groups cleared successfully");
        
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Critical error clearing 2-color bond groups: {}", e.what());
    }
}

// ============================================================================
// ?앹꽦?? ?뚮㈇?? 硫붿씤 ?뚮뜑留??⑥닔 
// ============================================================================

WorkspaceRuntimeModel::WorkspaceRuntimeModel() {
    atoms::domain::setSurroundingsVisible(false);

    // *** 異붽?: ElementDatabase 珥덇린??***
    m_elementDB = &atoms::domain::ElementDatabase::getInstance();
    
    float identity[3][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };
    atoms::domain::setCellMatrix(identity);
    atoms::domain::setCellModified(false);
    
    // BatchUpdateSystem 초기화
    m_batchSystem = std::make_unique<atoms::infrastructure::BatchUpdateSystem>(this);

    // VTKRenderer 珥덇린??(Phase 2 異붽?)
    m_vtkRenderer = std::make_unique<atoms::infrastructure::VTKRenderer>();

    // BondRenderer 珥덇린??(CP2 異붽?)
    m_bondRenderer = std::make_unique<atoms::infrastructure::BondRenderer>(m_vtkRenderer.get());

    // [異붽?] FileIOManager 珥덇린??(Phase 3)
    m_fileIOManager = std::make_unique<atoms::infrastructure::FileIOManager>(this);

    // AtomEditorUI 초기화
    m_atomEditorUI = std::make_unique<atoms::ui::AtomEditorUI>(this);

    // BondUI 초기화
    m_bondUI = std::make_unique<atoms::ui::BondUI>(this);

    // CellInfoUI 초기화
    m_cellInfoUI = std::make_unique<atoms::ui::CellInfoUI>(this);

    // BZPlotUI 초기화
    m_bzPlotUI = std::make_unique<atoms::ui::BZPlotUI>(this);
    m_bzPlotController = std::make_unique<atoms::domain::BZPlotController>();

    // SurroundingAtomManager 珥덇린??(CP8 異붽?)
    m_surroundingAtomManager = std::make_unique<atoms::domain::SurroundingAtomManager>(this);

    // PeriodicTableUI 초기화
    m_periodicTableUI = std::make_unique<atoms::ui::PeriodicTableUI>(this);

    // BravaisLatticeUI 초기화
    m_bravaisLatticeUI = std::make_unique<atoms::ui::BravaisLatticeUI>(this);

    SPDLOG_DEBUG("WorkspaceRuntimeModel initialized with clean state");

    // ChargeDensityUI 초기화
    m_chargeDensityUI = std::make_unique<atoms::ui::ChargeDensityUI>(this);

    // Phase 17-R6 service facades (direct ownership path)
    m_structureService = std::make_unique<structure::application::StructureService>();
    m_measurementService = std::make_unique<measurement::application::MeasurementService>();
    m_densityService = std::make_unique<density::application::DensityService>();

    SPDLOG_DEBUG("WorkspaceRuntimeModel initialized with ChargeDensityUI");    
}

WorkspaceRuntimeModel::~WorkspaceRuntimeModel() {
    SPDLOG_INFO("WorkspaceRuntimeModel destructor called - starting cleanup");
    
    try {
        clearCreatedAtomSelectionVisuals();
        clearAllCenterMeasurements();
        clearAllDihedralMeasurements();
        clearAllAngleMeasurements();
        clearAllDistanceMeasurements();

        // VTKRenderer瑜??듯븳 ?뺣━
        if (m_vtkRenderer) {
            m_vtkRenderer->clearAllAtomGroupsVTK();
            m_vtkRenderer->clearAllBondGroups();
            m_vtkRenderer->clearUnitCell();
        }
        
        // ?곗씠??援ъ“ ?뺣━
        createdAtoms.clear();
        createdBonds.clear();
        surroundingAtoms.clear();
        surroundingBonds.clear();
        atomGroups.clear();
        bondGroups.clear();
        atoms::domain::setSurroundingsVisible(false);
        atoms::domain::setCellVisible(false);
        
        SPDLOG_INFO("WorkspaceRuntimeModel destructor completed successfully");
    
    // ChargeDensityUI 초기화
    m_chargeDensityUI = std::make_unique<atoms::ui::ChargeDensityUI>(this);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Critical exception in WorkspaceRuntimeModel destructor: {}", e.what());
    }
}

structure::application::StructureService& WorkspaceRuntimeModel::structureService() noexcept {
    return *m_structureService;
}

const structure::application::StructureService& WorkspaceRuntimeModel::structureService() const noexcept {
    return *m_structureService;
}

measurement::application::MeasurementService& WorkspaceRuntimeModel::measurementService() noexcept {
    return *m_measurementService;
}

const measurement::application::MeasurementService& WorkspaceRuntimeModel::measurementService() const noexcept {
    return *m_measurementService;
}

density::application::DensityService& WorkspaceRuntimeModel::densityService() noexcept {
    return *m_densityService;
}

const density::application::DensityService& WorkspaceRuntimeModel::densityService() const noexcept {
    return *m_densityService;
}

void WorkspaceRuntimeModel::ApplyAtomChangesFromEditor() {
    applyAtomChanges();
}

void WorkspaceRuntimeModel::ApplyCellChangesFromEditor() {
    applyCellChanges();
}

bool WorkspaceRuntimeModel::EnterBZPlotMode(
    const std::string& path,
    int npoints,
    bool showVectors,
    bool showLabels,
    std::string& outErrorMessage) {
    return enterBZPlotMode(path, npoints, showVectors, showLabels, outErrorMessage);
}

void WorkspaceRuntimeModel::ExitBZPlotMode() {
    exitBZPlotMode();
}

void WorkspaceRuntimeModel::UpdateBatchPerformanceStats(
    float duration,
    size_t atomGroupCount,
    size_t bondGroupCount) {
    updatePerformanceStatsInternal(duration, atomGroupCount, bondGroupCount);
}

void WorkspaceRuntimeModel::setCellVisible(bool visible) {
    for (auto& [id, entry] : m_UnitCells) {
        (void)id;
        entry.visible = visible;
    }
    if (m_vtkRenderer) {
        m_vtkRenderer->setUnitCellVisible(visible);
    }
    atoms::domain::setCellVisible(visible);
    requestViewerRender();
}

void WorkspaceRuntimeModel::SetAtomGroupVisible(const std::string& symbol, bool visible) {
    m_AtomGroupVisibility[symbol] = visible;
    
    if (m_vtkRenderer) {
        m_vtkRenderer->setAtomGroupVisible(symbol, visible);
    }
    requestViewerRender();
}


bool WorkspaceRuntimeModel::IsAtomGroupVisible(const std::string& symbol) const {
    auto it = m_AtomGroupVisibility.find(symbol);
    if (it != m_AtomGroupVisibility.end()) {
        return it->second;
    }
    // 湲곕낯媛? ?꾩껜 援ъ“ visibility ?곕쫫
    return m_StructureVisible;
}

bool WorkspaceRuntimeModel::IsAtomVisibleById(uint32_t atomId) const {
    auto it = m_AtomVisibilityById.find(atomId);
    if (it != m_AtomVisibilityById.end()) {
        return it->second;
    }
    return true;
}

void WorkspaceRuntimeModel::SetAtomVisibilityForIds(const std::vector<uint32_t>& atomIds, bool visible) {
    bool changed = false;
    for (uint32_t atomId : atomIds) {
        if (atomId == 0) {
            continue;
        }
        auto it = m_AtomVisibilityById.find(atomId);
        if (it == m_AtomVisibilityById.end() || it->second != visible) {
            m_AtomVisibilityById[atomId] = visible;
            changed = true;
        }
    }
    if (!changed) {
        return;
    }
    refreshRenderedGroups();
    requestViewerRender();
}

bool WorkspaceRuntimeModel::IsAtomLabelVisibleById(uint32_t atomId) const {
    auto it = m_AtomLabelVisibilityById.find(atomId);
    if (it != m_AtomLabelVisibilityById.end()) {
        return it->second;
    }
    return false;
}

void WorkspaceRuntimeModel::SetAtomLabelVisibilityForIds(const std::vector<uint32_t>& atomIds, bool visible) {
    bool changed = false;
    for (uint32_t atomId : atomIds) {
        if (atomId == 0) {
            continue;
        }
        auto it = m_AtomLabelVisibilityById.find(atomId);
        if (it == m_AtomLabelVisibilityById.end() || it->second != visible) {
            m_AtomLabelVisibilityById[atomId] = visible;
            changed = true;
        }
    }
    if (!changed) {
        return;
    }
    refreshLabelActors();
    requestViewerRender();
}

void WorkspaceRuntimeModel::SetBondsVisible(bool visible) {
    m_BondsVisible = visible;
    for (const auto& [id, entry] : m_Structures) {
        (void)entry;
        m_StructureBondsVisible[id] = visible;
    }
    refreshRenderedGroups();
    requestViewerRender();
}

void WorkspaceRuntimeModel::SetBondsVisible(int32_t id, bool visible) {
    if (id < 0) {
        SetBondsVisible(visible);
        return;
    }
    m_StructureBondsVisible[id] = visible;
    refreshRenderedGroups();
    requestViewerRender();
}

bool WorkspaceRuntimeModel::IsBondsVisible(int32_t id) const {
    if (id < 0) {
        return m_BondsVisible;
    }
    auto it = m_StructureBondsVisible.find(id);
    if (it != m_StructureBondsVisible.end()) {
        return it->second;
    }
    return m_BondsVisible;
}

bool WorkspaceRuntimeModel::IsBondVisibleById(uint32_t bondId) const {
    auto it = m_BondVisibilityById.find(bondId);
    if (it != m_BondVisibilityById.end()) {
        return it->second;
    }
    return true;
}

void WorkspaceRuntimeModel::SetBondVisibilityForIds(const std::vector<uint32_t>& bondIds, bool visible) {
    bool changed = false;
    for (uint32_t bondId : bondIds) {
        if (bondId == 0) {
            continue;
        }
        auto it = m_BondVisibilityById.find(bondId);
        if (it == m_BondVisibilityById.end() || it->second != visible) {
            m_BondVisibilityById[bondId] = visible;
            changed = true;
        }
    }
    if (!changed) {
        return;
    }
    refreshRenderedGroups();
    requestViewerRender();
}

bool WorkspaceRuntimeModel::IsBondLabelVisibleById(uint32_t bondId) const {
    auto it = m_BondLabelVisibilityById.find(bondId);
    if (it != m_BondLabelVisibilityById.end()) {
        return it->second;
    }
    return false;
}

void WorkspaceRuntimeModel::SetBondLabelVisibilityForIds(const std::vector<uint32_t>& bondIds, bool visible) {
    bool changed = false;
    for (uint32_t bondId : bondIds) {
        if (bondId == 0) {
            continue;
        }
        auto it = m_BondLabelVisibilityById.find(bondId);
        if (it == m_BondLabelVisibilityById.end() || it->second != visible) {
            m_BondLabelVisibilityById[bondId] = visible;
            changed = true;
        }
    }
    if (!changed) {
        return;
    }
    refreshLabelActors();
    requestViewerRender();
}

std::vector<std::pair<std::string, size_t>> WorkspaceRuntimeModel::GetAtomGroupSummary() const {
    std::vector<std::pair<std::string, size_t>> summary;
    for (const auto& [symbol, group] : atomGroups) {
        // createdAtoms에서 해당 symbol의 개수 카운트
        size_t count = 0;
        for (const auto& atom : createdAtoms) {
            if (atom.symbol == symbol) {
                count++;
            }
        }
        if (count > 0) {
            summary.emplace_back(symbol, count);
        }
    }
    return summary;
}

void WorkspaceRuntimeModel::RequestBuilderSection(workbench::panel::BuilderRequest section) {
    switch (section) {
    case workbench::panel::BuilderRequest::BravaisLatticeTemplates:
        m_PendingBuilderSection = PendingBuilderSection::BravaisLatticeTemplates;
        break;
    case workbench::panel::BuilderRequest::AddAtoms:
        m_PendingBuilderSection = PendingBuilderSection::AddAtoms;
        break;
    case workbench::panel::BuilderRequest::BrillouinZone:
        m_PendingBuilderSection = PendingBuilderSection::BrillouinZone;
        break;
    }
}

void WorkspaceRuntimeModel::RequestEditorSection(workbench::panel::EditorRequest section) {
    switch (section) {
    case workbench::panel::EditorRequest::Atoms:
        m_PendingEditorSection = PendingEditorSection::Atoms;
        break;
    case workbench::panel::EditorRequest::Bonds:
        m_PendingEditorSection = PendingEditorSection::Bonds;
        break;
    case workbench::panel::EditorRequest::Cell:
        m_PendingEditorSection = PendingEditorSection::Cell;
        break;
    }
}

WorkspaceRuntimeModel::ChargeDensityAdvancedGridVisibilityState&
WorkspaceRuntimeModel::ensureChargeDensityAdvancedGridVisibilityState(int32_t meshId) {
    auto it = m_ChargeDensityAdvancedGridVisibility.find(meshId);
    if (it != m_ChargeDensityAdvancedGridVisibility.end()) {
        return it->second;
    }

    ChargeDensityAdvancedGridVisibilityState state {};
    Mesh* mesh = mesh::application::GetMeshQueryService().FindMeshByIdMutable(meshId);
    if (mesh && mesh->GetVolumeMeshActor()) {
        state.surfaceVisible = mesh->GetVolumeSurfaceVisibility();
        state.volumeVisible = mesh->GetVolumeRenderVisibility();
    }

    auto inserted = m_ChargeDensityAdvancedGridVisibility.emplace(meshId, state);
    return inserted.first->second;
}

bool WorkspaceRuntimeModel::getChargeDensityAdvancedGridVisibilityValue(
    const ChargeDensityAdvancedGridVisibilityState& state, bool volumeMode) {
    return volumeMode ? state.volumeVisible : state.surfaceVisible;
}

void WorkspaceRuntimeModel::setChargeDensityAdvancedGridVisibilityValue(
    ChargeDensityAdvancedGridVisibilityState& state, bool volumeMode, bool visible) {
    if (volumeMode) {
        state.volumeVisible = visible;
    } else {
        state.surfaceVisible = visible;
    }
}

void WorkspaceRuntimeModel::captureChargeDensityAdvancedGridVisibilityFromMesh(int32_t meshId, Mesh* mesh) {
    if (!mesh || !mesh->GetVolumeMeshActor()) {
        return;
    }
    const TreeNode* node = mesh::application::GetMeshQueryService().MeshTree()->GetTreeNodeById(meshId);
    if (node && node->GetParent() && node->GetParent()->GetId() > 0) {
        if (!IsStructureVisible(node->GetParent()->GetId())) {
            return;
        }
    }
    ChargeDensityAdvancedGridVisibilityState& state =
        ensureChargeDensityAdvancedGridVisibilityState(meshId);
    state.surfaceVisible = mesh->GetVolumeSurfaceVisibility();
    state.volumeVisible = mesh->GetVolumeRenderVisibility();
}

void WorkspaceRuntimeModel::setChargeDensityGridMeshVisibilityImmediate(
    int32_t meshId, bool surfaceVisible, bool volumeVisible) {
    auto& meshQueryService = mesh::application::GetMeshQueryService();
    Mesh* mesh = meshQueryService.FindMeshByIdMutable(meshId);
    if (!mesh || !mesh->GetVolumeMeshActor()) {
        return;
    }

    const bool anyVisible = surfaceVisible || volumeVisible;
    if (anyVisible) {
        MeshDetail::Instance().SetUiVolumeMeshVisibility(true);
    }
    mesh->SetVolumeSurfaceVisibility(surfaceVisible);
    mesh->SetVolumeRenderVisibility(volumeVisible);
    mesh->SetVolumeMeshVisibility(anyVisible);

    TreeNode* node = meshQueryService.MeshTree()->GetTreeNodeByIdMutable(meshId);
    if (!node) {
        return;
    }
    if (surfaceVisible && volumeVisible) {
        node->SetIconState(IconState::VISIBLE);
    } else if (!surfaceVisible && !volumeVisible) {
        node->SetIconState(IconState::HIDDEN);
    } else {
        node->SetIconState(IconState::PARTIAL);
    }
    mesh::domain::GetMeshRepository().SetParentIconState(node);
}

void WorkspaceRuntimeModel::applyChargeDensityAdvancedGridVisibilityToMesh(int32_t meshId, Mesh* mesh) {
    if (!mesh || !mesh->GetVolumeMeshActor()) {
        return;
    }
    ChargeDensityAdvancedGridVisibilityState& state =
        ensureChargeDensityAdvancedGridVisibilityState(meshId);
    bool surfaceVisible = state.surfaceVisible;
    bool volumeVisible = state.volumeVisible;

    const TreeNode* node = mesh::application::GetMeshQueryService().MeshTree()->GetTreeNodeById(meshId);
    if (node && node->GetParent() && node->GetParent()->GetId() > 0) {
        if (!IsStructureVisible(node->GetParent()->GetId())) {
            surfaceVisible = false;
            volumeVisible = false;
        }
    }

    setChargeDensityGridMeshVisibilityImmediate(meshId, surfaceVisible, volumeVisible);
}

bool WorkspaceRuntimeModel::IsChargeDensityAdvancedGridVisible(int32_t meshId, bool volumeMode) {
    if (meshId < 0) {
        return false;
    }
    const ChargeDensityAdvancedGridVisibilityState& state =
        ensureChargeDensityAdvancedGridVisibilityState(meshId);
    return getChargeDensityAdvancedGridVisibilityValue(state, volumeMode);
}

void WorkspaceRuntimeModel::SetChargeDensityAdvancedGridVisible(
    int32_t meshId, bool volumeMode, bool visible) {
    if (meshId < 0) {
        return;
    }

    ChargeDensityAdvancedGridVisibilityState& state =
        ensureChargeDensityAdvancedGridVisibilityState(meshId);
    setChargeDensityAdvancedGridVisibilityValue(state, volumeMode, visible);

    Mesh* mesh = mesh::application::GetMeshQueryService().FindMeshByIdMutable(meshId);
    if (!mesh || !mesh->GetVolumeMeshActor()) {
        return;
    }
    applyChargeDensityAdvancedGridVisibilityToMesh(meshId, mesh);
}

void WorkspaceRuntimeModel::SetAllChargeDensityAdvancedGridVisible(
    int32_t structureId, bool volumeMode, bool visible) {
    if (structureId < 0) {
        return;
    }

    const auto& meshQueryService = mesh::application::GetMeshQueryService();
    const TreeNode* structureNode = meshQueryService.MeshTree()->GetTreeNodeById(structureId);
    if (!structureNode) {
        return;
    }

    const TreeNode* child = structureNode->GetLeftChild();
    while (child) {
        Mesh* mesh = meshQueryService.FindMeshByIdMutable(child->GetId());
        if (mesh && !mesh->IsXsfStructure() && mesh->GetVolumeMeshActor()) {
            SetChargeDensityAdvancedGridVisible(child->GetId(), volumeMode, visible);
        }
        child = child->GetRightSibling();
    }
}

void WorkspaceRuntimeModel::ApplyChargeDensityAdvancedGridVisibilityForStructure(int32_t structureId) {
    if (structureId < 0) {
        return;
    }

    const auto& meshQueryService = mesh::application::GetMeshQueryService();
    const TreeNode* structureNode = meshQueryService.MeshTree()->GetTreeNodeById(structureId);
    if (!structureNode) {
        return;
    }

    const TreeNode* child = structureNode->GetLeftChild();
    while (child) {
        Mesh* mesh = meshQueryService.FindMeshByIdMutable(child->GetId());
        if (mesh && !mesh->IsXsfStructure() && mesh->GetVolumeMeshActor()) {
            applyChargeDensityAdvancedGridVisibilityToMesh(child->GetId(), mesh);
        }
        child = child->GetRightSibling();
    }
}

void WorkspaceRuntimeModel::setCurrentStructureGridRenderMode(bool volumeMode) {
    int32_t targetStructureId = m_CurrentStructureId;
    if (targetStructureId < 0) {
        targetStructureId = m_ChargeDensityStructureId;
    }
    if (targetStructureId < 0) {
        return;
    }

    const Mesh::VolumeRenderMode mode = volumeMode
        ? Mesh::VolumeRenderMode::Volume
        : Mesh::VolumeRenderMode::Surface;

    auto& meshQueryService = mesh::application::GetMeshQueryService();
    meshQueryService.MeshTree()->TraverseTree([&](const TreeNode* node, void*) {
        if (!node) {
            return;
        }
        const int32_t id = node->GetId();
        if (id <= 0) {
            return;
        }
        const TreeNode* parent = node->GetParent();
        if (!parent || parent->GetId() != targetStructureId) {
            return;
        }
        Mesh* mesh = meshQueryService.FindMeshByIdMutable(id);
        if (!mesh || !mesh->GetVolumeMeshActor()) {
            return;
        }
        captureChargeDensityAdvancedGridVisibilityFromMesh(id, mesh);
        mesh->SetVolumeRenderMode(mode);
        applyChargeDensityAdvancedGridVisibilityToMesh(id, mesh);
    });
}

void WorkspaceRuntimeModel::RequestDataMenu(workbench::panel::DataRequest request) {
    switch (request) {
    case workbench::panel::DataRequest::Isosurface:
        m_ChargeDensityViewType = ChargeDensityViewType::Simple;
        applyChargeDensityViewType();
        m_PendingAdvancedSection = PendingAdvancedSection::ChargeDensity;
        break;
    case workbench::panel::DataRequest::Surface:
        m_ChargeDensityViewType = ChargeDensityViewType::Advanced;
        applyChargeDensityViewType();
        setCurrentStructureGridRenderMode(false);
        m_PendingAdvancedSection = PendingAdvancedSection::ChargeDensity;
        break;
    case workbench::panel::DataRequest::Volumetric:
        m_ChargeDensityViewType = ChargeDensityViewType::Advanced;
        applyChargeDensityViewType();
        setCurrentStructureGridRenderMode(true);
        m_PendingAdvancedSection = PendingAdvancedSection::ChargeDensity;
        break;
    case workbench::panel::DataRequest::Plane:
        m_ChargeDensityViewType = ChargeDensityViewType::Advanced;
        applyChargeDensityViewType();
        if (m_chargeDensityUI) {
            m_chargeDensityUI->setSliceVisible(true);
        }
        m_PendingAdvancedSection = PendingAdvancedSection::SliceViewer;
        break;
    }
}

void WorkspaceRuntimeModel::RequestForcedBuilderWindowLayout(const ImVec2& pos, const ImVec2& size, int frames) {
    m_BuilderWindowLayoutRequest.enabled = true;
    m_BuilderWindowLayoutRequest.pos = pos;
    m_BuilderWindowLayoutRequest.size = size;
    m_BuilderWindowLayoutRequest.framesRemaining = std::max(frames, 1);
}

void WorkspaceRuntimeModel::RequestForcedEditorWindowLayout(const ImVec2& pos, const ImVec2& size, int frames) {
    m_EditorWindowLayoutRequest.enabled = true;
    m_EditorWindowLayoutRequest.pos = pos;
    m_EditorWindowLayoutRequest.size = size;
    m_EditorWindowLayoutRequest.framesRemaining = std::max(frames, 1);
}

void WorkspaceRuntimeModel::RequestForcedAdvancedWindowLayout(const ImVec2& pos, const ImVec2& size, int frames) {
    m_AdvancedWindowLayoutRequest.enabled = true;
    m_AdvancedWindowLayoutRequest.pos = pos;
    m_AdvancedWindowLayoutRequest.size = size;
    m_AdvancedWindowLayoutRequest.framesRemaining = std::max(frames, 1);
}

void WorkspaceRuntimeModel::applyForcedWindowLayout(WindowLayoutRequest& request) {
    if (!request.enabled || request.framesRemaining <= 0) {
        return;
    }

    ImGui::SetNextWindowDockID(0, ImGuiCond_Always);
    ImGui::SetNextWindowPos(request.pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(request.size, ImGuiCond_Always);

    --request.framesRemaining;
    if (request.framesRemaining <= 0) {
        request.enabled = false;
    }
}

void WorkspaceRuntimeModel::renderChargeDensityViewerContent() {
    if (m_PendingAdvancedSection == PendingAdvancedSection::ChargeDensity) {
        m_PendingAdvancedSection = PendingAdvancedSection::None;
    }

    applyChargeDensityViewType();
    ImGui::Spacing();
    if (m_chargeDensityUI) {
        m_chargeDensityUI->renderDataInformation();
        ImGui::Spacing();
    }

    if (m_ChargeDensityViewType == ChargeDensityViewType::Simple) {
        m_ActiveChargeDensityGridMeshId = -1;
        if (m_chargeDensityUI) {
            m_chargeDensityUI->render();
        }
        return;
    }

    auto& meshQueryService = mesh::application::GetMeshQueryService();
    std::vector<int32_t> volumeMeshIds;
    std::vector<std::string> volumeMeshNames;

    meshQueryService.MeshTree()->TraverseTree([&](const TreeNode* node, void*) {
        if (!node) {
            return;
        }
        int32_t id = node->GetId();
        if (id <= 0) {
            return;
        }
        const TreeNode* parent = node->GetParent();
        if (!parent || parent->GetId() == 0) {
            return;
        }
        const Mesh* parentMesh = meshQueryService.FindMeshById(parent->GetId());
        if (!parentMesh || !parentMesh->IsXsfStructure()) {
            return;
        }
        Mesh* mesh = meshQueryService.FindMeshByIdMutable(id);
        if (!mesh || !mesh->GetVolumeMeshActor()) {
            return;
        }
        volumeMeshIds.push_back(id);
        volumeMeshNames.emplace_back(mesh->GetName());
    });

    if (volumeMeshIds.empty()) {
        m_ActiveChargeDensityGridMeshId = -1;
        ImGui::TextDisabled("No grid volume data loaded");
        return;
    }

    static int selectedIndex = 0;
    static int32_t lastTreeSelection = -1;

    int32_t treeSelected = ModelTree::Instance().GetSelectedMeshId();
    if (treeSelected != -1 && treeSelected != lastTreeSelection) {
        auto it = std::find(volumeMeshIds.begin(), volumeMeshIds.end(), treeSelected);
        if (it != volumeMeshIds.end()) {
            selectedIndex = static_cast<int>(std::distance(volumeMeshIds.begin(), it));
        }
        lastTreeSelection = treeSelected;
    }

    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(volumeMeshIds.size())) {
        selectedIndex = 0;
    }

    std::vector<const char*> nameCstrs;
    nameCstrs.reserve(volumeMeshNames.size());
    for (const auto& name : volumeMeshNames) {
        nameCstrs.push_back(name.c_str());
    }

    ImGui::Text("Mesh");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##ChargeDensityMeshSelect", &selectedIndex, nameCstrs.data(),
                 static_cast<int>(nameCstrs.size()));

    int32_t activeMeshId = volumeMeshIds[selectedIndex];
    m_ActiveChargeDensityGridMeshId = activeMeshId;
    MeshDetail::Instance().RenderVolumeControls(
        activeMeshId, "ChargeDensityVolumeControls", false);
}

void WorkspaceRuntimeModel::renderSliceViewerContent() {
    if (m_PendingAdvancedSection == PendingAdvancedSection::SliceViewer) {
        m_PendingAdvancedSection = PendingAdvancedSection::None;
    }

    if (m_chargeDensityUI) {
        if (m_ActiveChargeDensityGridMeshId > 0) {
            const Mesh* activeMesh =
                mesh::application::GetMeshQueryService().FindMeshById(m_ActiveChargeDensityGridMeshId);
            if (activeMesh && activeMesh->GetVolumeMeshActor()) {
                const double* range = activeMesh->GetVolumeDataRange();
                const double dataMin = (range != nullptr) ? range[0] : 0.0;
                const double dataMax = (range != nullptr) ? range[1] : 1.0;
                m_chargeDensityUI->syncSliceDisplayFromVolume(
                    activeMesh->GetName(),
                    activeMesh->GetVolumeWindow(),
                    activeMesh->GetVolumeLevel(),
                    activeMesh->GetVolumeColorMidpoint(),
                    activeMesh->GetVolumeColorSharpness(),
                    dataMin,
                    dataMax);
            } else {
                m_ActiveChargeDensityGridMeshId = -1;
            }
        }
        m_chargeDensityUI->renderSliceViewer();
    } else {
        ImGui::TextDisabled("No charge density data loaded");
    }
}

void WorkspaceRuntimeModel::RenderChargeDensityViewerWindow(bool* openWindow) {
    if (openWindow != nullptr && !*openWindow) {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (viewport != nullptr) {
        const ImVec2 size(viewport->WorkSize.x * 0.3f, viewport->WorkSize.y * 0.7f);
        const ImVec2 pos(viewport->WorkPos.x + viewport->WorkSize.x - size.x, viewport->WorkPos.y);
        ImGui::SetNextWindowDockID(0, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
    } else {
        ImGui::SetNextWindowSize(ImVec2(480, 500), ImGuiCond_FirstUseEver);
    }
    applyForcedWindowLayout(m_AdvancedWindowLayoutRequest);
    if (ImGui::Begin("Charge Density Viewer", openWindow)) {
        renderChargeDensityViewerContent();
    }
    ImGui::End();
}

void WorkspaceRuntimeModel::RenderSliceViewerWindow(bool* openWindow) {
    if (openWindow != nullptr && !*openWindow) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
    applyForcedWindowLayout(m_AdvancedWindowLayoutRequest);
    if (ImGui::Begin("2D Slice Viewer", openWindow)) {
        renderSliceViewerContent();
    }
    ImGui::End();
}





void WorkspaceRuntimeModel::Render(bool* openBuilder, bool* openEditor) {
    auto applyForcedWindowLayout = [](WindowLayoutRequest& request) {
        if (!request.enabled || request.framesRemaining <= 0) {
            return;
        }

        ImGui::SetNextWindowDockID(0, ImGuiCond_Always);
        ImGui::SetNextWindowPos(request.pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(request.size, ImGuiCond_Always);

        --request.framesRemaining;
        if (request.framesRemaining <= 0) {
            request.enabled = false;
        }
    };

    auto requestBuilderSectionOpen = [&](PendingBuilderSection section) {
        if (m_PendingBuilderSection == section) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            m_PendingBuilderSection = PendingBuilderSection::None;
        }
    };
    auto requestEditorSectionOpen = [&](PendingEditorSection section) {
        if (m_PendingEditorSection == section) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            m_PendingEditorSection = PendingEditorSection::None;
        }
    };

    auto renderBuilderContents = [&]() {
        requestBuilderSectionOpen(PendingBuilderSection::BravaisLatticeTemplates);
        if (ImGui::CollapsingHeader("Crystal Templates")) {
            if (m_bravaisLatticeUI) {
                m_bravaisLatticeUI->render();
            }
        }

        requestBuilderSectionOpen(PendingBuilderSection::AddAtoms);
        if (ImGui::CollapsingHeader("Periodic Table")) {
            if (m_periodicTableUI) {
                m_periodicTableUI->render();
            }
        }

        requestBuilderSectionOpen(PendingBuilderSection::BrillouinZone);
        if (ImGui::CollapsingHeader("Brillouin Zone Plot")) {
            if (m_bzPlotUI) {
                m_bzPlotUI->render();
            }
        }
    };

    auto renderEditorContents = [&]() {
        requestEditorSectionOpen(PendingEditorSection::Atoms);
        if (ImGui::CollapsingHeader("Created Atoms")) {
            if (m_atomEditorUI) {
                m_atomEditorUI->render();
            }
        }

        requestEditorSectionOpen(PendingEditorSection::Bonds);
        if (ImGui::CollapsingHeader("Bonds Management")) {
            if (m_bondUI) {
                m_bondUI->render();
            }
        }

        requestEditorSectionOpen(PendingEditorSection::Cell);
        if (ImGui::CollapsingHeader("Cell Information")) {
            if (m_cellInfoUI) {
                m_cellInfoUI->render();
            }
        }

    };

    // Crystal Builder 창
    if (openBuilder != nullptr) {
        if (*openBuilder) {
            // ??李??ш린 ?ㅼ젙 (泥섏쓬 ?대┫ ?뚮쭔 ?곸슜)
            ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
            applyForcedWindowLayout(m_BuilderWindowLayoutRequest);
            
            if (ImGui::Begin("Crystal Builder", openBuilder)) {
                renderBuilderContents();
            }
            ImGui::End();
        }
    }
    else {
        ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
        applyForcedWindowLayout(m_BuilderWindowLayoutRequest);
        
        if (ImGui::Begin("Crystal Builder")) {
            renderBuilderContents();
        }
        ImGui::End();
    }

    // Crystal Editor 창
    if (openEditor != nullptr) {
        if (*openEditor) {
            // ??李??ш린 ?ㅼ젙 (泥섏쓬 ?대┫ ?뚮쭔 ?곸슜)
            ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
            applyForcedWindowLayout(m_EditorWindowLayoutRequest);
            
            if (ImGui::Begin("Crystal Editor", openEditor)) {
                renderEditorContents();
            }
            ImGui::End();
        }
    }
    else {
        ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
        applyForcedWindowLayout(m_EditorWindowLayoutRequest);
        
        if (ImGui::Begin("Crystal Editor")) {
            renderEditorContents();
        }
        ImGui::End();
    }
}

void WorkspaceRuntimeModel::RenderAdvancedView(bool* openAdvanced) {
    auto applyForcedWindowLayout = [&](WindowLayoutRequest& request) {
        if (!request.enabled || request.framesRemaining <= 0) {
            return;
        }

        ImGui::SetNextWindowDockID(0, ImGuiCond_Always);
        ImGui::SetNextWindowPos(request.pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(request.size, ImGuiCond_Always);

        --request.framesRemaining;
        if (request.framesRemaining <= 0) {
            request.enabled = false;
        }
    };

    auto renderChargeDensityViewer = [&]() {
        if (m_PendingAdvancedSection == PendingAdvancedSection::ChargeDensity) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            m_PendingAdvancedSection = PendingAdvancedSection::None;
        }
        if (ImGui::CollapsingHeader("Charge Density Viewer", ImGuiTreeNodeFlags_DefaultOpen)) {
            applyChargeDensityViewType();
            ImGui::Spacing();
            if (m_chargeDensityUI) {
                m_chargeDensityUI->renderDataInformation();
                ImGui::Spacing();
            }

            if (m_ChargeDensityViewType == ChargeDensityViewType::Simple) {
                if (m_chargeDensityUI) {
                    m_chargeDensityUI->render();
                }
            } else {
                auto& meshQueryService = mesh::application::GetMeshQueryService();
                std::vector<int32_t> volumeMeshIds;
                std::vector<std::string> volumeMeshNames;

                meshQueryService.MeshTree()->TraverseTree([&](const TreeNode* node, void*) {
                    if (!node) {
                        return;
                    }
                    int32_t id = node->GetId();
                    if (id <= 0) {
                        return;
                    }
                    const TreeNode* parent = node->GetParent();
                    if (!parent || parent->GetId() == 0) {
                        return;
                    }
                    const Mesh* parentMesh = meshQueryService.FindMeshById(parent->GetId());
                    if (!parentMesh || !parentMesh->IsXsfStructure()) {
                        return;
                    }
                    Mesh* mesh = meshQueryService.FindMeshByIdMutable(id);
                    if (!mesh || !mesh->GetVolumeMeshActor()) {
                        return;
                    }
                    volumeMeshIds.push_back(id);
                    volumeMeshNames.emplace_back(mesh->GetName());
                });

                if (volumeMeshIds.empty()) {
                    ImGui::TextDisabled("No grid volume data loaded");
                } else {
                    static int selectedIndex = 0;
                    static int32_t lastTreeSelection = -1;

                    int32_t treeSelected = ModelTree::Instance().GetSelectedMeshId();
                    if (treeSelected != -1 && treeSelected != lastTreeSelection) {
                        auto it = std::find(volumeMeshIds.begin(), volumeMeshIds.end(), treeSelected);
                        if (it != volumeMeshIds.end()) {
                            selectedIndex = static_cast<int>(std::distance(volumeMeshIds.begin(), it));
                        }
                        lastTreeSelection = treeSelected;
                    }

                    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(volumeMeshIds.size())) {
                        selectedIndex = 0;
                    }

                    std::vector<const char*> nameCstrs;
                    nameCstrs.reserve(volumeMeshNames.size());
                    for (const auto& name : volumeMeshNames) {
                        nameCstrs.push_back(name.c_str());
                    }

                    ImGui::Text("Mesh");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(-1);
                    ImGui::Combo("##ChargeDensityMeshSelect", &selectedIndex, nameCstrs.data(),
                                 static_cast<int>(nameCstrs.size()));

                    int32_t activeMeshId = volumeMeshIds[selectedIndex];
                    MeshDetail::Instance().RenderVolumeControls(
                        activeMeshId, "ChargeDensityVolumeControls", false);
                }
            }
        }

        ImGui::Spacing();
        if (m_PendingAdvancedSection == PendingAdvancedSection::SliceViewer) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            m_PendingAdvancedSection = PendingAdvancedSection::None;
        }
        if (ImGui::CollapsingHeader("2D Slice Viewer", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (m_chargeDensityUI) {
                m_chargeDensityUI->renderSliceViewer();
            } else {
                ImGui::TextDisabled("No charge density data loaded");
            }
        }
    };

    if (openAdvanced != nullptr) {
        if (*openAdvanced) {
            ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
            applyForcedWindowLayout(m_AdvancedWindowLayoutRequest);
            if (ImGui::Begin("Advanced View", openAdvanced)) {
                renderChargeDensityViewer();
            }
            ImGui::End();
        }
    } else {
        ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
        applyForcedWindowLayout(m_AdvancedWindowLayoutRequest);
        if (ImGui::Begin("Advanced View")) {
            renderChargeDensityViewer();
        }
        ImGui::End();
    }
}

void WorkspaceRuntimeModel::applyChargeDensityViewType() {
    if (!m_ChargeDensityViewTypeInitialized) {
        m_ChargeDensityViewTypeInitialized = true;
    }
    // View type controls the panel layout only; actor visibility is managed independently.
    setChargeDensityVolumeSuppressed(false);
    setGridChargeDensitySuppressed(false);
    m_LastChargeDensityViewType = m_ChargeDensityViewType;
}

void WorkspaceRuntimeModel::setChargeDensityVolumeSuppressed(bool suppressed) {
    if (m_ChargeDensityVolumeMeshId < 0) {
        return;
    }
    Mesh* mesh = mesh::application::GetMeshQueryService().FindMeshByIdMutable(m_ChargeDensityVolumeMeshId);
    if (!mesh || !mesh->GetVolumeMeshActor()) {
        return;
    }
    mesh->SetVolumeMeshSuppressed(suppressed);
}

void WorkspaceRuntimeModel::setGridChargeDensitySuppressed(bool suppressed) {
    auto& meshQueryService = mesh::application::GetMeshQueryService();
    meshQueryService.MeshTree()->TraverseTree([&](const TreeNode* node, void*) {
        if (!node) {
            return;
        }
        int32_t id = node->GetId();
        if (id <= 0) {
            return;
        }
        if (m_ChargeDensityVolumeMeshId >= 0 && id == m_ChargeDensityVolumeMeshId) {
            return;
        }
        const TreeNode* parent = node->GetParent();
        if (!parent || parent->GetId() == 0) {
            return;
        }
        const Mesh* parentMesh = meshQueryService.FindMeshById(parent->GetId());
        if (!parentMesh || !parentMesh->IsXsfStructure()) {
            return;
        }
        Mesh* mesh = meshQueryService.FindMeshByIdMutable(id);
        if (!mesh || !mesh->GetVolumeMeshActor()) {
            return;
        }
        mesh->SetVolumeMeshSuppressed(suppressed);
    });
}

void WorkspaceRuntimeModel::LoadXSFFile(const std::string& filePath) {
    SPDLOG_INFO("Loading XSF file with FileIOManager: {}", filePath);

    if (m_bzPlotUI && m_bzPlotUI->IsShowingBZ()) {
        exitBZPlotMode();
        m_bzPlotUI->SetShowingBZ(false);
    }

    // 기존 구조 초기화
    BatchGuard guard = createBatchGuard();

    if (!m_fileIOManager) {
        SPDLOG_ERROR("FileIOManager not initialized");
        throw std::runtime_error("FileIOManager not initialized");
    }

    std::string errorMessage;
    std::vector<uint32_t> newAtomIds;
    bool ok = m_fileIOManager->initializeStructureFromXSF(filePath, this, errorMessage, &newAtomIds);
    if (!ok) {
        SPDLOG_ERROR("Error during XSF file loading: {}", errorMessage);
        throw std::runtime_error(errorMessage.empty() ? "XSF loading failed" : errorMessage);
    }

    if (!newAtomIds.empty()) {
        createBondsForAtoms(newAtomIds, true, false, false);
    }

    SPDLOG_DEBUG("XSF loading completed");
}

void WorkspaceRuntimeModel::ParseXSFFileWithManager(const std::string& filePath) {
    if (!m_fileIOManager) {
        SPDLOG_ERROR("FileIOManager not initialized");
        throw std::runtime_error("FileIOManager not initialized");
    }
    
    auto result = m_fileIOManager->loadXSFFile(filePath);
    if (!result.success) {
        SPDLOG_ERROR("Failed to parse XSF file: {}", result.errorMessage);
        throw std::runtime_error("XSF parsing failed: " + result.errorMessage);
    }

    std::string errorMessage;
    std::vector<uint32_t> newAtomIds;
    if (!m_fileIOManager->initializeStructure(result.cellVectors, result.atoms, this, errorMessage, &newAtomIds)) {
        SPDLOG_ERROR("Failed to initialize structure from parsed data: {}", errorMessage);
        throw std::runtime_error(errorMessage.empty() ? "Structure initialization failed" : errorMessage);
    }

    if (!newAtomIds.empty()) {
        createBondsForAtoms(newAtomIds, true, false, false);
    }
    
    SPDLOG_INFO("XSF file parsed and initialized successfully: {} atoms", result.atoms.size());
}

bool WorkspaceRuntimeModel::LoadXSFParsedData(const atoms::infrastructure::FileIOManager::ParseResult& result, bool renderCell) {
    if (m_bzPlotUI && m_bzPlotUI->IsShowingBZ()) {
        exitBZPlotMode();
        m_bzPlotUI->SetShowingBZ(false);
    }

    BatchGuard guard = createBatchGuard();

    std::vector<uint32_t> newAtomIds;
    try {
        if (renderCell) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    cellInfo.matrix[i][j] = result.cellVectors[i][j];
                }
            }
            cellInfo.modified = true;
            atoms::domain::calculateInverseMatrix(cellInfo.matrix, cellInfo.invmatrix);
            createUnitCell(cellInfo.matrix);
            atoms::domain::setCellVisible(true);
        } else {
            atoms::domain::setCellVisible(false);
        }

        const auto& elementDB = atoms::domain::ElementDatabase::getInstance();
        for (const auto& atom : result.atoms) {
            const std::string& symbol = atom.symbol;
            const float* position = atom.position;
            atoms::domain::Color4f color = elementDB.getDefaultColor(symbol);
            float radius = elementDB.getDefaultRadius(symbol);
            createAtomSphere(symbol.c_str(), color, radius, position);
            if (!createdAtoms.empty()) {
                newAtomIds.push_back(createdAtoms.back().id);
            }
        }

        if (!newAtomIds.empty()) {
            createBondsForAtoms(newAtomIds, true, false, false);
        }
        atoms::domain::setSurroundingsVisible(false);
        return true;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Error applying XSF parsed data: {}", e.what());
        return false;
    }
}

// setBravaisLattice, LoadXSFFile에서 호출
void WorkspaceRuntimeModel::clearAllUnifiedAtomGroups() {
    SPDLOG_INFO("Clearing all unified atom groups...");
    try {
        // 1) ?뚮뜑留??뚯씠?꾨씪???뺣━ (?명봽??
        if (m_vtkRenderer) {
            m_vtkRenderer->clearAllAtomGroupsVTK();
            m_vtkRenderer->clearAllAtomLabelActors();
        }

        // 2) ?꾨찓???곗씠???뺣━
        atoms::domain::clearAllAtomGroups();

        SPDLOG_INFO("Successfully cleared all unified atom groups");
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Error clearing unified atom groups: {}", e.what());
    }
}

void WorkspaceRuntimeModel::resetStructure() {
    clearAllCenterMeasurements();
    clearAllDihedralMeasurements();
    clearAllAngleMeasurements();
    clearAllDistanceMeasurements();
    createdAtoms.clear();
    surroundingAtoms.clear();
    m_AtomVisibilityById.clear();
    m_AtomLabelVisibilityById.clear();

    clearAllUnifiedAtomGroups();
    clearAllBonds();
    m_BondVisibilityById.clear();
    m_BondLabelVisibilityById.clear();
    if (m_vtkRenderer) {
        m_vtkRenderer->clearUnitCell();
        m_vtkRenderer->clearAllStructureLabelActors();
    }
    m_UnitCells.clear();
    atoms::domain::setCellVisible(false);
    cellInfo.hasCell = false;

    ClearChargeDensity();

}

// struct CellInfo --> cell_manager.h 濡??대룞

bool WorkspaceRuntimeModel::isCellVisible() const {
    return atoms::domain::isCellVisible();
}

// cellInfo가 수정되었거나, 단위행렬이 아니면 Cell이 정의된 것으로 판단
bool WorkspaceRuntimeModel::hasUnitCell() const {
    return atoms::domain::hasUnitCell();
}

// Surroundings visible getter/setter 援ы쁽 異붽?
bool WorkspaceRuntimeModel::isSurroundingsVisible() const {
    return atoms::domain::isSurroundingsVisible();
}

void WorkspaceRuntimeModel::setSurroundingsVisible(bool visible) {
    atoms::domain::setSurroundingsVisible(visible);
}

void WorkspaceRuntimeModel::SetBoundaryAtomsEnabled(bool enabled) {
    if (enabled == isSurroundingsVisible()) {
        return;
    }

    if (enabled) {
        createSurroundingAtoms();
    } else {
        hideSurroundingAtoms();
    }
    setSurroundingsVisible(enabled);
    requestViewerRender();
}

bool WorkspaceRuntimeModel::IsBZPlotMode() const {
    return m_bzPlotUI && m_bzPlotUI->IsShowingBZ();
}

void WorkspaceRuntimeModel::setBravaisLattice(
    atoms::domain::BravaisLatticeType type,
    const atoms::domain::BravaisParameters& params,
    bool preserveExistingAtoms
) {
    SPDLOG_INFO("Setting Bravais lattice: type={}, a={:.3f}, b={:.3f}, c={:.3f}, "
                "alpha={:.1f}째, beta={:.1f}째, gamma={:.1f}째",
                static_cast<int>(type), params.a, params.b, params.c,
                params.alpha, params.beta, params.gamma);

    if (m_bzPlotUI && m_bzPlotUI->IsShowingBZ()) {
        exitBZPlotMode();
        m_bzPlotUI->SetShowingBZ(false);
    }
    
    bool wasSurroundingsVisible = isSurroundingsVisible();
    if (wasSurroundingsVisible) {
        hideSurroundingAtoms();
    }

    try {
        // 기존 구조 초기화
        BatchGuard guard = createBatchGuard();

        // ========================================================================
        // 1. Domain Layer?먯꽌 寃⑹옄 踰≫꽣 怨꾩궛
        // ========================================================================
        float cellMatrix[3][3];
        atoms::domain::CrystalStructureGenerator::generateLatticeVectors(
            type, params, cellMatrix
        );
        
        SPDLOG_DEBUG("Generated cell matrix:");
        SPDLOG_DEBUG("  v1 = [{:.3f}, {:.3f}, {:.3f}]", 
                     cellMatrix[0][0], cellMatrix[1][0], cellMatrix[2][0]);
        SPDLOG_DEBUG("  v2 = [{:.3f}, {:.3f}, {:.3f}]", 
                     cellMatrix[0][1], cellMatrix[1][1], cellMatrix[2][1]);
        SPDLOG_DEBUG("  v3 = [{:.3f}, {:.3f}, {:.3f}]", 
                     cellMatrix[0][2], cellMatrix[1][2], cellMatrix[2][2]);
        
        if (preserveExistingAtoms) {
            // ========================================================================
            // 2. Unit Cell 媛깆떊 (?먯옄 ?좎?)
            // ========================================================================
            SPDLOG_INFO("Updating unit cell while preserving existing atoms...");
            clearUnitCell();
            createUnitCell(cellMatrix);

            for (auto& atom : createdAtoms) {
                float cartPos[3];
                // Direct(분수) 좌표 -> Cartesian 변환
                atoms::domain::fractionalToCartesian(atom.fracPosition, cartPos, cellMatrix);

                atom.position[0] = cartPos[0];
                atom.position[1] = cartPos[1];
                atom.position[2] = cartPos[2];
                atom.tempPosition[0] = cartPos[0];
                atom.tempPosition[1] = cartPos[1];
                atom.tempPosition[2] = cartPos[2];
                atom.tempFracPosition[0] = atom.fracPosition[0];
                atom.tempFracPosition[1] = atom.fracPosition[1];
                atom.tempFracPosition[2] = atom.fracPosition[2];
                atom.tempSymbol = atom.symbol;
                atom.tempRadius = atom.radius;
                atom.modified = false;

                auto it = atomGroups.find(atom.symbol);
                if (it == atomGroups.end()) {
                    continue;
                }
                auto& group = it->second;
                auto idIt = std::find(group.atomIds.begin(), group.atomIds.end(), atom.id);
                if (idIt == group.atomIds.end()) {
                    continue;
                }

                size_t instanceIndex = static_cast<size_t>(std::distance(group.atomIds.begin(), idIt));
                if (instanceIndex >= group.transforms.size()) {
                    continue;
                }

                vtkSmartPointer<vtkTransform> transform = group.transforms[instanceIndex];
                if (!transform) {
                    continue;
                }

                transform->Identity();
                transform->Translate(cartPos[0], cartPos[1], cartPos[2]);

                float adjustedRadius = atom.radius * 0.5f;
                float scaleFactor = adjustedRadius / 0.5f;
                transform->Scale(scaleFactor, scaleFactor, scaleFactor);

                scheduleAtomGroupUpdate(atom.symbol);
            }

            clearAllBonds();
            if (wasSurroundingsVisible) {
                createSurroundingAtoms();
            } else if (!createdAtoms.empty()) {
                createAllBonds();
            }
        } else {
            // ========================================================================
            // 2. Unit Cell 媛깆떊 (援ъ“ 珥덇린??
            // ========================================================================
            SPDLOG_INFO("Resetting structure for new lattice...");
            resetStructure();
            createUnitCell(cellMatrix);
            clearAllBonds();
        }
        
        // ========================================================================
        // 3. Unit Cell ?쒓컖???쒖꽦??        // ========================================================================
        if (m_vtkRenderer) {
            m_vtkRenderer->setUnitCellVisible(true);
        }
        atoms::domain::setCellVisible(true);
        
        SPDLOG_INFO("Bravais lattice setup complete: {} ({} atoms)",
                    atoms::domain::CrystalStructureGenerator::getLatticeName(type),
                    createdAtoms.size());
        
    } catch (const std::exception& e) {
        // ?ㅻ쪟 諛쒖깮 ?쒖뿉???꾨줈洹몃옩??怨꾩냽 ?ㅽ뻾?섎룄濡??덉쇅瑜??ㅼ떆 ?섏?吏 ?딆쓬
        SPDLOG_ERROR("Failed to set Bravais lattice: {}", e.what());
    }
}

bool WorkspaceRuntimeModel::enterBZPlotMode(
    const std::string& path,
    int npoints,
    bool showVectors,
    bool showLabels,
    std::string& outErrorMessage
) {
    outErrorMessage.clear();
    if (!m_bzPlotController) {
        outErrorMessage = "BZPlotController not initialized";
        SPDLOG_ERROR("{}", outErrorMessage);
        return false;
    }

    setMeasurementVisibilitySuppressedByBZ(true);
    const bool success = m_bzPlotController->enterBZPlotMode(
        this,
        cellInfo,
        path,
        npoints,
        showVectors,
        showLabels,
        outErrorMessage
    );
    if (!success) {
        setMeasurementVisibilitySuppressedByBZ(false);
    }
    return success;
}

void WorkspaceRuntimeModel::exitBZPlotMode() {
    if (!m_bzPlotController) {
        SPDLOG_ERROR("BZPlotController not initialized");
        return;
    }
    m_bzPlotController->exitBZPlotMode(this);
    setMeasurementVisibilitySuppressedByBZ(false);
}

// ============================================================================
// (251212) ATOM RENDERING SYSTEM - VTK ?명꽣?섏씠??// ============================================================================

void WorkspaceRuntimeModel::initializeUnifiedAtomGroupVTK(const std::string& symbol, float radius) {
    if (m_vtkRenderer) {
        m_vtkRenderer->initializeAtomGroupVTK(symbol, radius);
    }
}

void WorkspaceRuntimeModel::updateUnifiedAtomGroupVTK(const std::string& symbol) {
    if (atomGroups.find(symbol) == atomGroups.end()) {
        SPDLOG_ERROR("Unified atom group not found for symbol: {}", symbol);
        return;
    }
    
    auto& group = atomGroups[symbol];

    std::vector<vtkSmartPointer<vtkTransform>> filteredTransforms;
    std::vector<atoms::domain::Color4f> filteredColors;
    filteredTransforms.reserve(group.transforms.size());
    filteredColors.reserve(group.colors.size());

    for (size_t i = 0; i < group.atomIds.size(); ++i) {
        int32_t structureId = -1;
        if (!getAtomStructureId(group.atomIds[i], structureId)) {
            continue;
        }
        if (IsStructureVisible(structureId) && IsAtomVisibleById(group.atomIds[i])) {
            filteredTransforms.push_back(group.transforms[i]);
            filteredColors.push_back(group.colors[i]);
        }
    }
    
    if (m_vtkRenderer) {
        m_vtkRenderer->updateAtomGroupVTK(symbol, filteredTransforms, filteredColors);
    }
}

// ============================================================================
// 寃고빀 洹몃９ VTK ?⑥닔??[異붽? - 硫ㅻ쾭 ?⑥닔]
// ============================================================================

void WorkspaceRuntimeModel::initializeBondGroupVTK(const std::string& bondTypeKey, float radius) {
    if (m_bondRenderer) {
        m_bondRenderer->initializeBondGroup(bondTypeKey, radius);
        return;
    }
    if (m_vtkRenderer) {
        m_vtkRenderer->initializeBondGroup(bondTypeKey, radius);
    }
}

void WorkspaceRuntimeModel::updateBondGroupVTK(const std::string& bondTypeKey) {
    if (bondGroups.find(bondTypeKey) == bondGroups.end()) {
        SPDLOG_ERROR("Bond group info not found for key: {}", bondTypeKey);
        return;
    }
    
    auto& group = bondGroups[bondTypeKey];

    std::vector<vtkSmartPointer<vtkTransform>> t1;
    std::vector<vtkSmartPointer<vtkTransform>> t2;
    std::vector<atoms::domain::Color4f> c1;
    std::vector<atoms::domain::Color4f> c2;
    t1.reserve(group.transforms1.size());
    t2.reserve(group.transforms2.size());
    c1.reserve(group.colors1.size());
    c2.reserve(group.colors2.size());

    for (size_t i = 0; i < group.bondIds.size(); ++i) {
        int32_t structureId = -1;
        if (!getBondStructureId(group.bondIds[i], structureId)) {
            continue;
        }
        if (IsStructureVisible(structureId) &&
            IsBondsVisible(structureId) &&
            IsBondVisibleById(group.bondIds[i])) {
            t1.push_back(group.transforms1[i]);
            t2.push_back(group.transforms2[i]);
            c1.push_back(group.colors1[i]);
            c2.push_back(group.colors2[i]);
        }
    }
    
    if (m_bondRenderer) {
        atoms::domain::BondGroupInfo filteredGroup = group;
        filteredGroup.transforms1 = std::move(t1);
        filteredGroup.transforms2 = std::move(t2);
        filteredGroup.colors1 = std::move(c1);
        filteredGroup.colors2 = std::move(c2);
        m_bondRenderer->updateBondGroup(bondTypeKey, filteredGroup);
        return;
    }
    if (m_vtkRenderer) {
        m_vtkRenderer->updateBondGroup(bondTypeKey,
                                       t1,
                                       t2,
                                       c1,
                                       c2);
    }
}

void WorkspaceRuntimeModel::clearAllBondGroupsVTK() {
    if (m_bondRenderer) {
        m_bondRenderer->clearAllBondGroups();
        return;
    }
    if (m_vtkRenderer) {
        m_vtkRenderer->clearAllBondGroups();
    }
}

/**
 * @brief 吏?뺣맂 ?먯옄??媛꾩쓽 寃고빀???앹꽦?섎뒗 怨좎꽦???⑥닔 (ID 湲곕컲?쇰줈 ?섏젙??
 * @param atomIds 寃고빀???앹꽦???먯옄?ㅼ쓽 ID 諛곗뿴 (鍮꾩뼱?덉쑝硫?紐⑤뱺 ?먯옄)
 * @param includeOriginal ?먮낯 ?먯옄 ?ы븿 ?щ?
 * @param includeSurrounding 二쇰? ?먯옄 ?ы븿 ?щ?
 * @param clearExisting 湲곗〈 寃고빀 ?쒓굅 ?щ?
 */
// createAllBonds, createSurroundingAtoms, hideSurroundingAtoms
void WorkspaceRuntimeModel::createBondsForAtoms(
    const std::vector<uint32_t>& atomIds,  // ?뵩 ???蹂寃? size_t ??uint32_t
    bool includeOriginal, bool includeSurrounding, bool clearExisting
) {
    atoms::domain::createBondsForAtoms(
        this,
        atomIds,
        includeOriginal,
        includeSurrounding,
        clearExisting,
        bondScalingFactor,
        *m_elementDB
    );
}

/**
 * @brief createBondsForAtoms({}, true, true, true) ?ㅽ뻾
 */
void WorkspaceRuntimeModel::createAllBonds() {
    // Full rebuild path: restart bond serial numbering.
    resetBondIdCounter();
    createBondsForAtoms({}, true, true, true);
}

/**
 * @brief 紐⑤뱺 寃고빀 ?쒓굅 - CPU ?몄뒪?댁떛 紐⑤뱶留?泥섎━
 * 
 * ?뵩 蹂寃쎌궗??
 * - VTK 媛쒕퀎 ?≫꽣 ?쒓굅 猷⑦봽 ?꾩쟾 ?쒓굅
 * - 洹몃９ 湲곕컲 ?뺣━留??섑뻾
 * - BondInfo??VTK 媛앹껜媛 ?놁쑝誘濡?愿??肄붾뱶 遺덊븘?? */
void WorkspaceRuntimeModel::clearAllBonds() {
    atoms::domain::clearAllBonds(this);
    resetBondIdCounter();
    m_BondVisibilityById.clear();
    m_BondLabelVisibilityById.clear();
    if (m_vtkRenderer) {
        m_vtkRenderer->clearAllBondLabelActors();
    }
}

// shouldCreateBond --> createBond
/**
 * @brief 寃고빀 ?앹꽦 ?⑥닔 - ?듯빀 CPU ?몄뒪?댁떛 紐⑤뱶留??ъ슜
 * 
 * ?뵩 蹂寃쎌궗??
 * - VTK 媛쒕퀎 ?≫꽣 ?앹꽦 ?꾩쟾 ?쒓굅
 * - BondInfo??VTK 媛앹껜 ?좊떦 肄붾뱶 ?쒓굅
 * - ??긽 isInstanced = true濡??ㅼ젙
 */
void WorkspaceRuntimeModel::createBond(
    const atoms::domain::AtomInfo& atom1, 
    const atoms::domain::AtomInfo& atom2, 
    atoms::domain::BondType bondType
) {
    atoms::domain::createBond(this, atom1, atom2, bondType, bondScalingFactor, *m_elementDB);
}

// createBond
uint32_t WorkspaceRuntimeModel::generateUniqueBondId() {
    return structure::domain::GetStructureRepository().GenerateBondId();
}

void WorkspaceRuntimeModel::resetBondIdCounter(uint32_t startId) {
    structure::domain::GetStructureRepository().ResetBondIdCounter(startId);
}

// updatePerformanceStatsInternal
int WorkspaceRuntimeModel::getTotalBondCount() {
    int totalCount = 0;
    
    for (const auto& [bondTypeKey, group] : bondGroups) {
        totalCount += static_cast<int>(group.transforms1.size());
    }
    
    totalCount += static_cast<int>(createdBonds.size());
    totalCount += static_cast<int>(surroundingBonds.size());
    
    SPDLOG_DEBUG("Total bond count: {} (Groups: {}, Individual: {})", 
                totalCount, 
                totalCount - static_cast<int>(createdBonds.size() + surroundingBonds.size()),
                static_cast<int>(createdBonds.size() + surroundingBonds.size()));
    
    return totalCount;
}

// 諛곗튂 ?낅뜲?댄듃 愿??
// 5. addBondsToGroups() ?⑥닔 ?듯빀 - ?몃뜳??愿由?媛쒖꽑 (atoms_template.cpp)
void WorkspaceRuntimeModel::addBondsToGroups(int atomIndex) {
    atoms::domain::addBondsToGroups(this, atomIndex, bondScalingFactor, *m_elementDB);
}

// [?섏젙] VTKRenderer ?ъ슜
void WorkspaceRuntimeModel::updateAllBondGroupThickness() {
    SPDLOG_DEBUG("Updating bond thickness for all groups to {:.1f}", bondThickness);
    atoms::domain::updateAllBondGroupThickness(this, bondThickness);
}

// [?섏젙] VTKRenderer ?ъ슜
void WorkspaceRuntimeModel::updateAllBondGroupOpacity() {
    SPDLOG_DEBUG("Updating bond opacity for all groups to {:.1f}", bondOpacity);
    atoms::domain::updateAllBondGroupOpacity(this, bondOpacity);
}

float WorkspaceRuntimeModel::getBondThickness() const {
    return bondThickness;
}

void WorkspaceRuntimeModel::setBondThickness(float value) {
    bondThickness = value;
}

float WorkspaceRuntimeModel::getBondOpacity() const {
    return bondOpacity;
}

void WorkspaceRuntimeModel::setBondOpacity(float value) {
    bondOpacity = value;
}

float WorkspaceRuntimeModel::getBondScalingFactor() const {
    return bondScalingFactor;
}

void WorkspaceRuntimeModel::setBondScalingFactor(float value) {
    bondScalingFactor = value;
}

float WorkspaceRuntimeModel::getBondToleranceFactor() const {
    return bondToleranceFactor;
}

void WorkspaceRuntimeModel::setBondToleranceFactor(float value) {
    bondToleranceFactor = value;
}

WorkspaceRuntimeModel::BondPerfStats WorkspaceRuntimeModel::getBondPerformanceStats() const {
    BondPerfStats stats;
    stats.lastUpdateTime   = g_performanceStats.lastUpdateTime;
    stats.averageUpdateTime = g_performanceStats.averageUpdateTime;
    stats.updateCount      = g_performanceStats.updateCount;
    return stats;
}

// InitializeAtomicStructure, createSurroundingAtoms
/**
 * @brief ?먯옄 ?앹꽦 ?⑥닔 - ?듯빀 ?쒖뒪?쒕쭔 ?ъ슜 (?덇굅???숆린???쒓굅)
 * 
 * ?뵩 ?④퀎 2 蹂寃쎌궗??
 * - elementTransforms, elementColors, elementAtomIndices ?숆린???꾩쟾 ?쒓굅
 * - ?듯빀 atomGroups ?쒖뒪?쒕쭔 ?ъ슜
 * - ?댁쨷 ?쒖뒪???숆린??濡쒖쭅 ?쒓굅
 */
void WorkspaceRuntimeModel::createAtomSphere(
    const char* symbol, 
    // const ImVec4& color, 
    const atoms::domain::Color4f& color, 
    float radius, 
    const float position[3], atoms::domain::AtomType atomType
) {

    std::string symbolStr(symbol);
    
    // 통합 원자 그룹 초기화
    initializeAtomGroup(symbolStr, radius);

    // 그룹이 없으면 visibility 상태 초기화
    if (m_AtomGroupVisibility.find(symbolStr) == m_AtomGroupVisibility.end()) {
        m_AtomGroupVisibility[symbolStr] = true;
    }

    float adjustedRadius = radius * 0.5f;
    
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Identity();
    transform->Translate(position[0], position[1], position[2]);
    
    float scaleFactor = adjustedRadius / 0.5f;
    transform->Scale(scaleFactor, scaleFactor, scaleFactor);
    
    // AtomType???곕Ⅸ ?됱긽 議곗젙
    atoms::domain::Color4f adjustedColor = color;
    if (atomType == atoms::domain::AtomType::SURROUNDING) {
        float colorMultiplier = 200.0f / 255.0f;
        adjustedColor.r *= colorMultiplier;
        adjustedColor.g *= colorMultiplier;
        adjustedColor.b *= colorMultiplier;
    }
    
    // 怨좎쑀 ID ?앹꽦
    uint32_t atomId = atoms::domain::generateUniqueAtomId();
    
    // ???듯빀 洹몃９ ?쒖뒪?쒖뿉留?異붽?
    size_t originalIndex = (atomType == atoms::domain::AtomType::ORIGINAL) ? 
                          createdAtoms.size() : surroundingAtoms.size();
    
    // (domain) atom_manager.cpp                      
    atoms::domain::addAtomToGroup(symbolStr, transform, adjustedColor, atomId, atomType, originalIndex);
    
    // AtomInfo ?앹꽦 (CPU ?몄뒪?댁떛留?
    atoms::domain::AtomInfo atomInfo(symbolStr, adjustedColor, radius, position, atomType);
    atomInfo.id = atomId;
    atomInfo.structureId = m_CurrentStructureId;
    atomInfo.isInstanced = true;  // ??긽 true
    atomInfo.bondRadius = std::max(
        atoms::domain::ElementDatabase::getInstance().getDefaultRadius(symbolStr),
        0.001f);
    
    // 洹몃９ ???몄뒪?댁뒪 ?몃뜳???ㅼ젙
    if (atomGroups.find(symbolStr) != atomGroups.end()) {
        atomInfo.instanceIndex = atomGroups[symbolStr].transforms.size() - 1;
    }
    
    atoms::domain::cartesianToFractional(position, atomInfo.fracPosition, cellInfo.invmatrix);
    
    // 임시 값들 초기화
    atomInfo.tempSymbol = symbolStr;
    atomInfo.tempPosition[0] = position[0];
    atomInfo.tempPosition[1] = position[1];
    atomInfo.tempPosition[2] = position[2];
    atomInfo.tempFracPosition[0] = atomInfo.fracPosition[0];
    atomInfo.tempFracPosition[1] = atomInfo.fracPosition[1];
    atomInfo.tempFracPosition[2] = atomInfo.fracPosition[2];
    atomInfo.tempRadius = radius;
    atomInfo.modified = false;
    atomInfo.selected = false;
    
    // 원래/주변 원자 벡터에 추가
    if (atomType == atoms::domain::AtomType::ORIGINAL) {
        createdAtoms.push_back(atomInfo);
    } else {
        surroundingAtoms.push_back(atomInfo);
    }
    
    // 諛곗튂 ?ㅼ?以꾨쭅
    scheduleAtomGroupUpdate(symbolStr);
    
    SPDLOG_DEBUG("Created atom {} with radius {:.3f}, ID {}, type {} (unified system only)", 
                symbolStr, adjustedRadius, atomId, 
                atomType == AtomType::ORIGINAL ? "ORIGINAL" : "SURROUNDING");
}

void WorkspaceRuntimeModel::clearUnitCell() {
    int32_t structureId = m_CurrentStructureId;
    if (m_vtkRenderer) {
        if (structureId >= 0) {
            m_vtkRenderer->clearUnitCell(structureId);
        } else {
            m_vtkRenderer->clearUnitCell();
        }
    }
    
    if (structureId >= 0) {
        m_UnitCells.erase(structureId);
    } else {
        m_UnitCells.clear();
    }
    
    cellEdgeActors.clear();  // 기존 유지
    atoms::domain::setCellVisible(false);
    cellInfo.hasCell = false;
}

void WorkspaceRuntimeModel::createUnitCell(const float matrix[3][3]) {
    atoms::domain::setCellMatrix(matrix);

    int32_t structureId = m_CurrentStructureId;
    if (structureId >= 0) {
        auto& entry = m_UnitCells[structureId];
        entry.hasCell = true;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                entry.matrix[i][j] = matrix[i][j];
            }
        }
        if (m_vtkRenderer) {
            m_vtkRenderer->createUnitCell(structureId, matrix);
            bool effectiveVisible = entry.visible && IsStructureVisible(structureId);
            m_vtkRenderer->setUnitCellVisible(structureId, effectiveVisible);
        }
    } else {
        if (m_vtkRenderer) {
            m_vtkRenderer->createUnitCell(matrix);
        }
    }
    
    atoms::domain::setCellVisible(true);
    cellInfo.hasCell = true;
}

// *** ?섏젙: createSurroundingAtoms ?⑥닔???먯옄 諛섏?由?寃??媛쒖꽑 ***
void WorkspaceRuntimeModel::createSurroundingAtoms() {
    if (m_surroundingAtomManager) {
        m_surroundingAtomManager->createSurroundingAtoms();
    }
}

// renderCreatedAtomsSection, applyAtomChanges
/**
 * @brief 二쇰? ?먯옄 ?④린湲?- ?듯빀 洹몃９?먯꽌留??쒓굅
 * 
 * ?뵩 蹂寃쎌궗??
 * - isInstanced 泥댄겕 濡쒖쭅 ?꾩쟾 ?쒓굅 (紐⑤뱺 ?먯옄媛 ??긽 ?몄뒪?댁뒪??
 * - VTK 媛쒕퀎 ?≫꽣 ?쒓굅 遺꾧린 ?꾩쟾 ?쒓굅  
 * - 洹몃９ 湲곕컲 ?쒓굅留??섑뻾
 */
void WorkspaceRuntimeModel::hideSurroundingAtoms() {
    if (m_surroundingAtomManager) {
        m_surroundingAtomManager->hideSurroundingAtoms();
    }
}

/**
 * @brief ?먯옄 蹂寃쎌궗???곸슜 - ?꾩튂 蹂寃???紐⑤뱺 寃고빀 ?ъ깮?? * 
 * ?뵩 二쇱슂 蹂寃쎌궗??
 * - ?꾩튂 蹂寃??? clearAllBonds() ??createAllBonds() ?꾩껜 ?ъ깮?? * - Symbol 蹂寃??? 湲곗〈 濡쒖쭅 ?좎? (洹몃９ 媛??대룞)
 * - 紐⑤뱺 蹂寃????꾩쟾??寃고빀 ?ш뎄?깆쑝濡??쇨???蹂댁옣
 */
void WorkspaceRuntimeModel::applyAtomChanges() {
    SPDLOG_INFO("Applying atom changes (bond rebuild only for symbol/position changes)...");
    
    // 기존 구조 초기화
    BatchGuard guard = createBatchGuard();
    
    try {
        atoms::domain::applyAtomChanges(this, bondScalingFactor, *m_elementDB);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Error during applyAtomChanges: {}", e.what());
        
        // ?ㅻ쪟 諛쒖깮 ???곗씠???뺣━
        createdAtoms.clear();
        createdBonds.clear();
        
        throw; // ?덉쇅 ?ъ쟾?? ?곸쐞 BatchGuard媛 ?덉쟾?섍쾶 ?뺣━
    }
    
    SPDLOG_INFO("Successfully applied atom changes");
}

void WorkspaceRuntimeModel::applyCellChanges() {
    SPDLOG_INFO("Cell matrix edit mode ended - applying changes with batch system");
    
    // 寃⑹옄 蹂寃????꾩껜 援ъ“ ?ш퀎?곗씠 ?꾩슂?섎?濡?諛곗튂 泥섎━ ?곸슜
    // 기존 구조 초기화
    BatchGuard guard = createBatchGuard();
    
    try {
        atoms::domain::applyCellChanges(this);
    } catch (const std::exception& e) {
        // BatchGuard媛 ?덉쇅 ?곹솴?먯꽌???덉쟾?섍쾶 ?뺣━
        SPDLOG_ERROR("Error applying cell matrix changes: {}", e.what());
    }
}

/**
 * @brief ?깅뒫 ?듦퀎 ?낅뜲?댄듃 (BatchUpdateSystem ?듯빀 踰꾩쟾)
 * 
 * BatchUpdateSystem?먯꽌 ?몄텧?섎뒗 ?대? 硫붿꽌?쒕줈 蹂寃? * 
 * @param duration 諛곗튂 ?ㅽ뻾 ?쒓컙 (諛由ъ큹)
 * @param atomGroupCount 泥섎━???먯옄 洹몃９ ?? * @param bondGroupCount 泥섎━??寃고빀 洹몃９ ?? */
void WorkspaceRuntimeModel::updatePerformanceStatsInternal(float duration, 
                                                   size_t atomGroupCount, 
                                                   size_t bondGroupCount) {
    g_performanceStats.lastUpdateTime = duration;
    g_performanceStats.updateCount++;
    
    // ?대룞 ?됯퇏 怨꾩궛
    if (g_performanceStats.updateCount == 1) {
        g_performanceStats.averageUpdateTime = duration;
    } else {
        float alpha = 0.1f; // 吏???대룞 ?됯퇏 媛以묒튂
        g_performanceStats.averageUpdateTime = 
            alpha * duration + (1.0f - alpha) * g_performanceStats.averageUpdateTime;
    }
    
    // ?깅뒫 寃쎄퀬
    if (duration > 100.0f) { // 100ms ?댁긽 ?뚯슂 ??寃쎄퀬
        SPDLOG_WARN("Batch update took {:.2f} ms - consider optimization", duration);
    }
    
    // ?듦퀎 ?낅뜲?댄듃 (?섏젙: pendingAtomGroups/pendingBondGroups ???留ㅺ컻蹂???ъ슜)
    g_performanceStats.totalAtoms = static_cast<int>(createdAtoms.size() + surroundingAtoms.size());
    g_performanceStats.totalGroups = static_cast<int>(atomGroupCount + bondGroupCount);
    g_performanceStats.totalBonds = getTotalBondCount();
    
    // ?곸꽭 ?깅뒫 濡쒓렇 (?붾쾭洹?紐⑤뱶?먯꽌留?
    if (duration > 50.0f) {
        SPDLOG_DEBUG("Performance details - Duration: {:.2f}ms, Atoms: {}, Groups: {}, Bonds: {}", 
                    duration, g_performanceStats.totalAtoms, 
                    g_performanceStats.totalGroups, g_performanceStats.totalBonds);
    }
}

/**
 * @brief ?깅뒫 ?듦퀎 ?낅뜲?댄듃 (?명솚???섑띁)
 * 
 * 湲곗〈 肄붾뱶????명솚?깆쓣 ?꾪븳 ?섑띁 硫붿꽌?? * BatchUpdateSystem???듯빐 ?꾩옱 ?湲?以묒씤 洹몃９ ?섎? 媛?몄샂
 * 
 * @param duration 諛곗튂 ?ㅽ뻾 ?쒓컙 (諛由ъ큹)
 */
void WorkspaceRuntimeModel::updatePerformanceStats(float duration) {
    // BatchUpdateSystem에서 현재 대기 중인 그룹 수 가져오기
    size_t atomGroupCount = 0;
    size_t bondGroupCount = 0;
    
    if (m_batchSystem) {
        atomGroupCount = m_batchSystem->getPendingAtomGroupCount();
        bondGroupCount = m_batchSystem->getPendingBondGroupCount();
    }
    
    // ?대? 硫붿꽌???몄텧
    updatePerformanceStatsInternal(duration, atomGroupCount, bondGroupCount);
}

const atoms::domain::AtomInfo* WorkspaceRuntimeModel::findAtomById(uint32_t atomId) const {
    if (atomId == 0) {
        return nullptr;
    }
    for (const auto& atom : createdAtoms) {
        if (atom.id == atomId) {
            return &atom;
        }
    }
    for (const auto& atom : surroundingAtoms) {
        if (atom.id == atomId) {
            return &atom;
        }
    }
    return nullptr;
}

bool WorkspaceRuntimeModel::resolvePickedAtom(
    vtkActor* actor,
    const double pickPos[3],
    uint32_t& atomId,
    int32_t& structureId,
    std::array<double, 3>& atomPosition) const {
    atomId = 0;
    structureId = -1;
    atomPosition = { 0.0, 0.0, 0.0 };

    if (!actor) {
        return false;
    }

    const std::string symbol = GetSymbolByActor(actor);
    if (symbol.empty()) {
        return false;
    }

    auto it = atomGroups.find(symbol);
    if (it == atomGroups.end()) {
        return false;
    }

    const auto& groupInfo = it->second;
    float minDist = std::numeric_limits<float>::max();
    size_t nearestIndex = SIZE_MAX;
    for (size_t i = 0; i < groupInfo.transforms.size(); ++i) {
        if (!groupInfo.transforms[i]) {
            continue;
        }
        double* pos = groupInfo.transforms[i]->GetPosition();
        const float dx = static_cast<float>(pickPos[0] - pos[0]);
        const float dy = static_cast<float>(pickPos[1] - pos[1]);
        const float dz = static_cast<float>(pickPos[2] - pos[2]);
        const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (dist < minDist) {
            minDist = dist;
            nearestIndex = i;
            atomPosition = { pos[0], pos[1], pos[2] };
        }
    }

    if (nearestIndex == SIZE_MAX || nearestIndex >= groupInfo.atomIds.size()) {
        return false;
    }

    atomId = groupInfo.atomIds[nearestIndex];
    if (atomId == 0) {
        return false;
    }

    const atoms::domain::AtomInfo* atomInfo = findAtomById(atomId);
    if (!atomInfo) {
        return false;
    }

    structureId = atomInfo->structureId;
    return true;
}

render::application::PickedAtomInfo WorkspaceRuntimeModel::ResolvePickedAtomInfo(
    vtkActor* actor,
    const double pickPos[3]) const {
    render::application::PickedAtomInfo pickedInfo;
    if (pickPos) {
        pickedInfo.pickPosition = { pickPos[0], pickPos[1], pickPos[2] };
    }

    uint32_t atomId = 0;
    int32_t structureId = -1;
    std::array<double, 3> atomPosition { 0.0, 0.0, 0.0 };
    if (!resolvePickedAtom(actor, pickPos, atomId, structureId, atomPosition)) {
        return pickedInfo;
    }

    pickedInfo.hit = true;
    pickedInfo.atomId = atomId;
    pickedInfo.structureId = structureId;
    pickedInfo.atomPosition = atomPosition;
    return pickedInfo;
}

std::string WorkspaceRuntimeModel::GetSymbolByActor(vtkActor* actor) const {
    if (!actor || !m_vtkRenderer) {
        return "";
    }
    
    // VTKRenderer의 atomGroups에서 검색
    for (const auto& [symbol, group] : m_vtkRenderer->getAtomGroups()) {
        if (group.actor.Get() == actor) {
            return symbol;
        }
    }
    
    return "";
}

void WorkspaceRuntimeModel::UpdateHoveredAtomByPicker(vtkActor* actor, double pickPos[3]) {
    m_HoveredAtom.isHovered = false;
    m_HoveredAtom.displayLabel.clear();
    
    if (!actor) return;
    
    std::string symbol = GetSymbolByActor(actor);
    if (symbol.empty()) return;
    
    // atoms::domain::atomGroups에서 transforms 가져오기
    auto it = atomGroups.find(symbol);
    if (it == atomGroups.end()) return;
    
    const auto& groupInfo = it->second;
    
    // pickPos?먯꽌 媛??媛源뚯슫 ?먯옄 李얘린
    float minDist = std::numeric_limits<float>::max();
    size_t nearestIndex = SIZE_MAX;
    
    for (size_t i = 0; i < groupInfo.transforms.size(); ++i) {
        if (!groupInfo.transforms[i]) continue;
        
        double* pos = groupInfo.transforms[i]->GetPosition();
        float dx = static_cast<float>(pickPos[0] - pos[0]);
        float dy = static_cast<float>(pickPos[1] - pos[1]);
        float dz = static_cast<float>(pickPos[2] - pos[2]);
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        if (dist < minDist) {
            minDist = dist;
            nearestIndex = i;
            m_HoveredAtom.isHovered = true;
            m_HoveredAtom.symbol = symbol;
            m_HoveredAtom.x = static_cast<float>(pos[0]);
            m_HoveredAtom.y = static_cast<float>(pos[1]);
            m_HoveredAtom.z = static_cast<float>(pos[2]);
        }
    }

    if (!m_HoveredAtom.isHovered) {
        return;
    }

    m_HoveredAtom.displayLabel = symbol;

    if (nearestIndex >= groupInfo.atomTypes.size() ||
        nearestIndex >= groupInfo.originalIndices.size()) {
        return;
    }

    std::unordered_map<uint32_t, size_t> originalSerialByAtomId;
    originalSerialByAtomId.reserve(createdAtoms.size());
    for (size_t atomIndex = 0; atomIndex < createdAtoms.size(); ++atomIndex) {
        const auto& atom = createdAtoms[atomIndex];
        if (atom.id != 0) {
            originalSerialByAtomId[atom.id] = atomIndex + 1;
        }
    }

    const auto atomType = groupInfo.atomTypes[nearestIndex];
    const size_t originalIndex = groupInfo.originalIndices[nearestIndex];

    if (atomType == atoms::domain::AtomType::ORIGINAL) {
        if (originalIndex < createdAtoms.size()) {
            m_HoveredAtom.displayLabel = symbol + "#" + std::to_string(originalIndex + 1);
            return;
        }

        if (nearestIndex < groupInfo.atomIds.size()) {
            const uint32_t atomId = groupInfo.atomIds[nearestIndex];
            auto itSerial = originalSerialByAtomId.find(atomId);
            if (itSerial != originalSerialByAtomId.end()) {
                m_HoveredAtom.displayLabel = symbol + "#" + std::to_string(itSerial->second);
            } else {
                m_HoveredAtom.displayLabel = symbol + "#?";
            }
        } else {
            m_HoveredAtom.displayLabel = symbol + "#?";
        }
        return;
    }

    uint32_t sourceOriginalAtomId = 0;
    if (originalIndex < surroundingAtoms.size()) {
        sourceOriginalAtomId = surroundingAtoms[originalIndex].originalAtomId;
    }

    if (sourceOriginalAtomId == 0 && nearestIndex < groupInfo.atomIds.size()) {
        const uint32_t hoveredAtomId = groupInfo.atomIds[nearestIndex];
        for (const auto& atom : surroundingAtoms) {
            if (atom.id == hoveredAtomId) {
                sourceOriginalAtomId = atom.originalAtomId;
                break;
            }
        }
    }

    auto itSerial = originalSerialByAtomId.find(sourceOriginalAtomId);
    if (itSerial != originalSerialByAtomId.end()) {
        m_HoveredAtom.displayLabel = symbol + "(#" + std::to_string(itSerial->second) + ")";
    } else {
        m_HoveredAtom.displayLabel = symbol + "(#?)";
    }
    
    //if (m_HoveredAtom.isHovered) {
    //    SPDLOG_INFO("Hovered atom: {} ({:.4f}, {:.4f}, {:.4f})", 
    //        m_HoveredAtom.symbol, m_HoveredAtom.x, m_HoveredAtom.y, m_HoveredAtom.z);
    //}
}

void WorkspaceRuntimeModel::ClearHover() {
    m_HoveredAtom.isHovered = false;
    m_HoveredAtom.displayLabel.clear();
}

//void WorkspaceRuntimeModel::RenderAtomTooltip(float mouseX, float mouseY) {
//    // ???붾쾭源? ??긽 ?몄텧?섎뒗吏 ?뺤씤
//    SPDLOG_DEBUG("RenderAtomTooltip called: mouseX={}, mouseY={}, isHovered={}", 
//                 mouseX, mouseY, m_HoveredAtom.isHovered);
//    if (!m_HoveredAtom.isHovered) return;
//    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
//    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 6.0f));
//    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.9f));
//    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
//    ImGui::SetNextWindowPos(ImVec2(mouseX + 15.0f, mouseY + 15.0f));
//    ImGui::Begin("##AtomTooltip", nullptr, 
//        ImGuiWindowFlags_NoTitleBar |
//        ImGuiWindowFlags_NoResize |
//        ImGuiWindowFlags_NoMove |
//        ImGuiWindowFlags_NoScrollbar |
//        ImGuiWindowFlags_AlwaysAutoResize |
//        ImGuiWindowFlags_NoSavedSettings |
//        ImGuiWindowFlags_NoFocusOnAppearing |
//        ImGuiWindowFlags_NoBringToFrontOnFocus |
//        ImGuiWindowFlags_NoInputs
//    );
//    float originalScale = ImGui::GetFont()->Scale;
//    ImGui::GetFont()->Scale = 1.4f;
//    ImGui::PushFont(ImGui::GetFont());
//    // Si (0.0123, 1.2345, 2.3456)
//    ImGui::Text("%s (%.4f, %.4f, %.4f)", 
//        m_HoveredAtom.symbol.c_str(),
//        m_HoveredAtom.x, 
//        m_HoveredAtom.y, 
//        m_HoveredAtom.z);
//    ImGui::GetFont()->Scale = originalScale;
//    ImGui::PopFont();
//    ImGui::End();
//    ImGui::PopStyleColor(2);
//    ImGui::PopStyleVar(2);
//}

void WorkspaceRuntimeModel::RenderAtomTooltip(float mouseX, float mouseY)
{
    // ??Settings?먯꽌 爰쇱졇 ?덉쑝硫??꾨Т 寃껊룄 ??洹몃┝
    if (!m_NodeInfoEnabled)
        return;

    if (!m_HoveredAtom.isHovered)
        return;

    ImGui::SetNextWindowPos(
        ImVec2(mouseX + 10.0f, mouseY - 30.0f),
        ImGuiCond_Always
    );

    ImGui::Begin(
        "##AtomHoverTooltip",
        nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoInputs
    );

    ImGui::SetWindowFontScale(1.2f);

    ImGui::Text(
        "%s (%.4f, %.4f, %.4f)",
        m_HoveredAtom.displayLabel.empty() ? m_HoveredAtom.symbol.c_str() : m_HoveredAtom.displayLabel.c_str(),
        m_HoveredAtom.x,
        m_HoveredAtom.y,
        m_HoveredAtom.z
    );

    ImGui::End();
}

void WorkspaceRuntimeModel::SelectAtomByPicker(vtkActor* actor, double pickPos[3]) {
    uint32_t atomId = 0;
    int32_t structureId = -1;
    std::array<double, 3> atomPosition {};
    if (!resolvePickedAtom(actor, pickPos, atomId, structureId, atomPosition)) {
        return;
    }

    const atoms::domain::AtomInfo* pickedAtom = findAtomById(atomId);
    if (!pickedAtom) {
        return;
    }

    const uint32_t targetOriginalAtomId =
        (pickedAtom->atomType == atoms::domain::AtomType::SURROUNDING)
            ? pickedAtom->originalAtomId
            : pickedAtom->id;
    if (targetOriginalAtomId == 0) {
        return;
    }

    ClearSelection();

    atoms::domain::AtomInfo* toggledAtom = nullptr;
    for (auto& atom : createdAtoms) {
        if (atom.id != targetOriginalAtomId) {
            continue;
        }
        atom.selected = !atom.selected;
        toggledAtom = &atom;
        break;
    }

    if (!toggledAtom) {
        return;
    }

    if (toggledAtom->selected) {
        m_SelectedAtom.isSelected = true;
        m_SelectedAtom.symbol = toggledAtom->symbol;
        m_SelectedAtom.x = toggledAtom->position[0];
        m_SelectedAtom.y = toggledAtom->position[1];
        m_SelectedAtom.z = toggledAtom->position[2];
    } else {
        m_SelectedAtom.isSelected = false;
        m_SelectedAtom.symbol.clear();
    }
    m_SelectedActor = nullptr;

    syncCreatedAtomSelectionVisuals();
    requestViewerRender();
}

void WorkspaceRuntimeModel::SelectSameElementAtomsByPicker(vtkActor* actor, double pickPos[3]) {
    uint32_t atomId = 0;
    int32_t structureId = -1;
    std::array<double, 3> atomPosition {};
    if (!resolvePickedAtom(actor, pickPos, atomId, structureId, atomPosition)) {
        return;
    }
    (void)structureId;
    (void)atomPosition;

    const atoms::domain::AtomInfo* pickedAtom = findAtomById(atomId);
    if (!pickedAtom) {
        return;
    }

    ClearSelection();

    bool anySelected = false;
    for (auto& atom : createdAtoms) {
        if (atom.id == 0 || atom.structureId < 0) {
            continue;
        }
        if (atom.symbol != pickedAtom->symbol) {
            continue;
        }
        if (!IsStructureVisible(atom.structureId) || !IsAtomVisibleById(atom.id)) {
            continue;
        }
        atom.selected = true;
        anySelected = true;
    }

    const uint32_t selectedOriginalAtomId =
        (pickedAtom->atomType == atoms::domain::AtomType::SURROUNDING)
            ? pickedAtom->originalAtomId
            : pickedAtom->id;
    const atoms::domain::AtomInfo* selectedAtom = findAtomById(selectedOriginalAtomId);
    if (anySelected && selectedAtom) {
        m_SelectedAtom.isSelected = true;
        m_SelectedAtom.symbol = selectedAtom->symbol;
        m_SelectedAtom.x = selectedAtom->position[0];
        m_SelectedAtom.y = selectedAtom->position[1];
        m_SelectedAtom.z = selectedAtom->position[2];
    } else {
        m_SelectedAtom.isSelected = false;
        m_SelectedAtom.symbol.clear();
    }
    m_SelectedActor = nullptr;

    syncCreatedAtomSelectionVisuals();
    requestViewerRender();
}

void WorkspaceRuntimeModel::ClearCreatedAtomSelection() {
    bool changed = false;
    for (auto& atom : createdAtoms) {
        if (!atom.selected) {
            continue;
        }
        atom.selected = false;
        changed = true;
    }

    ClearSelection();
    if (changed || !m_CreatedAtomSelectionVisuals.empty()) {
        syncCreatedAtomSelectionVisuals();
        requestViewerRender();
    }
}

void WorkspaceRuntimeModel::SyncCreatedAtomSelectionVisualsFromState() {
    syncCreatedAtomSelectionVisuals();
    requestViewerRender();
}

bool WorkspaceRuntimeModel::isCenterMeasurementMode() const {
    return m_MeasurementMode == MeasurementMode::GeometricCenter ||
        m_MeasurementMode == MeasurementMode::CenterOfMass;
}

bool WorkspaceRuntimeModel::IsMeasurementDragSelectionEnabled() const {
    switch (m_MeasurementMode) {
        case MeasurementMode::Distance:
        case MeasurementMode::Angle:
        case MeasurementMode::Dihedral:
            return false;
        case MeasurementMode::GeometricCenter:
        case MeasurementMode::CenterOfMass:
            return true;
        case MeasurementMode::None:
        default:
            return false;
    }
}

void WorkspaceRuntimeModel::ClearSelection() {
    if (m_SelectedActor) {
        m_SelectedActor->GetProperty()->SetEdgeVisibility(false);
        m_SelectedActor = nullptr;
    }
    m_SelectedAtom.isSelected = false;
    m_SelectedAtom.symbol = "";
}

void WorkspaceRuntimeModel::HandleMeasurementEmptyClick() {
    if (!m_MeasurementPickedAtomIds.empty()) {
        m_MeasurementPickedAtomIds.clear();
        clearMeasurementPickVisuals();
        requestViewerRender();
    }
}

void WorkspaceRuntimeModel::HandleDragSelectionInScreenRect(
    int x0,
    int y0,
    int x1,
    int y1,
    class vtkRenderer* renderer,
    int viewportHeight,
    bool additive) {
    if (!renderer || viewportHeight <= 0) {
        return;
    }

    const bool measurementModeActive = IsMeasurementModeActive();
    if (measurementModeActive && !IsMeasurementDragSelectionEnabled()) {
        return;
    }

    const std::vector<uint32_t> atomIds = collectAtomIdsInScreenRect(
        x0,
        y0,
        x1,
        y1,
        renderer,
        viewportHeight,
        measurementModeActive);

    if (measurementModeActive) {
        applyDragSelectionToMeasurement(atomIds, additive);
    } else {
        applyDragSelectionToCreatedAtoms(atomIds, additive);
    }
}

std::vector<uint32_t> WorkspaceRuntimeModel::collectAtomIdsInScreenRect(
    int x0,
    int y0,
    int x1,
    int y1,
    class vtkRenderer* renderer,
    int viewportHeight,
    bool includeSurrounding) const {
    std::vector<uint32_t> atomIds;
    if (!renderer || viewportHeight <= 0) {
        return atomIds;
    }

    const int minX = std::min(x0, x1);
    const int maxX = std::max(x0, x1);
    const int minY = std::min(y0, y1);
    const int maxY = std::max(y0, y1);
    if (maxX < minX || maxY < minY) {
        return atomIds;
    }

    std::unordered_set<uint32_t> uniqueAtomIds;
    uniqueAtomIds.reserve(createdAtoms.size() + surroundingAtoms.size());

    auto appendIfInsideRect = [&](const atoms::domain::AtomInfo& atom) {
        if (atom.id == 0 || atom.structureId < 0) {
            return;
        }
        if (!includeSurrounding && atom.atomType != atoms::domain::AtomType::ORIGINAL) {
            return;
        }
        if (!IsStructureVisible(atom.structureId) || !IsAtomVisibleById(atom.id)) {
            return;
        }

        renderer->SetWorldPoint(
            static_cast<double>(atom.position[0]),
            static_cast<double>(atom.position[1]),
            static_cast<double>(atom.position[2]),
            1.0);
        renderer->WorldToDisplay();
        const double* display = renderer->GetDisplayPoint();
        if (!display) {
            return;
        }
        if (!std::isfinite(display[0]) || !std::isfinite(display[1]) || !std::isfinite(display[2])) {
            return;
        }
        if (display[2] < 0.0 || display[2] > 1.0) {
            return;
        }

        const double screenX = display[0];
        const double screenY = static_cast<double>(viewportHeight) - display[1];
        if (screenX < static_cast<double>(minX) ||
            screenX > static_cast<double>(maxX) ||
            screenY < static_cast<double>(minY) ||
            screenY > static_cast<double>(maxY)) {
            return;
        }

        if (uniqueAtomIds.insert(atom.id).second) {
            atomIds.push_back(atom.id);
        }
    };

    for (const auto& atom : createdAtoms) {
        appendIfInsideRect(atom);
    }
    if (includeSurrounding) {
        for (const auto& atom : surroundingAtoms) {
            appendIfInsideRect(atom);
        }
    }

    return atomIds;
}

void WorkspaceRuntimeModel::applyDragSelectionToMeasurement(const std::vector<uint32_t>& atomIds, bool additive) {
    if (!IsMeasurementModeActive()) {
        return;
    }

    const bool centerMode = isCenterMeasurementMode();
    const size_t targetPickCount = measurementTargetPickCount();
    if (!centerMode && targetPickCount == 0) {
        return;
    }

    if (!additive) {
        m_MeasurementPickedAtomIds.clear();
    }
    if (!centerMode && m_MeasurementPickedAtomIds.size() >= targetPickCount && !atomIds.empty()) {
        m_MeasurementPickedAtomIds.clear();
    }

    for (uint32_t atomId : atomIds) {
        if (atomId == 0) {
            continue;
        }
        if (std::find(
                m_MeasurementPickedAtomIds.begin(),
                m_MeasurementPickedAtomIds.end(),
                atomId) != m_MeasurementPickedAtomIds.end()) {
            continue;
        }

        const atoms::domain::AtomInfo* atom = findAtomById(atomId);
        if (!atom || atom->structureId < 0) {
            continue;
        }

        if (m_MeasurementPickedAtomIds.empty()) {
            m_MeasurementPickedAtomIds.push_back(atomId);
        } else {
            const atoms::domain::AtomInfo* firstAtom = findAtomById(m_MeasurementPickedAtomIds.front());
            if (!firstAtom || firstAtom->structureId != atom->structureId) {
                m_MeasurementPickedAtomIds.clear();
                m_MeasurementPickedAtomIds.push_back(atomId);
            } else if (centerMode || m_MeasurementPickedAtomIds.size() < targetPickCount) {
                m_MeasurementPickedAtomIds.push_back(atomId);
            }
        }

        if (!centerMode && m_MeasurementPickedAtomIds.size() >= targetPickCount) {
            break;
        }
    }

    syncMeasurementPickVisuals();
    if (!centerMode && m_MeasurementPickedAtomIds.size() == targetPickCount) {
        createMeasurementFromPickedAtoms();
    }
    requestViewerRender();
}

void WorkspaceRuntimeModel::applyDragSelectionToCreatedAtoms(const std::vector<uint32_t>& atomIds, bool additive) {
    if (IsMeasurementModeActive()) {
        return;
    }

    std::unordered_set<uint32_t> selectedIds;
    selectedIds.reserve(atomIds.size());
    for (uint32_t atomId : atomIds) {
        if (atomId != 0) {
            selectedIds.insert(atomId);
        }
    }

    bool changed = false;
    for (auto& atom : createdAtoms) {
        const bool shouldSelect = additive
            ? (atom.selected || selectedIds.find(atom.id) != selectedIds.end())
            : (selectedIds.find(atom.id) != selectedIds.end());
        if (atom.selected != shouldSelect) {
            atom.selected = shouldSelect;
            changed = true;
        }
    }

    if (changed) {
        syncCreatedAtomSelectionVisuals();
        requestViewerRender();
    }
}

void WorkspaceRuntimeModel::clearCreatedAtomSelectionVisuals() {
    for (auto& visual : m_CreatedAtomSelectionVisuals) {
        if (visual.shellActor) {
            render::application::GetRenderGateway().RemoveActor(visual.shellActor);
            visual.shellActor = nullptr;
        }
    }
    m_CreatedAtomSelectionVisuals.clear();
}

void WorkspaceRuntimeModel::syncCreatedAtomSelectionVisuals() {
    clearCreatedAtomSelectionVisuals();
    if (IsMeasurementModeActive()) {
        return;
    }

    for (const auto& atom : createdAtoms) {
        if (!atom.selected || atom.id == 0 || atom.structureId < 0) {
            continue;
        }
        if (!IsStructureVisible(atom.structureId) || !IsAtomVisibleById(atom.id)) {
            continue;
        }

        CreatedAtomSelectionVisual visual;
        visual.atomId = atom.id;
        visual.shellActor = createCreatedAtomSelectionShellActor(atom);
        if (visual.shellActor) {
            render::application::GetRenderGateway().AddActor(visual.shellActor, false);
        }
        m_CreatedAtomSelectionVisuals.push_back(std::move(visual));
    }
}

void WorkspaceRuntimeModel::clearMeasurementPickVisuals() {
    for (auto& visual : m_MeasurementPickVisuals) {
        if (visual.shellActor) {
            render::application::GetRenderGateway().RemoveActor(visual.shellActor);
            visual.shellActor = nullptr;
        }
        if (visual.orderTextActor) {
            render::application::GetRenderGateway().RemoveActor2D(visual.orderTextActor);
            visual.orderTextActor = nullptr;
        }
    }
    m_MeasurementPickVisuals.clear();
}

void WorkspaceRuntimeModel::syncMeasurementPickVisuals() {
    clearMeasurementPickVisuals();
    if (!IsMeasurementModeActive()) {
        return;
    }

    const bool centerMode = isCenterMeasurementMode();
    std::vector<uint32_t> validPickedAtomIds;
    validPickedAtomIds.reserve(m_MeasurementPickedAtomIds.size());
    for (size_t index = 0; index < m_MeasurementPickedAtomIds.size(); ++index) {
        const uint32_t atomId = m_MeasurementPickedAtomIds[index];
        const atoms::domain::AtomInfo* atom = findAtomById(atomId);
        if (!atom || atom->structureId < 0) {
            continue;
        }
        validPickedAtomIds.push_back(atomId);

        MeasurementPickVisual visual;
        visual.atomId = atomId;
        visual.order = index + 1;
        visual.shellActor = createMeasurementPickShellActor(*atom, visual.order);
        if (!centerMode) {
            visual.orderTextActor = createMeasurementOrderTextActor(*atom, visual.order);
        }

        const bool visible = !m_MeasurementVisibilitySuppressedByBZ &&
            IsStructureVisible(atom->structureId) &&
            IsAtomVisibleById(atomId);
        if (visual.shellActor) {
            visual.shellActor->SetVisibility(visible ? 1 : 0);
            render::application::GetRenderGateway().AddActor(visual.shellActor, false);
        }
        if (visual.orderTextActor) {
            visual.orderTextActor->SetVisibility(visible ? 1 : 0);
            render::application::GetRenderGateway().AddActor2D(visual.orderTextActor, false);
        }
        m_MeasurementPickVisuals.push_back(std::move(visual));
    }
    m_MeasurementPickedAtomIds = std::move(validPickedAtomIds);
}

size_t WorkspaceRuntimeModel::measurementTargetPickCount() const {
    switch (m_MeasurementMode) {
    case MeasurementMode::Distance:
        return 2;
    case MeasurementMode::Angle:
        return 3;
    case MeasurementMode::Dihedral:
        return 4;
    case MeasurementMode::GeometricCenter:
    case MeasurementMode::CenterOfMass:
        return 0;
    default:
        return 0;
    }
}

void WorkspaceRuntimeModel::createMeasurementFromPickedAtoms() {
    switch (m_MeasurementMode) {
    case MeasurementMode::Distance:
        if (m_MeasurementPickedAtomIds.size() == 2) {
            createDistanceMeasurement(m_MeasurementPickedAtomIds[0], m_MeasurementPickedAtomIds[1]);
        }
        break;
    case MeasurementMode::Angle:
        if (m_MeasurementPickedAtomIds.size() == 3) {
            createAngleMeasurement(
                m_MeasurementPickedAtomIds[0],
                m_MeasurementPickedAtomIds[1],
                m_MeasurementPickedAtomIds[2]);
        }
        break;
    case MeasurementMode::Dihedral:
        if (m_MeasurementPickedAtomIds.size() == 4) {
            createDihedralMeasurement(
                m_MeasurementPickedAtomIds[0],
                m_MeasurementPickedAtomIds[1],
                m_MeasurementPickedAtomIds[2],
                m_MeasurementPickedAtomIds[3]);
        }
        break;
    case MeasurementMode::GeometricCenter:
    case MeasurementMode::CenterOfMass:
        commitCenterMeasurementFromPickedAtoms();
        break;
    default:
        break;
    }
}

void WorkspaceRuntimeModel::clampMeasurementStyles() {
    auto clampColor = [](std::array<float, 3>& color) {
        for (float& channel : color) {
            channel = clampFloatRange(channel, 0.0f, 1.0f);
        }
    };

    clampColor(m_DistanceStyle.lineColor);
    m_DistanceStyle.lineWidth = clampFloatRange(
        m_DistanceStyle.lineWidth,
        kMinMeasurementLineWidth,
        kMaxMeasurementLineWidth);
    m_DistanceStyle.labelScale = snapToOneDecimal(clampFloatRange(
        m_DistanceStyle.labelScale,
        kMinMeasurementLabelScale,
        kMaxMeasurementLabelScale));

    clampColor(m_AngleStyle.lineColor);
    m_AngleStyle.lineWidth = clampFloatRange(
        m_AngleStyle.lineWidth,
        kMinMeasurementLineWidth,
        kMaxMeasurementLineWidth);
    m_AngleStyle.arcRadiusScale = snapToOneDecimal(clampFloatRange(
        m_AngleStyle.arcRadiusScale,
        kMinAngleArcRadiusScale,
        kMaxAngleArcRadiusScale));
    m_AngleStyle.labelScale = snapToOneDecimal(clampFloatRange(
        m_AngleStyle.labelScale,
        kMinMeasurementLabelScale,
        kMaxMeasurementLabelScale));

    clampColor(m_DihedralStyle.baseLineColor);
    m_DihedralStyle.baseLineWidth = clampFloatRange(
        m_DihedralStyle.baseLineWidth,
        kMinMeasurementLineWidth,
        kMaxMeasurementLineWidth);
    clampColor(m_DihedralStyle.helperPlane1Color);
    m_DihedralStyle.helperPlane1Opacity = clampFloatRange(
        m_DihedralStyle.helperPlane1Opacity,
        kMinDihedralPlaneOpacity,
        kMaxDihedralPlaneOpacity);
    clampColor(m_DihedralStyle.helperPlane2Color);
    m_DihedralStyle.helperPlane2Opacity = clampFloatRange(
        m_DihedralStyle.helperPlane2Opacity,
        kMinDihedralPlaneOpacity,
        kMaxDihedralPlaneOpacity);
    m_DihedralStyle.labelScale = snapToOneDecimal(clampFloatRange(
        m_DihedralStyle.labelScale,
        kMinMeasurementLabelScale,
        kMaxMeasurementLabelScale));

    auto clampCenterStyle = [&](CenterStyle& style) {
        clampColor(style.centerPlusColor);
        style.centerPlusScale = snapToOneDecimal(clampFloatRange(
            style.centerPlusScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale));
        clampColor(style.selectedPlusColor);
        style.selectedPlusScale = snapToOneDecimal(clampFloatRange(
            style.selectedPlusScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale));
        style.coordinateLabelScale = snapToOneDecimal(clampFloatRange(
            style.coordinateLabelScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale));
    };
    clampCenterStyle(m_GeometricCenterStyle);
    clampCenterStyle(m_CenterOfMassStyle);
}

void WorkspaceRuntimeModel::applyDistanceStyleToMeasurement(DistanceMeasurement& measurement) {
    applyLineStyleToActor(
        measurement.lineActor,
        m_DistanceStyle.lineColor,
        m_DistanceStyle.lineWidth);
    applyTextScaleToActor(
        measurement.textActor,
        m_DistanceStyle.labelScale,
        kMeasurementValueBaseFontSize);
}

void WorkspaceRuntimeModel::applyAngleStyleToMeasurement(AngleMeasurement& measurement) {
    applyLineStyleToActor(
        measurement.lineActor12,
        m_AngleStyle.lineColor,
        m_AngleStyle.lineWidth);
    applyLineStyleToActor(
        measurement.lineActor23,
        m_AngleStyle.lineColor,
        m_AngleStyle.lineWidth);
    applyLineStyleToActor(
        measurement.arcActor,
        m_AngleStyle.lineColor,
        m_AngleStyle.lineWidth);
    applyTextScaleToActor(
        measurement.textActor,
        m_AngleStyle.labelScale,
        kMeasurementValueBaseFontSize);
}

void WorkspaceRuntimeModel::applyDihedralStyleToMeasurement(DihedralMeasurement& measurement) {
    applyLineStyleToActor(
        measurement.lineActor12,
        m_DihedralStyle.baseLineColor,
        m_DihedralStyle.baseLineWidth);
    applyLineStyleToActor(
        measurement.lineActor23,
        m_DihedralStyle.baseLineColor,
        m_DihedralStyle.baseLineWidth);
    applyLineStyleToActor(
        measurement.lineActor34,
        m_DihedralStyle.baseLineColor,
        m_DihedralStyle.baseLineWidth);

    if (measurement.helperPlaneActor1) {
        vtkProperty* property = measurement.helperPlaneActor1->GetProperty();
        if (property) {
            property->SetColor(
                static_cast<double>(m_DihedralStyle.helperPlane1Color[0]),
                static_cast<double>(m_DihedralStyle.helperPlane1Color[1]),
                static_cast<double>(m_DihedralStyle.helperPlane1Color[2]));
            property->SetOpacity(static_cast<double>(m_DihedralStyle.helperPlane1Opacity));
            property->SetRepresentationToSurface();
            property->SetEdgeVisibility(false);
        }
    }
    if (measurement.helperPlaneActor2) {
        vtkProperty* property = measurement.helperPlaneActor2->GetProperty();
        if (property) {
            property->SetColor(
                static_cast<double>(m_DihedralStyle.helperPlane2Color[0]),
                static_cast<double>(m_DihedralStyle.helperPlane2Color[1]),
                static_cast<double>(m_DihedralStyle.helperPlane2Color[2]));
            property->SetOpacity(static_cast<double>(m_DihedralStyle.helperPlane2Opacity));
            property->SetRepresentationToSurface();
            property->SetEdgeVisibility(false);
        }
    }

    applyTextScaleToActor(
        measurement.textActor,
        m_DihedralStyle.labelScale,
        kMeasurementValueBaseFontSize);
}

void WorkspaceRuntimeModel::applyCenterStyleToMeasurement(CenterMeasurement& measurement) {
    const CenterStyle& activeStyle =
        measurement.type == MeasurementType::CenterOfMass
        ? m_CenterOfMassStyle
        : m_GeometricCenterStyle;

    if (measurement.centerPlusTextActor) {
        vtkTextProperty* property = textPropertyFromActor2D(measurement.centerPlusTextActor);
        if (property) {
            property->SetColor(
                static_cast<double>(activeStyle.centerPlusColor[0]),
                static_cast<double>(activeStyle.centerPlusColor[1]),
                static_cast<double>(activeStyle.centerPlusColor[2]));
            const int baseFontSize = measurement.centerPlusBaseFontSize > 0
                ? measurement.centerPlusBaseFontSize
                : property->GetFontSize();
            property->SetFontSize(scaledFontSize(baseFontSize, activeStyle.centerPlusScale));
        }
    }

    for (size_t i = 0; i < measurement.selectedAtomPlusTextActors.size(); ++i) {
        vtkSmartPointer<vtkActor2D>& actor = measurement.selectedAtomPlusTextActors[i];
        vtkTextProperty* property = textPropertyFromActor2D(actor);
        if (!property) {
            continue;
        }
        property->SetColor(
            static_cast<double>(activeStyle.selectedPlusColor[0]),
            static_cast<double>(activeStyle.selectedPlusColor[1]),
            static_cast<double>(activeStyle.selectedPlusColor[2]));
        const int baseFontSize =
            (i < measurement.selectedAtomPlusBaseFontSizes.size() &&
             measurement.selectedAtomPlusBaseFontSizes[i] > 0)
            ? measurement.selectedAtomPlusBaseFontSizes[i]
            : property->GetFontSize();
        property->SetFontSize(scaledFontSize(baseFontSize, activeStyle.selectedPlusScale));
    }

    if (measurement.centerCoordinateTextActor) {
        vtkTextProperty* property = textPropertyFromActor2D(measurement.centerCoordinateTextActor);
        if (property) {
            const int baseFontSize = measurement.centerCoordinateBaseFontSize > 0
                ? measurement.centerCoordinateBaseFontSize
                : kMeasurementValueBaseFontSize;
            property->SetFontSize(scaledFontSize(baseFontSize, activeStyle.coordinateLabelScale));
        }
    }
}

bool WorkspaceRuntimeModel::rebuildAngleMeasurementGeometry(AngleMeasurement& measurement) {
    const atoms::domain::AtomInfo* atom1 = findAtomById(measurement.atomId1);
    const atoms::domain::AtomInfo* atom2 = findAtomById(measurement.atomId2);
    const atoms::domain::AtomInfo* atom3 = findAtomById(measurement.atomId3);
    if (!atom1 || !atom2 || !atom3) {
        return false;
    }
    if (atom1->structureId < 0 ||
        atom1->structureId != atom2->structureId ||
        atom1->structureId != atom3->structureId ||
        atom1->structureId != measurement.structureId) {
        return false;
    }

    double angleDeg = 0.0;
    std::array<double, 3> arcStart {};
    std::array<double, 3> arcEnd {};
    std::array<double, 3> arcCenter {};
    std::array<double, 3> textPos {};
    if (!computeAngleMeasurementGeometry(
            atom1->position,
            atom2->position,
            atom3->position,
            static_cast<double>(m_AngleStyle.arcRadiusScale),
            angleDeg,
            arcStart,
            arcEnd,
            arcCenter,
            textPos)) {
        return false;
    }

    vtkSmartPointer<vtkActor> newArcActor = createAngleArcActor(arcStart, arcEnd, arcCenter);
    vtkSmartPointer<vtkActor2D> newTextActor =
        createMeasurementValueTextActor(formatAngleText(angleDeg), textPos);

    if (measurement.arcActor) {
        render::application::GetRenderGateway().RemoveMeasurementOverlayActor(measurement.arcActor);
    }
    if (measurement.textActor) {
        render::application::GetRenderGateway().RemoveActor2D(measurement.textActor);
    }

    measurement.angleDeg = angleDeg;
    measurement.arcActor = std::move(newArcActor);
    measurement.textActor = std::move(newTextActor);

    if (measurement.arcActor) {
        render::application::GetRenderGateway().AddMeasurementOverlayActor(measurement.arcActor, false);
    }
    if (measurement.textActor) {
        render::application::GetRenderGateway().AddActor2D(measurement.textActor, false, true);
    }

    return true;
}

void WorkspaceRuntimeModel::applyDistanceStyleToAllMeasurements() {
    for (auto& measurement : m_DistanceMeasurements) {
        applyDistanceStyleToMeasurement(measurement);
        applyDistanceMeasurementVisibility(measurement);
    }
}

void WorkspaceRuntimeModel::applyAngleStyleToAllMeasurements(bool rebuildGeometry) {
    for (auto& measurement : m_AngleMeasurements) {
        if (rebuildGeometry) {
            rebuildAngleMeasurementGeometry(measurement);
        }
        applyAngleStyleToMeasurement(measurement);
        applyAngleMeasurementVisibility(measurement);
    }
}

void WorkspaceRuntimeModel::applyDihedralStyleToAllMeasurements() {
    for (auto& measurement : m_DihedralMeasurements) {
        applyDihedralStyleToMeasurement(measurement);
        applyDihedralMeasurementVisibility(measurement);
    }
}

void WorkspaceRuntimeModel::applyCenterStyleToAllMeasurements() {
    for (auto& measurement : m_CenterMeasurements) {
        applyCenterStyleToMeasurement(measurement);
        applyCenterMeasurementVisibility(measurement);
    }
}

void WorkspaceRuntimeModel::renderMeasurementStyleOptionsOverlay() {
    bool styleChanged = false;
    bool rebuildAngleGeometry = false;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Style");

    constexpr ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs;

    switch (m_MeasurementMode) {
    case MeasurementMode::Distance:
        styleChanged |= ImGui::ColorEdit3(
            "Line Color##DistanceStyle",
            m_DistanceStyle.lineColor.data(),
            colorFlags);
        styleChanged |= ImGui::SliderFloat(
            "Line Width (px)##DistanceStyle",
            &m_DistanceStyle.lineWidth,
            kMinMeasurementLineWidth,
            kMaxMeasurementLineWidth,
            "%.1f");
        styleChanged |= ImGui::SliderFloat(
            "Value Label Size##DistanceStyle",
            &m_DistanceStyle.labelScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale,
            "%.1fx");
        if (ImGui::Button("Reset Style##DistanceStyle")) {
            m_DistanceStyle = DistanceStyle {};
            styleChanged = true;
        }
        break;
    case MeasurementMode::Angle:
        styleChanged |= ImGui::ColorEdit3(
            "Line(1,2) Color##AngleStyle",
            m_AngleStyle.lineColor.data(),
            colorFlags);
        styleChanged |= ImGui::SliderFloat(
            "Line Width (px)##AngleStyle",
            &m_AngleStyle.lineWidth,
            kMinMeasurementLineWidth,
            kMaxMeasurementLineWidth,
            "%.1f");
        if (ImGui::SliderFloat(
                "Arc Radius Scale##AngleStyle",
                &m_AngleStyle.arcRadiusScale,
                kMinAngleArcRadiusScale,
                kMaxAngleArcRadiusScale,
                "%.1fx")) {
            styleChanged = true;
            rebuildAngleGeometry = true;
        }
        styleChanged |= ImGui::SliderFloat(
            "Value Label Size##AngleStyle",
            &m_AngleStyle.labelScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale,
            "%.1fx");
        if (ImGui::Button("Reset Style##AngleStyle")) {
            m_AngleStyle = AngleStyle {};
            styleChanged = true;
            rebuildAngleGeometry = true;
        }
        break;
    case MeasurementMode::Dihedral:
        styleChanged |= ImGui::ColorEdit3(
            "Base Line Color##DihedralStyle",
            m_DihedralStyle.baseLineColor.data(),
            colorFlags);
        styleChanged |= ImGui::SliderFloat(
            "Base Line Width (px)##DihedralStyle",
            &m_DihedralStyle.baseLineWidth,
            kMinMeasurementLineWidth,
            kMaxMeasurementLineWidth,
            "%.1f");
        styleChanged |= ImGui::ColorEdit3(
            "Helper Plane 1 Color##DihedralStyle",
            m_DihedralStyle.helperPlane1Color.data(),
            colorFlags);
        styleChanged |= ImGui::SliderFloat(
            "Helper Plane 1 Opacity##DihedralStyle",
            &m_DihedralStyle.helperPlane1Opacity,
            kMinDihedralPlaneOpacity,
            kMaxDihedralPlaneOpacity,
            "%.2f");
        styleChanged |= ImGui::ColorEdit3(
            "Helper Plane 2 Color##DihedralStyle",
            m_DihedralStyle.helperPlane2Color.data(),
            colorFlags);
        styleChanged |= ImGui::SliderFloat(
            "Helper Plane 2 Opacity##DihedralStyle",
            &m_DihedralStyle.helperPlane2Opacity,
            kMinDihedralPlaneOpacity,
            kMaxDihedralPlaneOpacity,
            "%.2f");
        styleChanged |= ImGui::SliderFloat(
            "Value Label Size##DihedralStyle",
            &m_DihedralStyle.labelScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale,
            "%.1fx");
        if (ImGui::Button("Reset Style##DihedralStyle")) {
            m_DihedralStyle = DihedralStyle {};
            styleChanged = true;
        }
        break;
    case MeasurementMode::GeometricCenter: {
        CenterStyle& style = m_GeometricCenterStyle;
        styleChanged |= ImGui::ColorEdit3(
            "Center + Color##GeometricCenterStyle",
            style.centerPlusColor.data(),
            colorFlags);
        styleChanged |= ImGui::SliderFloat(
            "Center + Size##GeometricCenterStyle",
            &style.centerPlusScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale,
            "%.1fx");
        styleChanged |= ImGui::ColorEdit3(
            "Selected + Color##GeometricCenterStyle",
            style.selectedPlusColor.data(),
            colorFlags);
        styleChanged |= ImGui::SliderFloat(
            "Selected + Size##GeometricCenterStyle",
            &style.selectedPlusScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale,
            "%.1fx");
        styleChanged |= ImGui::SliderFloat(
            "Coordinate Text Size##GeometricCenterStyle",
            &style.coordinateLabelScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale,
            "%.1fx");
        if (ImGui::Button("Reset Style##GeometricCenterStyle")) {
            style = CenterStyle {};
            styleChanged = true;
        }
        break;
    }
    case MeasurementMode::CenterOfMass: {
        CenterStyle& style = m_CenterOfMassStyle;
        styleChanged |= ImGui::ColorEdit3(
            "Center + Color##CenterOfMassStyle",
            style.centerPlusColor.data(),
            colorFlags);
        styleChanged |= ImGui::SliderFloat(
            "Center + Size##CenterOfMassStyle",
            &style.centerPlusScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale,
            "%.1fx");
        styleChanged |= ImGui::ColorEdit3(
            "Selected + Color##CenterOfMassStyle",
            style.selectedPlusColor.data(),
            colorFlags);
        styleChanged |= ImGui::SliderFloat(
            "Selected + Size##CenterOfMassStyle",
            &style.selectedPlusScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale,
            "%.1fx");
        styleChanged |= ImGui::SliderFloat(
            "Coordinate Text Size##CenterOfMassStyle",
            &style.coordinateLabelScale,
            kMinMeasurementLabelScale,
            kMaxMeasurementLabelScale,
            "%.1fx");
        if (ImGui::Button("Reset Style##CenterOfMassStyle")) {
            style = CenterStyle {};
            styleChanged = true;
        }
        break;
    }
    default:
        break;
    }

    if (!styleChanged) {
        return;
    }

    clampMeasurementStyles();
    switch (m_MeasurementMode) {
    case MeasurementMode::Distance:
        applyDistanceStyleToAllMeasurements();
        break;
    case MeasurementMode::Angle:
        applyAngleStyleToAllMeasurements(rebuildAngleGeometry);
        break;
    case MeasurementMode::Dihedral:
        applyDihedralStyleToAllMeasurements();
        break;
    case MeasurementMode::GeometricCenter:
    case MeasurementMode::CenterOfMass:
        applyCenterStyleToAllMeasurements();
        break;
    default:
        break;
    }
    requestViewerRender();
}

std::string WorkspaceRuntimeModel::buildMeasurementAtomLabel(const atoms::domain::AtomInfo* atom) const {
    if (!atom) {
        return "?#?";
    }

    auto findOriginalSerial = [&](uint32_t originalAtomId) -> std::string {
        if (originalAtomId == 0) {
            return "?";
        }
        for (size_t atomIndex = 0; atomIndex < createdAtoms.size(); ++atomIndex) {
            if (createdAtoms[atomIndex].id == originalAtomId) {
                return std::to_string(atomIndex + 1);
            }
        }
        return "?";
    };

    if (atom->atomType == atoms::domain::AtomType::ORIGINAL) {
        return atom->symbol + "#" + findOriginalSerial(atom->id);
    }
    return atom->symbol + "(#" + findOriginalSerial(atom->originalAtomId) + ")";
}

std::string WorkspaceRuntimeModel::buildCenterMeasurementDisplayName(
    MeasurementType type,
    const std::vector<uint32_t>& atomIds) const {
    const char* prefix = "Center";
    if (type == MeasurementType::GeometricCenter) {
        prefix = "Geometric Center";
    } else if (type == MeasurementType::CenterOfMass) {
        prefix = "Center of Mass";
    }

    constexpr size_t kPreviewLimit = 3;
    std::ostringstream oss;
    oss << prefix << " (" << atomIds.size() << " atoms: ";
    if (atomIds.empty()) {
        oss << "?";
    } else {
        const size_t previewCount = std::min(atomIds.size(), kPreviewLimit);
        for (size_t i = 0; i < previewCount; ++i) {
            const atoms::domain::AtomInfo* atom = findAtomById(atomIds[i]);
            if (i > 0) {
                oss << "-";
            }
            oss << buildMeasurementAtomLabel(atom);
        }
        if (atomIds.size() > previewCount) {
            oss << "-...";
        }
    }
    oss << ")";
    return oss.str();
}

bool WorkspaceRuntimeModel::createGeometricCenterMeasurement(const std::vector<uint32_t>& atomIds) {
    return createCenterMeasurement(MeasurementType::GeometricCenter, atomIds);
}

bool WorkspaceRuntimeModel::createCenterOfMassMeasurement(const std::vector<uint32_t>& atomIds) {
    return createCenterMeasurement(MeasurementType::CenterOfMass, atomIds);
}

bool WorkspaceRuntimeModel::createCenterMeasurement(
    MeasurementType type,
    const std::vector<uint32_t>& atomIds) {
    if (atomIds.size() < 2) {
        return false;
    }

    std::unordered_set<uint32_t> uniqueAtomIds;
    uniqueAtomIds.reserve(atomIds.size());

    std::vector<const atoms::domain::AtomInfo*> atoms;
    atoms.reserve(atomIds.size());
    int32_t structureId = -1;
    for (uint32_t atomId : atomIds) {
        if (atomId == 0 || !uniqueAtomIds.insert(atomId).second) {
            continue;
        }
        const atoms::domain::AtomInfo* atom = findAtomById(atomId);
        if (!atom || atom->structureId < 0) {
            continue;
        }
        if (structureId < 0) {
            structureId = atom->structureId;
        }
        if (atom->structureId != structureId) {
            return false;
        }
        atoms.push_back(atom);
    }
    if (atoms.size() < 2 || structureId < 0) {
        return false;
    }

    std::array<double, 3> centerPos { 0.0, 0.0, 0.0 };
    if (type == MeasurementType::GeometricCenter) {
        for (const auto* atom : atoms) {
            centerPos[0] += static_cast<double>(atom->position[0]);
            centerPos[1] += static_cast<double>(atom->position[1]);
            centerPos[2] += static_cast<double>(atom->position[2]);
        }
        const double scale = 1.0 / static_cast<double>(atoms.size());
        centerPos[0] *= scale;
        centerPos[1] *= scale;
        centerPos[2] *= scale;
    } else if (type == MeasurementType::CenterOfMass) {
        double totalMass = 0.0;
        for (const auto* atom : atoms) {
            double mass = 0.0;
            if (m_elementDB) {
                mass = static_cast<double>(m_elementDB->getAtomicMass(atom->symbol));
            }
            if (mass <= 0.0) {
                SPDLOG_WARN(
                    "Invalid atomic mass for '{}' while computing center of mass. Falling back to 1.0",
                    atom->symbol);
                mass = 1.0;
            }
            centerPos[0] += static_cast<double>(atom->position[0]) * mass;
            centerPos[1] += static_cast<double>(atom->position[1]) * mass;
            centerPos[2] += static_cast<double>(atom->position[2]) * mass;
            totalMass += mass;
        }
        if (totalMass <= 1e-12) {
            return false;
        }
        centerPos[0] /= totalMass;
        centerPos[1] /= totalMass;
        centerPos[2] /= totalMass;
    } else {
        return false;
    }

    std::vector<uint32_t> measurementAtomIds;
    measurementAtomIds.reserve(atoms.size());
    double markerRadiusSum = 0.0;
    for (const auto* atom : atoms) {
        measurementAtomIds.push_back(atom->id);
        markerRadiusSum += renderedSelectionSphereRadius(*atom);
    }
    const double markerRadius =
        std::max(0.05, markerRadiusSum / static_cast<double>(atoms.size()));
    const size_t styleIndex = m_NextCenterMarkerStyleIndex++;
    const CenterStyle& activeStyle =
        type == MeasurementType::CenterOfMass
        ? m_CenterOfMassStyle
        : m_GeometricCenterStyle;
    const std::array<double, 3> centerPlusColor = toDoubleColor(activeStyle.centerPlusColor);
    constexpr std::array<double, 3> kCenterCircleColor = { 1.0, 0.0, 0.0 };
    const std::array<double, 3> selectedPlusColor = toDoubleColor(activeStyle.selectedPlusColor);
    constexpr std::array<double, 3> kSelectedCircleColor = { 0.55, 0.55, 0.55 };
    constexpr double kCenterCircleOpacity = 0.4;
    constexpr double kSelectedCircleOpacity = 0.35;
    const std::array<double, 3> centerTextPos = {
        centerPos[0],
        centerPos[1],
        centerPos[2] - std::max(0.04, markerRadius * 1.35)
    };

    CenterMeasurement measurement;
    measurement.id = m_NextMeasurementId++;
    measurement.structureId = structureId;
    measurement.type = type;
    measurement.atomIds = std::move(measurementAtomIds);
    measurement.centerPos = centerPos;
    measurement.displayName = buildCenterMeasurementDisplayName(type, measurement.atomIds);
    measurement.visible = true;
    measurement.styleIndex = styleIndex;
    measurement.centerBackgroundCircleActor = createCircleTextActor(
        centerPos,
        markerRadius,
        kCenterCircleColor,
        kCenterCircleOpacity);
    measurement.centerPlusTextActor = createPlusTextActor(
        centerPos,
        markerRadius,
        centerPlusColor);
    measurement.centerCoordinateTextActor = createMeasurementValueTextActor(
        formatCenterCoordinateText(type, centerPos),
        centerTextPos);
    if (measurement.centerPlusTextActor) {
        vtkTextProperty* property = textPropertyFromActor2D(measurement.centerPlusTextActor);
        if (property) {
            measurement.centerPlusBaseFontSize = property->GetFontSize();
        }
    }
    if (measurement.centerCoordinateTextActor) {
        vtkTextProperty* property = textPropertyFromActor2D(measurement.centerCoordinateTextActor);
        if (property) {
            measurement.centerCoordinateBaseFontSize = property->GetFontSize();
        }
    }

    for (const auto* atom : atoms) {
        const std::array<double, 3> atomPos = {
            static_cast<double>(atom->position[0]),
            static_cast<double>(atom->position[1]),
            static_cast<double>(atom->position[2])
        };
        const double atomMarkerRadius = renderedSelectionSphereRadius(*atom);
        vtkSmartPointer<vtkActor2D> atomBackgroundCircle = createCircleTextActor(
            atomPos,
            atomMarkerRadius,
            kSelectedCircleColor,
            kSelectedCircleOpacity);
        if (atomBackgroundCircle) {
            render::application::GetRenderGateway().AddActor2D(atomBackgroundCircle, false);
            measurement.selectedAtomBackgroundCircleActors.push_back(std::move(atomBackgroundCircle));
        }
        vtkSmartPointer<vtkActor2D> atomPlusText = createPlusTextActor(
            atomPos,
            atomMarkerRadius,
            selectedPlusColor);
        if (atomPlusText) {
            vtkTextProperty* property = textPropertyFromActor2D(atomPlusText);
            if (property) {
                measurement.selectedAtomPlusBaseFontSizes.push_back(property->GetFontSize());
            } else {
                measurement.selectedAtomPlusBaseFontSizes.push_back(0);
            }
            render::application::GetRenderGateway().AddActor2D(atomPlusText, false);
            measurement.selectedAtomPlusTextActors.push_back(std::move(atomPlusText));
        }
    }
    if (measurement.centerBackgroundCircleActor) {
        render::application::GetRenderGateway().AddActor2D(measurement.centerBackgroundCircleActor, false);
    }
    if (measurement.centerPlusTextActor) {
        render::application::GetRenderGateway().AddActor2D(measurement.centerPlusTextActor, false);
    }
    if (measurement.centerCoordinateTextActor) {
        render::application::GetRenderGateway().AddActor2D(measurement.centerCoordinateTextActor, false, true);
    }
    applyCenterStyleToMeasurement(measurement);
    applyCenterMeasurementVisibility(measurement);
    m_CenterMeasurements.push_back(std::move(measurement));
    return true;
}

bool WorkspaceRuntimeModel::commitCenterMeasurementFromPickedAtoms() {
    if (!isCenterMeasurementMode()) {
        return false;
    }

    std::vector<uint32_t> atomIds;
    atomIds.reserve(m_MeasurementPickedAtomIds.size());
    std::unordered_set<uint32_t> uniqueAtomIds;
    uniqueAtomIds.reserve(m_MeasurementPickedAtomIds.size());

    int32_t structureId = -1;
    for (uint32_t atomId : m_MeasurementPickedAtomIds) {
        if (atomId == 0 || !uniqueAtomIds.insert(atomId).second) {
            continue;
        }
        const atoms::domain::AtomInfo* atom = findAtomById(atomId);
        if (!atom || atom->structureId < 0) {
            continue;
        }
        if (structureId < 0) {
            structureId = atom->structureId;
        }
        if (atom->structureId != structureId) {
            continue;
        }
        atomIds.push_back(atomId);
    }
    if (atomIds.size() < 2) {
        m_MeasurementPickedAtomIds = std::move(atomIds);
        syncMeasurementPickVisuals();
        requestViewerRender();
        return false;
    }

    bool created = false;
    if (m_MeasurementMode == MeasurementMode::GeometricCenter) {
        created = createGeometricCenterMeasurement(atomIds);
    } else if (m_MeasurementMode == MeasurementMode::CenterOfMass) {
        created = createCenterOfMassMeasurement(atomIds);
    }
    if (!created) {
        return false;
    }

    m_MeasurementPickedAtomIds.clear();
    clearMeasurementPickVisuals();
    requestViewerRender();
    return true;
}

void WorkspaceRuntimeModel::createDistanceMeasurement(uint32_t atomId1, uint32_t atomId2) {
    if (atomId1 == 0 || atomId2 == 0 || atomId1 == atomId2) {
        return;
    }

    const atoms::domain::AtomInfo* atom1 = findAtomById(atomId1);
    const atoms::domain::AtomInfo* atom2 = findAtomById(atomId2);
    if (!atom1 || !atom2) {
        return;
    }
    if (atom1->structureId < 0 || atom1->structureId != atom2->structureId) {
        return;
    }

    DistanceMeasurement measurement;
    measurement.id = m_NextMeasurementId++;
    measurement.structureId = atom1->structureId;
    measurement.atomId1 = atomId1;
    measurement.atomId2 = atomId2;
    measurement.distance = calcDistance(atom1->position, atom2->position);
    measurement.displayName = "Distance (" +
        buildMeasurementAtomLabel(atom1) + "-" +
        buildMeasurementAtomLabel(atom2) + ")";
    measurement.visible = true;
    measurement.lineActor = createSegmentedDistanceLineActor(atom1->position, atom2->position);
    measurement.textActor = createDistanceTextActor(
        formatDistanceText(measurement.distance), atom1->position, atom2->position);
    applyDistanceStyleToMeasurement(measurement);

    if (measurement.lineActor) {
        render::application::GetRenderGateway().AddMeasurementOverlayActor(measurement.lineActor, false);
    }
    if (measurement.textActor) {
        render::application::GetRenderGateway().AddActor2D(measurement.textActor, false, true);
    }
    applyDistanceMeasurementVisibility(measurement);
    m_DistanceMeasurements.push_back(std::move(measurement));
}

void WorkspaceRuntimeModel::createAngleMeasurement(uint32_t atomId1, uint32_t atomId2, uint32_t atomId3) {
    if (atomId1 == 0 || atomId2 == 0 || atomId3 == 0) {
        return;
    }
    if (atomId1 == atomId2 || atomId2 == atomId3 || atomId1 == atomId3) {
        return;
    }

    const atoms::domain::AtomInfo* atom1 = findAtomById(atomId1);
    const atoms::domain::AtomInfo* atom2 = findAtomById(atomId2);
    const atoms::domain::AtomInfo* atom3 = findAtomById(atomId3);
    if (!atom1 || !atom2 || !atom3) {
        return;
    }
    if (atom1->structureId < 0 ||
        atom1->structureId != atom2->structureId ||
        atom1->structureId != atom3->structureId) {
        return;
    }

    double angleDeg = 0.0;
    std::array<double, 3> arcStart {};
    std::array<double, 3> arcEnd {};
    std::array<double, 3> arcCenter {};
    std::array<double, 3> textPos {};
    if (!computeAngleMeasurementGeometry(
            atom1->position,
            atom2->position,
            atom3->position,
            static_cast<double>(m_AngleStyle.arcRadiusScale),
            angleDeg,
            arcStart,
            arcEnd,
            arcCenter,
            textPos)) {
        return;
    }

    AngleMeasurement measurement;
    measurement.id = m_NextMeasurementId++;
    measurement.structureId = atom1->structureId;
    measurement.atomId1 = atomId1;
    measurement.atomId2 = atomId2;
    measurement.atomId3 = atomId3;
    measurement.angleDeg = angleDeg;
    measurement.displayName = "Angle (" +
        buildMeasurementAtomLabel(atom1) + "-" +
        buildMeasurementAtomLabel(atom2) + "-" +
        buildMeasurementAtomLabel(atom3) + ")";
    measurement.visible = true;
    measurement.lineActor12 = createMeasurementLineActor(atom1->position, atom2->position);
    measurement.lineActor23 = createMeasurementLineActor(atom2->position, atom3->position);
    measurement.arcActor = createAngleArcActor(arcStart, arcEnd, arcCenter);
    measurement.textActor = createMeasurementValueTextActor(formatAngleText(measurement.angleDeg), textPos);
    applyAngleStyleToMeasurement(measurement);

    if (measurement.lineActor12) {
        render::application::GetRenderGateway().AddMeasurementOverlayActor(measurement.lineActor12, false);
    }
    if (measurement.lineActor23) {
        render::application::GetRenderGateway().AddMeasurementOverlayActor(measurement.lineActor23, false);
    }
    if (measurement.arcActor) {
        render::application::GetRenderGateway().AddMeasurementOverlayActor(measurement.arcActor, false);
    }
    if (measurement.textActor) {
        render::application::GetRenderGateway().AddActor2D(measurement.textActor, false, true);
    }
    applyAngleMeasurementVisibility(measurement);
    m_AngleMeasurements.push_back(std::move(measurement));
}

void WorkspaceRuntimeModel::createDihedralMeasurement(
    uint32_t atomId1,
    uint32_t atomId2,
    uint32_t atomId3,
    uint32_t atomId4) {
    if (atomId1 == 0 || atomId2 == 0 || atomId3 == 0 || atomId4 == 0) {
        return;
    }
    if (atomId1 == atomId2 || atomId1 == atomId3 || atomId1 == atomId4 ||
        atomId2 == atomId3 || atomId2 == atomId4 || atomId3 == atomId4) {
        return;
    }

    const atoms::domain::AtomInfo* atom1 = findAtomById(atomId1);
    const atoms::domain::AtomInfo* atom2 = findAtomById(atomId2);
    const atoms::domain::AtomInfo* atom3 = findAtomById(atomId3);
    const atoms::domain::AtomInfo* atom4 = findAtomById(atomId4);
    if (!atom1 || !atom2 || !atom3 || !atom4) {
        return;
    }
    if (atom1->structureId < 0 ||
        atom1->structureId != atom2->structureId ||
        atom1->structureId != atom3->structureId ||
        atom1->structureId != atom4->structureId) {
        return;
    }

    DihedralGeometry geometry;
    if (!computeDihedralMeasurementGeometryLocal(
        atom1->position,
        atom2->position,
        atom3->position,
            atom4->position,
            geometry)) {
        return;
    }

    DihedralMeasurement measurement;
    measurement.id = m_NextMeasurementId++;
    measurement.structureId = atom1->structureId;
    measurement.atomId1 = atomId1;
    measurement.atomId2 = atomId2;
    measurement.atomId3 = atomId3;
    measurement.atomId4 = atomId4;
    measurement.dihedralDeg = geometry.dihedralDeg;
    measurement.displayName = "Dihedral (" +
        buildMeasurementAtomLabel(atom1) + "-" +
        buildMeasurementAtomLabel(atom2) + "-" +
        buildMeasurementAtomLabel(atom3) + "-" +
        buildMeasurementAtomLabel(atom4) + ")";
    measurement.visible = true;
    measurement.lineActor12 = createMeasurementLineActor(atom1->position, atom2->position);
    measurement.lineActor23 = createMeasurementLineActor(atom2->position, atom3->position);
    measurement.lineActor34 = createMeasurementLineActor(atom3->position, atom4->position);
    measurement.helperPlaneActor1 = createDihedralHelperPlaneActor(geometry.helperPlaneQuadA);
    measurement.helperPlaneActor2 = createDihedralHelperPlaneActor(geometry.helperPlaneQuadB);
    measurement.helperArcActor = createDihedralHelperArcActor(geometry.helperArcPoints);
    measurement.textActor = createMeasurementValueTextActor(
        formatDihedralText(measurement.dihedralDeg),
        geometry.textPos);
    applyDihedralStyleToMeasurement(measurement);

    if (measurement.lineActor12) {
        render::application::GetRenderGateway().AddMeasurementOverlayActor(measurement.lineActor12, false);
    }
    if (measurement.lineActor23) {
        render::application::GetRenderGateway().AddMeasurementOverlayActor(measurement.lineActor23, false);
    }
    if (measurement.lineActor34) {
        render::application::GetRenderGateway().AddMeasurementOverlayActor(measurement.lineActor34, false);
    }
    if (measurement.helperPlaneActor1) {
        render::application::GetRenderGateway().AddActor(measurement.helperPlaneActor1, false);
    }
    if (measurement.helperPlaneActor2) {
        render::application::GetRenderGateway().AddActor(measurement.helperPlaneActor2, false);
    }
    if (measurement.helperArcActor) {
        render::application::GetRenderGateway().AddMeasurementOverlayActor(measurement.helperArcActor, false);
    }
    if (measurement.textActor) {
        render::application::GetRenderGateway().AddActor2D(measurement.textActor, false, true);
    }

    applyDihedralMeasurementVisibility(measurement);
    m_DihedralMeasurements.push_back(std::move(measurement));
}

void WorkspaceRuntimeModel::applyDistanceMeasurementVisibility(DistanceMeasurement& measurement) {
    const bool effectiveVisible = !m_MeasurementVisibilitySuppressedByBZ &&
        measurement.visible &&
        IsStructureVisible(measurement.structureId);
    if (measurement.lineActor) {
        measurement.lineActor->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.textActor) {
        measurement.textActor->SetVisibility(effectiveVisible ? 1 : 0);
    }
}

void WorkspaceRuntimeModel::applyAngleMeasurementVisibility(AngleMeasurement& measurement) {
    const bool effectiveVisible = !m_MeasurementVisibilitySuppressedByBZ &&
        measurement.visible &&
        IsStructureVisible(measurement.structureId);
    if (measurement.lineActor12) {
        measurement.lineActor12->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.lineActor23) {
        measurement.lineActor23->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.arcActor) {
        measurement.arcActor->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.textActor) {
        measurement.textActor->SetVisibility(effectiveVisible ? 1 : 0);
    }
}

void WorkspaceRuntimeModel::applyDihedralMeasurementVisibility(DihedralMeasurement& measurement) {
    const bool effectiveVisible = !m_MeasurementVisibilitySuppressedByBZ &&
        measurement.visible &&
        IsStructureVisible(measurement.structureId);
    if (measurement.lineActor12) {
        measurement.lineActor12->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.lineActor23) {
        measurement.lineActor23->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.lineActor34) {
        measurement.lineActor34->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.helperPlaneActor1) {
        measurement.helperPlaneActor1->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.helperPlaneActor2) {
        measurement.helperPlaneActor2->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.helperArcActor) {
        measurement.helperArcActor->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.textActor) {
        measurement.textActor->SetVisibility(effectiveVisible ? 1 : 0);
    }
}

void WorkspaceRuntimeModel::applyCenterMeasurementVisibility(CenterMeasurement& measurement) {
    const bool effectiveVisible = !m_MeasurementVisibilitySuppressedByBZ &&
        measurement.visible &&
        IsStructureVisible(measurement.structureId);
    if (measurement.centerBackgroundCircleActor) {
        measurement.centerBackgroundCircleActor->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.centerPlusTextActor) {
        measurement.centerPlusTextActor->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (measurement.centerCoordinateTextActor) {
        measurement.centerCoordinateTextActor->SetVisibility(effectiveVisible ? 1 : 0);
    }
    for (auto& actor : measurement.selectedAtomBackgroundCircleActors) {
        if (actor) {
            actor->SetVisibility(effectiveVisible ? 1 : 0);
        }
    }
    for (auto& actor : measurement.selectedAtomPlusTextActors) {
        if (actor) {
            actor->SetVisibility(effectiveVisible ? 1 : 0);
        }
    }
}

void WorkspaceRuntimeModel::setMeasurementVisibilitySuppressedByBZ(bool suppressed) {
    if (m_MeasurementVisibilitySuppressedByBZ == suppressed) {
        return;
    }
    m_MeasurementVisibilitySuppressedByBZ = suppressed;
    for (auto& measurement : m_DistanceMeasurements) {
        applyDistanceMeasurementVisibility(measurement);
    }
    for (auto& measurement : m_AngleMeasurements) {
        applyAngleMeasurementVisibility(measurement);
    }
    for (auto& measurement : m_DihedralMeasurements) {
        applyDihedralMeasurementVisibility(measurement);
    }
    for (auto& measurement : m_CenterMeasurements) {
        applyCenterMeasurementVisibility(measurement);
    }
    syncMeasurementPickVisuals();
    requestViewerRender();
}

void WorkspaceRuntimeModel::removeDistanceMeasurementActors(DistanceMeasurement& measurement) {
    if (measurement.lineActor) {
        render::application::GetRenderGateway().RemoveMeasurementOverlayActor(measurement.lineActor);
        measurement.lineActor = nullptr;
    }
    if (measurement.textActor) {
        render::application::GetRenderGateway().RemoveActor2D(measurement.textActor);
        measurement.textActor = nullptr;
    }
}

void WorkspaceRuntimeModel::removeAngleMeasurementActors(AngleMeasurement& measurement) {
    if (measurement.lineActor12) {
        render::application::GetRenderGateway().RemoveMeasurementOverlayActor(measurement.lineActor12);
        measurement.lineActor12 = nullptr;
    }
    if (measurement.lineActor23) {
        render::application::GetRenderGateway().RemoveMeasurementOverlayActor(measurement.lineActor23);
        measurement.lineActor23 = nullptr;
    }
    if (measurement.arcActor) {
        render::application::GetRenderGateway().RemoveMeasurementOverlayActor(measurement.arcActor);
        measurement.arcActor = nullptr;
    }
    if (measurement.textActor) {
        render::application::GetRenderGateway().RemoveActor2D(measurement.textActor);
        measurement.textActor = nullptr;
    }
}

void WorkspaceRuntimeModel::removeDihedralMeasurementActors(DihedralMeasurement& measurement) {
    if (measurement.lineActor12) {
        render::application::GetRenderGateway().RemoveMeasurementOverlayActor(measurement.lineActor12);
        measurement.lineActor12 = nullptr;
    }
    if (measurement.lineActor23) {
        render::application::GetRenderGateway().RemoveMeasurementOverlayActor(measurement.lineActor23);
        measurement.lineActor23 = nullptr;
    }
    if (measurement.lineActor34) {
        render::application::GetRenderGateway().RemoveMeasurementOverlayActor(measurement.lineActor34);
        measurement.lineActor34 = nullptr;
    }
    if (measurement.helperPlaneActor1) {
        render::application::GetRenderGateway().RemoveActor(measurement.helperPlaneActor1);
        measurement.helperPlaneActor1 = nullptr;
    }
    if (measurement.helperPlaneActor2) {
        render::application::GetRenderGateway().RemoveActor(measurement.helperPlaneActor2);
        measurement.helperPlaneActor2 = nullptr;
    }
    if (measurement.helperArcActor) {
        render::application::GetRenderGateway().RemoveMeasurementOverlayActor(measurement.helperArcActor);
        measurement.helperArcActor = nullptr;
    }
    if (measurement.textActor) {
        render::application::GetRenderGateway().RemoveActor2D(measurement.textActor);
        measurement.textActor = nullptr;
    }
}

void WorkspaceRuntimeModel::removeCenterMeasurementActors(CenterMeasurement& measurement) {
    if (measurement.centerBackgroundCircleActor) {
        render::application::GetRenderGateway().RemoveActor2D(measurement.centerBackgroundCircleActor);
        measurement.centerBackgroundCircleActor = nullptr;
    }
    if (measurement.centerPlusTextActor) {
        render::application::GetRenderGateway().RemoveActor2D(measurement.centerPlusTextActor);
        measurement.centerPlusTextActor = nullptr;
    }
    if (measurement.centerCoordinateTextActor) {
        render::application::GetRenderGateway().RemoveActor2D(measurement.centerCoordinateTextActor);
        measurement.centerCoordinateTextActor = nullptr;
    }
    for (auto& actor : measurement.selectedAtomBackgroundCircleActors) {
        if (!actor) {
            continue;
        }
        render::application::GetRenderGateway().RemoveActor2D(actor);
        actor = nullptr;
    }
    measurement.selectedAtomBackgroundCircleActors.clear();
    for (auto& actor : measurement.selectedAtomPlusTextActors) {
        if (!actor) {
            continue;
        }
        render::application::GetRenderGateway().RemoveActor2D(actor);
        actor = nullptr;
    }
    measurement.selectedAtomPlusTextActors.clear();
    measurement.selectedAtomPlusBaseFontSizes.clear();
}

void WorkspaceRuntimeModel::clearAllDistanceMeasurements() {
    for (auto& measurement : m_DistanceMeasurements) {
        removeDistanceMeasurementActors(measurement);
    }
    m_DistanceMeasurements.clear();
    m_MeasurementPickedAtomIds.clear();
    clearMeasurementPickVisuals();
    if (m_AngleMeasurements.empty() &&
        m_DihedralMeasurements.empty() &&
        m_CenterMeasurements.empty()) {
        m_NextMeasurementId = 1;
        m_NextCenterMarkerStyleIndex = 0;
    }
}

void WorkspaceRuntimeModel::clearAllAngleMeasurements() {
    for (auto& measurement : m_AngleMeasurements) {
        removeAngleMeasurementActors(measurement);
    }
    m_AngleMeasurements.clear();
    m_MeasurementPickedAtomIds.clear();
    clearMeasurementPickVisuals();
    if (m_DistanceMeasurements.empty() &&
        m_DihedralMeasurements.empty() &&
        m_CenterMeasurements.empty()) {
        m_NextMeasurementId = 1;
        m_NextCenterMarkerStyleIndex = 0;
    }
}

void WorkspaceRuntimeModel::clearAllDihedralMeasurements() {
    for (auto& measurement : m_DihedralMeasurements) {
        removeDihedralMeasurementActors(measurement);
    }
    m_DihedralMeasurements.clear();
    m_MeasurementPickedAtomIds.clear();
    clearMeasurementPickVisuals();
    if (m_DistanceMeasurements.empty() &&
        m_AngleMeasurements.empty() &&
        m_CenterMeasurements.empty()) {
        m_NextMeasurementId = 1;
        m_NextCenterMarkerStyleIndex = 0;
    }
}

void WorkspaceRuntimeModel::clearAllCenterMeasurements() {
    for (auto& measurement : m_CenterMeasurements) {
        removeCenterMeasurementActors(measurement);
    }
    m_CenterMeasurements.clear();
    m_MeasurementPickedAtomIds.clear();
    clearMeasurementPickVisuals();
    if (m_DistanceMeasurements.empty() &&
        m_AngleMeasurements.empty() &&
        m_DihedralMeasurements.empty()) {
        m_NextMeasurementId = 1;
        m_NextCenterMarkerStyleIndex = 0;
    }
}

void WorkspaceRuntimeModel::removeDistanceMeasurementsByStructure(int32_t structureId) {
    if (structureId < 0) {
        return;
    }
    m_DistanceMeasurements.erase(
        std::remove_if(
            m_DistanceMeasurements.begin(),
            m_DistanceMeasurements.end(),
            [&](DistanceMeasurement& measurement) {
                if (measurement.structureId != structureId) {
                    return false;
                }
                removeDistanceMeasurementActors(measurement);
                return true;
            }),
        m_DistanceMeasurements.end());
}

void WorkspaceRuntimeModel::removeAngleMeasurementsByStructure(int32_t structureId) {
    if (structureId < 0) {
        return;
    }
    m_AngleMeasurements.erase(
        std::remove_if(
            m_AngleMeasurements.begin(),
            m_AngleMeasurements.end(),
            [&](AngleMeasurement& measurement) {
                if (measurement.structureId != structureId) {
                    return false;
                }
                removeAngleMeasurementActors(measurement);
                return true;
            }),
        m_AngleMeasurements.end());
}

void WorkspaceRuntimeModel::removeDihedralMeasurementsByStructure(int32_t structureId) {
    if (structureId < 0) {
        return;
    }
    m_DihedralMeasurements.erase(
        std::remove_if(
            m_DihedralMeasurements.begin(),
            m_DihedralMeasurements.end(),
            [&](DihedralMeasurement& measurement) {
                if (measurement.structureId != structureId) {
                    return false;
                }
                removeDihedralMeasurementActors(measurement);
                return true;
            }),
        m_DihedralMeasurements.end());
}

void WorkspaceRuntimeModel::removeCenterMeasurementsByStructure(int32_t structureId) {
    if (structureId < 0) {
        return;
    }
    m_CenterMeasurements.erase(
        std::remove_if(
            m_CenterMeasurements.begin(),
            m_CenterMeasurements.end(),
            [&](CenterMeasurement& measurement) {
                if (measurement.structureId != structureId) {
                    return false;
                }
                removeCenterMeasurementActors(measurement);
                return true;
            }),
        m_CenterMeasurements.end());
}

void WorkspaceRuntimeModel::pruneInvalidDistanceMeasurements() {
    if (!m_MeasurementPickedAtomIds.empty()) {
        bool allPickedValid = true;
        for (uint32_t pickedAtomId : m_MeasurementPickedAtomIds) {
            const atoms::domain::AtomInfo* atom = findAtomById(pickedAtomId);
            if (!atom || atom->structureId < 0) {
                allPickedValid = false;
                break;
            }
        }
        if (!allPickedValid) {
            m_MeasurementPickedAtomIds.clear();
        }
    }

    m_DistanceMeasurements.erase(
        std::remove_if(
            m_DistanceMeasurements.begin(),
            m_DistanceMeasurements.end(),
            [&](DistanceMeasurement& measurement) {
                const atoms::domain::AtomInfo* atom1 = findAtomById(measurement.atomId1);
                const atoms::domain::AtomInfo* atom2 = findAtomById(measurement.atomId2);
                const bool invalid = (!atom1 || !atom2 ||
                                      measurement.structureId < 0 ||
                                      atom1->structureId != measurement.structureId ||
                                      atom2->structureId != measurement.structureId);
                if (invalid) {
                    removeDistanceMeasurementActors(measurement);
                }
                return invalid;
            }),
        m_DistanceMeasurements.end());
}

void WorkspaceRuntimeModel::pruneInvalidAngleMeasurements() {
    m_AngleMeasurements.erase(
        std::remove_if(
            m_AngleMeasurements.begin(),
            m_AngleMeasurements.end(),
            [&](AngleMeasurement& measurement) {
                const atoms::domain::AtomInfo* atom1 = findAtomById(measurement.atomId1);
                const atoms::domain::AtomInfo* atom2 = findAtomById(measurement.atomId2);
                const atoms::domain::AtomInfo* atom3 = findAtomById(measurement.atomId3);
                const bool invalid = (!atom1 || !atom2 || !atom3 ||
                                      measurement.structureId < 0 ||
                                      atom1->structureId != measurement.structureId ||
                                      atom2->structureId != measurement.structureId ||
                                      atom3->structureId != measurement.structureId);
                if (invalid) {
                    removeAngleMeasurementActors(measurement);
                }
                return invalid;
            }),
        m_AngleMeasurements.end());
}

void WorkspaceRuntimeModel::pruneInvalidDihedralMeasurements() {
    m_DihedralMeasurements.erase(
        std::remove_if(
            m_DihedralMeasurements.begin(),
            m_DihedralMeasurements.end(),
            [&](DihedralMeasurement& measurement) {
                const atoms::domain::AtomInfo* atom1 = findAtomById(measurement.atomId1);
                const atoms::domain::AtomInfo* atom2 = findAtomById(measurement.atomId2);
                const atoms::domain::AtomInfo* atom3 = findAtomById(measurement.atomId3);
                const atoms::domain::AtomInfo* atom4 = findAtomById(measurement.atomId4);
                const bool invalid = (!atom1 || !atom2 || !atom3 || !atom4 ||
                                      measurement.structureId < 0 ||
                                      atom1->structureId != measurement.structureId ||
                                      atom2->structureId != measurement.structureId ||
                                      atom3->structureId != measurement.structureId ||
                                      atom4->structureId != measurement.structureId);
                if (invalid) {
                    removeDihedralMeasurementActors(measurement);
                }
                return invalid;
            }),
        m_DihedralMeasurements.end());
}

void WorkspaceRuntimeModel::pruneInvalidCenterMeasurements() {
    m_CenterMeasurements.erase(
        std::remove_if(
            m_CenterMeasurements.begin(),
            m_CenterMeasurements.end(),
            [&](CenterMeasurement& measurement) {
                if (measurement.structureId < 0 || measurement.atomIds.size() < 2) {
                    removeCenterMeasurementActors(measurement);
                    return true;
                }

                for (uint32_t atomId : measurement.atomIds) {
                    const atoms::domain::AtomInfo* atom = findAtomById(atomId);
                    if (!atom || atom->structureId != measurement.structureId) {
                        removeCenterMeasurementActors(measurement);
                        return true;
                    }
                }
                return false;
            }),
        m_CenterMeasurements.end());
}

std::vector<WorkspaceRuntimeModel::MeasurementListItem>
WorkspaceRuntimeModel::GetMeasurementsForStructure(int32_t structureId) {
    pruneInvalidDistanceMeasurements();
    pruneInvalidAngleMeasurements();
    pruneInvalidDihedralMeasurements();
    pruneInvalidCenterMeasurements();

    std::vector<MeasurementListItem> items;
    items.reserve(
        m_DistanceMeasurements.size() +
        m_AngleMeasurements.size() +
        m_DihedralMeasurements.size() +
        m_CenterMeasurements.size());
    for (const auto& measurement : m_DistanceMeasurements) {
        if (measurement.structureId != structureId) {
            continue;
        }
        MeasurementListItem item;
        item.id = measurement.id;
        item.type = MeasurementType::Distance;
        item.displayName = measurement.displayName;
        item.visible = measurement.visible;
        items.push_back(std::move(item));
    }
    for (const auto& measurement : m_AngleMeasurements) {
        if (measurement.structureId != structureId) {
            continue;
        }
        MeasurementListItem item;
        item.id = measurement.id;
        item.type = MeasurementType::Angle;
        item.displayName = measurement.displayName;
        item.visible = measurement.visible;
        items.push_back(std::move(item));
    }
    for (const auto& measurement : m_DihedralMeasurements) {
        if (measurement.structureId != structureId) {
            continue;
        }
        MeasurementListItem item;
        item.id = measurement.id;
        item.type = MeasurementType::Dihedral;
        item.displayName = measurement.displayName;
        item.visible = measurement.visible;
        items.push_back(std::move(item));
    }
    for (const auto& measurement : m_CenterMeasurements) {
        if (measurement.structureId != structureId) {
            continue;
        }
        MeasurementListItem item;
        item.id = measurement.id;
        item.type = measurement.type;
        item.displayName = measurement.displayName;
        item.visible = measurement.visible;
        items.push_back(std::move(item));
    }
    std::sort(items.begin(), items.end(), [](const MeasurementListItem& lhs, const MeasurementListItem& rhs) {
        return lhs.id < rhs.id;
    });
    return items;
}

void WorkspaceRuntimeModel::SetMeasurementVisible(uint32_t measurementId, bool visible) {
    for (auto& measurement : m_DistanceMeasurements) {
        if (measurement.id != measurementId) {
            continue;
        }
        measurement.visible = visible;
        applyDistanceMeasurementVisibility(measurement);
        requestViewerRender();
        return;
    }
    for (auto& measurement : m_AngleMeasurements) {
        if (measurement.id != measurementId) {
            continue;
        }
        measurement.visible = visible;
        applyAngleMeasurementVisibility(measurement);
        requestViewerRender();
        return;
    }
    for (auto& measurement : m_DihedralMeasurements) {
        if (measurement.id != measurementId) {
            continue;
        }
        measurement.visible = visible;
        applyDihedralMeasurementVisibility(measurement);
        requestViewerRender();
        return;
    }
    for (auto& measurement : m_CenterMeasurements) {
        if (measurement.id != measurementId) {
            continue;
        }
        measurement.visible = visible;
        applyCenterMeasurementVisibility(measurement);
        requestViewerRender();
        return;
    }
}

void WorkspaceRuntimeModel::RemoveMeasurement(uint32_t measurementId) {
    auto distanceIt = std::find_if(
        m_DistanceMeasurements.begin(),
        m_DistanceMeasurements.end(),
        [&](const DistanceMeasurement& measurement) {
            return measurement.id == measurementId;
        });
    if (distanceIt != m_DistanceMeasurements.end()) {
        removeDistanceMeasurementActors(*distanceIt);
        m_DistanceMeasurements.erase(distanceIt);
        requestViewerRender();
        return;
    }

    auto angleIt = std::find_if(
        m_AngleMeasurements.begin(),
        m_AngleMeasurements.end(),
        [&](const AngleMeasurement& measurement) {
            return measurement.id == measurementId;
        });
    if (angleIt == m_AngleMeasurements.end()) {
        auto dihedralIt = std::find_if(
            m_DihedralMeasurements.begin(),
            m_DihedralMeasurements.end(),
            [&](const DihedralMeasurement& measurement) {
                return measurement.id == measurementId;
            });
        if (dihedralIt != m_DihedralMeasurements.end()) {
            removeDihedralMeasurementActors(*dihedralIt);
            m_DihedralMeasurements.erase(dihedralIt);
            requestViewerRender();
            return;
        }

        auto centerIt = std::find_if(
            m_CenterMeasurements.begin(),
            m_CenterMeasurements.end(),
            [&](const CenterMeasurement& measurement) {
                return measurement.id == measurementId;
            });
        if (centerIt == m_CenterMeasurements.end()) {
            return;
        }
        removeCenterMeasurementActors(*centerIt);
        m_CenterMeasurements.erase(centerIt);
        requestViewerRender();
        return;
    }
    removeAngleMeasurementActors(*angleIt);
    m_AngleMeasurements.erase(angleIt);
    requestViewerRender();
}

void WorkspaceRuntimeModel::RemoveMeasurementsByStructure(int32_t structureId) {
    if (structureId < 0) {
        return;
    }

    removeDihedralMeasurementsByStructure(structureId);
    removeAngleMeasurementsByStructure(structureId);
    removeDistanceMeasurementsByStructure(structureId);
    removeCenterMeasurementsByStructure(structureId);
    if (m_DistanceMeasurements.empty() &&
        m_AngleMeasurements.empty() &&
        m_DihedralMeasurements.empty() &&
        m_CenterMeasurements.empty()) {
        m_NextMeasurementId = 1;
        m_NextCenterMarkerStyleIndex = 0;
    }
    requestViewerRender();
}

std::vector<WorkspaceRuntimeModel::DistanceMeasurementListItem>
WorkspaceRuntimeModel::GetDistanceMeasurementsForStructure(int32_t structureId) {
    pruneInvalidDistanceMeasurements();

    std::vector<DistanceMeasurementListItem> items;
    for (const auto& measurement : m_DistanceMeasurements) {
        if (measurement.structureId != structureId) {
            continue;
        }
        DistanceMeasurementListItem item;
        item.id = measurement.id;
        item.displayName = measurement.displayName;
        item.visible = measurement.visible;
        items.push_back(std::move(item));
    }
    return items;
}

void WorkspaceRuntimeModel::SetDistanceMeasurementVisible(uint32_t measurementId, bool visible) {
    SetMeasurementVisible(measurementId, visible);
}

void WorkspaceRuntimeModel::RemoveDistanceMeasurement(uint32_t measurementId) {
    RemoveMeasurement(measurementId);
}


bool WorkspaceRuntimeModel::LoadChgcarFile(const std::string& filePath) {
    SPDLOG_INFO("Loading CHGCAR file: {}", filePath);

    auto result = atoms::infrastructure::ChgcarParser::parse(filePath);
    return LoadChgcarParsedData(result);
}

bool WorkspaceRuntimeModel::LoadChgcarParsedData(const atoms::infrastructure::ChgcarParser::ParseResult& result) {
    if (!result.success) {
        SPDLOG_ERROR("Failed to parse CHGCAR: {}", result.errorMessage);
        return false;
    }

    // 기존 구조 초기화
    BatchGuard guard = createBatchGuard();


    // ??2.5. cellInfo ?ㅼ젙 異붽?
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cellInfo.matrix[i][j] = result.lattice[i][j];
        }
    }
    cellInfo.modified = true;
    cellInfo.hasCell = true;
    atoms::domain::calculateInverseMatrix(cellInfo.matrix, cellInfo.invmatrix);
    atoms::domain::setCellMatrix(result.lattice);  // ??domain?먮룄 ?ㅼ젙
 
    
    // 3. Unit Cell ?앹꽦
    createUnitCell(result.lattice);
    
    // 4. ?먯옄 ?앹꽦
    int atomIndex = 0;
    std::vector<uint32_t> newAtomIds;
    for (size_t elemIdx = 0; elemIdx < result.elements.size(); ++elemIdx) {
        const std::string& symbol = result.elements[elemIdx];
        int count = result.atomCounts[elemIdx];
        
        // ElementDatabase에서 정보 가져오기
        float radius = 1.0f;
        atoms::domain::Color4f color = {0.5f, 0.5f, 0.5f, 1.0f};
        
        if (m_elementDB) {
            const auto* elemInfo = m_elementDB->getElementInfo(symbol);
            if (elemInfo) {
                radius = elemInfo->covalentRadius;
                color = elemInfo->defaultColor;
            } else {
                // ElementDatabase?먯꽌 紐?李얠쑝硫?湲곕낯媛?硫붿꽌???ъ슜
                radius = m_elementDB->getDefaultRadius(symbol);
                color = m_elementDB->getDefaultColor(symbol);
            }
        }
        
        for (int i = 0; i < count; ++i) {
            const auto& pos = result.positions[atomIndex];
            
            float cartPos[3];
            if (result.isDirect) {
                // Direct(분수) 좌표 -> Cartesian 변환
                atoms::domain::fractionalToCartesian(pos.data(), cartPos, result.lattice);
            } else {
                cartPos[0] = pos[0];
                cartPos[1] = pos[1];
                cartPos[2] = pos[2];
            }
            
            createAtomSphere(symbol.c_str(), color, radius, cartPos, 
                            atoms::domain::AtomType::ORIGINAL);
            if (!createdAtoms.empty()) {
                newAtomIds.push_back(createdAtoms.back().id);
            }
            
            atomIndex++;
        }
    }
    
    // 5. 寃고빀 ?앹꽦 (?좉퇋 ?먯옄留?
    if (!newAtomIds.empty()) {
        createBondsForAtoms(newAtomIds, true, false, false);
    }
    
    // 6. Unit Cell ?쒖떆
    setCellVisible(true);
    
    // 7. ?꾪븯 諛??濡쒕뱶
    if (m_chargeDensityUI) {
        if (m_chargeDensityUI->loadFromParseResult(result)) {
            SetChargeDensityStructureId(m_CurrentStructureId);
            const bool structureVisible = IsStructureVisible(m_CurrentStructureId);
            m_chargeDensityUI->setStructureVisible(structureVisible);
        }
    }

    if (!result.density.empty()) {
        if (m_ChargeDensityVolumeMeshId >= 0) {
            mesh::application::GetMeshCommandService().DeleteMesh(m_ChargeDensityVolumeMeshId);
            m_ChargeDensityVolumeMeshId = -1;
        }

        vtkSmartPointer<vtkDataSet> surfaceDataSet;
        vtkSmartPointer<vtkDataSet> mediumSurfaceDataSet;
        vtkSmartPointer<vtkDataSet> lowSurfaceDataSet;
        vtkSmartPointer<vtkDataSet> volumeDataSet;
        vtkSmartPointer<vtkDataSet> mediumVolumeDataSet;
        vtkSmartPointer<vtkDataSet> lowVolumeDataSet;

        if (createChgcarDataSets(result, surfaceDataSet, volumeDataSet)) {
            const auto mediumResult = createDownsampledChgcarResult(result, 2);
            if (!isSameGridShape(result, mediumResult)) {
                createChgcarDataSets(mediumResult, mediumSurfaceDataSet, mediumVolumeDataSet);
            }

            const auto lowResult = createDownsampledChgcarResult(result, 4);
            if (!isSameGridShape(result, lowResult)) {
                createChgcarDataSets(lowResult, lowSurfaceDataSet, lowVolumeDataSet);
            }
        }

        if (surfaceDataSet) {
            auto& meshCommandService = mesh::application::GetMeshCommandService();
            int32_t parentId = m_CurrentStructureId >= 0 ? m_CurrentStructureId : 0;
            Mesh* mesh = meshCommandService.InsertMesh("noname_01", nullptr, nullptr, surfaceDataSet, parentId);
            if (mesh) {
                mesh->SetVolumeQualityDataSets(surfaceDataSet, mediumSurfaceDataSet, lowSurfaceDataSet,
                                               volumeDataSet, mediumVolumeDataSet, lowVolumeDataSet);
                mesh->SetVolumeMeshVisibility(IsStructureVisible(parentId));
                m_ChargeDensityVolumeMeshId = mesh->GetId();
            }
        }
    }

    applyChargeDensityViewType();

    SPDLOG_INFO("CHGCAR loaded: {} atoms, {}x{}x{} grid",
                atomIndex, result.gridShape[0], result.gridShape[1], result.gridShape[2]);

    return true;
}

void WorkspaceRuntimeModel::ClearChargeDensity() {
    if (m_chargeDensityUI) {
        m_chargeDensityUI->clear();
    }
    m_ActiveChargeDensityGridMeshId = -1;
    if (m_ChargeDensityVolumeMeshId >= 0) {
        mesh::application::GetMeshCommandService().DeleteMesh(m_ChargeDensityVolumeMeshId);
        m_ChargeDensityVolumeMeshId = -1;
    }
    m_ChargeDensityStructureId = -1;
}

bool WorkspaceRuntimeModel::HasChargeDensity() const {
    return m_chargeDensityUI && m_chargeDensityUI->hasData();
}

bool WorkspaceRuntimeModel::IsChargeDensityVisible() const {
    if (!m_chargeDensityUI) return false;
    return m_chargeDensityUI->isAnyIsosurfaceVisible();
}

void WorkspaceRuntimeModel::SetChargeDensityVisible(bool visible) {
    if (m_chargeDensityUI) {
        if (visible) {
            m_chargeDensityUI->setAllSimpleGridVisible(true);
            m_chargeDensityUI->setIsosurfaceVisible(true);
            m_chargeDensityUI->setMultipleIsosurfacesVisible(false);
        } else {
            m_chargeDensityUI->setAllSimpleGridVisible(false);
            m_chargeDensityUI->setIsosurfaceVisible(false);
            m_chargeDensityUI->setMultipleIsosurfacesVisible(false);
        }
    }
}

bool WorkspaceRuntimeModel::IsChargeDensitySimpleViewActive() const {
    return m_ChargeDensityViewType == ChargeDensityViewType::Simple;
}

void WorkspaceRuntimeModel::SyncChargeDensityViewTypeState() {
    applyChargeDensityViewType();
}

void WorkspaceRuntimeModel::SetChargeDensityStructureId(int32_t id) {
    if (id < 0) {
        m_ChargeDensityStructureId = -1;
        return;
    }
    m_ChargeDensityStructureId = id;
    if (m_chargeDensityUI) {
        m_chargeDensityUI->setStructureVisible(IsStructureVisible(id));
    }
    applyChargeDensityViewType();
}

bool WorkspaceRuntimeModel::IsStructureVisible(int32_t id) const {
    if (id < 0) {
        return m_StructureVisible;
    }
    auto it = m_Structures.find(id);
    if (it == m_Structures.end()) {
        return m_StructureVisible;
    }
    return it->second.visible;
}

std::vector<WorkspaceRuntimeModel::StructureEntry> WorkspaceRuntimeModel::GetStructures() const {
    std::vector<StructureEntry> structures;
    structures.reserve(m_Structures.size());
    for (const auto& [id, entry] : m_Structures) {
        structures.push_back(entry);
    }
    std::sort(structures.begin(), structures.end(), [](const StructureEntry& a, const StructureEntry& b) {
        return a.id < b.id;
    });
    return structures;
}

size_t WorkspaceRuntimeModel::GetAtomCountForStructure(int32_t id) const {
    size_t count = 0;
    for (const auto& atom : createdAtoms) {
        if (atom.structureId == id) {
            ++count;
        }
    }
    return count;
}

bool WorkspaceRuntimeModel::HasUnitCell(int32_t id) const {
    auto it = m_UnitCells.find(id);
    if (it == m_UnitCells.end()) {
        return false;
    }
    return it->second.hasCell;
}

bool WorkspaceRuntimeModel::IsUnitCellVisible(int32_t id) const {
    auto it = m_UnitCells.find(id);
    if (it == m_UnitCells.end()) {
        return false;
    }
    return it->second.visible;
}

void WorkspaceRuntimeModel::refreshRenderedGroups() {
    pruneInvalidCenterMeasurements();
    pruneInvalidDihedralMeasurements();
    pruneInvalidAngleMeasurements();
    pruneInvalidDistanceMeasurements();
    for (const auto& [symbol, group] : atomGroups) {
        (void)group;
        updateUnifiedAtomGroupVTK(symbol);
    }
    for (const auto& [key, group] : bondGroups) {
        (void)group;
        updateBondGroupVTK(key);
    }
    refreshLabelActors();
    for (auto& measurement : m_DistanceMeasurements) {
        applyDistanceMeasurementVisibility(measurement);
    }
    for (auto& measurement : m_AngleMeasurements) {
        applyAngleMeasurementVisibility(measurement);
    }
    for (auto& measurement : m_DihedralMeasurements) {
        applyDihedralMeasurementVisibility(measurement);
    }
    for (auto& measurement : m_CenterMeasurements) {
        applyCenterMeasurementVisibility(measurement);
    }
    syncMeasurementPickVisuals();
    syncCreatedAtomSelectionVisuals();
}

void WorkspaceRuntimeModel::refreshLabelActors() {
    if (!m_vtkRenderer) {
        return;
    }

    std::unordered_map<uint32_t, size_t> originalSerialByAtomId;
    originalSerialByAtomId.reserve(createdAtoms.size());
    for (size_t atomIndex = 0; atomIndex < createdAtoms.size(); ++atomIndex) {
        const auto& atom = createdAtoms[atomIndex];
        if (atom.id != 0) {
            originalSerialByAtomId[atom.id] = atomIndex + 1;
        }
    }

    std::vector<atoms::infrastructure::VTKRenderer::LabelActorSpec> atomLabels;
    atomLabels.reserve(createdAtoms.size() + surroundingAtoms.size());
    for (size_t atomIndex = 0; atomIndex < createdAtoms.size(); ++atomIndex) {
        const auto& atom = createdAtoms[atomIndex];
        if (atom.id == 0 || atom.structureId < 0) {
            continue;
        }
        if (!IsAtomLabelVisibleById(atom.id)) {
            continue;
        }
        if (!IsStructureVisible(atom.structureId) || !IsAtomVisibleById(atom.id)) {
            continue;
        }

        atoms::infrastructure::VTKRenderer::LabelActorSpec spec;
        spec.id = atom.id;
        spec.text = atom.symbol + "#" + std::to_string(atomIndex + 1);
        spec.worldPosition = {
            static_cast<double>(atom.position[0]),
            static_cast<double>(atom.position[1]),
            static_cast<double>(atom.position[2])
        };
        spec.visible = true;
        atomLabels.push_back(std::move(spec));
    }

    for (const auto& atom : surroundingAtoms) {
        if (atom.id == 0 || atom.structureId < 0) {
            continue;
        }
        if (!IsAtomLabelVisibleById(atom.id)) {
            continue;
        }
        if (!IsStructureVisible(atom.structureId) || !IsAtomVisibleById(atom.id)) {
            continue;
        }

        atoms::infrastructure::VTKRenderer::LabelActorSpec spec;
        spec.id = atom.id;
        auto originalSerialIt = originalSerialByAtomId.find(atom.originalAtomId);
        if (originalSerialIt != originalSerialByAtomId.end()) {
            spec.text = atom.symbol + "(#" + std::to_string(originalSerialIt->second) + ")";
        } else {
            spec.text = atom.symbol + "(#?)";
        }
        spec.worldPosition = {
            static_cast<double>(atom.position[0]),
            static_cast<double>(atom.position[1]),
            static_cast<double>(atom.position[2])
        };
        spec.visible = true;
        atomLabels.push_back(std::move(spec));
    }

    std::vector<atoms::infrastructure::VTKRenderer::LabelActorSpec> bondLabels;
    bondLabels.reserve(createdBonds.size() + surroundingBonds.size());
    auto appendBondLabel = [&](const atoms::domain::BondInfo& bond) {
        if (bond.id == 0 || bond.structureId < 0) {
            return;
        }
        if (!IsBondLabelVisibleById(bond.id)) {
            return;
        }
        if (!IsStructureVisible(bond.structureId) ||
            !IsBondsVisible(bond.structureId) ||
            !IsBondVisibleById(bond.id)) {
            return;
        }

        const atoms::domain::AtomInfo* atom1 = resolveBondAtom(bond, true);
        const atoms::domain::AtomInfo* atom2 = resolveBondAtom(bond, false);
        if (!atom1 || !atom2) {
            return;
        }

        const std::string bondTypeLabel = bond.bondGroupKey.empty() ? "Bond" : bond.bondGroupKey;

        atoms::infrastructure::VTKRenderer::LabelActorSpec spec;
        spec.id = bond.id;
        spec.text = bondTypeLabel + "#" + std::to_string(bond.id);
        spec.worldPosition = {
            (static_cast<double>(atom1->position[0]) + static_cast<double>(atom2->position[0])) * 0.5,
            (static_cast<double>(atom1->position[1]) + static_cast<double>(atom2->position[1])) * 0.5,
            (static_cast<double>(atom1->position[2]) + static_cast<double>(atom2->position[2])) * 0.5
        };
        spec.visible = true;
        bondLabels.push_back(std::move(spec));
    };

    for (const auto& bond : createdBonds) {
        appendBondLabel(bond);
    }
    for (const auto& bond : surroundingBonds) {
        appendBondLabel(bond);
    }

    m_vtkRenderer->syncAtomLabelActors(atomLabels);
    m_vtkRenderer->syncBondLabelActors(bondLabels);
}

std::string WorkspaceRuntimeModel::makeStructureKey(const std::string& baseKey, int32_t structureId) {
    return baseKey + "#" + std::to_string(structureId);
}

bool WorkspaceRuntimeModel::getAtomStructureId(uint32_t atomId, int32_t& structureId) {
    for (const auto& atom : createdAtoms) {
        if (atom.id == atomId) {
            structureId = atom.structureId;
            return true;
        }
    }
    for (const auto& atom : surroundingAtoms) {
        if (atom.id == atomId) {
            structureId = atom.structureId;
            return true;
        }
    }
    return false;
}

bool WorkspaceRuntimeModel::getBondStructureId(uint32_t bondId, int32_t& structureId) {
    for (const auto& bond : createdBonds) {
        if (bond.id == bondId) {
            structureId = bond.structureId;
            return true;
        }
    }
    for (const auto& bond : surroundingBonds) {
        if (bond.id == bondId) {
            structureId = bond.structureId;
            return true;
        }
    }
    return false;
}


















