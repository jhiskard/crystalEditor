#include "memfs_chunk_stream.h"

#include "../../config/log_config.h"

namespace io::platform {

void MemfsChunkStream::WriteChunk(
    const std::string& fileName,
    int32_t offset,
    uintptr_t data,
    int32_t length) {
    FILE*& file = m_FileMap[fileName];

    if (file == nullptr) {
        file = fopen(fileName.c_str(), "wb+");
        if (file == nullptr) {
            SPDLOG_ERROR("Failed to open file: {}", fileName);
            return;
        }
    }

    if (fseek(file, offset, SEEK_SET) != 0) {
        SPDLOG_ERROR("Failed to seek file: {}", fileName);
        return;
    }

    const size_t written = fwrite(reinterpret_cast<void*>(data), 1, static_cast<size_t>(length), file);
    if (written != static_cast<size_t>(length)) {
        SPDLOG_ERROR("Failed to write data to file: {}", fileName);
    }

    fflush(file);
}

void MemfsChunkStream::CloseFile(const std::string& fileName) {
    const auto it = m_FileMap.find(fileName);
    if (it == m_FileMap.end()) {
        return;
    }

    fclose(it->second);
    m_FileMap.erase(it);
}

void MemfsChunkStream::CloseAll() {
    for (auto& [_, file] : m_FileMap) {
        if (file != nullptr) {
            fclose(file);
        }
    }
    m_FileMap.clear();
}

} // namespace io::platform

