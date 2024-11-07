#include "Core/HW/SMBMod/SMBBackground.h"

SMBBackground::SMBBackground(std::string backgroundFile)
{
  readSMBBackgroundFile(backgroundFile);
}

SMBBackground::SMBBackground()
{
}


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

void SMBBackground::readSMBBackgroundFile(std::string fileName)
{
  std::vector<u8> file = readToVector(fileName);
  u8 version = file.at(0);
  u32 gmaOffset = 13;
  u32 tplOffset = (file.at(1) << 24) + (file.at(2) << 16) + (file.at(3) << 8) + file.at(4);
  u32 dspLOffset = (file.at(5) << 24) + (file.at(6) << 16) + (file.at(7) << 8) + file.at(8);
  u32 dspROffset = (file.at(9) << 24) + (file.at(10) << 16) + (file.at(11) << 8) + file.at(12);

  std::vector<u8> gma;
  std::vector<u8> tpl;
  std::vector<u8> dspL;
  std::vector<u8> dspR;

  // Fail if next offsets are larger than previous or if offsets are out of bounds
  if (!(tplOffset > gmaOffset && dspLOffset > tplOffset && dspROffset > dspLOffset) ||
      gmaOffset > file.size() || tplOffset > file.size() || dspLOffset > file.size() || dspROffset > file.size())
  {
    PanicAlertFmt("file is corrupted! {}", fileName);
    throw std::runtime_error("file is corrupted!");
  }

  for (u32 i = gmaOffset; i < tplOffset; i++)
  {
    gma.push_back(file.at(i));
  }
  for (u32 i = tplOffset; i < dspLOffset; i++)
  {
    tpl.push_back(file.at(i));
  }
  for (u32 i = dspLOffset; i < dspROffset; i++)
  {
    dspL.push_back(file.at(i));
  }
  for (u32 i = dspROffset; i < file.size(); i++)
  {
    dspR.push_back(file.at(i));
  }

  gmaFile = gma;
  tplFile = tpl;
  dspLFile = dspL;
  dspRFile = dspR;
  fileFormatVersion = version;
  bgHash = Common::SHA1::CalculateDigest(file);
}

