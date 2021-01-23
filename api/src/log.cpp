#include "log.hpp"
#include <spdlog/sinks/rotating_file_sink.h>

using namespace gallery;

Log gallery::g_log{};

Log::Log() noexcept {
  constexpr const size_t max_size{ 1048576 * 5 };
  constexpr const size_t max_files{ 5 };
  const std::string      logger_name{ "gallery" };
  const std::string      filename{ "logs/api.log" };

  instance = spdlog::rotating_logger_mt(logger_name, filename, max_size, max_files);
}

spdlog::logger* Log::operator->() {
  return instance.get();
}
