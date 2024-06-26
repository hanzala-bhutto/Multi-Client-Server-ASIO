#pragma once

#include <iostream>
#include <deque>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>
#include <memory>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <optional>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

namespace fs = std::filesystem;
using namespace fs;