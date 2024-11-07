#include "Core/HW/SMBMod/SMBLevel.h"

SMBLevel::SMBLevel(std::string stageFile)
{
  readSMBLevelFile(stageFile);
}

SMBLevel::SMBLevel()
{
}

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
  u16 timer = (file.at(2) << 8) + file.at(3);

  std::vector<u8> shaHash;
  std::vector<u8> tpl;
  std::vector<u8> lz;
  std::vector<u8> gma;
  std::string levelBuilder;
  std::string authorBuilder;
 
  for (int i = 4; i < 24; i++) // 2-21 is background SHA1
  {
    shaHash.push_back(file.at(i));
  }
  u32 tplOffset = (file.at(25) << 24) + (file.at(26) << 16) + (file.at(27) << 8) + file.at(28);
  u32 lzOffset = (file.at(29) << 24) + (file.at(30) << 16) + (file.at(31) << 8) + file.at(32);
  u32 levelNameOffset = 33;

  // Fail if offests are out of bounds
  if (lzOffset > file.size() || tplOffset > file.size())
  {
    PanicAlertFmt("file is corrupted! {} {:x} {:x}", fileName, tplOffset, lzOffset);
    throw std::runtime_error("file is corrupted!");
  }

  int i = levelNameOffset;
  while (i < file.size())
  {
    u8 byte = file.at(i);
    i++;
    if (byte == 0x00)
      break;
    else if (std::isalnum(byte) || std::ispunct(byte) || byte == ' ')
      levelBuilder += static_cast<char>(byte);
  }

  while (i < file.size())
  {
    u8 byte = file.at(i);
    i++;
    if (byte == 0x00)
      break;
    else if (std::isalnum(byte) || std::ispunct(byte) || byte == ' ')
      authorBuilder += static_cast<char>(byte);
  }

  for (u32 a = i; a < tplOffset; a++) 
  {
    gma.push_back(file.at(a));
  }
  for (u32 a = tplOffset; a < lzOffset; a++)
  {
    tpl.push_back(file.at(a));
  }
  for (u32 a = lzOffset; a < file.size(); a++)
  {
    lz.push_back(file.at(a));
  }

  bgHash = *new Digest();
  std::copy(shaHash.begin(), shaHash.end(), bgHash.begin());

  gmaFile = gma;
  tplFile = tpl;
  lzFile = lz;
  levelName = levelBuilder;
  author = authorBuilder;
  levelDifficulty = difficulty;
  fileFormatVersion = version;
  levelTimer = timer;
}

