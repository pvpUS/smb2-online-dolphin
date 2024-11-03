#include "Core/HW/SMBMod/SMBLevel.h"

SMBLevel::SMBLevel(std::string tplFileName, std::string gmaFileName, std::string lzFileName,
                   std::string bgTplFileName, std::string bgGmaFileName, std::string bgDspFileName,
                   std::string levelName, u8 levelDifficulty, std::string author,
                   u8 fileFormatVersion):
  tplFile(readToVector(tplFileName)),
  gmaFile(readToVector(gmaFileName)),
  lzFile(readToVector(lzFileName)),
  background(*new SMBBackground(bgTplFileName, bgGmaFileName,bgDspFileName)),
  levelName(levelName),
  levelDifficulty(levelDifficulty),
  author(author),
  fileFormatVersion(fileFormatVersion) {};

std::vector<u8> SMBLevel::readToVector(std::string fileName)
{
  std::ifstream file("stages/levels/" + fileName, std::ios::binary | std::ios::ate);
  if (!file.is_open())
  {
    PanicAlertFmt("file couldnt open!");
    throw std::runtime_error("Could not open file.");
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<u8> buffer(size);
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
  {
    PanicAlertFmt("file couldnt read!");
    throw std::runtime_error("Could not read file.");
  }
  return buffer;
}
