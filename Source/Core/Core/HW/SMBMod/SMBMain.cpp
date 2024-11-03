#include "Core/HW/SMBMod/SMBMain.h"

std::queue<std::array<uint8_t, BALL_STRUCT_LENGTH>> SMBMain::q;
u16 SMBMain::last_time = 0;
u16 SMBMain::frame_timer = 0;
u16 SMBMain::level = 1;
int SMBMain::frame_count = 0;
std::atomic<bool> SMBMain::running(true);
std::queue<sf::Packet> SMBMain::send_queue;
std::queue<sf::Packet> SMBMain::receive_queue;
std::mutex SMBMain::send_mutex;
std::mutex SMBMain::receive_mutex;
std::condition_variable SMBMain::send_cond;
std::condition_variable SMBMain::receive_cond;
std::vector<u64> SMBMain::starting_offsets{0x40000000, 0x40500000, 0x41000000, 0x41500000, 0x42000000};
std::vector<std::vector<u8>> SMBMain::level_files{};

void SMBMain::push(const std::array<uint8_t, BALL_STRUCT_LENGTH>& arr)
{
  if (q.size() >= QUEUE_CAPACITY)
  {
    pop();
  }
  q.push(arr);
}

std::array<uint8_t, BALL_STRUCT_LENGTH> SMBMain::front()
{
  if (!q.empty())
  {
    return q.front();
  }
  return std::array<uint8_t, BALL_STRUCT_LENGTH>{};
}

void SMBMain::pop()
{
  if (!q.empty())
  {
    std::array<uint8_t, BALL_STRUCT_LENGTH> arr = q.front();
    q.pop();
    Core::CPUThreadGuard guard(Core::System::GetInstance());
    
    int arr_pos = 0;
    for (int i = 0; i < segments.size(); i += 2)
    {
      for (int pos = segments[i]; pos < segments[i + 1]; pos++)
      {
        PowerPC::MMU::HostWrite_U8(guard, arr[arr_pos], (u32)(0x805BCB50 + pos));
        arr_pos++;
      }
    }
  }
}

bool SMBMain::empty()
{
  return q.empty();
}

size_t SMBMain::size()
{
  return q.size();
}

void SMBMain::frameLoop()
{
  Core::CPUThreadGuard guard(Core::System::GetInstance());

  initialSetup(guard);

  gameStateControl(guard);

  readBallPositions(guard);

  if (frame_count % 6 == 0) // Every 6th frame
  {
    prepareMovementPacket(guard);
  }

  // PanicAlertFmt("Unable to resolve write address {:x}", (u32)0x805BCB50);
  frame_count++;
}

void SMBMain::gameStateControl(Core::CPUThreadGuard& guard)
{
  if (PowerPC::MMU::HostRead_U32(guard, 0x8055399C) != level)
  {
    level = level == 1 ? 206 : 1;
  }

  // Next Level always 1
  PowerPC::MMU::HostWrite_U32(guard, level, 0x8055399C);
  // Next Level always 1 (fixes initial difficulty selection stage)
  PowerPC::MMU::HostWrite_U16(guard, level, 0x8047873A);
  // 99 Lives always
  PowerPC::MMU::HostWrite_U8(guard, 0x64, 0x805BC9A2);
}

void SMBMain::initialSetup(Core::CPUThreadGuard& guard)
{
  if (frame_count == 0)
  {
    SMBMain::readLevel();

    std::thread sender(SMBMain::send_tcp);
    std::thread receiver(SMBMain::receive_tcp);

    sender.detach();
    receiver.detach();
  }

  if (frame_count == 60)
  {
    modifyLookupTable(guard);
  }
}

void SMBMain::modifyLookupTable(Core::CPUThreadGuard& guard)
{
  // STG001 - Jungle
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[0]), 0x817F56D8);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[0].size()), 0x817F56DC);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[1]), 0x817F305C);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[1].size()), 0x817F3060);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[2]), 0x817F3050);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[2].size()), 0x817F3054);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[3]), 0x817EFAD4);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[3].size()), 0x817EFAD8);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[4]), 0x817EFAE0);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[4].size()), 0x817EFAE4);

  // STG206 - Playground
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[0]), 0x817F5EC4);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[0].size()), 0x817F5EC8);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[1]), 0x817F42BC);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[1].size()), 0x817F42C0);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[2]), 0x817F42B0);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[2].size()), 0x817F42B4);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[3]), 0x817EFB4C);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[3].size()), 0x817EFB50);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(starting_offsets[4]), 0x817EFB58);
  PowerPC::MMU::HostWrite_U32(guard, static_cast<u32>(SMBMain::level_files[4].size()), 0x817EFB5C);
}

void SMBMain::readBallPositions(Core::CPUThreadGuard& guard)
{
  std::unique_lock<std::mutex> lock(receive_mutex);
  while (!receive_queue.empty())
  {
    sf::Packet received_message = receive_queue.front();
    receive_queue.pop();
    size_t num_players = received_message.getDataSize() / 46;

    if (received_message.getDataSize() % 46 == 0)
    {
      for (int j = 0; j < num_players; j++)
      {
        int ball_number_offset = 0x1B0 * j;
        for (int i = 0; i < segments.size(); i += 2)
        {
          for (int pos = segments[i]; pos < segments[i + 1]; pos++)
          {
            u8 val;
            received_message >> val;
            PowerPC::MMU::HostWrite_U8(guard, val, 0x805BCB50 + ball_number_offset + pos);
          }
        }

        if (PowerPC::MMU::HostRead_U16(guard, 0x80553974) == last_time)
        {
          PowerPC::MMU::HostWrite_U8(guard, 0, (u32)(0x805BCB50 + ball_number_offset));
        }
      }
    }
  }
}

void SMBMain::prepareMovementPacket(Core::CPUThreadGuard& guard)
{
  std::array<uint8_t, BALL_STRUCT_LENGTH> arr;

  int arr_pos = 0;
  // Compress all necessary ball struct values into a smaller array
  for (int i = 0; i < segments.size(); i += 2)
  {
    for (int pos = segments[i]; pos < segments[i + 1]; pos++)
    {
      u8 val = PowerPC::MMU::HostRead_U8(guard, 0x805BC9A0 + pos);
      arr[arr_pos] = val;
      arr_pos++;
    }
  }

  // Only send packet if level timer isn't paused (player enters goal or on a menu) (looks glitchy
  // otherwise)
  if (PowerPC::MMU::HostRead_U16(guard, 0x80553974) != last_time)
  {
    sf::Packet packet;
    packet.append(arr.data(), arr.size() * sizeof(uint8_t));
    std::unique_lock<std::mutex> lock2(send_mutex);
    send_queue.push(packet);
    send_cond.notify_one();
  }
  last_time = PowerPC::MMU::HostRead_U16(guard, 0x80553974);
  frame_timer = PowerPC::MMU::HostRead_U16(guard, 0x80198c4e);  // This address almost always has a new value each frame
}

void SMBMain::send_tcp()
{
  sf::TcpSocket socket;
  sf::Socket::Status status = socket.connect("15.204.236.13", 5778);

  if (status != sf::Socket::Done)
  {
    return;
  }

  while (running)
  {
    std::unique_lock<std::mutex> lock(send_mutex);
    send_cond.wait(lock, [] { return !send_queue.empty() || !running; });


    while (!send_queue.empty())
    {
      sf::Packet packet = send_queue.front();
      send_queue.pop();
      lock.unlock();

      if (socket.send(packet) != sf::Socket::Done)
      {
        std::cerr << "Error sending message" << std::endl;
      }
      else
      {
        std::cout << "Message sent";
      }

      lock.lock();
    }
  }
}

void SMBMain::receive_tcp()
{
  sf::TcpSocket socket;
  sf::Socket::Status status = socket.connect("15.204.236.13", 5778);
  
  if (status != sf::Socket::Done)
  {
    return;
  }

  while (running)
  {
    sf::Packet packet;
    if (socket.receive(packet) == sf::Socket::Done)
    {
      std::unique_lock<std::mutex> lock2(receive_mutex);
      receive_queue.push(packet);
      receive_cond.notify_one();
    }
  }
}

void SMBMain::readLevel()
{
  std::vector<std::string> file_names = {"STAGE201.lz", "st201.tpl", "st201.gma", "bg201.gma", "bg201.tpl"};

  for (int i = 0; i < file_names.size(); i++)
  {
    std::ifstream file("levels/" + file_names[i], std::ios::binary | std::ios::ate);
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

    level_files.push_back(buffer);
  }
}

void SMBMain::stageInjection(u64 offset, u64 length, u8* buffer)
{
  
  for (int i = 0; i < SMBMain::starting_offsets.size(); ++i)
  {
    if (offset >= SMBMain::starting_offsets[i] && offset < SMBMain::starting_offsets[i] + 0x500000)
    {
      u64 file_pos = offset - starting_offsets[i];
      //PanicAlertFmt("{:x}", offset);

      for (int j = file_pos; j < file_pos + length; j++)
      {
        if (j < SMBMain::level_files[i].size())
        {
          *buffer = SMBMain::level_files[i][j];
          buffer++;
        }
      }

      return;
    }
  }
}
