#include "Core/HW/SMBMod/SMBLevel.h"

SMBLevel::SMBLevel(std::string tplFileName, std::string gmaFileName, std::string lzFileName,
                   std::string bgTplFileName, std::string bgGmaFileName, std::string bgDspLFileName,
                   std::string bgDspRFileName, std::string levelName, u8 levelDifficulty,
                   std::string author, u8 fileFormatVersion)
    : tplFile(readToVector(tplFileName)), gmaFile(readToVector(gmaFileName)),
      lzFile(readToVector(lzFileName)),
      background(*new SMBBackground(bgTplFileName, bgGmaFileName, bgDspLFileName, bgDspRFileName)),
      levelName(levelName), levelDifficulty(levelDifficulty), author(author),
      fileFormatVersion(fileFormatVersion){};

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

void SMBLevel::readSMBLevelFile(std::string fileName)
{
  std::vector<u8> file = readToVector(fileName);
  u8 version = file.at(0);
  u8 difficulty = file.at(1);

  std::vector<u8> shaHash;
  std::vector<u8> tpl;
  std::vector<u8> lz;
  std::vector<u8> gma;
  std::string levelBuilder;
  std::string authorBuilder;
  for (int i = 2; i < 23; i++) // 2-22 is background SHA1
  {
    shaHash.push_back(file.at(i));
  }
  u32 tplOffset = (file.at(23) << 24) + (file.at(24) << 16) + (file.at(25) << 8) + file.at(26);
  u32 lzOffset = (file.at(27) << 24) + (file.at(28) << 16) + (file.at(29) << 8) + file.at(30);
  u32 levelNameOffset = 31;

  // Fail if offests are out of bounds
  if (lzOffset > file.size() || tplOffset > file.size())
  {
    PanicAlertFmt("file is corrupted!");
    throw std::runtime_error("file is corrupted!");
  }

  int i = levelNameOffset;
  while (i < file.size())
  {
    u8 byte = file.at(i);
    i++;
    if (byte == 0x00)
      break;
    else if (std::isalnum(byte))
      levelBuilder += static_cast<char>(byte);
  }

  while (i < file.size())
  {
    u8 byte = file.at(i);
    i++;
    if (byte == 0x00)
      break;
    else if (std::isalnum(byte))
      authorBuilder += static_cast<char>(byte);
  }

  for (int a = i; a < tplOffset; a++) 
  {
    gma.push_back(file.at(a));
  }
  for (int i = tplOffset; i < lzOffset; i++)
  {
    tpl.push_back(file.at(i));
  }
  for (int i = lzOffset; i < file.size(); i++)
  {
    lz.push_back(file.at(i));
  }

  hash = shaHash;
  gmaFile = gma;
  tplFile = tpl;
  lzFile = lz;
  levelName = levelBuilder;
  author = authorBuilder;
  levelDifficulty = difficulty;
  fileFormatVersion = version;
}

