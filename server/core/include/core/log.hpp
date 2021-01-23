#pragma once
#include <spdlog/logger.h>

namespace core {
  class Log {
    std::shared_ptr<spdlog::logger> instance_;

  public:
    Log() noexcept;
    spdlog::logger* operator->() { return instance_.get(); }
  };

  extern Log g_log;
} // namespace core
