#pragma once
#include "Core/PowerPC/PowerPC.h"
#include "Core/PowerPC/MMU.h"
#include "Core/Core.h"
#include "Core/System.h"
#include "Common/MsgHandler.h"
#include <queue>
#include <vector>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <SFML/Network.hpp>
#include <fstream>

const size_t BALL_STRUCT_LENGTH = 46;
const size_t QUEUE_CAPACITY = 100;
const std::vector<int> segments = {0x00, 0x10, 0x1C, 0x2E, 0x88, 0x8C, 0x90, 0x94, 0x9C, 0xA0};


class SMBMain
{
private:
  static std::queue<std::array<uint8_t, BALL_STRUCT_LENGTH>> q;
  static u16 last_time;
  static u16 frame_timer;
  static u16 level;
  static std::atomic<bool> running;
  static std::queue<sf::Packet> send_queue;
  static std::queue<sf::Packet> receive_queue;
  static std::mutex send_mutex;
  static std::mutex receive_mutex;
  static std::condition_variable send_cond;
  static std::condition_variable receive_cond;
  static std::vector<std::vector<u8>> level_files;

public:
  static int frame_count;
  static std::vector<u64> starting_offsets;
  static void push(const std::array<uint8_t, BALL_STRUCT_LENGTH>& arr);
  static std::array<uint8_t, BALL_STRUCT_LENGTH> front();
  static void pop();
  static bool empty();
  static size_t size();
  static void send_tcp();
  static void receive_tcp();
  static void gameStateControl(Core::CPUThreadGuard& guard);
  static void modifyLookupTable(Core::CPUThreadGuard& guard);
  static void prepareMovementPacket(Core::CPUThreadGuard& guard);
  static void readBallPositions(Core::CPUThreadGuard& guard);
  static void initialSetup(Core::CPUThreadGuard& guard);
  static void frameLoop();
  static void readLevel();
  static void stageInjection(u64 offset, u64 length, u8* buffer);
};
