#include "xsf_parser.h"

#include "../../config/log_config.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>

namespace io::infrastructure {

namespace {

constexpr int kMinProgressIntervalMs = 1000;
constexpr uint64_t kLineStep = 200;
constexpr uint64_t kTokenStep = 4096;
constexpr uint64_t kValueStep = 16384;

void reportProgress(
    std::istream& stream,
    std::streampos totalSize,
    float& lastProgress,
    std::chrono::steady_clock::time_point& lastProgressTime,
    const XsfParser::ProgressCallback& progressCallback,
    bool force = false) {
    if (!progressCallback || totalSize <= 0) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (!force && lastProgressTime.time_since_epoch().count() != 0) {
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastProgressTime);
        if (elapsed.count() < kMinProgressIntervalMs) {
            return;
        }
    }

    std::streampos pos = stream.tellg();
    if (pos == std::streampos(-1)) {
        if (!force) {
            return;
        }
        pos = totalSize;
    } else if (pos <= 0 && !force) {
        return;
    }

    float progress = static_cast<float>(pos) / static_cast<float>(totalSize);
    progress = std::clamp(progress, 0.0f, 1.0f);
    if (!force && lastProgress >= 0.0f && progress - lastProgress < 0.01f && progress < 1.0f) {
        return;
    }

    lastProgress = progress;
    lastProgressTime = now;
    progressCallback(progress);
}

std::string getSymbolFromAtomicNumber(int atomicNumber) {
    static const char* kChemicalSymbols[] = {
        "X", "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
        "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
        "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
        "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
        "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn",
        "Sb", "Te", "I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd",
        "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb",
        "Lu", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg",
        "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra", "Ac", "Th",
        "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm",
        "Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds",
        "Rg", "Cn", "Nh", "Fl", "Mc", "Lv", "Ts", "Og"
    };
    constexpr size_t kSymbolCount = sizeof(kChemicalSymbols) / sizeof(kChemicalSymbols[0]);

    if (atomicNumber >= 0 && atomicNumber < static_cast<int>(kSymbolCount)) {
        return kChemicalSymbols[atomicNumber];
    }
    return "X";
}

bool parsePrimvecLine(
    const std::string& line,
    int primvecLine,
    float cellVectors[3][3]) {
    if (primvecLine < 0 || primvecLine >= 3) {
        return false;
    }

    std::istringstream iss(line);
    float v1 = 0.0f;
    float v2 = 0.0f;
    float v3 = 0.0f;
    if (!(iss >> v1 >> v2 >> v3)) {
        return false;
    }

    cellVectors[primvecLine][0] = v1;
    cellVectors[primvecLine][1] = v2;
    cellVectors[primvecLine][2] = v3;
    return true;
}

bool parseAtomLine(
    const std::string& line,
    std::vector<atoms::infrastructure::FileIOManager::AtomData>& atoms) {
    std::istringstream iss(line);
    std::string firstToken;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!(iss >> firstToken >> x >> y >> z)) {
        return false;
    }

    int atomicNumber = 0;
    std::istringstream tokenStream(firstToken);
    if (tokenStream >> atomicNumber) {
        atoms.emplace_back(getSymbolFromAtomicNumber(atomicNumber), x, y, z);
        return true;
    }

    atoms.emplace_back(firstToken, x, y, z);
    return true;
}

std::string ltrim(const std::string& input) {
    const size_t start = input.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return std::string();
    }
    return input.substr(start);
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

bool isKeywordLine(const std::string& trimmed) {
    return startsWith(trimmed, "BEGIN_") ||
           startsWith(trimmed, "END_") ||
           startsWith(trimmed, "PRIMVEC") ||
           startsWith(trimmed, "PRIMCOORD") ||
           startsWith(trimmed, "ATOMS");
}

} // namespace

XsfParser::ParseResult XsfParser::parseStructureFile(
    const std::string& filePath,
    const ProgressCallback& progressCallback) {
    ParseResult result;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.success = false;
        result.errorMessage = "Failed to open file: " + filePath;
        SPDLOG_ERROR("{}", result.errorMessage);
        return result;
    }

    file.seekg(0, std::ios::end);
    const std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    float lastProgress = -1.0f;
    std::chrono::steady_clock::time_point lastProgressTime {};
    uint64_t progressCounter = 0;

    std::string line;
    bool readingPrimvec = false;
    bool readingPrimcoord = false;
    int atomCount = 0;
    int currentAtomLine = 0;
    int primvecLine = 0;

    try {
        reportProgress(file, fileSize, lastProgress, lastProgressTime, progressCallback, true);
        while (std::getline(file, line)) {
            ++progressCounter;
            if (progressCounter % kLineStep == 0) {
                reportProgress(file, fileSize, lastProgress, lastProgressTime, progressCallback);
            }

            std::istringstream iss(line);
            std::string firstToken;
            iss >> firstToken;

            if (firstToken == "CRYSTAL") {
                continue;
            }
            if (firstToken == "PRIMVEC") {
                readingPrimvec = true;
                primvecLine = 0;
                continue;
            }
            if (readingPrimvec && primvecLine < 3) {
                parsePrimvecLine(line, primvecLine, result.cellVectors);
                ++primvecLine;
                if (primvecLine >= 3) {
                    readingPrimvec = false;
                }
            }

            if (firstToken == "PRIMCOORD") {
                readingPrimcoord = true;
                currentAtomLine = 0;
                continue;
            }
            if (readingPrimcoord) {
                if (currentAtomLine == 0) {
                    iss.clear();
                    iss.str(line);
                    iss >> atomCount;
                    ++currentAtomLine;
                    continue;
                }

                if (currentAtomLine <= atomCount) {
                    parseAtomLine(line, result.atoms);
                    ++currentAtomLine;
                    if (currentAtomLine > atomCount) {
                        readingPrimcoord = false;
                    }
                }
            }
        }

        reportProgress(file, fileSize, lastProgress, lastProgressTime, progressCallback, true);
        file.close();
        if (progressCallback) {
            progressCallback(1.0f);
        }

        if (primvecLine == 3 && !result.atoms.empty()) {
            result.success = true;
        } else {
            result.success = false;
            result.errorMessage = "Invalid XSF file structure";
            SPDLOG_ERROR("{}: Cell vectors: {}, Atoms: {}", result.errorMessage, primvecLine, result.atoms.size());
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Exception during parsing: " + std::string(e.what());
        SPDLOG_ERROR("{}", result.errorMessage);
    }

    return result;
}

XsfParser::Grid3DParseResult XsfParser::parseGridFile(
    const std::string& filePath,
    const ProgressCallback& progressCallback) {
    Grid3DParseResult result;

    const std::string blockBegin = "BEGIN_BLOCK_DATAGRID_3D";
    const std::string blockEnd = "END_BLOCK_DATAGRID_3D";
    const std::string gridPrefix = "BEGIN_DATAGRID_3D";
    const float kCellEps = 1e-6f;

    {
        std::ifstream atomsFile(filePath);
        if (atomsFile.is_open()) {
            std::string line;
            bool inAtoms = false;
            while (std::getline(atomsFile, line)) {
                const std::string trimmed = ltrim(line);
                if (trimmed.empty()) {
                    continue;
                }
                if (!inAtoms) {
                    if (startsWith(trimmed, "ATOMS")) {
                        inAtoms = true;
                        continue;
                    }
                    if (startsWith(trimmed, blockBegin)) {
                        break;
                    }
                    continue;
                }
                if (isKeywordLine(trimmed)) {
                    break;
                }

                std::istringstream iss(trimmed);
                int atomicNumber = 0;
                float x = 0.0f;
                float y = 0.0f;
                float z = 0.0f;
                if (iss >> atomicNumber >> x >> y >> z) {
                    result.atoms.emplace_back(getSymbolFromAtomicNumber(atomicNumber), x, y, z);
                }
            }
        }
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.success = false;
        result.errorMessage = "Failed to open file: " + filePath;
        SPDLOG_ERROR("{}", result.errorMessage);
        return result;
    }

    file.seekg(0, std::ios::end);
    const std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    float lastProgress = -1.0f;
    std::chrono::steady_clock::time_point lastProgressTime {};
    uint64_t progressCounter = 0;
    bool inBlock = false;
    std::string token;

    reportProgress(file, fileSize, lastProgress, lastProgressTime, progressCallback, true);

    auto calcDownsampleDim = [](int dim, int factor) {
        if (dim <= 1) {
            return 1;
        }
        return (dim - 1) / factor + 1;
    };

    while (file >> token) {
        ++progressCounter;
        if (progressCounter % kTokenStep == 0) {
            reportProgress(file, fileSize, lastProgress, lastProgressTime, progressCallback);
        }

        if (token == blockBegin) {
            inBlock = true;
            continue;
        }
        if (token == blockEnd) {
            inBlock = false;
            continue;
        }
        if (!inBlock || token.rfind(gridPrefix, 0) != 0) {
            continue;
        }

        atoms::infrastructure::FileIOManager::Grid3DSet gridSet;
        auto& grid = gridSet.high;

        if (token.size() > gridPrefix.size()) {
            size_t start = gridPrefix.size();
            if (token[start] == '_' || token[start] == ':') {
                ++start;
            }
            if (start < token.size()) {
                grid.label = token.substr(start);
            }
        }
        if (grid.label.empty() && !(file >> grid.label)) {
            result.success = false;
            result.errorMessage = "Missing DATAGRID_3D label";
            return result;
        }

        if (!(file >> grid.dims[0] >> grid.dims[1] >> grid.dims[2])) {
            result.success = false;
            result.errorMessage = "Failed to read DATAGRID_3D dimensions";
            return result;
        }
        if (grid.dims[0] <= 0 || grid.dims[1] <= 0 || grid.dims[2] <= 0) {
            result.success = false;
            result.errorMessage = "Invalid DATAGRID_3D dimensions";
            return result;
        }

        gridSet.medium.label = grid.label;
        gridSet.low.label = grid.label;
        for (int axis = 0; axis < 3; ++axis) {
            gridSet.medium.dims[axis] = calcDownsampleDim(grid.dims[axis], 2);
            gridSet.low.dims[axis] = calcDownsampleDim(grid.dims[axis], 4);
        }
        gridSet.hasMedium = (gridSet.medium.dims[0] != grid.dims[0] ||
            gridSet.medium.dims[1] != grid.dims[1] ||
            gridSet.medium.dims[2] != grid.dims[2]);
        gridSet.hasLow = (gridSet.low.dims[0] != grid.dims[0] ||
            gridSet.low.dims[1] != grid.dims[1] ||
            gridSet.low.dims[2] != grid.dims[2]);

        if (!(file >> grid.origin[0] >> grid.origin[1] >> grid.origin[2])) {
            result.success = false;
            result.errorMessage = "Failed to read DATAGRID_3D origin";
            return result;
        }
        for (int axis = 0; axis < 3; ++axis) {
            gridSet.medium.origin[axis] = grid.origin[axis];
            gridSet.low.origin[axis] = grid.origin[axis];
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (!(file >> grid.vectors[i][j])) {
                    result.success = false;
                    result.errorMessage = "Failed to read DATAGRID_3D vectors";
                    return result;
                }
                gridSet.medium.vectors[i][j] = grid.vectors[i][j];
                gridSet.low.vectors[i][j] = grid.vectors[i][j];
            }
        }

        if (!result.hasCellVectors) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    result.cellVectors[i][j] = grid.vectors[i][j];
                }
            }
            result.hasCellVectors = true;
        } else if (result.cellVectorsConsistent) {
            for (int i = 0; i < 3 && result.cellVectorsConsistent; ++i) {
                for (int j = 0; j < 3; ++j) {
                    if (std::fabs(result.cellVectors[i][j] - grid.vectors[i][j]) > kCellEps) {
                        result.cellVectorsConsistent = false;
                        break;
                    }
                }
            }
        }

        const size_t totalValues =
            static_cast<size_t>(grid.dims[0]) *
            static_cast<size_t>(grid.dims[1]) *
            static_cast<size_t>(grid.dims[2]);
        const size_t mediumTotal =
            static_cast<size_t>(gridSet.medium.dims[0]) *
            static_cast<size_t>(gridSet.medium.dims[1]) *
            static_cast<size_t>(gridSet.medium.dims[2]);
        const size_t lowTotal =
            static_cast<size_t>(gridSet.low.dims[0]) *
            static_cast<size_t>(gridSet.low.dims[1]) *
            static_cast<size_t>(gridSet.low.dims[2]);

        grid.values.reserve(totalValues);
        if (gridSet.hasMedium) {
            gridSet.medium.values.reserve(mediumTotal);
        }
        if (gridSet.hasLow) {
            gridSet.low.values.reserve(lowTotal);
        }

        int iIndex = 0;
        int jIndex = 0;
        int kIndex = 0;
        for (size_t idx = 0; idx < totalValues; ++idx) {
            float value = 0.0f;
            if (!(file >> value)) {
                result.success = false;
                result.errorMessage = "Failed to read DATAGRID_3D values";
                return result;
            }
            grid.values.push_back(value);
            if (gridSet.hasMedium && (iIndex % 2 == 0) && (jIndex % 2 == 0) && (kIndex % 2 == 0)) {
                gridSet.medium.values.push_back(value);
            }
            if (gridSet.hasLow && (iIndex % 4 == 0) && (jIndex % 4 == 0) && (kIndex % 4 == 0)) {
                gridSet.low.values.push_back(value);
            }

            ++iIndex;
            if (iIndex >= grid.dims[0]) {
                iIndex = 0;
                ++jIndex;
                if (jIndex >= grid.dims[1]) {
                    jIndex = 0;
                    ++kIndex;
                }
            }
            if (idx % kValueStep == 0) {
                reportProgress(file, fileSize, lastProgress, lastProgressTime, progressCallback);
            }
        }

        std::string endToken;
        if (!(file >> endToken)) {
            result.success = false;
            result.errorMessage = "Missing END_DATAGRID_3D token";
            return result;
        }
        while (endToken != "END_DATAGRID_3D") {
            if (endToken == blockEnd) {
                result.success = false;
                result.errorMessage = "Unexpected END_BLOCK_DATAGRID_3D before END_DATAGRID_3D";
                return result;
            }
            if (!(file >> endToken)) {
                result.success = false;
                result.errorMessage = "Missing END_DATAGRID_3D token";
                return result;
            }
        }

        result.grids.push_back(std::move(gridSet));
    }

    reportProgress(file, fileSize, lastProgress, lastProgressTime, progressCallback, true);
    file.close();
    if (progressCallback) {
        progressCallback(1.0f);
    }

    if (result.grids.empty()) {
        result.success = false;
        result.errorMessage = "No DATAGRID_3D blocks found";
        return result;
    }

    result.success = true;
    return result;
}

} // namespace io::infrastructure
