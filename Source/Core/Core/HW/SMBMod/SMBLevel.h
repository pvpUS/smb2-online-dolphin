#pragma once
#include <string>
#include <vector>
#include "Core/Core.h"
#include "Common/Crypto/SHA1.h"
#include "Core/Core.h"
#include <queue>
#include <vector>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <Core/HW/SMBMod/SMBBackground.h>

using Digest = std::array<u8, 160 / 8>;

class SMBLevel
{
private:
  std::vector<u8> readToVector(std::string fileName);
  void readSMBLevelFile(std::string fileName);

public:
  SMBLevel(std::string stageFile);
  SMBLevel();
  std::vector<u8> gmaFile;
  std::vector<u8> tplFile;
  std::vector<u8> lzFile;
  std::string levelName;
  u8 levelDifficulty;
  std::string author;
  u8 fileFormatVersion;
  Digest bgHash;
};
