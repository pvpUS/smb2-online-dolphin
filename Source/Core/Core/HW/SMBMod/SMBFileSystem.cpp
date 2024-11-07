#include "Core/HW/SMBMod/SMBFileSystem.h"

SMBFileSystem::SMBFileSystem()
{
  reloadLevels();
  loadRandomLevel();
}

void SMBFileSystem::getFilesWithExtension(const fs::path& directory, std::vector<std::string>& filePaths, std::string extension)
{
  for (const auto& entry : fs::recursive_directory_iterator(directory))
  {
    if (entry.is_regular_file() && entry.path().extension() == ("." + extension))
    {
      filePaths.push_back(entry.path().lexically_relative(directory).string());
    }
  }
}

void SMBFileSystem::loadRandomLevel()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, static_cast<int>(levels.size() - 1));

  int random_index = dist(gen);

  currentLevel = levels[random_index];
}

void SMBFileSystem::reloadLevels()
{
  fs::path bgDirectory = "stages/backgrounds/";
  std::vector<std::string> bgs;

  fs::path lvlDirectory = "stages/levels/";
  std::vector<std::string> lvls;

  getFilesWithExtension(bgDirectory, bgs, "mbbg");
  getFilesWithExtension(lvlDirectory, lvls, "mblvl");

  for (std::string bg : bgs)
  {
    SMBBackground background = SMBBackground(bg);
    backgrounds[background.bgHash] = background;
  }

   for (std::string lvl : lvls)
  {
     levels.push_back(*new SMBLevel(lvl));
  }
}

std::vector<SMBLevel> SMBFileSystem::getLevels()
{
  return levels;
}
