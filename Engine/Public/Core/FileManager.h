#pragma once

#include <stdint.h>
#include <string>
#include <vector>

class FileBlob
{
  public:
    FileBlob(const std::vector<char>& inBlob);
    FileBlob() = default;

    size_t GetFileSize() const;
    const uint32_t* GetData() const;

  protected:
    std::vector<char> blob;
};

class FileManager
{
  public:
    static FileBlob ReadFile(const std::string& filename);
};
