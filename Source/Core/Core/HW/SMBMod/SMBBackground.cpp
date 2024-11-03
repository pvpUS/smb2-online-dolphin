#include "Core/HW/SMBMod/SMBBackground.h"

SMBBackground::SMBBackground(std::string tplFileName, std::string gmaFileName,
                             std::string dspFileName):
  tplFile(readToVector(tplFileName)),
  gmaFile(readToVector(gmaFileName)),
  dspFile(readToVector(dspFileName)) {};


std::vector<u8> SMBBackground::readToVector(std::string fileName)
{
  std::ifstream file("stages/backgrounds/" + fileName, std::ios::binary | std::ios::ate);
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

