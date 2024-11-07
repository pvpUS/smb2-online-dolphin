#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <random>
#include "Core/Core.h"
#include <Core/HW/SMBMod/SMBLevel.h>

namespace fs = std::filesystem;

struct ArrayHasher
{
  std::size_t operator()(const Digest& a) const
  {
    std::size_t h = 0;

    for (auto e : a)
    {
      h ^= std::hash<int>{}(e) + 0x9e3779b9 + (h << 6) + (h >> 2);
    }
    return h;
  }
};

class SMBFileSystem
{
private:
  std::vector<SMBLevel> levels;
  void getFilesWithExtension(const fs::path& directory, std::vector<std::string>& filePaths, std::string extension);

public:
  SMBFileSystem();
  void loadRandomLevel();
  void reloadLevels();
  SMBLevel currentLevel;
  std::unordered_map<Digest, SMBBackground, ArrayHasher> backgrounds;
  std::vector<SMBLevel> getLevels();
};
