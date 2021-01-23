#include "log.hpp"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

using namespace gallery;

#if defined(DEBUG)
  #define DEBUG_RELEASE_SWITCH(deb, rel) deb
#else
  #define DEBUG_RELEASE_SWITCH(deb, rel) rel
#endif

Log gallery::g_log{};

Log::Log() noexcept {
  constexpr const auto   log_level{ DEBUG_RELEASE_SWITCH(spdlog::level::trace, spdlog::level::info) };
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
  instance_->set_level(log_level);
}
