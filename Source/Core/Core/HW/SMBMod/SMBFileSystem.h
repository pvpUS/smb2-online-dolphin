#pragma once
#include <string>
#include <vector>
#include <Core/HW/SMBMod/SMBLevel.h>

class SMBFileSystem
{
public:
  SMBFileSystem();
  void loadRandomLevel();
  bool loadLevel(std::string levelPath);
  std::vector<SMBLevel> getLevels();
};
