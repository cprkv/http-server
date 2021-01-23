#include "log.hpp"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

using namespace gallery;

Log gallery::g_log{};

// todo: make it async and console and debug stuff.... https://github.com/gabime/spdlog

Log::Log() noexcept {
  constexpr const size_t max_file_size{ 1024 * 1024 * 4 };
  constexpr const size_t max_files{ 5 };
  const std::string      logger_name{ "gallery" };
  const std::string      filename{ "logs/api.log" };

  spdlog::init_thread_pool(1024 * 8, 1);

  auto stdout_sink   = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, max_file_size, max_files);
  auto sinks         = std::vector<spdlog::sink_ptr>{ stdout_sink, rotating_sink };

  instance_ = std::make_shared<spdlog::async_logger>(
      logger_name, sinks.begin(), sinks.end(), spdlog::thread_pool(),
      spdlog::async_overflow_policy::block);

#if defined(DEBUG)
  instance_->set_level(spdlog::level::trace);
#endif
}

spdlog::logger* Log::operator->() {
  return instance_.get();
}
