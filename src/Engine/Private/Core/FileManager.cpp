
#include "Core/FileManager.h"
#include <fstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

FileBlob::FileBlob(const std::vector<char>& inBlob) : blob(inBlob) {}

size_t FileBlob::GetFileSize() const { return blob.size(); }

const uint32_t* FileBlob::GetData() const
{
    return reinterpret_cast<const uint32_t*>(blob.data());
}

FileBlob FileManager::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    const size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
