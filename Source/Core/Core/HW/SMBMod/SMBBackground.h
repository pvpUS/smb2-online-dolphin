#pragma once
#include <string>
#include <vector>
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

using Digest = std::array<u8, 160 / 8>;

class SMBBackground
{
private:
  std::vector<u8> readToVector(std::string fileName);
public:
  SMBBackground(std::string tplFileName, std::string gmaFileName, std::string dspLFileName, std::string dspRFileName);
  std::vector<u8> tplFile;
  std::vector<u8> gmaFile;
  std::vector<u8> dspLFile;
  std::vector<u8> dspRFile;
  Digest bgHash;
};
