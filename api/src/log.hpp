#pragma once
#include <spdlog/logger.h>

namespace gallery {
  class Log {
    std::shared_ptr<spdlog::logger> instance;

  public:
    Log() noexcept;
    spdlog::logger* operator->();
  };

  extern Log g_log;
} // namespace gallery
