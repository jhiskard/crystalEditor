#pragma once

// Standard library
#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>

namespace io::platform {

/**
 * @brief MemFS chunk stream adapter for browser file upload path.
 * @details Owns fopen/fseek/fwrite lifecycle for chunked upload buffers.
 */
class MemfsChunkStream {
public:
    void WriteChunk(const std::string& fileName, int32_t offset, uintptr_t data, int32_t length);
    void CloseFile(const std::string& fileName);
    void CloseAll();

private:
    std::unordered_map<std::string, FILE*> m_FileMap;
};

} // namespace io::platform

